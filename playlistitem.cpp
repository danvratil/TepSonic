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

#include <QStringList>
#include "playlistitem.h"

PlaylistItem::PlaylistItem(const QList<QVariant> &data, PlaylistItem *parent)
{
    m_parentItem = parent;
    m_itemData = data;
}

PlaylistItem::~PlaylistItem()
{
    qDeleteAll(m_childItems);
}

void PlaylistItem::appendChild(PlaylistItem *item)
{
    m_childItems.append(item);
}

PlaylistItem *PlaylistItem::child(int row)
{
    return m_childItems.value(row);
}

int PlaylistItem::childCount() const
{
    return m_childItems.count();
}

int PlaylistItem::columnCount() const
{
    return m_itemData.count();
}

QVariant PlaylistItem::data(int column) const
{
    return m_itemData.value(column);
}

PlaylistItem *PlaylistItem::parent()
{
    return m_parentItem;
}

int PlaylistItem::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<PlaylistItem*>(this));
}
