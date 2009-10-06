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


#include "playlistbrowser.h"

#include <QDropEvent>
#include <QDebug>
#include <QList>
#include <QStringList>
#include <QUrl>
#include <QFileInfo>

PlaylistBrowser::PlaylistBrowser(QWidget* parent):
          QTreeView(parent)
{
    setAcceptDrops(true);
}

PlaylistBrowser::~PlaylistBrowser()
{
}

// Event: item is dragged over the widget
void PlaylistBrowser::dragEnterEvent(QDragEnterEvent *event)
{
         // Just accept the proposed action
        event->acceptProposedAction();
}

// Event: drag has moved above the widget
void PlaylistBrowser::dragMoveEvent(QDragMoveEvent* event)
{
        // Again - accept the proposed action
        event->acceptProposedAction();
}

 // Drop event (item is dropped on the widget)
void PlaylistBrowser::dropEvent(QDropEvent* event)
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

void PlaylistBrowser::keyPressEvent(QKeyEvent* event)
{
    // When 'delete' pressed, remove selected row from playlist
    if (event->matches(QKeySequence::Delete)) {
        QModelIndex index = selectionModel()->currentIndex();
        removeItem(index.row());
        //model()->removeRow(index.row(),index.parent());
    }

}


void PlaylistBrowser::addItem(QString file) //SLOT
{
    // Select the root item
    QModelIndex index = selectionModel()->currentIndex();

    // Insert new row
    if (!model()->insertRow(index.row()+1, index.parent()))
        return;

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
}

void PlaylistBrowser::removeItem(int row) //SLOT
{
        model()->removeRow(row,QModelIndex());
}

void PlaylistBrowser::removeItems(int row, int count) // SLOT
{
    /* When row removed the following row takes its place and therefor by deleting row no. "row" for "count"-time
       all the lines are removed */
    for (int i = 0; i<count; i++) {
        removeItem(row);
    }
}
