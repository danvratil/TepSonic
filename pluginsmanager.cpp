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
#include <QPluginLoader>
#include <QDebug>
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
        disconnect(static_cast<AbstractPlugin*>(pluginLoader->instance()),SLOT(trackChanged(QString)));
        disconnect(static_cast<AbstractPlugin*>(pluginLoader->instance()),SLOT(trackPositionChanged(qint64)));
        pluginLoader->unload();
    }
}


void PluginsManager::loadPlugins()
{
    QDir pluginsDir = QDir(qApp->applicationDirPath());
    pluginsDir.cd("plugins");
    foreach(QString filename, pluginsDir.entryList(QDir::Files)) {
        qDebug() << filename;
        if (QLibrary::isLibrary(filename)) {
            qDebug() << "Loading plugin " << pluginsDir.absoluteFilePath(filename);
            QPluginLoader *pluginLoader = new QPluginLoader(pluginsDir.absoluteFilePath(filename));
            QObject *plugin = pluginLoader->instance();
            if (plugin) {
                _plugins.append(pluginLoader);
                connect(_player,SIGNAL(trackChanged(QString)),static_cast<AbstractPlugin*>(plugin),SLOT(trackChanged(QString)));
                connect(_player,SIGNAL(stateChanged(Phonon::State,Phonon::State)),static_cast<AbstractPlugin*>(plugin),SLOT(playerStatusChanged(Phonon::State,Phonon::State)));
                connect(_player,SIGNAL(trackPositionChanged(qint64)),static_cast<AbstractPlugin*>(plugin),SLOT(trackPositionChanged(qint64)));
            }
        }
    }
}

int PluginsManager::pluginsCount()
{
    return _plugins.count();
}

QPluginLoader* PluginsManager::pluginAt(int index)
{
    return _plugins.at(index);
}
