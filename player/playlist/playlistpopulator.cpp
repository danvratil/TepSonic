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
    m_files.clear();
}

void PlaylistPopulator::run()
{
    do {
        if (m_files.size() > 0) {
            if (QFileInfo(m_files.first()).isDir()) {
                expandDir(m_files.takeFirst());
            }
            if (m_files.size() > 0) {
                if (QFileInfo(m_files.first()).suffix().toLower() == "m3u") {
                    expandPlaylist(m_files.takeFirst());
                }
            }
            if (m_files.size() > 0) {
                const Player::MetaData metadata = getFileMetaData(m_files.takeFirst());
                Q_EMIT insertItemToPlaylist(metadata, m_row);
                m_row++; // Next item will be inserted AFTER the recently inserted
            }
        }

    } while (!m_files.isEmpty());

    Q_EMIT playlistPopulated();
}

void PlaylistPopulator::expandDir(const QString &dir)
{
    const QStringList filters = SupportedFormats::getExtensionList();
    QDir dirlist(dir);
    dirlist.setNameFilters(filters);
    dirlist.setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    dirlist.setSorting(QDir::Name | QDir::LocaleAware);

    const QFileInfoList fileInfo = dirlist.entryInfoList();
    for (int i = 0; i < fileInfo.length(); i++) {
        const QFileInfo finfo = fileInfo.at(i);

        if (finfo.isFile()) {
            m_files << finfo.filePath();
        } else if (finfo.isDir()) {
            expandDir(finfo.absoluteFilePath());
        }
    }
}

void PlaylistPopulator::expandPlaylist(const QString &filename)
{
    const QDir playlistDir(QFileInfo(filename).path());
    QFile file(filename);

    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "PlaylistManager::addPlaylistFile: Failed to open file " << filename;
        return;
    }

    QStringList files;

    qint64 lineLength;
    while (!file.atEnd()) {
        const QByteArray line = file.readLine();
        const QString filepath = playlistDir.absoluteFilePath(line).remove("\n", Qt::CaseInsensitive);
        if (!filepath.isEmpty()) {
            files << filepath;
        }
    }

    file.close();

    m_files.prepend(files);
}

Player::MetaData PlaylistPopulator::getFileMetaData(const QString &file)
{
    const QFileInfo finfo(file);

    Player::MetaData metadata;

    if ((!finfo.exists()) || (!finfo.isFile())) {
        return metadata;
    }

    // Just a harmless check wheter the file is in DB - reading data from DB will be faster then from file
    DatabaseManager *dbManager = DatabaseManager::instance();

    /* Don't try to connect when connection was not available previously - attempts to connect are
      just slowing everything down */
    if (dbManager->connectionAvailable()) {
        QSqlField data("col", QVariant::String);
        data.setValue(file);
        QString fname = dbManager->sqlDb().driver()->formatValue(data, false);
        QSqlQuery query("SELECT `filename`,"
                        "       `trackname`,"
                        "       `track`,"
                        "       `length`,"
                        "       `interpret`,"
                        "       `genre`,"
                        "       `album`,"
                        "       `year`,"
                        "       `bitrate` "
                        "FROM `view_tracks` "
                        "WHERE `filename`=" + fname +
                        "LIMIT 1;",
                        dbManager->sqlDb());
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
    }

    if (metadata.filename.isEmpty()) {
        TagLib::FileRef f(file.toLocal8Bit().constData());

        if (f.isNull() || !f.tag()) {
            qDebug() << file << " failed to be loaded by TagLib.";
            metadata.filename = file.toUtf8().constData();
            metadata.title = QFileInfo(file).fileName();
            return metadata;
        }

        metadata.filename = file.toUtf8().constData();
        metadata.title = f.tag()->title().toCString(true);
        metadata.trackNumber = f.tag()->track();
        metadata.artist = f.tag()->artist().toCString(true);
        metadata.length = f.audioProperties()->length() * 1000;
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
    m_files.append(file);
    m_row = row;
}

void PlaylistPopulator::addFiles(const QStringList &files, int firstRow)
{
    m_files.append(files);
    m_row = firstRow;
}

#include "moc_playlistpopulator.cpp"
