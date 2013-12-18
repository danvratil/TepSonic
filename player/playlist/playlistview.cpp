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

#include "playlistview.h"
#include "playlistmodel.h"
#include "playlistproxymodel.h"
#include "playlistitemdelegate.h"
#include "taskmanager.h"
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
#include <QMimeData>

#ifdef Q_WS_WIN
#include <cstdlib>
#include <ctime>
#else
#include <stdlib.h>
#include <time.h>
#endif

PlaylistView::PlaylistView(QWidget *parent):
    QTreeView(parent)
{
    m_playlistModel = new PlaylistModel(this);
    PlaylistProxyModel *proxy = new PlaylistProxyModel(m_playlistModel, this);
    setModel(proxy);

    // Set up task manager
    TaskManager::instance()->setPlaylistModel(m_playlistModel);

    connect(m_playlistModel, SIGNAL(playlistLengthChanged(int, int)),
            this, SIGNAL(playlistLengthChanged(int, int)));

    setItemDelegate(new PlaylistItemDelegate(this));
    setAcceptDrops(true);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
    setDropIndicatorShown(true);
    setAlternatingRowColors(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    sortByColumn(-1);
    setWordWrap(false);
    setTextElideMode(Qt::ElideRight);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    header()->setContextMenuPolicy(Qt::CustomContextMenu);
    header()->setSortIndicatorShown(true);
    header()->setSectionsClickable(true);

    // Hide the first column (with filename) and the last one (with order number)
    hideColumn(PlaylistModel::FilenameColumn);
    hideColumn(PlaylistModel::RandomOrderColumn);

    connect(header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            this, SLOT(slotSortIndicatorChanged(int,Qt::SortOrder)));
    connect(TaskManager::instance(), SIGNAL(playlistPopulated()),
            model(), SLOT(invalidate()));
    connect(TaskManager::instance(), SIGNAL(insertItemToPlaylist(Player::MetaData, int)),
            m_playlistModel, SLOT(insertItem(Player::MetaData, int)));
}

PlaylistView::~PlaylistView()
{
}

PlaylistModel* PlaylistView::playlistModel() const
{
    return m_playlistModel;
}

void PlaylistView::clear()
{
    m_playlistModel->clear();
    setNowPlaying(QModelIndex());
    setStopTrack(QModelIndex());
}

QModelIndex PlaylistView::nowPlaying() const
{
    return m_nowPlaying;
}

void PlaylistView::setNowPlaying(const QModelIndex &index)
{
    // Break potential recursion
    if (m_nowPlaying == index) {
        return;
    }

    invalidateIndex(m_nowPlaying);
    m_nowPlaying = index;
    invalidateIndex(m_nowPlaying);
    Q_EMIT nowPlayingChanged(m_nowPlaying);
}

void PlaylistView::setStopTrack(const QModelIndex &index)
{
    invalidateIndex(m_stopTrack);
    m_stopTrack = index;
    invalidateIndex(m_stopTrack);
}

QModelIndex PlaylistView::stopTrack() const
{
    return m_stopTrack;
}

void PlaylistView::clearStopTrack()
{
    invalidateIndex(m_stopTrack);
    m_stopTrack = QModelIndex();
}

void PlaylistView::invalidateIndex(const QModelIndex &index)
{
    if (index.isValid()) {
        const QModelIndex left = index.sibling(index.row(), 0);
        const QModelIndex right = index.sibling(index.row(), model()->columnCount() - 1);
        Q_EMIT dataChanged(left, right);
    }
}

void PlaylistView::selectNextTrack()
{
    // If the track we just played was "stop-on-this" track then stop playback
    if (m_stopTrack.isValid() && m_stopTrack.row() == m_nowPlaying.row()) {
        Player::instance()->stop();
        return;
    }

    // 1) Random playback?
    if (Player::instance()->randomMode()) {
        int row = rand() % m_playlistModel->rowCount();
        setNowPlaying(model()->index(row, 0));

        // 2) Not last item?
    } else if (indexBelow(m_nowPlaying).isValid()) {
        setNowPlaying(indexBelow(m_nowPlaying));

        // 3) Repeat all playlist?
    } else if (Player::instance()->repeatMode() == Player::RepeatAll) {
        setNowPlaying(model()->index(0, 0));

        // 4) Stop, there's nothing else to play
    } else {
        return;
    }
}

void PlaylistView::selectPreviousTrack()
{
    if (m_nowPlaying.row() > 0 && model()->rowCount() > 0) {
        setNowPlaying(model()->index(m_nowPlaying.row() - 1, 0));
    }
}

void PlaylistView::setFilter(const QString &filter)
{
    qobject_cast<QSortFilterProxyModel*>(model())->setFilterRegExp(filter);
}

// Event: item is dragged over the widget
void PlaylistView::dragEnterEvent(QDragEnterEvent *event)
{
    // Just accept the proposed action
    event->acceptProposedAction();
}

// Event: drag has moved above the widget
void PlaylistView::dragMoveEvent(QDragMoveEvent *event)
{
    // Again - accept the proposed action
    event->acceptProposedAction();
}

void PlaylistView::mousePressEvent(QMouseEvent *event)
{
    // Remember the event position
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
    }

    // Handle all the default actions
    QTreeView::mousePressEvent(event);
}

void PlaylistView::mouseMoveEvent(QMouseEvent *event)
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
void PlaylistView::dropEvent(QDropEvent *event)
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

void PlaylistView::keyPressEvent(QKeyEvent *event)
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
            Q_EMIT doubleClicked(currentIndex());
            event->accept();
            return;
        }
    }
}

void PlaylistView::shuffle()
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

void PlaylistView::slotSortIndicatorChanged(int column, Qt::SortOrder order)
{
    model()->sort(column, order);
}
