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
#include <QMutex>
#include <QWaitCondition>
#include <QStringList>

class CollectionModel;

//! CollectionBuilder is a thread that builds collections
/*!
  CollectionBuilder loads data from SQL storage backend and comapres them
  with content of given folders. Then the SQL storage is updated and synced with
  the given folders.
  The threads is waiting sleeping and is awoken only when new folder is added to the list.
  When the work is done the thread falls asleep again.
*/
class CollectionBuilder : public QThread
{
    Q_OBJECT
public:

    //! Constructor
    /*!
      \param model pointer to a CollectionModel that should be populated
    */
    CollectionBuilder(CollectionModel **model);

    //! Destructor
    ~CollectionBuilder();

    //! Start the thread
    /*!
      Main thread method. Loads data from database and populates the model
    */
    void run();

public slots:
    //! Wake the thread and load given folder.
    void rebuildFolder(QStringList folder);

signals:
    //! Emitted when a change in collections is made
    void collectionChanged();

    //! Informs that building has begun
    void buildingStarted();

    //! Informs that building has finished
    void buildingFinished();


private:
    //! Pointer to pointer to CollectionModel that is populated
    CollectionModel **_collectionModel;

    //! Mutex for syncing access to model
    QMutex _mutex;

    //! Lock holds the thread sleeping when there's nothing to do
    QWaitCondition _lock;

    //! Allows thread to quit
    bool _canClose;

    //! List of folder to go through
    QStringList _folders;
};

#endif // COLLECTIONBUILDER_H
