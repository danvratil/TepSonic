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
#include "supportedformats.h"

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
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>
#include <QtSql/QSqlResult>
#include <QHash>
#include <QHashIterator>

CollectionBuilder::CollectionBuilder(CollectionModel **collectionModel)
{

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

            bool collectionsChanged = false;

            // This does not take any effect on Linux but on Windows it will not freeze the whole system :-)
            setPriority(QThread::LowPriority);

            DatabaseManager dbManager("collectionsUpdateConnection");
            if (dbManager.connectToDB()) {

                qDebug() << "Starting collections update...";

                QFileInfo fileInfo;
                // List of files in database
                QHash<QString,uint> dbFiles;
                QStringList toBeUpdated;
                QStringList toBeRemoved;
                QStringList filters = SupportedFormats::getExtensionList();

                {   // Populate dbFiles map by _ALL_ tracks from db
                    QSqlQuery query("SELECT filename, mtime FROM tracks;",*dbManager.sqlDb());
                    while (query.next()) {
                        dbFiles.insert(query.value(0).toString(),query.value(1).toUInt());
                    }
                }

                /* We don't need to do transaction on MySQL.
                   On SQLite it makes it MUCH faster because otherwise SQLite will commit
                   every INSERT individually and that makes ~60 commits per second (depends on the
                   speed of harddrive) meanwhile having everything in one transaction does only one
                   disk write and therefore writing data into the SQLite database takes < 1 second
                   (with separate transactions it takes tens of minites...) */
                if (dbManager.driverType()==DatabaseManager::SQLite) {
                    QSqlQuery("BEGIN TRANSACTION;",*dbManager.sqlDb());
                }

                do {
                    QDir dirlist(_folders.takeFirst());
                    dirlist.setNameFilters(filters);

                    QDirIterator dirIterator(dirlist,QDirIterator::Subdirectories);
                    while (dirIterator.hasNext()) {
                        dirIterator.next();
                        fileInfo = dirIterator.fileInfo();
                        if (fileInfo.isFile()) {
                            // If the file is not in database then insert
                            if (!dbFiles.contains(fileInfo.filePath().toUtf8())) {
                                insertTrack(fileInfo.absoluteFilePath(), dbManager.sqlDb());
                                dbFiles.remove(fileInfo.filePath().toUtf8());
                                collectionsChanged = true;
                            }

                            // If the file is in database but has another mtime then update it
                            if ((dbFiles.contains(fileInfo.filePath().toUtf8())) &&
                                (dbFiles[fileInfo.filePath().toUtf8()]!=fileInfo.lastModified().toTime_t())) {
                                updateTrack(fileInfo.absoluteFilePath(), dbManager.sqlDb());
                                dbFiles.remove(fileInfo.filePath().toUtf8());
                                collectionsChanged = true;
                            }

                            // If the file is in database and was not changed then do nothing and just
                            // proclaim it processed
                            if ((dbFiles.contains(fileInfo.filePath().toUtf8())) &&
                                (dbFiles[fileInfo.filePath().toUtf8()]==fileInfo.lastModified().toTime_t())) {
                                dbFiles.remove(fileInfo.filePath().toUtf8());
                            }

                        }

                    }
                } while (_folders.size()>0);

                // Get files that are in DB but not on harddrive
                QHashIterator<QString, uint> i(dbFiles);
                while (i.hasNext()) {
                    i.next();
                    removeTrack(i.key(), dbManager.sqlDb());
                    collectionsChanged = true;
                }
                dbFiles.clear();

                // Check for interprets/albums/genres... that are not used anymore
                cleanUpDatabase(dbManager.sqlDb());

                // Now write all data to the disk
                if (dbManager.driverType()==DatabaseManager::SQLite) {
                    QSqlQuery("COMMIT TRANSACTION;",*dbManager.sqlDb());
                }

                /* If anything has changed in collections, this signal will
                   cause CollectionBrowser to be repopulated */
                if (collectionsChanged) {
                    emit collectionChanged();
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

void CollectionBuilder::insertTrack(QString filename, QSqlDatabase *sqlDB)
{
    QFileInfo fileInfo(filename);
    QString fname = fileInfo.filePath().toUtf8();
    uint mtime = fileInfo.lastModified().toTime_t();

    TagLib::FileRef f(fname.toUtf8().constData());

    uint trackNo      = f.tag()->track();
    QString interpret = f.tag()->artist().toCString(true);
    QString album     = f.tag()->album().toCString(true);
    QString title     = f.tag()->title().toCString(true);
    uint year         = f.tag()->year();
    QString genre     = f.tag()->genre().toCString(true);
    uint length       = f.audioProperties()->length();

    if (interpret.isEmpty()) interpret = tr("Unkown artist");
    if (album.isEmpty()) album = tr("Unknown album");
    if (title.isEmpty()) title = fileInfo.baseName();

    bool albumInserted = false;

    QSqlField data("col",QVariant::String);
    {
        data.setValue(album);
        album = sqlDB->driver()->formatValue(data,false);
        QSqlQuery query("SELECT `id` FROM `albums` WHERE `album`="+album+";", *sqlDB);
        query.next();
        if (!query.isValid()) {
            QSqlQuery query("INSERT INTO `albums`(`album`,`tracksCnt`,`totalLength`) " \
                            "VALUES("+album+",1,"+QString::number(length)+");", *sqlDB);
            albumInserted = true;
        } else {
            data.setValue(query.value(0).toString());
            QString albumID = sqlDB->driver()->formatValue(data,false);
            QSqlQuery query("UPDATE `albums` " \
                            "SET `tracksCnt`=`tracksCnt`+1," \
                            "    `totalLength`=`totalLength`+"+QString::number(length)+" " \
                            "WHERE `id`="+albumID+";",*sqlDB);
        }
    }

    {
        data.setValue(genre);
        genre = sqlDB->driver()->formatValue(data,false);
        QSqlQuery query("SELECT COUNT(*) FROM `genres` WHERE `genre`="+genre+";", *sqlDB);
        query.next();
        if (query.value(0).toInt() == 0) {
            QSqlQuery query("INSERT INTO `genres`(`genre`) VALUES("+genre+");", *sqlDB);
        }
    }

    {
        data.setValue(interpret);
        interpret = sqlDB->driver()->formatValue(data,false);
        QSqlQuery query("SELECT `id` FROM `interprets` WHERE `interpret`="+interpret+";", *sqlDB);
        query.next();
        if (!query.isValid()) {
            QSqlQuery query("INSERT INTO `interprets`(`interpret`,`albumsCnt`,`totalLength`) " \
                            "VALUES("+interpret+",1,"+QString::number(length)+");", *sqlDB);
        } else {
            QString albumsCntQuery;
            if (albumInserted) {
                 albumsCntQuery = "`albumsCnt`=`albumsCnt`+1,";
             } else {
                 albumsCntQuery = "";
             }

            data.setValue(query.value(0).toString());
            QString interpretID = sqlDB->driver()->formatValue(data,false);
            QSqlQuery query("UPDATE `interprets` " \
                            "SET "+albumsCntQuery+
                            "    `totalLength`=`totalLength`+"+QString::number(length)+" " \
                            "WHERE `id`="+interpretID+";", *sqlDB);
        }
    }

    QString s_year;
    {
        data.setValue(year);
        s_year = sqlDB->driver()->formatValue(data,false);
        QSqlQuery query("SELECT COUNT(*) FROM `years` WHERE `year`="+s_year+";", *sqlDB);
        query.next();
        if (query.value(0).toInt() == 0) {
            QSqlQuery query("INSERT INTO `years`(`year`) VALUES("+s_year+");", *sqlDB);
        }
    }

    {
        QSqlQuery query(*sqlDB);
        query.prepare("INSERT INTO `tracks`(`filename`,`trackname`,`track`,`length`,`interpret`,`album`,`year`,`genre`,`mtime`)" \
                      "VALUES(?," \
                      "       ?," \
                      "       ?," \
                      "       ?," \
                      "       (SELECT `id` FROM `interprets` WHERE `interpret`="+interpret+")," \
                      "       (SELECT `id` FROM `albums` WHERE `album`="+album+")," \
                      "       (SELECT `id` FROM `years` WHERE `year`="+s_year+")," \
                      "       (SELECT `id` FROM `genres` WHERE `genre`="+genre+")," \
                      "       ?);");
        query.addBindValue(fname);
        query.addBindValue(title);
        query.addBindValue(trackNo);
        query.addBindValue(length);
        query.addBindValue(mtime);
        query.exec();
    }

}

void CollectionBuilder::updateTrack(QString filename, QSqlDatabase *sqlDb)
{
    // Do you know any faster way to do it? :-)
    removeTrack(filename,sqlDb);
    insertTrack(filename,sqlDb);
}

void CollectionBuilder::removeTrack(QString filename, QSqlDatabase *sqlDb)
{
    qDebug() << "Deleting " << filename;
    QFileInfo fileInfo(filename);
    QString fname = fileInfo.filePath().toUtf8();

    QSqlField data("col",QVariant::String);
    data.setValue(fname);
    fname = sqlDb->driver()->formatValue(data,false);
    QSqlQuery query("DELETE FROM `tracks` WHERE `filename`="+fname+";", *sqlDb);
}

void CollectionBuilder::cleanUpDatabase(QSqlDatabase *sqlDb)
{

    {
        QSqlQuery query("DELETE FROM `albums` WHERE `id` NOT IN (SELECT `album` FROM `tracks` GROUP BY `album`);", *sqlDb);
    }

    {
        QSqlQuery query("DELETE FROM `genres` WHERE `id` NOT IN (SELECT `genre` FROM `tracks` GROUP BY `genre`);", *sqlDb);
    }

    {
        QSqlQuery query("DELETE FROM `interprets` WHERE `id` NOT IN (SELECT `interpret` FROM `tracks` GROUP BY `interpret`);", *sqlDb);
    }

    {
        QSqlQuery query("DELETE FROM `years` WHERE `id` NOT IN (SELECT `year` FROM `tracks` GROUP BY `year`);", *sqlDb);
    }

}
