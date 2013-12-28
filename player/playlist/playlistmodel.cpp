/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <dan@progdan.cz>
 * Copyright 2009 Petr Los <petr_los@centrum.cz>
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


#include "playlistmodel.h"
#include "playlistproxymodel.h"
#include "player.h"
#include "tools.h"

#include <QtCore/QDebug>
#include <QtCore/QTime>

class PlaylistModel::Node
{
  public:
    Node():
        track(0),
        year(0),
        duration(0),
        bitrate(0),
        randomOrder(0)
    {
    }

    QString filename;
    QString interpret;
    QString trackname;
    QString album;
    QString genre;
    QString formattedLength;
    int track;
    int year;
    int duration;
    int bitrate;
    int randomOrder;
};

PlaylistModel::PlaylistModel(QObject *parent) :
    QAbstractItemModel(parent),
    m_totalLength(0)
{
}

PlaylistModel::~PlaylistModel()
{
    qDeleteAll(m_items);
}

int PlaylistModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return PlaylistModel::ColumnCount;
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.column() < PlaylistModel::ColumnCount);

    if (!index.isValid()) {
        return QVariant();
    }

    Node *node = m_items.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case FilenameColumn:
                return node->filename;
            case TrackColumn:
                return node->track;
            case InterpretColumn:
                return node->interpret;
            case TracknameColumn:
                return node->trackname;
            case AlbumColumn:
                return node->album;
            case GenreColumn:
                return node->genre;
            case YearColumn:
                return node->year;
            case LengthColumn:
                return node->formattedLength;
            case BitrateColumn:
                return QString::fromLatin1("%1 kbps").arg(node->bitrate);
            case RandomOrderColumn:
                return node->randomOrder;
        }
    }

    return QVariant();
}

bool PlaylistModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(role);
    Q_ASSERT(index.model() == this);

    Node *node = m_items.at(index.row());

    switch (index.column()) {
        case FilenameColumn:
            node->filename = value.toString();
            break;
        case TrackColumn:
            node->track = value.toInt();
            break;
        case InterpretColumn:
            node->interpret = value.toString();
            break;
        case TracknameColumn:
            node->trackname = value.toString();
            break;
        case AlbumColumn:
            node->album = value.toString();
            break;
        case GenreColumn:
            node->genre = value.toString();
            break;
        case YearColumn:
            node->year = value.toInt();
            break;
        case LengthColumn:
            node->duration = value.toInt();
            break;
        case BitrateColumn:
            node->bitrate = value.toInt();
            break;
        case RandomOrderColumn:
            node->randomOrder = value.toInt();
            break;
    }

    return true;
}


Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const
{
    Q_ASSERT(index.model() == this);

    if (!index.isValid()) {
        return 0;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const
{
    if (role == Qt::DisplayRole) {
        switch (section) {
            case FilenameColumn:
                return tr("Filename");
            case TrackColumn:
                return tr("Track");
            case InterpretColumn:
                return tr("Interpret");
            case TracknameColumn:
                return tr("Track Name");
            case AlbumColumn:
                return tr("Album");
            case GenreColumn:
                return tr("Genre");
            case YearColumn:
                return tr("Year");
            case BitrateColumn:
                return tr("Bitrate");
            case LengthColumn:
                return tr("Length");
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return createIndex(row, column);
}

QModelIndex PlaylistModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);

    return QModelIndex();
}

bool PlaylistModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    if (rows == 0) {
        return true;
    }

    int totalRemoveTime = 0;
    beginRemoveRows(parent, position, position + rows - 1);
    for (int i = 0; i < rows; i++) {
        Node *node = m_items.takeAt(position);
        totalRemoveTime += node->duration / 1000;
        delete node;
    }
    endRemoveRows();

    // Decrease total length of the playlist by total length of removed tracks
    m_totalLength -= totalRemoveTime;
    Q_EMIT playlistLengthChanged(m_totalLength, rowCount(QModelIndex()));

    return true;
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_items.count();
    }

    return 0;
}

void PlaylistModel::insertItem(const Player::MetaData &metadata, int row)
{
    if (row < 0) {
        row = 0;
    }

    Node *node =  new Node();
    node->filename = metadata.filename;
    node->track = metadata.trackNumber;
    node->interpret = metadata.artist;
    node->trackname = metadata.title;
    node->album = metadata.album;
    node->genre = metadata.genre;
    node->year = metadata.year;
    node->duration = metadata.length;
    node->formattedLength = metadata.formattedLength;
    node->duration = metadata.bitrate;

    beginInsertRows(QModelIndex(), row, row);
    m_items.insert(row, node);
    endInsertRows();

    m_totalLength += (metadata.length / 1000);
    Q_EMIT playlistLengthChanged(m_totalLength, rowCount(QModelIndex()));
}

void PlaylistModel::clear()
{
    beginResetModel();
    qDeleteAll(m_items);
    m_items.clear();
    endResetModel();
}
