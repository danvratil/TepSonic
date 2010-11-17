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
#include <QSettings>
#include <QSignalMapper>
#include <QSortFilterProxyModel>
#include <QItemSelectionModel>
#include <QLabel>
#include <QSystemTrayIcon>
#include <QFileSystemModel>


// These classes are used in inline methods
#include "playlist/playlistmodel.h"
#include "player.h"
#include "trayicon.h"


class PlaylistItemDelegate;
class PlaylistProxyModel;
class CollectionModel;
class CollectionProxyModel;
class CollectionItemDelegate;
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
    inline void setPluginsManager(PluginsManager *pluginsManager) {
        m_pluginsManager = pluginsManager;
    }

    //! Returns pointer to PluginsManager
    /*!
      \return Returns pointer to plugins manager
      \sa setPluginsManager()
    */
    PluginsManager* pluginsManager() {
        return m_pluginsManager;
    }

private:
    //! Pointer to main window's UI (generated from mainwindow.ui)
    Ui::MainWindow *m_ui;

    //! Pointer to PlaylistModel
    PlaylistModel *m_playlistModel;

    //! Pointer to PlaylistProxyModel
    PlaylistProxyModel *m_playlistProxyModel;

    //! Pointer to PlaylistItemDelegate
    PlaylistItemDelegate *m_playlistItemDelegate;

    //! Pointer to CollectionModel
    CollectionModel *m_collectionModel;

    //! Pointer to CollectionProxyModel
    CollectionProxyModel *m_collectionProxyModel;

    //! Pointer to CollectionItemDelegate
    CollectionItemDelegate *m_collectionItemDelegate;

    //! Pointer to QFileSystemModel
    QFileSystemModel *m_fileSystemModel;

    //! Pointer to Player
    Player *m_player;

    //! Pointer to PluginsManager
    PluginsManager *m_pluginsManager;

    //! Pointer to TaskManager
    TaskManager *m_taskManager;

    //! Group of QActions that contains Random ON and Random OFF actions
    QActionGroup *m_randomPlaybackGroup;

    //! Group of QActions that contains Repeat Track, Repeat All and Repeat Off actions
    QActionGroup *m_repeatPlaybackGroup;

    //! PlaylistBrowser selection model
    QItemSelectionModel *m_selectionModel;

    //! Application system tray icon
    TrayIcon *m_trayIcon;

    //! Tray icon menu
    QMenu *m_trayIconMenu;

    //! Collections popup menu
    QMenu *m_collectionsPopupMenu;

    //! Playlist popup menu
    QMenu *m_playlistPopupMenu;

    //! Signal mapper that maps items in _ui->playlistVisibleColumn menu to togglePlaylistColumnVisible()
    /*!
      \sa togglePlaylistColumnVisible()
    */
    QSignalMapper *m_playlistVisibleColumnContextMenuMapper;

    //! Application icon
    QIcon *m_appIcon;

    //! Label with length of current playlist (hh:mm:ss format)
    QLabel *m_playlistLengthLabel;

    //! Settings
    QSettings *m_settings;

    //! Decides if the main window will be closed or hidden on closeEvent
    /*!
      Determines if the main window can be closed. Is FALSE by default which causes the main window
      to be only hidden not terminated. If TRUE, the main window will be closed and application
      will be terminated.
      \sa closeEvent()
    */
    bool m_canClose;

    //! Creates additional popupmenus
    /*!
      This method is called from constructor just after _ui->setupUi() is called.
    */
    void createMenus();

    //! Binds global shortcuts to individual events
    /*!
      This method is called from constructor just after menus are created.
    */
    void bindShortcuts();

    //! Binds signals to slots
    void bindSignals();

    //! Create CollectionModel and CollectionProxyModel
    /*!
      This method is called only when collections are enabled in configuration
    */
    void setupCollections();

    //! Destroy CollectionModel and CollectionProxyModel
    /*!
      This method is called when preferences dialog is closed and collections were disabled in it
    */
    void destroyCollections();

    //! Set as current track the track on given row in playlist
    /*!
      \param row location of the new track in playlist
    */
    void setTrack(int row);


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

    //! When language is changed the window is retranslated
    void changeEvent(QEvent *e);

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

    //! When doubleclicked item is a track, move it to the playlist
    void collectionBrowserDoubleClick(QModelIndex index);

    //! Workaround for QTBUG  7585
    /*!
      When filter applied, the expand-icons are not displayed correctly.
      \link http://bugreports.qt.nokia.com/browse/QTBUG-7585
      Calling invalidate() before changing the filter solves the problem.
    */
    void fixCollectionProxyModel();

    //! Clears collectionSearchEdit field and resets collections filter
    void clearCollectionSearch();

    //! Clears playlistSearchEdit field and resets playlist filter
    void clearPlaylistSearch();

    //! Popups playlist header context menu
    /*!
      \param pos XY coordinates of cursor (relatively to MainWindow top-left corner) where the context menu will popup
    */
    void showPlaylistHeaderContextMenu(QPoint pos);

    //! Toggles visibility of given playlist column
    /*!
      \param column number of column that will be hidden or shown
    */
    void togglePlaylistColumnVisibility(int column);

    //! Opens the dialog to select a destination where to save the playlist and then calls PlaylistManager::savePlaylist()
    /*!
      \sa PlaylistManager::savePlaylist()
    */
    void savePlaylist();

    //! Play next track in playlist
    /*!
      When there is no track below the current in playlist and Player::repeatMode() is RepeatOff then the playback
      is stopped. When Player::repeatMode is repeatAll then first track is played. When Player::randomMode() is on
      then a random track from playlist is selected.
    */
    void nextTrack();

    //! Play or pause playback
    void playPause();

    //! Go to previous track.
    /*!
      When there is no track above, nothing happens.
    */
    void previousTrack();

    //! Stop the playback and reset the Player::CurrentSource
    void stopPlayer() {
        m_player->stop();
    }

    //! Set player repeat mode to \p RepeatAll
    void setRepeatModeAll() {
        m_player->setRepeatMode(Player::RepeatAll);
    }

    //! Set player repeat mode to \p RepeatOff (disable repeat)
    void setRepeatModeOff() {
        m_player->setRepeatMode(Player::RepeatOff);
    }

    //! Set player repeat mode to \p RepeatTrack
    void setRepeatModeTrack() {
        m_player->setRepeatMode(Player::RepeatTrack);
    }

    //! Disable player random mode
    void setRandomModeOff() {
        m_player->setRandomMode(false);
    }

    //! Enable player random mode
    void setRandomModeOn() {
        m_player->setRandomMode(true);
    }

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
    void playlistBrowserDoubleClick(QModelIndex index);

    //! Removes all items from playlist
    void clearPlaylist() {
        m_playlistModel->removeRows(0, m_playlistModel->rowCount(QModelIndex()), QModelIndex());
    }

    //! Opens settings dialog
    void openSettingsDialog();

    //! Called when preferences dialog is accepted, applies possible changes
    void settingsDialogAccepted();

    //! toggle visibility of main window
    void showHideWindow() {
        trayClicked(QSystemTrayIcon::Trigger);
    }

    //! Quit TepSonic
    void quitApp() {
        m_canClose = true;
        this->close();
    }

    //! Called when tray icon is clicked and toggles visibility of main window
    /*!
      \sa on_actionShow_Hide_triggered()
    */
    void trayClicked(QSystemTrayIcon::ActivationReason);

    //! Opens external default browser and navigates to TepSonic bugzilla
    void reportBug();

    //! Shows 'About' dialog
    void aboutTepSonic();

    //! Shows 'About Qt' dialog
    void aboutQt() {
        QMessageBox::aboutQt(this,tr("About Qt"));
    }

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

    //! Event that updates volume when scrolling on trayicon
    void trayIconMouseWheelScrolled(int delta);

    //! Open the popup
    void showCollectionsContextMenu(QPoint pos);

    //! Open the playlist popup menu
    void showPlaylistContextMenu(QPoint pos);

    //! Deletes file locally on hardisk and calls for collections rebuild
    void removeFileFromDisk();

signals:
    void settingsAccepted();


};

#endif // MAINWINDOW_H
