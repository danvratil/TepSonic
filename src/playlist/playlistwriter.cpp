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

#include "playlistwriter.h"

#include "playlistmodel.h"

#include <QDir>
#include <QFileInfo>

PlaylistWriter::PlaylistWriter(PlaylistModel *playlistModel)
{
    moveToThread(this);

    _outputFile = QString();
    _playlistModel = playlistModel;
    _canClose = false;

    start();
}

PlaylistWriter::~PlaylistWriter()
{
    // Allow thread to close and wake it
    if (isRunning()) {
        _canClose = true;
        _lock.wakeAll();
    }
    // Now wait until terminated
    wait();
}

void PlaylistWriter::run()
{
    do {

        if (!_outputFile.isEmpty())
        {
            QDir playlistDir(QFileInfo(_outputFile).path());

            QFile file(_outputFile);
            if (file.open(QFile::WriteOnly)) {

                file.flush();
                _mutex.lock();
                for (int i = 0; i < _playlistModel->rowCount(QModelIndex()); i++) {
                    QString trackfname = playlistDir.relativeFilePath(_playlistModel->index(i,0,QModelIndex()).data().toString());
                    file.write(QByteArray().append(trackfname).append("\n"));
                }
                _mutex.unlock();

            }
            file.close();
        }

        // Wait until awaken again
        if (!_canClose)
            _lock.wait(&_mutex);

    } while (!_canClose);
}

void PlaylistWriter::saveToFile(QString filename)
{
    _mutex.lock();
        _outputFile = filename;
    _mutex.unlock();
    _lock.wakeAll();
}
