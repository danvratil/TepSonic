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

#include <QStringList>
#include "collectionitem.h"

CollectionItem::CollectionItem(const QVector<QVariant> &data, CollectionItem *parent)
{
     parentItem = parent;
     itemData = data;
}

CollectionItem::~CollectionItem()
{
     qDeleteAll(childItems);
}

CollectionItem *CollectionItem::child(int number)
{
     return childItems.value(number);
}

int CollectionItem::childCount() const
{
     return childItems.count();
}

int CollectionItem::childNumber() const
{
     if (parentItem)
         return parentItem->childItems.indexOf(const_cast<CollectionItem*>(this));

     return 0;
}

int CollectionItem::columnCount() const
{
     return itemData.count();
}

QVariant CollectionItem::data(int column) const
{
     return itemData.value(column);
}

bool CollectionItem::insertChildren(int position, int count, int columns)
{
     if (position < 0 || position > childItems.size())
         return false;

     for (int row = 0; row < count; ++row) {
         QVector<QVariant> data(columns);
         CollectionItem *item = new CollectionItem(data, this);
         childItems.insert(position, item);
     }

     return true;
}

bool CollectionItem::insertColumns(int position, int columns)
{
     if (position < 0 || position > itemData.size())
         return false;

     for (int column = 0; column < columns; ++column)
         itemData.insert(position, QVariant());

     foreach (CollectionItem *child, childItems)
         child->insertColumns(position, columns);

     return true;
}

CollectionItem *CollectionItem::parent()
{
     return parentItem;
}

bool CollectionItem::removeChildren(int position, int count)
{
     if (position < 0 || position + count > childItems.size())
         return false;

     for (int row = 0; row < count; ++row)
         delete childItems.takeAt(position);

     return true;
}

bool CollectionItem::removeColumns(int position, int columns)
{
     if (position < 0 || position + columns > itemData.size())
         return false;

     for (int column = 0; column < columns; ++column)
         itemData.remove(position);

     foreach (CollectionItem *child, childItems)
         child->removeColumns(position, columns);

     return true;
}

bool CollectionItem::setData(int column, const QVariant &value)
{
     if (column < 0 || column >= itemData.size())
         return false;

     itemData[column] = value;
     return true;
}
