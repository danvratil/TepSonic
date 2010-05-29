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
class AbstractPlugin;

//! PluginsManager is a class for loading and providing access to plugins
/*!
  PluginsManager is a object that provides API for rest of the application to communicate
  with plugins and vice versa.
*/
class PluginsManager : public QObject
{
    Q_OBJECT
    public:

        //! Constructor
        /*!
          At the end of constructor the method loadPlugins() is called to load
          all available plugins.
          \param mainWindow pointer to main window object. This allows manager to connect
          plugins' slots with main window's signals and vice versa
          \param player pointer to Player object. This allows manager to connect plugins' slots
          with player's signals and vice versa.
        */
        explicit PluginsManager(MainWindow *mainWindow, Player *player);

        //! Destructor
        /*!
          Removes all connections between plugins and main window or player and unloads it from memory
        */
        ~PluginsManager();

        //! Counts loaded plugins
        /*!
          \return Returns number of loaded plugins
        */
        int pluginsCount();

        //! Returns pointer to n-th QPluginLoader
        /*!
          Returns pointer to n-th QPluginLoader. The plugin is then accessible via instance() method.
          \param index index of plugin to return
          \return Returns pointer to QPluginLoader on given index
        */
        QPluginLoader *pluginAt(int index);

    public slots:
        //! Initializes enabled plugins
        /*!
          Initializes enabled plugins
        */
        void initPlugins();

    private slots:
        //! Loads all available plugins
        /*!
          Loads all available plugin libraries. By default it searches in following directories: \n
          ./ (on Linux and Windows) \n
          ./plugins (on Linux and Windows) \n
          /usr/lib (on Linux) \n
          /usr/local/lib (on Linux) \n
          All plugins must be prefixed libtepsonic_ and must have .so (Linux) or .dll (Windows) extension.
        */
        void loadPlugins();

    private:
        //! List of QPluginLoaders containing loaded plugins
        QList<QPluginLoader*> _plugins;

        //! Pointer to main window
        MainWindow *_mainWindow;

        //! Pointer to player
        Player *_player;

};

#endif // PLUGINSMANAGER_H