/*
 * TEPSONIC
 * Copyright 2010 Dan Vratil <vratil@progdansoft.com>
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


#ifndef PLAYLISTPROXYMODEL_H
#define PLAYLISTPROXYMODEL_H

#include <QtGui/QSortFilterProxyModel>
#include <QtCore/QModelIndex>

//! PlaylistProxyModel is a filter model for playlist
/*!
  PlaylistProxyModel is subclassed from QSortFilterProxyModel and reimplements filterAcceptsRow() method
  in the way that items are filtered by values in all columns but the first (filename)
*/
class PlaylistProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    //! Constructor
    /*!
      \param parent pointer to model's parent object
    */
    PlaylistProxyModel(QObject *parent = 0);

  protected:
    //! Decides wheter the source row will be displayed of hidden
    /*!
      This method compares values of all columns (excluding the first one, with filename) of item on row \p sourceRow
      with current filter and if at least one column matches then the row will remain displayed.
      \param sourceRow number of row (relatively to \p sourceParent) that is being checked
      \param sourceParent index of row's parent item. Because all items in PlaylistModel have one common parent -
      the rootItem, this will always point to the rootItem
      \return Returns true when at least one column matches the filter and the row shall be displayed or false when
      no row matches the filter.
    */
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif // PLAYLISTPROXYMODEL_H
