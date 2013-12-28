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

#include "constants.h"
#include "pluginsmanager.h"
#include "abstractplugin.h"
#include "mainwindow.h"
#include "player.h"
#include "settings.h"


#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <QMap>
#include <QPluginLoader>
#include <QDebug>
#include <QSettings>
#include <QTimer>

PluginsManager::Plugin::Plugin():
    isEnabled(false)
{
}

PluginsManager::Plugin::~Plugin()
{
    if (loader) {
        loader->unload();
        delete loader;
    }
}

PluginsManager* PluginsManager::s_instance = 0;

PluginsManager *PluginsManager::instance()
{
    if (s_instance == 0) {
        s_instance = new PluginsManager();
    }

    return s_instance;
}

PluginsManager::PluginsManager():
    QObject()
{
    QMetaObject::invokeMethod(this, "loadPlugins", Qt::QueuedConnection);
}

PluginsManager::~PluginsManager()
{
    // Unload all plugins
    Q_FOREACH (Plugin *plugin, m_pluginsList) {
        disablePlugin(plugin);
        delete plugin;
    }
}

void PluginsManager::loadPlugins()
{
    const QStringList enabledPlugins = Settings::instance()->enabledPlugins();

    QList<QByteArray> pluginsDirs;
    pluginsDirs << qgetenv("QT_PLUGIN_PATH").split(':');
    pluginsDirs << QByteArray(LIBDIR) + "/qt/plugins";
    pluginsDirs << QByteArray(LIBDIR) + "/qt5/plugins";
    pluginsDirs << "/usr/lib/qt/plugins";

    Q_FOREACH (const QByteArray &pluginsDir, pluginsDirs) {
        const QString tepsonicPluginsDir = QString::fromLatin1(pluginsDir) + QLatin1String("/tepsonic");
        qDebug() << "Searching plugins in " << tepsonicPluginsDir;
        QDir dir(tepsonicPluginsDir);
        // Temporary list to avoid duplicate loading of plugins
        //dir.setNameFilters(QStringList() << QLatin1String("libtepsonic_*"));
        dir.setFilter(QDir::Files | QDir::NoSymLinks);

        const QStringList plugins = dir.entryList();
        if (plugins.isEmpty()) {
            continue;
        }

        qDebug() << "Plugins:" << plugins;
        // Now go through all the plugins found in the current folder
        Q_FOREACH (const QString &filename, dir.entryList()) {
            // Get complete path+filename
            const QString file = dir.path() + QDir::separator() + filename;
            // If the file is a library...
            Plugin *plugin = tryLoadPlugin(file);
            if (plugin) {
                const bool enabled = enabledPlugins.contains(plugin->id);
                if (enabled) {
                    enablePlugin(plugin);
                }
                qDebug() << "Loaded" << plugin->id << "(" << filename << ")";
                m_pluginsList << plugin;
            }
        }
    }

    Q_EMIT pluginsLoaded();
}

PluginsManager::Plugin *PluginsManager::tryLoadPlugin(const QString &filePath)
{
    QPluginLoader *loader = new QPluginLoader(filePath);
    const QJsonObject data = loader->metaData();
    if (data.isEmpty()) {
        qWarning() << filePath << "plugin file does not contain any metadata";
        return 0;
    }

    const QString iid = data[QLatin1String("IID")].toString();
    if (!iid.startsWith(QLatin1String("cz.progdan.tepsonic.plugins"))) {
        qWarning() << filePath << "is not a valid TepSonic plugin file";
        return 0;
    }

    const QJsonObject metadata = data[QLatin1String("MetaData")].toObject();
    const QJsonObject name = metadata[QLatin1String("name")].toObject();
    const QJsonObject description = metadata[QLatin1String("description")].toObject();
    const QJsonObject author = metadata[QLatin1String("author")].toObject();
    if (name.isEmpty() || description.isEmpty() || author.isEmpty()) {
        qWarning() << filePath << "is not a valid TepSonic plugin file";
        return 0;
    }

    Plugin *plugin = new Plugin;
    plugin->id = iid;
    plugin->loader = loader;
    plugin->name =  name[QLatin1String("en")].toString();
    plugin->description = description[QLatin1String("en")].toString();
    plugin->enabledByDefault = metadata[QLatin1String("enabledByDefault")].toBool();
    plugin->version = metadata[QLatin1String("version")].toString();
    plugin->author = author[QLatin1String("name")].toString();
    plugin->email = author[QLatin1String("email")].toString();
    plugin->hasConfigUI = metadata[QLatin1String("hasConfigUI")].toBool();
    plugin->isEnabled = false;

    return plugin;
}

int PluginsManager::pluginsCount() const
{
    return m_pluginsList.count();
}

PluginsManager::Plugin* PluginsManager::pluginAt(int index) const
{
    return m_pluginsList.at(index);
}

void PluginsManager::disablePlugin(Plugin *plugin)
{
    if (!plugin->isEnabled) {
        return;
    }

    AbstractPlugin* aplg = reinterpret_cast<AbstractPlugin*>(plugin->loader->instance());
    disconnect(this, &PluginsManager::settingsAccepted,
               aplg, &AbstractPlugin::settingsAccepted);
    aplg->quit();

    plugin->loader->unload();
    delete plugin->loader;
    plugin->loader = NULL;
    plugin->isEnabled = false;
}

void PluginsManager::enablePlugin(Plugin *plugin)
{
    // There's nothing to do when plugin is enabled
    if (plugin->isEnabled) {
        return;
    }

    if (!plugin->loader->load()) {
        qDebug() << plugin->id << " load error:" << plugin->loader->errorString();
        return;
    }

    AbstractPlugin *aplg = reinterpret_cast<AbstractPlugin*>(plugin->loader->instance());

    // Initialize the plugin!
    aplg->init();

    // Now we can connect all the signals/slots to the plugin
    connect(this, &PluginsManager::settingsAccepted,
            aplg, &AbstractPlugin::settingsAccepted);
    connect(aplg, &AbstractPlugin::error,
            this, &PluginsManager::error);

    plugin->isEnabled = true;

    // Install menus for the plugins
    for (int i = 0; i < menus.size(); i++) {
        installMenus(menus.keys().at(i), menus.values().at(i));
    }
}

void PluginsManager::installMenus(QMenu *menu, AbstractPlugin::MenuTypes menuType)
{
    if (!menus.contains(menu)) {
        menus.insert(menu, menuType);
    }

    for (int i = 0; i < m_pluginsList.size(); i++) {
        Plugin *plugin = m_pluginsList.at(i);
        if (!plugin || !plugin->isEnabled) {
            continue;
        }

        AbstractPlugin *aplg = reinterpret_cast<AbstractPlugin*>(plugin->loader->instance());
        if (!aplg) {
            continue;
        }

        aplg->setupMenu(menu, menuType);
    }
}

void PluginsManager::installPanes(QTabWidget *tabwidget)
{
    for (int i = 0; i < m_pluginsList.size(); i++) {
        Plugin *plugin = m_pluginsList.at(i);
        if (!plugin || !plugin->isEnabled) {
            continue;
        }

        AbstractPlugin *aplg = reinterpret_cast<AbstractPlugin*>(plugin->loader->instance());
        if (!aplg) {
            continue;
        }

        QWidget *pane = new QWidget(tabwidget);
        QString label;
        if (aplg->setupPane(pane, label))
            tabwidget->addTab(pane, label);
    }

    // Do not let plugins to automatically activate themselves
    tabwidget->setCurrentIndex(0);
}
