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

#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QActionGroup>
#include <QtCore/QSettings>
#include <QtCore/QSignalMapper>
#include <QtCore/QPointer>
#include <QtGui/QLabel>
#include <QtGui/QKeyEvent>
#include <QtGui/QFileSystemModel>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QItemSelectionModel>


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
class TaskManager;
class BookmarksManager;
class FileSystemModel;
class MetadataEditor;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow();
    ~MainWindow();

    void installPluginsMenus();

  public Q_SLOTS:
    void showError(const QString &error);
    void addPlaylistItem(const QString &filename);
    void setupPluginsUIs();

  Q_SIGNALS:
    void settingsAccepted();

  protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *e);

  private Q_SLOTS:
    void collectionBrowserDoubleClick(const QModelIndex &index);

    void fixCollectionProxyModel();

    void clearCollectionSearch();
    void clearPlaylistSearch();
    void showPlaylistHeaderContextMenu(const QPoint &pos);
    void togglePlaylistColumnVisibility(int column);
    void savePlaylist();

    void nextTrack();
    void playPause();
    void previousTrack();

    void setRepeatModeAll() {
        Player::instance()->setRepeatMode(Player::RepeatAll);
    }

    void setRepeatModeOff() {
        Player::instance()->setRepeatMode(Player::RepeatOff);
    }

    void setRepeatModeTrack() {
        Player::instance()->setRepeatMode(Player::RepeatTrack);
    }

    void setRandomModeOff() {
        Player::instance()->setRandomMode(false);
    }

    void setRandomModeOn() {
        Player::instance()->setRandomMode(true);
    }


    void playerStatusChanged(Phonon::State newState, Phonon::State oldState);
    void setCurrentTrack(const QModelIndex &index);
    void clearPlaylist();

    void openSettingsDialog();
    void settingsDialogAccepted();

    void showHideWindow() {
        trayClicked(QSystemTrayIcon::Trigger);
    }

    void quitApp() {
        m_canClose = true;
        this->close();
    }

    void trayClicked(QSystemTrayIcon::ActivationReason reason);
    void reportBug();
    void aboutTepSonic();
    void aboutQt() {
        QMessageBox::aboutQt(this, tr("About Qt"));
    }

    void showSupportedFormats();
    void updatePlayerTrack();

    void playlistLengthChanged(int totalLength, int tracksCount);
    void repeatModeChanged(Player::RepeatMode newMode);
    void randomModeChanged(bool newMode);
    void playerPosChanged(qint64 newPos);

    void showCollectionsContextMenu(const QPoint &pos);
    void showPlaylistContextMenu(const QPoint &pos);

    void removeFileFromDisk();

    void showMetadataEditor();
    void metadataEditorAccepted();

    void setStopTrackClicked();


  private:
    Ui::MainWindow *m_ui;
    PlaylistModel *m_playlistModel;
    PlaylistProxyModel *m_playlistProxyModel;
    PlaylistItemDelegate *m_playlistItemDelegate;
    CollectionModel *m_collectionModel;
    CollectionProxyModel *m_collectionProxyModel;
    CollectionItemDelegate *m_collectionItemDelegate;
    QFileSystemModel *m_fileSystemModel;
    QPointer<MetadataEditor> m_metadataEditor;
    BookmarksManager *m_bookmarksManager;
    TaskManager *m_taskManager;

    QActionGroup *m_randomPlaybackGroup;
    QActionGroup *m_repeatPlaybackGroup;
    TrayIcon *m_trayIcon;

    QMenu *m_trayIconMenu;
    QMenu *m_collectionsPopupMenu;
    QMenu *m_playlistPopupMenu;
    QSignalMapper *m_playlistVisibleColumnContextMenuMapper;

    QIcon *m_appIcon;
    QLabel *m_playlistLengthLabel;

    QSettings *m_settings;

    bool m_canClose;

    void createMenus();
    void bindShortcuts();
    void bindSignals();

    void setupCollections();
    void destroyCollections();

};

#endif // MAINWINDOW_H
