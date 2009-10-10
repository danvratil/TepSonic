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

#include "mainwindow.h"
#include "preferencesdialog.h"
#include "ui_mainwindow.h"

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
    trayIconMenu->addAction(ui->actionQuit_TepSonic);
    trayIcon->setContextMenu(trayIconMenu);

    QStringList headers = QStringList()<<"Filename"<<"Track"<<"Interpret"<<"Track name"<<"Album"<<"Genre"<<"Year"<<"Length";
    playlistModel = new PlaylistModel(headers,QString(),this);
    ui->playlistBrowser->setModel(playlistModel);
    // hide the first column (with filename)
    ui->playlistBrowser->hideColumn(0);

    canClose = false;

    restoreGeometry(settings.value("Window/Geometry", saveGeometry()).toByteArray());

    if (settings.value("Collections/EnableCollections",true).toBool()==false)
        ui->collectionBrowser->hide();

    ui->actionLabel->hide();
    ui->actionProgress->hide();

    player = new Player();
    connect(player->phononPlayer,SIGNAL(finished()),this,SLOT(updatePlayerTrack()));

    seekSlider = new Phonon::SeekSlider(this);
    volumeSlider = new Phonon::VolumeSlider(this);
    seekSlider->setMediaObject(player->phononPlayer);
    volumeSlider->setAudioOutput(player->audioOutput);
    volumeSlider->setFixedWidth(150);
    ui->playerControlLayout->addWidget(seekSlider);
    ui->playerControlLayout->addWidget(volumeSlider);

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
    QString str = QString("<h1>TepSonic</h1><i>Version ").append(QString(QApplication::applicationVersion())).append("</i><br><b>Author:</b> Dan \"ProgDan\" Vratil<br><b>Contact:</b> vratil@progdansoft.com<br/><b>Homepage:</b> www.progdan.homelinux.net<br/><br/>This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.");
    aboutDlg.about(this,tr("About TepSonic"),tr(str.toAscii()));
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
    QStringList fileNames = QFileDialog::getOpenFileNames(this,tr("Select file"),"",
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
    QString dirName = QFileDialog::getExistingDirectory(this,tr("Add directory"), QString(), QFileDialog::ShowDirsOnly);

    /* Here we should fire a thread that will create (recursively of course) list of files in dir (and subdirs) and put it
       into the playlist */
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
    // Select next track in playlist (random OR following)
}

void MainWindow::on_playlistBrowser_doubleClicked(QModelIndex index)
{
   // Play item on double click
   QVariant row = playlistModel->data(index.sibling(index.row(),0), Qt::DisplayRole);
   player->setTrack(row.toString(),true);
}
