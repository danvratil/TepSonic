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

#include "playlistmanager.h"
#include "playlistmodel.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QStringList>


PlaylistManager::PlaylistManager(PlaylistModel *model)
{
     _model = model;
}

PlaylistManager::~PlaylistManager()
{
}

void PlaylistManager::run()
{
    _mutex.lock();
    switch (_action) {
        case PlaylistManager::LoadPlaylist:
            for (int i = 0; i < _files.count(); i++) {
                QFileInfo finfo(_files.at(i));
                if (finfo.isDir()) {
                    p_addFolder(_files.at(i));
                }
                if (finfo.isFile()) {
                    if (finfo.suffix()=="m3u") {
                        p_loadPlaylistFile(_files.at(i));
                    } else {
                        p_addFile(_files.at(i));
                    }
                }
            }
            break;
        case PlaylistManager::SavePlaylist:
            p_savePlaylist(_files.at(0));
            break;
    }
    _mutex.unlock();
}

void PlaylistManager::p_addFile(QString filename)
{
    _model->addItem(filename);
}

void PlaylistManager::p_addFolder(QString folder)
{
    QStringList filters;
    filters << "*.mp3" << "*.mp4" << "*.wav" << "*.flac";
    QDir dirlist(folder);
    dirlist.setNameFilters(filters);
    QFileInfo fileInfo;

    QDirIterator dirIterator(dirlist,QDirIterator::Subdirectories);
    while (dirIterator.hasNext()) {
        fileInfo = dirIterator.fileInfo();
        if (fileInfo.isFile()) {
            _model->addItem(fileInfo.filePath());
        }
        dirIterator.next();
    }
}

void PlaylistManager::p_loadPlaylistFile(QString filename)
{
    qDebug() << "Loading playlist from file " << filename;
    QDir playlistDir(QFileInfo(filename).path());
    QFile file(filename);

    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "PlaylistManager::addPlaylistFile: Failed to open file " << filename;
        return;
    }

    char buf[1024];
    qint64 lineLength;
    do {
        lineLength = file.readLine(buf, sizeof(buf));
        if (lineLength != -1) {
            QString filepath = playlistDir.absoluteFilePath(buf).remove("\n",Qt::CaseInsensitive);
            _model->addItem(QFileInfo(filepath).canonicalFilePath());
        }
    } while (lineLength > -1);

    file.close();
    qDebug() << "Playlist loaded";
}

void PlaylistManager::p_savePlaylist(QString filename)
{
    qDebug() << "Saving playlist to " << filename;
    QDir playlistDir(QFileInfo(filename).path());

    QFile file(filename);
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "Failed to open file!";
        return;
    }
    file.flush();

    for (int i = 0; i < _model->rowCount(QModelIndex()); i++) {
        QString trackfname = playlistDir.relativeFilePath(_model->index(i,0,QModelIndex()).data().toString());
        file.write(QByteArray().append(trackfname).append("\n"));
    }

    file.close();
    qDebug() << "Done";
}



void PlaylistManager::add(QString filename)
{
    _mutex.lock();
    _action = PlaylistManager::LoadPlaylist;
    _files.clear();
    _files << filename;
    _mutex.unlock();
    start();
}

void PlaylistManager::add(QStringList filenames)
{
    _mutex.lock();
    _action = PlaylistManager::LoadPlaylist;
    _files.clear();
    _files << filenames;
    _mutex.unlock();
    start();
}

void PlaylistManager::saveToFile(QString filename)
{
    _mutex.lock();
    _action = PlaylistManager::SavePlaylist;
    _files.clear();
    _files << filename;
    _mutex.unlock();
    start();
}

void PlaylistManager::loadFromFile(QString filename)
{
    add(filename);
}


