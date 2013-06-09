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

#include "collectionproxymodel.h"

#include <QDebug>

CollectionProxyModel::CollectionProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

/**
 * Add recursively all children of given root item to the list
 */
QModelIndexList CollectionProxyModel::getChildren(const QModelIndex item) const
{
    QModelIndexList result;

    if (sourceModel()->hasChildren(item)) {
        for (int i = 0; i < sourceModel()->rowCount(item); i++) {
            const QModelIndex child = item.child(i, 0);
            if (child.isValid()) {
                result << child;
                if (sourceModel()->hasChildren(child)) {
                    result << getChildren(child);
                }
            }
        }
    }

    return result;
}

bool CollectionProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    // Current item
    const QModelIndex item = sourceModel()->index(sourceRow, 0, sourceParent);

    if (!item.isValid())
        return false;

    /* First check, if the parent is visible. If it is and it matches the filter
       then this item MUST be displayed (all children of valid parent must be visible) */
    if (item.parent().isValid())
        if (item.parent().data().toString().contains(filterRegExp()))
            return true;

    /* If the parent item is not valid or does not suite the filter (previous conditions were not met) then
       check if this item fits the filter. */
    if (item.data().toString().contains(filterRegExp()))
        return true;

    /* Finally, if none the previous conditions was met, the creates a QList of all of this item's
       children and if any of them matches the current filter. In that case this item must stay
       visible */
    const QModelIndexList children = getChildren(item);

    // Go through all the children and if any of them matches then set canHide to false
    for (int i = 0; i < children.count(); i++) {
        if (children.at(i).data().toString().contains(filterRegExp()))
            return true;
    }

    // If any of the previous rules was not applied then the item must be hidden
    return false;

}
