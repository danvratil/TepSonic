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
    if (fsmodel->fileInfo(dir).isFile()) {
        emit addTrackToPlaylist(fsmodel->filePath(dir));
        return;
    }

    if ((m_backDirs.isEmpty()) || (m_backDirs.first() != fsmodel->filePath(rootIndex())))
        m_backDirs.prepend(fsmodel->filePath(rootIndex()));

    setRootIndex(fsmodel->index(path));

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

    emit pathChanged(newpath);

    emit disableBack(m_backDirs.isEmpty());
    emit disableForward(m_forwardDirs.isEmpty());
    emit disableCdUp(newpath == QDir::rootPath());
}


void FileSystemBrowser::cdUp()
{
    QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel*>(model());
    if (!fsmodel) return;

    m_backDirs.prepend(fsmodel->filePath(rootIndex()));

    QModelIndex previousItem = rootIndex().parent();

    setRootIndex(previousItem);

    emit disableCdUp(fsmodel->filePath(previousItem) == QDir::rootPath());
    emit pathChanged(fsmodel->filePath(previousItem));
}


void FileSystemBrowser::goHome()
{
    QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel*>(model());
    if (!fsmodel) return;

    m_backDirs.prepend(fsmodel->filePath(rootIndex()));

    setRootIndex(fsmodel->index(QDir::homePath()));

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

    emit pathChanged(newPath);

    emit disableCdUp(newPath == QDir::rootPath());
    emit disableBack(m_backDirs.isEmpty());
}


void FileSystemBrowser::keyPressEvent(QKeyEvent *event)
{
    // Qt will translate these sequences to OS' native shortcut
    if (event->matches(QKeySequence::Back)) {
        goBack();
    }
    if (event->matches(QKeySequence::Forward)) {
        goForward();
    }

    // Qt does not have translation for 'cd up' so we are using the intuitive backspace
    if (event->key() == Qt::Key_Backspace) {
        cdUp();
    }

    // Qt also does not have translation for 'go home', so we are using Alt+Home or
    // Homepage button (this should be tested somewhere because I don't have this button)
    if (((event->modifiers() == Qt::AltModifier) && (event->key() == Qt::Key_Home))
        || (event->key() == Qt::Key_HomePage)) {
        goHome();
    }
}
