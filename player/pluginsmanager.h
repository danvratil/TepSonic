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
#include <QMenu>

#include "player.h"
#include "plugininterface.h"


typedef QString (*PluginNameFcn)(void);
typedef QString (*PluginIDFcn)(void);

//! PluginsManager is a class for loading and providing access to plugins
/*!
  PluginsManager is a object that provides API for rest of the application to communicate
  with plugins and vice versa.
*/
class PluginsManager : public QObject
{
    Q_OBJECT
    public:
        struct Plugin {
            QString pluginName;
            QString pluginID;
            QPluginLoader *pluginLoader;
            bool enabled;
            QString filename;
            bool hasUI;
        };

        //! Constructor
        explicit PluginsManager();

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
        struct Plugin* pluginAt(int index);

    public slots:
        //! Disables given plugin
        /*!
          Disable given plugin
          \param plugin
        */
        void disablePlugin(Plugin *plugin);

        //! Enables given plugin
        /*!
          Enable given plugin
          \param plugin
        */
        void enablePlugin(Plugin *plugin);

        //! Loads all available plugins
        /*!
          Loads all available plugin libraries.
          All plugins must be prefixed libtepsonic_.
        */
        void loadPlugins();

        //! Call setupMenu() for all loaded plugins */
        void installMenus(QMenu *menu, Plugins::MenuTypes menuType);

    private:
        //! Initializes given plugin
        /*!
          Initializes given plugin
        */
        void initPlugin(Plugin *plugin);

        //! List of Plugins containing loaded plugins
        QList<Plugin*> m_pluginsList;

        PluginsManager::Plugin* loadPlugin(QString filename);

        //! List of QMenus that plugins can install their menus to
        QMap<QMenu*,Plugins::MenuTypes> menus;

    signals:
         void settingsAccepted();
         void playerStatusChanged(Phonon::State,Phonon::State);
         void trackChanged(Player::MetaData);
         void trackFinished(Player::MetaData);
         void trackPositionChanged(qint64);
         void trackPaused(bool);
         void error(QString);



};

#endif // PLUGINSMANAGER_H
