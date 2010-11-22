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

TaskManager::TaskManager(PlaylistModel **playlistModel, CollectionModel **collectionModel)
{
    m_playlistModel = playlistModel;
    m_collectionModel = collectionModel;

    m_threadPool = new QThreadPool(this);
    m_collectionsThreadPool = new QThreadPool(this);
    // Only one collections thread at once. Another thread will be queued until the running thread is done
    m_collectionsThreadPool->setMaxThreadCount(1);

}

TaskManager::~TaskManager()
{
    m_threadPool->waitForDone();
    m_collectionsThreadPool->waitForDone();
    delete m_threadPool;
    delete m_collectionsThreadPool;
}

void TaskManager::addFilesToPlaylist(const QStringList &files, int row)
{
    if ((row == 0) && (*m_playlistModel))
        row = (*m_playlistModel)->rowCount(QModelIndex());

    PlaylistPopulator *playlistPopulator = new PlaylistPopulator();
    playlistPopulator->addFiles(files, row);
    connect(playlistPopulator, SIGNAL(insertItemToPlaylist(Player::MetaData,int)),
            this, SIGNAL(insertItemToPlaylist(Player::MetaData,int)),
            Qt::QueuedConnection);
    connect(playlistPopulator, SIGNAL(playlistPopulated()),
            this, SIGNAL(playlistPopulated()));

    m_threadPool->start(playlistPopulator);
}

void TaskManager::addFileToPlaylist(const QString &filename, int row)
{
    if ((row == 0) && (*m_playlistModel))
        row = (*m_playlistModel)->rowCount(QModelIndex());

    PlaylistPopulator *playlistPopulator = new PlaylistPopulator();
    playlistPopulator->addFile(filename, row);
    connect(playlistPopulator, SIGNAL(insertItemToPlaylist(Player::MetaData,int)),
            this, SIGNAL(insertItemToPlaylist(Player::MetaData,int)));
    connect(playlistPopulator, SIGNAL(playlistPopulated()),
            this, SIGNAL(playlistPopulated()));

    m_threadPool->start(playlistPopulator);
}

void TaskManager::savePlaylistToFile(const QString &filename)
{
    PlaylistWriter *playlistWriter = new PlaylistWriter(*m_playlistModel);
    playlistWriter->saveToFile(filename);
    connect(playlistWriter, SIGNAL(playlistSaved()),
            this, SIGNAL(playlistSaved()));

    m_threadPool->start(playlistWriter);
}

void TaskManager::populateCollections()
{
    CollectionPopulator *collectionPopulator = new CollectionPopulator(m_collectionModel);
    connect(collectionPopulator, SIGNAL(collectionsPopulated()),
            this, SIGNAL(collectionsPopulated()));
    connect(collectionPopulator, SIGNAL(clearCollectionModel()),
            this, SIGNAL(clearCollectionModel()));
    connect(collectionPopulator, SIGNAL(addChild(QModelIndex,QString,QString,QString,QString,QModelIndex*)),
            this, SIGNAL(insertItemToCollections(QModelIndex,QString,QString,QString,QString,QModelIndex*)),
            Qt::BlockingQueuedConnection);

    m_collectionsThreadPool->start(collectionPopulator);
}

void TaskManager::rebuildCollections(const QString &folder)
{
    QStringList dirs;
    if (folder.isEmpty()) {
        QSettings settings(QString(_CONFIGDIR).append("/main.conf"),QSettings::IniFormat,this);
        dirs << settings.value("Collections/SourcePaths",QStringList()).toStringList();
    } else {
        dirs << folder;
    }

    if (!dirs.isEmpty()) {

        CollectionBuilder *collectionBuilder = new CollectionBuilder(m_collectionModel);
        collectionBuilder->rebuildFolder(dirs);
        connect(collectionBuilder,SIGNAL(buildingStarted()),
                this,SLOT(collectionsRebuildingStarted()));
        connect(collectionBuilder,SIGNAL(buildingFinished()),
                this,SIGNAL(collectionsRebuilt()));
        connect(collectionBuilder,SIGNAL(buildingFinished()),
                this,SIGNAL(taskDone()));

        m_collectionsThreadPool->start(collectionBuilder);

    }
}

void TaskManager::collectionsRebuildingStarted()
{
    emit taskStarted(tr("Rebuilding collections"));
}
