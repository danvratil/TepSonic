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

#include <cstdlib>
#include <ctime>

#include "mainwindow.h"
#include "preferencesdialog.h"
#include "infopanel.h"
#include "ui_mainwindow.h"

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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    // Initialize pseudo-random numbers generator
    srand(time(NULL));

    canClose = false;

    // Create default UI
    ui->setupUi(this);

    appIcon = new QIcon(":/icons/mainIcon");
    QApplication::setWindowIcon(*appIcon);

    randomPlaybackGroup = new QActionGroup(this);
    randomPlaybackGroup->addAction(ui->actionRandom_ON);
    randomPlaybackGroup->addAction(ui->actionRandom_OFF);
    ui->actionRandom_OFF->setChecked(true);

    repeatPlaybackGroup = new QActionGroup(this);
    repeatPlaybackGroup->addAction(ui->actionRepeat_OFF);
    repeatPlaybackGroup->addAction(ui->actionRepeat_playlist);
    repeatPlaybackGroup->addAction(ui->actionRepeat_track);
    ui->actionRepeat_OFF->setChecked(true);

    trayIcon = new QSystemTrayIcon(*appIcon, this);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(trayClicked(QSystemTrayIcon::ActivationReason)));
    trayIcon->setVisible(true);

    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(ui->actionShow_Hide);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(ui->actionPrevious_track);
    trayIconMenu->addAction(ui->actionPlay_pause);
    trayIconMenu->addAction(ui->actionStop);
    trayIconMenu->addAction(ui->actionNext_track);
    trayIconMenu->addSeparator();
    trayIconMenu->addMenu(ui->menuRepeat);
    trayIconMenu->addMenu(ui->menuRandom);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(ui->actionQuit_TepSonic);
    trayIcon->setContextMenu(trayIconMenu);

    infoPanel = new InfoPanel(this);
    infoPanel->hide();
    ui->centralWidget->layout()->addWidget(infoPanel);

    playlistLengthLabel = new QLabel(this);
    ui->statusBar->addPermanentWidget(playlistLengthLabel,0);
    playlistLengthLabel->setText(tr("%n track(s)", "", 0).append(" (00:00)"));

    QStringList headers = QStringList()<< tr("Filename")
                                       << tr("Track")
                                       << tr("Interpret")
                                       << tr("Track name")
                                       << tr("Album")
                                       << tr("Genre")
                                       << tr("Year")
                                       << tr("Length");
    playlistModel = new PlaylistModel(headers,this);
    connect(playlistModel,SIGNAL(playlistLengthChanged(int,int)),this,SLOT(playlistLengthChanged(int,int)));
    playlistProxyModel = new PlaylistProxyModel(this);
    playlistProxyModel->setSourceModel(playlistModel);

    ui->playlistBrowser->setModel(playlistProxyModel);
    ui->playlistBrowser->setDragEnabled(true);
    ui->playlistBrowser->setDropIndicatorShown(true);
    ui->playlistBrowser->viewport()->setAcceptDrops(true);
    ui->playlistBrowser->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    // Open ui->menuVisible_columns when PlaylistBrowser's header's context menu is requested
    connect(ui->playlistBrowser->header(),SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showPlaylistContextMenu(QPoint)));
    // Hide the first column (with filename)
    ui->playlistBrowser->hideColumn(0);
    selectionModel = ui->playlistBrowser->selectionModel();

    // Not translatable
    headers = QStringList() << "title" << "filename";
    collectionModel = new CollectionModel(headers,this);
    collectionProxyModel = new CollectionProxyModel(this);
    collectionProxyModel->setSourceModel(collectionModel);
    collectionProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    collectionProxyModel->setFilterKeyColumn(0);
    collectionProxyModel->setDynamicSortFilter(true);

    ui->collectionBrowser->setModel(collectionProxyModel);
    ui->collectionBrowser->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->collectionBrowser->setDragEnabled(true);
    ui->collectionBrowser->setDropIndicatorShown(true);
    // Hide the second column that contains real filename
    ui->collectionBrowser->hideColumn(1);
    // Hide the header
    ui->collectionBrowser->header()->setHidden(true);

    collectionsUpdater = new CollectionsUpdater(collectionModel);
    collectionBuilder = new CollectionBuilder(collectionModel);
    connect(collectionsUpdater,SIGNAL(collectionsChanged()),collectionBuilder,SLOT(start()));

    playlistManager = new PlaylistManager(playlistModel);

    settings = new QSettings(QString(QDir::homePath()).append("/.tepsonic/main.conf"),QSettings::IniFormat,this);
    restoreGeometry(settings->value("Window/Geometry", saveGeometry()).toByteArray());
    restoreState(settings->value("Window/State", saveState()).toByteArray());

    if (settings->value("Collections/EnableCollections",true).toBool()==false) {
        ui->collectionBrowser->hide();
    } else {
        collectionBuilder->start();
        if (settings->value("Collections/AutoRebuildAfterStart",false).toBool()==true) {
            collectionsUpdater->start();
        }
    }

    // Load last playlist
    playlistManager->loadFromFile(QDir::homePath().append("/.tepsonic/last.m3u"));

    QList<QVariant> playlistColumnsStates(settings->value("Window/PlaylistColumnsStates", QList<QVariant>()).toList());
    QList<QVariant> playlistColumnsWidths(settings->value("Window/PlaylistColumnsWidths", QList<QVariant>()).toList());

    for (int i = 0; i < playlistColumnsStates.count()-1; i++) {
        if (playlistColumnsStates.at(i).toBool()) {
            ui->playlistBrowser->showColumn(i);
            ui->playlistBrowser->setColumnWidth(i, playlistColumnsWidths.at(i).toInt());
            ui->menuVisible_columns->actions().at(i)->setChecked(true);
        } else {
            ui->playlistBrowser->hideColumn(i);
            ui->menuVisible_columns->actions().at(i)->setChecked(false);
        }
    }

    ui->actionLabel->hide();
    ui->actionProgress->hide();

    player = new Player();
    connect(player->phononPlayer,SIGNAL(finished()),this,SLOT(updatePlayerTrack()));
    connect(player->phononPlayer,SIGNAL(stateChanged(Phonon::State,Phonon::State)),this,SLOT(playerStatusChanged(Phonon::State,Phonon::State)));

    ui->seekSlider->setMediaObject(player->phononPlayer);
    ui->volumeSlider->setAudioOutput(player->audioOutput);

    connect(ui->loadFileButton,SIGNAL(clicked()),ui->actionAdd_file,SLOT(trigger()));
    connect(ui->loadFolderButton,SIGNAL(clicked()),ui->actionAdd_folder,SLOT(trigger()));
    connect(ui->clearPlaylistButton,SIGNAL(clicked()),ui->actionClear_playlist,SLOT(trigger()));
    connect(ui->previousTrackButton,SIGNAL(clicked()),ui->actionPrevious_track,SLOT(trigger()));
    connect(ui->playPauseButton,SIGNAL(clicked()),ui->actionPlay_pause,SLOT(trigger()));
    connect(ui->stopButton,SIGNAL(clicked()),ui->actionStop,SLOT(trigger()));
    connect(ui->nextTrackButton,SIGNAL(clicked()),ui->actionNext_track,SLOT(trigger()));

    connect(ui->playlistSearchEdit,SIGNAL(textChanged(QString)),playlistProxyModel,SLOT(setFilterRegExp(QString)));
    connect(ui->colectionSearchEdit,SIGNAL(textChanged(QString)),collectionProxyModel,SLOT(setFilterRegExp(QString)));

    // Connect individual PlaylistBrowser columns' visibility state with QActions in ui->menuVisible_columns
    playlistVisibleColumnContextMenuMapper = new QSignalMapper(this);
    connect(ui->actionFilename,SIGNAL(toggled(bool)),playlistVisibleColumnContextMenuMapper,SLOT(map()));
    playlistVisibleColumnContextMenuMapper->setMapping(ui->actionFilename,0);
    connect(ui->actionTrack,SIGNAL(toggled(bool)),playlistVisibleColumnContextMenuMapper,SLOT(map()));
    playlistVisibleColumnContextMenuMapper->setMapping(ui->actionTrack,1);
    connect(ui->actionInterpret,SIGNAL(toggled(bool)),playlistVisibleColumnContextMenuMapper,SLOT(map()));
    playlistVisibleColumnContextMenuMapper->setMapping(ui->actionInterpret,2);
    connect(ui->actionTrackname,SIGNAL(toggled(bool)),playlistVisibleColumnContextMenuMapper,SLOT(map()));
    playlistVisibleColumnContextMenuMapper->setMapping(ui->actionTrackname,3);
    connect(ui->actionAlbum,SIGNAL(toggled(bool)),playlistVisibleColumnContextMenuMapper,SLOT(map()));
    playlistVisibleColumnContextMenuMapper->setMapping(ui->actionAlbum,4);
    connect(ui->actionGenre,SIGNAL(toggled(bool)),playlistVisibleColumnContextMenuMapper,SLOT(map()));
    playlistVisibleColumnContextMenuMapper->setMapping(ui->actionGenre,5);
    connect(ui->actionYear,SIGNAL(toggled(bool)),playlistVisibleColumnContextMenuMapper,SLOT(map()));
    playlistVisibleColumnContextMenuMapper->setMapping(ui->actionYear,6);
    connect(ui->actionLength,SIGNAL(toggled(bool)),playlistVisibleColumnContextMenuMapper,SLOT(map()));
    playlistVisibleColumnContextMenuMapper->setMapping(ui->actionLength,7);
    connect(playlistVisibleColumnContextMenuMapper,SIGNAL(mapped(int)),this,SLOT(togglePlaylistColumnVisible(int)));

}


MainWindow::~MainWindow()
{
    settings->setValue("Window/Geometry", saveGeometry());
    settings->setValue("Window/State", saveState());
    QList<QVariant> playlistColumnsStates;
    QList<QVariant> playlistColumnsWidths;
    for (int i = 0; i < ui->playlistBrowser->model()->columnCount(QModelIndex())-1; i++) {
        // Don't store "isColumnHidden" but "isColumnVisible"
        playlistColumnsStates.append(!ui->playlistBrowser->isColumnHidden(i));
        playlistColumnsWidths.append(ui->playlistBrowser->columnWidth(i));
    }
    settings->setValue("Window/PlaylistColumnsStates", playlistColumnsStates);
    settings->setValue("Window/PlaylistColumnsWidths", playlistColumnsWidths);

    if (collectionBuilder->isRunning()) {
        qDebug() << "Waiting for collection builder to finish";
        collectionBuilder->wait();
    }
    delete collectionBuilder;

    if (collectionsUpdater->isRunning()) {
        qDebug() << "Waiting for collections updater to finish";
        collectionsUpdater->wait();
    }
    delete collectionsUpdater;

    delete collectionModel;

    // Save current playlist to file
    playlistManager->saveToFile(QDir::homePath().append("/.tepsonic/last.m3u"));
    qDebug() << "Waiting for playlistManager to finish...";
    playlistManager->wait();
    delete playlistManager;

    delete playlistProxyModel;
    delete playlistModel;

    delete collectionProxyModel;

    delete ui;
}



 /* Show "About Qt" dialog */
void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this,tr("About Qt"));
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
    if (trayIcon->isVisible() && (canClose == false)) {
        this->hide();
        event->ignore();
    }
}



void MainWindow::on_actionQuit_TepSonic_triggered()
{
    canClose = true;
    this->close();
}



void MainWindow::on_actionShow_Hide_triggered()
{
    trayClicked(QSystemTrayIcon::Trigger);
}



void MainWindow::on_actionAdd_file_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                                          tr("Select file"),
                                                          "",
                                                          tr("Supported files (*.mp3 *.wav *.ogg *.flac *.m3u);;Playlists (*.m3u);;All files (*.*)"));
    playlistManager->add(fileNames);
}


void MainWindow::on_actionPreferences_triggered()
{
    // Show preferences dialog
    PreferencesDialog *prefDlg = new PreferencesDialog(settings,this);
    connect(prefDlg,SIGNAL(rebuildCollectionsRequested()),collectionsUpdater,SLOT(start()));
    prefDlg->exec();

}

void MainWindow::on_actionClear_playlist_triggered()
{
    playlistModel->removeRows(0,playlistModel->rowCount(QModelIndex()),QModelIndex());
}

void MainWindow::on_actionAdd_folder_triggered()
{
    //open folder dialog
    QString dirName = QFileDialog::getExistingDirectory(this,
                                                        tr("Add directory"),
                                                        QString(),
                                                        QFileDialog::ShowDirsOnly);

    playlistManager->add(dirName);
}

void MainWindow::updateActionBar(int progress, QString actionTitle)
{
    if (ui->actionLabel->isHidden()) {
        ui->actionLabel->show();
    }
    if (ui->actionProgress->isHidden()) {
        ui->actionProgress->show();
    }

    if (not actionTitle.isEmpty()) {
        ui->actionLabel->setText(actionTitle);
    }

    ui->actionProgress->setValue(progress);
}

void MainWindow::updatePlayerTrack()
{
    if (player->repeatMode()==Player::RepeatTrack) {
        player->phononPlayer->play();
    } else {
        on_actionNext_track_triggered();
    }
}

void MainWindow::on_playlistBrowser_doubleClicked(QModelIndex index)
{
   // Play item on row double click
   QString filename = playlistModel->index(index.row(),0,QModelIndex()).data().toString();
   player->setTrack(filename,true);
}

void MainWindow::on_actionRandom_ON_triggered()
{
    player->setRandomMode(true);
}

void MainWindow::on_actionRandom_OFF_triggered()
{
    player->setRandomMode(false);
}

void MainWindow::on_actionRepeat_track_triggered()
{
    player->setRepeatMode(Player::RepeatTrack);
}

void MainWindow::on_actionRepeat_OFF_triggered()
{
    player->setRepeatMode(Player::RepeatOff);
}

void MainWindow::on_actionRepeat_playlist_triggered()
{
    player->setRepeatMode(Player::RepeatAll);
}

void MainWindow::playerStatusChanged(Phonon::State newState, Phonon::State oldState)
{
    Q_UNUSED(oldState);

    switch (newState) {
        case Phonon::PlayingState:
            ui->playPauseButton->setIcon(QIcon(":/icons/pause"));
            ui->actionPlay_pause->setIcon(QIcon(":/icons/pause"));
            ui->stopButton->setEnabled(true);
            ui->actionStop->setEnabled(true);
            ui->trackTitleLabel->setText(player->phononPlayer->currentSource().fileName());
            break;
        case Phonon::PausedState:
            ui->playPauseButton->setIcon(QIcon(":/icons/start"));
            ui->actionPlay_pause->setIcon(QIcon(":/icons/start"));
            ui->stopButton->setEnabled(true);
            ui->actionStop->setEnabled(true);
            ui->trackTitleLabel->setText(QString(tr("%1 [paused]")).arg(ui->trackTitleLabel->text()));
            break;
        case Phonon::StoppedState:
            ui->playPauseButton->setIcon(QIcon(":/icons/start"));
            ui->actionPlay_pause->setIcon(QIcon(":/icons/start"));
            ui->stopButton->setEnabled(false);
            ui->actionStop->setEnabled(false);
            ui->trackTitleLabel->setText(tr("Player is stopped"));
            break;
        case Phonon::ErrorState:
            infoPanel->setMode(InfoPanel::ErrorMode);
            infoPanel->setMessage(player->phononPlayer->errorString());
            infoPanel->show();
            break;
        case Phonon::LoadingState:
            break;
        case Phonon::BufferingState:
            break;
    }
}

void MainWindow::on_actionPrevious_track_triggered()
{
    QModelIndexList selected = selectionModel->selectedRows(0);
    // Only when there is an item above
    if ((selected.count() > 0) && (selected.at(0).row()>0)) {
        QModelIndex topLeft = playlistModel->index(selected.at(0).row()-1,0,QModelIndex());
        QModelIndex bottomRight = playlistModel->index(selected.at(0).row()-1,
                                                       playlistModel->columnCount(QModelIndex())-1,
                                                       QModelIndex());
        // First cancel all selections
        for(int i=0;i<selected.count();i++) {
            selectionModel->select(selected.at(i),QItemSelectionModel::Clear);
        }
        // And then select the new row
        selectionModel->select(QItemSelection(topLeft,bottomRight),
                               QItemSelectionModel::Select);
        player->setTrack(selectionModel->selectedRows(0).at(0).data().toString(),true);
    }
}

void MainWindow::on_actionPlay_pause_triggered()
{
    if (player->phononPlayer->state() == Phonon::PlayingState) {
        player->phononPlayer->pause();
    } else {
        /* When the source is empty there are some files in playlist, select the
           first row and load it as current source */
        if ((player->phononPlayer->currentSource().fileName().isEmpty()) &&
            (playlistModel->rowCount() > 0)) {
            QModelIndex topLeft = playlistModel->index(0,0,QModelIndex());
            QModelIndex bottomRight = playlistModel->index(0,playlistModel->columnCount(QModelIndex())-1,QModelIndex());
            selectionModel->select(QItemSelection(topLeft,bottomRight),
                                   QItemSelectionModel::Select);
            player->setTrack(selectionModel->selectedRows(0).at(0).data().toString(),true);

        }
        player->phononPlayer->play();
    }
}

void MainWindow::on_actionStop_triggered()
{
    player->phononPlayer->stop();
    // Empty source
    player->phononPlayer->setCurrentSource(Phonon::MediaSource());
}

void MainWindow::on_actionNext_track_triggered()
{
    QModelIndexList selected = selectionModel->selectedRows(0);
    if (selected.count() > 0) {
        // Is Random mode on?
        if (player->randomMode()) {
            // Choose random row (generator initialized in constructor)
            int row = rand() % playlistModel->rowCount(QModelIndex());
            QModelIndex topLeft = playlistModel->index(row,
                                                       0,
                                                       QModelIndex());
            QModelIndex bottomRight = playlistModel->index(row,
                                                           playlistModel->columnCount(QModelIndex())-1,
                                                           QModelIndex());
            // Clear current selection(s)
            for (int i=0;i<selected.count();i++) {
                selectionModel->select(selected.at(i),QItemSelectionModel::Clear);
            }
            // Select the new item
            selectionModel->select(QItemSelection(topLeft,bottomRight),
                                   QItemSelectionModel::Select);
        } else { // When random mode is off...
            // Check if there is an item below the current
            if (selected.at(0).row()<playlistModel->rowCount(QModelIndex())-1) {
                QModelIndex topLeft = playlistModel->index(selected.at(0).row()+1,0,QModelIndex());
                QModelIndex bottomRight = playlistModel->index(selected.at(0).row()+1,
                                                               playlistModel->columnCount(QModelIndex())-1,
                                                               QModelIndex());
                // First clear current selection
                for (int i=0;i<selected.count();i++) {
                    selectionModel->select(selected.at(i),QItemSelectionModel::Clear);
                }
                // Select the next item
                selectionModel->select(QItemSelection(topLeft,bottomRight),
                                       QItemSelectionModel::Select);
                // If there is now row below, but player is set to "RepeatAll" then jump to the first track
            } else {
                if(player->repeatMode() == Player::RepeatAll) {
                    QModelIndex topLeft = playlistModel->index(0,0,QModelIndex());
                    QModelIndex bottomRight = playlistModel->index(0,
                                                                   playlistModel->columnCount(QModelIndex())-1,
                                                                   QModelIndex());
                    // First clear current selection
                    for (int i=0;i<selected.count();i++) {
                        selectionModel->select(selected.at(i),QItemSelectionModel::Clear);
                    }
                    // Now select the first item in playlist
                    selectionModel->select(QItemSelection(topLeft,bottomRight),
                                           QItemSelectionModel::Select);
                    // If there is no row below and we are not in repeat mode, then stop the playback
                } else {
                    return;
                } // if (player->repeatMode() == Player::RepeatAll)
            } // if (selected.at(0).row() == playlistModel->rowCount(QModelIndex()))
        } // if (player->randomMode() == false)
        // Now set the selected item as mediasource into Phonon player
        player->setTrack(selectionModel->selectedRows(0).at(0).data().toString(),true);
    } // if (selected.count() == 0)

}

void MainWindow::addPlaylistItem(QString filename)
{
    playlistModel->addItem(filename);
}

void MainWindow::showPlaylistContextMenu(QPoint pos)
{
    ui->menuVisible_columns->popup(ui->playlistBrowser->header()->mapToGlobal(pos));
}

void MainWindow::togglePlaylistColumnVisible(int column)
{
    ui->playlistBrowser->setColumnHidden(column,!ui->playlistBrowser->isColumnHidden(column));
}

void MainWindow::on_actionSave_playlist_triggered()
{
    QString filename;
    filename = QFileDialog::getSaveFileName(this,
                                            tr("Save playlist to..."),
                                            QString(),
                                            tr("M3U Playlist (*.m3u)"));
    playlistManager->saveToFile(filename);
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

    playlistLengthLabel->setText(tr("%n track(s)","",tracksCount).append(" (").append(sDays).append(sHours).append(sMins).append(":").append(sSecs).append(")"));
}

void MainWindow::on_clearPlaylistSearch_clicked()
{
    ui->playlistSearchEdit->setText("");
}

void MainWindow::on_cleatCollectionSearch_clicked()
{
    ui->colectionSearchEdit->setText("");
}
