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

/** Playlist Browser
 *    QTreeWidget with implemented DROP action
 *
 * Thanks to David Watzke
 *
 */

#ifndef PLAYLISTBROWSER_H
#define PLAYLISTBROWSER_H

#include <QTreeWidget>

class PlaylistBrowser : public QTreeWidget
{
    public:
        PlaylistBrowser(QWidget* = 0);

    protected:
        void dropEvent(QDropEvent*);
        void dragEnterEvent(QDragEnterEvent*);
        void dragMoveEvent(QDragMoveEvent*);
};

#endif // PLAYLISTBROWSER_H
