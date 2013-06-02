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


#ifndef PLAYLISTITEMDELEGATE_H
#define PLAYLISTITEMDELEGATE_H

#include <QtGui/QStyledItemDelegate>
#include <QtGui/QStyleOptionViewItem>
#include <QtGui/QPainter>
#include <QtCore/QModelIndex>

class PlaylistModel;
class PlaylistBrowser;
class PlaylistProxyModel;

class PlaylistItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    explicit PlaylistItemDelegate(QObject *parent = 0, PlaylistModel *playlistModel = 0,
                                  PlaylistBrowser *playlistBrowser = 0,
                                  PlaylistProxyModel *playlistProxyModel = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

  private:
    PlaylistModel *m_playlistModel;
    PlaylistBrowser *m_playlistBrowser;
    PlaylistProxyModel *m_playlistProxyModel;
};

#endif // PLAYLISTITEMDELEGATE_H
