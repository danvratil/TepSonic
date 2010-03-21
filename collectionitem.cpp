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

#include "collectionitem.h"

#include <QDebug>

CollectionItem::CollectionItem(const QVector<QVariant> &data, CollectionItem *parent)
{
     _parentItem = parent;
     _itemData = data;
}

CollectionItem::~CollectionItem()
{
     qDeleteAll(_childItems);
}

CollectionItem *CollectionItem::child(int index)
{
     if (index > _childItems.count())
         return NULL;

     return _childItems.value(index);
}

int CollectionItem::childCount() const
{
     return _childItems.count();
}

int CollectionItem::childNumber() const
{
     if (_parentItem)
         return _parentItem->_childItems.indexOf(const_cast<CollectionItem*>(this));

     return 0;
}

int CollectionItem::columnCount() const
{
     return _itemData.count();
}

QVariant CollectionItem::data(int column) const
{
    if ((column < 0) || (column > _itemData.size()))
        return QVariant();

    return _itemData.value(column);
}

bool CollectionItem::insertChildren(int position, int count, int columns)
{
     if (position < 0 || position > _childItems.size())
         return false;

     for (int row = 0; row < count; ++row) {
         QVector<QVariant> data(columns);
         CollectionItem *item = new CollectionItem(data, this);
         _childItems.insert(position, item);
     }

     return true;
}

CollectionItem *CollectionItem::parent()
{
     return _parentItem;
}

bool CollectionItem::removeChildren(int position, int count)
{
     if (position < 0 || position + count > _childItems.size())
         return false;

     for (int row = 0; row < count; ++row)
         delete _childItems.takeAt(position);

     return true;
}

bool CollectionItem::setData(int column, const QVariant &value)
{

     if (column < 0 || column >= _itemData.size())
         return false;

     _itemData[column] = value;
     return true;
}
