/*
 * TEPSONIC
 * Copyright 2010 Dan Vratil <vratil@progdansoft.com>
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


#ifndef BOOKMARKSBROWSER_H
#define BOOKMARKSBROWSER_H

#include <QListView>
#include <QObject>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>


class BookmarksBrowser : public QListView
{
    Q_OBJECT
    public:
        explicit BookmarksBrowser(QWidget *parent = 0);

    signals:

    public slots:
        void setFilter(QString);
        void addItem(QString);
        void removeAt(int);

        QModelIndex mapToSource(QModelIndex);
        QModelIndex mapFromSource(QModelIndex);

    private:
        QStandardItemModel *m_model;
        QSortFilterProxyModel *m_proxyModel;



};

#endif // BOOKMARKSBROWSER_H
