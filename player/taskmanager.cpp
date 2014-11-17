/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <me@dvratil.cz>
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
#include "collections/collectionbuilder.h"

#include <core/settings.h>

#include <QDir>
#include <QSettings>
#include <QStringList>

TaskManager *TaskManager::s_instance = 0;

TaskManager* TaskManager::instance()
{
    if (s_instance == 0) {
        s_instance = new TaskManager();
    }

    return s_instance;
}

TaskManager::TaskManager():
    m_collectionsThreadPool(new QThreadPool(this))
{
    // Only one collections thread at once. Another thread will be queued until the running thread is done
    m_collectionsThreadPool->setMaxThreadCount(1);
}

void TaskManager::destroy()
{
    delete s_instance;
    s_instance = 0;
}

TaskManager::~TaskManager()
{
    m_collectionsThreadPool->waitForDone();
    delete m_collectionsThreadPool;
}

void TaskManager::rebuildCollections(const QString &folder)
{
    QStringList dirs;
    if (folder.isEmpty()) {
        dirs = TepSonic::Settings::instance()->collectionsSourcePaths();
    } else {
        const QFileInfo finfo (folder);
        if (finfo.isDir()) {
            dirs << folder;
        } else {
            dirs << finfo.absolutePath();
        }
    }

    if (!dirs.isEmpty()) {
        CollectionBuilder *collectionBuilder = new CollectionBuilder();
        collectionBuilder->rebuildFolder(dirs);
        connect(collectionBuilder, &CollectionBuilder::buildingStarted,
                [=]() { Q_EMIT taskStarted(tr("Rebuilding collections")); });
        connect(collectionBuilder,&CollectionBuilder::buildingFinished,
                this, &TaskManager::collectionsRebuilt);
        connect(collectionBuilder,&CollectionBuilder::buildingFinished,
                this, &TaskManager::taskDone);

        m_collectionsThreadPool->start(collectionBuilder);
    }
}
