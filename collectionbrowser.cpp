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
    qDebug() << "CollectionBrowser::dropEvent() : TODO!!!";
}

void CollectionBrowser::keyPressEvent(QKeyEvent* event)
{
    // When 'delete' pressed, remove selected row from collections
    if (event->matches(QKeySequence::Delete)) {
        QModelIndex index = selectionModel()->currentIndex();
        removeRow(index.parent(),index.row());
        //model()->removeRow(index.row(),index.parent());
    }

}


QModelIndex CollectionBrowser::insertChild(QModelIndex index, QString title, QString filename)
{
    if (!model()->insertRow(0, index))
        return QModelIndex();

    QModelIndex child;
    // Some title like artist/album/track name
    child = model()->index(0, 0, index);
    model()->setData(child, title);
    // Real file name (in hidden column)
    child = model()->index(0, 1, index);
    model()->setData(child, filename);

    return child;
}


QModelIndex CollectionBrowser::insertRow(QModelIndex index, QString title, QString filename)
{
    if (!model()->insertRow(index.row()+1, index.parent()))
        return QModelIndex();

    QModelIndex child;
    // Some title like artist/album/track name
    child = model()->index(index.row()+1, 0, index.parent());
    model()->setData(child, title, Qt::EditRole);
    // Readl file name (in hidden column)
    child = model()->index(index.row()+1, 1, index.parent());
    model()->setData(child, filename, Qt::EditRole);

    return model()->index(index.row()+1,0,index.parent());
}

void CollectionBrowser::removeRow(QModelIndex parent, int row)
{
    model()->removeRow(parent.child(row,0).row(), parent);
 }
