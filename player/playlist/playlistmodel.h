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


#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QReadWriteLock>
#include <QStringList>

#include "player.h"

class PlaylistProxyModel;

class PlaylistModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    enum Columns {
        FilenameColumn = 0,
        TrackColumn = 1,
        InterpretColumn = 2,
        TracknameColumn = 3,
        AlbumColumn = 4,
        GenreColumn = 5,
        YearColumn = 6,
        LengthColumn = 7,
        BitrateColumn = 8,
        ColumnCount
    };

    enum Roles {
        MetaDataRole = Qt::UserRole + 1
    };

    PlaylistModel(QObject *parent = 0);
    ~PlaylistModel();

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);
    QVariant data(const QModelIndex &index, int role) const;

    void savePlaylist(const QString &file);
    void loadPlaylist(const QString &file);

  public Q_SLOTS:
    void addFile(const QString &file);
    void addFiles(const QStringList &files);

    void insertFiles(const QStringList &files, int row);
    void insertItem(const MetaData &metadata, int row);

    void clear();

  Q_SIGNALS:
    void playlistLengthChanged(int totalLength, int tracksCount);

  private Q_SLOTS:
    void onMetaDataAvailable(int beginIndex, int endIndex);
    void onMetaDataDone();

  private:
    void loadMetaDataRunnable(const MetaData::List::Iterator &start,
                              const MetaData::List::Iterator &end);

    MetaData::List m_items;
    mutable QReadWriteLock *m_itemsLock;

    // total length in seconds
    int m_totalLength;


};


#endif // PLAYLISTMODEL_H
