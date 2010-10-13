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

#include "playlistpopulator.h"
#include "supportedformats.h"
#include "player.h"
#include "databasemanager.h"
#include "tools.h"

#include <QDirIterator>
#include <QMutexLocker>
#include <QDebug>
#include <QFileInfo>
#include <QSqlField>
#include <QSqlDriver>
#include <QSqlQuery>

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>

PlaylistPopulator::PlaylistPopulator()
{
    _files.clear();

}

void PlaylistPopulator::run()
{
    do {

        if (_files.size() > 0) {
            if (QFileInfo(_files.first()).isDir()) {
                expandDir(_files.takeFirst());
            }
            if (_files.size()>0) {
                if (QFileInfo(_files.first()).suffix().toLower()=="m3u") {
                    expandPlaylist(_files.takeFirst());
                }
            }
            if (_files.size() > 0) {

                Player::MetaData metadata = getFileMetaData(_files.takeFirst());
                emit insertItemToPlaylist(metadata, _row);
                _row++; // Next item will be inserted AFTER the recently inserted
            }
        }

    } while (!_files.isEmpty());

    emit playlistPopulated();
}

void PlaylistPopulator::expandDir(QString dir)
{
    QStringList filters = SupportedFormats::getExtensionList();
    //QDir dirlist(dir,QString(),QDir::Name,QDir::NoDotAndDotDot);
    QDir dirlist(dir);
    dirlist.setNameFilters(filters);
    QFileInfo fileInfo;

    QStringList files;

    QDirIterator dirIterator(dirlist,QDirIterator::Subdirectories);

    while (dirIterator.hasNext()) {
        dirIterator.next();
        fileInfo = dirIterator.fileInfo();
        if (fileInfo.isFile()) {
            files << fileInfo.filePath();
        }
    }

    files.append(_files);

    // We don't need to lock the mutex here since we are already in locked mutex
    _files = files;
}

void PlaylistPopulator::expandPlaylist(QString filename)
{
    QDir playlistDir(QFileInfo(filename).path());
    QFile file(filename);

    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "PlaylistManager::addPlaylistFile: Failed to open file " << filename;
        return;
    }

    QStringList files;

    char buf[1024];
    qint64 lineLength;
    do {
        lineLength = file.readLine(buf, sizeof(buf));
        if (lineLength != -1) {
            QString filepath = playlistDir.absoluteFilePath(buf).remove("\n",Qt::CaseInsensitive);
            if (filepath.length()>0)
                files << filepath;
        }
    } while (lineLength > -1);

    file.close();

    files.append(_files);
    _files = files;
}

Player::MetaData PlaylistPopulator::getFileMetaData(QString file)
{
    QFileInfo finfo(file);

    Player::MetaData metadata;

    if ((!finfo.exists()) || (!finfo.isFile()))
        return metadata;



    // Just a harmless check wheter the file is in DB - reading data from DB will be faster then from file
    DatabaseManager dbManager("playlistPopulatorConnection");
    /* Don't try to connect when connection was not available previously - attempts to connect are
      just slowing everything down */
    if (DatabaseManager::connectionAvailable()) {
        if (dbManager.connectToDB()) {
            QSqlField data("col",QVariant::String);
            data.setValue(file);
            QString fname = dbManager.sqlDb()->driver()->formatValue(data,false);
            QSqlQuery query("SELECT `filename`," \
                            "       `trackname`," \
                            "       `track`," \
                            "       `length`," \
                            "       `interpret`," \
                            "       `genre`," \
                            "       `album`," \
                            "       `year`," \
                            "       `bitrate`" \
                            "FROM `view_tracks` "\
                            "WHERE `filename`="+fname+ \
                            "LIMIT 1;",
                            *dbManager.sqlDb());
            if (query.first()) {
                metadata.filename = query.value(0).toString();
                metadata.title = query.value(1).toString();
                metadata.trackNumber = query.value(2).toUInt();
                metadata.length = query.value(3).toUInt();
                metadata.artist = query.value(4).toString();
                metadata.genre = query.value(5).toString();
                metadata.album = query.value(6).toString();
                metadata.year = query.value(7).toUInt();
                metadata.bitrate = query.value(8).toInt();
            }
        } else {
            qDebug() << "PlaylistPopulator: Disabling connection to DB for this session";
        }
    }

    if (metadata.filename.isEmpty()) {
        TagLib::FileRef f(file.toLocal8Bit().constData());

        metadata.filename = file.toUtf8().constData();
        metadata.title = f.tag()->title().toCString(true);
        metadata.trackNumber = f.tag()->track();
        metadata.artist = f.tag()->artist().toCString(true);
        metadata.length = f.audioProperties()->length()*1000;
        metadata.album = f.tag()->album().toCString(true);
        metadata.genre = f.tag()->genre().toCString(true);
        metadata.year = f.tag()->year();
        metadata.bitrate = f.audioProperties()->bitrate();

        if (metadata.title.isEmpty())
            metadata.title = finfo.fileName();
    }

    // And length of the track to the total length of the playlist
    metadata.formattedLength = formatMilliseconds(metadata.length);

    return metadata;
}

void PlaylistPopulator::addFile(const QString &file, int row)
{
    _files.append(file);
    _row = row;
}

void PlaylistPopulator::addFiles(const QStringList &files, int firstRow)
{
    _files.append(files);
    _row = firstRow;
}
