#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QCloseEvent>

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

    canClose = false;

}

MainWindow::~MainWindow()
{
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

void MainWindow::on_actionLoad_file_triggered()
{
    trayClicked(QSystemTrayIcon::Trigger);
}
