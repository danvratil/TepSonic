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

#include "playlistitemdelegate.h"
#include "playlistmodel.h"
#include "playlistbrowser.h"
#include "playlistproxymodel.h"

#include <QRect>

PlaylistItemDelegate::PlaylistItemDelegate(QObject *parent,
                                           PlaylistModel *playlistModel,
                                           PlaylistBrowser *playlistBrowser,
                                           PlaylistProxyModel *playlistProxyModel):
    QStyledItemDelegate(parent),
    m_playlistModel(playlistModel),
    m_playlistBrowser(playlistBrowser),
    m_playlistProxyModel(playlistProxyModel)
{
}

void PlaylistItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    QRect rect(option.rect);
    rect.setLeft(rect.left() - 3);

    painter->setFont(option.font);
    painter->setPen(option.palette.text().color());

    const QModelIndex mappedIndex = m_playlistProxyModel->mapToSource(index);

    if (m_playlistModel->stopTrack().row() == mappedIndex.row()) {
        if (mappedIndex.row() == m_playlistModel->currentItem().row()) {
            painter->fillRect(rect, option.palette.link());
        } else {
            painter->fillRect(rect, option.palette.dark().color());
        }
        painter->setPen(option.palette.light().color());
    } else if (m_playlistModel->currentItem().isValid() && mappedIndex.row() == m_playlistModel->currentItem().row()) {
        painter->fillRect(rect, option.palette.link());
        painter->setPen(option.palette.highlightedText().color());
    } else if (option.state & QStyle::State_Selected) {
        painter->fillRect(rect, option.palette.highlight());
    }

    painter->drawText(QRect(option.rect.left(),
                            option.rect.top(),
                            m_playlistBrowser->columnWidth(index.column()),
                            option.rect.bottom()),
                      option.fontMetrics.elidedText(mappedIndex.data().toString(),
                              Qt::ElideRight,
                              m_playlistBrowser->columnWidth(mappedIndex.column()))
                     );

}

#include "moc_playlistitemdelegate.cpp"
