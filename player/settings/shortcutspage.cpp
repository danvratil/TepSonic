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

#include "shortcutspage.h"
#include "ui_shortcutspage.h"
#include "changeshortcutdialog.h"

#include <QTreeWidgetItem>

using namespace SettingsPages;

ShortcutsPage::ShortcutsPage(QWidget *parent)
{
    ui = new Ui::ShortcutsPage();
    ui->setupUi(this);
    connect(ui->shortcutsList, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(changeShortcut(QModelIndex)));
}

void ShortcutsPage::changeShortcut(QModelIndex index)
{
    csw = new ChangeShortcutDialog(index, this);
    connect(csw, SIGNAL(shortcutChanged(QModelIndex,QKeySequence)),
            this, SLOT(shortcutChanged(QModelIndex,QKeySequence)));
    csw->show();
}

void ShortcutsPage::shortcutChanged(QModelIndex index, QKeySequence shortcut)
{
    QTreeWidgetItem *item = static_cast<QTreeWidgetItem*>(index.internalPointer());
    item->setData(1,
                  Qt::EditRole,
                  QVariant(shortcut.toString(QKeySequence::NativeText)));
}
