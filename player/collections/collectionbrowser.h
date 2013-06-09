/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <dan@progdan.cz>
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

#ifndef COLLECTIONBROWSER_H
#define COLLECTIONBROWSER_H

#include <QTreeView>

//! A View for displaying tree structure of collections
/*!
  Class CollectionBrowser subclasses QTreeView and extends it for capability
  of providing data when drag&drop operation (CollectionBrowser->PlaylistBrowser) is
  started
*/
class CollectionBrowser : public QTreeView
{
  public:
    //! Constructor
    CollectionBrowser(QWidget *parent = 0);

    //! Destructor
    ~CollectionBrowser();

  protected:
    //! Event that is fired when drag operation is started
    void startDrag(Qt::DropActions dropActions);

    //! Event handler that is fired when a key is pressed
    /*!
      It is called only when CollectionBrowser has focus when the key
      is pressed
    */
    void keyPressEvent(QKeyEvent *event);

};

#endif // COLLECTIONBROWSER_H
