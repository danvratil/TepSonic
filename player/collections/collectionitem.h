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


#ifndef COLLECTIONITEM_H
#define COLLECTIONITEM_H

#include <QStandardItem>
#include <QList>
#include <QVariant>
#include <QVector>

//! CollectionItem is a item in collections tree structure
/*!
  CollectionItem is subclassed from QStandardItem and provides
  default methods to create tree structure.
  It holds it's children in childItems and data for all columns
  in itemData.
*/
class CollectionItem: public QStandardItem
{
public:

    //! Constructor
    /*!
      Initialize new item with data and parent. When no data are given, an empty item is created and displayed.
      Empty item may not be visible if user's window manager's style does not highlight items in views.
      \param data set of data (in various data types) to be displayed in columns
      \param parent parent of the item. When creating root item for a model, the parent should be left 0. When
      creating item that is child of another item the parent should be present. It enabled traversing through
      the structure.
    */
    CollectionItem(const QVector<QVariant> &data, CollectionItem *parent = 0);

    //! Destructor
    ~CollectionItem();

    //! Returns n-th child of current item
    /*!
      Returns n-th child of current item. Children are indexed from 0. When given index is higher then index
      of the last child, null pointer is returned.
      \param index index of child
      \return Pointer to matching CollectionItem or null pointer
    */
    CollectionItem *child(int index);

    //! Returns count of item's children
    /*!
      Returns count of current item's children. It does not include children's children.
      \return Count of children items
    */
    int childCount() const;

    //! Returns count of item's columns
    /*!
      Returns number of item's columns. By default this method returns 2
      \return Retruns number of item's columns.
    */
    int columnCount() const;

    //! Returns data in n-th column
    /*!
      Returns QVariant with data stored in given column. When invalid column number is given
      empty QVariant is returned.
      \param column Index of column
      \return Data from given column
    */
    QVariant data(int column) const;

    //! Inserts empty children
    /*!
      Inserts \p count children with \p columns number of columns to \p position position
      \param position index of first child to be inserted
      \param count number of children to be insrted
      \param columns number of columns the children will have
      \return Returns true on success, false when fails
    */
    bool insertChildren(int position, int count, int columns);

    //! Returns parent CollectionItem
    /*!
      Parent is read from property set in constructor. If no parent is set, a null pointer
      is returned.
      \return Pointer to parent CollectionItem or null pointer
      \sa CollectionItem()
    */
    CollectionItem *parent();

    //! Removes item's children
    /*!
      Removes \p count of children from \p position index to \p position + \p count.
      \param position index of first child to remove
      \param count number of children to remove
      \return Returns true on success, false when fails
    */
    bool removeChildren(int position, int count);

    //! Sets item's data in given column
    /*! Sets data in column \p column to \p value.
      \param column column to set data in
      \param value data to be set in given column
      \return Returns true on success, false when fails (for example when invalid column number is given)
    */
    bool setData(int column, const QVariant &value);

    //! Returns index of this children relatively to it's parent
    /*! Returns index (row number) of this item relatively to it's parent item
      \return Returns this item's row number relatively
    */
    int childNumber() const;

private:
    //! List of children
    QList<CollectionItem*> _childItems;

    //! List of values in columns
    QVector<QVariant> _itemData;

    //! Pointer to parent item
    CollectionItem *_parentItem;
};
#endif // COLLECTIONITEM_H
