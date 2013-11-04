/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <dan@progdan.cz>
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

#ifndef COLLECTIONMODEL_P_H
#define COLLECTIONMODEL_P_H

#include "collectionmodel.h"

#include <QSet>

class Node
{
  public:
    typedef QList<Node*> List;

    explicit Node(Node *parent_):
        internalId(-1),
        parentId(-1)
    {
        parent = parent_;
        if (parent) {
            parent->children << this;
        }
    }

    virtual ~Node()
    {
        qDeleteAll(children);
        children.clear();
    }

    void addChild(Node *child)
    {
        child->parent = this;
        children << child;
    }

    virtual QVariant data(int role) const
    {
        if (role == CollectionModel::NodeTypeRole) {
            return CollectionModel::RootNodeType;
        }

        return QVariant();
    }

    int internalId;
    int parentId;

    QList<Node*> children;
    Node *parent;
};

class ArtistNode: public Node
{
  public:
    explicit ArtistNode(Node *parent):
        Node(parent),
        albumsCount(0),
        totalDuration(0)
    {
    }

    virtual ~ArtistNode()
    {
    }

    virtual QVariant data(int role) const
    {
        switch (role) {
            case CollectionModel::NodeTypeRole:
                return CollectionModel::ArtistNodeType;
            case CollectionModel::ArtistNameRole:
                return artistName;
            case CollectionModel::AlbumsCountRole:
                return albumsCount;
            case CollectionModel::DurationRole:
                return totalDuration;
            default:
                return Node::data(role);
        }

        return QVariant();
    }

    QString artistName;
    int albumsCount;
    int totalDuration;
};

class AlbumNode: public Node
{
  public:
    explicit AlbumNode(Node *parent):
        Node(parent),
        tracksCount(0),
        totalDuration(0)
    {
    }

    virtual ~AlbumNode()
    {
    }

    virtual QVariant data(int role) const
    {
        switch (role) {
            case CollectionModel::NodeTypeRole:
                return CollectionModel::AlbumNodeType;
            case CollectionModel::AlbumNameRole:
                return albumName;
            case CollectionModel::TrackCountRole:
                return tracksCount;
            case CollectionModel::DurationRole:
                return totalDuration;
            default:
                return Node::data(role);
        }

        return QVariant();
    }

    QString albumName;
    int tracksCount;
    int totalDuration;
};

class TrackNode: public Node
{
  public:
    explicit TrackNode(Node *parent):
        Node(parent)
    {
    }

    virtual ~TrackNode()
    {
    }

    virtual QVariant data(int role) const
    {
        switch (role) {
            case CollectionModel::NodeTypeRole:
                return CollectionModel::TrackNodeType;
            case CollectionModel::TrackTitleRole:
                return trackTitle;
            case CollectionModel::FilePathRole:
                return filePath;
            case CollectionModel::GenreRole:
                return genre;
            case CollectionModel::DurationRole:
                return duration;
            default:
                return Node::data(role);
        }

        return QVariant();
    }

    QString trackTitle;
    QString filePath;
    QString genre;
    int duration;
};


class CollectionModel::Private
{
  public:
    explicit Private(CollectionModel *parent);
    virtual ~Private();

    QModelIndex indexForNode(Node *node) const;
    Node* getNode(const QModelIndex &index) const;

    void __k__onArtistsPopulated();
    void __k__onAlbumsPopulated();
    void __k__onTracksPopulated();

    /* Runnables */
    void populateArtists();
    Node::List populateArtistsRunnable();
    void populateAlbums(Node *parent);
    Node::List populateAlbumsRunnable(Node *parent);
    void populateTracks(Node *parent);
    Node::List populateTracksRunnable(Node *parent);

    Node *root;
    QHash<int /* internalId */, Node*> artists;
    QHash<int /* internalId */, Node*> albums;
    QSet<Node*> populatedNodes;
    QSet<Node*> pendingNodes;

  private:
    CollectionModel * const q;
};

#endif
