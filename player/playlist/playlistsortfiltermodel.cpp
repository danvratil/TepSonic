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

#include "playlistsortfiltermodel.h"
#include "playlistproxymodel.h"

#include <QDebug>

PlaylistSortFilterModel::PlaylistSortFilterModel(QObject* parent):
    QSortFilterProxyModel(parent)
{
    setSourceModel(new PlaylistProxyModel(this));
    setFilterRole(Qt::DisplayRole);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setDynamicSortFilter(false);
}


bool PlaylistSortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);

    if (filterRegExp().isEmpty()) {
        return true;
    }

    for (int i = 0; i < columnCount(); ++i) {
        const QModelIndex idx = sourceModel()->index(sourceRow, i);
        if (idx.data().toString().contains(filterRegExp())) {
            return true;
        }
    }

    return false;
}
