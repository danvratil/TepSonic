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

#include "collectionbrowser.h"
#include "collectionproxymodel.h"

#include <QDropEvent>
#include <QDebug>
#include <QList>
#include <QStringList>
#include <QUrl>
#include <QFileInfo>

CollectionBrowser::CollectionBrowser(QWidget* parent):
          QTreeView(parent)
{
    setAcceptDrops(false);
    setDragDropMode(DragOnly);

}

CollectionBrowser::~CollectionBrowser()
{
}

void CollectionBrowser::startDrag(Qt::DropActions)
{
    QModelIndexList indexes = selectedIndexes();

    QString filename;

    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);;

    QModelIndex index;
    for (int i = 0; i < indexes.count(); i++) {
        index = indexes.at(i).sibling(indexes.at(i).row(),1);
        if (index.data().toString().isEmpty()) {
            // Artist(s) is being dragged (child of the dragged item is empty)
            if (index.sibling(index.row(),0).child(0,1).data().toString().isEmpty()) {
                int album = 0;
                QModelIndex artistIndex = index.sibling(index.row(),0);
                QModelIndex albumIndex = artistIndex.child(album,0);
                while (albumIndex.sibling(album,0).isValid()) {
                    QModelIndex trackIndex = albumIndex.sibling(album,0).child(0,0);
                    int row = 0;
                    while (trackIndex.sibling(row,0).isValid()) {
                        stream << trackIndex.sibling(row,1).data().toString();
                        row++;
                    }
                    album++;
                }
            // Album(s) is being dragged
            } else {
                QModelIndex albumIndex = index.sibling(index.row(),0);
                QModelIndex trackIndex = albumIndex.child(0,0);
                int row = 0;
                while (trackIndex.sibling(row,0).isValid()) {
                    stream << trackIndex.sibling(row,1).data().toString();
                    row++;
                }
            }
        // Single track/s is being dragged
        } else {
            stream << index.data().toString();
        }
    }

    mimeData->setData("data/tepsonic-tracks", encodedData);
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);
}


void CollectionBrowser::keyPressEvent(QKeyEvent* event)
{
    // When 'delete' pressed, remove selected row from collections
    if (event->matches(QKeySequence::Delete)) {
        QModelIndex index = selectionModel()->currentIndex();
        static_cast<CollectionProxyModel*>(model())->sourceModel()->removeRow(selectionModel()->currentIndex().row(),selectionModel()->currentIndex().parent());
    }

}




