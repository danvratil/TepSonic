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

#include <QFileDialog>
#include <QListWidgetItem>

using namespace SettingsPages;

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

    connect(m_ui->dbEngineCombo, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(changeEngine(QString)));
    connect(m_ui->rebuildCollectionsButton, SIGNAL(clicked()),
            this, SIGNAL(rebuildCollections()));
    connect(m_ui->addPathButton, SIGNAL(clicked()),
            this, SLOT(addPath()));
    connect(m_ui->removePathButton, SIGNAL(clicked()),
            this, SLOT(removePath()));
    connect(m_ui->removeAllPathsButton, SIGNAL(clicked()),
            this, SLOT(removeAllPaths()));
    connect(m_ui->enableCollectionsCheckbox, SIGNAL(toggled(bool)),
            this, SLOT(collectionStateToggled()));
}

CollectionsPage::~CollectionsPage()
{
    delete m_ui;
}

void CollectionsPage::changeEngine(const QString &currEngine)
{
    if (currEngine == "SQLite") {
        m_ui->mysqlSettings->setDisabled(true);
    } else {
        m_ui->mysqlSettings->setEnabled(true);
    }
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
    m_ui->dbEngineCombo->setEnabled(checked);
    m_ui->rebuildCollectionsButton->setEnabled(checked);
    if (checked && m_ui->dbEngineCombo->currentText() == "MySQL") {
        m_ui->mysqlSettings->setEnabled(true);
    } else {
        m_ui->mysqlSettings->setEnabled(false);
    }

    m_ui->tabWidget->setTabEnabled(TAB_SOURCES, checked);
    // Always enabled
    m_ui->enableCollectionsCheckbox->setEnabled(true);
}

void CollectionsPage::loadSettings(QSettings *settings)
{
    settings->beginGroup("Collections");
    m_ui->enableCollectionsCheckbox->setChecked(settings->value("EnableCollections", true).toBool());
    m_ui->autoupdateCollectionsCheckbox->setChecked(settings->value("AutoRebm_uildAfterStart", true).toBool());
    m_ui->collectionsPathsList->addItems(settings->value("SourcePaths", QStringList()).toStringList());
    m_ui->dbEngineCombo->setCurrentIndex(settings->value("StorageEngine", 0).toInt());

    settings->beginGroup("MySQL");
    m_ui->mysqlServerEdit->setText(settings->value("Server", "127.0.0.1").toString());
    m_ui->mysqlUsernameEdit->setText(settings->value("Username", QString()).toString());
    m_ui->mysqlPasswordEdit->setText(settings->value("Password", QString()).toString());
    m_ui->mysqlDatabaseEdit->setText(settings->value("Database", QString()).toString());
    settings->endGroup();
    settings->endGroup();

    collectionStateToggled();
}

void CollectionsPage::saveSettings(QSettings *settings)
{
    settings->beginGroup("Collections");
    settings->setValue("EnableCollections", m_ui->enableCollectionsCheckbox->isChecked());
    settings->setValue("AutoRebm_uildAfterStart", m_ui->autoupdateCollectionsCheckbox->isChecked());

    QStringList items;
    for (int i = 0; i < m_ui->collectionsPathsList->count(); i++) {
        items.append(m_ui->collectionsPathsList->item(i)->text());
    }
    settings->setValue("SourcePaths", items);
    settings->setValue("StorageEngine", m_ui->dbEngineCombo->currentIndex());

    settings->beginGroup("MySQL");
    settings->setValue("Server", m_ui->mysqlServerEdit->text());
    settings->setValue("Username", m_ui->mysqlUsernameEdit->text());
    // I'd like to have the password encrypted (but not hashed!) - I don't like passwords in plaintext...
    settings->setValue("Password", m_ui->mysqlPasswordEdit->text());
    settings->setValue("Database", m_ui->mysqlDatabaseEdit->text());
    settings->endGroup();
    settings->endGroup();
}
