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

#include <QMainWindow>
#include <QPointer>
#include <QMenu>

#include <core/player.h>
#include <core/actionmanager.h>

class QLabel;

namespace TepSonic
{
    class PlaylistModel;
    class DatabaseManager;
}

namespace Ui
{
    class MainWindow;
}

class MetadataEditor;

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow();
    ~MainWindow();

    void installPluginsMenus();

    TepSonic::PlaylistModel* playlistModel() const;

  public Q_SLOTS:
    void showError(const QString &error);
    void setupPluginsUIs();
    void toggleWindowVisible();

  Q_SIGNALS:
    void settingsAccepted();

  protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *e);

  private Q_SLOTS:
    void clearCollectionSearch();
    void clearPlaylistSearch();
    void savePlaylist();

    void playPause();

    void playerStatusChanged(Phonon::State newState, Phonon::State oldState);
    void setCurrentTrack(const QModelIndex &index);

    void openSettingsDialog();
    void settingsDialogAccepted();

    void reportBug();
    void aboutTepSonic();

    void showSupportedFormats();
    void updatePlayerTrack();

    void playlistLengthChanged(int totalLength, int tracksCount);
    void playerPosChanged(qint64 newPos);

    void showMetadataEditor();
    void metadataEditorAccepted();

    void setStopTrackClicked();

    void onCollectionViewDoubleClicked(const QModelIndex &index);

  private:
    Ui::MainWindow *m_ui;
    QPointer<MetadataEditor> m_metadataEditor;

    QLabel *m_playlistLengthLabel;

    bool m_canClose;

    void createMenus();

    template<typename T>
    void addAction(QMenu *menu, const QString &name, T *receiver, void (T::*method)(void))
    {
        QAction *action = TepSonic::ActionManager::instance()->action(name);
        connect(action, &QAction::triggered, receiver, method);
        menu->addAction(action);
    }

    template<typename Functor>
    void addAction(QMenu *menu, const QString &name, Functor f)
    {
        QAction *action = TepSonic::ActionManager::instance()->action(name);
        connect(action, &QAction::triggered, f);
        menu->addAction(action);
    }

    void bindShortcuts();
    void bindSignals();

    void setupCollections();
    void destroyCollections();

};

#endif // MAINWINDOW_H
