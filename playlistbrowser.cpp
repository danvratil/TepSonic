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

PlaylistBrowser::PlaylistBrowser(QWidget* parent):
          QTreeView(parent)
{
    setAcceptDrops(true);
}

PlaylistBrowser::~PlaylistBrowser()
{
}

void PlaylistBrowser::addTracks(QStringList *filesList)
{
    //lunch thread and give it list of files,
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

         //TODO: Accept QTreeWidgetItem
         // Get text from informations about the event
        QString text = event->mimeData()->text();


        // TODO: QTreeWidgetItem was accepted - add it into the playlist browser
        // If the text is not emtpy add it to the list
        /*if(!text.isEmpty())
                addItem(event->mimeData()->text());*/
}
