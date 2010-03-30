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
#include <Phonon/MediaObject>

PluginsManager::PluginsManager(MainWindow *mainWindow, Player *player)
{
    _mainWindow = mainWindow;
    _player = player;
    loadPlugins();
}

PluginsManager::~PluginsManager()
{
    // Unload all plugins
    foreach (QPluginLoader *pluginLoader, _plugins) {
        disconnect(static_cast<AbstractPlugin*>(pluginLoader->instance()),SLOT(settingsAccepted()));
        disconnect(static_cast<AbstractPlugin*>(pluginLoader->instance()),SLOT(playerStatusChanged(Phonon::State,Phonon::State)));
        disconnect(static_cast<AbstractPlugin*>(pluginLoader->instance()),SLOT(trackChanged(MetaData)));
        disconnect(static_cast<AbstractPlugin*>(pluginLoader->instance()),SLOT(trackFinished(MetaData)));
        disconnect(static_cast<AbstractPlugin*>(pluginLoader->instance()),SLOT(trackPositionChanged(qint64)));
        static_cast<AbstractPlugin*>(pluginLoader->instance())->quit();
        pluginLoader->unload();
    }
}


void PluginsManager::loadPlugins()
{
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
    // On Linux we want to search in default library paths
    pluginsDirs << "/usr/lib/tepsonic/plugins/";
    pluginsDirs << "/usr/local/lib/tepsonic/plugins/";
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
                QObject *plugin = pluginLoader->instance();
                if (plugin) {
                    _plugins.append(pluginLoader);
                    static_cast<AbstractPlugin*>(plugin)->_initialized = false;
                }
            }
        }
    }
    initPlugins();
}

void PluginsManager::initPlugins()
{
    QSettings settings(QDir::homePath().append("/.tepsonic/main.conf"),QSettings::IniFormat,this);
    settings.beginGroup("Plugins");
    QMap<QString,QVariant> plugins = settings.value("pluginsEnabled").toMap();
    settings.endGroup();

    foreach (QPluginLoader *pluginLoader, _plugins) {
        QObject *plugin = pluginLoader->instance();
        QString pluginName = static_cast<AbstractPlugin*>(plugin)->pluginName();
        if (((!plugins.contains(pluginName)) || (plugins[pluginName].toBool()==true)) && (static_cast<AbstractPlugin*>(plugin)->_initialized==false)) {
            connect(_player,SIGNAL(trackChanged(Player::MetaData)),static_cast<AbstractPlugin*>(plugin),SLOT(trackChanged(Player::MetaData)));
            connect(_player,SIGNAL(trackFinished(Player::MetaData)),static_cast<AbstractPlugin*>(plugin),SLOT(trackFinished(Player::MetaData)));
            connect(_player,SIGNAL(stateChanged(Phonon::State,Phonon::State)),static_cast<AbstractPlugin*>(plugin),SLOT(playerStatusChanged(Phonon::State,Phonon::State)));
            connect(_player,SIGNAL(trackPositionChanged(qint64)),static_cast<AbstractPlugin*>(plugin),SLOT(trackPositionChanged(qint64)));
            connect(static_cast<AbstractPlugin*>(plugin),SIGNAL(error(QString)),_mainWindow,SLOT(showError(QString)));
            static_cast<AbstractPlugin*>(plugin)->init();
            static_cast<AbstractPlugin*>(plugin)->_initialized = true;
        }

        /* If the plugins is formally disabled, but still initialized then remove all connections and call it's metod
           quit()
        */
        if ((plugins[pluginName].toBool()==false) && (static_cast<AbstractPlugin*>(plugin)->_initialized==true)) {
            disconnect(static_cast<AbstractPlugin*>(plugin),SLOT(settingsAccepted()));
            disconnect(static_cast<AbstractPlugin*>(plugin),SLOT(playerStatusChanged(Phonon::State,Phonon::State)));
            disconnect(static_cast<AbstractPlugin*>(plugin),SLOT(trackChanged(MetaData)));
            disconnect(static_cast<AbstractPlugin*>(plugin),SLOT(trackFinished(MetaData)));
            disconnect(static_cast<AbstractPlugin*>(plugin),SLOT(trackPositionChanged(qint64)));
            static_cast<AbstractPlugin*>(plugin)->_initialized = false;
            static_cast<AbstractPlugin*>(plugin)->quit();
        }

    } // end of loop
}

int PluginsManager::pluginsCount()
{
    return _plugins.count();
}

QPluginLoader* PluginsManager::pluginAt(int index)
{
    return _plugins.at(index);
}
