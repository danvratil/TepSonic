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

CollectionItem::CollectionItem(const QVector<QVariant> &data, CollectionItem *parent):
        m_itemData(data),
        m_parentItem(parent)
{
    m_childItems = new QList<CollectionItem*>();
}

CollectionItem::~CollectionItem()
{
    qDeleteAll(m_childItems->begin(),m_childItems->end());
    m_childItems->clear();
    delete m_childItems;
    m_childItems = 0;
}

CollectionItem *CollectionItem::child(int index)
{
    if (index > m_childItems->count())
        return NULL;

    return m_childItems->value(index);
}

int CollectionItem::childCount() const
{
    if (m_childItems == 0) return 0;

    return m_childItems->count();
}

int CollectionItem::childNumber() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems->indexOf(const_cast<CollectionItem*>(this));

    return 0;
}

int CollectionItem::columnCount() const
{
    return m_itemData.count();
}

QVariant CollectionItem::data(int column) const
{
    if ((column < 0) || (column > m_itemData.size()))
        return QVariant();

    return m_itemData.value(column);
}

bool CollectionItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > m_childItems->size())
        return false;

    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        CollectionItem *item = new CollectionItem(data, this);
        m_childItems->insert(position, item);
    }

    return true;
}

CollectionItem *CollectionItem::parent()
{
    return m_parentItem;
}

bool CollectionItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_childItems->size())
        return false;

    for (int row = 0; row < count; ++row)
        delete m_childItems->takeAt(position);

    return true;
}

bool CollectionItem::setData(int column, const QVariant &value)
{

    if (column < 0 || column >= m_itemData.size())
        return false;

    m_itemData[column] = value;
    return true;
}
