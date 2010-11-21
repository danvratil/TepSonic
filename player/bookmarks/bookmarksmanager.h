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


#ifndef BOOKMARKSMANAGER_H
#define BOOKMARKSMANAGER_H

#include <QObject>
#include <QList>
#include <QPair>
#include <QPoint>
#include <QMenu>
#include <QModelIndex>
#include <QLineEdit>


class BookmarksBrowser;
class AddBookmarkDlg;

namespace Ui {
    class MainWindow;
}

class BookmarksManager: public QObject
{
    Q_OBJECT
    public:
        BookmarksManager(Ui::MainWindow *ui);
        ~BookmarksManager();

    public slots:
        void toggleBookmarks();
        void showAddBookmarkDialog(QString path);
        void removeBookmark();
        void addBookmark(QString title, QString path);
        void showBookmarksContextMenu(QPoint pos);
        void openBookmarkInFSB(QModelIndex);


    private:
        BookmarksBrowser *m_bookmarksBrowser;
        QLineEdit *m_bookmarksFilterEdit;
        QList< QPair<QString,QString> >  m_bookmarks;
        Ui::MainWindow *m_mwui;
        AddBookmarkDlg *m_addBookmarkDlg;
        QMenu *m_contextMenu;

        bool m_fwdBtnEnabled;
        bool m_backBtnEnabled;
        bool m_upBtnEnabled;

        bool m_dontDeleteBookmarksBrowser;
};

#endif // BOOKMARKSMANAGER_H
