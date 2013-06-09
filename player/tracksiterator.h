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

#ifndef TRACKSITERATOR_H
#define TRACKSITERATOR_H

#include <QObject>
#include <QThread>

class PlaylistModel;

class TracksIterator : public QThread
{
    Q_OBJECT

  public:
    TracksIterator(const QString &topDir, PlaylistModel *model);
    void run();

  private:
    QString m_rootDir;
    PlaylistModel *m_model;

  Q_SIGNALS:
    void fileFound(const QString &filename);
};

#endif // TRACKSITERATOR_H
