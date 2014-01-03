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

#include "playlistproxymodel.h"

#include <core/playlist.h>
#include <core/player.h>

Q_DECLARE_METATYPE(TepSonic::MetaData)

using namespace TepSonic;

PlaylistProxyModel::PlaylistProxyModel(QObject *parent):
    QIdentityProxyModel(parent)
{
    setSourceModel(Player::instance()->playlist());
}

Qt::ItemFlags PlaylistProxyModel::flags(const QModelIndex &index) const
{
    const QModelIndex mappedProxyIndex = index.sibling(index.row(), 0);
    return QIdentityProxyModel::flags(mappedProxyIndex);
}

int PlaylistProxyModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 9;
}

QVariant PlaylistProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return tr("Filename");
            case 1:
                return tr("Track Number");
            case 2:
                return tr("Artist");
            case 3:
                return tr("Track Title");
            case 4:
                return tr("Album");
            case 5:
                return tr("Genre");
            case 6:
                return tr("Year");
            case 7:
                return tr("Length");
            case 8:
                return tr("Bitrate");
        }
    }

    return QIdentityProxyModel::headerData(section, orientation, role);
}

QVariant PlaylistProxyModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    const QModelIndex sourceIndex = sourceModel()->index(index.row(), 0);
    const MetaData metaData = sourceModel()->data(sourceIndex, Playlist::MetaDataRole).value<MetaData>();
    switch (index.column()) {
        case 0:
            return metaData.fileName();
        case 1:
            return metaData.trackNumber();
        case 2:
            return metaData.artist();
        case 3:
            return metaData.title();
        case 4:
            return metaData.album();
        case 5:
            return metaData.genre();
        case 6:
            return metaData.year();
        case 7:
            return metaData.formattedLength();
        case 8:
            return QString::fromLatin1("%1 kbps").arg(metaData.bitrate());
    }

    return QVariant();
}

bool PlaylistProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::DisplayRole) {
        return QIdentityProxyModel::setData(index, value, role);
    }

    const QModelIndex sourceIndex = sourceModel()->index(index.row(), 0);
    MetaData metaData = sourceModel()->data(sourceIndex, Playlist::MetaDataRole).value<MetaData>();
    switch (index.column()) {
        case 0:
            metaData.setFileName(value.toString());
            break;
        case 1:
            metaData.setTrackNumber(value.toUInt());
            break;
        case 2:
            metaData.setArtist(value.toString());
            break;
        case 3:
            metaData.setTitle(value.toString());
            break;
        case 4:
            metaData.setAlbum(value.toString());
            break;
        case 5:
            metaData.setGenre(value.toString());
            break;
        case 6:
            metaData.setYear(value.toUInt());
            break;
        case 7:
            metaData.setLength(value.toLongLong());
            break;
        case 8:
            metaData.setBitrate(value.toInt());
            break;
    }

    return sourceModel()->setData(sourceIndex, QVariant::fromValue(metaData), Playlist::MetaDataRole);
}

QModelIndex PlaylistProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (proxyIndex.column() > 0) {
        return QModelIndex();
    }

    return QIdentityProxyModel::mapToSource(proxyIndex);
}
