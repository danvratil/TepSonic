/*
 * TEPSONIC
 * Copyright 2009 Dan Vratil <vratil@progdansoft.com>
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

#include <QtGui>
#include <QTime>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlField>
#include <QtSql/QSqlQuery>


#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>

#include "playlistmodel.h"
#include "playlistitem.h"
#include "databasemanager.h"
#include "tools.h"


PlaylistModel::PlaylistModel(const QStringList &headers, QObject *parent)
        : QAbstractItemModel(parent)
{
    QVector<QVariant> rootData;
    foreach (QString header, headers)
    rootData << header;

    _totalLength = 0;

    rootItem = new PlaylistItem(rootData);
}

PlaylistModel::~PlaylistModel()
{
    delete rootItem;
}

int PlaylistModel::columnCount(const QModelIndex & /* parent */) const
{
    return rootItem->columnCount();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    PlaylistItem *item = getItem(index);

    return item->data(index.column());
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

PlaylistItem *PlaylistModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        PlaylistItem *item = static_cast<PlaylistItem*>(index.internalPointer());
        if (item) return item;
    }
    return rootItem;
}

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    PlaylistItem *parentItem = getItem(parent);

    PlaylistItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

bool PlaylistModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    PlaylistItem *parentItem = getItem(parent);
    bool success;

     _mutex.lock();

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();

     _mutex.unlock();

    return success;
}

QModelIndex PlaylistModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    PlaylistItem *childItem = getItem(index);
    PlaylistItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool PlaylistModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    PlaylistItem *parentItem = getItem(parent);
    bool success = true;

    int totalRemoveTime = 0;
    for (int i=position;i<position+rows;i++) {
        QString trackLength = index(i,7,QModelIndex()).data().toString();
        if (trackLength.length()==5) {
            trackLength.prepend("00:");
        }
        QTime tl(0,0,0,0);
        totalRemoveTime += tl.secsTo(QTime::fromString(trackLength,"hh:mm:ss"));
    }

     _mutex.lock();

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

     _mutex.unlock();

    if (success) {
        // Decrease total length of the playlist by total length of removed tracks
        _totalLength -= totalRemoveTime;
        emit playlistLengthChanged(_totalLength,rowCount(QModelIndex()));
    }

    return success;
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    PlaylistItem *parentItem = getItem(parent);

    return parentItem->childCount();
}

bool PlaylistModel::setData(const QModelIndex &index, const QVariant &value,
                            int role)
{
    if (role != Qt::EditRole)
        return false;

    PlaylistItem *item = getItem(index);

     _mutex.lock();

    bool result = item->setData(index.column(), value);

     _mutex.unlock();

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool PlaylistModel::setHeaderData(int section, Qt::Orientation orientation,
                                  const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

     _mutex.lock();

    bool result = rootItem->setData(section, value);

     _mutex.unlock();

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

bool PlaylistModel::addItem(QString file)
{
    if (file.isEmpty())
        return false;

    QFileInfo finfo(file);
    if ((!finfo.exists()) || (!finfo.isFile()))
        return false;

    QString filename = "";
    uint track;
    QString title;
    QString interpret;
    QString album;
    QString genre;
    uint year;
    QTime length(0,0,0,0);
    uint lengthSeconds;

    // Just a harmless check wheter the file is in DB - reading data from DB will be faster then from file
    DatabaseManager dbManager("playlistModelConnection");
    if (dbManager.connectToDB()) {
        QSqlField data("col",QVariant::String);
        data.setValue(file);
        QString fname = dbManager.sqlDb()->driver()->formatValue(data,false);
        QSqlQuery query("SELECT `filename`," \
                        "       `trackname`," \
                        "       `track`," \
                        "       `length`," \
                        "       `interpret`," \
                        "       `genre`," \
                        "       `album`," \
                        "       `year`" \
                        "FROM `view_tracks` "\
                        "WHERE `filename`="+fname+ \
                        "LIMIT 1;",
                        *dbManager.sqlDb());
        if (query.first()) {
            filename = query.value(0).toString();
            title = query.value(1).toString();
            track = query.value(2).toUInt();
            lengthSeconds = query.value(3).toUInt();
            interpret = query.value(4).toString();
            genre = query.value(5).toString();
            album = query.value(6).toString();
            year = query.value(7).toUInt();
        }
     }

    if (filename.isEmpty()) {
        TagLib::FileRef f(file.toUtf8().constData());

        filename = file.toUtf8().constData();
        title = f.tag()->title().toCString(true);
        track = f.tag()->track();
        interpret = f.tag()->artist().toCString(true);
        lengthSeconds = f.audioProperties()->length();
        album = f.tag()->album().toCString(true);
        genre = f.tag()->genre().toCString(true);
        year = f.tag()->year();

        if (title.isEmpty())
            title = finfo.fileName();
    }

    // And length of the track to the total length of the playlist
    _totalLength += lengthSeconds;
    QString trackLengthString = formatMilliseconds(lengthSeconds*1000);

    // Select the root item
    QModelIndex root;


    // Insert new row
    if (!insertRow(rowCount(root), root))
        return false;

    rootItem->child(rootItem->childCount()-1)->setData(0,QVariant(filename));
    rootItem->child(rootItem->childCount()-1)->setData(1,QVariant(track));
    rootItem->child(rootItem->childCount()-1)->setData(2,QVariant(interpret));
    rootItem->child(rootItem->childCount()-1)->setData(3,QVariant(title));
    rootItem->child(rootItem->childCount()-1)->setData(4,QVariant(album));
    rootItem->child(rootItem->childCount()-1)->setData(5,QVariant(genre));
    rootItem->child(rootItem->childCount()-1)->setData(6,QVariant(year));
    rootItem->child(rootItem->childCount()-1)->setData(7,QVariant(trackLengthString));

    emit playlistLengthChanged(_totalLength, rowCount(QModelIndex()));

    return true;
}
