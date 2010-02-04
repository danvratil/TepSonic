/*
 * TEPSONIC
 * Copyright 2009 Dan Vratil <vratil@progdansoft.com>
 * Copyright 2009 Petr Los <petr_los@centrum.cz>
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

#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>
#include <QMutex>
#include <QThread>
#include <QStringList>

class PlaylistModel;

class PlaylistManager : public QThread
{

    Q_OBJECT
    Q_ENUMS(PlaylistManagerAction)
    public:
        enum PlaylistManagerAction { LoadPlaylist, SavePlaylist };
        PlaylistManager(PlaylistModel *model);
        ~PlaylistManager();
        void run();

    public slots:
        void add(QString filename);
        void add(QStringList filenames);
        void saveToFile(QString filename);
        void loadFromFile(QString filename);

    private:
        void p_addFile(QString filename);
        void p_addFolder(QString folder);
        void p_loadPlaylistFile(QString filename);
        void p_savePlaylist(QString filename);

        PlaylistModel *_model;
        QStringList _files;
        QMutex _mutex;
        PlaylistManagerAction _action;

};

#endif // PLAYLISTMANAGER_H
