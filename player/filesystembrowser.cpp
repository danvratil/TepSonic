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

#include "filesystembrowser.h"
#include "constants.h"

#include <QMimeData>
#include <QByteArray>
#include <QDataStream>
#include <QDir>
#include <QDrag>
#include <QFileSystemModel>

#include <QSettings>


FileSystemBrowser::FileSystemBrowser(QWidget *parent):
    QListView(parent)
{
    setAcceptDrops(false);
    setDragDropMode(DragOnly);
    setContextMenuPolicy(Qt::CustomContextMenu);

    m_contextMenu = new QMenu(this);
    m_contextMenu->addAction(tr("Add to bookmarks"), this, SLOT(emitAddBookmark()));

    connect(this, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(setRootDir(QModelIndex)));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));

    connect(this, SIGNAL(pathChanged(QString)),
            this, SLOT(saveCurrentPath(QString)));
}

void FileSystemBrowser::setModel(QAbstractItemModel *model)
{
    QAbstractItemView::setModel(model);

    QSettings settings(_CONFIGDIR + QDir::separator() + "main.conf", QSettings::IniFormat);
    settings.beginGroup(QLatin1String("Last Session"));
    const QString lastPath = settings.value(QLatin1String("LastFSBPath"), QDir::homePath()).toString();

    // Wait until all signals are connected and eventloop is running
    QMetaObject::invokeMethod(this, "goToDir", Qt::QueuedConnection,
                              Q_ARG(QString, lastPath));
}

void FileSystemBrowser::saveCurrentPath(const QString &path)
{
    QSettings settings(_CONFIGDIR + QDir::separator() + "main.conf", QSettings::IniFormat);
    settings.beginGroup(QLatin1String("Last Session"));
    settings.setValue(QLatin1String("LastFSBPath"), path);
}

void FileSystemBrowser::startDrag(Qt::DropActions supportedActions)
{
    const QModelIndexList indexes = selectedIndexes();

    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);;

    for (int i = 0; i < indexes.count(); i++) {
        stream << static_cast<QFileSystemModel *>(model())->filePath(indexes.at(i));
    }

    mimeData->setData("data/tepsonic-tracks", encodedData);
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);
}


void FileSystemBrowser::setRootDir(const QModelIndex &dir)
{
    QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel *>(model());
    if (!fsmodel) {
        return;
    }

    const QString path = fsmodel->filePath(dir);

    // If the selected item is a file then add it to playlist
    if (fsmodel->fileInfo(dir).isFile()) {
        Q_EMIT addTrackToPlaylist(fsmodel->filePath(dir));
        return;
    }

    // Prevent from having two same items in backDirs in row
    if ((m_backDirs.isEmpty()) || (m_backDirs.first() != fsmodel->filePath(rootIndex())))
        m_backDirs.prepend(fsmodel->filePath(rootIndex()));

    setRootIndex(fsmodel->index(path));
    setCurrentIndex(QModelIndex());

    Q_EMIT pathChanged(path);
    Q_EMIT disableCdUp(path == QDir::rootPath() || path.isEmpty());
    Q_EMIT disableBack(m_backDirs.isEmpty());
}

void FileSystemBrowser::goBack()
{
    if (m_backDirs.isEmpty()) {
        return;
    }

    QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel *>(model());
    if (!fsmodel) {
        return;
    }

    m_forwardDirs.prepend(fsmodel->filePath(rootIndex()));

    const QString newpath = m_backDirs.takeFirst();
    setRootIndex(fsmodel->index(newpath));
    setCurrentIndex(QModelIndex());

    Q_EMIT pathChanged(newpath);
    Q_EMIT disableCdUp(newpath == QDir::rootPath() || newpath.isEmpty());
    Q_EMIT disableBack(m_backDirs.isEmpty());
    Q_EMIT disableForward(false);
}

void FileSystemBrowser::goForward()
{
    if (m_forwardDirs.isEmpty()) {
        return;
    }

    QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel *>(model());
    if (!fsmodel) {
        return;
    }

    m_backDirs.prepend(fsmodel->filePath(rootIndex()));

    const QString newpath = m_forwardDirs.takeFirst();
    setRootIndex(fsmodel->index(newpath));
    setCurrentIndex(QModelIndex());

    Q_EMIT pathChanged(newpath);
    Q_EMIT disableBack(m_backDirs.isEmpty());
    Q_EMIT disableForward(m_forwardDirs.isEmpty());
    Q_EMIT disableCdUp(newpath == QDir::rootPath() || newpath.isEmpty());
}

void FileSystemBrowser::cdUp()
{
    QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel *>(model());
    if (!fsmodel) {
        return;
    }

    QModelIndex previousItem = rootIndex().parent();

    // When the top-level directory ("C:\" or "/") is reached
    if (fsmodel->filePath(rootIndex().parent()).isEmpty()) {
        // On Windows set root path to "My Computer" (so that user see list of drives) which is above top-level dir.
        // This has no effect on Linux/Mac, where it returns "/"
        fsmodel->setRootPath(fsmodel->myComputer().toString());
        // Redefine the Previous item to point to the "My Computer" (or just "/" on Linux/Mac)
        previousItem = fsmodel->index(fsmodel->rootPath());
    }

    m_backDirs.prepend(fsmodel->filePath(rootIndex()));

    setRootIndex(previousItem);
    setCurrentIndex(QModelIndex());

    const QString newpath = fsmodel->filePath(previousItem);
    Q_EMIT disableCdUp(newpath == QDir::rootPath() || newpath.isEmpty());
    Q_EMIT pathChanged(newpath);
}

void FileSystemBrowser::goHome()
{
    QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel *>(model());
    if (!fsmodel) {
        return;
    }

    m_backDirs.prepend(fsmodel->filePath(rootIndex()));

    setRootIndex(fsmodel->index(QDir::homePath()));
    setCurrentIndex(QModelIndex());

    const QString newpath = fsmodel->fileName(rootIndex());
    Q_EMIT pathChanged(QDir::homePath());
    Q_EMIT disableBack(m_backDirs.isEmpty());
    Q_EMIT disableCdUp(newpath == QDir::rootPath() || newpath.isEmpty());
}

void FileSystemBrowser::goToDir(const QString &newPath)
{
    QDir dir(newPath);
    if (!dir.exists()) {
        return;
    }

    QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel *>(model());
    if (!fsmodel) {
        return;
    }

    // Because when pathChanged() is emitted, the fsbPath edit is updated and this slot is fired.
    // That's why we need to stop here now
    if (newPath == fsmodel->filePath(rootIndex())) {
        return;
    }

    m_backDirs.prepend(fsmodel->filePath(rootIndex()));

    setRootIndex(fsmodel->index(newPath));
    setCurrentIndex(QModelIndex());

    Q_EMIT pathChanged(newPath);
    Q_EMIT disableCdUp(newPath == QDir::rootPath() || newPath.isEmpty());
    Q_EMIT disableBack(m_backDirs.isEmpty());
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
        QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel *>(model());
        if (!fsmodel) return;
        QModelIndex newIndex = fsmodel->index(currentIndex().row() - 1, 0, rootIndex());
        if (newIndex.isValid())
            setCurrentIndex(newIndex);
        else {
            int row = fsmodel->rowCount(rootIndex()) - 1;
            setCurrentIndex(fsmodel->index(row, 0, rootIndex()));
        }
        event->accept();
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
        event->accept();
    }

    // Enter key to enter selected folder or add selected track to playlist
    else if ((event->key() == Qt::Key_Enter) ||
             (event->key() == Qt::Key_Return)) {
        setRootDir(currentIndex());
        event->accept();
    }
}

void FileSystemBrowser::emitAddBookmark()
{
    QFileSystemModel *fsmodel = qobject_cast<QFileSystemModel *>(model());
    if (!fsmodel) {
        return;
    }

    Q_EMIT addBookmark(fsmodel->filePath(currentIndex()));
}

void FileSystemBrowser::showContextMenu(const QPoint &pos)
{
    if (!indexAt(pos).isValid()) {
        m_contextMenu->actions().at(0)->setDisabled(true);
    }

    m_contextMenu->popup(mapToGlobal(pos));
}

#include "moc_filesystembrowser.cpp"
