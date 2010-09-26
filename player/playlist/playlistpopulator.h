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

#ifndef PLAYLISTPOPULATOR_H
#define PLAYLISTPOPULATOR_H

#include <QRunnable>
#include <QObject>
#include <QStringList>
#include <QMutex>
#include <QWaitCondition>

class PlaylistModel;

//! PlaylistPopulator is thread that populates the playlist by given data
/*!
  PlaylistPopulator is subclassed from QThread. The thread is populating data from \p _files until
  the list is empty using FIFO method. When the list is empty the thread is suspended until new files are added
  by addFiles() or addFile() methods.
*/
class PlaylistPopulator : public QObject, public QRunnable
{
    Q_OBJECT
    public:
        //! Constructor
        /*!
          Constructor sets the playlistModel pointer and launches the thread
          \param playlistModel pointer to PlaylistModel
        */
        explicit PlaylistPopulator(PlaylistModel *playlistModel);

        //! Main loop
        void run();

    signals:
        //! Emitted when new all items from list are populated into the model.
        /*!
          The signal is emitted just before the thread is suspended.
        */
        void filesAdded();

        //! Emited when each new file is added to the model
        /*!
          \warning This can enormously slow down populating!
        */
        void fileAdded();

    public slots:
        //! Appends given list to \p _files list
        /*!
          Waits until _files mutex is unlocked, then locks it (this blocks the thread!), appends
          list of files to the \p _files list and unlocks it again.
          \note This is a blocking method.
          \note Folders and playlists are supported too. They are parsed by expandDir() or expandPlaylist().
          \param files list of files to be appended
          \sa addFile()
        */
        void addFiles(const QStringList &files);

        //! Appends given file to \p _files list
        /*!
          Waits until _files mutex is unlocked, then locks it (this blocks the thread!), appends
          the file to the \p _files list and unlocks it again.
          \note This is a blocking method.
          \note Folders and playlists are supported too. They are parsed by expandDir() or expandPlaylist().
          \param file file to be appended
          \sa addFiles()
        */
        void addFile(const QString &file);

    private:
        //! Expands the _files by list of files in \p dir
        /*!
          Removes given \p dir from the files list and replaces it by a list of all files in the given \p dir and
          it's subdirs
          \param dir folder to expand
        */
        void expandDir(QString dir);

        //! Expands the _files by list of files listed in \p filename playlist
        /*!
          Removes given \p filename from the files list and replaces it by a list of all files in given playlist
          \param filename playlist file to load
        */
        void expandPlaylist(QString filename);

        //! Pointer to playlist
        PlaylistModel *_playlistModel;

        //! List of files that are loaded
        QStringList _files;

};

#endif // PLAYLISTPOPULATOR_H
