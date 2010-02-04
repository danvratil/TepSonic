/* TEPSONIC
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


#ifndef COLLECTIONSMANAGER_H
#define COLLECTIONSMANAGER_H

#include <QObject>
#include <QMutex>
#include <QThread>


class CollectionModel;

class DatabaseManager;

class CollectionsManager : public QThread
{
    Q_OBJECT
    Q_ENUMS(CollectionsManagerActions)
    public:
        enum CollectionsManagerActions { UpdateCollections, UpdateCollectionBrowser };
        CollectionsManager(CollectionModel *model);
        ~CollectionsManager();
        void run();

    public slots:
        void updateCollections();
        void updateCollectionBrowser();

    private:
        void p_updateCollections();
        void p_updateCollectionBrowser();

        QMutex _mutex;

        CollectionModel *_model;
        CollectionsManagerActions _action;
        DatabaseManager *_dbManager;
};

#endif // COLLECTIONSMANAGER_H
