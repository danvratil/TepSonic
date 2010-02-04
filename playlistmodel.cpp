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

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>

#include "playlistmodel.h"
#include "playlistitem.h"


PlaylistModel::PlaylistModel(const QStringList &headers, QObject *parent)
     : QAbstractItemModel(parent)
{
     QVector<QVariant> rootData;
     foreach (QString header, headers)
         rootData << header;

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

void PlaylistModel::addItem(QString file)
{
    if (file.isEmpty())
        return;

    // Select the root item
    QModelIndex root;

    // Insert new row
    if (!insertRow(root.row()+1, root.parent()))
        return;

    /**
     * TAGLIB comes here
     */
    TagLib::FileRef f(file.toUtf8());
    int truckNumber = f.tag()->track();
    TagLib::String artist = f.tag()->artist();
    TagLib::String title = f.tag()->title();
    TagLib::String album = f.tag()->album();
    TagLib::String genre = f.tag()->genre();
    int year = f.tag()->year();
    int totalTimeNum = f.audioProperties()->length();
    QString hours,mins,secs;
    int iHours, iMins, iSecs;
    // Only if time is longed then 1 hour the hours will be prepended to the time
    if (totalTimeNum>3600) {
        iHours = totalTimeNum/3600;
    } else {
        iHours = 0;
    }
    iMins = (totalTimeNum - iHours*3600)/60;
    iSecs = totalTimeNum - iHours*3600 - iMins*60;
    if (iHours>0) {
        hours = QString::number(iHours).append(":");
        if (iHours<10) {
            hours.prepend("0");
        }
    } else {
        hours = "";
    }
    mins = QString::number(iMins).append(":");
    if (iMins<10) {
        mins.prepend("0");
    }
    secs = QString::number(iSecs);
    if (iSecs<10) {
        secs.prepend("0");
    }

    // Child item
    QModelIndex child;
    // Store the filename into the first column. The other columns will be filled by separated thread
    child = index(root.row()+1, 0, root.parent());
    setData(child, QVariant(file), Qt::EditRole);
    // Track number
    child = index(root.row()+1, 1, root.parent());
    setData(child, QVariant(truckNumber), Qt::EditRole);
    // Interpret
    child = index(root.row()+1, 2, root.parent());
    setData(child, QVariant(QString(artist.toCString(true))), Qt::EditRole);
    // Track title
    child = index(root.row()+1, 3, root.parent());
    setData(child, QVariant(QString(title.toCString(true))), Qt::EditRole);
    // Album
    child = index(root.row()+1, 4, root.parent());
    setData(child, QVariant(QString(album.toCString(true))), Qt::EditRole);
    // Genre
    child = index(root.row()+1, 5, root.parent());
    setData(child, QVariant(QString(genre.toCString(true))), Qt::EditRole);
    // Year
    child = index(root.row()+1, 6, root.parent());
    setData(child, QVariant(year), Qt::EditRole);
    // Total length
    child = index(root.row()+1, 7, root.parent());
    setData(child, QVariant(hours.append(mins).append(secs)), Qt::EditRole);
}

void PlaylistModel::removeItem(int index)
{
    removeRow(index,QModelIndex());
}

void PlaylistModel::removeItems(int first, int count)
{
    removeRows(first,count,QModelIndex());
}
