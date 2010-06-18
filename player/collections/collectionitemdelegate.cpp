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
    painter->setFont(QFont(painter->font().family(),8));
    painter->setPen(option.palette.text().color());
    CollectionModel *parentModel = const_cast<CollectionModel*>(dynamic_cast<const CollectionModel*>(index.model()));
    CollectionItem *item = static_cast<CollectionItem*>(index.internalPointer());

    if ((option.state & QStyle::State_Selected) ||
        (option.state & QStyle::State_Open)) {

        QModelIndex moi = index;
        if (!_currentIndexes.contains(moi)) {
            _currentIndexes.append(moi);
            parentModel->redraw();
        }

        if (option.state & QStyle::State_Selected) {
            QRect rect(option.rect);
            rect.setLeft(rect.left()-3);
            painter->fillRect(rect,option.palette.highlight());
        }

        painter->drawText(QPoint(option.rect.left(),
                                 option.rect.top()+10),
                          item->data(0).toString());

        painter->setFont(QFont(painter->font().family(),7));
        painter->setPen(Qt::gray);
        painter->drawText(QPoint(option.rect.left(),
                                 option.rect.top()+25),
                          item->data(2).toString());
        painter->drawText(QPoint((painter->viewport().width()-painter->fontMetrics().width(item->data(3).toString())-8),
                                 option.rect.top()+25),
                          item->data(3).toString());


    } else {
        // Remove index from list (if it is in it) <= cool :D
        if (_currentIndexes.removeOne(index)) {
            parentModel->redraw();
        }
        painter->drawText(QPoint(option.rect.left(),
                                 option.rect.top()+10),
                          item->data(0).toString());
    }

}

QSize CollectionItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size(option.decorationSize.width(),option.decorationSize.height());
    if (_currentIndexes.contains(index)) {
        size.setHeight(30);
    }

    return size;
}
