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
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QHash>
#include <QHashIterator>

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
            emit buildingStarted();

            // This does not take any effect on Linux but on Windows it will not freeze the whole system :-)
            setPriority(QThread::LowPriority);

            DatabaseManager dbManager("collectionsUpdateConnection");
            if (dbManager.connectToDB()) {

                QSqlDatabase sqlConn = QSqlDatabase::database("collectionsUpdateConnection");

                qDebug() << "Starting collections update...";

                QFileInfo fileInfo;
                // List of files in database
                QHash<QString,uint> dbFiles;
                QStringList toBeUpdated;
                QStringList toBeRemoved;
                QStringList filters;

                bool anythingUpdated = false;

                {   // Populeate dbFiles map by _ALL_ tracks from db
                    QSqlQuery query("SELECT filename,mtime FROM Tracks;",sqlConn);
                    while (query.next()) {
                        dbFiles.insert(query.value(0).toString(),query.value(1).toUInt());
                    }
                }

                filters << "*.mp3" << "*.mp4" << "*.wav" << "*.flac";
                int filesProcessed = 0;

                do {
                    QDir dirlist(_folders.takeFirst());
                    dirlist.setNameFilters(filters);

                    QDirIterator dirIterator(dirlist,QDirIterator::Subdirectories);
                    while (dirIterator.hasNext()) {
                        fileInfo = dirIterator.fileInfo();
                        if (fileInfo.isFile()) {
                            // If the file is not in database OR mtime are different then the file will be updated
                            if ((!dbFiles.contains(fileInfo.filePath().toUtf8())) ||
                                (dbFiles[fileInfo.filePath().toUtf8()]!=fileInfo.lastModified().toTime_t())) {
                                    toBeUpdated << fileInfo.filePath();
                            }
                            if (!dbFiles.contains(fileInfo.filePath().toUtf8())) {
                                toBeUpdated << fileInfo.filePath();
                            }
                            dbFiles.remove(fileInfo.filePath().toUtf8());
                        }
                        dirIterator.next();
                        filesProcessed++;
                    }
                } while (_folders.size()>0);
                // Find all pairs with value > 0 and put the list of filenames to toBeRemoved stringlist
                QHashIterator<QString, uint> i(dbFiles);
                while (i.hasNext()) {
                    i.next();
                    //toBeRemoved << sqlConn.driver()->escapeIdentifier(i.key().toUtf8(),QSqlDriver::);
                    toBeRemoved << i.key().toUtf8();
                 }
                dbFiles.clear();

                QSqlQuery removeQuery(sqlConn);
                removeQuery.prepare("DELETE FROM Tracks WHERE `filename` IN (:wherefield);");
                removeQuery.bindValue(":wherefield",toBeRemoved);
                removeQuery.execBatch();

                QSqlQuery updateQuery(sqlConn);
                updateQuery.prepare("REPLACE INTO Tracks (filename,trackNo,artist,album,title,mtime)"\
                                    "VALUES (?, ?, ?, ?, ?, ?)");
                QStringList filenames;
                QStringList artists;
                QStringList albums;
                QStringList titles;
                QStringList trackNos;
                QStringList mtimes;
                foreach (QString filename, toBeUpdated) {
                    QFileInfo fileInfo(filename);
                    TagLib::FileRef f(filename.toUtf8().constData());
                    trackNos <<  QString::number(f.tag()->track());
                    filenames << fileInfo.filePath().toUtf8();
                    QString artist = f.tag()->artist().toCString(true);
                    QString album = f.tag()->album().toCString(true);
                    QString title = f.tag()->title().toCString(true);
                    if (artist.isEmpty()) artist = tr("Unkown artist");
                    if (album.isEmpty()) album = tr("Unknown album");
                    if (title.isEmpty()) title = fileInfo.baseName();
                    mtimes << QString::number(fileInfo.lastModified().toTime_t());
                    artists << artist;
                    albums << album;
                    titles << title;
                }
                updateQuery.bindValue(0,filenames);
                updateQuery.bindValue(1,trackNos);
                updateQuery.bindValue(2,artists);
                updateQuery.bindValue(3,albums);
                updateQuery.bindValue(4,titles);
                updateQuery.bindValue(5,mtimes);

                updateQuery.execBatch();

                if ((!toBeRemoved.empty()) || (!toBeUpdated.empty())) {
                    emit(collectionChanged());
                }

            } // if (dbManager.connectToDB())

        }

        // We don't want to lock the thread when the thread is allowed to close
        if (!_canClose)
            emit buildingFinished();
            _lock.wait(&_mutex);

    } while (!_canClose);

}

void CollectionBuilder::rebuildFolder(QStringList folder)
{
    _mutex.lock();
        _folders.append(folder);
    _mutex.unlock();
    _lock.wakeAll();
}
