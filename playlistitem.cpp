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

PlaylistItem::PlaylistItem(const QStringList data, PlaylistItem *parent)
{
    myParent = parent;
    myData = data;
}

PlaylistItem::~PlaylistItem() {}

/*void PlaylistItem::appendChild(PlaylistItem *item)
{
    m_childItems.append(item);
}*/

/*PlaylistItem *PlaylistItem::child(int row)
{
    return m_childItems.value(row);
}*/

/*int PlaylistItem::childCount() const
{
    return m_childItems.count();
}*/

int PlaylistItem::columnCount() const
{
    return playlistList.count();
}

PlaylistItem* PlaylistItem::data(int row) const
{
    return playlistList.at(row);
}

PlaylistItem* PlaylistItem::parent() const
{
    return myParent;
}

int PlaylistItem::row() const
{
    if (myParent)
        return playlistList.indexOf(const_cast<PlaylistItem*>(this));
}

 void PlaylistItem::setData(QStringList listOfData)
 {
     myData = listOfData;
 }

 QStringList PlaylistItem::data()
 {
     return myData;
 }

 void PlaylistItem::setPlaylistItem(PlaylistItem* item)
 {
    playlistList << item;
 }

 QList<PlaylistItem*> PlaylistItem::list()
 {
    return playlistList;
 }

 void PlaylistItem::setList(QList<PlaylistItem*> list)
 {
     playlistList = list;
 }
