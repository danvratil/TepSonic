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
#include "playlistbrowser.h"

#include <QRect>


PlaylistItemDelegate::PlaylistItemDelegate(QObject *parent, PlaylistModel *playlistModel, PlaylistBrowser *playlistBrowser):
        QStyledItemDelegate(parent)
{
    _playlistModel = playlistModel;
    _playlistBrowser = playlistBrowser;


}

void PlaylistItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect rect(option.rect);
    rect.setLeft(rect.left()-3);
    painter->setPen(option.palette.text().color());
    painter->setPen(option.font.pixelSize());
    painter->setFont(option.font);

    if (index.row() == _playlistModel->currentItem().row()) {
        painter->fillRect(rect,option.palette.link());
        painter->setPen(option.palette.dark().color());
    } else if (option.state & QStyle::State_Selected) {
        painter->fillRect(rect,option.palette.highlight());
    }

    painter->drawText(QRect(option.rect.left(),
                            option.rect.top(),
                            _playlistBrowser->columnWidth(index.column()),
                            option.rect.bottom()),
                      option.fontMetrics.elidedText(index.data().toString(),
                                                    Qt::ElideRight,
                                                    _playlistBrowser->columnWidth(index.column()))
                      );

}
