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

#include "bookmarksbrowser.h"
#include "bookmarksmanager.h"

#include <QStandardItem>
#include <QDrag>
#include <QMimeData>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

BookmarksBrowser::BookmarksBrowser(BookmarksManager *manager, QWidget *parent) :
    QListView(parent),
    m_booksmarkManager(manager)
{
    setAcceptDrops(false);
    setDragDropMode(DragOnly);

    m_model = new QStandardItemModel(this);

    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    setModel(m_proxyModel);
}

BookmarksBrowser::~BookmarksBrowser()
{

}

void BookmarksBrowser::startDrag(Qt::DropActions supportedActions)
{
    if (!m_booksmarkManager) {
        return;
    }

    const QModelIndexList indexes = selectedIndexes();

    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);;

    for (int i = 0; i < indexes.count(); i++) {
        int row = m_proxyModel->mapToSource(indexes.at(i)).row();
        stream << m_booksmarkManager->getBookmarkPath(row);
    }

    mimeData->setData("data/tepsonic-tracks", encodedData);
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);
}

void BookmarksBrowser::setFilter(const QString &filter)
{
    m_proxyModel->setFilterFixedString(filter);
}

void BookmarksBrowser::addItem(const QString &title)
{
    QStandardItem *item = new QStandardItem(title);
    m_model->appendRow(item);
}

void BookmarksBrowser::removeAt(int row)
{
    //!! The row is unmapped!!
    const QModelIndex unmapped = m_proxyModel->index(row, 0, QModelIndex());
    const QModelIndex mapped = m_proxyModel->mapToSource(unmapped);

    m_model->removeRow(mapped.row());
}

QModelIndex BookmarksBrowser::mapToSource(const QModelIndex &index) const
{
    return m_proxyModel->mapToSource(index);
}

QModelIndex BookmarksBrowser::mapFromSource(const QModelIndex &index) const
{
    return m_proxyModel->mapFromSource(index);
}

#include "moc_bookmarksbrowser.cpp"
