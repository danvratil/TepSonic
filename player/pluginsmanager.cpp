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


#include "constants.h"
#include "pluginsmanager.h"
#include "plugininterface.h"
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

PluginsManager::PluginsManager()
{
    _lastPluginID = 0;
}

PluginsManager::~PluginsManager()
{
    // Unload all plugins
    foreach (Plugin *plugin, _pluginsList) {
        disablePlugin(plugin);
        plugin->pluginLoader->unload();
    }
}


void PluginsManager::loadPlugins()
{
    QSettings settings(QString(_CONFIGDIR).append("/main.conf"),QSettings::IniFormat,this);
    settings.beginGroup("Plugins");
    QMap<QString,QVariant> pluginsEnabled = settings.value("pluginsEnabled").toMap();
    settings.endGroup();

    QStringList pluginsDirs;
    /* By default we will search in applicationDirPath/plugins
       This is common situation on Windows where the plugins will probably be stored in
       C:\Program Files\TepSonic\plugins
       On Linux it can be usefull for developers who havie their TepSonic source
       for example in /home/me/tepsonic/ and plugins in /home/me/tepsonic/plugins.
       Putting this folder first will also result in preferring these plugins before
       same-called plugins installed somewhere in system */
    pluginsDirs << qApp->applicationDirPath()+QDir::separator()+"plugins";
    // Default extension on windows
    QString ext = "dll";
#ifndef Q_OS_WIN32
    // LIBDIR is defined in dirs.pri and it is a location where all project's libs are installed
    pluginsDirs << LIBDIR;
    qDebug() << "Searching in " << pluginsDirs;
    // If we are on Linux, override the extension
    ext = "so";
#endif

    QDir pluginsDir;
    pluginsDir.setNameFilters(QStringList() << "libtepsonic_*."+ext);
    foreach(QString folder, pluginsDirs) {
        pluginsDir.setPath(folder);
        foreach(QString filename, pluginsDir.entryList(QDir::Files)) {
            if (QLibrary::isLibrary(filename)) {
                qDebug() << "Loading plugin " << pluginsDir.absoluteFilePath(filename);

                QPluginLoader *pluginLoader = new QPluginLoader(pluginsDir.absoluteFilePath(filename));
                pluginLoader->load();
                Plugin *plugin = new PluginsManager::Plugin;
                plugin->pluginLoader = pluginLoader;
                plugin->pluginID = (_lastPluginID+1);
                AbstractPlugin *aplg = qobject_cast<AbstractPlugin*>(pluginLoader->instance());
                plugin->pluginName = aplg->pluginName();
                if ((pluginsEnabled.contains(plugin->pluginName)) || (pluginsEnabled[plugin->pluginName]==true)) {
                    enablePlugin(plugin);
                } else {
                    plugin->enabled = false;
                }

                _pluginsList.append(plugin);

                _lastPluginID++;
            }
        }
    }
}

int PluginsManager::pluginsCount()
{
    return _pluginsList.count();
}

struct PluginsManager::Plugin* PluginsManager::pluginAt(int index)
{
    return _pluginsList.at(index);
}

void PluginsManager::disablePlugin(Plugin *plugin)
{
    if (!plugin->enabled) return;

    AbstractPlugin* aplg = qobject_cast<AbstractPlugin*>(plugin->pluginLoader->instance());

    disconnect(aplg,SLOT(settingsAccepted()));
    disconnect(aplg,SLOT(playerStatusChanged(Phonon::State,Phonon::State)));
    disconnect(aplg,SLOT(trackChanged(Player::MetaData)));
    disconnect(aplg,SLOT(trackFinished(Player::MetaData)));
    disconnect(aplg,SLOT(trackPositionChanged(qint64)));
    aplg->quit();
    plugin->enabled=false;
}

void PluginsManager::enablePlugin(Plugin *plugin)
{
    if (plugin->enabled) return;

    AbstractPlugin *aplg = qobject_cast<AbstractPlugin*>(plugin->pluginLoader->instance());

    connect(this,SIGNAL(trackChanged(Player::MetaData)),aplg,SLOT(trackChanged(Player::MetaData)));
    connect(this,SIGNAL(trackFinished(Player::MetaData)),aplg,SLOT(trackFinished(Player::MetaData)));
    connect(this,SIGNAL(stateChanged(Phonon::State,Phonon::State)),aplg,SLOT(playerStatusChanged(Phonon::State,Phonon::State)));
    connect(this,SIGNAL(trackPositionChanged(qint64)),aplg,SLOT(trackPositionChanged(qint64)));
    connect(aplg,SIGNAL(error(QString)),this,SIGNAL(error(QString)));
    aplg->init();
    aplg->_initialized = true;
    plugin->enabled = true;
}
