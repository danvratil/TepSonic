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

#ifndef COLLECTIONSUPDATER_H
#define COLLECTIONSUPDATER_H

#include <QThread>

class CollectionModel;

//! CollectionsUpdater is a thread for updating the collection database
/*!
  CollectionsUpdater is subclassed from QThread. It's creates list of all files in
  folders that were set in Settings dialog. Then a QMap filename-mtime (last-modified time) is created
  and all items in the map are compared against the items stored in database.
  Items that have different mtime are listed as \p toBeUpdated, items that were found on filesystem but
  were not in database are listed as \p toBeAdded and items that where found in database but not on disk
  are listed as \p toBeRemoved. Then items from single lists are taken and appropriate action is taken (add/update/delete)
  in database. When any of listed actions is taken signal collectionsChanged() is emitted at the end of the run() method to
  notify CollectionsBuilder that the model must be cleared and re-populated.
  \sa run(), collectionsChanged()
*/
class CollectionsUpdater : public QThread
{
    Q_OBJECT
    public:
        //! Constructor
        /*!
          Intialize the thread
          \param model Pointer to CollectionModel
          \warning \p model parameter will be removed soon
         */
        CollectionsUpdater(CollectionModel *model);

        //! Main thread's method that performs all the action described above.
        /*!
          When called the thread is fired and collections are rebuild
        */
        void run();

    signals:
        //! Notifies about changes in collection
        /*!
          Emitted when something was changed in the collections.
        */
        void collectionsChanged();

    private:
        //! Pointer to CollectionModel
        CollectionModel *_model;
};

#endif // COLLECTIONSUPDATER_H
