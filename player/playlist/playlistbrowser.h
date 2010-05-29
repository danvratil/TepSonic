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

#ifndef PLAYLISTBROWSER_H
#define PLAYLISTBROWSER_H

#include <QObject>
#include <QTreeView>
#include <QStringList>

class PlaylistManager;

//! PlaylistBrowser class is a QTreeView subclass with support for drop events
/*!
  PlaylistBrowser class i subclasses from QTreeView and provides support for ending
  drag&drop operations. Dropped items are passed to PlaylistManager which loads them into
  the PlaylistModel from separate thread without freezing the UI
*/
class PlaylistBrowser : public QTreeView
{
    Q_OBJECT

    public:
        //! Constructor
        /*!
          \param parent pointer to parent QWidget
        */
        PlaylistBrowser(QWidget *parent= 0);

        //! Destructor
        ~PlaylistBrowser();

    protected:
        //! Called when items are dropped on the browser
        /*!
          When drag&drop action is finished by dropping items into PlaylistBrowser the items are read
          from \p dropEvent and passed to PlaylistManager which loads the files into the PlaylistModel
          \param dropEvent provides more informations about the event
        */
        void dropEvent(QDropEvent *dropEvent);

        //! Called when items are dragged into the PlaylistBrowser
        /*!
          \param dragEnterEvent provides more informations about the event
        */
        void dragEnterEvent(QDragEnterEvent *dragEnterEvent);

        //! Called when items are moved within the PlaylistBrowser
        /*!
          Currently the action is just accepted without any response
          \todo move items within PlaylistBrowser by dragging them from place to place
          \param dragMoveEvent provides more informations about the event
        */
        void dragMoveEvent(QDragMoveEvent *dragMoveEvent);

        //! Called when a key is pressed when the PlaylistBrowser has focus
        /*!
          Removes selected items from PlaylistModel
          \param keyEvent provides more informations about the event
        */
        void keyPressEvent(QKeyEvent *keyEvent);

        //! PlaylistManager
        PlaylistManager *_playlistManager;

    signals:
        void addedFiles(QStringList);



};

#endif // PLAYLISTBROWSER_H