/*
 * TEPSONIC
 * Copyright 2009 Dan Vratil <vratil@progdansoft.com>
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
#include "playlistitem.h"
#include "playlistproxymodel.h"
#include "player.h"
#include "playlistbrowser.h"

#include <QtCore/QDebug>
#include <QtCore/QTime>

PlaylistModel::PlaylistModel(QObject *parent, const QStringList &headers,
                             PlaylistProxyModel *playlistProxyModel) :
    QAbstractItemModel(parent),
    m_totalLength(0),
    m_dbConnectionAvailable(true),
    m_proxyModel(playlistProxyModel)
{
    QVector<QVariant> rootData;
    Q_FOREACH (const QString &header, headers) {
        rootData << header;
    }

    m_rootItem = new PlaylistItem(rootData);
}

PlaylistModel::~PlaylistModel()
{
    delete m_rootItem;
}

int PlaylistModel::columnCount(const QModelIndex & /* parent */) const
{
    return m_rootItem->columnCount();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    QVariant data;

    if (!index.isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return QVariant();
    }

    PlaylistItem *item = getItem(index);

    data = item->data(index.column());

    if (index.column() == PlaylistBrowser::BitrateColumn) {
        return QVariant(data.toString().append(" kbps"));
    }

    return item->data(index.column());
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return 0;
    }

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

PlaylistItem *PlaylistModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        PlaylistItem *item = static_cast<PlaylistItem *>(index.internalPointer());
        if (item) {
            return item;
        }
    }
    return m_rootItem;
}

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return m_rootItem->data(section);
    }

    return QVariant();
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0) {
        return QModelIndex();
    }

    PlaylistItem *parentItem = getItem(parent);

    PlaylistItem *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

bool PlaylistModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    PlaylistItem *parentItem = getItem(parent);
    bool success;

    m_mutex.lock();

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, m_rootItem->columnCount());
    endInsertRows();

    m_mutex.unlock();

    return success;
}

QModelIndex PlaylistModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    PlaylistItem *childItem = getItem(index);
    PlaylistItem *parentItem = childItem->parent();

    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool PlaylistModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    if (rows == 0) {
        return true;
    }

    bool success = true;

    int totalRemoveTime = 0;
    for (int i = position; i < position + rows; i++) {
        QString trackLength = index(i, PlaylistBrowser::LengthColumn, QModelIndex()).data().toString();
        if (trackLength.length() == 5) {
            trackLength.prepend("00:");
        }
        QTime tl(0, 0, 0, 0);
        totalRemoveTime += tl.secsTo(QTime::fromString(trackLength, "hh:mm:ss"));

        if (i == m_stopTrack.row()) {
            m_stopTrack = QModelIndex();
        }

        if (i == m_currentItem.row()) {
            m_currentItem = QModelIndex();
        }
    }

    m_mutex.lock();

    beginRemoveRows(parent, position, position + rows - 1);
    success = m_rootItem->removeChildren(position, rows);
    endRemoveRows();

    m_mutex.unlock();

    if (success) {
        // Decrease total length of the playlist by total length of removed tracks
        m_totalLength -= totalRemoveTime;
        Q_EMIT playlistLengthChanged(m_totalLength, rowCount(QModelIndex()));
    }

    return success;
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    PlaylistItem *parentItem = getItem(parent);

    return parentItem->childCount();
}

bool PlaylistModel::setData(const QModelIndex &index, const QVariant &value,
                            int role)
{
    if (role != Qt::EditRole) {
        return false;
    }

    PlaylistItem *item = getItem(index);

    m_mutex.lock();
    const bool result = item->setData(index.column(), value);
    m_mutex.unlock();

    if (result) {
        Q_EMIT dataChanged(index, index);
    }

    return result;
}

bool PlaylistModel::setHeaderData(int section, Qt::Orientation orientation,
                                  const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal) {
        return false;
    }

    m_mutex.lock();
    const bool result = m_rootItem->setData(section, value);
    m_mutex.unlock();

    if (result) {
        Q_EMIT headerDataChanged(orientation, section, section);
    }

    return result;
}

bool PlaylistModel::insertItem(const Player::MetaData &metadata, int row)
{
    if (row < 0) {
        row = 0;
    }

    // Insert new row
    if (!insertRow(row, QModelIndex())) {
        return false;
    }

    PlaylistItem *child = m_rootItem->child(row);
    child->setData(PlaylistBrowser::FilenameColumn, QVariant(metadata.filename));
    child->setData(PlaylistBrowser::TrackColumn, QVariant(metadata.trackNumber));
    child->setData(PlaylistBrowser::InterpretColumn, QVariant(metadata.artist));
    child->setData(PlaylistBrowser::TracknameColumn, QVariant(metadata.title));
    child->setData(PlaylistBrowser::AlbumColumn, QVariant(metadata.album));
    child->setData(PlaylistBrowser::GenreColumn, QVariant(metadata.genre));
    child->setData(PlaylistBrowser::YearColumn, QVariant(metadata.year));
    child->setData(PlaylistBrowser::LengthColumn, QVariant(metadata.formattedLength));
    // the "kbps" string is appended in PlaylistItemDelegate while drawing
    child->setData(PlaylistBrowser::BitrateColumn, QVariant(QString::number(metadata.bitrate)));

    m_totalLength += (metadata.length / 1000);
    Q_EMIT playlistLengthChanged(m_totalLength, rowCount(QModelIndex()));

    return true;
}

QModelIndex PlaylistModel::currentItem() const
{
    return m_currentItem;
}

void PlaylistModel::setCurrentItem(const QModelIndex &currentIndex)
{
    Q_EMIT layoutAboutToBeChanged();
    m_currentItem = currentIndex;
    Q_EMIT layoutChanged();
}

QModelIndex PlaylistModel::nextItem(const QModelIndex &index) const
{
    if (rowCount() == 0) {
        return QModelIndex();
    }

    if (!index.isValid() && !m_currentItem.isValid()) {
        return this->index(0, 0);
    }

    const QModelIndex current = m_currentItem;
    const QModelIndex mappedIndex = m_proxyModel->mapFromSource(current);
    const QModelIndex sibling = mappedIndex.sibling(mappedIndex.row() + 1, 0);
    if (sibling.isValid()) {
        return m_proxyModel->mapToSource(sibling);
    }

    return QModelIndex();
}

QModelIndex PlaylistModel::previousItem(const QModelIndex &index) const
{
    if (rowCount(QModelIndex()) == 0) {
        return QModelIndex();
    }

    if (!index.isValid() && !m_currentItem.isValid()) {
        return this->index(0, 0);
    }

    const QModelIndex current = m_currentItem;
    const QModelIndex mappedIndex = m_proxyModel->mapFromSource(index);
    const QModelIndex sibling = mappedIndex.sibling(mappedIndex.row() - 1, 0);
    if (sibling.isValid()) {
        return m_proxyModel->mapToSource(sibling);
    }

    return QModelIndex();
}

void PlaylistModel::setStopTrack(const QModelIndex &track)
{
    const QModelIndex mappedTrack = m_proxyModel->mapToSource(track);

    // if the current stop-on-this track is the same as the new one, toggle it
    if (mappedTrack == m_stopTrack) {
        m_stopTrack = QModelIndex();
        return;
    }

    m_stopTrack = mappedTrack;
}

QModelIndex PlaylistModel::getStopTrack() const
{
    return m_stopTrack;
}

void PlaylistModel::clear()
{
    removeRows(0, rowCount(QModelIndex()), QModelIndex());
    m_stopTrack = QModelIndex();
    m_currentItem = QModelIndex();
}
