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

#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QList>

#include "playlistitem.h"

class PlaylistItem;

class PlaylistModel : public QAbstractItemModel
{
    Q_OBJECT
    public:
         //overloading constructor!
        PlaylistModel(QObject *parent = 0);
        PlaylistModel(const QStringList data, QObject *parent = 0);
        ~PlaylistModel();

        QVariant data(const QModelIndex &index, int role) const;            //virtual
        QModelIndex index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const; //virtual
        QModelIndex parent(const QModelIndex &index) const;
        void setModelData( QList<QStringList>);
        int rowCount(const QModelIndex &parent = QModelIndex()) const;      //virtual
        int columnCount(const QModelIndex &parent = QModelIndex()) const;   //virtual
        Qt::ItemFlags flags(const QModelIndex &index) const;
        PlaylistItem* root() const;
        bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
        PlaylistItem* getItem(const QModelIndex &index) const;

    public slots:
        void addLines(QList<QStringList>);

    private:
        PlaylistItem* m_rootItem;
};

#endif // PLAYLISTMODEL_H
