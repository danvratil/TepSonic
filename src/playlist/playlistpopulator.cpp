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

#include "playlistpopulator.h"
#include "playlistmodel.h"

#include <QDirIterator>
#include <QMutexLocker>
#include <QDebug>
#include <QFileInfo>

PlaylistPopulator::PlaylistPopulator(PlaylistModel *playlistModel)
{
    moveToThread(this);

    _playlistModel = playlistModel;
    _canClose = false;
    _files.clear();

    // Start the thread and pause it immediatelly
    start();
}

PlaylistPopulator::~PlaylistPopulator()
{
    // Allow closing and wake the thread.
    if (isRunning()) {
        _canClose = true;
        _lock.wakeAll();
    }
    // Wait until sucessfully terminated
    wait();
}

void PlaylistPopulator::run()
{
    do
    {
        _mutex.lock();
        if (_files.size() > 0) {
            if (QFileInfo(_files.first()).isDir()) {
                expandDir(_files.takeFirst());
            }
            if (_files.size()>0) {
                if (QFileInfo(_files.first()).suffix().toLower()=="m3u") {
                    expandPlaylist(_files.takeFirst());
                }
            }
            if (_files.size() > 0) {
                if (_playlistModel->addItem(_files.takeFirst()))
                    emit fileAdded();
            }
        }
        _mutex.unlock();

        _mutex.lock();
        bool empty = _files.isEmpty();
        _mutex.unlock();

        /* When the _files is empty thread is suspended until awoken again
           by adding new file */
        if ((empty) && (!_canClose)) {
            emit filesAdded();
            _lock.wait(&_mutex);
        }
    } while (!_canClose);
}

void PlaylistPopulator::expandDir(QString dir)
{
    QStringList filters;
    filters << "*.mp3" << "*.mp4" << "*.wav" << "*.flac";
    //QDir dirlist(dir,QString(),QDir::Name,QDir::NoDotAndDotDot);
    QDir dirlist(dir);
    dirlist.setNameFilters(filters);
    QFileInfo fileInfo;

    QStringList files;

    QDirIterator dirIterator(dirlist,QDirIterator::Subdirectories);
    while (dirIterator.hasNext()) {
        fileInfo = dirIterator.fileInfo();
        if (fileInfo.isFile()) {
            files << fileInfo.filePath();
        }
        dirIterator.next();
    }

    files.append(_files);

    // We don't need to lock the mutex here since we are already in locked mutex
    _files = files;
}

void PlaylistPopulator::expandPlaylist(QString filename)
{
    QDir playlistDir(QFileInfo(filename).path());
    QFile file(filename);

    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "PlaylistManager::addPlaylistFile: Failed to open file " << filename;
        return;
    }

    QStringList files;

    char buf[1024];
    qint64 lineLength;
    do {
        lineLength = file.readLine(buf, sizeof(buf));
        if (lineLength != -1) {
            QString filepath = playlistDir.absoluteFilePath(buf).remove("\n",Qt::CaseInsensitive);
            if (filepath.length()>0)
                files << filepath;
        }
    } while (lineLength > -1);

    file.close();

    files.append(_files);
    _files = files;
}

void PlaylistPopulator::addFile(const QString &file)
{
    // Add new file to the list and wake the worker thread (if sleeping)
    _mutex.lock();
        _files.append(file);
    _mutex.unlock();
    _lock.wakeAll();
}

void PlaylistPopulator::addFiles(const QStringList &files)
{
    // Add new file to the list and wake the workert thread (if sleeping)
    _mutex.lock();
        _files.append(files);
    _mutex.unlock();
    _lock.wakeAll();
}
