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

#include <QtCore/QRunnable>
#include <QtCore/QObject>
#include <QtCore/QModelIndex>

class CollectionModel;

//! CollectionPopulator populates the CollectionModel with data loaded from SQL storage backend
/*!
  CollectionPopulator is subclassed from QThread. The thread is sleeping until awoken by calling
  populate() method. When the model is populated by data from the SQL the thread falls asleep again
*/
class CollectionPopulator : public QObject, public QRunnable
{
    Q_OBJECT

  public:
    //! Constructor
    /*!
      Constructor that sets up collectionModel and launches the thread
      \param collectionModel ptr-to-ptr CollectionModel
    */
    explicit CollectionPopulator(CollectionModel *collectionModel);

    //! Main thread method
    void run();

  Q_SIGNALS:
    //! Emitted when whole collections are populated
    void collectionsPopulated();

    void clearCollectionModel();

    void addChild(const QModelIndex &parent, const QString &title,
                  const QString &filename, const QString &data1,
                  const QString &data2, QModelIndex &item);

  private:
    //! Pointer to pointer to CollectionModel
    CollectionModel *m_collectionModel;

};

#endif // COLLECTIONPOPULATOR_H
