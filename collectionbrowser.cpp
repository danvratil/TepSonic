/*
 * Collection Browser
 *    QTreeWidget with implemented DRAG action
 *
 * Thanks to David Watzke
 *
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
