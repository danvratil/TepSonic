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


#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QList>
#include <QStringList>
#include <QVariant>

//TODO: sorting

class PlaylistItem
{
    public:
        PlaylistItem(const QString data, int colCount = 0 , PlaylistItem * mainParent = 0,PlaylistItem *parent = 0);
        ~PlaylistItem();

        int rowCount() const;
        int columnCount() const;
        void setRow(int);
        PlaylistItem* parent() const;
         //for root Item
        PlaylistItem* item(int row);
         //add some item to list in this root pointer
        void setPlaylistItem(PlaylistItem* item,PlaylistItem* parent);
         //return whole list of items
        QList<PlaylistItem*> list(PlaylistItem* parent);
         //set whole list at one shot
        void setList(QList<PlaylistItem*>);
        QStringList data(PlaylistItem* parent);

    private:
         //data for current item
        QString m_myData;
          //used only by one rootItem (pokud jsem to pochopil dobre)
        QList< PlaylistItem* > m_playlistList;
         //parent of current pointer
        PlaylistItem *m_myParent;
        PlaylistItem* m_mainParent;
        int m_colCount;
};

#endif // PLAYLISTITEM_H
