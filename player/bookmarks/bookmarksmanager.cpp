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


#include "bookmarksmanager.h"
#include "bookmarksbrowser.h"
#include "addbookmarkdlg.h"
#include "constants.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QDir>
#include <QMenu>
#include <QMessageBox>
#include <QListWidgetItem>


BookmarksManager::BookmarksManager(Ui::MainWindow *ui):
        m_bookmarksBrowser(0),
        m_mwui(ui),
        m_addBookmarkDlg(0),
        m_contextMenu(0),
        m_dontDeleteBookmarksBrowser(false)
{
    QSettings settings(_CONFIGDIR + QDir::separator() + "bookmarks",QSettings::IniFormat,this);

    QStringList bookmarks = settings.childGroups();
    foreach (QString bookmark, bookmarks) {
        QString title = settings.value(bookmark+"/title", QString()).toString();
        QString path = settings.value(bookmark+"/path", QString()).toString();
        m_bookmarks.append(QPair<QString,QString>(title, path));
    }

    m_contextMenu = new QMenu();
    m_contextMenu->addAction(tr("Remove bookmark"), this, SLOT(removeBookmark()));

}


BookmarksManager::~BookmarksManager()
{
    QSettings settings(_CONFIGDIR + QDir::separator() + "bookmarks",QSettings::IniFormat,this);

    for (int i = 0; i < m_bookmarks.length(); i++)
    {
        //no qobject_cast since BookmarksItem is not a QObject
        QPair<QString,QString> pair = m_bookmarks.at(i);
        settings.setValue(QString::number(i)+"/title", pair.first);
        settings.setValue(QString::number(i)+"/path", pair.second);
    }

    delete m_contextMenu;
}


void BookmarksManager::toggleBookmarks()
{
    if (m_bookmarksBrowser) {
        /* BEWARE: HACK! When this method is called from openBookmarkInFSB() (via
           m_mwui->fsbBookmarksBtn->toggled() slot, the variable is set to true,
           because the bookmarks browser must not be deleted from here (it crashes tepsonic).
           The browser is then deleted in the openBookmarkInFSB() method.
        */
        if (m_dontDeleteBookmarksBrowser)
            m_bookmarksBrowser->setHidden(true);
        else
            delete m_bookmarksBrowser;
        delete m_bookmarksFilterEdit;
        m_bookmarksBrowser = 0;
        m_bookmarksFilterEdit = 0;
        m_mwui->filesystemBrowser->setVisible(true);
        m_mwui->fsbPath->setVisible(true);
        m_mwui->fsbBackBtn->setEnabled(m_backBtnEnabled);
        m_mwui->fsbFwBtn->setEnabled(m_fwdBtnEnabled);
        m_mwui->fsbCdUpBtn->setEnabled(m_upBtnEnabled);
        m_mwui->fsbGoHomeBtn->setEnabled(true);
    } else {
        m_mwui->filesystemBrowser->setHidden(true);
        m_mwui->fsbPath->setHidden(true);

        m_bookmarksFilterEdit = new QLineEdit();
        m_mwui->fsbTab->layout()->addWidget(m_bookmarksFilterEdit);

        m_bookmarksBrowser = new BookmarksBrowser(this);
        m_bookmarksBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
        m_mwui->fsbTab->layout()->addWidget(m_bookmarksBrowser);

        for (int i = 0; i < m_bookmarks.length(); i++)
            m_bookmarksBrowser->addItem(m_bookmarks.at(i).first);

        connect(m_bookmarksBrowser, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(showBookmarksContextMenu(QPoint)));
        connect(m_bookmarksFilterEdit, SIGNAL(textChanged(QString)),
                m_bookmarksBrowser, SLOT(setFilter(QString)));
        connect(m_bookmarksBrowser, SIGNAL(doubleClicked(QModelIndex)),
                this, SLOT(openBookmarkInFSB(QModelIndex)));


        m_fwdBtnEnabled = m_mwui->fsbFwBtn->isEnabled();
        m_backBtnEnabled = m_mwui->fsbBackBtn->isEnabled();
        m_upBtnEnabled = m_mwui->fsbCdUpBtn->isEnabled();
        m_mwui->fsbBackBtn->setDisabled(true);
        m_mwui->fsbFwBtn->setDisabled(true);
        m_mwui->fsbCdUpBtn->setDisabled(true);
        m_mwui->fsbGoHomeBtn->setDisabled(true);

    }
}


void BookmarksManager::showAddBookmarkDialog(QString path)
{
    m_addBookmarkDlg = new AddBookmarkDlg(path);
    connect(m_addBookmarkDlg, SIGNAL(accepted(QString,QString)),
            this, SLOT(addBookmark(QString,QString)));
    m_addBookmarkDlg->exec();

}


void BookmarksManager::removeBookmark()
{

    QMessageBox msgbox(tr("Remove bookmark?"),
                       tr("Do you really want to remove this bookmark?"),
                       QMessageBox::Question,
                       QMessageBox::Yes,
                       QMessageBox::No,
                       0); // empty 3rd button
    if (msgbox.exec() == QMessageBox::No)
        return;

    QModelIndex bmark = m_bookmarksBrowser->currentIndex();

    m_bookmarks.removeAt(bmark.row());
    m_bookmarksBrowser->removeAt(bmark.row());
    QSettings settings(_CONFIGDIR + QDir::separator() + "bookmarks",QSettings::IniFormat,this);
    settings.remove(QString::number(bmark.row()));
}


void BookmarksManager::showBookmarksContextMenu(QPoint pos)
{
    if (!m_bookmarksBrowser->indexAt(pos).isValid())
        m_contextMenu->actions().at(0)->setDisabled(true);

    QPoint ppos = m_bookmarksBrowser->mapToGlobal(pos);
    m_contextMenu->popup(ppos);
}



void BookmarksManager::addBookmark(QString title, QString path)
{
    m_bookmarks.append(QPair<QString,QString>(title,path));
}



void BookmarksManager::openBookmarkInFSB(QModelIndex bookmark)
{
    QModelIndex mappedRow = m_bookmarksBrowser->mapToSource(bookmark);

    QString dir = m_bookmarks.at(mappedRow.row()).second;


    m_mwui->filesystemBrowser->goToDir(dir);

    /* The toggleBookmarks() method will not delete the bookmarks browser, just hide it. Deleting
       the browser in toggleBookmarks() would crash TepSonic when called via fsbBookmarksBtn->toggled()
       signal.
    */
    m_dontDeleteBookmarksBrowser = true;

    // This will automatically hide the bookmarks window and display FSB
    m_mwui->fsbBookmarksBtn->setChecked(false);

    // Disable the flag again, return to standart behavior
    m_dontDeleteBookmarksBrowser = false;

    // Delete the bookmarksBrowser now
    delete m_bookmarksBrowser;
}



QString BookmarksManager::getBookmarkPath(int row)
{
    return m_bookmarks.at(row).second;
}
