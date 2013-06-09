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

#ifndef FILESYSTEMBROWSER_H
#define FILESYSTEMBROWSER_H

#include <QListView>
#include <QKeyEvent>
#include <QMenu>
#include <QStringList>
#include <QModelIndex>

class FileSystemBrowser : public QListView
{
    Q_OBJECT

  public:
    FileSystemBrowser(QWidget *parent = 0);

    void startDrag(Qt::DropActions supportedActions);

    virtual void setModel(QAbstractItemModel *model);

  protected:
    void keyPressEvent(QKeyEvent *event);

  private Q_SLOTS:
    void setRootDir(const QModelIndex &dir);
    void emitAddBookmark();
    void saveCurrentPath(const QString &path);

  public Q_SLOTS:
    void goBack();
    void goForward();
    void goHome();
    void cdUp();
    void goToDir(const QString &newPath);
    void showContextMenu(const QPoint &pos);

  private:

    QStringList m_forwardDirs;
    QStringList m_backDirs;
    QMenu *m_contextMenu;

  Q_SIGNALS:
    void pathChanged(const QString &newPath);
    void addTrackToPlaylist(const QString &filename);
    void disableForward(bool disable);
    void disableBack(bool disable);
    void disableCdUp(bool disable);
    void addBookmark(const QString &path);


};

#endif // FILESYSTEMBROWSER_H
