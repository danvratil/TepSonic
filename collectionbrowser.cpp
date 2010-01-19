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


#include <QDropEvent>
#include <QDebug>
#include <QList>
#include <QStringList>
#include <QUrl>
#include <QFileInfo>

CollectionBrowser::CollectionBrowser(QWidget* parent):
          QTreeView(parent)
{
    setAcceptDrops(true);
}

CollectionBrowser::~CollectionBrowser()
{
}

// Event: item is dragged over the widget
void CollectionBrowser::dragEnterEvent(QDragEnterEvent *event)
{
         // Just accept the proposed action
        event->acceptProposedAction();
}

// Event: drag has moved above the widget
void CollectionBrowser::dragMoveEvent(QDragMoveEvent* event)
{
        // Again - accept the proposed action
        event->acceptProposedAction();
}

 // Drop event (item is dropped on the widget)
void CollectionBrowser::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        for (int i = 0; i < urls.size(); i++) {
            if (QFileInfo(urls.at(i).toLocalFile()).isFile()) {
                addItem(urls.at(i).toLocalFile());
            }
        }
        event->setAccepted(true);
    } else {
        event->setAccepted(false);
    }
}

void CollectionBrowser::keyPressEvent(QKeyEvent* event)
{
    // When 'delete' pressed, remove selected row from collections
    if (event->matches(QKeySequence::Delete)) {
        QModelIndex index = selectionModel()->currentIndex();
        removeItem(index.row());
        //model()->removeRow(index.row(),index.parent());
    }

}


QModelIndex CollectionBrowser::addItem(QString file) //SLOT
{
    // Select the root item
    QModelIndex index = selectionModel()->currentIndex();

    // Insert new row
    if (!model()->insertRow(index.row()+1, index.parent()))
        return QModelIndex();

    // Child item
    QModelIndex child;
    // Store the filename into the first column. The other columns will be filled by separated thread
    child = model()->index(index.row()+1, 0, index.parent());
    model()->setData(child, QVariant(file), Qt::EditRole);
    // Default track number
    child = model()->index(index.row()+1, 1, index.parent());
    model()->setData(child, QVariant(0), Qt::EditRole);
    // Extract filename from the path and insert it as a trackname
    child = model()->index(index.row()+1, 3, index.parent());
    model()->setData(child, QVariant(QFileInfo(file).fileName()),Qt::EditRole);

    return child;
}

void CollectionBrowser::removeItem(int row) //SLOT
{
        model()->removeRow(row,QModelIndex());
}

void CollectionBrowser::removeItems(int row, int count) // SLOT
{
    /* When row removed the following row takes its place and therefor by deleting row number "row" for "count"-times
       all the lines are removed */
    for (int i = 0; i<count; i++) {
        removeItem(row);
    }
}
