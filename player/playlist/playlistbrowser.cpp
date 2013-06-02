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
#include "tools.h"

#include <QtGui/QApplication>
#include <QtGui/QDropEvent>
#include <QtGui/QItemSelectionRange>
#include <QtGui/QDrag>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QModelIndex>


#ifdef Q_WS_WIN
#include <cstdlib>
#include <ctime>
#else
#include <stdlib.h>
#include <time.h>
#endif

PlaylistBrowser::PlaylistBrowser(QWidget *parent):
    QTreeView(parent)
{
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setWordWrap(false);
    setTextElideMode(Qt::ElideRight);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
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
void PlaylistBrowser::dragMoveEvent(QDragMoveEvent *event)
{
    // Again - accept the proposed action
    event->acceptProposedAction();
}

void PlaylistBrowser::mousePressEvent(QMouseEvent *event)
{
    // Remember the event position
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
    }

    // Handle all the default actions
    QAbstractItemView::mousePressEvent(event);
}

void PlaylistBrowser::mouseMoveEvent(QMouseEvent *event)
{
    // No left buttton? Run!
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }

    // If the move distance is shorter then startDragDistance() then this is not drag event and let's go away
    if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
        return;
    }

    // Initiate the drag!
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);;

    // Selected indexes
    QItemSelection selection = selectionModel()->selection();

    PlaylistProxyModel *proxyModel = qobject_cast<PlaylistProxyModel *>(model());
    if (!proxyModel) {
        return;
    }
    selection = proxyModel->mapSelectionToSource(selection);
    PlaylistModel *model = qobject_cast<PlaylistModel *>(proxyModel->sourceModel());
    if (!model) {
        return;
    }

    for (int i = 0; i < selection.indexes().size(); i ++) {
        PlaylistItem *item = model->getItem(selection.indexes().at(i));
        stream << item->data(PlaylistBrowser::FilenameColumn).toString();
        stream << item->data(PlaylistBrowser::TrackColumn).toString();
        stream << item->data(PlaylistBrowser::InterpretColumn).toString();
        stream << item->data(PlaylistBrowser::TracknameColumn).toString();
        stream << item->data(PlaylistBrowser::AlbumColumn).toString();
        stream << item->data(PlaylistBrowser::GenreColumn).toString();
        stream << item->data(PlaylistBrowser::YearColumn).toString();
        stream << item->data(PlaylistBrowser::LengthColumn).toString();
        stream << item->data(PlaylistBrowser::BitrateColumn).toString();
        // Remove the row from view
        model->removeRow(selection.indexes().at(i).row());
    }

    mimeData->setData("data/tepsonic-playlist-items", encodedData);
    drag->setMimeData(mimeData);

    drag->exec(Qt::MoveAction);
}

// Drop event (item is dropped on the widget)
void PlaylistBrowser::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();

    if (event->mimeData()->hasFormat("data/tepsonic-tracks") || event->mimeData()->hasUrls()) {

        QStringList files;
        // Drop from CollectionsBrowser
        if (event->mimeData()->hasFormat("data/tepsonic-tracks")) {
            QByteArray encodedData = event->mimeData()->data("data/tepsonic-tracks");
            QDataStream stream(&encodedData, QIODevice::ReadOnly);
            QStringList newItems;

            while (!stream.atEnd()) {
                QString text;
                stream >> text;
                files << text;
            }
            // Drop from URLs
        } else if (event->mimeData()->hasUrls()) {

            QList<QUrl> urlList = event->mimeData()->urls();

            Q_FOREACH (const QUrl &url, urlList) {
                files << url.path();
            }
        }

        //row where the items were dropped
        int row = indexAt(event->pos()).row();
        if (row == -1) {
            row = model()->rowCount();
        }

        Q_EMIT addedFiles(files, row);
    }

    // Drop from internal move
    if (event->mimeData()->hasFormat("data/tepsonic-playlist-items")) {
        QByteArray encodedData = event->mimeData()->data("data/tepsonic-playlist-items");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);

        // We need the original PlaylistModel
        PlaylistProxyModel *proxyModel = qobject_cast<PlaylistProxyModel *>(model());
        if (!proxyModel) {
            return;
        }
        PlaylistModel *model = qobject_cast<PlaylistModel *>(proxyModel->sourceModel());
        if (!model) {
            return;
        }

        // Some math about where to insert the item
        int row = indexAt(event->pos()).row();
        if (row == -1) {
            row = model->rowCount();
        }
        else if (row < model->rowCount()) {
            row += 1;
        }

        Player::MetaData metadata;
        QString s;
        while (!stream.atEnd()) {
            stream >> s;
            metadata.filename = s;
            stream >> s;
            metadata.trackNumber = s.toInt();
            stream >> s;
            metadata.artist = s;
            stream >> s;
            metadata.title = s;
            stream >> s;
            metadata.album = s;
            stream >> s;
            metadata.genre = s;
            stream >> s;
            metadata.year = s.toInt();
            stream >> s;
            metadata.formattedLength = s;
            stream >> s;
            metadata.bitrate = s.toInt();
            // length in milliseconds - to properly count total length of playlist
            metadata.length = formattedLengthToSeconds(metadata.formattedLength) * 1000;

            model->insertItem(metadata, row);
        }
    }

    event->setAccepted(true);
}

void PlaylistBrowser::keyPressEvent(QKeyEvent *event)
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
    }
    break;
    case Qt::Key_Up: { // Key UP
        QModelIndex prevItem = indexAbove(currentIndex());
        if (prevItem.isValid())
            setCurrentIndex(prevItem);
    }
    break;
    case Qt::Key_Enter:  // key ENTER (on numeric keypad)
    case Qt::Key_Return: { // Key ENTER
        PlaylistProxyModel *ppmodel = qobject_cast<PlaylistProxyModel *>(model());
        if (!ppmodel) return;
        PlaylistModel *pmodel = qobject_cast<PlaylistModel *>(ppmodel->sourceModel());
        if (!pmodel) return;
        pmodel->setCurrentItem(currentIndex());
        emit setTrack(currentIndex().row());
    }
    break;
    }
    event->accept();

}

void PlaylistBrowser::setStopTrack()
{
    PlaylistProxyModel *ppmodel = qobject_cast<PlaylistProxyModel *>(model());
    if (!ppmodel) {
        return;
    }

    PlaylistModel *pmodel = qobject_cast<PlaylistModel *>(ppmodel->sourceModel());
    if (!pmodel) {
        return;
    }

    pmodel->setStopTrack(selectedIndexes().first());
}

void PlaylistBrowser::shuffle()
{
    srand(time(0)); // We needs microseconds
    const int rowCount = model()->rowCount();
    for (int row = 0; row < rowCount; row++) {
        qulonglong order = (qulonglong)rand();
        model()->setData(model()->index(row, PlaylistBrowser::RandomOrderColumn), QVariant(order));
    }

    model()->sort(PlaylistBrowser::RandomOrderColumn, Qt::AscendingOrder);
}
