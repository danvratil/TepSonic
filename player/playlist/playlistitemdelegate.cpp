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


#include "playlistitemdelegate.h"
#include "playlistmodel.h"
#include "playlistitem.h"

#include <QApplication>
#include <QFont>
#include <QPoint>
#include <QRect>

#include <QDebug>

PlaylistItemDelegate::PlaylistItemDelegate(QObject *parent, PlaylistModel *playlistModel):
        QStyledItemDelegate(parent)
{
    _playlistModel = playlistModel;

}

void PlaylistItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect rect(option.rect);
    rect.setLeft(rect.left()-3);
    painter->setPen(option.palette.text().color());

    if (index.row() == _playlistModel->currentItem().row()) {
        painter->fillRect(rect,option.palette.link());
        painter->setPen(option.palette.dark().color());
    } else if (option.state & QStyle::State_Selected) {
        painter->fillRect(rect,option.palette.highlight());
    }

    painter->drawText(QPoint(option.rect.left(),
                             option.rect.top()+10),
                      index.data().toString());
}
