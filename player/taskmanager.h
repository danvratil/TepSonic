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

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>
#include <QThreadPool>
#include <QModelIndex>

#include "player.h"

class PlaylistPopulator;
class PlaylistWriter;
class CollectionBuilder;
class PlaylistModel;

//! Task manager is object that provides common API for working threads
/*!
  Task manager holds all the workers threads and provides simplified API to access
  the threads.
  The threads are \list
    \li PlaylistPopulator - for populating the PlaylistModel
    \li PlaylistWriter - for saving current playlist to file
    \li CollectionBuilder - for building and updating collections
    \li CollectionPopulator - for populating the CollectionModel
  \endlist
*/
class TaskManager : public QObject
{
    Q_OBJECT

  public:
    //! Constructor
    /*!
      \param playlistModel pointer to PlaylistModel
      \param collectionModel pointer to CollectionModel
    */
    explicit TaskManager();

    //! Destructor
    /*!
      Waits until all threads are stopped and terminated
    */
    ~TaskManager();

    void setPlaylistModel(PlaylistModel *model);

  Q_SIGNALS:
    //! Emitted when all data in PlaylistPopulator are processed
    void playlistPopulated();

    //! Emitted when data from PlaylistPopulator are send
    void insertItemToPlaylist(const Player::MetaData &metadata, int row);

    //! Emitted when the playlist is sucessfully saved
    void playlistSaved();

    //! Emited when rebuilding collection is finished
    void collectionsRebuilt();

    //! Emited when a task is finished
    void taskDone();

    //! Emited when a task is started describing the started task
    /*!
      \param action description of the task
    */
    void taskStarted(const QString &action);

  public Q_SLOTS:
    //! Appends given file to playlist
    /*!
      When called, PlaylistPopulator::addFile() method is called and
      the thread is resumed to append the file to the model
      \param filename file to add
    */
    void addFileToPlaylist(const QString &filename, int row = 0);

    //! Appends given files to playlist
    /*!
      When called, PlaylistPopulator::addFiles() method is called and
      the thread is resumed to append the files to the model
      \param files files to add
    */
    void addFilesToPlaylist(const QStringList &files, int row = 0);

    //! Saves current playlist to given file
    /*!
      When called, PlaylistWriter::saveToFile() method is called and
      the thread is resumed to save the playlist to the given file
      \param filename file to save to playlist into
    */
    void savePlaylistToFile(const QString &filename);

    //! Starts rebuilding the collections in given folder
    /*!
      When called, CollectionBuilder::rebuildFolder() method is called and
      the thread is resumed to scan for changes in given folder and store the data
      in the database storage backend.
      \param folder folder to update, when empty string is given then all collection folders
      are rebuild
    */
    void rebuildCollections(const QString &folder = QString());

  private Q_SLOTS:

    //! Emits signals with information about collections rebuilding has started
    void collectionsRebuildingStarted();

  private:
    //! Pointer to pointer to PlaylistModel
    PlaylistModel *m_playlistModel;

    QThreadPool *m_threadPool;

    //! This pool is specially for collections where having two tasks running simultaneously is
    // not very safe.
    QThreadPool *m_collectionsThreadPool;
};

#endif // TASKMANAGER_H
