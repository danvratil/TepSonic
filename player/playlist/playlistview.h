/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <me@dvratil.cz>
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

#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <QTreeView>
#include <QStringList>

namespace TepSonic
{
    class Playlist;
}

class PlaylistView : public QTreeView
{
    Q_OBJECT

  public:
    PlaylistView(QWidget *parent = 0);
    ~PlaylistView();

  public Q_SLOTS:
    void setFilter(const QString &filter);

  protected:
    void dropEvent(QDropEvent *dropEvent);
    void dragEnterEvent(QDragEnterEvent *dragEnterEvent);
    void dragMoveEvent(QDragMoveEvent *dragMoveEvent);
    void keyPressEvent(QKeyEvent *keyEvent);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

  private Q_SLOTS:
    void slotSortIndicatorChanged(int column, Qt::SortOrder order);
    void slotHeaderContextMenuRequested(const QPoint &pos);
    void slotContextMenuRequested(const QPoint &pos);
    void slotItemDoubleClicked(const QModelIndex &index);
    void slotCurrentTrackChanged();
    void slotStopTrackChanged();

  private:
    void invalidateRow(int row);

    QPoint m_dragStartPosition;
    int m_currentTrack;
    int m_stopTrack;
};

#endif // PLAYLISTVIEW_H
