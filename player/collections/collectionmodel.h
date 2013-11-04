/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <dan@progdan.cz>
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

#ifndef COLLECTIONMODEL_H
#define COLLECTIONMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QStringList>

class Node;

class CollectionModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    class Private;

    enum NodeType {
        RootNodeType,
        ArtistNodeType,
        AlbumNodeType,
        TrackNodeType
    };

    enum Roles {
        NodeTypeRole = Qt::UserRole,
        ArtistNameRole,
        AlbumNameRole,
        TrackTitleRole,

        AlbumsCountRole,
        TrackCountRole,

        GenreRole,
        DurationRole,
        FilePathRole
    };

    CollectionModel(QObject *parent = 0);
    ~CollectionModel();

    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QStringList getItemChildrenTracks(const QModelIndex &parent);

    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

  public Q_SLOTS:
    void clear();

    // Hack - we call this from delegate to force re-layouting when size of
    // item changes
    void redraw() { Q_EMIT layoutChanged(); }

  protected:
    Qt::DropActions supportedDropActions() const;

  private:
    Private * const d;
    friend class Private;

    Q_PRIVATE_SLOT(d, void __k__onArtistsPopulated());
    Q_PRIVATE_SLOT(d, void __k__onAlbumsPopulated());
    Q_PRIVATE_SLOT(d, void __k__onTracksPopulated());
};


#endif // COLLECTIONMODEL_H
