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

#include "collectionmodel.h"
#include "collectionitem.h"

#include <QtSql/QSqlQuery>

CollectionModel::CollectionModel(const QStringList &headers, QObject *parent)
     : QAbstractItemModel(parent)
{
     QVector<QVariant> rootData;
     foreach (QString header, headers)
         rootData << header;

     rootItem = new CollectionItem(rootData);
     setSupportedDragActions(Qt::CopyAction);
}

CollectionModel::~CollectionModel()
{
     delete rootItem;
}

int CollectionModel::columnCount(const QModelIndex & /* parent */) const
{
     return rootItem->columnCount();
}

QVariant CollectionModel::data(const QModelIndex &index, int role) const
{
     if (!index.isValid())
         return QVariant();

     if (role != Qt::DisplayRole && role != Qt::EditRole)
         return QVariant();

     CollectionItem *item = getItem(index);

     return item->data(index.column());
}

Qt::ItemFlags CollectionModel::flags(const QModelIndex &index) const
{
     if (!index.isValid())
         return 0;

     return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}

CollectionItem *CollectionModel::getItem(const QModelIndex &index) const
{
     if (index.isValid()) {
         CollectionItem *item = static_cast<CollectionItem*>(index.internalPointer());
         if (item) return item;
     }
     return rootItem;
}

QVariant CollectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
     if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
         return rootItem->data(section);

     return QVariant();
}

QModelIndex CollectionModel::index(int row, int column, const QModelIndex &parent) const
{
     if (parent.isValid() && parent.column() != 0)
         return QModelIndex();

     CollectionItem *parentItem = getItem(parent);

     CollectionItem *childItem = parentItem->child(row);
     if (childItem)
         return createIndex(row, column, childItem);
     else
         return QModelIndex();
}

bool CollectionModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
     bool success;

     beginInsertColumns(parent, position, position + columns - 1);
     success = rootItem->insertColumns(position, columns);
     endInsertColumns();

     return success;
}

bool CollectionModel::insertRows(int position, int rows, const QModelIndex &parent)
{
     CollectionItem *parentItem = getItem(parent);
     bool success;

     beginInsertRows(parent, position, position + rows - 1);
     success = parentItem->insertChildren(position, rows, rootItem->columnCount());
     endInsertRows();

     return success;
}

QModelIndex CollectionModel::parent(const QModelIndex &index) const
{
     if (!index.isValid())
         return QModelIndex();

     CollectionItem *childItem = getItem(index);
     CollectionItem *parentItem = childItem->parent();

     if (parentItem == rootItem)
         return QModelIndex();

     return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool CollectionModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
     bool success;

     beginRemoveColumns(parent, position, position + columns - 1);
     success = rootItem->removeColumns(position, columns);
     endRemoveColumns();

     if (rootItem->columnCount() == 0)
         removeRows(0, rowCount());

     return success;
}

bool CollectionModel::removeRows(int position, int rows, const QModelIndex &parent)
{
     CollectionItem *parentItem = getItem(parent);
     bool success = true;

     beginRemoveRows(parent, position, position + rows - 1);
     success = parentItem->removeChildren(position, rows);
     endRemoveRows();

     return success;
}

int CollectionModel::rowCount(const QModelIndex &parent) const
{
     CollectionItem *parentItem = getItem(parent);

     return parentItem->childCount();
}

bool CollectionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
     if (role != Qt::EditRole)
         return false;

     CollectionItem *item = getItem(index);
     bool result = item->setData(index.column(), value);

     if (result)
         emit dataChanged(index, index);

     return result;
}

bool CollectionModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
     if (role != Qt::EditRole || orientation != Qt::Horizontal)
         return false;

     bool result = rootItem->setData(section, value);

     if (result)
         emit headerDataChanged(orientation, section, section);

     return result;
}

QModelIndex CollectionModel::addChild(QModelIndex root, QString title, QString filename)
{
    if (!insertRow(0, root))
        return QModelIndex();

    QModelIndex child;
    // Some title like artist/album/track name
    child = index(0, 0, root);
    setData(child, title);
    // Real file name (in hidden column)
    child = index(0, 1, root);
    setData(child, filename);

    return index(0,0,root);
}


QModelIndex CollectionModel::addRow(QModelIndex root, QString title, QString filename)
{
    if (!insertRow(root.row()+1, root.parent()))
        return QModelIndex();

    QModelIndex child;
    // Some title like artist/album/track name
    child = index(root.row()+1, 0, root.parent());
    setData(child, title, Qt::EditRole);
    // Readl file name (in hidden column)
    child = index(root.row()+1, 1, root.parent());
    setData(child, filename, Qt::EditRole);

    return index(root.row()+1,0,root.parent());
}

Qt::DropActions CollectionModel::supportedDropActions() const
 {
     return Qt::CopyAction | Qt::MoveAction;
 }
