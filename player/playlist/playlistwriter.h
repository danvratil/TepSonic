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

#ifndef PLAYLISTWRITER_H
#define PLAYLISTWRITER_H

#include <QRunnable>
#include <QObject>
#include <QMutex>
#include <QWaitCondition>

class PlaylistModel;

//! PlaylistWriter saves current playlist to given file
/*!
  PlaylistWriter is sublcassed from QThread. When the thread is awoken current playlist
  is saved to given file in m3u plain format
*/
class PlaylistWriter : public QObject, public QRunnable
{
    Q_OBJECT

  public:
    //! Constructor
    /*!
      Sets up the playlistModel and launches the thread.
      \param playlistModel pointer to PlaylistModel
    */
    explicit PlaylistWriter(PlaylistModel *playlistModel);


    //! Main loop
    void run();

  public Q_SLOTS:
    //! Wakes the thread and passes the given \p filename to it.
    /*!
      The playlist is saved into the given filename
        \param filename file to save the current playlist to
    */
    void saveToFile(const QString &filename);

  private:
    //! Pointer to PlaylistModel
    PlaylistModel *m_playlistModel;

    //! File to output the playlist into
    QString m_outputFile;

  Q_SIGNALS:
    //! Emitted when playlist is succesfully written to a file
    /*!
      The signal is emitted after each playlist in queue is saved.
    */
    void playlistSaved();


};

#endif // PLAYLISTWRITER_H
