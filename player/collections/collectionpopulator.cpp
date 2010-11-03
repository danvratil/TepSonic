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

    m_collectionModel = collectionModel;

}

void CollectionPopulator::run()
{

    if ((*m_collectionModel) == NULL) return;


    (*m_collectionModel)->clear();

    DatabaseManager dbManager("populateCollectionBrowserConnection");
    if (!dbManager.connectToDB())
        return;

    emit clearCollectionModel();


    QModelIndex albumsParent;
    QModelIndex tracksParent;

    // First load Various Artists
    QSqlQuery artistsQuery("SELECT COUNT(album) AS albumsCnt,SUM(totalLength) AS totalLength " \
                           "FROM view_various_artists;",
                           *dbManager.sqlDb());
    artistsQuery.next();
    albumsParent = (*m_collectionModel)->addChild(QModelIndex(),
                                                  tr("Various Artists"),
                                                  QString(),
                                                  tr("%n album(s)", "", artistsQuery.value(0).toInt()),
                                                  formatMilliseconds(artistsQuery.value(1).toInt()*1000));
    QSqlQuery albumsQuery("SELECT view_various_artists.album AS id,albums.album,view_various_artists.tracksCnt," \
                          "view_various_artists.totalLength FROM view_various_artists " \
                          "LEFT JOIN albums ON view_various_artists.album=albums.id " \
                          "ORDER BY albums.album ASC;",
                          *dbManager.sqlDb());
    while (albumsQuery.next()) {
        tracksParent = (*m_collectionModel)->addChild(albumsParent,
                                                      albumsQuery.value(1).toString(),
                                                      QString(),
                                                      tr("%n tracks(s)", "", albumsQuery.value(2).toInt()),
                                                      formatMilliseconds(albumsQuery.value(3).toInt()*1000));

        QSqlQuery tracksQuery("SELECT trackname,filename,genre,length,interpret FROM view_tracks WHERE albumID="+albumsQuery.value(0).toString()+" ORDER BY track ASC;",
                              *dbManager.sqlDb());
        while (tracksQuery.next()) {
            (*m_collectionModel)->addChild(tracksParent,
                                           tracksQuery.value(4).toString()+" - "+tracksQuery.value(0).toString(),
                                           tracksQuery.value(1).toString(),
                                           tracksQuery.value(2).toString(),
                                           formatMilliseconds(tracksQuery.value(3).toInt()*1000));
        }
    }


    artistsQuery.exec("SELECT id,interpret,albumsCnt,totalLength FROM interprets " \
                      "WHERE id NOT IN (SELECT interpret FROM tracks " \
                      "    WHERE album IN(SELECT album FROM view_various_artists)) "\
                      "ORDER BY interpret ASC");
    while (artistsQuery.next()) {
        albumsParent = (*m_collectionModel)->addChild(QModelIndex(),
                                                      artistsQuery.value(1).toString(),
                                                      QString(),
                                                      tr("%n album(s)", "", artistsQuery.value(2).toInt()),
                                                      formatMilliseconds(artistsQuery.value(3).toInt()*1000));
        QSqlQuery albumsQuery("SELECT id,album,tracksCnt,totalLength FROM albums " \
                              "WHERE id IN (SELECT album FROM tracks WHERE interpret="+artistsQuery.value(0).toString()+") AND " \
                              "id NOT IN (SELECT album FROM view_various_artists) " \
                              "ORDER BY album ASC;",
                              *dbManager.sqlDb());
        while (albumsQuery.next()) {
            tracksParent = (*m_collectionModel)->addChild(albumsParent,
                                                          albumsQuery.value(1).toString(),
                                                          QString(),
                                                          tr("%n track(s)", "", albumsQuery.value(2).toInt()),
                                                          formatMilliseconds(albumsQuery.value(3).toInt()*1000, true));

            QSqlQuery tracksQuery("SELECT trackname,filename,genre,length FROM view_tracks WHERE albumID="+albumsQuery.value(0).toString()+" AND interpretID="+artistsQuery.value(0).toString()+" ORDER BY track ASC;",
                                  *dbManager.sqlDb());
            while (tracksQuery.next()) {
                (*m_collectionModel)->addChild(tracksParent,
                                               tracksQuery.value(0).toString(),
                                               tracksQuery.value(1).toString(),
                                               tracksQuery.value(2).toString(),
                                               formatMilliseconds(tracksQuery.value(3).toInt()*1000));
            }
        }
    }

    emit collectionsPopulated();

}
