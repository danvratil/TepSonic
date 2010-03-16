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


#ifndef PLUGINSMANAGER_H
#define PLUGINSMANAGER_H

#include <QObject>
#include <QList>
#include <QPluginLoader>

class MainWindow;
class Player;

class PluginsManager : public QObject
{
    Q_OBJECT
    public:
        explicit PluginsManager(MainWindow *mainWindow, Player *player);
        ~PluginsManager();

        //QList<QWidget*> settingsWidgets();
        int pluginsCount();
        QPluginLoader *pluginAt(int index);

    public slots:
        void loadPlugins();

    private:
        QList<QPluginLoader*> _plugins;
        MainWindow *_mainWindow;
        Player *_player;

};

#endif // PLUGINSMANAGER_H
