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

#include <QObject>
#include <QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QActionGroup>
#include <QtGui/QSystemTrayIcon>
#include <QSettings>
#include <QSignalMapper>
#include <QSortFilterProxyModel>
#include <QItemSelectionModel>
#include <QLabel>

// These two classes are used in inline methods
#include "playlist/playlistmodel.h"
#include "player.h"
//#include "tracksiterator.h"

class PlaylistProxyModel;
class CollectionModel;
class CollectionProxyModel;
class DatabaseManager;
class PluginsManager;
class TaskManager;

namespace Ui
{
    class MainWindow;
}

//! MainWindow is TepSonic's main window
/*!
  MainWindow is subclassed from QMainWindow and provides main user interface for TepSonic.
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(PluginsManager* pluginsManager
               READ pluginsManager
               WRITE setPluginsManager)

public:
    //! Constructor
    /*!
      Initializes main window. Recieves pointer to Player as parameter. Pointer to PluginsManager is set via method
      setPluginsManager() because PluginsManager is created after the MainWindow.
      \param player pointer to Player object
      \sa setPluginsManager()
    */
    MainWindow(Player *player);

    //! Destructor
    ~MainWindow();

    //! Sets pointer to PluginsManager
    /*!
      Gives the main window access to the PluginsManager. At normal circumstances it would be passed in constructor
      but the PluginsManager is created after the main window so it's not possible (without risking invalid reference)
      \param pluginsManager pointer to PluginsManager object
      \sa pluginsManager()
    */
    inline void setPluginsManager(PluginsManager *pluginsManager) { _pluginsManager = pluginsManager; }

    //! Returns pointer to PluginsManager
    /*!
      \return Returns pointer to plugins manager
      \sa setPluginsManager()
    */
    PluginsManager* pluginsManager() { return _pluginsManager; }

private:
    //! Pointer to main window's UI (generated from mainwindow.ui)
    Ui::MainWindow *_ui;

    //! Pointer to PlaylistModel
    PlaylistModel *_playlistModel;

    //! Pointer to PlaylistProxyModel
    PlaylistProxyModel *_playlistProxyModel;

    //! Pointer to CollectionModel
    CollectionModel *_collectionModel;

    //! Pointer to CollectionProxyModel
    CollectionProxyModel *_collectionProxyModel;

    //! Pointer to Player
    Player *_player;

    //! Pointer to PluginsManager
    PluginsManager *_pluginsManager;

    //! Pointer to TaskManager
    TaskManager *_taskManager;

    //! Group of QActions that contains Random ON and Random OFF actions
    QActionGroup *_randomPlaybackGroup;

    //! Group of QActions that contains Repeat Track, Repeat All and Repeat Off actions
    QActionGroup *_repeatPlaybackGroup;

    //! PlaylistBrowser selection model
    QItemSelectionModel *_selectionModel;

    //! Application system tray icon
    QSystemTrayIcon *_trayIcon;

    //! Tray icon menu
    QMenu *_trayIconMenu;

    //! Signal mapper that maps items in _ui->playlistVisibleColumn menu to togglePlaylistColumnVisible()
    /*!
      \sa togglePlaylistColumnVisible()
    */
    QSignalMapper *_playlistVisibleColumnContextMenuMapper;

    //! Application icon
    QIcon *_appIcon;

    //! Label with length of current playlist (hh:mm:ss format)
    QLabel *_playlistLengthLabel;

    //! Settings
    QSettings *_settings;

    //! Decides if the main window will be closed or hidden on closeEvent
    /*!
      Determines if the main window can be closed. Is FALSE by default which causes the main window
      to be only hidden not terminated. If TRUE, the main window will be closed and application
      will be terminated.
      \sa closeEvent()
    */
    bool _canClose;


protected:
    //! Close event handler
    /*!
      Executed when trying to close main window (eg. close application). According
      to state of 'canClose' variable either allows to destroy the main window and close
      the application (TRUE) or just hides main window (recoverable by clicking tray icon)
      and ignores the event (FALSE)
      \param event pointer to QCloseEvent with more informations about the event
    */
    void closeEvent(QCloseEvent *event);

public slots:
    //! Displays \p error message in status bar
    /*!
      \param error message that describes the error
    */
    void showError(QString error);

    //! Add file \p filename to playlist
    /*!
      \param filename name of file to be added
    */
    void addPlaylistItem(const QString &filename);

private slots:


    //! Workaround for QTBUG  7585
    /*!
      When filter applied, the expand-icons are not displayed correctly.
      \link http://bugreports.qt.nokia.com/browse/QTBUG-7585
      Calling invalidate() before changing the filter solves the problem.
    */
    void fixCollectionProxyModel();

    //! Clears collectionSearchEdit field and resets collections filter
    void on_clearCollectionSearch_clicked();

    //! Clears playlistSearchEdit field and resets playlist filter
    void on_clearPlaylistSearch_clicked();

    //! Popups playlist header context menu
    /*!
      \param pos XY coordinates of cursor (relatively to MainWindow top-left corner) where the context menu will popup
    */
    void showPlaylistContextMenu(QPoint pos);

    //! Toggles visibility of given playlist column
    /*!
      \param column number of column that will be hidden or shown
    */
    void togglePlaylistColumnVisible(int column);

    //! Opens the dialog to select a destination where to save the playlist and then calls PlaylistManager::savePlaylist()
    /*!
      \sa PlaylistManager::savePlaylist()
    */
    void on_actionSave_playlist_triggered();

    //! Play next track in playlist
    /*!
      When there is no track below the current in playlist and Player::repeatMode() is RepeatOff then the playback
      is stopped. When Player::repeatMode is repeatAll then first track is played. When Player::randomMode() is on
      then a random track from playlist is selected.
    */
    void on_actionNext_track_triggered();

    //! Play or pause playback
    void on_actionPlay_pause_triggered();

    //! Go to previous track.
    /*!
      When there is no track above, nothing happens.
    */
    void on_actionPrevious_track_triggered();

    //! Stop the playback and reset the Player::CurrentSource
    void on_actionStop_triggered() { _player->stop(); }

    //! Set player repeat mode to \p RepeatAll
    void on_actionRepeat_playlist_triggered() { _player->setRepeatMode(Player::RepeatAll); }

    //! Set player repeat mode to \p RepeatOff (disable repeat)
    void on_actionRepeat_OFF_triggered() { _player->setRepeatMode(Player::RepeatOff); }

    //! Set player repeat mode to \p RepeatTrack
    void on_actionRepeat_track_triggered() { _player->setRepeatMode(Player::RepeatTrack); }

    //! Disable player random mode
    void on_actionRandom_OFF_triggered() { _player->setRandomMode(false); }

    //! Enable player random mode
    void on_actionRandom_ON_triggered() { _player->setRandomMode(true); }

    //! Called when player status is changed
    /*!
      \param newState current player state
      \param oldState previous player state
    */
    void playerStatusChanged(Phonon::State newState, Phonon::State oldState);

    //! Called when a item in playlist is double-clicked.
    /*!
      Passed index points to clicked item. This item is set as current player source and playback is started.
      \sa on_actionPlay_pause_triggered()
    */
    void on_playlistBrowser_doubleClicked(QModelIndex index);

    //! Opens dialog for folder selection and then calls PlaylistManager to populate the playlist
    void on_actionAdd_folder_triggered();

    //! Removes all items from playlist
    void on_actionClear_playlist_triggered() { _playlistModel->removeRows(0,_playlistModel->rowCount(QModelIndex()),QModelIndex()); }

    //! Opens preferences dialog
    void on_actionPreferences_triggered();

    //! Opens dialog for files and then calls PlaylistManager to populate the playlist
    void on_actionAdd_file_triggered();

    //! toggle visibility of main window
    void on_actionShow_Hide_triggered() { trayClicked(QSystemTrayIcon::Trigger); }

    //! Quit TepSonic
    void on_actionQuit_TepSonic_triggered() { _canClose = true; this->close(); }

    //! Called when tray icon is clicked and toggles visibility of main window
    /*!
      \sa on_actionShow_Hide_triggered()
    */
    void trayClicked(QSystemTrayIcon::ActivationReason);

    //! Opens external default browser and navigates to TepSonic bugzilla
    void on_actionReport_a_bug_triggered();

    //! Shows 'About' dialog
    void on_actionAbout_TepSonic_triggered();

    //! Shows 'About Qt' dialog
    void on_actionAbout_Qt_triggered() { QMessageBox::aboutQt(this,tr("About Qt")); }

    //! Called when new track is set in Player.
    void updatePlayerTrack();

    //! Called when items are added/removed from playlist to update display of the length of the playlist
    /*!
      \param totalLength total length of playlist in seconds
      \param tracksCount number of tracks in playlist
    */
    void playlistLengthChanged(int totalLength, int tracksCount);

    //! Called when Player's RepeatMode is changed
    void repeatModeChanged(Player::RepeatMode newMode);

    //! Called when Player's RandomMode is changed
    void randomModeChanged(bool newMode);

    //! Called when position in track is changed
    void playerPosChanged(qint64 newPos);

};

#endif // MAINWINDOW_H
