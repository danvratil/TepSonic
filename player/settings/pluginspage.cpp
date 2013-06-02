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

#include "pluginspage.h"
#include "ui_pluginspage.h"

#include "pluginsmanager.h"
#include "abstractplugin.h"

extern PluginsManager *pluginsManager;

using namespace SettingsPages;

PluginsPage::PluginsPage(QWidget *parent):
    SettingsPage(parent)
{
    m_ui = new Ui::PluginsPage();
    m_ui->setupUi(this);
    m_ui->tabs->setCurrentIndex(0);
    connect(m_ui->pluginsList, SIGNAL(itemChanged(QListWidgetItem *)),
            this, SLOT(pluginsListItemChanged(QListWidgetItem *)));
}

PluginsPage::~PluginsPage()
{
    delete m_ui;
}

void PluginsPage::pluginsListItemChanged(QListWidgetItem *item)
{
    int pluginIndex = m_ui->pluginsList->row(item);

    PluginsManager::Plugin *plugin = pluginsManager->pluginAt(pluginIndex);

    if (item->checkState() == Qt::Checked) {
        pluginsManager->enablePlugin(plugin);
        // If the plugin has a config m_ui then add a tab with the m_ui to Plugins page
        AbstractPlugin *aplg = reinterpret_cast<AbstractPlugin *>(plugin->pluginLoader->instance());
        if (aplg->hasConfigUI()) {
            QWidget *pluginWidget = new QWidget();
            m_ui->tabs->insertTab(pluginIndex + 1, pluginWidget, plugin->pluginName);
            aplg->settingsWidget(pluginWidget);
        }
    } else {
        QWidget *widget = m_ui->tabs->widget(pluginIndex + 1);
        pluginsManager->disablePlugin(plugin);
        m_ui->tabs->removeTab(pluginIndex + 1);
        delete widget;
    }
}

void PluginsPage::loadSettings(QSettings *settings)
{
    // Iterate through all plugins
    for (int i = 0; i < pluginsManager->pluginsCount(); i++) {
        if (pluginsManager->pluginAt(i)->enabled) {
            // If the plugin has a config m_ui then add a tab with the m_ui to Plugins page
            if (pluginsManager->pluginAt(i)->hasUI) {
                QWidget *pluginWidget = new QWidget();
                m_ui->tabs->addTab(pluginWidget, pluginsManager->pluginAt(i)->pluginName);
                AbstractPlugin *plugin = reinterpret_cast<AbstractPlugin *>(pluginsManager->pluginAt(i)->pluginLoader->instance());
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
        m_ui->pluginsList->addItem(item);
    }

    // PluginsManager knows better
    Q_UNUSED(settings);
}

void PluginsPage::saveSettings(QSettings *settings)
{
    settings->beginGroup("Plugins");

    QMap<QString, QVariant> plugins;
    // Go through all plugins in the m_ui list
    for (int i = 0; i < m_ui->pluginsList->count(); i++) {
        QListWidgetItem *item = m_ui->pluginsList->item(i);
        // insert into the map value pluginid - checked(bool)
        plugins.insert(pluginsManager->pluginAt(i)->pluginID,
                       QVariant((item->checkState() == 2)));
    }
    settings->setValue("pluginsEnabled", QVariant(plugins));
    settings->endGroup();
}
