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

        //! Installs plugin's UI on given parentWidget.
        virtual void settingsWidget(QWidget *parentWidget) = 0;

        //! Returns name of plugin
        virtual QString pluginName() = 0;
};

Q_DECLARE_INTERFACE(PluginInterface,"TepSonic.PluginInterface/1.0");

#endif // PLUGININTERFACE_H
