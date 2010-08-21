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

#include <QApplication>
#include <QFont>
#include <QPoint>


#include <QDebug>

CollectionItemDelegate::CollectionItemDelegate(QObject *parent):
        QStyledItemDelegate(parent)
{

}

void CollectionItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setFont(option.font);
    painter->setPen(option.palette.text().color());
    CollectionModel *parentModel = const_cast<CollectionModel*>(dynamic_cast<const CollectionModel*>(index.model()));
    CollectionItem *item = static_cast<CollectionItem*>(index.internalPointer());

    // If item is selected or opened
    if ((option.state & QStyle::State_Selected) ||
        (option.state & QStyle::State_Open)) {

        /* If item is selected or opened but not in list of selected/opened items then
           add it to the list and force redrawing it */
        QModelIndex moi = index;
        if (!_currentIndexes.contains(moi)) {
            _currentIndexes.append(moi);
            parentModel->redraw();
        }

        // It item is selected then draw highlighted background
        if (option.state & QStyle::State_Selected) {
            QRect rect(option.rect);
            rect.setLeft(rect.left()-3);
            painter->fillRect(rect,option.palette.highlight());
        }

        // Draw first row containing the interpret/album/track name
        painter->drawText(QPoint(option.rect.left(),
                                 option.rect.top()+option.fontMetrics.height()),
                          item->data(0).toString());

        painter->setFont(QFont(option.font.family(),8));
        painter->setPen(Qt::gray);
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
        // Remove index from list (if it is in it)
        //                              ^-------- cool :D
        if (_currentIndexes.removeOne(index)) {
            parentModel->redraw();
        }
        painter->drawText(QPoint(option.rect.left(),
                                 option.rect.top()+option.fontMetrics.height()),
                          item->data(0).toString());
    }

}

QSize CollectionItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size(option.decorationSize.width(),
               option.decorationSize.height()+2);
    // When the item is selected or opened, it will be higher
    if (_currentIndexes.contains(index)) {
        // Get height of the second row, which uses smaller font
        QFontMetrics fm(QFont(option.font.family(),8));
        // The height of item is height of first row + second row + padding + bottom margin
        size.setHeight(option.fontMetrics.height()+fm.height()+8);
    }

    return size;
}
