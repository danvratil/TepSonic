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

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>

class PlaylistPopulator;
class CollectionPopulator;
class PlaylistWriter;
class CollectionBuilder;
class PlaylistModel;
class CollectionModel;

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
        explicit TaskManager(PlaylistModel *playlistModel, CollectionModel *collectionModel);

        //! Destructor
        /*!
          Waits until all threads are stopped and terminated
        */
        ~TaskManager();

    signals:
        // Emitted when progress of any thread is changed.
        /*!
          Returns overall progress of all running tasks together
          \param action action(s) that are being taken
          \param progress progress of all running tasks together
        */
        void progressChanged(QString action, int progress);

        //! Emitted when all items from playlist are populated
        void playlistPopulated();

        //! Emitted when the playlist is sucessfully saved
        void playlistSaved();

        //! Emitted when the CollectionModel is sucessfully populated
        void collectionsPopulated();

        //! Emited when rebuilding collection is finished
        void collectionsRebuilt();

        //! Emitted when collections are rebuild and a change is made (some track is added, updated or removed)
        void collectionsChanged();

    public slots:
        //! Appends given file to playlist
        /*!
          When called, PlaylistPopulator::addFile() method is called and
          the thread is resumed to append the file to the model
          \param filename file to add
        */
        void addFileToPlaylist(const QString &filename);

        //! Appends given files to playlist
        /*!
          When called, PlaylistPopulator::addFiles() method is called and
          the thread is resumed to append the files to the model
          \param files files to add
        */
        void addFilesToPlaylist(const QStringList &files);

        //! Saves current playlist to given file
        /*!
          When called, PlaylistWriter::saveToFile() method is called and
          the thread is resumed to save the playlist to the given file
          \param filename file to save to playlist into
        */
        void savePlaylistToFile(const QString &filename);

        //! Starts populating the collection browser
        /*!
          When called, CollectionPopulator::populate() method is called and
          the thread is resumed to populate the collection browser by data from
          database storage backend
        */
        void populateCollections();

        //! Starts rebuilding the collections in given folder
        /*!
          When called, CollectionBuilder::rebuildFolder() method is called and
          the thread is resumed to scan for changes in given folder and store the data
          in the database storage backend.
          \param folder folder to update, when empty string is given then all collection folders
          are rebuild
        */
        void rebuildCollections(const QString &folder = QString());

    private:
        //! Pointer to PlaylistPopulator
        PlaylistPopulator *_playlistPopulator;

        //! Pointer to CollectionPopulator
        CollectionPopulator *_collectionPopulator;

        //! Pointer to PlaylistWriter
        PlaylistWriter *_playlistWriter;

        //! Pointer to CollectionBuilder
        CollectionBuilder *_collectionBuilder;

        //! Pointer to PlaylistModel
        PlaylistModel *_playlistModel;

        //! Pointer to CollectionModel
        CollectionModel *_collectionModel;
};

#endif // TASKMANAGER_H
