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

#include "playlistwriter.h"

#include "playlistmodel.h"

#include <QDir>
#include <QFileInfo>

PlaylistWriter::PlaylistWriter(PlaylistModel *playlistModel):
    QObject(),
    QRunnable(),
    m_playlistModel(playlistModel)
{
}

void PlaylistWriter::run()
{
    if (!m_outputFile.isEmpty()) {
        const QDir playlistDir(QFileInfo(m_outputFile).path());
        QFile file(m_outputFile);
        if (file.open(QFile::WriteOnly)) {
            file.flush();
            for (int i = 0; i < m_playlistModel->rowCount(QModelIndex()); i++) {
                const QString trackfname = playlistDir.relativeFilePath(m_playlistModel->index(i, 0, QModelIndex()).data().toString());
                file.write(trackfname.toLatin1() + '\n');
            }
        }
        file.close();
    }

    Q_EMIT playlistSaved();
}

void PlaylistWriter::saveToFile(const QString &filename)
{
    m_outputFile = filename;
}
