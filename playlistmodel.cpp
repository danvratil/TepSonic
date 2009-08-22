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

#include <QtGui>

#include "playlistmodel.h"
#include "playlistitem.h"
#include <QStringList>
#include <QString>
#include <QDebug>
#include <QMap>

PlaylistModel::PlaylistModel(const QStringList data, QObject *parent)
        : QAbstractItemModel(parent)
{
    QStringList rootData = data;
    if(data.count() < 1)
        rootData << "Filename" << "Track" << "Title" << "Artist" << "Album" << "Year" << "Length"; //header
    m_rootItem = new PlaylistItem(QString());
}

PlaylistModel::PlaylistModel(QObject *parent)
        : QAbstractItemModel(parent)
{
    QStringList rootData;
    rootData <<"Filename" << "Track" << "Title" << "Artist" << "Album" << "Year" << "Length";
    m_rootItem = new PlaylistItem(QString());
}

PlaylistModel::~PlaylistModel()
{
    delete m_rootItem;
}


int PlaylistModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<PlaylistItem*>(parent.internalPointer())->columnCount();
    else
        return m_rootItem->columnCount();
}



QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    PlaylistItem *item = static_cast<PlaylistItem*>(index.internalPointer());

    return QVariant(item->data(item->parent()));
}


Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    PlaylistItem *parentItem;

    parentItem = m_rootItem;

    PlaylistItem *childItem = parentItem->list(m_rootItem).at(row)->list(m_rootItem->list(m_rootItem).at(row)).at(column);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex PlaylistModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    PlaylistItem *childItem = static_cast<PlaylistItem*>(index.internalPointer());
    PlaylistItem *parentItem = childItem->parent();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(0, 0, parentItem);
}


int PlaylistModel::rowCount(const QModelIndex &parent) const
{

    PlaylistItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<PlaylistItem*>(parent.internalPointer());
    return parentItem->rowCount();
}

void PlaylistModel::setModelData(QList<QStringList> lines)
{
    int i;
    for(i = 0; i < lines.count();i++)     //initialize parent group and add it to the rootparent list
        m_rootItem->setPlaylistItem(new PlaylistItem(QString(),lines[i].count(),m_rootItem),m_rootItem);

    for(i = 0; i < lines.count();i++)
        for(int a = 0; a < lines[i].count();i++)
        {
            PlaylistItem *item = new PlaylistItem(lines.at(i).at(a),lines[i].count(),m_rootItem, m_rootItem->item(i));
            m_rootItem->list(m_rootItem).at(i)->setPlaylistItem(item, m_rootItem->list(m_rootItem).at(i));
            (void) index(i,a,QModelIndex());
        }
}

PlaylistItem* PlaylistModel::root() const
{
    return m_rootItem;
}

 // virtual method!
bool PlaylistModel::setData( const QModelIndex & index, const QVariant & value, int role)
{
     // For nicer gcc output :-)
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
    return 0;    
}

PlaylistItem* PlaylistModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
         PlaylistItem *item = static_cast<PlaylistItem*>(index.internalPointer());
         if (item) return item;
     }
     return m_rootItem;
     qDebug() << "m_rootItem returned";
 }

void PlaylistModel::addLines(QList<QStringList>list)
{
    setModelData(list);
}
