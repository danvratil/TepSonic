/*
 * TEPSONIC
 * Copyright 2009 Dan Vratil <vratil@progdansoft.com>
 * Copyright 2009 Petr Los <petr_los@centrum.cz>
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

#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>
#include <QMutex>
#include <QThread>
#include <QStringList>

class PlaylistModel;

//! PlaylistManager is a thread to adding items to playlist and saving playlist to file
/*!
  PlaylistManage is subclasses from QThread and provides API to enable populating and saving
  playlist without freezing the UI.
  add(), saveToFile() and loadFromFile() are public slots that set up and lunch the thread. The Manager can
  perform only one action at time.
*/
class PlaylistManager : public QThread
{

    Q_OBJECT
    Q_ENUMS(PlaylistManagerAction)
    public:
        //! Enumaration of manager's action
        enum PlaylistManagerAction { LoadPlaylist,
                                     SavePlaylist
                                    };

        //! Constructor
        /*!
          \param model pointer to source PlaylistModel to read data from
        */
        PlaylistManager(PlaylistModel *model);

        //! Destructor
        ~PlaylistManager();

        //! Main thread function
        /*!
          When called, according to value in _action one of private methods p_addFile(), p_addFolder(),
          p_loadPlaylistFile() or p_savePlaylist() is called
        */
        void run();

    public slots:
        //! Adds file to be loaded
        /*!
          Adds one file to playlist
          \param filename File to add
        */
        void add(QString filename);

        //! Adds list of files to playlist
        /*!
          Appends list of files to current playlist
          \param filenames List of files
        */
        void add(QStringList filenames);

        //! Saves playlist to given file
        /*!
          Saves items in playlist to given file. The playlist is stored in pure M3U (list of files including
          relative paths from location of playlist)
          \param filename name of file to save playlist into
          \todo Add support for various playlist types
        */
        void saveToFile(QString filename);

        //! Loads playlist from given file
        /*!
          Loads files from given file. Currently the only supported format is pure M3U playlist with
          relative or absolute paths
          \param filename file with list of files to be loaded to playlist
          \todo Add support for various playlist types
        */
        void loadFromFile(QString filename);

    private:
        //! Appends given file to the PlaylistModel
        /*!
          \param filename file to append
        */
        void p_addFile(QString filename);

        //! Appends content of given folder to the PlaylistModel
        /*!
          First the full list of files is obtained fom the folder (recursively) and then
          for each file is called method p_addFile()
          \param folder folder to be appended
        */
        void p_addFolder(QString folder);

        //! Loads content of given playlist
        /*!
          Loads files from playlist one by one and for each file calls p_addFile()
          \param filename file with the playlist
        */
        void p_loadPlaylistFile(QString filename);

        //! Saves model to playlist
        /*!
          Loads all items from PlaylistModel, converts it's path to be relative to location
          of the playlist file and saves it to the playlist.
          \param filename file to save the playlist into
        */
        void p_savePlaylist(QString filename);

        //! Pointer to PlaylistModel
        PlaylistModel *_model;

        //! List of files that will be loaded when method run() is called
        QStringList _files;

        //! Mutex makes this manager thread-save
        QMutex _mutex;

        //! Action that will be performed when run() is called
        PlaylistManagerAction _action;

};

#endif // PLAYLISTMANAGER_H
