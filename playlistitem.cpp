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

#include "playlistitem.h"

PlaylistItem::PlaylistItem(const QVector<QVariant> &data, PlaylistItem *parent)
{
     if (data.isEmpty()) {
        itemData.fill(QString(),8);
     } else {
        itemData = data;
     }
     parentItem = parent;
}

PlaylistItem::~PlaylistItem()
{
     qDeleteAll(childItems);
}

PlaylistItem *PlaylistItem::child(int number)
{
     return childItems.value(number);
}

int PlaylistItem::childCount() const
{
     return childItems.count();
}

int PlaylistItem::childNumber() const
{
     if (parentItem)
         return parentItem->childItems.indexOf(const_cast<PlaylistItem*>(this));

     return 0;
}

int PlaylistItem::columnCount() const
{
     return itemData.count();
}

QVariant PlaylistItem::data(int column) const
{
     return itemData.value(column);
}

bool PlaylistItem::insertChildren(int position, int count, int columns)
{
     if (position < 0 || position > childItems.size())
         return false;

     for (int row = 0; row < count; ++row) {
         QVector<QVariant> data(columns);
         PlaylistItem *item = new PlaylistItem(data, this);
         childItems.insert(position, item);
     }

     return true;
}

void PlaylistItem::appendChild(PlaylistItem *item)
{
    childItems.append(item);
}

bool PlaylistItem::insertColumns(int position, int columns)
{
     if (position < 0 || position > itemData.size())
         return false;

     for (int column = 0; column < columns; ++column)
         itemData.insert(position, QVariant());

     foreach (PlaylistItem *child, childItems)
         child->insertColumns(position, columns);

     return true;
}

PlaylistItem *PlaylistItem::parent()
{
     return parentItem;
}

bool PlaylistItem::removeChildren(int position, int count)
{
     if (position < 0 || position + count > childItems.size())
         return false;

     for (int row = 0; row < count; ++row)
         delete childItems.takeAt(position);

     return true;
}

bool PlaylistItem::removeColumns(int position, int columns)
{
     if (position < 0 || position + columns > itemData.size())
         return false;

     for (int column = 0; column < columns; ++column)
         itemData.remove(position,columns);

     foreach (PlaylistItem *child, childItems)
         child->removeColumns(position, columns);

     return true;
}

bool PlaylistItem::setData(int column, const QVariant &value)
{
     if (column < 0 || column >= itemData.size())
         return false;

     itemData[column] = value;
     return true;
}
