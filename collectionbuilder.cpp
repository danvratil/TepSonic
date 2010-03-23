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


#include "collectionbuilder.h"
#include "collectionmodel.h"
#include "databasemanager.h"

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>

#include <QDebug>

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QModelIndex>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

CollectionBuilder::CollectionBuilder(CollectionModel *collectionModel)
{
    moveToThread(this);

    _collectionModel = collectionModel;
    _canClose = false;

    start();
}

CollectionBuilder::~CollectionBuilder()
{
    if (isRunning()) {
        _canClose = true;
        _lock.wakeAll();
    }
    wait();
}

void CollectionBuilder::run()
{

    do {

        if (!_folders.isEmpty()) {

            // This does not take any effect on Linux but on Windows it will not freeze the whole system :-)
            setPriority(QThread::LowPriority);

            DatabaseManager dbManager("collectionsUpdateConnection");
            if (!dbManager.connectToDB()) {
                return;
            }
            QSqlDatabase sqlConn = QSqlDatabase::database("collectionsUpdateConnection");

            qDebug() << "Starting collections update...";

            QFileInfo fileInfo;
            // List of files in database
            QMap<QString,uint> dbFiles;
            QStringList toBeUpdated;
            QStringList toBeRemoved;
            QStringList filters;

            bool anythingUpdated = false;

            {   // Populeate dbFiles map by _ALL_ tracks from db
                QSqlQuery query("SELECT filename,mtime FROM Tracks;",sqlConn);
                qDebug() << sqlConn.lastError().text();
                while (query.next()) {
                    dbFiles[query.value(0).toString()] = query.value(1).toUInt();
                }
            }

            filters << "*.mp3" << "*.mp4" << "*.wav" << "*.flac";
            int filesProcessed = 0;

            for (int i = 0; i < _folders.count(); i++) {
                QDir dirlist(_folders.at(i));
                dirlist.setNameFilters(filters);

                QDirIterator dirIterator(dirlist,QDirIterator::Subdirectories);
                while (dirIterator.hasNext()) {
                    fileInfo = dirIterator.fileInfo();
                    if (fileInfo.isFile()) {
                        // If the file is not in database OR mtime are different then the file will be updated
                        if ((!dbFiles.contains(fileInfo.filePath())) ||
                            (dbFiles[fileInfo.filePath()]!=fileInfo.lastModified().toTime_t())) {
                            toBeUpdated << fileInfo.filePath();
                        }
                        /* Now set the mtime to 0 which indicates that we have already checked this file. When checking
                           is done, we can assume that all pairs with value > 0 are files that are stored in DB but are not
                           anymore in the list of files on harddrive and therefor should be removed from DB */
                        dbFiles[fileInfo.filePath()]=0;
                    }
                    dirIterator.next();
                    filesProcessed++;
                }
            }
            // Find all pairs with value > 0 and put the list of filenames to toBeRemoved stringlist
            QMap<QString,uint>::iterator dbFilesIterator = dbFiles.begin();
            while (dbFilesIterator != dbFiles.end()) {
                if (dbFilesIterator.value()>0) {
                    toBeRemoved << dbFilesIterator.key();
                }
                ++dbFilesIterator;
            }
            dbFiles.clear(); // Memory saving...

            // First remove files that are to be removed
            if (toBeRemoved.count() > 0) {
                QSqlQuery query(sqlConn);
                query.prepare("DELETE FORM Tracks WHERE filename IN (?);");
                /*foreach (QString file, toBeRemoved) {
                    query.bindValue(0,file);
                }*/
                query.addBindValue(toBeRemoved);
                query.exec();
                qDebug() << sqlConn.lastError().text();
                if (query.numRowsAffected() > 0) {
                    anythingUpdated = true;
                }
            }
            toBeRemoved.clear(); // Memory saving...

            QStringList values;
            // Next we will prepare set of input data for each file that should be updated
            QRegExp rx("\'");
            if (toBeUpdated.count() > 0) {
                QVariantList filenames, trackNos, artists, albums, titles, mtimes;
                for (int i = 0; i < toBeUpdated.count(); i++) {
                    TagLib::FileRef f(toBeUpdated.at(i).toUtf8().constData());
                    int trackNo = f.tag()->track();
                    QString filename = toBeUpdated.at(i);
                    QString artist = f.tag()->artist().toCString(true);
                    QString album = f.tag()->album().toCString(true);
                    QString title = f.tag()->title().toCString(true);
                    if (artist.isEmpty()) artist = tr("Unkown artist");
                    if (album.isEmpty()) album = tr("Unknown album");
                    if (title.isEmpty()) title = QFileInfo(toBeUpdated.at(i).toUtf8()).baseName();
                    uint mtime = QFileInfo(toBeUpdated.at(i)).lastModified().toTime_t();
                    filenames << filename;
                    trackNos << trackNo;
                    artists << artist;
                    albums << album;
                    titles << title;
                    mtimes << mtime;
                }
                QSqlQuery query(sqlConn);
                query.prepare("REPLACE INTO Tracks (filename,trackNo,artist,album,title,mtime)"\
                              "VALUES (?, ?, ?, ?, ?, ?)");
                query.addBindValue(filenames);
                query.addBindValue(trackNos);
                query.addBindValue(artists);
                query.addBindValue(albums);
                query.addBindValue(titles);
                query.addBindValue(mtimes);
                query.execBatch();
                if (query.numRowsAffected() > 0) {
                    anythingUpdated = true;
                }
                qDebug() << query.lastError().text();

            }
            values.clear();

            qDebug() << "Collections were updated";

            /*if (anythingUpdated) {
                emit(collectionsChanged());
            }*/


        }

        // We don't want to lock the thread when the thread is allowed to close
        if (!_canClose)
            _lock.wait(&_mutex);

    } while (!_canClose);

}

void CollectionBuilder::rebuildFolder(QString folder)
{
    _mutex.lock();
        _folders.append(folder);
    _mutex.unlock();
    _lock.wakeAll();
}
