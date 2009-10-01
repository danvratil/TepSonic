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
    this->_mainParent = mainParent;
    _myParent = parent;
    _myData = data;
    _colCount = colCount;
}

PlaylistItem::~PlaylistItem() {}

int PlaylistItem::rowCount() const
{
    return _playlistList.count();
}

PlaylistItem* PlaylistItem::item(int row)
{
    return _mainParent->_playlistList.at(row);
}

PlaylistItem* PlaylistItem::parent() const
{
    return _myParent;
}

 void PlaylistItem::setPlaylistItem(PlaylistItem* item,PlaylistItem* parent)
 {
    parent->_playlistList << item;
 }

 void PlaylistItem::setList(QList< PlaylistItem*> list)
 {
     _playlistList = list;
 }

 int PlaylistItem::columnCount() const
 {
    return _colCount;
 }

QStringList PlaylistItem::data(PlaylistItem* parent)
{
    QStringList childData;
    for(int i = 0; i < parent->_playlistList.count();i++)
        childData << parent->_myData;
    return childData;
}

QList<PlaylistItem*> PlaylistItem::list(PlaylistItem* parent)
{
    return parent->_playlistList;
}

void PlaylistItem::removeRow(int row)
{
    if ((row >= 0) && (row < _playlistList.size())) {
        _playlistList.removeAt(row);
    }
}
