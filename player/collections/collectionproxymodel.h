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

#ifndef COLLECTIONPROXYMODEL_H
#define COLLECTIONPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QModelIndexList>

// Class CollectionProxyModel provides filter model for collections
/*!
  Class CollectionProxyModel is subclassed from QSortFilterProxyModel and provides custom special implementation
  of filterAcceptsRow() method to provide special filtering.
  The filter behaves like this: when a interpret's name matches the filter, it's displayed including all it's children and
  subchildren. If it does not match, but any of it's children matches the filter, the interpret and the album with all it's
  content are displayed. If the album does not match the filter but some of it's children does, the intepret, the album and
  the matching track are displayed.
*/
class CollectionProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    //! Constructor
    /*!
      Initializes the model
      \param parent parent of the model
    */
    CollectionProxyModel(QObject *parent = 0);

  protected:
    //! Called for each item in model, decides wheter the item will be displayed or not
    /*!
      Checks, wheter the item determined by \p sourceParent and \p sourceRow matches the
      filter or not.
      \param sourceRow position of the item relatively to it's parent
      \param sourceParent QModelIndex of the item's parent
      \return Returns true when the item matches the filter and should be displayed. When the item
      does not match the filter or simply should not be displayed, returns false.
    */
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

  private:
    //! Returns list of children of given parent
    /*!
      Recursively returns list of all parent's children including children's children.
      \param parent QModelIndex of parent item
      \return Returns list of parent's children
    */
    QModelIndexList getChildren(const QModelIndex parent) const;

};

#endif // COLLECTIONPROXYMODEL_H
