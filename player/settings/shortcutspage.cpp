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

#include "shortcutspage.h"
#include "ui_shortcutspage.h"
#include "changeshortcutdialog.h"
#include "settings.h"

#include <QTreeWidgetItem>

using namespace SettingsPages;

ShortcutsPage::ShortcutsPage(QWidget *parent):
    SettingsPage(parent)
{
    m_ui = new Ui::ShortcutsPage();
    m_ui->setupUi(this);
    connect(m_ui->shortcutsList, &QTreeWidget::doubleClicked,
            this, &ShortcutsPage::changeShortcut);
}

ShortcutsPage::~ShortcutsPage()
{
    delete m_ui;
}

void ShortcutsPage::changeShortcut(const QModelIndex &index)
{
    m_csw = new ChangeShortcutDialog(index, this);
    connect(m_csw, &ChangeShortcutDialog::shortcutChanged,
            this, &ShortcutsPage::shortcutChanged);
    m_csw->show();
}

void ShortcutsPage::shortcutChanged(const QModelIndex &index, const QKeySequence &shortcut)
{
    QTreeWidgetItem *item = static_cast<QTreeWidgetItem *>(index.internalPointer());
    item->setData(1, Qt::EditRole, QVariant(shortcut.toString(QKeySequence::NativeText)));

    if (m_csw) {
        delete m_csw;
    }
}

void ShortcutsPage::loadSettings()
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setData(0, Qt::DisplayRole, tr("Play/pause"));
    item->setData(1, Qt::DisplayRole, Settings::instance()->shortcutPlayPause());
    m_ui->shortcutsList->addTopLevelItem(item);

    item = new QTreeWidgetItem();
    item->setData(0, Qt::EditRole, tr("Stop"));
    item->setData(1, Qt::EditRole, Settings::instance()->shortcutStop());
    m_ui->shortcutsList->addTopLevelItem(item);

    item = new QTreeWidgetItem();
    item->setData(0, Qt::EditRole, tr("Previous track"));
    item->setData(1, Qt::EditRole, Settings::instance()->shortcutPreviousTrack());
    m_ui->shortcutsList->addTopLevelItem(item);

    item = new QTreeWidgetItem();
    item->setData(0, Qt::EditRole, tr("Next track"));
    item->setData(1, Qt::EditRole, Settings::instance()->shortcutNextTrack());
    m_ui->shortcutsList->addTopLevelItem(item);

    item = new QTreeWidgetItem();
    item->setData(0, Qt::EditRole, tr("Show/Hide window"));
    item->setData(1, Qt::EditRole, Settings::instance()->shortcutToggleWindow());
    m_ui->shortcutsList->addTopLevelItem(item);
}

void ShortcutsPage::saveSettings()
{
    Settings::instance()->setShortcutPlayPause(m_ui->shortcutsList->topLevelItem(0)->data(1, Qt::DisplayRole).toString());
    Settings::instance()->setShortcutStop(m_ui->shortcutsList->topLevelItem(1)->data(1, Qt::DisplayRole).toString());
    Settings::instance()->setShortcutPreviousTrack(m_ui->shortcutsList->topLevelItem(2)->data(1, Qt::DisplayRole).toString());
    Settings::instance()->setShortcutNextTrack(m_ui->shortcutsList->topLevelItem(3)->data(1, Qt::DisplayRole).toString());
    Settings::instance()->setShortcutToggleWindow(m_ui->shortcutsList->topLevelItem(4)->data(1, Qt::DisplayRole).toString());
}
