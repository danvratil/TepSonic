/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <me@dvratil.cz>
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
#include "playlistitemdelegate.h"
#include "playlistsortfiltermodel.h"

#include <core/actionmanager.h>
#include <core/player.h>
#include <core/supportedformats.h>
#include <core/utils.h>
#include <core/playlist.h>

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
#include <QMenu>

#ifdef Q_WS_WIN
#include <cstdlib>
#include <ctime>
#else
#include <stdlib.h>
#include <time.h>
#endif

using namespace TepSonic;

PlaylistView::PlaylistView(QWidget *parent):
    QTreeView(parent),
    m_currentTrack(-1),
    m_stopTrack(-1)
{
    setModel(new PlaylistSortFilterModel(this));

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
    hideColumn(0);

    connect(header(), &QHeaderView::sortIndicatorChanged,
            this, &PlaylistView::slotSortIndicatorChanged);
    connect(header(), &QHeaderView::customContextMenuRequested,
            this, &PlaylistView::slotHeaderContextMenuRequested);
    connect(this, &PlaylistView::customContextMenuRequested,
            this, &PlaylistView::slotContextMenuRequested);
    connect(this, &PlaylistView::doubleClicked,
            this, &PlaylistView::slotItemDoubleClicked);
    connect(Player::instance(), &Player::trackChanged,
            this, &PlaylistView::slotCurrentTrackChanged);
    connect(Player::instance(), &Player::stopTrackChanged,
            this, &PlaylistView::slotStopTrackChanged);
}

PlaylistView::~PlaylistView()
{
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

    for (int i = 0; i < selection.indexes().size(); i++) {
        const QModelIndex index = selection.indexes().at(i);
        for (int j = 0; j < model()->columnCount(); j++)  {
            stream << index.sibling(index.row(), j).data().toString();
        }
        // Remove the row from view
        model()->removeRow(selection.indexes().at(i).row());
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


        // Non-recursively expand folders
        QMutableListIterator<QString> iter(files);
        while (iter.hasNext()) {
            QFileInfo finfo(iter.next());
            if (finfo.isDir()) {
                const QDir dir(finfo.filePath());
                const QStringList subfiles = dir.entryList(SupportedFormats::extensionsList(),
                                                           QDir::Files);
                Q_FOREACH (const QString &file, subfiles) {
                    iter.insert(finfo.filePath() + QLatin1Char('/') + file);
                }
                const QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
                Q_FOREACH (const QString &file, subdirs) {
                    iter.insert(finfo.filePath() + QLatin1Char('/') + file);
                }
                iter.findPrevious(finfo.filePath());
                iter.remove();
            }
        }

        //row where the items were dropped
        int row = indexAt(event->pos()).row();
        if (row == -1) {
            row = model()->rowCount();
        }

        if (!files.isEmpty()) {
            Player::instance()->playlist()->insert(files, row);
        }
    }

    // Drop from internal move
    if (event->mimeData()->hasFormat(QLatin1String("data/tepsonic-playlist-items"))) {
        QByteArray encodedData = event->mimeData()->data(QLatin1String("data/tepsonic-playlist-items"));
        QDataStream stream(&encodedData, QIODevice::ReadOnly);

        Playlist *model = Player::instance()->playlist();
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

        MetaData::List metadataList;
        MetaData metadata;
        QString s;
        while (!stream.atEnd()) {
            stream >> s;
            metadata.setFileName(s);
            stream >> s;
            metadata.setTrackNumber(s.toUInt());
            stream >> s;
            metadata.setArtist(s);
            stream >> s;
            metadata.setTitle(s);
            stream >> s;
            metadata.setAlbum(s);
            stream >> s;
            metadata.setGenre(s);
            stream >> s;
            metadata.setYear(s.toUInt());
            stream >> s;
            metadata.setBitrate(s.toInt());

            metadataList << metadata;
        }
        Player::instance()->playlist()->insert(metadataList, row);
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
            const QModelIndexList selected = selectionModel()->selectedRows();
            if (selected.isEmpty()) { 
                return;
            }
            int lastRow = selected.last().row();
            for (int i = selected.size() - 1; i >= 0; --i) {
                model()->removeRow(selected.at(i).row());
            }
            if (lastRow >= model()->rowCount()) {
                lastRow = model()->rowCount() - 1;
                if (lastRow == -1) {
                    return;
                }
            }
            selectionModel()->select(QItemSelection(model()->index(lastRow, 0),
                                                    model()->index(lastRow, model()->columnCount() - 1)),
                                     QItemSelectionModel::ClearAndSelect);
            event->accept();
            return;
        }

        case Qt::Key_Enter:  // key ENTER (on numeric keypad)
        case Qt::Key_Return: { // Key ENTER
            Player::instance()->setCurrentTrack(currentIndex().row());
            event->accept();
            return;
        }
    }
}

void PlaylistView::slotSortIndicatorChanged(int column, Qt::SortOrder order)
{
    model()->sort(column, order);
}

void PlaylistView::slotHeaderContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = ActionManager::instance()->menu(QStringLiteral("PlaylistVisibleColumns"));
    menu->popup(header()->mapToGlobal(pos));
}

void PlaylistView::slotContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = ActionManager::instance()->menu(QStringLiteral("PlaylistContextMenu"));
    const bool valid = (currentIndex().isValid() || indexAt(pos).isValid());

    for (int i = 0; i < menu->actions().count(); i++) {
        menu->actions().at(i)->setEnabled(valid);
    }

    menu->popup(mapToGlobal(pos));
}

void PlaylistView::slotItemDoubleClicked(const QModelIndex &index)
{
    QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(model());
    const QModelIndex mappedIndex = proxy->mapToSource(index);
    if (!mappedIndex.isValid()) {
        return;
    }

    Player::instance()->setCurrentTrack(mappedIndex.row());
    Player::instance()->play();
}

void PlaylistView::invalidateRow(int row)
{
    if (row > -1) {
        dataChanged(model()->index(row, 0),
            model()->index(row, model()->columnCount()));
    }
}


void PlaylistView::slotCurrentTrackChanged()
{
    invalidateRow(m_currentTrack);
    m_currentTrack = Player::instance()->currentTrack();
    invalidateRow(m_currentTrack);
}

void PlaylistView::slotStopTrackChanged()
{
    invalidateRow(m_stopTrack);
    m_stopTrack = Player::instance()->stopTrack();
    invalidateRow(m_stopTrack);
}
