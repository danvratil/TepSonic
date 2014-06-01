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


#ifndef TEPSONIC_PLAYLIST_H
#define TEPSONIC_PLAYLIST_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QReadWriteLock>
#include <QStringList>

#include <core/metadata.h>

#include "tepsonic-core-export.h"

namespace TepSonic
{

class TEPSONIC_CORE_EXPORT Playlist : public QAbstractItemModel
{
    Q_OBJECT

  public:
    enum Roles {
        MetaDataRole = Qt::UserRole + 1
    };

    Playlist(QObject *parent = 0);
    ~Playlist();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant data(const QModelIndex &index, int role) const;

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                  const QModelIndex &destinationParent, int destinationChild);

    void savePlaylist(const QString &file);
    void loadPlaylist(const QString &file);

    void insert(const QStringList &files, int row = -1);
    void insert(const MetaData::List &metaData, int row = -1);

    void remove(int start, int count = 1);

    void move(int start, int count, int newStart);

    void setTrack(int index, const MetaData &metaData);
    MetaData track(int index) const;

    int totalLength() const;

  public Q_SLOTS:
    void shuffle();
    void clear();

  Q_SIGNALS:
    void playlistLengthChanged(int totalLength, int tracksCount);

  private:
    class Private;
    Private * const d;
    friend class Private;

};

} // namespace TepSonic

#endif // TEPSONIC_PLAYLIST_H
