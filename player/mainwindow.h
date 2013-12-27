/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <dan@progdan.cz>
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

#include <QMessageBox>
#include <QMainWindow>
#include <QActionGroup>
#include <QSignalMapper>
#include <QPointer>
#include <QLabel>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QItemSelectionModel>
#include <QSystemTrayIcon>


#include "player.h"

class DatabaseManager;
class MetadataEditor;
class TrayIcon;

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
    void setupPluginsUIs();

  Q_SIGNALS:
    void settingsAccepted();

  protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *e);

  private Q_SLOTS:
    void clearCollectionSearch();
    void clearPlaylistSearch();
    void showPlaylistHeaderContextMenu(const QPoint &pos);
    void savePlaylist();

    void playPause();

    void playerStatusChanged(Phonon::State newState, Phonon::State oldState);
    void setCurrentTrack(const QModelIndex &index);

    void openSettingsDialog();
    void settingsDialogAccepted();

    void trayClicked(QSystemTrayIcon::ActivationReason reason);
    void reportBug();
    void aboutTepSonic();

    void showSupportedFormats();
    void updatePlayerTrack();

    void playlistLengthChanged(int totalLength, int tracksCount);
    void repeatModeChanged(Player::RepeatMode newMode);
    void randomModeChanged(bool newMode);
    void playerPosChanged(qint64 newPos);

    void showPlaylistContextMenu(const QPoint &pos);

    void showMetadataEditor();
    void metadataEditorAccepted();

    void setStopTrackClicked();


  private:
    Ui::MainWindow *m_ui;
    QPointer<MetadataEditor> m_metadataEditor;

    QActionGroup *m_randomPlaybackGroup;
    QActionGroup *m_repeatPlaybackGroup;
    TrayIcon *m_trayIcon;

    QMenu *m_trayIconMenu;
    QMenu *m_collectionsPopupMenu;
    QMenu *m_playlistPopupMenu;
    QSignalMapper *m_playlistVisibleColumnContextMenuMapper;

    QIcon *m_appIcon;
    QLabel *m_playlistLengthLabel;

    bool m_canClose;

    void createMenus();
    void bindShortcuts();
    void bindSignals();

    void setupCollections();
    void destroyCollections();

};

#endif // MAINWINDOW_H
