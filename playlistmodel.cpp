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
#include <QStringList>
#include <QString>
#include <QDebug>
#include <QMap>

PlaylistModel::PlaylistModel(const QStringList data, QObject *parent)
        : QAbstractItemModel(parent)
{
    QStringList rootData = data;
    if(data.count() < 1)
        rootData << "Filename" << "Track" << "Title" << "Artist" << "Album" << "Year" << "Length"; //header
    _rootItem = new PlaylistItem(QString());
}

PlaylistModel::PlaylistModel(QObject *parent)
        : QAbstractItemModel(parent)
{
    QStringList rootData;
    rootData <<"Filename" << "Track" << "Title" << "Artist" << "Album" << "Year" << "Length";
    _rootItem = new PlaylistItem(QString());
}

PlaylistModel::~PlaylistModel()
{
    delete _rootItem;
}


int PlaylistModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<PlaylistItem*>(parent.internalPointer())->columnCount();
    else
        return _rootItem->columnCount();
}



QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    PlaylistItem *item = static_cast<PlaylistItem*>(index.internalPointer());

    return QVariant(item->data(item->parent()));
}


Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    PlaylistItem *parentItem;

    parentItem = _rootItem;

    PlaylistItem *childItem = parentItem->list(_rootItem).at(row)->list(_rootItem->list(_rootItem).at(row)).at(column);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex PlaylistModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    PlaylistItem *childItem = static_cast<PlaylistItem*>(index.internalPointer());
    PlaylistItem *parentItem = childItem->parent();

    if (parentItem == _rootItem)
        return QModelIndex();

    return createIndex(0, 0, parentItem);
}


int PlaylistModel::rowCount(const QModelIndex &parent) const
{

    PlaylistItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = _rootItem;
    else
        parentItem = static_cast<PlaylistItem*>(parent.internalPointer());
    return parentItem->rowCount();
}

void PlaylistModel::setModelData(QList<QStringList> lines)
{
    int i;
    PlaylistItem *item;
    PlaylistItem *subitem;
    for(i = 0; i < lines.count();i++)     //initialize parent group and add it to the rootparent list
    {
        item  = new PlaylistItem(QString("subparent"),lines[i].count(),_rootItem);
        _rootItem->setPlaylistItem(item,_rootItem);
        for(int a = 0; a < lines[i].count();a++)
        {
            subitem = new PlaylistItem(lines[i].at(a),lines[i].count(),_rootItem, item);
            _rootItem->list(_rootItem).at(i)->setPlaylistItem(subitem, _rootItem->list(_rootItem).at(i));
            // Re-index the list
            (void) index(i,a,QModelIndex());
        }
    }
}

PlaylistItem* PlaylistModel::root() const
{
    return _rootItem;
}

 // virtual method!
bool PlaylistModel::setData( const QModelIndex & index, const QVariant & value, int role)
{
     // For nicer gcc output :-)
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
    return 0;    
}

PlaylistItem* PlaylistModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
         PlaylistItem *item = static_cast<PlaylistItem*>(index.internalPointer());
         if (item) return item;
     }
     return _rootItem;
 }

void PlaylistModel::addLines(QList<QStringList>list)
{
    setModelData(list);
}

bool PlaylistModel::removeRows(int row, int count, const QModelIndex &parent) const
{
    PlaylistItem *parentItem;

    if (parent.column()==0) {
        return 0;
    }
    if (!parent.isValid()) {
        parentItem = _rootItem;
    } else {
        parentItem =  static_cast<PlaylistItem*>(parent.internalPointer());
    }

    int oldCount = rowCount(parent);

    for (int i = 0; i < count; i++) {
        /* Keep removing the first row. When the row is removed a following row replaces it. We remove that
           row for count-times, so we remove all required rows */
        parentItem->removeRow(row);
    }

    // If all rows were removed, return true
    if (rowCount(parent)+count == oldCount) {
        return true;
    } else {
        return false;
    }
}
