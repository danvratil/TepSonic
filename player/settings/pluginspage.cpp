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

#include "pluginspage.h"
#include "ui_pluginspage.h"

#include "pluginsmanager.h"
#include "abstractplugin.h"

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

    PluginsManager *pluginsManager = PluginsManager::instance();
    PluginsManager::Plugin *plugin = pluginsManager->pluginAt(pluginIndex);

    if (item->checkState() == Qt::Checked) {
        pluginsManager->enablePlugin(plugin);
        // If the plugin has a config m_ui then add a tab with the m_ui to Plugins page
        AbstractPlugin *aplg = reinterpret_cast<AbstractPlugin *>(plugin->loader->instance());
        if (aplg->hasConfigUI()) {
            QWidget *pluginWidget = new QWidget();
            m_ui->tabs->insertTab(pluginIndex + 1, pluginWidget, plugin->name);
            aplg->configUI(pluginWidget);
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
    PluginsManager *pluginsManager = PluginsManager::instance();
    // Iterate through all plugins
    for (int i = 0; i < pluginsManager->pluginsCount(); i++) {
        if (pluginsManager->pluginAt(i)->isEnabled) {
            // If the plugin has a config m_ui then add a tab with the m_ui to Plugins page
            AbstractPlugin *plugin = reinterpret_cast<AbstractPlugin *>(pluginsManager->pluginAt(i)->loader->instance());
            if (plugin->hasConfigUI()) {
                QWidget *pluginWidget = new QWidget();
                m_ui->tabs->addTab(pluginWidget, pluginsManager->pluginAt(i)->name);
                plugin->configUI(pluginWidget);
            }
        }

        QListWidgetItem *item = new QListWidgetItem(pluginsManager->pluginAt(i)->name);
        // If the plugin is not yet listed in the QSettings then set it as disabled by default
        if (pluginsManager->pluginAt(i)->isEnabled) {
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
    settings->beginGroup(QLatin1String("Plugins"));

    // Go through all plugins in the m_ui list
    for (int i = 0; i < m_ui->pluginsList->count(); i++) {
        QListWidgetItem *item = m_ui->pluginsList->item(i);
        settings->setValue(PluginsManager::instance()->pluginAt(i)->id, QVariant((item->checkState() == 2)));
    }
    settings->endGroup();
}

#include "moc_pluginspage.cpp"
