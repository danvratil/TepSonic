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

#include "filesystembrowser.h"

#include <QFileSystemModel>
#include <QMimeData>
#include <QByteArray>
#include <QDataStream>
#include <QDrag>
#include <QDir>

#include <QDebug>

FileSystemBrowser::FileSystemBrowser(QWidget *parent):
        QListView(parent)
{
    setAcceptDrops(false);
    setDragDropMode(DragOnly);

    connect(this, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(setRootDir(QModelIndex)));

}


void FileSystemBrowser::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = selectedIndexes();

    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);;

    for (int i = 0; i < indexes.count(); i++) {
        stream << static_cast<QFileSystemModel*>(model())->filePath(indexes.at(i));
    }

    mimeData->setData("data/tepsonic-tracks", encodedData);
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);
}


void FileSystemBrowser::setRootDir(QModelIndex dir)
{
    QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel*>(model());
    if (!fsmodel) return;

    QString path = fsmodel->filePath(dir);

    // If the selected item is a file then add it to playlist
    if (fsmodel->fileInfo(dir).isFile()) {
        emit addTrackToPlaylist(fsmodel->filePath(dir));
        return;
    }

    // Prevent from having two same items in backDirs in row
    if ((m_backDirs.isEmpty()) || (m_backDirs.first() != fsmodel->filePath(rootIndex())))
        m_backDirs.prepend(fsmodel->filePath(rootIndex()));

    setRootIndex(fsmodel->index(path));
    setCurrentIndex(QModelIndex());

    emit pathChanged(path);

    emit disableCdUp(path == QDir::rootPath());
    emit disableBack(m_backDirs.isEmpty());
}


void FileSystemBrowser::goBack()
{
    if (m_backDirs.isEmpty()) return;

    QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel*>(model());
    if (!fsmodel) return;

    m_forwardDirs.prepend(fsmodel->filePath(rootIndex()));

    QString newpath = m_backDirs.takeFirst();
    setRootIndex(fsmodel->index(newpath));
    setCurrentIndex(QModelIndex());

    emit pathChanged(newpath);

    emit disableCdUp(newpath == QDir::rootPath());
    emit disableBack(m_backDirs.isEmpty());
    emit disableForward(false);
}


void FileSystemBrowser::goForward()
{
    if (m_forwardDirs.isEmpty()) return;

    QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel*>(model());
    if (!fsmodel) return;

    m_backDirs.prepend(fsmodel->filePath(rootIndex()));

    QString newpath = m_forwardDirs.takeFirst();
    setRootIndex(fsmodel->index(newpath));
    setCurrentIndex(QModelIndex());

    emit pathChanged(newpath);

    emit disableBack(m_backDirs.isEmpty());
    emit disableForward(m_forwardDirs.isEmpty());
    emit disableCdUp(newpath == QDir::rootPath());
}


void FileSystemBrowser::cdUp()
{
    QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel*>(model());
    if (!fsmodel) return;

    QModelIndex previousItem = rootIndex().parent();

    if (fsmodel->filePath(rootIndex().parent()).isEmpty())
        return;

    m_backDirs.prepend(fsmodel->filePath(rootIndex()));


    setRootIndex(previousItem);
    setCurrentIndex(QModelIndex());

    emit disableCdUp(fsmodel->filePath(previousItem) == QDir::rootPath());
    emit pathChanged(fsmodel->filePath(previousItem));
}


void FileSystemBrowser::goHome()
{
    QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel*>(model());
    if (!fsmodel) return;

    m_backDirs.prepend(fsmodel->filePath(rootIndex()));

    setRootIndex(fsmodel->index(QDir::homePath()));
    setCurrentIndex(QModelIndex());

    emit pathChanged(QDir::homePath());

    emit disableBack(m_backDirs.isEmpty());
    emit disableCdUp(fsmodel->fileName(rootIndex()) == QDir::rootPath());
}


void FileSystemBrowser::goToDir(QString newPath)
{
    QDir dir(newPath);
    if (!dir.exists()) return;

    QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel*>(model());
    if (!fsmodel) return;

    // Because when pathChanged() is emitted, the fsbPath edit is updated and this slot is fired.
    // That's why we need to stop here now
    if (newPath == fsmodel->filePath(rootIndex())) return;

    m_backDirs.prepend(fsmodel->filePath(rootIndex()));

    setRootIndex(fsmodel->index(newPath));
    setCurrentIndex(QModelIndex());

    emit pathChanged(newPath);

    emit disableCdUp(newPath == QDir::rootPath());
    emit disableBack(m_backDirs.isEmpty());
}


void FileSystemBrowser::keyPressEvent(QKeyEvent *event)
{
    // Qt will translate this to platform-native shortcut for "Back"
    if (event->matches(QKeySequence::Back)) {
        goBack();
        event->accept();
    }

    // Qt will translate this to platform-native shortcut for "Forward"
    else if (event->matches(QKeySequence::Forward)) {
        goForward();
        event->accept();
    }

    // Backspace or Alt+Up for "Up". There is no predefined key sequence in Qt for this
    else if ((event->key() == Qt::Key_Backspace) ||
        ((event->modifiers() == Qt::AltModifier) && (event->key() == Qt::Key_Up))) {
        cdUp();
        event->accept();
    }

    // Alt+Home or Homepage button (this must be tested because I don't have such button) to
    // navigate to home dir
    else if (((event->modifiers() == Qt::AltModifier) && (event->key() == Qt::Key_Home))
        || (event->key() == Qt::Key_HomePage)) {
        goHome();
        event->accept();
    }

    // Arrow up to select an item above current item (or last item when non is selected)
    else if (event->key() == Qt::Key_Up) {
        QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel*>(model());
        if (!fsmodel) return;
        QModelIndex newIndex = fsmodel->index(currentIndex().row()-1,0, rootIndex());
        if (newIndex.isValid())
            setCurrentIndex(newIndex);
        else {
            int row = fsmodel->rowCount(rootIndex())-1;
            setCurrentIndex(fsmodel->index(row,0, rootIndex()));
        }
        event->accept();
    }

    // Arrow down to select an item below current item (or first item when non is selected)
    else if (event->key() == Qt::Key_Down) {
        QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel*>(model());
        if (!fsmodel) return;
        QModelIndex newIndex = fsmodel->index(currentIndex().row()+1,0, rootIndex());
        if (newIndex.isValid())
            setCurrentIndex(newIndex);
        else {
            setCurrentIndex(fsmodel->index(0, 0, rootIndex()));
        }
        event->accept();
    }

    // Enter key to enter selected folder or add selected track to playlist
    else if ((event->key() == Qt::Key_Enter) ||
        (event->key() == Qt::Key_Return)) {
        setRootDir(currentIndex());
        event->accept();
    }
}
