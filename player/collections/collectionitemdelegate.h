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

#ifndef COLLECTIONITEMDELEGATE_H
#define COLLECTIONITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QList>
#include <QModelIndex>
#include <QSize>

class CollectionProxyModel;

class CollectionItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    explicit CollectionItemDelegate(QObject *parent = 0, CollectionProxyModel *proxyModel = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option = QStyleOptionViewItem(),
                   const QModelIndex &index = QModelIndex()) const;

  private:
    mutable QList<QModelIndex> m_currentIndexes;

    CollectionProxyModel *m_proxyModel;
};

#endif // COLLECTIONITEMDELEGATE_H
