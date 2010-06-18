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


#include "collectionpopulator.h"
#include "collectionmodel.h"
#include "tools.h"
#include "databasemanager.h"

#include <QSqlField>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QVariant>

CollectionPopulator::CollectionPopulator(CollectionModel **collectionModel)
{

    _collectionModel = collectionModel;
    _canClose = false;

    start();
}

CollectionPopulator::~CollectionPopulator()
{
    // Let the thread end
    if (isRunning()) {
        _canClose = true;
        _lock.wakeAll();
    }
    // Wait for it to quit
    wait();
}

void CollectionPopulator::run()
{

    do {

        if ((*_collectionModel) != NULL) {

            DatabaseManager dbManager("populateCollectionBrowserConnection");
            if (!dbManager.connectToDB()) {
                return;
            }
            _mutex.lock();
            (*_collectionModel)->clear();

            QModelIndex albumsParent;
            QModelIndex tracksParent;
            {
                QSqlQuery artistsQuery("SELECT id,interpret,albumsCnt,totalLength FROM interprets ORDER BY interpret ASC",
                                       *dbManager.sqlDb());
                while (artistsQuery.next()) {
                    albumsParent = (*_collectionModel)->addChild(QModelIndex(),
                                                                 artistsQuery.value(1).toString(),
                                                                 QString(),
                                                                 tr("%n album(s)","",artistsQuery.value(2).toInt()),
                                                                 formatMilliseconds(artistsQuery.value(3).toInt()*1000));
                    QSqlQuery albumsQuery("SELECT id,album,tracksCnt,totalLength FROM albums WHERE id IN (SELECT album FROM tracks WHERE interpret="+artistsQuery.value(0).toString()+") ORDER BY album ASC;",
                                          *dbManager.sqlDb());
                    while (albumsQuery.next()) {
                        tracksParent = (*_collectionModel)->addChild(albumsParent,
                                                                     albumsQuery.value(1).toString(),
                                                                     QString(),
                                                                     tr("%n track(s)","",albumsQuery.value(2).toInt()),
                                                                     formatMilliseconds(albumsQuery.value(3).toInt()*1000, true));

                        QSqlQuery tracksQuery("SELECT trackname,filename,genre,length FROM view_tracks WHERE albumID="+albumsQuery.value(0).toString()+" AND interpretID="+artistsQuery.value(0).toString()+" ORDER BY track ASC;",
                                              *dbManager.sqlDb());
                        while (tracksQuery.next()) {
                            (*_collectionModel)->addChild(tracksParent,
                                                          tracksQuery.value(0).toString(),
                                                          tracksQuery.value(1).toString(),
                                                          tracksQuery.value(2).toString(),
                                                          formatMilliseconds(tracksQuery.value(3).toInt()*1000));
                        }
                    }
                }
            }
            _mutex.unlock();

            emit collectionsPopulated();

        }

        // We don't want to lock the thread when the thread is allowed to quit
        if (!_canClose)
            // Wait for next awaking
            _lock.wait(&_mutex);

    } while (!_canClose);

}

void CollectionPopulator::populate()
{

    // Unlock thread for one iteration
    _lock.wakeAll();
}
