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

#ifndef PLAYLISTPROXYMODEL_H
#define PLAYLISTPROXYMODEL_H

#include <QIdentityProxyModel>
#include <QModelIndex>

class PlaylistProxyModel : public QIdentityProxyModel
{
    Q_OBJECT

  public:
    PlaylistProxyModel(QObject *parent = 0);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QModelIndex mapToSource(const QModelIndex &proxyIndex) const;

private Q_SLOTS:
    void onSourceDataChanged(const QModelIndex &sourceTopLeft,
                             const QModelIndex &sourceBottomRight,
                             const QVector<int> &roles);
};

#endif // PLAYLISTPROXYMODEL_H
