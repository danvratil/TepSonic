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

#ifndef PLAYLISTITEMDELEGATE_H
#define PLAYLISTITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QModelIndex>

class QAbstractProxyModel;
class PlaylistView;

class PlaylistItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    explicit PlaylistItemDelegate(PlaylistView *parent);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

  private:
    QAbstractProxyModel *mProxyModel;
};

#endif // PLAYLISTITEMDELEGATE_H
