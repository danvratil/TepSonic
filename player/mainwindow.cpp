/*
 * TEPSONIC
 * Copyright 2009 Dan Vratil <vratil@progdansoft.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.
 */

// For random()
#include <cstdlib>
#include <ctime>

#include "constants.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "preferencesdialog.h"
#include "playlist/playlistitemdelegate.h"
#include "playlist/playlistproxymodel.h"
#include "playlist/playlistmodel.h"
#include "collections/collectionproxymodel.h"
#include "collections/collectionmodel.h"
#include "collections/collectionitem.h"
#include "collections/collectionitemdelegate.h"
#include "abstractplugin.h"
#include "taskmanager.h"
#include "pluginsmanager.h"
#include "tools.h"
#include "supportedformats.h"

#include <QMessageBox>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QDate>
#include <QDateTime>
#include <QTime>
#include <phonon/seekslider.h>
#include <phonon/volumeslider.h>
#include "qxtglobalshortcut.h"

#include <QDebug>

MainWindow::MainWindow(Player *player)
{

    _settings = new QSettings(_CONFIGDIR + QDir::separator() + "main.conf",QSettings::IniFormat,this);

    // Initialize pseudo-random numbers generator
    srand(time(NULL));

    _canClose = false;
    _collectionProxyModel = NULL;
    _collectionModel = NULL;

    // Create default UI
    _ui = new Ui::MainWindow();
    _ui->setupUi(this);

    createMenus();
    bindShortcuts();

    _appIcon = new QIcon(":/icons/mainIcon");
    QApplication::setWindowIcon(*_appIcon);

    _trayIcon = new TrayIcon(*_appIcon, this);
    connect(_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayClicked(QSystemTrayIcon::ActivationReason)));
    connect(_trayIcon, SIGNAL(mouseWheelScrolled(int)),
            this, SLOT(trayIconMouseWheelScrolled(int)));
    _trayIcon->setVisible(true);
    _trayIcon->setContextMenu(_trayIconMenu);
    _trayIcon->setToolTip(tr("Player is stopped"));


    _playlistLengthLabel = new QLabel(this);
    _ui->statusBar->addPermanentWidget(_playlistLengthLabel,0);
    _playlistLengthLabel->setText(tr("%n track(s)", "", 0).append(" (00:00)"));

    QStringList headers = QStringList()<< tr("Filename")
                          << tr("Track")
                          << tr("Interpret")
                          << tr("Track name")
                          << tr("Album")
                          << tr("Genre")
                          << tr("Year")
                          << tr("Length");

    _playlistProxyModel = new PlaylistProxyModel(this);
    _playlistModel = new PlaylistModel(this, headers, _playlistProxyModel);
    _playlistItemDelegate = new PlaylistItemDelegate(this, _playlistModel, _ui->playlistBrowser, _playlistProxyModel);

    connect(_playlistModel,SIGNAL(playlistLengthChanged(int,int)),
            this,SLOT(playlistLengthChanged(int,int)));

    _playlistProxyModel->setSourceModel(_playlistModel);
    _playlistProxyModel->setDynamicSortFilter(false);

    _ui->playlistBrowser->setModel(_playlistProxyModel);
    _ui->playlistBrowser->setItemDelegate(_playlistItemDelegate);
    _ui->playlistBrowser->setDragEnabled(true);
    _ui->playlistBrowser->setDropIndicatorShown(true);
    _ui->playlistBrowser->setSortingEnabled(true);
    _ui->playlistBrowser->viewport()->setAcceptDrops(true);
    _ui->playlistBrowser->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    _ui->playlistBrowser->setAlternatingRowColors(true);
    _ui->playlistBrowser->sortByColumn(-1);
    // Open ui->menuVisible_columns when PlaylistBrowser's header's context menu is requested
    connect(_ui->playlistBrowser->header(),SIGNAL(customContextMenuRequested(QPoint)),
            this,SLOT(showPlaylistContextMenu(QPoint)));
    // Hide the first column (with filename)
    _ui->playlistBrowser->hideColumn(0);
    _selectionModel = _ui->playlistBrowser->selectionModel();

    _taskManager = new TaskManager(&_playlistModel,&_collectionModel);
    connect(_ui->playlistBrowser,SIGNAL(addedFiles(QStringList,int)),_taskManager,SLOT(addFilesToPlaylist(QStringList,int)));
    connect(_taskManager,SIGNAL(collectionsPopulated()),this,SLOT(fixCollectionProxyModel()));
    connect(_taskManager,SIGNAL(taskStarted(QString)),_ui->statusBar,SLOT(showWorkingBar(QString)));
    connect(_taskManager,SIGNAL(taskDone()),_ui->statusBar,SLOT(cancelAction()));
    // This refreshes the filter when an item is added to the playlist so the item appears immediately
    connect(_taskManager,SIGNAL(playlistPopulated()),
            _playlistProxyModel, SLOT(invalidate()));
    connect(_taskManager,SIGNAL(insertItemToPlaylist(Player::MetaData,int)),
            _playlistModel,SLOT(insertItem(Player::MetaData, int)));
    connect(_taskManager, SIGNAL(clearCollectionModel()), _collectionModel, SLOT(clear()));

    restoreGeometry(_settings->value("Window/Geometry", saveGeometry()).toByteArray());
    restoreState(_settings->value("Window/State", saveState()).toByteArray());
    _ui->viewsSplitter->restoreState(_settings->value("Window/ViewsSplit").toByteArray());

    if (_settings->value("Collections/EnableCollections",true).toBool()==true) {
        setupCollections();
        _taskManager->populateCollections();
        if (_settings->value("Collections/AutoRebuildAfterStart",false).toBool()==true) {
            _taskManager->rebuildCollections();
        }
    } else {
        _ui->collectionWidget->hide();
    }

    _player = player;
    connect(_player,SIGNAL(trackFinished()),this,SLOT(updatePlayerTrack()));
    connect(_player,SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this,SLOT(playerStatusChanged(Phonon::State,Phonon::State)));
    connect(_player,SIGNAL(repeatModeChanged(Player::RepeatMode)),
            this,SLOT(repeatModeChanged(Player::RepeatMode)));
    connect(_player,SIGNAL(randomModeChanged(bool)),
            this,SLOT(randomModeChanged(bool)));
    connect(_player,SIGNAL(trackPositionChanged(qint64)),
            this,SLOT(playerPosChanged(qint64)));

    _ui->seekSlider->setMediaObject(_player->mediaObject());
    _ui->volumeSlider->setAudioOutput(_player->audioOutput());

    // Load last playlist
    if (_settings->value("Preferences/RestoreSession").toBool()) {
        _taskManager->addFileToPlaylist(QString(_CONFIGDIR).append("/last.m3u"));
        _player->setRandomMode(_settings->value("LastSession/RandomMode",false).toBool());
        _player->setRepeatMode(Player::RepeatMode(_settings->value("LastSession/RepeatMode",0).toInt()));
    }

    QList<QVariant> playlistColumnsStates(_settings->value("Window/PlaylistColumnsStates", QList<QVariant>()).toList());
    QList<QVariant> playlistColumnsWidths(_settings->value("Window/PlaylistColumnsWidths", QList<QVariant>()).toList());

    for (int i = 0; i < playlistColumnsStates.count()-1; i++) {
        if (playlistColumnsStates.at(i).toBool()) {
            _ui->playlistBrowser->showColumn(i);
            _ui->playlistBrowser->setColumnWidth(i, playlistColumnsWidths.at(i).toInt());
            _ui->menuVisible_columns->actions().at(i)->setChecked(true);
        } else {
            _ui->playlistBrowser->hideColumn(i);
            _ui->menuVisible_columns->actions().at(i)->setChecked(false);
        }
    }



    connect(_ui->loadFileButton,SIGNAL(clicked()),_ui->actionAdd_file,SLOT(trigger()));
    connect(_ui->loadFolderButton,SIGNAL(clicked()),_ui->actionAdd_folder,SLOT(trigger()));
    connect(_ui->clearPlaylistButton,SIGNAL(clicked()),_ui->actionClear_playlist,SLOT(trigger()));
    connect(_ui->previousTrackButton,SIGNAL(clicked()),_ui->actionPrevious_track,SLOT(trigger()));
    connect(_ui->playPauseButton,SIGNAL(clicked()),_ui->actionPlay_pause,SLOT(trigger()));
    connect(_ui->stopButton,SIGNAL(clicked()),_ui->actionStop,SLOT(trigger()));
    connect(_ui->nextTrackButton,SIGNAL(clicked()),_ui->actionNext_track,SLOT(trigger()));


    connect(_ui->playlistSearchEdit,SIGNAL(textChanged(QString)),_playlistProxyModel,SLOT(setFilterRegExp(QString)));

    // Connect individual PlaylistBrowser columns' visibility state with QActions in ui->menuVisible_columns
    _playlistVisibleColumnContextMenuMapper = new QSignalMapper(this);
    connect(_ui->actionFilename,SIGNAL(toggled(bool)),_playlistVisibleColumnContextMenuMapper,SLOT(map()));
    _playlistVisibleColumnContextMenuMapper->setMapping(_ui->actionFilename,0);
    connect(_ui->actionTrack,SIGNAL(toggled(bool)),_playlistVisibleColumnContextMenuMapper,SLOT(map()));
    _playlistVisibleColumnContextMenuMapper->setMapping(_ui->actionTrack,1);
    connect(_ui->actionInterpret,SIGNAL(toggled(bool)),_playlistVisibleColumnContextMenuMapper,SLOT(map()));
    _playlistVisibleColumnContextMenuMapper->setMapping(_ui->actionInterpret,2);
    connect(_ui->actionTrackname,SIGNAL(toggled(bool)),_playlistVisibleColumnContextMenuMapper,SLOT(map()));
    _playlistVisibleColumnContextMenuMapper->setMapping(_ui->actionTrackname,3);
    connect(_ui->actionAlbum,SIGNAL(toggled(bool)),_playlistVisibleColumnContextMenuMapper,SLOT(map()));
    _playlistVisibleColumnContextMenuMapper->setMapping(_ui->actionAlbum,4);
    connect(_ui->actionGenre,SIGNAL(toggled(bool)),_playlistVisibleColumnContextMenuMapper,SLOT(map()));
    _playlistVisibleColumnContextMenuMapper->setMapping(_ui->actionGenre,5);
    connect(_ui->actionYear,SIGNAL(toggled(bool)),_playlistVisibleColumnContextMenuMapper,SLOT(map()));
    _playlistVisibleColumnContextMenuMapper->setMapping(_ui->actionYear,6);
    connect(_ui->actionLength,SIGNAL(toggled(bool)),_playlistVisibleColumnContextMenuMapper,SLOT(map()));
    _playlistVisibleColumnContextMenuMapper->setMapping(_ui->actionLength,7);
    connect(_playlistVisibleColumnContextMenuMapper,SIGNAL(mapped(int)),this,SLOT(togglePlaylistColumnVisible(int)));

}


MainWindow::~MainWindow()
{
    _settings->setValue("Window/Geometry", saveGeometry());
    _settings->setValue("Window/State", saveState());
    _settings->setValue("Window/ViewsSplit",_ui->viewsSplitter->saveState());
    QList<QVariant> playlistColumnsStates;
    QList<QVariant> playlistColumnsWidths;
    for (int i = 0; i < _ui->playlistBrowser->model()->columnCount(QModelIndex())-1; i++) {
        // Don't store "isColumnHidden" but "isColumnVisible"
        playlistColumnsStates.append(!_ui->playlistBrowser->isColumnHidden(i));
        playlistColumnsWidths.append(_ui->playlistBrowser->columnWidth(i));
    }
    _settings->setValue("Window/PlaylistColumnsStates", playlistColumnsStates);
    _settings->setValue("Window/PlaylistColumnsWidths", playlistColumnsWidths);

    _settings->setValue("LastSession/RepeatMode", int(_player->repeatMode()));
    _settings->setValue("LastSession/RandomMode", _player->randomMode());

    // Save current playlist to file
    _taskManager->savePlaylistToFile(QString(_CONFIGDIR).append("/last.m3u"));

    qDebug() << "Waiting for taskManager to finish...";
    delete _taskManager;

    delete _playlistProxyModel;
    delete _playlistModel;

    delete _collectionProxyModel;
    delete _collectionModel;

    delete _ui;
}

void MainWindow::createMenus()
{
    _randomPlaybackGroup = new QActionGroup(this);
    _randomPlaybackGroup->addAction(_ui->actionRandom_ON);
    _randomPlaybackGroup->addAction(_ui->actionRandom_OFF);
    _ui->actionRandom_OFF->setChecked(true);

    _repeatPlaybackGroup = new QActionGroup(this);
    _repeatPlaybackGroup->addAction(_ui->actionRepeat_OFF);
    _repeatPlaybackGroup->addAction(_ui->actionRepeat_playlist);
    _repeatPlaybackGroup->addAction(_ui->actionRepeat_track);
    _ui->actionRepeat_OFF->setChecked(true);


    _trayIconMenu = new QMenu(this);
    _trayIconMenu->addAction(_ui->actionShow_Hide);
    _trayIconMenu->addSeparator();
    _trayIconMenu->addAction(_ui->actionPrevious_track);
    _trayIconMenu->addAction(_ui->actionPlay_pause);
    _trayIconMenu->addAction(_ui->actionStop);
    _trayIconMenu->addAction(_ui->actionNext_track);
    _trayIconMenu->addSeparator();
    _trayIconMenu->addMenu(_ui->menuRepeat);
    _trayIconMenu->addMenu(_ui->menuRandom);
    _trayIconMenu->addSeparator();
    _trayIconMenu->addAction(_ui->actionQuit_TepSonic);

    _collectionsPopupMenu = new QMenu(this);
    _collectionsPopupMenu->addAction(tr("Expand all"),_ui->collectionBrowser,SLOT(expandAll()));
    _collectionsPopupMenu->addAction(tr("Collapse all"),_ui->collectionBrowser,SLOT(collapseAll()));
    _collectionsPopupMenu->addSeparator();
    _collectionsPopupMenu->addAction(tr("Delete file from disk"),this,SLOT(removeFileFromDisk()));
    _ui->collectionBrowser->setContextMenuPolicy(Qt::CustomContextMenu);

}

void MainWindow::bindShortcuts()
{
    _settings->beginGroup("Shortcuts");
    QxtGlobalShortcut *sc1 = new QxtGlobalShortcut(this);
    sc1->setShortcut(QKeySequence::fromString(_settings->value("PlayPause","Meta+P").toString()));
    connect(sc1,SIGNAL(activated()),this,SLOT(on_actionPlay_pause_triggered()));
    QxtGlobalShortcut *sc2 = new QxtGlobalShortcut(this);
    sc2->setShortcut(QKeySequence::fromString(_settings->value("Stop","Meta+S").toString()));
    connect(sc2,SIGNAL(activated()),this,SLOT(on_actionStop_triggered()));
    QxtGlobalShortcut *sc3 = new QxtGlobalShortcut(this);
    sc3->setShortcut(QKeySequence::fromString(_settings->value("PrevTrack","Meta+P").toString()));
    connect(sc3,SIGNAL(activated()),this,SLOT(on_actionPrevious_track_triggered()));
    QxtGlobalShortcut *sc4 = new QxtGlobalShortcut(this);
    sc4->setShortcut(QKeySequence::fromString(_settings->value("NextTrack","Meta+N").toString()));
    connect(sc4,SIGNAL(activated()),this,SLOT(on_actionNext_track_triggered()));
    QxtGlobalShortcut *sc5 = new QxtGlobalShortcut(this);
    sc5->setShortcut(QKeySequence::fromString(_settings->value("ShowHideWin","Meta+H").toString()));
    connect(sc5,SIGNAL(activated()),this,SLOT(on_actionShow_Hide_triggered()));
    _settings->endGroup();
}

/* Show "About Tepsonic" dialog */
void MainWindow::on_actionAbout_TepSonic_triggered()
{
    QMessageBox aboutDlg;

    QStringList developers;
    developers << "Dan Vrátil";
    QStringList artwork;
    artwork << "Matěj Zvěřina"
    << "Michael Ruml";

    QString str = "<h1>"+QApplication::applicationName()+"</h1>"
                  +tr("Version %1").arg(QApplication::applicationVersion())+
                  "<p>This program is free software; you can redistribute it and/or modify it under the terms of "
                  "the GNU General Public License as published by the Free Software Foundation; either version "
                  "2 of the License, or (at your option) any later version.</p>"
                  "<h2>"+tr("Developers")+":</h2>"
                  "<p>"+developers.join(", ")+"</p>"
                  "<h2>"+tr("Artwork")+":</h2>"
                  "<p>"+artwork.join(", ")+"</p>"
                  "<p>&copy; 2009 - 2010 <a href=\"mailto:vratil@progdansoft.com\">Dan Vrátil</a></p>";
    aboutDlg.about(this,tr("About TepSonic"),str.toAscii());
}



void MainWindow::on_actionReport_a_bug_triggered()
{
    QDesktopServices::openUrl(QUrl("http://bugs.tepsonic.org", QUrl::TolerantMode));
}



void MainWindow::trayClicked(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        if (this->isHidden()) {
            this->show();
        } else {
            this->hide();
        }
    }
}



void MainWindow::closeEvent(QCloseEvent* event)
{
    // if tray is visible hide the windows and ignore the event
    if (_trayIcon->isVisible() && (_canClose == false)) {
        this->hide();
        event->ignore();
    }
}

void MainWindow::on_actionAdd_file_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                            tr("Select file"),
			    QDir::homePath(),
			    tr("Supported files") + " (" + SupportedFormats::getExtensionList().join(" ") +
				");;" + tr("Playlists") + " (*.m3u);;" + tr("All files") + " (*.*)");
    _taskManager->addFilesToPlaylist(fileNames);
}


void MainWindow::on_actionSettings_triggered()
{
    // Show preferences dialog
    PreferencesDialog *prefDlg = new PreferencesDialog(this);
    connect(prefDlg,SIGNAL(rebuildCollections()),_taskManager,SLOT(rebuildCollections()));
    connect(prefDlg,SIGNAL(accepted()),this,SIGNAL(settingsAccepted()));
    connect(prefDlg,SIGNAL(accepted()),this,SLOT(preferencesAccepted()));
    prefDlg->exec();

}

void MainWindow::on_actionAdd_folder_triggered()
{
    //open folder dialog
    QString dirName = QFileDialog::getExistingDirectory(this,
                      tr("Add directory"),
                      QString(),
                      QFileDialog::ShowDirsOnly);
    _taskManager->addFileToPlaylist(dirName);
}

void MainWindow::updatePlayerTrack()
{
    if (_player->repeatMode()==Player::RepeatTrack) {
        _player->setTrack(_player->currentSource().fileName());
        _player->play();
    } else {
        on_actionNext_track_triggered();
    }
}

void MainWindow::on_playlistBrowser_doubleClicked(QModelIndex index)
{
    QModelIndex mappedIndex = _playlistProxyModel->mapToSource(index);
    // Play item on row double click
    _playlistModel->setCurrentItem(_playlistModel->index(mappedIndex.row(),0,QModelIndex()));
    setTrack(mappedIndex.row());
}

void MainWindow::playerStatusChanged(Phonon::State newState, Phonon::State oldState)
{
    Q_UNUSED(oldState);

    Player::MetaData metadata = _player->currentMetaData();

    QString playing;
    if (metadata.title.isEmpty()) {
        playing = QFileInfo(metadata.filename).fileName();
    } else {
        playing = metadata.title;
    }
    if (!metadata.artist.isEmpty()) {
        playing = metadata.artist+" - "+playing;
    }

    switch (newState) {
    case Phonon::PlayingState:
        _ui->playPauseButton->setIcon(QIcon(":/icons/pause"));
        _ui->actionPlay_pause->setIcon(QIcon(":/icons/pause"));
        _ui->stopButton->setEnabled(true);
        _ui->actionStop->setEnabled(true);
        _ui->trackTitleLabel->setText(playing);
        _ui->playbackTimeLabel->setText("00:00:00");
        _trayIcon->setToolTip(tr("Playing: ")+playing);
        break;
    case Phonon::PausedState:
        _ui->playPauseButton->setIcon(QIcon(":/icons/start"));
        _ui->actionPlay_pause->setIcon(QIcon(":/icons/start"));
        _ui->stopButton->setEnabled(true);
        _ui->actionStop->setEnabled(true);
        _ui->trackTitleLabel->setText(tr("%1 [paused]").arg(_ui->trackTitleLabel->text()));
        _trayIcon->setToolTip(tr("%1 [paused]").arg(_trayIcon->toolTip()));
        break;
    case Phonon::StoppedState:
        _ui->playPauseButton->setIcon(QIcon(":/icons/start"));
        _ui->actionPlay_pause->setIcon(QIcon(":/icons/start"));
        _ui->stopButton->setEnabled(false);
        _ui->actionStop->setEnabled(false);
        _ui->trackTitleLabel->setText(tr("Player is stopped"));
        _ui->playbackTimeLabel->setText(tr("Stopped"));
        _trayIcon->setToolTip(tr("Player is stopped"));
        break;
    case Phonon::ErrorState:
        _ui->statusBar->showMessage(_player->errorString(),5000);
        break;
    case Phonon::LoadingState:
        //todo
        break;
    case Phonon::BufferingState:
        //todo
        break;
    }
}

void MainWindow::on_actionPrevious_track_triggered()
{
    // We must remap the current item to get it's real position in current filter
    QModelIndex currentItem = _playlistProxyModel->mapFromSource(_playlistModel->currentItem());
    if ((currentItem.row() > 0) && (_playlistModel->rowCount(QModelIndex()) > 0)) {

        _playlistModel->setCurrentItem(_playlistModel->previousItem());

        setTrack(_playlistModel->currentItem().row());
    }
}

void MainWindow::on_actionPlay_pause_triggered()
{
    if (_player->playerState() == Phonon::PlayingState) {
        _player->pause();
    } else {
        /* When the source is empty there are some files in playlist, select the
           first row and load it as current source */
        if ((_player->currentSource().fileName().isEmpty()) &&
                (_playlistModel->rowCount() > 0)) {
            _playlistModel->setCurrentItem(_playlistModel->index(0,0,QModelIndex()));
            setTrack(0);
        }
        _player->play();
    }
}


void MainWindow::on_actionNext_track_triggered()
{
    QModelIndex currentItem = _playlistModel->currentItem();

    if (_player->randomMode()) {
        int row = rand() % _playlistModel->rowCount(QModelIndex());
        _playlistModel->setCurrentItem(_playlistModel->index(row,0,QModelIndex()));
    } else {
        if (_playlistModel->nextItem().isValid()) {
            _playlistModel->setCurrentItem(_playlistModel->nextItem());
        } else {
            if (_player->repeatMode() == Player::RepeatAll) {
                _playlistModel->setCurrentItem(_playlistModel->index(0,0,QModelIndex()));
            } else {
                return;
            }
        }
    }

    setTrack(_playlistModel->currentItem().row());
}

void MainWindow::addPlaylistItem(const QString &filename)
{
    _taskManager->addFileToPlaylist(filename);
}

void MainWindow::showPlaylistContextMenu(QPoint pos)
{
    _ui->menuVisible_columns->popup(_ui->playlistBrowser->header()->mapToGlobal(pos));
}

void MainWindow::togglePlaylistColumnVisible(int column)
{
    _ui->playlistBrowser->setColumnHidden(column,!_ui->playlistBrowser->isColumnHidden(column));
}

void MainWindow::on_actionSave_playlist_triggered()
{
    QString filename;
    filename = QFileDialog::getSaveFileName(this,
                                            tr("Save playlist to..."),
                                            QString(),
                                            tr("M3U Playlist (*.m3u)"));
    _taskManager->savePlaylistToFile(filename);
}

void MainWindow::playlistLengthChanged(int totalLength, int tracksCount)
{
    QString time = formatTimestamp(totalLength);
    _playlistLengthLabel->setText(tr("%n track(s)","",tracksCount).append(" ("+time+")"));
}

void MainWindow::on_clearPlaylistSearch_clicked()
{
    _ui->playlistSearchEdit->setText("");
}

void MainWindow::on_clearCollectionSearch_clicked()
{
    _ui->colectionSearchEdit->setText("");
}

void MainWindow::showError(QString error)
{
    _ui->statusBar->showMessage(error,5000);
}

void MainWindow::fixCollectionProxyModel()
{
    // Workaround for QTBUG 7585
    // http://bugreports.qt.nokia.com/browse/QTBUG-7585
    /*_collectionProxyModel->invalidate();
    _collectionProxyModel->setFilterRegExp("");*/
}

void MainWindow::repeatModeChanged(Player::RepeatMode newMode)
{
    switch (newMode) {
    case Player::RepeatOff:
        _ui->actionRepeat_OFF->setChecked(true);
        break;
    case Player::RepeatTrack:
        _ui->actionRepeat_track->setChecked(true);
        break;
    case Player::RepeatAll:
        _ui->actionRepeat_playlist->setChecked(true);
        break;
    }
}

void MainWindow::randomModeChanged(bool newMode)
{
    if (newMode == true) {
        _ui->actionRandom_ON->setChecked(true);
    } else {
        _ui->actionRandom_OFF->setChecked(true);
    }
}

void MainWindow::playerPosChanged(qint64 newPos)
{
    _ui->playbackTimeLabel->setText(formatMilliseconds(newPos,true));
}

void MainWindow::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        _ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_collectionBrowser_doubleClicked(QModelIndex index)
{
    QString file;
    //QModelIndex mappedIndex = _collectionProxyModel->mapToSource(index);
    QModelIndex mappedIndex = index;

    file = mappedIndex.sibling(index.row(),1).data().toString();

    if (not file.isEmpty()) {
        _taskManager->addFileToPlaylist(file);
    }
}

void MainWindow::trayIconMouseWheelScrolled(int delta)
{
    if (delta>0) {
        _player->audioOutput()->setMuted(false);
        if (_player->audioOutput()->volume() < 1) {
            _player->audioOutput()->setVolume(_player->audioOutput()->volume()+0.1);
        } else {
            _player->audioOutput()->setVolume(1);
        }
    } else {
        // not a typo
        if (_player->audioOutput()->volume() > 0.01) {
            _player->audioOutput()->setVolume(_player->audioOutput()->volume()-0.1);
        } else {
            _player->audioOutput()->setMuted(true);
        }
    }
}

void MainWindow::preferencesAccepted()
{
    if (_settings->value("Collections/EnableCollections").toBool()) {
        /* When collections were enabled in preferences but were not created during program startup,
           create them now otherwise we can expect that collectionBrowser is already displayed and
           working so no action is needed */
        if (_collectionModel == NULL) {
          setupCollections();
          _taskManager->populateCollections();
        }
    } else {
        destroyCollections();
    }
}

void MainWindow::showCollectionsContextMenu(QPoint pos)
{
    _collectionsPopupMenu->popup(_ui->collectionBrowser->mapToGlobal(pos));
}

void MainWindow::removeFileFromDisk()
{
    qDebug() << _ui->collectionBrowser->mapFromGlobal(_collectionsPopupMenu->pos());
    QModelIndex item = _ui->collectionBrowser->indexAt( _ui->collectionBrowser->mapFromGlobal(_collectionsPopupMenu->pos()));
    QString itemName = item.data().toString();
    QString file = item.sibling(item.row(),1).data().toString();

    QString question;
    if (file.isEmpty()) {
        question = QString(tr("Are you sure you want to remove all content of %1 from hard disk?").arg(itemName));
    } else {
        question = QString(tr("Are you sure you want to remove track %1 (%2) from hard disk?").arg(itemName, file));
    }

    if (QMessageBox::question(this,
                              tr("Confirm removal"),
                              question,
                              QMessageBox::Yes,
                              QMessageBox::No) == QMessageBox::Yes) {


        if (file.isEmpty()) {
            QStringList files = _collectionModel->getItemChildrenTracks(item);
            for (int i = 0; i < files.count(); i++) {
                // Remove file
                QFile::remove(files.at(i));
                // And save only path instead of filepath
                files[i] = QFileInfo(files.at(i)).absolutePath();
            }
            // Remove duplicate paths from list
            files = files.toSet().toList();
            // Rebuild collections in all paths that were affected
            for (int i = 0; i < files.count(); i++) {
                _taskManager->rebuildCollections(files.at(i));
            }
        } else {
            // Remove the file
            QFile::remove(file);
            // Rebuild collections in the file's path
            _taskManager->rebuildCollections(QFileInfo(file).absolutePath());
        }
    }
}

void MainWindow::setupCollections()
{
    // Not translatable
    QStringList headers;
    headers = QStringList() << "title" << "filename" << "data1" << "data2";
    _collectionModel = new CollectionModel(headers,this);

    /*_collectionProxyModel = new CollectionProxyModel(this);
    _collectionProxyModel->setSourceModel(_collectionModel);
    _collectionProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    _collectionProxyModel->setFilterKeyColumn(0);
    _collectionProxyModel->setDynamicSortFilter(true);*/

    //_collectionItemDelegate = new CollectionItemDelegate(this, _collectionProxyModel);
    _collectionItemDelegate = new CollectionItemDelegate(this);

    _ui->collectionBrowser->setModel(_collectionModel);
    _ui->collectionBrowser->setItemDelegate(_collectionItemDelegate);
    _ui->collectionBrowser->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _ui->collectionBrowser->setDragEnabled(true);
    _ui->collectionBrowser->setDropIndicatorShown(true);
    _ui->collectionBrowser->setAlternatingRowColors(true);
    _ui->collectionBrowser->setRootIsDecorated(true);
    // Hide the last three columns that cotain filename and additional data
    _ui->collectionBrowser->hideColumn(1);
    _ui->collectionBrowser->hideColumn(2);
    _ui->collectionBrowser->hideColumn(3);
    // Hide the header
    _ui->collectionBrowser->header()->setHidden(true);

    connect(_ui->collectionBrowser,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showCollectionsContextMenu(QPoint)));
    // Workaround for QTBUG 7585 (http://bugreports.qt.nokia.com/browse/QTBUG-7585)
    // Calling invalidate() before changing filter solves problem with expand icons being displayed incorrectly
    // Affects Qt 4.6.0 to 4.6.3
    connect(_ui->colectionSearchEdit,SIGNAL(textChanged(QString)),_collectionProxyModel,SLOT(invalidate()));
    connect(_ui->colectionSearchEdit,SIGNAL(textChanged(QString)),_collectionProxyModel,SLOT(setFilterRegExp(QString)));

    // Since we disabled the Proxy model, no search inputs are needed
    _ui->label_2->hide();
    _ui->colectionSearchEdit->hide();
    _ui->clearCollectionSearch->hide();

    _ui->collectionWidget->show();
}

void MainWindow::destroyCollections()
{
    _ui->collectionBrowser->setModel(NULL);
    delete _collectionProxyModel;
    _collectionProxyModel = NULL;
    delete _collectionModel;
    _collectionModel = NULL;

    _ui->collectionWidget->hide();
}

void MainWindow::setTrack(int row)
{
    QModelIndex currentItem = _playlistModel->currentItem();
    QModelIndex index = _playlistModel->index(currentItem.row(),0,QModelIndex());
    QString filename = _playlistModel->data(index,0).toString();
    _player->setTrack(filename,true);
}
