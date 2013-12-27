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

#ifndef FILESYSTEMVIEW_H
#define FILESYSTEMVIEW_H

#include <QListView>
#include <QString>
#include <QModelIndex>
#include <QStack>

class QFileSystemModel;
class FileSystemView : public QListView
{
    Q_OBJECT

  public:
    FileSystemView(QWidget *parent = 0);
    virtual ~FileSystemView();

    bool canGoBack() const;
    bool canGoForward() const;
    bool canGoUp() const;

  public Q_SLOTS:
    void goBack();
    void goHome();
    void goForward();
    void cdUp();
    void setPath(const QString &newPath);

  Q_SIGNALS:
    void pathChanged(const QString &newPath);
    void trackSelected(const QString &filePath);

  protected:
    void startDrag(Qt::DropActions supportedActions);
    void keyPressEvent(QKeyEvent *event);

  private Q_SLOTS:
    void setRootDir(const QModelIndex &dir);

  private:

    QStack<QString> mBackHistory;
    QStack<QString> mForwardHistory;
    QFileSystemModel *mFSModel;
};

#endif // FILESYSTEMBROWSER_H
