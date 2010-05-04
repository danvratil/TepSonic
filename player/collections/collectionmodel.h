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

#ifndef COLLECTIONMODEL_H
#define COLLECTIONMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class CollectionItem;

//! CollectionModel is a model that contains tree of \p CollectionItems displayed in CollectionBrowser
/*!
  CollectionModel is subclassed from QAbstractItemModel and provides default API for CollectionBrowser to
  display the tree.
  By default the model has two column and the tree structure has three levels. The first column contains names
  of interprets, albums or tracks. The second column contains filename of the track and is hidden. The filename
  is set only for the third level, eg. tracks.
*/
class CollectionModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    //! Constructor
    /*!
      Initializes the model. Recieves list of header.
      \param headers list of headers for columns (two by default)
      \param parent pointer to model's parent
    */
    CollectionModel(const QStringList &headers, QObject *parent = 0);

    //! Destructor
    ~CollectionModel();

    //! Provides data on given \p index
    /*!
      Returns data in QVariant that are located on given \p index (QModelIndex)
      \param index index that points to item that data are stored in
      \param role role of the data
      \return Returns data in QVariant or empty QVariant when invalid index is given
    */
    QVariant data(const QModelIndex &index, int role) const;

    //! Returns header data in column \p section with given orientation and role.
    /*!
      Returns QVariant with data in header in column \p section. Data must have \p Qt::Horizontal orientation
      and \p Qt::DisplayRole by default
      \param section number of column from which to obtain the data
      \param orientation orientation of the header
      \param role role of the header item
      \return Returns data in QVariant or empty QVariant when invalid \p section, \p orientation or \p role is given.
    */
    QVariant headerData(int section, Qt::Orientation orientation = Qt::Horizontal, int role = Qt::DisplayRole) const;

    //! Returns QModelIndex of item
    /*!
      Returns index of item with coordinates \p row and \p column relative to \p parent.
      \param row row of item relatively to \p parent
      \param column column of item relatively to \p parent
      \param parent index of parent item. When empty or invalid QModelIndex is passed \p row and \p column are taken relatively to model root item
      \return Returns QModelIndex that points to item on given coordinates relatively to given parent.
    */
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    //! Returns parent of given item
    /*!
      Returns QModelIndex of parent of given item
      \param index QModelIndex of item whose parent will be returned
      \return Returns QModelIndex of parent of given item
    */
    QModelIndex parent(const QModelIndex &index) const;

    //! Returns number of parent's rows
    /*!
      Returns number of parent's children (rows). This does not include children's children.
      \param parent QModelIndex of parent item
      \return Returns number of parent's children.
    */
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    //! Returns number of columns of given item
    /*!
      Returns number of columns of given item.
      \param parent QModelIndex of item
      \return Returns number of columns of \p parent
    */
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    //! Returns flags of given item
    /*!
      Returns Qt::ItemFlags of item on given index
      \param index QModelIndex of the item
      \return Returns bool enumeration of item's Qt::ItemFlags
    */
    Qt::ItemFlags flags(const QModelIndex &index) const;

    //! Sets data to given item
    /*!
      Sets data to given item on index \p index. Data's role can be specified too.
      \param index QModelIndex of item
      \param value value to be set to the item
      \param role role of the data
      \return Returns true on success, false when fails (invalid index for example)
    */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    //! Sets header data
    /*!
      Sets header data to column \p section. It should have Qt::Horizontal orientation and Qt::EditRole role.
      When data are set sucessfully, headerDataChanged() signal is emitted
      \param section column for which new data will be set
      \param orientation orientation of the header (must he Qt::Horizontal)
      \param value data to be set
      \param role role of the data (must he Qt::EditRole)
      \return Returns true when successful, false when fails
      \sa headerDataChanged()
    */
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);

    //! Inserts new children to given parent
    /*!
      Inserts given number of rows on given position to given parent
      \param position position of first item to be inserted to
      \param rows number of rows to be inserted
      \param parent QModelIndex of parent to which the new children will be added
      \return Returns true on success, false when fails
    */
    bool insertRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());

    //! Removes children from given parent
    /*!
      Removes given number of rows beginning from given position. The children are taken from given parent.
      This includes removal of children's children.
      \param position position of first item to be removed
      \param rows number of rows to be removed
      \param parent QModelIndex of item from which the items will be removed
      \return Returns true on success, false when fails
    */
    bool removeRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());

    //! Comfortible function to append child to the tree structure
    /*!
      Appends new child to given \p index and sets \p title to first column and \p filename
      to second column.
      CollectionModel has three levels - artists, albums and tracks. Only the last level must
      have \p filename set. For the first two levels it's optional because it's not used.
      \param index QModelIndex of parent the new children will be appended to
      \param title title of the children (value of the first column) with name of artist/ablum/track
      \param filename name of file with a song. Mandatory only for tracks.
      \return Returns QModelIndex of created child.
    */
    QModelIndex addChild(const QModelIndex &index, QString title, QString filename = "");

    //! Remove all items and their children from model
    /*!
      This is the best method how to remove all items from collections
    */
    void clear();

protected:
    //! Returns list of accepted drop acctions.
    /*!
      \return Returns Qt::CopyAction and Qt::MoveAction
    */
    Qt::DropActions supportedDropActions() const;

private:
    //! Converts QModelIndex to CollectionItem
    /*!
      Returns CollectionItem to which given QModelIndex points
      \param index QModelIndex of item
      \return Returns pointer to CollectionItem the given QModelIndex points on
    */
    CollectionItem *getItem(const QModelIndex &index) const;

    //! Pointer to root CollectionItem
    CollectionItem *_rootItem;
};


#endif // COLLECTIONMODEL_H
