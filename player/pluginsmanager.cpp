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


#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <QMap>
#include <QPluginLoader>
#include <QDebug>
#include <QSettings>
#include <QTimer>
#include <phonon/mediaobject.h>

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
    QSettings settings(QString(_CONFIGDIR).append(QLatin1String("/main.conf")),
                       QSettings::IniFormat);
    settings.beginGroup(QLatin1String("Plugins"));

    QDir pluginsDir(QLatin1String(PKGDATADIR) + QDir::separator() + QLatin1String("tepsonic") + QDir::separator() + QLatin1String("plugins"));

    // Temporary list to avoid duplicate loading of plugins
    pluginsDir.setNameFilters(QStringList() << QLatin1String("*.desktop"));
    pluginsDir.setFilter(QDir::Files | QDir::NoSymLinks);

    qDebug() << "Searching plugins in " << pluginsDir;
    // Now go through all the plugins found in the current folder
    Q_FOREACH (const QString &filename, pluginsDir.entryList()) {
        // Get complete path+filename
        const QString desktopFile = pluginsDir.path() + QDir::separator() + filename;
        // If the file is a library...
        Plugin *plugin = parseDesktopFile(desktopFile);
        if (plugin) {
            const bool enabled = settings.value(plugin->id, plugin->enabledByDefault).toBool();
            if (enabled) {
                enablePlugin(plugin);
            }
            m_pluginsList << plugin;
        }
    }

    Q_EMIT pluginsLoaded();
}

PluginsManager::Plugin *PluginsManager::parseDesktopFile(const QString &filePath)
{
    QStringList mandatoryFields;
    mandatoryFields << QLatin1String("Name")
                    << QLatin1String("Comment")
                    << QLatin1String("Type")
                    << QLatin1String("X-TepSonic-Plugin-Id")
                    << QLatin1String("X-TepSonic-Plugin-Library")
                    << QLatin1String("X-TepSonic-Plugin-Version")
                    << QLatin1String("X-TepSonic-Plugin-Author");

    QSettings parser(filePath, QSettings::IniFormat);
    parser.beginGroup(QLatin1String("Desktop Entry"));
    Q_FOREACH (const QString &field, mandatoryFields) {
        if (!parser.contains(field)) {
            qDebug() << filePath << "is missing mandatory field" << field;
            return 0;
        }
    }

    const QString libraryName = parser.value(QLatin1String("X-TepSonic-Plugin-Library")).toString();
    const QString extension = QLatin1String(".so");

    const QString libPath = QLatin1String(LIBDIR) + QDir::separator() + libraryName + extension;
    if (!QFile::exists(libPath)) {
        qDebug() << libPath << "plugin library does not exist";
        return 0;
    }

    Plugin *plugin = new Plugin;
    plugin->libraryFilePath = libPath;
    plugin->name = parser.value(QLatin1String("Name")).toString();
    plugin->description = parser.value(QLatin1String("Comment")).toString();
    plugin->id = parser.value(QLatin1String("X-TepSonic-Plugin-Id")).toString();
    plugin->enabledByDefault = parser.value(QLatin1String("X-TepSonic-Plugin-EnabledByDefault"), false).toBool();
    plugin->version = parser.value(QLatin1String("X-TepSonic-Plugin->Version")).toString();
    plugin->author = parser.value(QLatin1String("X-TepSonic-Plugin-Author")).toString();
    plugin->email = parser.value(QLatin1String("X-TepSonic-Plugin-Email")).toString();

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

    disconnect(aplg,SLOT(settingsAccepted()));
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

    QPluginLoader *pluginLoader = new QPluginLoader(plugin->libraryFilePath);
    if (! pluginLoader->load()) {
        qDebug() << plugin->id << " load error:" << pluginLoader->errorString();
        return;
    }
    plugin->loader = pluginLoader;

    AbstractPlugin *aplg = reinterpret_cast<AbstractPlugin*>(plugin->loader->instance());

    // Initialize the plugin!
    aplg->init();

    // Now we can connect all the signals/slots to the plugin
    connect(this, SIGNAL(settingsAccepted()),
            aplg, SLOT(settingsAccepted()));
    connect(aplg, SIGNAL(error(QString)),
            this, SIGNAL(error(QString)));

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

#include "moc_pluginsmanager.cpp"
