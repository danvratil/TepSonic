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


PlaylistModel::PlaylistModel(const QStringList &headers, const QString &data,
                      QObject *parent)
     : QAbstractItemModel(parent)
{
     QVector<QVariant> rootData;
     foreach (QString header, headers)
         rootData << header;

     rootItem = new PlaylistItem(rootData);
     setupModelData(data.split(QString("\n")), rootItem);
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

bool PlaylistModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
     bool success;

     beginInsertColumns(parent, position, position + columns - 1);
     success = rootItem->insertColumns(position, columns);
     endInsertColumns();

     return success;
}

bool PlaylistModel::insertRows(int position, int rows, const QModelIndex &parent)
{
     PlaylistItem *parentItem = getItem(parent);
     bool success;

     beginInsertRows(parent, position, position + rows - 1);
     success = parentItem->insertChildren(position, rows, rootItem->columnCount());
     endInsertRows();

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

bool PlaylistModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
     bool success;

     beginRemoveColumns(parent, position, position + columns - 1);
     success = rootItem->removeColumns(position, columns);
     endRemoveColumns();

     if (rootItem->columnCount() == 0)
         removeRows(0, rowCount());

     return success;
 }

bool PlaylistModel::removeRows(int position, int rows, const QModelIndex &parent)
{
     PlaylistItem *parentItem = getItem(parent);
     bool success = true;

     beginRemoveRows(parent, position, position + rows - 1);
     success = parentItem->removeChildren(position, rows);
     endRemoveRows();

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
     bool result = item->setData(index.column(), value);

     if (result)
         emit dataChanged(index, index);

     return result;
}

bool PlaylistModel::setHeaderData(int section, Qt::Orientation orientation,
                               const QVariant &value, int role)
{
     if (role != Qt::EditRole || orientation != Qt::Horizontal)
         return false;

     bool result = rootItem->setData(section, value);

     if (result)
         emit headerDataChanged(orientation, section, section);

     return result;
}
