/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <dan@progdan.cz>
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
#include "tools.h"

#include <QApplication>
#include <QDropEvent>
#include <QItemSelectionRange>
#include <QDrag>
#include <QHeaderView>
#include <QDir>
#include <QDebug>
#include <QList>
#include <QStringList>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QModelIndex>


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
    setEditTriggers(NoEditTriggers);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(true);
    setDefaultDropAction(Qt::MoveAction);
    setWordWrap(false);
    setTextElideMode(Qt::ElideRight);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    header()->setSortIndicatorShown(true);
    header()->setClickable(true);

    connect(header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            this, SLOT(slotSortIndicatorChanged(int,Qt::SortOrder)));
}

PlaylistBrowser::~PlaylistBrowser()
{
}

QModelIndex PlaylistBrowser::nowPlaying() const
{
    return m_nowPlaying;
}

void PlaylistBrowser::setNowPlaying(const QModelIndex &index)
{
    invalidateIndex(m_nowPlaying);
    m_nowPlaying = index;
    invalidateIndex(m_nowPlaying);
}

void PlaylistBrowser::setStopTrack(const QModelIndex &index)
{
    invalidateIndex(m_stopTrack);
    m_stopTrack = index;
    invalidateIndex(m_stopTrack);
}

QModelIndex PlaylistBrowser::stopTrack() const
{
    return m_stopTrack;
}

void PlaylistBrowser::clearStopTrack()
{
    invalidateIndex(m_stopTrack);
    m_stopTrack = QModelIndex();
}

void PlaylistBrowser::invalidateIndex(const QModelIndex &index)
{
    if (index.isValid()) {
        const QModelIndex left = index.sibling(index.row(), 0);
        const QModelIndex right = index.sibling(index.row(), model()->columnCount() - 1);
        emit dataChanged(left, right);
    }
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
    QTreeView::mousePressEvent(event);
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

    for (int i = 0; i < selection.indexes().size(); i++) {
        const QModelIndex index = selection.indexes().at(i);
        for (int j = 0; j < PlaylistModel::ColumnCount; j++)  {
            stream << index.sibling(index.row(), j).data().toString();
        }
        // Remove the row from view
        model->removeRow(selection.indexes().at(i).row());
    }

    mimeData->setData(QLatin1String("data/tepsonic-playlist-items"), encodedData);
    drag->setMimeData(mimeData);

    drag->exec(Qt::MoveAction);
}

// Drop event (item is dropped on the widget)
void PlaylistBrowser::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();

    if (event->mimeData()->hasFormat(QLatin1String("data/tepsonic-tracks")) || event->mimeData()->hasUrls()) {

        QStringList files;
        // Drop from CollectionsBrowser
        if (event->mimeData()->hasFormat(QLatin1String("data/tepsonic-tracks"))) {
            QByteArray encodedData = event->mimeData()->data(QLatin1String("data/tepsonic-tracks"));
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
    if (event->mimeData()->hasFormat(QLatin1String("data/tepsonic-playlist-items"))) {
        QByteArray encodedData = event->mimeData()->data(QLatin1String("data/tepsonic-playlist-items"));
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
    QTreeView::keyPressEvent(event);
    if (event->isAccepted()) {
        return;
    }

    switch (event->key()) {
        case Qt::Key_Delete: { // Key DELETE
            for (int i = 0; i < selectedIndexes().size(); i++) {
                model()->removeRow(selectedIndexes().at(i).row());
            }
            event->accept();
            return;
        }

        case Qt::Key_Enter:  // key ENTER (on numeric keypad)
        case Qt::Key_Return: { // Key ENTER
            setNowPlaying(currentIndex());
            emit doubleClicked(currentIndex());
            event->accept();
            return;
        }
    }
}

void PlaylistBrowser::shuffle()
{
    header()->setSortIndicator(-1, Qt::AscendingOrder);

    srand(time(0)); // We needs microseconds
    const int rowCount = model()->rowCount();
    for (int row = 0; row < rowCount; row++) {
        qulonglong order = (qulonglong)rand();
        model()->setData(model()->index(row, PlaylistModel::RandomOrderColumn), QVariant(order));
    }

    model()->sort(PlaylistModel::RandomOrderColumn, Qt::AscendingOrder);
}

void PlaylistBrowser::slotSortIndicatorChanged(int column, Qt::SortOrder order)
{
    model()->sort(column, order);
}


#include "moc_playlistbrowser.cpp"
