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


#include "tracksiterator.h"
#include "playlistmodel.h"
#include "supportedformats.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QDebug>


TracksIterator::TracksIterator(const QString &topDir, PlaylistModel *model):
    m_rootDir(topDir),
    m_model(model)
{
}

void TracksIterator::run()
{
    const QStringList filters = SupportedFormats::getExtensionList();
    QDir dirlist(m_rootDir);
    dirlist.setNameFilters(filters);
    QFileInfo fileInfo;

    QDirIterator dirIterator(dirlist,QDirIterator::Subdirectories);
    while (dirIterator.hasNext()) {
        fileInfo = dirIterator.fileInfo();
        if (fileInfo.isFile()) {
            m_model->addItem(fileInfo.filePath());
        }
        dirIterator.next();
    }

    exec();
}
