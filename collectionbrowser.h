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

/**
 * Collection Browser
 *    QTreeWidget with implemented DRAG action
 *
 * Thanks to David Watzke
 *
 */

#ifndef COLLECTIONBROWSER_H
#define COLLECTIONBROWSER_H

#include <QTreeWidget>

class CollectionBrowser : public QTreeWidget
{
    public:
        CollectionBrowser(QWidget* = 0);

    private:
        QPoint dragStartPosition;

    protected:
        void mousePressEvent(QMouseEvent*);
        void mouseMoveEvent(QMouseEvent*);
};

#endif // COLLECTIONBROWSER_H
