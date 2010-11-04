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

#include "collectionspage.h"
#include "ui_collectionspage.h"

#include <QFileDialog>
#include <QListWidgetItem>

using namespace SettingsPages;

CollectionsPage::CollectionsPage(QWidget *parent):
        QWidget(parent),
        m_collectionsSourceChanged(false)
{
    ui = new Ui::CollectionsPage();
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);

    connect(ui->dbEngineCombo, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(changeEngine(QString)));
    connect(ui->rebuildCollectionsButton, SIGNAL(clicked()),
            this, SIGNAL(rebuildCollections()));
    connect(ui->addPathButton, SIGNAL(clicked()),
            this, SLOT(addPath()));
    connect(ui->removePathButton, SIGNAL(clicked()),
            this, SLOT(removePath()));
    connect(ui->removeAllPathsButton, SIGNAL(clicked()),
            this, SLOT(removeAllPaths()));

}

void CollectionsPage::changeEngine(QString currEngine)
{
    if (currEngine == "SQLite") {
        ui->mysqlSettings->setDisabled(true);
    } else {
        ui->mysqlSettings->setEnabled(true);
    }
}

void CollectionsPage::addPath()
{
    QString dirName = QFileDialog::getExistingDirectory(this,
                      tr("Add directory"),
                      QString(),
                      QFileDialog::ShowDirsOnly);
    if (!dirName.isEmpty()) {
        ui->collectionsPathsList->addItem(dirName);
    }
    m_collectionsSourceChanged = true;
}

void CollectionsPage::removePath()
{
    foreach (QListWidgetItem *item, ui->collectionsPathsList->selectedItems()) {
        delete ui->collectionsPathsList->takeItem(ui->collectionsPathsList->row(item));
    }
    m_collectionsSourceChanged = true;
}

void CollectionsPage::removeAllPaths()
{
    ui->collectionsPathsList->clear();
    m_collectionsSourceChanged = true;
}
