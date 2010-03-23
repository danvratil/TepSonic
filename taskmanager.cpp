/*
 * TEPSONIC
 * Copyright 2010 Dan Vratil <vratil@progdansoft.com>
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

#include "taskmanager.h"

#include "playlistmodel.h"
#include "collectionmodel.h"
#include "playlistpopulator.h"
#include "playlistwriter.h"
#include "collectionpopulator.h"
#include "collectionbuilder.h"

#include <QDir>
#include <QSettings>

TaskManager::TaskManager(PlaylistModel *playlistModel, CollectionModel *collectionModel)
{
    _playlistModel = playlistModel;
    _collectionModel = collectionModel;

    _playlistPopulator = new PlaylistPopulator(_playlistModel);
    _playlistWriter = new PlaylistWriter(_playlistModel);
    _collectionPopulator = new CollectionPopulator(_collectionModel);
    _collectionBuilder = new CollectionBuilder(_collectionModel);

    connect(_collectionBuilder,SIGNAL(collectionChanged()),
            _collectionPopulator,SLOT(populate()));
}

TaskManager::~TaskManager()
{
    delete _playlistPopulator;
    delete _playlistWriter;
    delete _collectionPopulator;
    delete _collectionBuilder;
}

void TaskManager::addFilesToPlaylist(const QStringList &files)
{
    _playlistPopulator->addFiles(files);
}

void TaskManager::addFileToPlaylist(const QString &filename)
{
    _playlistPopulator->addFile(filename);
}

void TaskManager::savePlaylistToFile(const QString &filename)
{
    _playlistWriter->saveToFile(filename);
}

void TaskManager::populateCollections()
{
    _collectionPopulator->populate();
}

void TaskManager::rebuildCollections(const QString &folder)
{
    QString dirs = folder;
    if (folder.isEmpty()) {
        QSettings settings(QDir::homePath().append("/.tepsonic/main.conf"));
        dirs = settings.value("SourcePath",QString()).toString();
    }

    if (!dirs.isEmpty()) {
        _collectionBuilder->rebuildFolder(dirs);
    }
}
