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

#include "filesystemview.h"

#include <core/settings.h>
#include <core/supportedformats.h>

#include <QMimeData>
#include <QByteArray>
#include <QDataStream>
#include <QDir>
#include <QDrag>
#include <QKeyEvent>
#include <QFileSystemModel>

using namespace TepSonic;

FileSystemView::FileSystemView(QWidget *parent):
    QListView(parent)
{

    mFSModel = new QFileSystemModel(this);
    mFSModel->setRootPath(QDir::rootPath());
    mFSModel->setNameFilters(SupportedFormats::extensionsList());
    mFSModel->setNameFilterDisables(false);
    setModel(mFSModel);

    setAcceptDrops(false);
    setDragDropMode(DragOnly);

    connect(this, &FileSystemView::doubleClicked,
            this, &FileSystemView::setRootDir);
    connect(this, &FileSystemView::pathChanged,
            [=](const QString &path) { Settings::instance()->setSessionFSBrowserPath(path); });
}

FileSystemView::~FileSystemView()
{
}

bool FileSystemView::canGoBack() const
{
    return !mBackHistory.isEmpty();
}

bool FileSystemView::canGoForward() const
{
    return !mForwardHistory.isEmpty();
}

bool FileSystemView::canGoUp() const
{
    return rootIndex().parent().isValid();
}


void FileSystemView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);

    const QModelIndexList indexes = selectedIndexes();

    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);;

    for (int i = 0; i < indexes.count(); i++) {
        stream << mFSModel->filePath(indexes.at(i));
    }

    mimeData->setData(QLatin1String("data/tepsonic-tracks"), encodedData);
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);
}


void FileSystemView::setRootDir(const QModelIndex &dir)
{
    QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel *>(model());
    const QString path = fsmodel->filePath(dir);

    // If the selected item is a file then add it to playlist
    if (fsmodel->fileInfo(dir).isFile()) {
        Q_EMIT trackSelected(path);
        return;
    }

    // Prevent from having two same items in backDirs in row
    if ((mBackHistory.isEmpty()) || (mBackHistory.top() != path)) {
        mBackHistory.push(fsmodel->filePath(rootIndex()));
    }

    setRootIndex(fsmodel->index(path));
    setCurrentIndex(QModelIndex());

    Q_EMIT pathChanged(path);
}

void FileSystemView::goBack()
{
    const QString path = mBackHistory.pop();
    mForwardHistory.push(mFSModel->filePath(rootIndex()));

    setPath(path);
}

void FileSystemView::cdUp()
{
    QModelIndex previousItem = rootIndex().parent();

    // When the top-level directory ("C:\" or "/") is reached
    if (mFSModel->filePath(rootIndex().parent()).isEmpty()) {
        // On Windows set root path to "My Computer" (so that user see list of drives) which is above top-level dir.
        // This has no effect on Linux/Mac, where it returns "/"
        mFSModel->setRootPath(mFSModel->myComputer().toString());
        // Redefine the Previous item to point to the "My Computer" (or just "/" on Linux/Mac)
        previousItem = mFSModel->index(mFSModel->rootPath());
    }

    mBackHistory.push(mFSModel->filePath(rootIndex()));

    setRootIndex(previousItem);
    setCurrentIndex(QModelIndex());

    const QString newpath = mFSModel->filePath(previousItem);
    Q_EMIT pathChanged(newpath);
}

void FileSystemView::goForward()
{
    const QString path = mForwardHistory.pop();
    mBackHistory.push(mFSModel->filePath(rootIndex()));

    setPath(path);
}

void FileSystemView::goHome()
{
    mBackHistory.push(mFSModel->filePath(rootIndex()));

    setRootIndex(mFSModel->index(QDir::homePath()));
    setCurrentIndex(QModelIndex());

    const QString newpath = mFSModel->fileName(rootIndex());
    Q_EMIT pathChanged(QDir::homePath());
}

void FileSystemView::setPath(const QString &newPath)
{
    QDir dir(newPath);
    if (!dir.exists()) {
        return;
    }

    setRootIndex(mFSModel->index(newPath));
    setCurrentIndex(QModelIndex());

    Q_EMIT pathChanged(newPath);
}

void FileSystemView::keyPressEvent(QKeyEvent *event)
{
    // Qt will translate this to platform-native shortcut for "Back"
    if (event->matches(QKeySequence::Back)) {
        goBack();
    }

    // Qt will translate this to platform-native shortcut for "Forward"
    else if (event->matches(QKeySequence::Forward)) {
        goForward();
    }

    // Backspace or Alt+Up for "Up". There is no predefined key sequence in Qt for this
    else if ((event->key() == Qt::Key_Backspace) ||
             ((event->modifiers() == Qt::AltModifier) && (event->key() == Qt::Key_Up))) {
        cdUp();
    }

    // Alt+Home or Homepage button (this must be tested because I don't have such button) to
    // navigate to home dir
    else if (((event->modifiers() == Qt::AltModifier) && (event->key() == Qt::Key_Home))
             || (event->key() == Qt::Key_HomePage)) {
        goHome();
    }

    // Arrow up to select an item above current item (or last item when non is selected)
    else if (event->key() == Qt::Key_Up) {
        QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel *>(model());
        if (!fsmodel) return;
        QModelIndex newIndex = fsmodel->index(currentIndex().row() - 1, 0, rootIndex());
        if (newIndex.isValid())
            setCurrentIndex(newIndex);
        else {
            int row = fsmodel->rowCount(rootIndex()) - 1;
            setCurrentIndex(fsmodel->index(row, 0, rootIndex()));
        }
    }

    // Arrow down to select an item below current item (or first item when non is selected)
    else if (event->key() == Qt::Key_Down) {
        QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel *>(model());
        if (!fsmodel) return;
        QModelIndex newIndex = fsmodel->index(currentIndex().row() + 1, 0, rootIndex());
        if (newIndex.isValid())
            setCurrentIndex(newIndex);
        else {
            setCurrentIndex(fsmodel->index(0, 0, rootIndex()));
        }
    }

    // Enter key to enter selected folder or add selected track to playlist
    else if ((event->key() == Qt::Key_Enter) ||
             (event->key() == Qt::Key_Return)) {
        setRootDir(currentIndex());
    }

    // Chain up
    else {
        QListView::keyPressEvent(event);
        return;
    }

    event->accept();
}
