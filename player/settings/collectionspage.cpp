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

#include "collectionspage.h"
#include "ui_collectionspage.h"
#include "taskmanager.h"

#include <core/settings.h>

#include <QFileDialog>
#include <QListWidgetItem>

using namespace SettingsPages;
using namespace TepSonic;

enum {
    TAB_SETTINGS,
    TAB_SOURCES
};

CollectionsPage::CollectionsPage(QWidget *parent):
    SettingsPage(parent),
    m_collectionsSourceChanged(false)
{
    m_ui = new Ui::CollectionsPage();
    m_ui->setupUi(this);
    m_ui->tabWidget->setCurrentIndex(TAB_SETTINGS);

    connect(m_ui->rebuildCollectionsButton, &QPushButton::clicked,
            [=]() { saveSettings(); TaskManager::instance()->rebuildCollections(); });
    connect(m_ui->addPathButton, &QPushButton::clicked,
            this, &CollectionsPage::addPath);
    connect(m_ui->removePathButton, &QPushButton::clicked,
            this, &CollectionsPage::removePath);
    connect(m_ui->removeAllPathsButton, &QPushButton::clicked,
            this, &CollectionsPage::removeAllPaths);
    connect(m_ui->enableCollectionsCheckbox, &QCheckBox::toggled,
            this, &CollectionsPage::collectionStateToggled);
}

CollectionsPage::~CollectionsPage()
{
    delete m_ui;
}

void CollectionsPage::addPath()
{
    const QString dirName = QFileDialog::getExistingDirectory(this,
                    tr("Add directory"),
                     QString(), QFileDialog::ShowDirsOnly);
    if (!dirName.isEmpty()) {
        m_ui->collectionsPathsList->addItem(dirName);
    }
    m_collectionsSourceChanged = true;
}

void CollectionsPage::removePath()
{
    Q_FOREACH (QListWidgetItem * item, m_ui->collectionsPathsList->selectedItems()) {
        delete m_ui->collectionsPathsList->takeItem(m_ui->collectionsPathsList->row(item));
    }
    m_collectionsSourceChanged = true;
}

void CollectionsPage::removeAllPaths()
{
    m_ui->collectionsPathsList->clear();
    m_collectionsSourceChanged = true;
}

void CollectionsPage::collectionStateToggled()
{
    const bool checked = m_ui->enableCollectionsCheckbox->isChecked();

    m_ui->autoupdateCollectionsCheckbox->setEnabled(checked);
    m_ui->rebuildCollectionsButton->setEnabled(checked);
    m_ui->tabWidget->setTabEnabled(TAB_SOURCES, checked);
    // Always enabled
    m_ui->enableCollectionsCheckbox->setEnabled(true);
}

void CollectionsPage::loadSettings()
{
    m_ui->enableCollectionsCheckbox->setChecked(Settings::instance()->collectionsEnabled());
    m_ui->autoupdateCollectionsCheckbox->setChecked(Settings::instance()->collectionsAutoRebuild());
    m_ui->collectionsPathsList->addItems(Settings::instance()->collectionsSourcePaths());

    collectionStateToggled();
}

void CollectionsPage::saveSettings()
{
    Settings::instance()->setCollectionsEnabled(m_ui->enableCollectionsCheckbox->isChecked());
    Settings::instance()->setCollectionsAutoRebuild(m_ui->autoupdateCollectionsCheckbox->isChecked());

    QStringList items;
    for (int i = 0; i < m_ui->collectionsPathsList->count(); i++) {
        items.append(m_ui->collectionsPathsList->item(i)->text());
    }
    Settings::instance()->setCollectionsSourcePaths(items);
}
