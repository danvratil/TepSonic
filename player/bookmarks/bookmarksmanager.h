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

#ifndef BOOKMARKSMANAGER_H
#define BOOKMARKSMANAGER_H

#include <QObject>
#include <QList>
#include <QPair>
#include <QPoint>
#include <QModelIndex>

class BookmarksBrowser;
class AddBookmarkDlg;
class QLineEdit;
class QMenu;

namespace Ui
{
class MainWindow;
}

class BookmarksManager: public QObject
{
    Q_OBJECT
  public:
    explicit BookmarksManager(Ui::MainWindow *ui, QObject *parent = 0);
    virtual ~BookmarksManager();

    QString getBookmarkPath(int row) const;

  public Q_SLOTS:
    void toggleBookmarks();
    void showAddBookmarkDialog(const QString &path);
    void removeBookmark();
    void addBookmark(const QString &title, const QString &path);
    void showBookmarksContextMenu(const QPoint &pos) const;
    void openBookmarkInFSB(const QModelIndex &index);

  private:
    void loadBookmarks();
    void saveBookmarks();

    BookmarksBrowser *m_bookmarksBrowser;
    QLineEdit *m_bookmarksFilterEdit;
    QList< QPair<QString, QString> >  m_bookmarks;
    Ui::MainWindow *m_mwui;
    AddBookmarkDlg *m_addBookmarkDlg;
    QMenu *m_contextMenu;

    bool m_fwdBtnEnabled;
    bool m_backBtnEnabled;
    bool m_upBtnEnabled;

    bool m_dontDeleteBookmarksBrowser;
};

#endif // BOOKMARKSMANAGER_H
