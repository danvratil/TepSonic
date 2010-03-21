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


#ifndef COLLECTIONBUILDER_H
#define COLLECTIONBUILDER_H

#include <QThread>

class CollectionModel;

//! CollectionBuilder is a thread that fills given CollectionModel
/*!
  CollectionBuilder loads data from SQL storage backend and puts them
  to given CollectionModel creating the tree structure of collections
*/
class CollectionBuilder : public QThread
{
    Q_OBJECT
    public:

        //! Constructor
        /*!
          \param model pointer to a CollectionModel that should be populated
        */
        CollectionBuilder(CollectionModel *model);

        //! Start the thread
        /*!
          Main thread method. Loads data from database and populates the model
        */
        void run();

    private:
        //! Pointer to CollectionModel that is populated
        CollectionModel *_model;
};

#endif // COLLECTIONBUILDER_H
