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

#include "collectionview.h"
#include "collectionproxymodel.h"
#include "collectionmodel.h"
#include "collectionitemdelegate.h"
#include "taskmanager.h"

#include <QHeaderView>
#include <QDebug>
#include <QList>
#include <QStringList>
#include <QUrl>
#include <QFileInfo>
#include <QDropEvent>
#include <QDrag>
#include <QMimeData>

CollectionView::CollectionView(QWidget* parent):
    QTreeView(parent)
{
    setItemDelegate(new CollectionItemDelegate(this));

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAcceptDrops(false);
    setDragEnabled(true);
    setDragDropMode(DragOnly);
    setDropIndicatorShown(true);
    setAlternatingRowColors(true);
    setRootIsDecorated(true);

    // Hide the last three columns that cotain filename and additional data
    hideColumn(1);
    hideColumn(2);
    hideColumn(3);

    // Hide the header
    header()->setHidden(true);
}

CollectionView::~CollectionView()
{
}

void CollectionView::disableCollections()
{
    QAbstractItemModel *m = model();
    setModel(0);
    m->deleteLater();
}

void CollectionView::enableCollections()
{
    if (!model()) {
        setModel(new CollectionModel(this));
    }
}

void CollectionView::startDrag(Qt::DropActions actions)
{
    Q_UNUSED(actions);

    const QModelIndexList indexes = selectedIndexes();

    QString filename;

    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);;

    Q_FOREACH (const QModelIndex &index, indexes) {
        CollectionModel::NodeType nodeType
            = static_cast<CollectionModel::NodeType>(index.data(CollectionModel::NodeTypeRole).toUInt());
        if (nodeType == CollectionModel::TrackNodeType) {
            stream << index.data(CollectionModel::FilePathRole).toString();
        } else if (nodeType == CollectionModel::AlbumNodeType) {
            loadAlbum(index, stream);
        } else if (nodeType == CollectionModel::ArtistNodeType) {
            const int albumsCount = model()->rowCount(index);
            for (int i = 0; i < albumsCount; ++i) {
                const QModelIndex album = index.child(i, 0);
                loadAlbum(album, stream);
            }
        } else {
            Q_ASSERT(!"Invalid nodeType");
        }
    }

    mimeData->setData(QLatin1String("data/tepsonic-tracks"), encodedData);
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);
}

void CollectionView::loadAlbum(const QModelIndex &album, QDataStream &stream) const
{
    const int childrenCount = model()->rowCount(album);
    for (int i = 0; i < childrenCount; ++i) {
        const QModelIndex child = album.child(i, 0);
        stream << child.data(CollectionModel::FilePathRole).toString();
    }
}

void CollectionView::keyPressEvent(QKeyEvent* event)
{
    // When 'delete' pressed, remove selected row from collections
    if (event->matches(QKeySequence::Delete)) {
        model()->removeRow(selectionModel()->currentIndex().row(),
                           selectionModel()->currentIndex().parent());
    }

    QTreeView::keyPressEvent(event);
}
