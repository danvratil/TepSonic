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

void CollectionModel::Private::__k__onArtistsPopulated()
{
    // Can't use qobject_cast on templated classes
    QFutureWatcher<Node::List> *artistsWatcher = static_cast<QFutureWatcher<Node::List>*>(q->sender());
    Node::List nodes = artistsWatcher->result();
    qDebug() << "Fetched" << nodes.count() << "artists from collections";
    q->beginInsertRows(QModelIndex(), 0, nodes.count() - 1);
    Q_FOREACH (Node *node, nodes) {
        root->addChild(node);
        artists.insert(node->internalId, node);
    }
    q->endInsertRows();

    artistsWatcher->deleteLater();

    QFutureWatcher<Node::List> *watcher = new QFutureWatcher<Node::List>();
    QFuture<Node::List> future = QtConcurrent::run<Node::List>(this, &CollectionModel::Private::populateAlbums);
    watcher->setFuture(future);
    q->connect(watcher, SIGNAL(finished()),
               q, SLOT(__k__onAlbumsPopulated()));
}

void CollectionModel::Private::__k__onAlbumsPopulated()
{
    QFutureWatcher<Node::List> *albumsWatcher = static_cast<QFutureWatcher<Node::List>*>(q->sender());
    Node::List nodes = albumsWatcher->result();
    qDebug() << "Fetched" << nodes.count() << "albums from collections";

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

    QFutureWatcher<Node::List> *watcher = new QFutureWatcher<Node::List>();
    QFuture<Node::List> future = QtConcurrent::run<Node::List>(this, &CollectionModel::Private::populateTracks);
    watcher->setFuture(future);
    q->connect(watcher, SIGNAL(finished()),
               q, SLOT(__k__onTracksPopulated()));
}

void CollectionModel::Private::__k__onTracksPopulated()
{
    QFutureWatcher<Node::List> *tracksWatcher = static_cast<QFutureWatcher<Node::List>*>(q->sender());
    Node::List nodes = tracksWatcher->result();
    qDebug() << "Fetched" << nodes.count() << "tracks from collections";

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

QList<Node*> CollectionModel::Private::populateArtists()
{
    DatabaseManager dbMgr(QLatin1String("collectionModel"));
    QSqlDatabase db = dbMgr.sqlDb();

    QSqlQuery vaQuery("SELECT COUNT(album) AS albumsCnt, SUM(totalLength) AS totalLength "
                      "FROM view_various_artists", db);
    vaQuery.next();

    Node::List nodes;

    ArtistNode *vaNode = new ArtistNode(0);
    vaNode->internalId = 0;
    vaNode->albumsCount = vaQuery.value(0).toInt();
    vaNode->totalDuration = vaQuery.value(1).toInt();
    vaNode->artistName = tr("Various Artists");
    nodes << vaNode;

    QSqlQuery query("SELECT id, interpret, albumsCnt, totalLength "
                    "FROM interprets "
                    "WHERE id NOT IN (SELECT interpret FROM tracks "
                    "                 WHERE album IN (SELECT album "
                    "                                 FROM view_various_artists) "
                    "                 ) "
                    "ORDER BY interpret ASC", db);
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

QList<Node*> CollectionModel::Private::populateAlbums()
{
    DatabaseManager dbMgr(QLatin1String("collectionModel"));
    QSqlDatabase db = dbMgr.sqlDb();

    Node::List nodes;
    QSqlQuery vaQuery("SELECT view_various_artists.album AS id, "
                      "       albums.album, "
                      "       view_various_artists.tracksCnt, "
                      "       view_various_artists.totalLength "
                      "FROM view_various_artists "
                      "LEFT JOIN albums ON view_various_artists.album = albums.id "
                      "ORDER BY albums.album ASC", db);
    while (vaQuery.next()) {
        AlbumNode *albumNode = new AlbumNode(0);
        albumNode->internalId = vaQuery.value(0).toInt();
        albumNode->parentId = 0;
        albumNode->albumName = vaQuery.value(1).toString();
        albumNode->tracksCount = vaQuery.value(2).toInt();
        albumNode->tracksCount = vaQuery.value(3).toInt();
        nodes << albumNode;
    }

    QSqlQuery query("SELECT DISTINCT albums.id, "
                    "       tracks.interpret, "
                    "       albums.album, "
                    "       albums.tracksCnt, "
                    "       albums.totalLength "
                    "FROM tracks "
                    "LEFT JOIN albums ON (tracks.album = albums.id) "
                    "WHERE albums.id NOT IN (SELECT album FROM view_various_artists) "
                    "ORDER BY albums.album ASC", db);
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

QList<Node*> CollectionModel::Private::populateTracks()
{
    DatabaseManager dbMgr(QLatin1String("collectionModel"));
    QSqlDatabase db = dbMgr.sqlDb();

    Node::List nodes;

    QSqlQuery query("SELECT id, albumID, trackname, filename, genre, length "
                    "FROM view_tracks "
                    "ORDER BY track ASC", db);
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

CollectionModel::CollectionModel(QObject *parent):
    QAbstractItemModel(parent),
    d(new Private(this))
{
    setSupportedDragActions(Qt::CopyAction);

    QFutureWatcher<Node::List> *watcher = new QFutureWatcher<Node::List>();
    QFuture<Node::List> future = QtConcurrent::run(d, &CollectionModel::Private::populateArtists);
    watcher->setFuture(future);
    connect(watcher, SIGNAL(finished()),
            this, SLOT(__k__onArtistsPopulated()));
}

CollectionModel::~CollectionModel()
{
    delete d;
}

int CollectionModel::columnCount(const QModelIndex & /* parent */) const
{
    return 1;
}

int CollectionModel::rowCount(const QModelIndex &parent) const
{
    Node *node = d->getNode(parent);
    return node->children.count();
}

QVariant CollectionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Node *node = d->getNode(index);
    return node->data(role);
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
    Node *childItem = parentNode->children.at(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

QModelIndex CollectionModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    Node *node = d->getNode(index);
    Node *parentNode = node->parent;

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
