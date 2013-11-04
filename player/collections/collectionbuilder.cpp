/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <dan@progdan.cz>
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
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlField>
#include <QSqlError>
#include <QSqlResult>
#include <QHash>
#include <QHashIterator>

CollectionBuilder::CollectionBuilder():
    QObject(),
    QRunnable()
{
}

void CollectionBuilder::run()
{
    DatabaseManager *dbManager = DatabaseManager::instance();
    if (m_folders.isEmpty() || !dbManager->connectionAvailable()) {
        Q_EMIT buildingFinished();
        return;
    }

    Q_EMIT buildingStarted();

    bool collectionsChanged = false;
    qDebug() << "Starting collections update...";

    QFileInfo fileInfo;
    // List of files in database
    QHash<QString, uint> dbFiles;
    const QStringList filters = SupportedFormats::getExtensionList();
    {
        // Populate dbFiles map by ALL tracks from db
        QSqlQuery query(dbManager->sqlDb());
        query.prepare(QLatin1String("SELECT filename, mtime FROM tracks WHERE filename LIKE :folder"));
        query.bindValue(QLatin1String(":folder"), m_folders.first() + QLatin1Char('%'));
        query.exec();
        while (query.next()) {
            dbFiles.insert(query.value(0).toString(), query.value(1).toUInt());
        }
    }

    /* We don't need to do transaction on MySQL.
       On SQLite it makes it MUCH faster because otherwise SQLite will commit
       every INSERT individually and that makes ~60 commits per second (depends on the
       speed of harddrive) meanwhile having everything in one transaction does only one
       disk write and therefore writing data into the SQLite database takes < 1 second
       (with separate transactions it takes tens of minutes...) */
    if (dbManager->driverType() == DatabaseManager::SQLite) {
        QSqlQuery(QLatin1String("BEGIN TRANSACTION;"), dbManager->sqlDb());
    }

    QHash<QString, int> atirts;
    do {
        QDir dirlist(m_folders.takeFirst());
        dirlist.setNameFilters(filters);

        QDirIterator dirIterator(dirlist, QDirIterator::Subdirectories);
        while (dirIterator.hasNext()) {
            dirIterator.next();
            fileInfo = dirIterator.fileInfo();
            if (fileInfo.isFile()) {
                // If the file is not in database then insert
                if (!dbFiles.contains(fileInfo.filePath())) {
                    insertTrack(fileInfo.absoluteFilePath());
                    dbFiles.remove(fileInfo.filePath());
                    collectionsChanged = true;
                }

                // If the file is in database but has another mtime then update it
                if ((dbFiles.contains(fileInfo.filePath())) &&
                        (dbFiles[fileInfo.filePath()] != fileInfo.lastModified().toTime_t())) {
                    updateTrack(fileInfo.absoluteFilePath());
                    dbFiles.remove(fileInfo.filePath());
                    collectionsChanged = true;
                }

                // If the file is in database and was not changed then do nothing and just
                // proclaim it processed
                if ((dbFiles.contains(fileInfo.filePath())) &&
                        (dbFiles[fileInfo.filePath()] == fileInfo.lastModified().toTime_t())) {
                    dbFiles.remove(fileInfo.filePath());
                }

            }

        }
    } while (m_folders.size() > 0);

    // Get files that are in DB but not on harddrive
    QHashIterator<QString, uint> i(dbFiles);
    while (i.hasNext()) {
        i.next();
        removeTrack(i.key());
        collectionsChanged = true;
    }
    dbFiles.clear();

    // Check for interprets/albums/genres... that are not used anymore
    cleanUpDatabase();

    // Now write all data to the disk
    if (dbManager->driverType() == DatabaseManager::SQLite) {
        QSqlQuery(QLatin1String("COMMIT TRANSACTION;"), dbManager->sqlDb());
    }

    /* If anything has changed in collections, this signal will
        cause CollectionBrowser to be repopulated */
    if (collectionsChanged) {
        emit collectionChanged();
    }

    emit buildingFinished();
}

void CollectionBuilder::rebuildFolder(const QStringList &folder)
{
    m_folders.append(folder);
}

void CollectionBuilder::insertTrack(const QString &filename)
{
    const QFileInfo fileInfo(filename);
    const QString fname = fileInfo.filePath();
    const uint mtime = fileInfo.lastModified().toTime_t();

    const TagLib::FileRef f(fname.toLocal8Bit().constData());
    // FileRef::isNull() is not enough sometimes. Let's check the tag() too...
    if (f.isNull() || !f.tag()) {
        qDebug() << filename << "cannot be registered in the collection. TagLib.";
        // to prevent crash calling f.tag() if it's NULL
        return;
    }

    uint trackNo = f.tag()->track();
    QString interpret = QString::fromLatin1(f.tag()->artist().toCString(true));
    QString album = QString::fromLatin1(f.tag()->album().toCString(true));
    QString title = QString::fromLatin1(f.tag()->title().toCString(true));
    uint year = f.tag()->year();
    QString genre = QString::fromLatin1(f.tag()->genre().toCString(true));
    uint length = f.audioProperties()->length();
    int bitrate = f.audioProperties()->bitrate();

    if (interpret.isEmpty()) {
        interpret = tr("Unkown artist");
    }
    if (album.isEmpty()) {
        album = tr("Unknown album");
    }
    if (title.isEmpty()) {
        title = fileInfo.baseName();
    }

    bool albumInserted = false;

    QSqlField data(QLatin1String("col"), QVariant::String);
    {
        data.setValue(album);
        album = DatabaseManager::instance()->sqlDb().driver()->formatValue(data, false);
        QSqlQuery query(DatabaseManager::instance()->sqlDb());
        query.prepare(QLatin1String("SELECT `id` FROM `albums` WHERE `album`=:album"));
        query.bindValue(QLatin1String(":album"), album);
        query.exec();
        query.next();
        if (!query.isValid()) {
            QSqlQuery query(DatabaseManager::instance()->sqlDb());
            query.prepare(QLatin1String("INSERT INTO `albums`(`album`,`tracksCnt`,`totalLength`) "
                                        "VALUES(:album, 1, :length)"));
            query.bindValue(QLatin1String(":album"), album);
            query.bindValue(QLatin1String(":length"), length);
            query.exec();
            albumInserted = true;
        } else {
            data.setValue(query.value(0).toString());
            const QString albumID = DatabaseManager::instance()->sqlDb().driver()->formatValue(data, false);
            QSqlQuery query(DatabaseManager::instance()->sqlDb());
            query.prepare(QLatin1String("UPDATE `albums` "
                                        "SET `tracksCnt`=`tracksCnt`+1,"
                                        "    `totalLength`=`totalLength`+:length "
                                        "WHERE `id`=:albumId"));
            query.bindValue(QLatin1String(":length"), length);
            query.bindValue(QLatin1String(":albumId"), albumID.toInt());
            query.exec();
        }
    }

    {
        data.setValue(genre);
        genre = DatabaseManager::instance()->sqlDb().driver()->formatValue(data, false);
        QSqlQuery query(DatabaseManager::instance()->sqlDb());
        query.prepare(QLatin1String("SELECT COUNT(*) FROM `genres` WHERE `genre`=:genre"));
        query.bindValue(QLatin1String(":genre"), genre);
        query.exec();
        query.next();
        if (query.value(0).toInt() == 0) {
            QSqlQuery query;
            query.prepare(QLatin1String("INSERT INTO `genres`(`genre`) VALUES(:genre)"));
            query.bindValue(QLatin1String(":genre"), genre);
            query.exec();
        }
    }

    {
        data.setValue(interpret);
        interpret = DatabaseManager::instance()->sqlDb().driver()->formatValue(data, false);
        QSqlQuery query(DatabaseManager::instance()->sqlDb());
        query.prepare(QLatin1String("SELECT `id` FROM `interprets` WHERE `interpret`=:interpret"));
        query.bindValue(QLatin1String(":interpret"), interpret);
        query.exec();
        query.next();
        if (!query.isValid()) {
            QSqlQuery query;
            query.prepare(QLatin1String("INSERT INTO `interprets`(`interpret`,`albumsCnt`,`totalLength`) "
                                        "VALUES(:interpret, 1, :length)")); 
            query.bindValue(QLatin1String(":interpret"), interpret);
            query.bindValue(QLatin1String(":length"), length);
            query.exec();
        } else {
            QString albumsCntQuery;
            if (albumInserted) {
                albumsCntQuery = QLatin1String("`albumsCnt`=`albumsCnt`+1,");
            } else {
                albumsCntQuery = QLatin1String("");
            }

            data.setValue(query.value(0).toString());
            const QString interpretID = DatabaseManager::instance()->sqlDb().driver()->formatValue(data, false);
            QSqlQuery query(DatabaseManager::instance()->sqlDb());
            query.prepare(QLatin1String("UPDATE `interprets` "
                                        "SET ") + albumsCntQuery +
                                        QLatin1String("    `totalLength`=`totalLength`+:length "
                                        "WHERE `id`= :interpretID"));
            query.bindValue(QLatin1String(":length"), length);
            query.bindValue(QLatin1String(":interpretID"), interpretID);
            query.exec();
        }
    }

    QString s_year;
    {
        data.setValue(year);
        s_year = DatabaseManager::instance()->sqlDb().driver()->formatValue(data, false);
        QSqlQuery query(DatabaseManager::instance()->sqlDb());
        query.prepare(QLatin1String("SELECT COUNT(*) FROM `years` WHERE `year`=:year"));
        query.bindValue(QLatin1String(":year"), s_year.toInt());
        query.exec();
        query.next();
        if (query.value(0).toInt() == 0) {
            QSqlQuery query(DatabaseManager::instance()->sqlDb());
            query.prepare(QLatin1String("INSERT INTO `years`(`year`) VALUES(:year)"));
            query.bindValue(QLatin1String(":year"), s_year.toInt());
            query.exec();
        }
    }

    {
        QSqlQuery query(DatabaseManager::instance()->sqlDb());
        query.prepare(QLatin1String(
                     "INSERT INTO `tracks`(`filename`,`trackname`,`track`,`length`,`interpret`,`album`,`year`,`genre`,`mtime`,`bitrate`)"
                      "VALUES(:filename,"
                      "       :trackname,"
                      "       :track,"
                      "       :length,"
                      "       (SELECT `id` FROM `interprets` WHERE `interpret`=:interpret),"
                      "       (SELECT `id` FROM `albums` WHERE `album`=:album),"
                      "       (SELECT `id` FROM `years` WHERE `year`=:year),"
                      "       (SELECT `id` FROM `genres` WHERE `genre`=:genre),"
                      "       :mtime,"
                      "       :bitrate);"));
        query.bindValue(QLatin1String(":filename"), fname);
        query.bindValue(QLatin1String(":tracnkane"), title);
        query.bindValue(QLatin1String(":track"), trackNo);
        query.bindValue(QLatin1String(":length"), length);
        query.bindValue(QLatin1String(":interpret"), interpret);
        query.bindValue(QLatin1String(":album"), album);
        query.bindValue(QLatin1String(":year"), year);
        query.bindValue(QLatin1String(":genre"), genre);
        query.bindValue(QLatin1String(":mtime"), mtime);
        query.bindValue(QLatin1String(":bitrate"), bitrate);
        query.exec();
    }

}

void CollectionBuilder::updateTrack(const QString &filename)
{
    // Do you know any faster way to do it? :-)
    removeTrack(filename);
    insertTrack(filename);
}

void CollectionBuilder::removeTrack(const QString &filename)
{
    const QFileInfo fileInfo(filename);
    QString fname = fileInfo.filePath();

    QSqlField data(QLatin1String("col"), QVariant::String);
    data.setValue(fname);
    fname = DatabaseManager::instance()->sqlDb().driver()->formatValue(data, false);
    QSqlQuery query(DatabaseManager::instance()->sqlDb());
    query.prepare(QLatin1String("DELETE FROM `tracks` WHERE `filename`=:filename"));
    query.bindValue(QLatin1String(":filename"), fname);
    query.exec();
}

void CollectionBuilder::cleanUpDatabase()
{
    {
        QSqlQuery query(QLatin1String("DELETE FROM `albums` WHERE `id` NOT IN (SELECT `album` FROM `tracks` GROUP BY `album`);"),
                        DatabaseManager::instance()->sqlDb());
    }

    {
        QSqlQuery query(QLatin1String("DELETE FROM `genres` WHERE `id` NOT IN (SELECT `genre` FROM `tracks` GROUP BY `genre`);"),
                        DatabaseManager::instance()->sqlDb());
    }

    {
        QSqlQuery query(QLatin1String("DELETE FROM `interprets` WHERE `id` NOT IN (SELECT `interpret` FROM `tracks` GROUP BY `interpret`);"),
                        DatabaseManager::instance()->sqlDb());
    }

    {
        QSqlQuery query(QLatin1String("DELETE FROM `years` WHERE `id` NOT IN (SELECT `year` FROM `tracks` GROUP BY `year`);"),
                        DatabaseManager::instance()->sqlDb());
    }
}

#include "moc_collectionbuilder.cpp"
