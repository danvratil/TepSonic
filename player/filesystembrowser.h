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

    public slots:
        void goBack();
        void goForward();
        void goHome();
        void cdUp();
        void goToDir(QString newPath);

    private:
        QStringList m_forwardDirs;
        QStringList m_backDirs;

    signals:
        void pathChanged(QString newPath);
        void addTrackToPlaylist(QString filename);
        void disableForward(bool);
        void disableBack(bool);
        void disableCdUp(bool);


};

#endif // FILESYSTEMBROWSER_H
