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

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QPluginLoader>
#include <QtGui/QMenu>
#include <QtGui/QTabWidget>

#include "player.h"
#include "plugininterface.h"

typedef QString(*PluginNameFcn)(void);
typedef QString(*PluginIDFcn)(void);

class PluginsManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PluginsManager)

  public:
    typedef struct {
        QString pluginName;
        QString pluginID;
        QPluginLoader *pluginLoader;
        bool enabled;
        QString filename;
        bool hasUI;
    } Plugin;

    static PluginsManager* instance();

    ~PluginsManager();

    int pluginsCount() const;

    Plugin *pluginAt(int index) const;

  public Q_SLOTS:
    void disablePlugin(Plugin *plugin);

    void enablePlugin(Plugin *plugin);

    void loadPlugins();

    void installMenus(QMenu *menu, Plugins::MenuTypes menuType);
    void installPanes(QTabWidget *tabwidgets);

  private:
    explicit PluginsManager();
    static PluginsManager *s_instance;

    void initPlugin(Plugin *plugin);

    QList<Plugin *> m_pluginsList;
    PluginsManager::Plugin *loadPlugin(const QString &filename);
    QMap<QMenu *, Plugins::MenuTypes> menus;

  Q_SIGNALS:
    // Signals emitted by plugins
    void settingsAccepted();

    // PluginsManager's own signals
    void pluginsLoaded();

};

#endif // PLUGINSMANAGER_H
