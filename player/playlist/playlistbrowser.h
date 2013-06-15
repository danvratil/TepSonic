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

#ifndef PLAYLISTBROWSER_H
#define PLAYLISTBROWSER_H

#include <QTreeView>
#include <QStringList>

class PlaylistBrowser : public QTreeView
{

    Q_OBJECT

  public:
    PlaylistBrowser(QWidget *parent = 0);

    ~PlaylistBrowser();

    QModelIndex nowPlaying() const;
    void setNowPlaying(const QModelIndex &index);

    QModelIndex stopTrack() const;
    void setStopTrack(const QModelIndex &index);
    void clearStopTrack();

  public Q_SLOTS:
    void shuffle();

  protected:
    void dropEvent(QDropEvent *dropEvent);
    void dragEnterEvent(QDragEnterEvent *dragEnterEvent);
    void dragMoveEvent(QDragMoveEvent *dragMoveEvent);
    void keyPressEvent(QKeyEvent *keyEvent);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

  Q_SIGNALS:
    void addedFiles(const QStringList &files, int row);

  private:
    void invalidateIndex(const QModelIndex &index);

    QPoint m_dragStartPosition;

    QModelIndex m_nowPlaying;
    QModelIndex m_stopTrack;

};

#endif // PLAYLISTBROWSER_H
