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

#include "collectionmodel.h"
#include "collectionmodel_p.h"
#include "databasemanager.h"

#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QFuture>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

Q_DECLARE_METATYPE(Node*)

CollectionModel::Private::Private(CollectionModel *parent):
    root(new Node(0)),
    q(parent)
{
}

CollectionModel::Private::~Private()
{
    delete root;
}

Node* CollectionModel::Private::getNode(const QModelIndex &index) const
{
    if (index.isValid()) {
        Node *node = static_cast<Node*>(index.internalPointer());
        Q_ASSERT(node);
        return node;
    }

    return root;
}

QModelIndex CollectionModel::Private::indexForNode(Node *node) const
{
    Node *parent = node->parent;
    return q->createIndex(parent->children.indexOf(node), 0, node);
}

void CollectionModel::Private::populateArtists()
{
    pendingNodes.insert(root);
    QFutureWatcher<Node::List> *watcher = new QFutureWatcher<Node::List>();
    QFuture<Node::List> future = QtConcurrent::run<Node::List>(this, &CollectionModel::Private::populateArtistsRunnable);
    watcher->setFuture(future);
    connect(watcher, SIGNAL(finished()), q, SLOT(__k__onArtistsPopulated()));
}

Node::List CollectionModel::Private::populateArtistsRunnable()
{
    /*
    QSqlQuery vaQuery("SELECT COUNT(album) AS albumsCnt, SUM(totalLength) AS totalLength "
                      "FROM view_various_artists",
                      DatabaseManager::instance()->sqlDb());
    vaQuery.next();
    */

    Node::List nodes;

    /*
    ArtistNode *vaNode = new ArtistNode(0);
    vaNode->internalId = 0;
    vaNode->albumsCount = vaQuery.value(0).toInt();
    vaNode->totalDuration = vaQuery.value(1).toInt();
    vaNode->artistName = tr("Various Artists");
    nodes << vaNode;
    */

    QSqlQuery query(QLatin1String(
                    "SELECT id, interpret, albumsCnt, totalLength "
                    "FROM interprets "
      /*              "WHERE id NOT IN (SELECT interpret FROM tracks "
                    "                 WHERE album IN (SELECT album "
                    "                                 FROM view_various_artists) "
                    "                 ) "*/
                    "ORDER BY interpret ASC"),
                    DatabaseManager::instance()->sqlDb());

    while (query.next()) {
        ArtistNode *node = new ArtistNode(0);
        node->internalId = query.value(0).toInt();
        node->artistName = query.value(1).toString();
        node->albumsCount = query.value(2).toInt();
        node->totalDuration = query.value(3).toInt();
        nodes << node;
    }

    return nodes;
}

void CollectionModel::Private::__k__onArtistsPopulated()
{
    // Can't use qobject_cast on templated classes
    QFutureWatcher<Node::List> *artistsWatcher = static_cast<QFutureWatcher<Node::List>*>(q->sender());
    Node::List nodes = artistsWatcher->result();
    qDebug() << "Fetched" << nodes.count() << "artists from collections";

    populatedNodes.insert(root);
    pendingNodes.remove(root);

    q->beginInsertRows(QModelIndex(), 0, nodes.count() - 1);
    Q_FOREACH (Node *node, nodes) {
        root->addChild(node);
        artists.insert(node->internalId, node);
    }
    q->endInsertRows();

    artistsWatcher->deleteLater();

}

void CollectionModel::Private::populateAlbums(Node *parentNode)
{
    pendingNodes.insert(parentNode);

    QFutureWatcher<Node::List> *watcher = new QFutureWatcher<Node::List>();
    watcher->setProperty("ParentNode", QVariant::fromValue(parentNode));
    QFuture<Node::List> future = QtConcurrent::run<Node::List>(this, &CollectionModel::Private::populateAlbumsRunnable, parentNode);
    watcher->setFuture(future);
    connect(watcher, SIGNAL(finished()), q, SLOT(__k__onAlbumsPopulated()));
}

Node::List CollectionModel::Private::populateAlbumsRunnable(Node *parentNode)
{
    Node::List nodes;
    /*
    QSqlQuery vaQuery("SELECT view_various_artists.album AS id, "
                      "       albums.album, "
                      "       view_various_artists.tracksCnt, "
                      "       view_various_artists.totalLength "
                      "FROM view_various_artists "
                      "LEFT JOIN albums ON view_various_artists.album = albums.id "
                      "ORDER BY albums.album ASC",
                      DatabaseManager::instance()->sqlDb());

    while (vaQuery.next()) {
        AlbumNode *albumNode = new AlbumNode(0);
        albumNode->internalId = vaQuery.value(0).toInt();
        albumNode->parentId = 0;
        albumNode->albumName = vaQuery.value(1).toString();
        albumNode->tracksCount = vaQuery.value(2).toInt();
        albumNode->tracksCount = vaQuery.value(3).toInt();
        nodes << albumNode;
    }
    */

    QSqlQuery query(DatabaseManager::instance()->sqlDb());
    query.prepare(QLatin1String(
                  "SELECT DISTINCT albums.id, "
                  "       tracks.interpret, "
                  "       albums.album, "
                  "       albums.tracksCnt, "
                  "       albums.totalLength "
                  "FROM tracks "
                  "LEFT JOIN albums ON (tracks.album = albums.id) "
                  "WHERE albums.id NOT IN (SELECT album FROM view_various_artists) AND tracks.interpret = ? "
                  "ORDER BY albums.album ASC"));
    query.addBindValue(parentNode->internalId);
    query.exec();
    while (query.next()) {
        AlbumNode *albumNode = new AlbumNode(0);
        albumNode->internalId = query.value(0).toInt();
        albumNode->parentId = query.value(1).toInt();
        albumNode->albumName = query.value(2).toString();
        albumNode->tracksCount = query.value(3).toInt();
        albumNode->totalDuration = query.value(4).toInt();
        nodes << albumNode;
    }

    return nodes;
}

void CollectionModel::Private::__k__onAlbumsPopulated()
{
    QFutureWatcher<Node::List> *albumsWatcher = static_cast<QFutureWatcher<Node::List>*>(q->sender());
    Node *parentNode = albumsWatcher->property("ParentNode").value<Node*>();
    Node::List nodes = albumsWatcher->result();
    qDebug() << "Fetched" << nodes.count() << "albums for artist" << parentNode->internalId;

    populatedNodes.insert(parentNode);
    pendingNodes.remove(parentNode);

    Q_FOREACH (Node *node, nodes) {
        Node *artist = artists.value(node->parentId);
        if (!artist) {
            qDebug() << "Failed to find artist" << node->parentId << "for album"
                     << node->internalId << "(" << static_cast<AlbumNode*>(node)->albumName << ")";
            // FIXME: We need to change the database structure in order to correctly
            // support various artists etc.
            //Q_ASSERT(artist);
            continue;
        }
        const int count = artist->children.count();
        q->beginInsertRows(indexForNode(artist), count, count + 1);
        artist->addChild(node);
        albums.insert(node->internalId, node);
        q->endInsertRows();
    }

    albumsWatcher->deleteLater();
}

void CollectionModel::Private::populateTracks(Node *parentNode)
{
    pendingNodes.insert(parentNode);

    QFutureWatcher<Node::List> *watcher = new QFutureWatcher<Node::List>();
    watcher->setProperty("ParentNode", QVariant::fromValue(parentNode));
    QFuture<Node::List> future = QtConcurrent::run<Node::List>(this, &CollectionModel::Private::populateTracksRunnable, parentNode);
    watcher->setFuture(future);
    connect(watcher, SIGNAL(finished()), q, SLOT(__k__onAlbumsPopulated()));
}

QList<Node*> CollectionModel::Private::populateTracksRunnable(Node *parentNode)
{
    Node::List nodes;

    QSqlQuery query(DatabaseManager::instance()->sqlDb());
    query.prepare(QLatin1String(
                  "SELECT id, albumID, trackname, filename, genre, length "
                  "FROM view_tracks "
                  "WHERE albumId = ? "
                  "ORDER BY track ASC"));
    query.addBindValue(parentNode->internalId);
    query.exec();
    while (query.next()) {
        TrackNode *trackNode = new TrackNode(0);
        trackNode->internalId = query.value(0).toInt();
        trackNode->parentId = query.value(1).toInt();
        trackNode->trackTitle = query.value(2).toString();
        trackNode->filePath = query.value(3).toString();
        trackNode->genre = query.value(4).toString();
        trackNode->duration = query.value(5).toInt();
        nodes << trackNode;
    }

    return nodes;
}

void CollectionModel::Private::__k__onTracksPopulated()
{
    QFutureWatcher<Node::List> *tracksWatcher = static_cast<QFutureWatcher<Node::List>*>(q->sender());
    Node *parentNode = tracksWatcher->property("ParentNode").value<Node*>();
    Node::List nodes = tracksWatcher->result();
    qDebug() << "Fetched" << nodes.count() << "tracks for album" << parentNode->internalId;

    populatedNodes.insert(parentNode);
    pendingNodes.remove(parentNode);

    Q_FOREACH (Node *node, nodes) {
        Node *album = albums.value(node->parentId);
        if (!album) {
            qDebug() << "Failed to find album" << node->parentId << "for track"
                     << node->internalId << "(" << static_cast<TrackNode*>(node)->trackTitle << ")";
            //Q_ASSERT(album);
            continue;
        }
        const int count = album->children.count();
        q->beginInsertRows(indexForNode(album), count, count + 1);
        album->addChild(node);
        q->endInsertRows();
    }

    tracksWatcher->deleteLater();
}


CollectionModel::CollectionModel(QObject *parent):
    QAbstractItemModel(parent),
    d(new Private(this))
{
    setSupportedDragActions(Qt::CopyAction);
}

CollectionModel::~CollectionModel()
{
    delete d;
}

int CollectionModel::columnCount(const QModelIndex & /* parent */) const
{
    return 1;
}

bool CollectionModel::canFetchMore(const QModelIndex &parent) const
{
    Node *parentNode = d->getNode(parent);
    NodeType type = static_cast<NodeType>(parentNode->data(NodeTypeRole).toInt());
    if (type == TrackNodeType) {
        return false;
    }

    if (d->populatedNodes.contains(parentNode)) {
        return false;
    }

    return true;
}

void CollectionModel::fetchMore(const QModelIndex &parent)
{
    Node *parentNode = d->getNode(parent);
    NodeType type = static_cast<NodeType>(parentNode->data(NodeTypeRole).toInt());
    switch (type) {
        case CollectionModel::RootNodeType:
            d->populateArtists();
            break;
        case CollectionModel::ArtistNodeType:
            d->populateAlbums(parentNode);
            break;
        case CollectionModel::AlbumNodeType:
            d->populateTracks(parentNode);
            break;
        default:
            break;
    }
}

int CollectionModel::rowCount(const QModelIndex &parent) const
{
    Node *parentNode = d->getNode(parent);
    if (d->populatedNodes.contains(parentNode)) {
        return parentNode->children.count();
    } else {
        NodeType type = static_cast<NodeType>(parentNode->data(NodeTypeRole).toInt());
        switch (type) {
            case CollectionModel::ArtistNodeType:
                return static_cast<ArtistNode*>(parentNode)->albumsCount;
            case CollectionModel::AlbumNodeType:
                return static_cast<AlbumNode*>(parentNode)->tracksCount;
            default:
                return 0;
        }
    }
}

QVariant CollectionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Node *node = d->getNode(index);
    if (!d->populatedNodes.contains(node)) {
        if (role == Qt::DisplayRole) {
            return tr("Fetching data...");
        } else if (role == CollectionModel::NodeTypeRole) {
            NodeType parentType = static_cast<NodeType>(node->data(NodeTypeRole).toInt());
            switch (parentType) {
                case RootNodeType:
                    return ArtistNodeType;
                case ArtistNodeType:
                    return AlbumNodeType;
                case AlbumNodeType:
                    return TrackNodeType;
                case TrackNodeType:
                    return QVariant(); // should not happen
            }
        }

        return QVariant();
    } else {
        return node->data(role);
    }
}

Qt::ItemFlags CollectionModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return 0;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}

QModelIndex CollectionModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0) {
        return QModelIndex();
    }

    Node *parentNode = d->getNode(parent);
    if (d->populatedNodes.contains(parentNode)) {
        Node *childItem = parentNode->children.at(row);
        if (childItem) {
            return createIndex(row, column, childItem);
        } else {
            // Q_ASSERT?
            return QModelIndex();
        }
    } else {
        return createIndex(row, column, parentNode);
    }
}

QModelIndex CollectionModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    Node *node = d->getNode(index);
    Node *parentNode;
    if (!d->populatedNodes.contains(node)) {
        parentNode = node;
    } else {
        parentNode = node->parent;
    }

    if (parentNode == d->root) {
        return QModelIndex();
    }

    Node *parent2Node = parentNode->parent;
    return createIndex(parent2Node->children.indexOf(parentNode), 0, parentNode);
}

Qt::DropActions CollectionModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

void CollectionModel::clear()
{
    beginResetModel();
    delete d->root;
    d->root = new Node(0);
    endResetModel();
}

QStringList CollectionModel::getItemChildrenTracks(const QModelIndex &parent)
{
    QStringList result;

    Node *node = d->getNode(parent);

    for (int i = 0; i < node->children.count(); i++)
    {
        if (node->data(NodeTypeRole).toUInt() != TrackNodeType) {
            result << getItemChildrenTracks(index(i, 0, parent));
        } else {
            result << node->data(FilePathRole).toString();
        }
    }

    return result;
}

#include "moc_collectionmodel.cpp"
