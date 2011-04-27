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
 * 
 * Contributors: Petr Vaněk 
 */

#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "pluginsmanager.h"
#include "mainwindow.h"
#include "constants.h"
#include "player.h"

#include "playerpage.h"
#include "collectionspage.h"
#include "pluginspage.h"
#include "shortcutspage.h"
#include "ui_playerpage.h"
#include "ui_collectionspage.h"
#include "ui_pluginspage.h"
#include "ui_shortcutspage.h"

#include "abstractplugin.h"

#include <QFileDialog>
#include <QDebug>
#include <QStandardItemModel>

SettingsDialog::SettingsDialog(MainWindow *parent):
        _ui(new Ui::SettingsDialog),
        _parent(parent)
{
    extern PluginsManager *pluginsManager;
    extern Player *player;

    _ui->setupUi(this);
    _ui->pagesButtons->item(0)->setSelected(true);

    connect(_ui->pagesButtons, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));


    _player = new SettingsPages::PlayerPage();
    _collections = new SettingsPages::CollectionsPage();
    _plugins = new SettingsPages::PluginsPage();
    _shortcuts = new SettingsPages::ShortcutsPage();

    connect(_collections, SIGNAL(rebuildCollections()),
            this, SLOT(emitRebuildCollections()));
    connect(_ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(dialogAccepted()));
    connect(_plugins, SIGNAL(pluginDisabled(int)),
            this, SLOT(disablePlugin(int)));
    connect(_plugins, SIGNAL(pluginEnabled(int)),
            this, SLOT(enablePlugin(int)));

    _ui->pages->addWidget(_player);
    _ui->pages->addWidget(_collections);
    _ui->pages->addWidget(_plugins);
    _ui->pages->addWidget(_shortcuts);

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
    _player->ui->outputDevicesList->setCurrentIndex (_player->getOutputDeviceModelIndex (settings.value("OutputDevice", 0).toInt()));
    m_oldOutputDeviceIndex = settings.value("OutputDevice", 0).toInt();
    QList<Phonon::Effect*> *effects = player->effects();
    for (int i = 0; i < effects->count(); i++)
    {
        Phonon::EffectDescription ed = effects->at(i)->description();
        bool state = settings.value("Effects/"+ed.name(), false).toBool();
        if (state)
            qobject_cast<QStandardItemModel*>(_player->ui->playerEffectsList->model())->item(i, 0)->setCheckState(Qt::Checked);
    }
    settings.endGroup();

    settings.beginGroup("Plugins");
    QMap<QString,QVariant> plugins = settings.value("pluginsEnabled").toMap();
    settings.endGroup();

    // Iterate through all plugins
    for (int i=0; i < pluginsManager->pluginsCount(); i++) {

        if (pluginsManager->pluginAt(i)->enabled) {
            // If the plugin has a config UI then add a tab with the UI to Plugins page
            if (pluginsManager->pluginAt(i)->hasUI) {
                QWidget *pluginWidget = new QWidget();
                _plugins->ui->tabs->addTab(pluginWidget, pluginsManager->pluginAt(i)->pluginName);
                AbstractPlugin *plugin = reinterpret_cast<AbstractPlugin*>(pluginsManager->pluginAt(i)->pluginLoader->instance());
                plugin->settingsWidget(pluginWidget);
            }
        }

        QListWidgetItem *item = new QListWidgetItem(pluginsManager->pluginAt(i)->pluginName);
        // If the plugin is not yet listed in the QSettings then set it as disabled by default
        if (pluginsManager->pluginAt(i)->enabled) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
        _plugins->ui->pluginsList->addItem(item);

    }


    settings.beginGroup("Shortcuts");
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setData(0, Qt::EditRole, tr("Play/pause"));
    item->setData(1, Qt::EditRole, settings.value("PlayPause", "Meta+Space"));
    _shortcuts->ui->shortcutsList->addTopLevelItem(item);
    item = new QTreeWidgetItem();
    item->setData(0, Qt::EditRole, tr("Stop"));
    item->setData(1, Qt::EditRole, settings.value("Stop", "Meta+S"));
    _shortcuts->ui->shortcutsList->addTopLevelItem(item);
    item = new QTreeWidgetItem();
    item->setData(0, Qt::EditRole, tr("Previous track"));
    item->setData(1, Qt::EditRole, settings.value("PrevTrack", "Meta+P"));
    _shortcuts->ui->shortcutsList->addTopLevelItem(item);
    item = new QTreeWidgetItem();
    item->setData(0, Qt::EditRole, tr("Next track"));
    item->setData(1, Qt::EditRole, settings.value("NextTrack", "Meta+N"));
    _shortcuts->ui->shortcutsList->addTopLevelItem(item);
    item = new QTreeWidgetItem();
    item->setData(0, Qt::EditRole, tr("Show/Hide window"));
    item->setData(1, Qt::EditRole, settings.value("ShowHideWin", "Meta+H"));
    _shortcuts->ui->shortcutsList->addTopLevelItem(item);
    settings.endGroup();

}

SettingsDialog::~SettingsDialog()
{
    delete _ui;
}

void SettingsDialog::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        _ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void SettingsDialog::dialogAccepted()
{

    extern PluginsManager *pluginsManager;
    extern Player *player;

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
    settings.setValue("OutputDevice", _player->getOutputDeviceIndex(_player->ui->outputDevicesList->currentIndex()));
    QList<Phonon::Effect*> *effects = player->effects();
    for (int i = 0; i < effects->count(); i++) {
        Phonon::EffectDescription ed = effects->at(i)->description();
        settings.setValue("Effects/"+ed.name(), qobject_cast<QStandardItemModel*>(_player->ui->playerEffectsList->model())->item(i,0)->checkState() == Qt::Checked);
        player->enableEffect(effects->at(i), (qobject_cast<QStandardItemModel*>(_player->ui->playerEffectsList->model())->item(i,0)->checkState() == Qt::Checked));
    }
    settings.endGroup(); // Preferences group

    settings.beginGroup("Plugins");
    QMap<QString,QVariant> plugins;
    // Go through all plugins in the UI list
    for (int i = 0; i < _plugins->ui->pluginsList->count(); i++) {
        QListWidgetItem *item = _plugins->ui->pluginsList->item(i);
        // insert into the map value pluginid - checked(bool)
        plugins.insert(pluginsManager->pluginAt(i)->pluginID,QVariant((item->checkState()==2)));
    }
    settings.setValue("pluginsEnabled",QVariant(plugins));
    settings.endGroup();

    settings.beginGroup("Shortcuts");
    settings.setValue("PlayPause", _shortcuts->ui->shortcutsList->topLevelItem(0)->data(1,Qt::EditRole));
    settings.setValue("Stop", _shortcuts->ui->shortcutsList->topLevelItem(1)->data(1,Qt::EditRole));
    settings.setValue("PrevTrack", _shortcuts->ui->shortcutsList->topLevelItem(2)->data(1,Qt::EditRole));
    settings.setValue("NextTrack", _shortcuts->ui->shortcutsList->topLevelItem(3)->data(1,Qt::EditRole));
    settings.setValue("ShowHideWin", _shortcuts->ui->shortcutsList->topLevelItem(4)->data(1,Qt::EditRole));
    settings.endGroup();

    if (_collections->collectionsSourceChanged()) {
        emit rebuildCollections();
    }

    if (settings.value("Preferences/OutputDevice", 0).toInt() != m_oldOutputDeviceIndex)
        emit outputDeviceChanged();

    accept();
    this->close();
}

void SettingsDialog::changePage(QListWidgetItem* current, QListWidgetItem* previous)
{
    Q_UNUSED(previous);
    _ui->pages->setCurrentIndex(_ui->pagesButtons->row(current));
}

void SettingsDialog::emitRebuildCollections()
{
    // Save current state of collections configurations and emit rebuilding

    QSettings settings(QString(_CONFIGDIR).append("/main.conf"),QSettings::IniFormat,this);
    settings.beginGroup("Collections");
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
    emit rebuildCollections();
}

void SettingsDialog::enablePlugin(int pluginIndex)
{
    extern PluginsManager *pluginsManager;

    pluginsManager->enablePlugin(pluginsManager->pluginAt(pluginIndex));

    if (!pluginsManager->pluginAt(pluginIndex) || !pluginsManager->pluginAt(pluginIndex)->pluginLoader)
        return;

    AbstractPlugin *plugin = reinterpret_cast<AbstractPlugin*>(pluginsManager->pluginAt(pluginIndex)->pluginLoader->instance());
    // If the plugin has a config UI then add a tab with the UI to Plugins page
    if (plugin->hasConfigUI()) {
        QWidget *pluginWidget = new QWidget();
        _plugins->ui->tabs->insertTab(pluginIndex+1, pluginWidget, pluginsManager->pluginAt(pluginIndex)->pluginName);
        plugin->settingsWidget(pluginWidget);
    }
}

void SettingsDialog::disablePlugin(int pluginIndex)
{
    extern PluginsManager *pluginsManager;

    pluginsManager->disablePlugin(pluginsManager->pluginAt(pluginIndex));
    _plugins->ui->tabs->removeTab(pluginIndex+1);
}
