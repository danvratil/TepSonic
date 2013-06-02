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
#include <phonon/mediaobject.h>

PluginsManager::PluginsManager(){}

PluginsManager::~PluginsManager()
{
    // Unload all plugins
    foreach (Plugin *plugin, m_pluginsList) {
        disablePlugin(plugin);
        delete plugin;
    }
}


void PluginsManager::loadPlugins()
{
    QSettings settings(QString(_CONFIGDIR).append("/main.conf"),
                       QSettings::IniFormat);
    settings.beginGroup("Plugins");
    QMap<QString,QVariant> pluginsEnabledList = settings.value("pluginsEnabled").toMap();
    settings.endGroup();

    QStringList pluginsDirs;
    /* By default we will search in applicationDirPath/plugins
       This is common situation on Windows where the plugins will probably be stored in
       C:\Program Files\TepSonic\plugins
       On Linux it can be usefull for developers who havie their TepSonic source
       for example in /home/me/tepsonic/ and plugins in /home/me/tepsonic/plugins.
       Putting this folder first will also result in preferring these plugins before
       same-called plugins installed somewhere in system */
#ifdef Q_WS_WIN
    pluginsDirs << QDir::toNativeSeparators(qApp->applicationDirPath())+QDir::separator()+"plugins";
    QCoreApplication::addLibraryPath(QDir::toNativeSeparators(qApp->applicationDirPath()));
#endif
#ifdef Q_WS_MAC
    #ifdef APPLEBUNDLE
    pluginsDirs << QCoreApplication::applicationDirPath() + "/../plugins/";
    #else
    pluginsDirs << LIBDIR;
    #endif
    qDebug() << "Searching in " << pluginsDirs;
#endif
#ifdef Q_WS_X11
    QDir appDir(qApp->applicationDirPath());
    appDir.cdUp();
    pluginsDirs << appDir.path()+QDir::separator()+"lib";
    // LIBDIR is defined in player/CMakeLists.txt and it is a location where all project's libs are installed
    if (LIBDIR != appDir.path()+QDir::separator()+"lib") {
        pluginsDirs << LIBDIR;
    }
    qDebug() << "Searching in " << pluginsDirs;
#endif

    // Temporary list to avoid duplicate loading of plugins
    QStringList pluginsNames;
    QDir pluginsDir;
    pluginsDir.setNameFilters(QStringList() << "libtepsonic_*");
    pluginsDir.setFilter(QDir::Files | QDir::NoSymLinks);
    foreach(QString folder, pluginsDirs) {
        // Use each of the pluginsDirs
        pluginsDir.setPath(folder);
        // Now go through all the plugins found in the current folder
        foreach(QString filename, pluginsDir.entryList()) {
            // Get complete path+filename
            QString pluginFile = folder+QDir::separator()+filename;
            // If the file is a library...
            if (QLibrary::isLibrary(pluginFile)) {

                // Load the library and resolve "pluginName" symbol
                QLibrary lib(pluginFile);
                PluginIDFcn pluginID = (PluginIDFcn)lib.resolve("pluginID");
                if (pluginID == 0) {
                    qDebug() << lib.errorString() << "...skipping!";
                    continue;
                }

                QString pluginid = pluginID();

                if (!pluginsNames.contains(pluginid)) {
                    qDebug() << "Found plugin " << pluginid;
                    Plugin *plugin = loadPlugin(pluginFile);
                    if (pluginsEnabledList[pluginid]==true) {
                        enablePlugin(plugin);
                    }
                    pluginsNames.append(pluginid);
                }
            }
        }
    }

    emit pluginsLoaded();
}

int PluginsManager::pluginsCount()
{
    return m_pluginsList.count();
}

struct PluginsManager::Plugin* PluginsManager::pluginAt(int index)
{
    return m_pluginsList.at(index);
}

void PluginsManager::disablePlugin(Plugin *plugin)
{
    if (!plugin->enabled) return;

    AbstractPlugin* aplg = reinterpret_cast<AbstractPlugin*>(plugin->pluginLoader->instance());

    disconnect(aplg,SLOT(settingsAccepted()));
    aplg->quit();

    plugin->pluginLoader->unload();
    delete plugin->pluginLoader;

    plugin->pluginLoader = NULL;
    plugin->hasUI = false;
    plugin->enabled = false;
}

void PluginsManager::enablePlugin(Plugin *plugin)
{
    // There's nothing to do when plugin is enabled
    if (plugin->enabled) return;

    QPluginLoader *pluginLoader = new QPluginLoader(plugin->filename);
    if (! pluginLoader->load()) {
        qDebug() << plugin->pluginName << " load error:" << pluginLoader->errorString();
        return;
    }
    plugin->pluginLoader = pluginLoader;


    AbstractPlugin *aplg = reinterpret_cast<AbstractPlugin*>(plugin->pluginLoader->instance());

    // Initialize the plugin!
    aplg->init();
    aplg->m_initialized = true;

    // Now we can connect all the signals/slots to the plugin
    connect(this,SIGNAL(settingsAccepted()),aplg,SLOT(settingsAccepted()));
    connect(aplg,SIGNAL(error(QString)),this,SIGNAL(error(QString)));

    plugin->hasUI = aplg->hasConfigUI();
    plugin->enabled = true;

    // Install menus for the plugins
    for (int i = 0; i < menus.size(); i++)
        installMenus(menus.keys().at(i), menus.values().at(i));

}

PluginsManager::Plugin* PluginsManager::loadPlugin(QString filename)
{
    // Check if the plugin isn't already loaded
    QLibrary lib(filename);
    PluginIDFcn pluginID = (PluginIDFcn)lib.resolve("pluginID");
    QString pluginid = pluginID();

    for (int i = 0; i < m_pluginsList.count(); i++) {
        if (m_pluginsList.at(i)->pluginID == pluginid)
            return m_pluginsList.at(i);
    }

    PluginNameFcn pluginName = (PluginNameFcn)lib.resolve("pluginName");

    // Create new Plugin structure
    Plugin *plugin = new PluginsManager::Plugin;
    plugin->pluginLoader = NULL;
    plugin->enabled = false;
    plugin->pluginName = pluginName();
    plugin->pluginID = pluginid;
    plugin->filename = filename;
    plugin->hasUI = false;

    // Add it to the list of plugins
    m_pluginsList.append(plugin);

    return plugin;
}

void PluginsManager::installMenus(QMenu *menu, Plugins::MenuTypes menuType)
{
    if (!menus.contains(menu))
        menus.insert(menu, menuType);

    for (int i = 0; i < m_pluginsList.size(); i++)
    {
        Plugin *plugin = m_pluginsList.at(i);
        if (!plugin) continue;

        if (!plugin->enabled) continue;

        AbstractPlugin *aplg = reinterpret_cast<AbstractPlugin*>(plugin->pluginLoader->instance());
        if (!aplg) continue;

        aplg->setupMenu(menu, menuType);
    }
}

void PluginsManager::installPanes(QTabWidget *tabwidget)
{
    for (int i = 0; i < m_pluginsList.size(); i++)
    {
        Plugin *plugin = m_pluginsList.at(i);
        if (!plugin) continue;

        if (!plugin->enabled) continue;

        AbstractPlugin *aplg = reinterpret_cast<AbstractPlugin*>(plugin->pluginLoader->instance());
        if (!aplg) continue;

        QWidget *pane = new QWidget(tabwidget);
        QString label;
        if (aplg->setupPane(pane, &label))
            tabwidget->addTab(pane, label);
    }

    // Do not let plugins to automatically activate themselves
    tabwidget->setCurrentIndex(0);
}
