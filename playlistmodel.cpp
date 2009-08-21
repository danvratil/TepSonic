/*
 * TEPSONIC
 * Copyright 2009 Dan Vratil <vratil@progdansoft.com>
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

PlaylistModel::PlaylistModel(const QString &data, QObject *parent)
        : QAbstractItemModel(parent)
{
    QStringList rootData;

    rootData << "Filename" << "Track" << "Title" << "Artist" << "Album" << "Year" << "Length";
    rootItem = new PlaylistItem(rootData);

}

PlaylistModel::~PlaylistModel()
{
    delete rootItem;
}


int PlaylistModel::columnCount(const QModelIndex &parent) const //make it easier
{
    if (parent.isValid())
        return static_cast<PlaylistItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}



QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    PlaylistItem *item = static_cast<PlaylistItem*>(index.internalPointer());

    //return item->data(index.column());
}


/*Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}*/


QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        //return rootItem->data(section);

    return QVariant();
}


QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent)
            const
{
    /*if (!hasIndex(row, column, parent))
        return QModelIndex();

    PlaylistItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<PlaylistItem*>(parent.internalPointer());

    PlaylistItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();*/
}

QModelIndex PlaylistModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    PlaylistItem *childItem = static_cast<PlaylistItem*>(index.internalPointer());
    PlaylistItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}


int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    /*
    PlaylistItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<PlaylistItem*>(parent.internalPointer());
    return parentItem->childCount();
    */
    //return itemList.count();
    return 0;
}


/*void PlaylistModel::setupModelData(const QStringList &lines, PlaylistItem *parent)
{
    QList<PlaylistItem*> parents;
    QList<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;

    while (number < lines.count()) {
        int position = 0;
        while (position < lines[number].length()) {
            if (lines[number].mid(position, 1) != " ")
                break;
            position++;
        }

        QString lineData = lines[number].mid(position).trimmed();

        if (!lineData.isEmpty()) {
            // Read the column data from the rest of the line.
            QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
            QList<QVariant> columnData;
            for (int column = 0; column < columnStrings.count(); ++column)
                columnData << columnStrings[column];

            if (position > indentations.last()) {
                // The last child of the current parent is now the new parent
                // unless the current parent has no children.

                if (parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount()-1);
                    indentations << position;
                }
            } else {
                while (position < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }

            // Append a new item to the current parent's list of children.
            parents.last()->appendChild(new PlaylistItem(columnData, parents.last()));
        }

        number++;
    }
}*/
