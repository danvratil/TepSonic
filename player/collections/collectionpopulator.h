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


#ifndef COLLECTIONPOPULATOR_H
#define COLLECTIONPOPULATOR_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class CollectionModel;

//! CollectionPopulator populates the CollectionModel with data loaded from SQL storage backend
/*!
  CollectionPopulator is subclassed from QThread. The thread is sleeping until awoken by calling
  populate() method. When the model is populated by data from the SQL the thread falls asleep again
*/
class CollectionPopulator : public QThread
{
    Q_OBJECT
public:
    //! Constructor
    /*!
      Constructor that sets up collectionModel and launches the thread
      \param collectionModel ptr-to-ptr CollectionModel
    */
    explicit CollectionPopulator(CollectionModel **collectionModel);

    //! Destructor
    /*!
      Allows thread to quit and wakes the thread and wait until it quits
    */
    ~CollectionPopulator();

    //! Main thread method
    void run();

public slots:
    //! Wakes up the thread
    void populate();

signals:
    //! Emitted when whole collections are populated
    void collectionsPopulated();

private:
    //! Pointer to pointer to CollectionModel
    CollectionModel **_collectionModel;

    //! Mutex for syncing access to model
    QMutex _mutex;

    //! Locks the thread until awaken
    QWaitCondition _lock;

    //! Can I quit the thread daddy?
    bool _canClose;

};

#endif // COLLECTIONPOPULATOR_H
