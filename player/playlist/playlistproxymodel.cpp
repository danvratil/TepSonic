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

#include "playlistproxymodel.h"

PlaylistProxyModel::PlaylistProxyModel(QObject *parent):
    QSortFilterProxyModel(parent)
{
    setFilterRole(Qt::EditRole);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setDynamicSortFilter(false);
}

bool PlaylistProxyModel::filterAcceptsRow(int sourceRow,
        const QModelIndex &sourceParent) const
{
    for (int col = 1; col <= 6; col++) {
        const QModelIndex index = sourceModel()->index(sourceRow, col, sourceParent);
        if (sourceModel()->data(index).toString().contains(filterRegExp())) {
            return true;
        }
    }
    return false;
}
