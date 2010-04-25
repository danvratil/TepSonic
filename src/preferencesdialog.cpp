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

#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "preferencespages.h"
#include "pluginsmanager.h"
#include "mainwindow.h"
#include "constants.h"

#include "abstractplugin.h"

#include <QFileDialog>
#include <QDebug>

PreferencesDialog::PreferencesDialog(MainWindow *parent):
    _ui(new Ui::PreferencesDialog)
{
    _ui->setupUi(this);

    _parent = parent;

    _player = new PreferencesPages::Player;
    _collections = new PreferencesPages::Collections;
    _plugins = new PreferencesPages::Plugins;
    connect(_collections,SIGNAL(rebuildCollections()),
            this,SIGNAL(rebuildCollections()));

    _ui->pages->addWidget(_player);
    _ui->pages->addWidget(_collections);
    _ui->pages->addWidget(_plugins);

    QSettings settings(QString(_CONFIGDIR).append("/main.conf"),QSettings::IniFormat,this);
    settings.beginGroup("Collections");
    _collections->ui->enableCollectionsCheckbox->setChecked(settings.value("EnableCollections",true).toBool());
    _collections->ui->autoupdateCollectionsCheckbox->setChecked(settings.value("AutoRebuildAfterStart",true).toBool());
    _collections->ui->collectionsPathsList->addItems(settings.value("SourcePaths",QStringList()).toStringList());
    _collections->ui->dbEngineCombo->setCurrentIndex(settings.value("StorageEngine",0).toInt());
    settings.beginGroup("MySQL");
    _collections->ui->mysqlServerEdit->setText(settings.value("Server","127.0.0.1").toString());
    _collections->ui->mysqlUsernameEdit->setText(settings.value("Username",QString()).toString());
    _collections->ui->mysqlPasswordEdit->setText(settings.value("Password",QString()).toString());
    _collections->ui->mysqlDatabaseEdit->setText(settings.value("Database",QString()).toString());
    settings.endGroup();
    settings.endGroup();
    settings.beginGroup("Preferences");
    _player->ui->restorePreviousSessionCheckBox->setChecked(settings.value("RestoreSession",true).toBool());
    settings.endGroup();

    settings.beginGroup("Plugins");
    QMap<QString,QVariant> plugins = settings.value("pluginsEnabled").toMap();
    settings.endGroup();

    // Iterate through all plugins
    for (int i=0; i < _parent->pluginsManager()->pluginsCount(); i++) {

        // Get plugin name
        QString pluginName = static_cast<AbstractPlugin*>(_parent->pluginsManager()->pluginAt(i)->instance())->pluginName();

        // If the plugin has a config UI then add a tab with the UI to Plugins page
        if (static_cast<AbstractPlugin*>(_parent->pluginsManager()->pluginAt(i)->instance())->hasConfigUI()) {
            QWidget *pluginWidget = new QWidget();
            _plugins->ui->tabs->addTab(pluginWidget,pluginName);
            static_cast<AbstractPlugin*>(_parent->pluginsManager()->pluginAt(i)->instance())->settingsWidget(pluginWidget);
        }

        bool enabled;
        // If the plugin is not yet listed in the settings then set it enabled by default
        if (plugins.contains(pluginName)) {
            enabled = plugins[pluginName].toBool();
        } else {
            enabled = true;
        }

        // Add new item to plugins list on Plugins page
        QListWidgetItem *item = new QListWidgetItem(pluginName);
        if (enabled) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
        _plugins->ui->pluginsList->addItem(item);
    }

}

PreferencesDialog::~PreferencesDialog()
{
    delete _ui;
}

void PreferencesDialog::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        _ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void PreferencesDialog::on_buttonBox_accepted()
{
    QSettings settings(QString(_CONFIGDIR).append("/main.conf"),QSettings::IniFormat,this);
    settings.beginGroup("Collections");
    settings.setValue("EnableCollections",_collections->ui->enableCollectionsCheckbox->isChecked());
    settings.setValue("AutoRebuildAfterStart",_collections->ui->autoupdateCollectionsCheckbox->isChecked());
    QStringList items;
    for (int i = 0; i < _collections->ui->collectionsPathsList->count(); i++) {
        items.append(_collections->ui->collectionsPathsList->item(i)->text());
    }
    settings.setValue("SourcePaths",items);
    settings.setValue("StorageEngine",_collections->ui->dbEngineCombo->currentIndex());
    settings.beginGroup("MySQL");
    settings.setValue("Server",_collections->ui->mysqlServerEdit->text());
    settings.setValue("Username",_collections->ui->mysqlUsernameEdit->text());
    // I'd like to have the password encrypted (but not hashed!) - I don't like password in plaintext...
    settings.setValue("Password",_collections->ui->mysqlPasswordEdit->text());
    settings.setValue("Database",_collections->ui->mysqlDatabaseEdit->text());
    settings.endGroup(); // MySQL group
    settings.endGroup(); // Collections group
    settings.beginGroup("Preferences");
    settings.setValue("RestoreSession",_player->ui->restorePreviousSessionCheckBox->isChecked());
    settings.endGroup(); // Preferences group
    settings.beginGroup("Plugins");
    QMap<QString,QVariant> plugins;
    for (int i = 0; i < _plugins->ui->pluginsList->count(); i++) {
        QListWidgetItem *item = _plugins->ui->pluginsList->item(i);
        plugins.insert(item->text(),QVariant(item->checkState()));
    }
    settings.setValue("pluginsEnabled",QVariant(plugins));
    settings.endGroup();

    if (_collections->collectionsSourceChanged()) {
        emit rebuildCollections();
    }

    emit accepted();
    this->close();
}

void PreferencesDialog::on_buttonBox_rejected()
{
    this->close();
}

void PreferencesDialog::on_pagesButtons_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
    Q_UNUSED(previous);
    _ui->pages->setCurrentIndex(_ui->pagesButtons->row(current));
}
