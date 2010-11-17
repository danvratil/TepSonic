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
#include "playlistproxymodel.h"
#include "playlistitem.h"

#include <QDir>
#include <QDropEvent>
#include <QDebug>
#include <QList>
#include <QStringList>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QModelIndex>


PlaylistBrowser::PlaylistBrowser(QWidget* parent):
        QTreeView(parent)
{
    setAcceptDrops(true);
    setWordWrap(false);
    setTextElideMode(Qt::ElideRight);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
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

    QStringList files;

    while (!stream.atEnd()) {
        QString text;
        stream >> text;
        files << text;
    }

    //row where the items were dropped
    int row = indexAt(event->pos()).row();
    if (row == -1) row = model()->rowCount(QModelIndex());

    emit addedFiles(files, row);

    event->setAccepted(true);
}

void PlaylistBrowser::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
        case Qt::Key_Delete: // Key DELETE
            for (int i = 0; i < selectedIndexes().size(); i++) {
                model()->removeRow(selectedIndexes().at(i).row());
            }
            break;
        case Qt::Key_Down: { // Key DOWN
            QModelIndex nextItem = indexBelow(currentIndex());
            if (nextItem.isValid())
                setCurrentIndex(nextItem);
            } break;
        case Qt::Key_Up: { // Key UP
            QModelIndex prevItem = indexAbove(currentIndex());
            if (prevItem.isValid())
                setCurrentIndex(prevItem);
            } break;
        case Qt::Key_Enter:  // key ENTER (on numeric keypad)
        case Qt::Key_Return: { // Key ENTER
            PlaylistProxyModel *ppmodel = qobject_cast<PlaylistProxyModel*>(model());
            if (!ppmodel) return;
            PlaylistModel *pmodel = qobject_cast<PlaylistModel*>(ppmodel->sourceModel());
            if (!pmodel) return;
            pmodel->setCurrentItem(currentIndex());
            emit setTrack(currentIndex().row());
            } break;
    }
    event->accept();

}

void PlaylistBrowser::setStopTrack()
{
    PlaylistProxyModel* ppmodel = qobject_cast<PlaylistProxyModel*>(model());

    if(!ppmodel)
        return;

    PlaylistModel* pmodel = qobject_cast<PlaylistModel*>(ppmodel->sourceModel());

    if(!pmodel)
        return;

    pmodel->setStopTrack(selectedIndexes().first());
}
