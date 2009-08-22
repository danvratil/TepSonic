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
#include "playlistitem.h"

PlaylistItem::PlaylistItem(const QString data, int colCount, PlaylistItem * mainParent, PlaylistItem *parent)
{
    this->m_mainParent = mainParent;
    m_myParent = parent;
    m_myData = data;
    m_colCount = colCount;
}

PlaylistItem::~PlaylistItem() {}

int PlaylistItem::rowCount() const
{
    return m_playlistList.count();
}

PlaylistItem* PlaylistItem::item(int row)
{
    return m_mainParent->m_playlistList.at(row);
}

PlaylistItem* PlaylistItem::parent() const
{
    return m_myParent;
}

 void PlaylistItem::setPlaylistItem(PlaylistItem* item,PlaylistItem* parent)
 {
    parent->m_playlistList << item;
 }

 void PlaylistItem::setList(QList< PlaylistItem*> list)
 {
     m_playlistList = list;
 }

 int PlaylistItem::columnCount() const
 {
    return m_colCount;
 }

QStringList PlaylistItem::data(PlaylistItem* parent)
{
    QStringList childData;
    for(int i = 0; i < parent->m_playlistList.count();i++)
        childData << parent->m_myData;
    return childData;
}

QList<PlaylistItem*> PlaylistItem::list(PlaylistItem* parent)
{
    return parent->m_playlistList;
}
