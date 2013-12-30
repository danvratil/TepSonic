/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <dan@progdan.cz>
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

#ifndef TEPSONIC_PLUGINSMANAGER_H
#define TEPSONIC_PLUGINSMANAGER_H

#include <QObject>
#include <QList>
#include <QPluginLoader>
#include <QMenu>
#include <QTabWidget>

#include <abstractplugin.h>

#include "tepsonic-core-export.h"

namespace TepSonic
{

class TEPSONIC_CORE_EXPORT PluginsManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PluginsManager)

  public:
    class Plugin
    {
      public:
        Plugin();
        ~Plugin();

        QString name;
        QString description;
        QString id;
        bool enabledByDefault;
        bool hasConfigUI;
        QPluginLoader *loader;
        QString version;
        QString author;
        QString email;

        bool isEnabled;
    };

    static PluginsManager* instance();

    ~PluginsManager();

    int pluginsCount() const;
    Plugin *pluginAt(int index) const;

  public Q_SLOTS:
    void disablePlugin(Plugin *plugin);
    void enablePlugin(Plugin *plugin);

    void loadPlugins();

    void installMenus(QMenu *menu, AbstractPlugin::MenuTypes menuType);
    void installPanes(QTabWidget *tabwidgets);

  private:
    explicit PluginsManager();
    static PluginsManager *s_instance;

    Plugin* tryLoadPlugin(const QString &filePath);
    void initPlugin(Plugin *plugin);

    QList<Plugin *> m_pluginsList;
    QMap<QMenu *, AbstractPlugin::MenuTypes> menus;

  Q_SIGNALS:
    // Signals emitted by plugins
    void settingsAccepted();

    // PluginsManager's own signals
    void pluginsLoaded();

    void error(const QString &error);

};

} // namespace TepSonic

#endif // TEPSONIC_PLUGINSMANAGER_H
