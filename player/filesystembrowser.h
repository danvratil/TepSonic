/*
 * TEPSONIC
 * Copyright 2009 Dan Vratil <vratil@progdansoft.com>
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
#include <QStringList>
#include <QKeyEvent>
#include <QMenu>
#include <QModelIndex>

class FileSystemBrowser : public QListView
{
    Q_OBJECT
    public:
        FileSystemBrowser(QWidget *parent = 0);

        void startDrag(Qt::DropActions supportedActions);

    protected:
        void keyPressEvent(QKeyEvent *event);


    private slots:
        void setRootDir(QModelIndex dir);
        void emitAddBookmark();


    public slots:
        void goBack();
        void goForward();
        void goHome();
        void cdUp();
        void goToDir(QString newPath);
        void showContextMenu(QPoint pos);

    private:
        QStringList m_forwardDirs;
        QStringList m_backDirs;
        QMenu *m_contextMenu;

    signals:
        void pathChanged(QString newPath);
        void addTrackToPlaylist(QString filename);
        void disableForward(bool);
        void disableBack(bool);
        void disableCdUp(bool);
        void addBookmark(QString path);


};

#endif // FILESYSTEMBROWSER_H
