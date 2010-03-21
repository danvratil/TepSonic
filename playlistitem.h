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

#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QStandardItem>
#include <QList>
#include <QVariant>
#include <QVector>

//! PlaylistItem is a model item that represents one row in PlaylistModel
/*!
  PlaylistItem is subclassed from QStandardItem. It represents entire row in PlaylistModel
  by storing values of all columns in itemData vector.
*/
class PlaylistItem: public QStandardItem
 {
 public:
     //! Constructor
     /*!
       Initializes the item
       \param data vector that contains data for all columns
       \param parent pointer to parent item
     */
     PlaylistItem(const QVector<QVariant> &data, PlaylistItem *parent = 0);

     //! Destructor
     ~PlaylistItem();

     //! Returns pointer to n-th item's child
     /*!
       \param number number of child to be returned
       \return Returns pointer to matching PlaylistItem or null pointer.
     */
     PlaylistItem *child(int number);

     //! Returns number of item's children
     /*!
       \return Returns number of item's children.
     */
     int childCount() const;

     //! Returns number of item's columns
     /*!
       \return Returs number of item's columns. It should be 8 by default
     */
     int columnCount() const;

     //! Returns item's data for given column
     /*!
       \param column column of which data are retrieved
       \return Returns QVariant with column data.
     */
     QVariant data(int column) const;

     //! Appends new children to this item
     /*!
       Appends PlaylistItem to list of item's children.
       \param item pointer to new PlaylistItem to be appended
     */
     void appendChild(PlaylistItem *item);

     //! Inserts given number of rows and column beginning on given row
     /*!
       Inserts \p count items with \columns columns to list of item's children from \p position row.
       \param position position where to insert first row
       \param count number of rows to insert
       \param columns number of columns the item will have
       \return Returns true on success, false when fails.
     */
     bool insertChildren(int position, int count, int columns);

     //! Returns pointer to item's parent
     /*!
       \return Returns pointer to item's parent or null pointer if item has no parent.
     */
     PlaylistItem *parent();

     //! Removes item's children
     /*!
       Removes given number of item's children beginning with child on \p position position.
       \param position position of first item to be removed
       \param count number of rows to be removed
       \return Returns true on success, false when fails.
     */
     bool removeChildren(int position, int count);

     //! Returns number of current item relatively to it's parent.
     /*!
       Returns index (number) of current item relatively to it's parent
       \return Returns number of current item.
     */
     int childNumber() const;

     //! Sets given data for given column
     /*!
       Sets value of \p column to \value.
       \param column number of column to set data in
       \param value data to be set
       \return Returns true on success, false when fails.
      */
     bool setData(int column, const QVariant &value);

 private:
     //! List of item's child PlaylistItems
     QList<PlaylistItem*> childItems;

     //! Vector with data for single columns
     QVector<QVariant> itemData;

     //! Pointer to parent PlaylistItem
     PlaylistItem *parentItem;
 };
#endif // PLAYLISTITEM_H
