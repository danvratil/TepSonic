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
#include "playlistbrowser.h"

#include <QtCore/QDebug>

PlaylistItem::PlaylistItem(const QVector<QVariant> &data, PlaylistItem *parent):
    QStandardItem()
{
    if (data.isEmpty()) {
        m_itemData.fill(QString(), PlaylistBrowser::ColumnsCount);
    } else {
        m_itemData = data;
    }
    m_parentItem = parent;
}

PlaylistItem::~PlaylistItem()
{
    qDeleteAll(m_childItems);
}

PlaylistItem *PlaylistItem::child(int number)
{
    if (number > m_childItems.size()) {
        return 0;
    }

    return m_childItems.value(number);
}

int PlaylistItem::childCount() const
{
    return m_childItems.size();
}

int PlaylistItem::childNumber() const
{
    if (m_parentItem) {
        return m_parentItem->m_childItems.indexOf(const_cast<PlaylistItem *>(this));
    }

    return 0;
}

int PlaylistItem::columnCount() const
{
    return m_itemData.count();
}

QVariant PlaylistItem::data(int column) const
{
    return m_itemData.value(column);
}

bool PlaylistItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > m_childItems.size()) {
        return false;
    }

    for (int row = 0; row < count; ++row) {
        const QVector<QVariant> data(columns);
        PlaylistItem *item = new PlaylistItem(data, this);
        m_childItems.insert(position, item);
    }

    return true;
}

void PlaylistItem::appendChild(PlaylistItem *item)
{
    m_childItems.append(item);
}

PlaylistItem *PlaylistItem::parent()
{
    return m_parentItem;
}

bool PlaylistItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_childItems.size()) {
        return false;
    }

    for (int row = 0; row < count; ++row) {
        delete m_childItems.takeAt(position);
    }

    return true;
}

bool PlaylistItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= m_itemData.size()) {
        return false;
    }

    m_itemData[column] = value;
    return true;
}
