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


#include "collectionsupdater.h"

#include <QStringList>
#include <QFileInfo>
#include <QMap>
#include <QtSql/QSqlQuery>
#include <QDir>
#include <QDirIterator>
#include <QRegExp>
#include <QDateTime>

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>

CollectionsUpdater::CollectionsUpdater(QSettings *settings)
{
    _settings = settings;
}

CollectionsUpdater::~CollectionsUpdater()
{
}

void CollectionsUpdater::run()
{
    // Store all paths that will be searched
    QStringList paths = _settings->value("Collections/SourcePaths",QStringList()).toStringList();
    QFileInfo fileInfo;
    // List of files in database
    QMap<QString,uint> dbFiles;
    QStringList toBeUpdated;
    QStringList toBeRemoved;
    QStringList filters;

    bool anythingUpdated = false;

    QSqlQuery query("SELECT `filename`,`mtime` FROM `Tracks`;");
    while (query.next()) {
        dbFiles[query.value(0).toString()] = query.value(1).toUInt();
    }

    filters << "*.mp3" << "*.mp4" << "*.wav" << "*.flac";
    int filesProcessed = 0;
    for (int i = 0; i < paths.count(); i++) {
        QDir dirlist(paths.at(i));
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
        QSqlQuery query("DELETE FROM `Tracks` WHERE `filename` IN ('"+toBeRemoved.join("','")+"');");
        if (query.numRowsAffected() > 0) {
            anythingUpdated = true;
        }
    }
    toBeRemoved.clear(); // Memory saving...

    QStringList values;
    // Next we will prepare set of input data for each file that should be updated
    QRegExp rx("\'");
    if (toBeUpdated.count() > 0) {
        for (int i = 0; i < toBeUpdated.count(); i++) {
            TagLib::FileRef f(toBeUpdated.at(i).toUtf8());
            int trackNo = f.tag()->track();
            QString filename = toBeUpdated.at(i);
            QString artist = f.tag()->artist().toCString(true);
            QString album = f.tag()->album().toCString(true);
            QString title = f.tag()->title().toCString(true);
            filename.replace(rx,"\\'");
            artist.replace(rx,"\\'");
            album.replace(rx,"\\'");
            title.replace(rx,"\\'");
            uint mtime = QFileInfo(toBeUpdated.at(i)).lastModified().toTime_t();
            values << "'"+filename+"',"+QString::number(trackNo)+",'"+artist+"','"+album+"','"+title+"',"+QString::number(mtime);
        }
        QSqlQuery query("INSERT INTO `Tracks`" \
                        "   (`filename`,`trackNo`,`artist`,`album`,`title`,`mtime`)" \
                        "   VALUES("+values.join("),(")+")" \
                        "ON DUPLICATE KEY UPDATE `trackNo`=VALUES(`trackNo`)," \
                        "                        `artist`=VALUES(`artist`)," \
                        "                        `album`=VALUES(`album`)," \
                        "                        `title`=VALUES(`title`)," \
                        "                        `mtime`=VALUES(`mtime`);");
        if (query.numRowsAffected() > 0) {
            anythingUpdated = true;
        }
        values.clear();
    }

    emit collectionsUpdated(anythingUpdated);

    exec();
}
