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


#include "playlist.h"
#include "player.h"
#include "utils.h"
#include "m3u.h"
#include "databasemanager.h"

#include <QDebug>
#include <QTime>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlDriver>

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>

using namespace TepSonic;

Q_DECLARE_METATYPE(MetaData);

class Playlist::Private
{
  public:
    Private(Playlist *parent);
    ~Private();
    void loadMetaDataRunnable(const QVector<QPersistentModelIndex> &indexes);

    MetaData::List items;
    mutable QReadWriteLock *itemsLock;

    // total length in seconds
    int totalLength;
    int currentTrack;
    int stopTrack;

    Playlist * const q;
};

Playlist::Private::Private(Playlist *parent):
    itemsLock(new QReadWriteLock(QReadWriteLock::Recursive)),
    totalLength(0),
    currentTrack(-1),
    stopTrack(-1),
    q(parent)
{
    qRegisterMetaType<QVector<int>>();
}

Playlist::Private::~Private()
{
    itemsLock->lockForWrite();
    items.clear();
    itemsLock->unlock();
    delete itemsLock;
}

void Playlist::Private::loadMetaDataRunnable(const QVector<QPersistentModelIndex> &indexes)
{
    if (indexes.isEmpty()) {
        return;
    }

    Q_FOREACH (const QPersistentModelIndex &index, indexes) {
        if (!index.isValid()) {
            continue;
        }

        const QString fileName = index.data(Playlist::MetaDataRole).value<MetaData>().fileName();
        const QFileInfo finfo(fileName);

        if ((!finfo.exists()) || (!finfo.isFile())) {
            continue;
        }

        /* Don't try to connect when connection was not available previously - attempts to connect are
        just slowing everything down */
        if (DatabaseManager::instance()->connectionAvailable()) {
            QSqlField data(QLatin1String("col"), QVariant::String);
            data.setValue(fileName);
            QString fname = DatabaseManager::instance()->sqlDb().driver()->formatValue(data, false);
            QSqlQuery query(DatabaseManager::instance()->sqlDb());
            query.prepare(QLatin1String(
                "SELECT `trackname`, `track`, `length`, `interpret`, `genre`,"
                "       `album`, `year`, `bitrate` "
                "FROM `view_tracks` "
                "WHERE `filename`= :filename "
                "LIMIT 1"));
            query.bindValue(QLatin1String(":filename"), fileName);
            if (query.exec() && query.first()) {
                QWriteLocker locker(itemsLock);
                MetaData &metaData = items[index.row()];
                metaData.setTitle(query.value(0).toString());
                metaData.setTrackNumber(query.value(1).toUInt());
                metaData.setLength(query.value(2).toUInt());
                metaData.setArtist(query.value(3).toString());
                metaData.setGenre(query.value(4).toString());
                metaData.setAlbum(query.value(5).toString());
                metaData.setYear(query.value(6).toUInt());
                metaData.setBitrate(query.value(7).toInt());
                locker.unlock();
                Q_EMIT q->dataChanged(index, index);
                continue;
            }
        }

        TagLib::FileRef f(qPrintable(fileName));

        if (f.isNull() || !f.tag()) {
            qDebug() << finfo.fileName() << " failed to be loaded by TagLib.";
            continue;
        }

        QWriteLocker locker(itemsLock);
        MetaData &metaData = items[index.row()];
        metaData.setTitle(QString::fromUtf8(f.tag()->title().toCString(true)));
        metaData.setTrackNumber(f.tag()->track());
        metaData.setArtist(QString::fromUtf8(f.tag()->artist().toCString(true)));
        metaData.setLength(f.audioProperties()->length() * 1000);
        metaData.setAlbum(QString::fromUtf8(f.tag()->album().toCString(true)));
        metaData.setGenre(QString::fromUtf8(f.tag()->genre().toCString(true)));
        metaData.setYear(f.tag()->year());
        metaData.setBitrate(f.audioProperties()->bitrate());

        if (metaData.title().isEmpty()) {
            metaData.setTitle(finfo.fileName());
        }
        locker.unlock();
        Q_EMIT q->dataChanged(index, index);
    }
}

Playlist::Playlist(QObject *parent) :
    QAbstractItemModel(parent),
    d(new Private(this))
{
}

Playlist::~Playlist()
{
    delete d;
}

QModelIndex Playlist::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return createIndex(row, column);
}

QModelIndex Playlist::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);

    return QModelIndex();
}

int Playlist::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        QReadLocker locker(d->itemsLock);
        return d->items.count();
    }

    return 0;
}

int Playlist::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 1;
}

bool Playlist::setData(const QModelIndex &index, const QVariant &value, int role)
{
    QWriteLocker locker(d->itemsLock);

    Q_ASSERT(index.model() == this);

    if (role == Playlist::MetaDataRole) {
        d->itemsLock->lockForWrite();
        d->items.replace(index.row(), value.value<MetaData>());
        d->itemsLock->unlock();

        Q_EMIT dataChanged(index, index);
        return true;
    }

    return false;
}

QVariant Playlist::data(const QModelIndex &index, int role) const
{
    QReadLocker locker(d->itemsLock);

    Q_ASSERT(index.model() == this);

    if (!index.isValid()) {
        return QVariant();
    }

    d->itemsLock->lockForRead();
    const MetaData metaData = d->items.at(index.row());
    d->itemsLock->unlock();
    if (role == Playlist::MetaDataRole) {
        return QVariant::fromValue(metaData);
    }

    return QVariant();
}

bool Playlist::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    remove(row, count);
    return true;
}

bool Playlist::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                        const QModelIndex &destinationParent, int destinationChild)
{
    Q_UNUSED(sourceParent);
    Q_UNUSED(destinationParent);

    move(sourceRow, count, destinationChild);
    return true;
}

void Playlist::savePlaylist(const QString &file)
{
    QStringList playlist;
    playlist.reserve(d->items.count());
    d->itemsLock->lockForRead();
    const MetaData::List list = d->items;
    d->itemsLock->unlock();
    Q_FOREACH (const MetaData &metaData, list) {
        playlist << metaData.fileName();
    }

    M3U::writeToFile(playlist, file);
}

void Playlist::loadPlaylist(const QString &file)
{
    const QStringList playlist = M3U::loadFromFile(file);
    if (!playlist.isEmpty()) {
        insert(playlist);
    }
}

void Playlist::insert(const QStringList &files, int row)
{
    if (row == -1) {
        d->itemsLock->lockForRead();
        row = d->items.count();
        d->itemsLock->unlock();
    }

    beginInsertRows(QModelIndex(), row, row + files.size() - 1);
    d->itemsLock->lockForWrite();
    const int count = files.count();
    QVector<QPersistentModelIndex> indexesToProcess;
    for (int i = 0; i < count; ++i) {
        MetaData md;
        md.setFileName(files.at(i));
        md.setTitle(QFileInfo(md.fileName()).fileName());
        d->items.insert(row + i, md);
        indexesToProcess << QPersistentModelIndex(index(row + i, 0));
    }
    d->itemsLock->unlock();
    endInsertRows();

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    // QtConcurrent::map crashes, see QTBUG-35280 - we use run() instead
    QFuture<void> future = QtConcurrent::run(d, &Playlist::Private::loadMetaDataRunnable, indexesToProcess);
    watcher->setFuture(future);
    watcher->setPendingResultsLimit(1);
    watcher->setProperty("startOffset", row);
    watcher->setProperty("count", files.count());
    connect(watcher, &QFutureWatcher<void>::finished,
            this, &Playlist::onMetaDataDone);
}

void Playlist::onMetaDataDone()
{
    QFutureWatcher<void> *watcher = static_cast<QFutureWatcher<void> *>(sender());
    watcher->deleteLater();

    const int offset = watcher->property("startOffset").toInt();
    const int count = watcher->property("count").toInt();
    d->itemsLock->lockForRead();
    for (int i = offset; i < offset + count; ++i) {
        const MetaData &metaData = d->items.at(i);
        d->totalLength += (metaData.length() / 1000);
    }
    d->itemsLock->unlock();

    Q_EMIT playlistLengthChanged(d->totalLength, d->items.count());
    /*Q_EMIT dataChanged(index(offset, 0),
                       index(offset + count - 1, 0));
    */
}

void Playlist::insert(const MetaData::List &metaData, int row)
{
    if (row == -1) {
        d->itemsLock->lockForRead();
        row = d->items.count();
        d->itemsLock->unlock();
    }

    beginInsertRows(QModelIndex(), row, row + metaData.count() - 1);
    const int count = metaData.count();
    d->itemsLock->lockForWrite();
    for (int i = 0; i < count; ++i) {
        const MetaData &md = metaData.at(i);
        d->items.insert(row + i, md);
        d->totalLength += md.length();
    }
    d->itemsLock->unlock();
    endInsertRows();

    Q_EMIT playlistLengthChanged(d->totalLength, d->items.count());
}

void Playlist::remove(int start, int count)
{
    beginRemoveRows(QModelIndex(), start, start + count - 1);
    d->itemsLock->lockForWrite();
    for (int i = 0; i < count; ++i) {
        const MetaData &metaData = d->items.takeAt(start);
        d->totalLength -= metaData.length();
    }
    d->itemsLock->unlock();
    endRemoveRows();

    Q_EMIT playlistLengthChanged(d->totalLength, d->items.count());
}

void Playlist::move(int start, int count, int newStart)
{
    beginMoveRows(QModelIndex(), start, start + count - 1, QModelIndex(), newStart);
    d->itemsLock->lockForWrite();
    MetaData::List toMove;
    for (int i = 0; i < count; ++i) {
        d->items.move(start + i, newStart + i);
    }
    d->itemsLock->unlock();
    endMoveRows();
}

void Playlist::setTrack(int i, const MetaData &metaData)
{
    d->itemsLock->lockForWrite();
    d->items.replace(i, metaData);
    d->itemsLock->unlock();
    Q_EMIT dataChanged(index(i, 0), index(i, 0));
}

MetaData Playlist::track(int index) const
{
    QReadLocker locker(d->itemsLock);
    return d->items.at(index);
}

int Playlist::totalLength() const
{
    return d->totalLength;
}

void Playlist::shuffle()
{
    beginResetModel();
    d->itemsLock->lockForWrite();
    std::random_shuffle(d->items.begin(), d->items.end());
    d->itemsLock->unlock();
    endResetModel();
}

void Playlist::clear()
{
    beginResetModel();
    d->itemsLock->lockForWrite();
    d->items.clear();
    d->itemsLock->unlock();
    d->currentTrack = -1;
    d->stopTrack = -1;
    d->totalLength = 0;
    endResetModel();

    Q_EMIT playlistLengthChanged(0 ,0);
}
