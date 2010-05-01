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
#include "constants.h"
#include "collections/collectionmodel.h"
#include "collections/collectionpopulator.h"
#include "collections/collectionbuilder.h"
#include "playlist/playlistmodel.h"
#include "playlist/playlistpopulator.h"
#include "playlist/playlistwriter.h"


#include <QDir>
#include <QSettings>
#include <QStringList>

TaskManager::TaskManager(PlaylistModel *playlistModel, CollectionModel *collectionModel)
{
    _playlistModel = playlistModel;
    _collectionModel = collectionModel;

    _playlistPopulator = new PlaylistPopulator(_playlistModel);
    _playlistWriter = new PlaylistWriter(_playlistModel);
    _collectionPopulator = new CollectionPopulator(_collectionModel);
    _collectionBuilder = new CollectionBuilder(_collectionModel);

    connect(_collectionBuilder,SIGNAL(collectionChanged()),
            this,SLOT(populateCollections()));
    connect(_collectionPopulator,SIGNAL(collectionsPopulated()),
            this,SIGNAL(collectionsPopulated()));
    connect(_collectionBuilder,SIGNAL(buildingStarted()),
            this,SLOT(collectionsRebuildingStarted()));
    connect(_collectionBuilder,SIGNAL(buildingFinished()),
            this,SIGNAL(taskDone()));
    connect(_playlistPopulator,SIGNAL(filesAdded()),
            this,SIGNAL(playlistPopulated()));
    connect(_playlistPopulator,SIGNAL(fileAdded()),
            this,SIGNAL(playlistPopulated()));
    connect(_playlistWriter,SIGNAL(playlistSaved()),
            this,SIGNAL(playlistSaved()));
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
    QStringList dirs;
    if (folder.isEmpty()) {
        QSettings settings(QString(_CONFIGDIR).append("/main.conf"),QSettings::IniFormat,this);
        dirs << settings.value("Collections/SourcePaths",QStringList()).toStringList();
    }

    if (!dirs.isEmpty()) {
        _collectionBuilder->rebuildFolder(dirs);
    }
}

void TaskManager::collectionsRebuildingStarted()
{
    emit taskStarted(tr("Rebuilding collections"));
}
