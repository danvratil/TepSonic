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


#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QList>
#include <QStringList>
#include <QVariant>
//#include <qDebug>

//TODO: sorting

class PlaylistItem
{
    public:
        PlaylistItem(const QStringList data, PlaylistItem *parent = 0);
        ~PlaylistItem();

        int columnCount() const;
        void setRow(int);
        PlaylistItem* parent() const;
        PlaylistItem* data(int row) const;  //for root Item
        void setData(QStringList);          //set data
        QStringList data();                 //return data
        void setPlaylistItem(PlaylistItem*);//add some item to list in this root pointer
        int row() const;                    //return row of current item
        QList<PlaylistItem*> list();        //return whole list of items
        void setList(QList<PlaylistItem*>);  //set whole list at oneshot

    private:
        QStringList myData;        //data for current item
        QList<PlaylistItem*> playlistList;  //used only by one rootItem (pokud jsem to pochopil dobre)
        PlaylistItem *myParent;     //parent of current pointer
};

#endif // PLAYLISTITEM_H
