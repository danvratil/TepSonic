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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QActionGroup>
#include <QtGui/QSystemTrayIcon>
#include <QSettings>
#include <QSignalMapper>
#include <QItemSelectionModel>

#include "playlistmodel.h"
#include "collectionmodel.h"
#include "infopanel.h"
#include "player.h"
#include "tracksiterator.h"
#include "collectionsmanager.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    /**
     * Determines if the main window can be closed. Is FALSE by default which causes the main window
     * to be only hidden not terminated. If TRUE, the main window will be closed and application
     * will be terminated.*/
    bool canClose;

private:
    Ui::MainWindow *ui;

    InfoPanel *infoPanel;

    QActionGroup *randomPlaybackGroup;
    QActionGroup *repeatPlaybackGroup;

    PlaylistModel *playlistModel;
    CollectionModel *collectionModel;

    CollectionsManager *collectionsManager;

    QItemSelectionModel *selectionModel;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    QSignalMapper *playlistVisibleColumnContextMenuMapper;

    QIcon *appIcon;

    QSettings *settings;
    Player *player;

    TracksIterator *tracksIterator;

protected:
    /**
     * Executed when trying to close main window (eg. close application). According
     * to state of 'canClose' variable either allows to destroy the main window and close
     * the application (TRUE) or just hides main window (recoverable by clicking tray icon)
     * and ignores the event (FALSE)
     */
    void closeEvent(QCloseEvent*);

private slots:

    void showPlaylistContextMenu(QPoint pos);
    void togglePlaylistColumnVisible(int column);

    /**
     * Update some player properties
     */
    void on_actionNext_track_triggered();
    void on_actionStop_triggered();
    void on_actionPlay_pause_triggered();
    void on_actionPrevious_track_triggered();
    void on_actionRepeat_playlist_triggered();
    void on_actionRepeat_OFF_triggered();
    void on_actionRepeat_track_triggered();
    void on_actionRandom_OFF_triggered();
    void on_actionRandom_ON_triggered();

    void on_playerStatusChanged(Phonon::State, Phonon::State);

    /**
     * Activate item in playlist
     */
    void on_playlistBrowser_doubleClicked(QModelIndex index);

    /**
     * Add folder to playlist
     */
    void on_actionAdd_folder_triggered();

    /**
     * Remove all items from playlist
     */
    void on_actionClear_playlist_triggered();

    /**
     * Open Preferences dialog
     */
    void on_actionPreferences_triggered();

    /**
     * Open Add file dialog
     */
    void on_actionAdd_file_triggered();

    /**
     * Hide (or show, depends on current status of main window) the main window
     */
    void on_actionShow_Hide_triggered();

    /**
     * Try to quit TepSonic
     */
    void on_actionQuit_TepSonic_triggered();

    /**
     * Tray icon click
     */
    void trayClicked(QSystemTrayIcon::ActivationReason);

    /**
     * Report a bug in TepSonic. Openes default browser and navigates to ProgDan's bugzilla
     */
    void on_actionReport_a_bug_triggered();

    /**
     * Show 'About TepSonic' dialog
     */
    void on_actionAbout_TepSonic_triggered();

    /**
     * Show 'About Qt' dialog (just a tribute to Qt :-)
     */
    void on_actionAbout_Qt_triggered();

    /**
     * Set new track to player
     */
    void updatePlayerTrack();

    /**
     * Add new track to playlist
     */
    void addPlaylistItem(QString);

  public slots:

    /**
     * Update action bar
     */
    void updateActionBar(int progress, QString actionTitle = QString());

};

#endif // MAINWINDOW_H
