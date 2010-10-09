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

#include <QtGui>

#include "playlistmodel.h"
#include "playlistitem.h"
#include "playlistproxymodel.h"
#include "player.h"

#include <QDebug>


PlaylistModel::PlaylistModel(QObject *parent, const QStringList &headers, PlaylistProxyModel *playlistProxyModel)
        : QAbstractItemModel(parent)
{
    QVector<QVariant> rootData;
    foreach (QString header, headers)
        rootData << header;

    _totalLength = 0;
    _dbConnectionAvailable = true;
    _currentItem = QModelIndex();
    _proxyModel = playlistProxyModel;

    rootItem = new PlaylistItem(rootData);
}

PlaylistModel::~PlaylistModel()
{
    delete rootItem;
}

int PlaylistModel::columnCount(const QModelIndex & /* parent */) const
{
    return rootItem->columnCount();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    PlaylistItem *item = getItem(index);

    return item->data(index.column());
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

PlaylistItem *PlaylistModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        PlaylistItem *item = static_cast<PlaylistItem*>(index.internalPointer());
        if (item) return item;
    }
    return rootItem;
}

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    PlaylistItem *parentItem = getItem(parent);

    PlaylistItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

bool PlaylistModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    PlaylistItem *parentItem = getItem(parent);
    bool success;

     _mutex.lock();

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();

     _mutex.unlock();

    return success;
}

QModelIndex PlaylistModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    PlaylistItem *childItem = getItem(index);
    PlaylistItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool PlaylistModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    if (rows == 0) return true;

    bool success = true;
    bool renewCurrentItem = false;

    int totalRemoveTime = 0;
    for (int i=position;i<position+rows;i++) {
        QString trackLength = index(i,7,QModelIndex()).data().toString();
        if (trackLength.length()==5) {
            trackLength.prepend("00:");
        }
        QTime tl(0,0,0,0);
        totalRemoveTime += tl.secsTo(QTime::fromString(trackLength,"hh:mm:ss"));

        if (i == _currentItem.row()) {
            renewCurrentItem = true;
        }
    }

     _mutex.lock();

    beginRemoveRows(parent, position, position + rows - 1);
    success = rootItem->removeChildren(position, rows);
    endRemoveRows();

    if (renewCurrentItem) {
        setCurrentItem(index(position,0,QModelIndex()));
    }

     _mutex.unlock();

    if (success) {
        // Decrease total length of the playlist by total length of removed tracks
        _totalLength -= totalRemoveTime;
        emit playlistLengthChanged(_totalLength,rowCount(QModelIndex()));
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
    if (role != Qt::EditRole)
        return false;

    PlaylistItem *item = getItem(index);

     _mutex.lock();

    bool result = item->setData(index.column(), value);

     _mutex.unlock();

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool PlaylistModel::setHeaderData(int section, Qt::Orientation orientation,
                                  const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

     _mutex.lock();

    bool result = rootItem->setData(section, value);

     _mutex.unlock();

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

bool PlaylistModel::insertItem(Player::MetaData metadata, int row)
{
    if (row < 0) row = 0;

    // Insert new row
    if (!insertRow(row, QModelIndex()))
        return false;

    rootItem->child(row)->setData(0,QVariant(metadata.filename));
    rootItem->child(row)->setData(1,QVariant(metadata.trackNumber));
    rootItem->child(row)->setData(2,QVariant(metadata.artist));
    rootItem->child(row)->setData(3,QVariant(metadata.title));
    rootItem->child(row)->setData(4,QVariant(metadata.album));
    rootItem->child(row)->setData(5,QVariant(metadata.genre));
    rootItem->child(row)->setData(6,QVariant(metadata.year));
    rootItem->child(row)->setData(7,QVariant(metadata.formattedLength));
    rootItem->child(row)->setData(8,QVariant(QString::number(metadata.bitrate)+" kbps"));

    _totalLength += (metadata.length/1000);
    emit playlistLengthChanged(_totalLength, rowCount(QModelIndex()));

    return true;
}

QModelIndex PlaylistModel::currentItem()
{
    if (_currentItem.isValid()) {
        return _currentItem;
    } else {
        return index(0,0,QModelIndex());
    }
}

void PlaylistModel::setCurrentItem(QModelIndex currentIndex)
{
    if (!currentIndex.isValid())
        return;

    emit layoutAboutToBeChanged();
    _currentItem = currentIndex;

    rootItem->child(_currentItem.row())->setSelected(true);
    emit layoutChanged();

}


QModelIndex PlaylistModel::nextItem(QModelIndex index)
{
    if (rowCount(QModelIndex()) == 0) return QModelIndex();

    if (!index.isValid()) {
        index = currentItem();
    }

    // remap the given index from PlaylistModel to PlaylistProxyModel (if possible)
    QModelIndex mappedIndex = _proxyModel->mapFromSource(index);
    if (!mappedIndex.isValid()) mappedIndex = index;

    if (index.row() >= rowCount()) {
        return QModelIndex();
    }

    // Get ModelIndex of the next item in the PlaylistProxyModel
    QModelIndex prevItem =  _proxyModel->index(mappedIndex.row()+1,0,QModelIndex());
    // Remap the result to PlaylistModel
    return _proxyModel->mapToSource(prevItem);
}

QModelIndex PlaylistModel::previousItem(QModelIndex index)
{
    if (rowCount(QModelIndex()) == 0) QModelIndex();

    if (!index.isValid()) {
        index = currentItem();
    }

    // remap the given index from PlaylistModel to PlaylistProxyModel (if possible)
    QModelIndex mappedIndex = _proxyModel->mapFromSource(index);
    if (!mappedIndex.isValid()) mappedIndex = index;

    if (index.row() == 0) {
        return QModelIndex();
    }

    // Get index of the previous item in PlaylistProxyModel
    QModelIndex prevItem =  _proxyModel->index(mappedIndex.row()-1,0,QModelIndex());
    // And remap the result to PlaylistModel
    return _proxyModel->mapToSource(prevItem);

 }
