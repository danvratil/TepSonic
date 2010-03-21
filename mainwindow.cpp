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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "preferencesdialog.h"

#include "abstractplugin.h"
#include "collectionitem.h"

#include <QMessageBox>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QDate>
#include <QDateTime>
#include <QTime>
#include <Phonon/SeekSlider>
#include <Phonon/VolumeSlider>

#include <QDebug>

MainWindow::MainWindow(Player *player)
{
    // Initialize pseudo-random numbers generator
    srand(time(NULL));

    _canClose = false;

    // Create default UI
    _ui = new Ui::MainWindow();
    _ui->setupUi(this);

    _appIcon = new QIcon(":/icons/mainIcon");
    QApplication::setWindowIcon(*_appIcon);

    _randomPlaybackGroup = new QActionGroup(this);
    _randomPlaybackGroup->addAction(_ui->actionRandom_ON);
    _randomPlaybackGroup->addAction(_ui->actionRandom_OFF);
    _ui->actionRandom_OFF->setChecked(true);

    _repeatPlaybackGroup = new QActionGroup(this);
    _repeatPlaybackGroup->addAction(_ui->actionRepeat_OFF);
    _repeatPlaybackGroup->addAction(_ui->actionRepeat_playlist);
    _repeatPlaybackGroup->addAction(_ui->actionRepeat_track);
    _ui->actionRepeat_OFF->setChecked(true);

    _trayIcon = new QSystemTrayIcon(*_appIcon, this);
    connect(_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayClicked(QSystemTrayIcon::ActivationReason)));
    _trayIcon->setVisible(true);

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
    _trayIcon->setContextMenu(_trayIconMenu);

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
    _playlistModel = new PlaylistModel(headers,this);
    connect(_playlistModel,SIGNAL(playlistLengthChanged(int,int)),
            this,SLOT(playlistLengthChanged(int,int)));
    _playlistProxyModel = new PlaylistProxyModel(this);
    _playlistProxyModel->setSourceModel(_playlistModel);

    _ui->playlistBrowser->setModel(_playlistProxyModel);
    _ui->playlistBrowser->setDragEnabled(true);
    _ui->playlistBrowser->setDropIndicatorShown(true);
    _ui->playlistBrowser->viewport()->setAcceptDrops(true);
    _ui->playlistBrowser->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    // Open ui->menuVisible_columns when PlaylistBrowser's header's context menu is requested
    connect(_ui->playlistBrowser->header(),SIGNAL(customContextMenuRequested(QPoint)),
            this,SLOT(showPlaylistContextMenu(QPoint)));
    // Hide the first column (with filename)
    _ui->playlistBrowser->hideColumn(0);
    _selectionModel = _ui->playlistBrowser->selectionModel();

    // Not translatable
    headers = QStringList() << "title" << "filename";
    _collectionModel = new CollectionModel(headers,this);
    _collectionProxyModel = new CollectionProxyModel(this);
    _collectionProxyModel->setSourceModel(_collectionModel);
    _collectionProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    _collectionProxyModel->setFilterKeyColumn(0);
    _collectionProxyModel->setDynamicSortFilter(true);

    _ui->collectionBrowser->setModel(_collectionProxyModel);
    _ui->collectionBrowser->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _ui->collectionBrowser->setDragEnabled(true);
    _ui->collectionBrowser->setDropIndicatorShown(true);
    // Hide the second column that contains real filename
    _ui->collectionBrowser->hideColumn(1);
    // Hide the header
    _ui->collectionBrowser->header()->setHidden(true);

    _collectionsUpdater = new CollectionsUpdater(_collectionModel);
    _collectionBuilder = new CollectionBuilder(_collectionModel);
    connect(_collectionsUpdater,SIGNAL(collectionsChanged()),
            _collectionBuilder,SLOT(start()));

    _playlistManager = new PlaylistManager(_playlistModel);

    _settings = new QSettings(QString(QDir::homePath()).append("/.tepsonic/main.conf"),QSettings::IniFormat,this);
    restoreGeometry(_settings->value("Window/Geometry", saveGeometry()).toByteArray());
    restoreState(_settings->value("Window/State", saveState()).toByteArray());

    if (_settings->value("Collections/EnableCollections",true).toBool()==false) {
        _ui->collectionBrowser->hide();
    } else {
        _collectionBuilder->start();
        if (_settings->value("Collections/AutoRebuildAfterStart",false).toBool()==true) {
            _collectionsUpdater->start();
        }
    }

    // Load last playlist
    _playlistManager->loadFromFile(QDir::homePath().append("/.tepsonic/last.m3u"));

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

    _player = player;
    connect(_player,SIGNAL(trackFinished()),this,SLOT(updatePlayerTrack()));
    connect(_player,SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this,SLOT(playerStatusChanged(Phonon::State,Phonon::State)));

    _ui->seekSlider->setMediaObject(_player->mediaObject());
    _ui->volumeSlider->setAudioOutput(_player->audioOutput());

    connect(_ui->loadFileButton,SIGNAL(clicked()),_ui->actionAdd_file,SLOT(trigger()));
    connect(_ui->loadFolderButton,SIGNAL(clicked()),_ui->actionAdd_folder,SLOT(trigger()));
    connect(_ui->clearPlaylistButton,SIGNAL(clicked()),_ui->actionClear_playlist,SLOT(trigger()));
    connect(_ui->previousTrackButton,SIGNAL(clicked()),_ui->actionPrevious_track,SLOT(trigger()));
    connect(_ui->playPauseButton,SIGNAL(clicked()),_ui->actionPlay_pause,SLOT(trigger()));
    connect(_ui->stopButton,SIGNAL(clicked()),_ui->actionStop,SLOT(trigger()));
    connect(_ui->nextTrackButton,SIGNAL(clicked()),_ui->actionNext_track,SLOT(trigger()));

    connect(_ui->playlistSearchEdit,SIGNAL(textChanged(QString)),_playlistProxyModel,SLOT(setFilterRegExp(QString)));
    connect(_ui->colectionSearchEdit,SIGNAL(textChanged(QString)),_collectionProxyModel,SLOT(setFilterRegExp(QString)));
    connect(_ui->colectionSearchEdit,SIGNAL(textChanged(QString)),_playlistProxyModel,SLOT(invalidate()));

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
    QList<QVariant> playlistColumnsStates;
    QList<QVariant> playlistColumnsWidths;
    for (int i = 0; i < _ui->playlistBrowser->model()->columnCount(QModelIndex())-1; i++) {
        // Don't store "isColumnHidden" but "isColumnVisible"
        playlistColumnsStates.append(!_ui->playlistBrowser->isColumnHidden(i));
        playlistColumnsWidths.append(_ui->playlistBrowser->columnWidth(i));
    }
    _settings->setValue("Window/PlaylistColumnsStates", playlistColumnsStates);
    _settings->setValue("Window/PlaylistColumnsWidths", playlistColumnsWidths);

    if (_collectionBuilder->isRunning()) {
        qDebug() << "Waiting for collection builder to finish";
        _collectionBuilder->wait();
    }
    delete _collectionBuilder;

    if (_collectionsUpdater->isRunning()) {
        qDebug() << "Waiting for collections updater to finish";
        _collectionsUpdater->wait();
    }
    delete _collectionsUpdater;

    delete _collectionModel;

    // Save current playlist to file
    _playlistManager->saveToFile(QDir::homePath().append("/.tepsonic/last.m3u"));
    qDebug() << "Waiting for playlistManager to finish...";
    _playlistManager->wait();
    delete _playlistManager;

    delete _playlistProxyModel;
    delete _playlistModel;

    delete _collectionProxyModel;

    delete _ui;
}

 /* Show "About Tepsonic" dialog */
void MainWindow::on_actionAbout_TepSonic_triggered()
{
    QMessageBox aboutDlg;

    QString str = QString(tr("<h1>TepSonic</h1><i>Version %1</i><br>" \
                             "<b>Author:</b> Dan \"ProgDan\" Vrátil<br>" \
                             "<b>Contact:</b> vratil@progdansoft.com<br/>" \
                             "<b>Homepage:</b> www.progdan.homelinux.net<br/><br/>" \
                             "This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.")).arg(QString(QApplication::applicationVersion()));
    aboutDlg.about(this,tr("About TepSonic"),str.toAscii());
}



void MainWindow::on_actionReport_a_bug_triggered()
{
    QDesktopServices::openUrl(QUrl("http://bugzilla.progdan.homelinux.net/index.php?project=5&do=index&switch=1", QUrl::TolerantMode));
}



void MainWindow::trayClicked(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger) {
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
                                                          "",
                                                          tr("Supported files (*.mp3 *.wav *.ogg *.flac *.m3u);;Playlists (*.m3u);;All files (*.*)"));
    _playlistManager->add(fileNames);
}


void MainWindow::on_actionPreferences_triggered()
{
    // Show preferences dialog
    PreferencesDialog *prefDlg = new PreferencesDialog(this);
    for (int i = 0; i < _pluginsManager->pluginsCount(); i++) {
        if (static_cast<AbstractPlugin*>(_pluginsManager->pluginAt(i)->instance())->hasConfigUI())
            prefDlg->addPlugin(_pluginsManager->pluginAt(i));
    }
    connect(prefDlg,SIGNAL(rebuildCollectionsRequested()),_collectionsUpdater,SLOT(start()));
    prefDlg->exec();

}

void MainWindow::on_actionAdd_folder_triggered()
{
    //open folder dialog
    QString dirName = QFileDialog::getExistingDirectory(this,
                                                        tr("Add directory"),
                                                        QString(),
                                                        QFileDialog::ShowDirsOnly);

    _playlistManager->add(dirName);
}

void MainWindow::updatePlayerTrack()
{
    if (_player->repeatMode()==Player::RepeatTrack) {
        _player->play();
    } else {
        on_actionNext_track_triggered();
    }
}

void MainWindow::on_playlistBrowser_doubleClicked(QModelIndex index)
{
   // Play item on row double click
   QString filename = _playlistModel->index(index.row(),0,QModelIndex()).data().toString();
   _player->setTrack(filename,true);
}

void MainWindow::playerStatusChanged(Phonon::State newState, Phonon::State oldState)
{
    Q_UNUSED(oldState);

    switch (newState) {
        case Phonon::PlayingState:
            _ui->playPauseButton->setIcon(QIcon(":/icons/pause"));
            _ui->actionPlay_pause->setIcon(QIcon(":/icons/pause"));
            _ui->stopButton->setEnabled(true);
            _ui->actionStop->setEnabled(true);
            _ui->trackTitleLabel->setText(_player->currentSource().fileName());
            break;
        case Phonon::PausedState:
            _ui->playPauseButton->setIcon(QIcon(":/icons/start"));
            _ui->actionPlay_pause->setIcon(QIcon(":/icons/start"));
            _ui->stopButton->setEnabled(true);
            _ui->actionStop->setEnabled(true);
            _ui->trackTitleLabel->setText(QString(tr("%1 [paused]")).arg(_ui->trackTitleLabel->text()));
            break;
        case Phonon::StoppedState:
            _ui->playPauseButton->setIcon(QIcon(":/icons/start"));
            _ui->actionPlay_pause->setIcon(QIcon(":/icons/start"));
            _ui->stopButton->setEnabled(false);
            _ui->actionStop->setEnabled(false);
            _ui->trackTitleLabel->setText(tr("Player is stopped"));
            break;
        case Phonon::ErrorState:
            _ui->statusBar->showMessage(_player->errorString(),5000);
            break;
        case Phonon::LoadingState:
            break;
        case Phonon::BufferingState:
            break;
    }
}

void MainWindow::on_actionPrevious_track_triggered()
{
    QModelIndexList selected = _selectionModel->selectedRows(0);
    // Only when there is an item above
    if ((selected.count() > 0) && (selected.at(0).row()>0)) {
        QModelIndex topLeft = _playlistModel->index(selected.at(0).row()-1,0,QModelIndex());
        QModelIndex bottomRight = _playlistModel->index(selected.at(0).row()-1,
                                                        _playlistModel->columnCount(QModelIndex())-1,
                                                        QModelIndex());
        // First cancel all selections
        for(int i=0;i<selected.count();i++) {
            _selectionModel->select(selected.at(i),QItemSelectionModel::Clear);
        }
        // And then select the new row
        _selectionModel->select(QItemSelection(topLeft,bottomRight),
                                QItemSelectionModel::Select);
        _player->setTrack(_selectionModel->selectedRows(0).at(0).data().toString(),true);
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
            QModelIndex topLeft = _playlistModel->index(0,0,QModelIndex());
            QModelIndex bottomRight = _playlistModel->index(0,_playlistModel->columnCount(QModelIndex())-1,QModelIndex());
            _selectionModel->select(QItemSelection(topLeft,bottomRight),
                                    QItemSelectionModel::Select);
            _player->setTrack(_selectionModel->selectedRows(0).at(0).data().toString(),true);

        }
        _player->play();
    }
}


void MainWindow::on_actionNext_track_triggered()
{
    QModelIndexList selected = _selectionModel->selectedRows(0);
    if (selected.count() > 0) {
        // Is Random mode on?
        if (_player->randomMode()) {
            // Choose random row (generator initialized in constructor)
            int row = rand() % _playlistModel->rowCount(QModelIndex());
            QModelIndex topLeft = _playlistModel->index(row,
                                                       0,
                                                       QModelIndex());
            QModelIndex bottomRight = _playlistModel->index(row,
                                                            _playlistModel->columnCount(QModelIndex())-1,
                                                            QModelIndex());
            // Clear current selection(s)
            for (int i=0;i<selected.count();i++) {
                _selectionModel->select(selected.at(i),QItemSelectionModel::Clear);
            }
            // Select the new item
            _selectionModel->select(QItemSelection(topLeft,bottomRight),
                                    QItemSelectionModel::Select);
        } else { // When random mode is off...
            // Check if there is an item below the current
            if (selected.at(0).row() < _playlistModel->rowCount(QModelIndex())-1) {
                QModelIndex topLeft = _playlistModel->index(selected.at(0).row()+1,0,QModelIndex());
                QModelIndex bottomRight = _playlistModel->index(selected.at(0).row()+1,
                                                                 _playlistModel->columnCount(QModelIndex())-1,
                                                               QModelIndex());
                // First clear current selection
                for (int i = 0; i < selected.count(); i++) {
                    _selectionModel->select(selected.at(i),QItemSelectionModel::Clear);
                }
                // Select the next item
                _selectionModel->select(QItemSelection(topLeft,bottomRight),
                                        QItemSelectionModel::Select);
                // If there is now row below, but player is set to "RepeatAll" then jump to the first track
            } else {
                if(_player->repeatMode() == Player::RepeatAll) {
                    QModelIndex topLeft = _playlistModel->index(0,0,QModelIndex());
                    QModelIndex bottomRight = _playlistModel->index(0,
                                                                    _playlistModel->columnCount(QModelIndex())-1,
                                                                    QModelIndex());
                    // First clear current selection
                    for (int i = 0; i < selected.count(); i++) {
                        _selectionModel->select(selected.at(i),QItemSelectionModel::Clear);
                    }
                    // Now select the first item in playlist
                    _selectionModel->select(QItemSelection(topLeft,bottomRight),
                                            QItemSelectionModel::Select);
                    // If there is no row below and we are not in repeat mode, then stop the playback
                } else {
                    return;
                } // if (_player->repeatMode() == Player::RepeatAll)
            } // if (selected.at(0).row() == playlistModel->rowCount(QModelIndex()))
        } // if (_player->randomMode() == false)
        // Now set the selected item as mediasource into Phonon player
        _player->setTrack(_selectionModel->selectedRows(0).at(0).data().toString(),true);
    } // if (selected.count() == 0)

}

void MainWindow::addPlaylistItem(const QString &filename)
{
    _playlistModel->addItem(filename);
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
    _playlistManager->saveToFile(filename);
}

void MainWindow::playlistLengthChanged(int totalLength, int tracksCount)
{
    int days = totalLength / 86400;
    int hours = (totalLength - (days*86400))/ 3600;
    int mins = (totalLength - (days*86400) - (hours*3600))/60;
    int secs = totalLength - (days*86400) - (hours*3600) - (mins*60);

    QString sDays;
    QString sHours;
    QString sMins;
    QString sSecs;

    if (days > 0) {
        sDays = tr("%n day(s)","",hours).append(" ");
    }

    if (hours<10) {
        sHours = QString("0").append(QString::number(hours)).append(":");
    } else {
        sHours = QString::number(hours).append(":");
    }
    if (hours == 0) sHours = QString();
    if (mins<10) {
        sMins = QString("0").append(QString::number(mins));
    } else {
        sMins = QString::number(mins);
    }
    if (secs<10) {
        sSecs = QString("0").append(QString::number(secs));
    } else {
        sSecs = QString::number(secs);
    }

    _playlistLengthLabel->setText(tr("%n track(s)","",tracksCount).append(" (").append(sDays).append(sHours).append(sMins).append(":").append(sSecs).append(")"));
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

#include "moc_mainwindow.cpp"
