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
#include "tracksiterator.h"

#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <Phonon/SeekSlider>
#include <Phonon/VolumeSlider>


#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    // Initialize pseudo-random numbers generator
    srand(time(NULL));

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

    QStringList headers = QStringList()<< tr("Filename")
                                       << tr("Track")
                                       << tr("Interpret")
                                       << tr("Track name")
                                       << tr("Album")
                                       << tr("Genre")
                                       << tr("Year")
                                       << tr("Length");
    playlistModel = new PlaylistModel(headers,QString(),this);
    ui->playlistBrowser->setModel(playlistModel);
    // hide the first column (with filename)
    ui->playlistBrowser->hideColumn(0);

    selectionModel = ui->playlistBrowser->selectionModel();

    canClose = false;

    restoreGeometry(settings.value("Window/Geometry", saveGeometry()).toByteArray());

    if (settings.value("Collections/EnableCollections",true).toBool()==false)
        ui->collectionBrowser->hide();

    ui->actionLabel->hide();
    ui->actionProgress->hide();

    player = new Player();
    connect(player->phononPlayer,SIGNAL(finished()),this,SLOT(updatePlayerTrack()));
    connect(player->phononPlayer,SIGNAL(stateChanged(Phonon::State,Phonon::State)),this,SLOT(on_playerStatusChanged(Phonon::State,Phonon::State)));

    ui->seekSlider->setMediaObject(player->phononPlayer);
    ui->volumeSlider->setAudioOutput(player->audioOutput);

    connect(ui->loadFileButton,SIGNAL(clicked()),ui->actionAdd_file,SLOT(trigger()));
    connect(ui->loadFolderButton,SIGNAL(clicked()),ui->actionAdd_folder,SLOT(trigger()));
    connect(ui->clearPlaylistButton,SIGNAL(clicked()),ui->actionClear_playlist,SLOT(trigger()));
    connect(ui->previousTrackButton,SIGNAL(clicked()),ui->actionPrevious_track,SLOT(trigger()));
    connect(ui->playPauseButton,SIGNAL(clicked()),ui->actionPlay_pause,SLOT(trigger()));
    connect(ui->stopButton,SIGNAL(clicked()),ui->actionStop,SLOT(trigger()));
    connect(ui->nextTrackButton,SIGNAL(clicked()),ui->actionNext_track,SLOT(trigger()));

}



MainWindow::~MainWindow()
{
    settings.setValue("Window/Geometry", saveGeometry());

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
                                                          tr("Supported audio files (*.mp3 *.wav *.ogg *.flac);;All files (*.*)"));
    for (int file = 0; file < fileNames.count(); file++) {
        ui->playlistBrowser->addItem(fileNames.at(file));
    }
}


void MainWindow::on_actionPreferences_triggered()
{
    // Show preferences dialog
    PreferencesDialog *prefDlg = new PreferencesDialog(settings,this);
    prefDlg->exec();

}

void MainWindow::on_actionClear_playlist_triggered()
{
    ui->playlistBrowser->removeItems(0,playlistModel->rowCount());
}

void MainWindow::on_actionAdd_folder_triggered()
{
    //open folder dialog
    QString dirName = QFileDialog::getExistingDirectory(this,
                                                        tr("Add directory"),
                                                        QString(),
                                                        QFileDialog::ShowDirsOnly);

    // Fire file iterator thread
    tracksIterator = new TracksIterator(dirName);
    connect(tracksIterator,SIGNAL(fileFound(QString)),this,SLOT(addPlaylistItem(QString)));
    tracksIterator->start();
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
   QVariant row = playlistModel->data(index.sibling(index.row(),0), Qt::DisplayRole);
   player->setTrack(row.toString(),true);
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

void MainWindow::on_playerStatusChanged(Phonon::State newState, Phonon::State oldState)
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
            int row = rand() % playlistModel->rowCount(QModelIndex())-1;
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
                } // if (player->repeatMode() == Player::RepeatAll)
            } // if (selected.at(0).row() == playlistModel->rowCount(QModelIndex()))
        } // if (player->randomMode() == false)
        // Now set the selected item as mediasource into Phonon player
        player->setTrack(selectionModel->selectedRows(0).at(0).data().toString(),true);
    } // if (selected.count() == 0)

}

void MainWindow::addPlaylistItem(QString filename)
{
    ui->playlistBrowser->addItem(filename);
}
