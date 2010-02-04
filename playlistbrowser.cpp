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

#include "playlistmodel.h"

#include <QDir>
#include <QDropEvent>
#include <QDebug>
#include <QList>
#include <QStringList>
#include <QUrl>
#include <QFile>
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
    event->acceptProposedAction();
    QByteArray encodedData = event->mimeData()->data("data/tepsonic-tracks");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QStringList newItems;

    while (!stream.atEnd()) {
        QString text;
        stream >> text;
        static_cast<PlaylistModel*>(model())->addItem(text);
    }

    event->setAccepted(true);
}

void PlaylistBrowser::keyPressEvent(QKeyEvent* event)
{
    // When 'delete' pressed, remove selected row from playlist
    if (event->matches(QKeySequence::Delete)) {
        model()->removeRow(selectionModel()->currentIndex().row());
    }
}

void PlaylistBrowser::savePlaylist(QString filename)
{

}
