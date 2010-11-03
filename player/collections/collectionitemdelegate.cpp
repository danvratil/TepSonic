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


#include "collectionitemdelegate.h"
#include "collectionmodel.h"
#include "collectionitem.h"
#include "collectionproxymodel.h"
#include "collectionbrowser.h"

#include <QApplication>
#include <QFont>
#include <QPoint>


#include <QDebug>

CollectionItemDelegate::CollectionItemDelegate(QObject *parent, CollectionProxyModel *proxyModel):
        QStyledItemDelegate(parent)
{
    m_proxyModel = proxyModel;
}

void CollectionItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) return;

    // Set default font and color
        painter->setFont(option.font);
    painter->setPen(option.palette.text().color());

    // We need to remap the modelIndex from the proxy model to the original model
    //QModelIndex mappedIndex = m_proxyModel->mapToSource(index);
    QModelIndex mappedIndex = index;

    CollectionModel *parentModel = const_cast<CollectionModel*>(dynamic_cast<const CollectionModel*>(mappedIndex.model()));
    CollectionItem *item = static_cast<CollectionItem*>(mappedIndex.internalPointer());

    // If item is selected or opened
    if ((option.state & QStyle::State_Selected) ||
        (option.state & QStyle::State_Open)) {

        /* If item is selected or opened but not in list of selected/opened items then
           add it to the list and force redrawing it */
        QModelIndex moi = mappedIndex;
        if (!m_currentIndexes.contains(moi)) {
            m_currentIndexes.append(moi);
            parentModel->redraw();
        }

        // It item is selected then draw highlighted background
        if (option.state & QStyle::State_Selected) {
            QRect rect(option.rect);
            rect.setLeft(rect.left()-3);
            painter->fillRect(rect, option.palette.highlight());
        }

        // Draw first row containing the interpret/album/track name
        painter->drawText(QPoint(option.rect.left(),
                                 option.rect.top()+option.fontMetrics.height()),
                          item->data(0).toString());

        painter->setFont(QFont(option.font.family(),8));
        //bottom line that is height of the first row + height of the second row + some padding
        int bottomLine = option.rect.top()+option.fontMetrics.height()+painter->fontMetrics().height()+4;

        // Draw number of albums/tracks
        painter->drawText(QPoint(option.rect.left(),
                                 bottomLine),
                          item->data(2).toString());
        // Draw collection/album/track length
        int textLength = painter->fontMetrics().width(item->data(3).toString()+" ");
        painter->drawText(QPoint((painter->viewport().width()-textLength),
                                 bottomLine),
                          item->data(3).toString());


        // if the item is nor selected or opened then draw it traditional way
    } else {
        //Remove index from list (if listed) and force it's redrawal
        if (m_currentIndexes.removeOne(mappedIndex)) {
            parentModel->redraw();
        }
        painter->drawText(QPoint(option.rect.left(),
                                 option.rect.top()+option.fontMetrics.height()),
                          item->data(0).toString());
    }
}

QSize CollectionItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //QModelIndex mappedIndex = m_proxyModel->mapToSource(index);
    QModelIndex mappedIndex = index;

    QSize size(option.decorationSize.width(),
               option.decorationSize.height()+2);
    // When the item is selected or opened, it will be higher
    if (m_currentIndexes.contains(mappedIndex)) {
        // Get height of the second row, which uses smaller font
        QFontMetrics fm(QFont(option.font.family(),8));
        // The height of item is height of first row + second row + padding + bottom margin
        size.setHeight(option.fontMetrics.height()+fm.height()+8);
    }

    return size;
}
