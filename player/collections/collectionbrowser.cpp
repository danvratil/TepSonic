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

#include "collectionbrowser.h"
#include "collectionproxymodel.h"
#include "collectionmodel.h"

#include <QDebug>
#include <QList>
#include <QStringList>
#include <QUrl>
#include <QFileInfo>
#include <QDropEvent>

CollectionBrowser::CollectionBrowser(QWidget* parent):
    QTreeView(parent)
{
    setAcceptDrops(false);
    setDragDropMode(DragOnly);

    Q_UNUSED (parent);
}

CollectionBrowser::~CollectionBrowser()
{
}

void CollectionBrowser::startDrag(Qt::DropActions actions)
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

void CollectionBrowser::loadAlbum(const QModelIndex &album, QDataStream &stream) const
{
    const int childrenCount = model()->rowCount(album);
    for (int i = 0; i < childrenCount; ++i) {
        const QModelIndex child = album.child(i, 0);
        stream << child.data(CollectionModel::FilePathRole).toString();
    }
}

void CollectionBrowser::keyPressEvent(QKeyEvent* event)
{
    // When 'delete' pressed, remove selected row from collections
    if (event->matches(QKeySequence::Delete)) {
        model()->removeRow(selectionModel()->currentIndex().row(),
                           selectionModel()->currentIndex().parent());
    }
}

#include "moc_collectionbrowser.cpp"
