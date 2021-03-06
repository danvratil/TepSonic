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

#ifndef FILESYSTEMWIDGET_H
#define FILESYSTEMWIDGET_H

#include <QWidget>
#include <QStack>

class FileSystemView;
class QLineEdit;
class QPushButton;

class FileSystemWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit FileSystemWidget(QWidget* parent = 0);
    virtual ~FileSystemWidget();

  Q_SIGNALS:
    void trackSelected(const QString &filePath);

  private Q_SLOTS:
    void setPath(const QString &path);

  private:
    QPushButton *createButton(const QString &icon, const QString &title,
                              void (FileSystemView::*slot)(void));

    QPushButton *mBackButton;
    QPushButton *mUpButton;
    QPushButton *mForwardButton;
    QPushButton *mHomeButton;
    QLineEdit *mFSPathEditor;
    FileSystemView *mFSView;
};

#endif // FILESYSTEMWIDGET_H
