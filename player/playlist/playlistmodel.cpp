/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <dan@progdan.cz>
 * Copyright 2009 Petr Los <petr_los@centrum.cz>
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


#include "playlistmodel.h"
#include "playlistproxymodel.h"
#include "player.h"
#include "utils.h"
#include "m3u.h"
#include "databasemanager.h"

#include <QtCore/QDebug>
#include <QtCore/QTime>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlDriver>

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>

Q_DECLARE_METATYPE(MetaData);

void PlaylistModel::loadMetaDataRunnable(const MetaData::List::Iterator &start,
                                         const MetaData::List::Iterator &end)
{
    QWriteLocker locker(m_itemsLock);
    if (start == m_items.end()) {
        return;
    }

    for (MetaData::List::Iterator iter = start; iter <= end && iter < m_items.end(); ++iter) {
        const QFileInfo finfo((*iter).fileName());

        if ((!finfo.exists()) || (!finfo.isFile())) {
            continue;
        }

        /* Don't try to connect when connection was not available previously - attempts to connect are
        just slowing everything down */
        if (DatabaseManager::instance()->connectionAvailable()) {
            QSqlField data(QLatin1String("col"), QVariant::String);
            data.setValue((*iter).fileName());
            QString fname = DatabaseManager::instance()->sqlDb().driver()->formatValue(data, false);
            QSqlQuery query(DatabaseManager::instance()->sqlDb());
            query.prepare(QLatin1String(
                "SELECT `trackname`, `track`, `length`, `interpret`, `genre`,"
                "       `album`, `year`, `bitrate` "
                "FROM `view_tracks` "
                "WHERE `filename`= :filename "
                "LIMIT 1"));
            query.bindValue(QLatin1String(":filename"), (*iter).fileName());
            if (query.exec() && query.first()) {
                (*iter).setTitle(query.value(0).toString());
                (*iter).setTrackNumber(query.value(1).toUInt());
                (*iter).setLength(query.value(2).toUInt());
                (*iter).setArtist(query.value(3).toString());
                (*iter).setGenre(query.value(4).toString());
                (*iter).setAlbum(query.value(5).toString());
                (*iter).setYear(query.value(6).toUInt());
                (*iter).setBitrate(query.value(7).toInt());
                continue;
            }
        }

        TagLib::FileRef f((*iter).fileName().toUtf8().constData());

        if (f.isNull() || !f.tag()) {
            qDebug() << finfo.fileName() << " failed to be loaded by TagLib.";
            continue;
        }

        (*iter).setTitle(QString::fromUtf8(f.tag()->title().toCString(true)));
        (*iter).setTrackNumber(f.tag()->track());
        (*iter).setArtist(QString::fromUtf8(f.tag()->artist().toCString(true)));
        (*iter).setLength(f.audioProperties()->length() * 1000);
        (*iter).setAlbum(QString::fromUtf8(f.tag()->album().toCString(true)));
        (*iter).setGenre(QString::fromUtf8(f.tag()->genre().toCString(true)));
        (*iter).setYear(f.tag()->year());
        (*iter).setBitrate(f.audioProperties()->bitrate());

        if ((*iter).title().isEmpty()) {
            (*iter).setTitle(finfo.fileName());
        }
    }
}

PlaylistModel::PlaylistModel(QObject *parent) :
    QAbstractItemModel(parent),
    m_itemsLock(new QReadWriteLock(QReadWriteLock::Recursive)),
    m_totalLength(0)
{
}

PlaylistModel::~PlaylistModel()
{
    m_itemsLock->lockForWrite();
    m_items.clear();
    m_itemsLock->unlock();
    delete m_itemsLock;
}

int PlaylistModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return PlaylistModel::ColumnCount;
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    QReadLocker locker(m_itemsLock);

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.column() < PlaylistModel::ColumnCount);

    if (!index.isValid()) {
        return QVariant();
    }

    const MetaData metaData = m_items.at(index.row());
    if (role == PlaylistModel::MetaDataRole) {
        return QVariant::fromValue(metaData);
    } else if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case FilenameColumn:
                return metaData.fileName();
            case TrackColumn:
                return metaData.trackNumber();
            case InterpretColumn:
                return metaData.artist();
            case TracknameColumn:
                return metaData.title();
            case AlbumColumn:
                return metaData.album();
            case GenreColumn:
                return metaData.genre();
            case YearColumn:
                return metaData.year();
            case LengthColumn:
                return metaData.formattedLength();
            case BitrateColumn:
                return QString::fromLatin1("%1 kbps").arg(metaData.bitrate());
        }
    }

    return QVariant();
}

bool PlaylistModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    QWriteLocker locker(m_itemsLock);

    Q_UNUSED(role);
    Q_ASSERT(index.model() == this);

    MetaData::List::Iterator iter = m_items.begin();
    iter += index.row();

    switch (index.column()) {
        case FilenameColumn:
            (*iter).setFileName(value.toString());
            break;
        case TrackColumn:
            (*iter).setTrackNumber(value.toUInt());
            break;
        case InterpretColumn:
            (*iter).setArtist(value.toString());
            break;
        case TracknameColumn:
            (*iter).setTitle(value.toString());
            break;
        case AlbumColumn:
            (*iter).setAlbum(value.toString());
            break;
        case GenreColumn:
            (*iter).setGenre(value.toString());
            break;
        case YearColumn:
            (*iter).setYear(value.toUInt());
            break;
        case LengthColumn:
            (*iter).setLength(value.toInt());
            break;
        case BitrateColumn:
            (*iter).setBitrate(value.toInt());
            break;
    }

    return true;
}


Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const
{
    Q_ASSERT(index.model() == this);

    if (!index.isValid()) {
        return 0;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const
{
    if (role == Qt::DisplayRole) {
        switch (section) {
            case FilenameColumn:
                return tr("Filename");
            case TrackColumn:
                return tr("Track");
            case InterpretColumn:
                return tr("Interpret");
            case TracknameColumn:
                return tr("Track Name");
            case AlbumColumn:
                return tr("Album");
            case GenreColumn:
                return tr("Genre");
            case YearColumn:
                return tr("Year");
            case BitrateColumn:
                return tr("Bitrate");
            case LengthColumn:
                return tr("Length");
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return createIndex(row, column);
}

QModelIndex PlaylistModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);

    return QModelIndex();
}

bool PlaylistModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    if (rows == 0) {
        return true;
    }

    int totalRemoveTime = 0;
    beginRemoveRows(parent, position, position + rows - 1);
    m_itemsLock->lockForWrite();
    for (int i = 0; i < rows; i++) {
        const MetaData metadata = m_items.takeAt(position);
        totalRemoveTime += metadata.length() / 1000;
    }
    m_itemsLock->unlock();
    endRemoveRows();

    // Decrease total length of the playlist by total length of removed tracks
    m_totalLength -= totalRemoveTime;
    Q_EMIT playlistLengthChanged(m_totalLength, rowCount(QModelIndex()));

    return true;
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        QReadLocker locker(m_itemsLock);
        return m_items.count();
    }

    return 0;
}

void PlaylistModel::insertItem(const MetaData &metadata, int row)
{
    if (row < 0) {
        row = 0;
    }

    beginInsertRows(QModelIndex(), row, row);
    m_itemsLock->lockForWrite();
    m_items.insert(row, metadata);
    m_itemsLock->unlock();
    endInsertRows();

    m_totalLength += (metadata.length() / 1000);
    Q_EMIT playlistLengthChanged(m_totalLength, m_items.count());
}

void PlaylistModel::clear()
{

    beginResetModel();
    m_itemsLock->lockForWrite();
    m_items.clear();
    m_itemsLock->unlock();
    endResetModel();
}

void PlaylistModel::addFile(const QString &file)
{
    addFiles(QStringList() << file);
}

void PlaylistModel::addFiles(const QStringList &files)
{
    insertFiles(files, m_items.count());
}

void PlaylistModel::insertFiles(const QStringList &files, int row)
{
    if (files.isEmpty()) {
        return;
    }

    const int startIndex = (row >= 0) ? row : 0;

    beginInsertRows(QModelIndex(), startIndex, startIndex + files.size() - 1);
    // lock after beginInsertRows(), which calls rowCount() internally, so we would deadlock
    QWriteLocker locker(m_itemsLock);
    for (int i = 0; i < files.count(); ++i) {
        MetaData md;
        md.setFileName(files.at(i));
        md.setTitle(QFileInfo(md.fileName()).fileName());
        m_items.insert(row + i, md);
    }
    locker.unlock();
    endInsertRows();

    const MetaData::List::Iterator start = m_items.begin() + row;
    const MetaData::List::iterator end = start + files.count();

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    // QtConcurrent::map crashes, see QTBUG-35280 - we use run() instead
    QFuture<void> future = QtConcurrent::run(this, &PlaylistModel::loadMetaDataRunnable, start, end);
    watcher->setFuture(future);
    watcher->setPendingResultsLimit(1);
    watcher->setProperty("startOffset", startIndex);
    connect(watcher, &QFutureWatcher<void>::resultsReadyAt,
            this, &PlaylistModel::onMetaDataAvailable);
    connect(watcher, &QFutureWatcher<void>::finished,
            this, &PlaylistModel::onMetaDataDone);
}

void PlaylistModel::onMetaDataAvailable(int beginIndex, int endIndex)
{
    qDebug() << "onMetaDataAvailable:" << beginIndex << endIndex;
    QFutureWatcher<void> *watcher = static_cast<QFutureWatcher<void> *>(sender());
    const int offset = watcher->property("startOffset").toInt();

    int totalLength = 0;
    for (int i = offset + beginIndex; i <= endIndex; ++i) {
        const MetaData &metaData = m_items.at(i);
        totalLength += (metaData.length() / 1000);
    }
    m_totalLength += totalLength;

    Q_EMIT playlistLengthChanged(m_totalLength, m_items.count());
    Q_EMIT dataChanged(index(offset + beginIndex, 0),
                       index(offset + endIndex, ColumnCount));
}

void PlaylistModel::onMetaDataDone()
{
    QFutureWatcher<void> *watcher = static_cast<QFutureWatcher<void> *>(sender());
    watcher->deleteLater();

    const int offset = watcher->property("startOffset").toInt();
    qDebug() << "onMetaDataDone:" << offset << watcher->future().resultCount();

    Q_EMIT dataChanged(index(offset, 0), index(offset + watcher->future().resultCount(), 0));
}


void PlaylistModel::loadPlaylist(const QString &file)
{
    const QStringList playlist = M3U::loadFromFile(file);
    if (!playlist.isEmpty()) {
        addFiles(playlist);
    }
}

void PlaylistModel::savePlaylist(const QString& file)
{
    QStringList playlist;
    playlist.reserve(m_items.count());
    m_itemsLock->lockForRead();
    Q_FOREACH (const MetaData &metaData, m_items) {
        playlist << metaData.fileName();
    }
    m_itemsLock->unlock();

    if (!playlist.isEmpty()) {
        M3U::writeToFile(playlist, file);
    }
}
