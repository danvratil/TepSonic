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

#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class PlaylistItem;

//! PlaylistModel is a list model that contains items in playlist
/*!
  PlaylistModel is subclassed from QAbstractItemModel. It's row-oriented because
  PlaylistBrowser is a QListView subclass.
  Each row has only one item and each item contains vector of 8 QVariants according
  to columns of the view.
*/
class PlaylistModel : public QAbstractItemModel
 {
     Q_OBJECT

 public:
     //! Constructor
     /*!
       Intializes the model and sets headers values
       \param headers list of column headers
       \parent parent object
     */
     PlaylistModel(const QStringList &headers, QObject *parent = 0);

     //! Destructor
     ~PlaylistModel();

     //! Provides access to data on given \p index
     /*!
       Returns value stored on place that \p index points to. The role must be
       either Qt::DisplayRole or Qt::EditRole
       \param index QModelIndex of cell whose value is reqested
       \param role role of required item. Only Qt::DisplayRole and Qt::EditRole are supported
       \return Returns QVariant with data on given \p index or empty QVariant if \p index or \p role are invalid
     */
     QVariant data(const QModelIndex &index, int role) const;

     //! Returns header data for given column
     /*!
       This method is called by the PlaylistBrowser and provides information about value
       and orientation of header for given section (column)
       \param section section (column) of header
       \param orientation orientation of the header. By default Qt::Horizontal is required
       \param role display role of the header. By default Qt::DisplayRole is required
       \return Returns QVariant with data for the given section
     */
     QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole) const;

     //! Returns index of item on given coordinates
     /*!
       Returns index of item on \p row and \p column relativley to given \p parent.
       \param row row of the item relatively to \p parent
       \param column column of the item relatively to \p parent
       \param parent parent item; in PlaylistModel the only valid parent is model root therefor
       this parameter is optional
       \return Returns QModelIndex of the item stored on given coords
     */
     QModelIndex index(int row, int column,
                       const QModelIndex &parent = QModelIndex()) const;

     //! Returns parent of given index
     /*!
       Returns index of parent of given index. In this model this method will always
       return index of root item.
       \param index index of item whose parent we want to get
       \return Returns QModelIndex of \p index's parent
     */
     QModelIndex parent(const QModelIndex &index) const;

     //! Returns number of rows of given parent
     /*!
       Returns number of rows (children) of given \p parent.
        \param parent parent item whose children count will be returned. In PlaylistModel there is no other
        parent then root item, therefor this parameter is optional
        \return Returns number of rows
     */
     int rowCount(const QModelIndex &parent = QModelIndex()) const;

     //! Returns number of columns of given parent
     /*!
       Returns number of columns of given \p parent.
        \param parent parent item whose column count will be returned. In PlaylistModel there is no other
        parent then root item and all items have the same column count, therefor this parameter is optional
        \return Returns number of columns
     */
     int columnCount(const QModelIndex &parent = QModelIndex()) const;

     //! Returns flags of given item
     /*!
       Returns flags of given item.
       \param index index of item whose flags will be returned
       \return Returns flags of given item.
     */
     Qt::ItemFlags flags(const QModelIndex &index) const;

     //! Sets data of item on given index
     /*!
       Sets data to \p value on given \p index. The item must have the proper \p role.
       \param index index of item to change data in
       \param value new data
       \param role must be Qt::EditRole (or empty)
       \return Returns true on success, false when fails
     */
     bool setData(const QModelIndex &index, const QVariant &value,
                  int role = Qt::EditRole);

     //! Sets header data for given section (column)
     /*!
       Sets data for given column header
       \param section section (column) number
       \param orientation orientation of the header (Qt::Horizontal by default)
       \param value the value to be set
       \param role role of the item, Qt::EditRole by default
       \return Returns true on success. false when fails.
     */
     bool setHeaderData(int section, Qt::Orientation orientation,
                        const QVariant &value, int role = Qt::EditRole);

     //! Inserts given number of rows into to playlist
     /*!
       Inserts given number of empty rows to playlist. First row will be inserted to
       \p position.
       \param position position of first item to insert
       \param rows number of rows to insert
       \param parent new items will be children of this parent. By default PlaylistModel has only
       rootItem as parent item and therefor this parameter is optional
       \return Returns true on sucess, false when fails
     */
     bool insertRows(int position, int rows,
                     const QModelIndex &parent = QModelIndex());

     //! Removes given number of rows from to playlist
     /*!
       Removes given number of rows from playlist. First row will be removed from
       \p position.
       \param position position of first item to be removed
       \param rows number of rows to remove
       \param parent items will be removed from given parent. By default PlaylistModel has only root item as a parent
       item and therefor this parameter is optional
       \return Returns true on sucess, false when fails
     */
     bool removeRows(int position, int rows,
                     const QModelIndex &parent = QModelIndex());

 public slots:
    //! Appends new file to the playlist
    /*!
      First a new row is appended to the end of the playlist and then meta data are loaded using
      TagLib library and inserted into columns.
      \param file track file to load
      \sa PlaylistManager
    */
    bool addItem(QString file);

 private:
     //! Returns pointer to PlaylistModel located on given index
     /*!
       \param index index of item to convert
       \return Returns pointer to PlaylistModel
     */
     PlaylistItem *getItem(const QModelIndex &index) const;

     //! Root item. It's parent for all rows
     PlaylistItem *rootItem;

     //! Total length of playlist in seconds
     int _totalLength;

 signals:
     //! This signal is emmited when length of playlist is changed
     /*!
       \param totalLength total length of playlist in seconds
       \param tracksCount number of tracks in playlist
     */
     void playlistLengthChanged(int totalLength, int tracksCount);

 };


#endif // PLAYLISTMODEL_H