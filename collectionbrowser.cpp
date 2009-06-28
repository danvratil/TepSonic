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
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>

CollectionBrowser::CollectionBrowser(QWidget* parent): QTreeWidget(parent)
{
}

 // Event of mouse button press
void CollectionBrowser::mousePressEvent(QMouseEvent* event)
{
         // First to call standard implementation of the event
        QTreeView::mousePressEvent(event);

         // If LMB is pressed then store current cursor position
        if(event->button() == Qt::LeftButton)
                dragStartPosition = event->pos();

        event->accept();
}

 // Event of mouse movement
void CollectionBrowser::mouseMoveEvent(QMouseEvent* event)
{
         // If LMB is not pressed, don't start the drag action
        if (!(event->buttons() & Qt::LeftButton))
                return;

         /* Don't start the drag action if the cursor since LMB is pressed didn't
            moved at least of QApplication::startDragDistance() */
        if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
                return;

         // Get index of cell below the cursor
        QModelIndex index = indexAt(event->pos());

         /* Don't start the drag action if there is not a cell below
            the cursor (eg. the index is not valid) */
        if(!index.isValid())
                return;

         // Create a drag object
        QDrag* drag = new QDrag(this);

         // Save data we need to transport
        QMimeData* mime = new QMimeData;

        //TODO: Transport QTreeWidgetItem
        /*// Transport text stored in dragged cell
        mime->setText( index.data().toString() );
        drag->setMimeData(mime);*/

         // Start the drag
        drag->exec(Qt::CopyAction);
}
