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

#ifndef COLLECTIONBROWSER_H
#define COLLECTIONBROWSER_H

#include <QTreeView>

class CollectionBrowser : public QTreeView
{
    public:
        CollectionBrowser(QWidget* = 0);
        ~CollectionBrowser();

    protected:
        void dropEvent(QDropEvent*);
        void dragEnterEvent(QDragEnterEvent*);
        void dragMoveEvent(QDragMoveEvent*);
        void keyPressEvent(QKeyEvent*);


    public slots:
        QModelIndex addItem(QString fileName);
        void removeItem(int row);
        void removeItems(int row, int count);

};

#endif // COLLECTIONBROWSER_H
