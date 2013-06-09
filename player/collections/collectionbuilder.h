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

#ifndef COLLECTIONBUILDER_H
#define COLLECTIONBUILDER_H

#include <QRunnable>
#include <QObject>
#include <QStringList>
#include <QSqlDatabase>

class CollectionModel;

//! CollectionBuilder is a thread that builds collections
/*!
  CollectionBuilder loads data from SQL storage backend and comapres them
  with content of given folders. Then the SQL storage is updated and synced with
  the given folders.
  The threads is waiting sleeping and is awoken only when new folder is added to the list.
  When the work is done the thread falls asleep again.
*/
class CollectionBuilder : public QObject, public QRunnable
{
    Q_OBJECT

  public:

    //! Constructor
    CollectionBuilder();

    //! Start the thread
    /*!
      Main thread method. Loads data from database and populates the model
    */
    void run();

  public Q_SLOTS:
    //! Wake the thread and load given folder.
    void rebuildFolder(const QStringList &folder);

  Q_SIGNALS:
    //! Emitted when a change in collections is made
    void collectionChanged();

    //! Informs that building has begun
    void buildingStarted();

    //! Informs that building has finished
    void buildingFinished();

  private:
    //! List of folder to go through
    QStringList m_folders;

    //! Load informations about given file and store it in database
    /*!
      \param filename File to process
      \param    sqlDb Pointer to SQL connection to be used
    */
    void insertTrack(const QString &filename, QSqlDatabase sqlDb);

    //! Load new informations about given file and update the database
    /*!
      \param filename File to process
      \param sqlDb Pointer to SQL connection to be used
    */
    void updateTrack(const QString &filename, QSqlDatabase sqlDb);

    //! Remove given track from database
    /*!
      \param filename File to process
      \param sqlDb Pointer to SQL connection to be used
    */
    void removeTrack(const QString &filename, QSqlDatabase sqlDb);

    //! Check for interprets/albums/genres...that are not used anymore
    /*!
      \param sqlDb Pointer to SQL connection to be used
    */
    void cleanUpDatabase(QSqlDatabase sqlDb);
};

#endif // COLLECTIONBUILDER_H
