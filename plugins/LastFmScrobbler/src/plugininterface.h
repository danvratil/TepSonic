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

#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>


class QWidget;
class QString;
class QMenu;


namespace Plugins {

    enum MenuTypes {
        MainMenu = 0,
        TrayMenu = 1,
        PlaylistPopup = 2,
        CollectionsPopup = 3
    };

}

//! Interface for plugins. Plugins should not be subclassed from this interface, use AbstractPlugin class instead
/*!
  PluginInterface is an interface that must be common for all plugins. All it's methods are pure virtual.
  It allows Qt to check if plugins were build against the same interface as the current application provides. In
  other case, the plugin will not be loaded.
  See AbstractPlugin for full description of plugins API.
 */
class PluginInterface
{
    public:

        //! Destructor
        virtual ~PluginInterface() {}

        //! Initializes the plugin
        /*!
          The plugin should not be initiated in the constructor. The constructor is only for setting plugin's name
          and hasConfigUI. The initialization itself (like loading settings, connection to network etc) should be done
          here. This allows user to disable the plugin (init() is not called) but TepSonic still knows about the plugin
          without initializing it.
        */
        virtual void init() = 0;

        //! Uninitializes the plugin
        /*!
          The plugin should stop doing it's work but it won't be destroyed. The plugin will be formally disabled.
        */
        virtual void quit() = 0;

        //! Installs plugin's UI on given parentWidget.
        virtual void settingsWidget(QWidget *parentWidget) = 0;

        //! Allows plugin to setup custom menu to givem menu. The type of menu is set in menuType.
        virtual void setupMenu(QMenu *menu, Plugins::MenuTypes menuType) = 0;

};


Q_DECLARE_INTERFACE(PluginInterface,"TepSonic.PluginInterface/1.0");

#endif // PLUGININTERFACE_H
