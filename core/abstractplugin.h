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

#ifndef TEPSONIC_ABSTRACTPLUGIN_H
#define TEPSONIC_ABSTRACTPLUGIN_H

#include <QObject>

#include "tepsonic-core-export.h"

class QWidget;
class QMenu;

namespace TepSonic
{

class TEPSONIC_CORE_EXPORT AbstractPlugin : public QObject
{
    Q_OBJECT

  public:
    enum MenuTypes {
        MainMenu = 0,
        TrayMenu = 1,
        PlaylistPopup = 2,
        CollectionsPopup = 3
    };

    explicit AbstractPlugin(QObject *parent = 0);
    virtual ~AbstractPlugin();

    virtual void init() = 0;
    virtual void quit();

    virtual void configUI(QWidget *parentWidget);

    virtual void setupMenu(QMenu *menu, AbstractPlugin::MenuTypes menuType);
    virtual bool setupPane(QWidget *widget, QString &label);

  public Q_SLOTS:
    virtual void settingsAccepted();

  protected:
    void emitError(const QString &error);

  Q_SIGNALS:
    void error(const QString &msg);

  private:
    class Private;
    friend class PluginsManager;
};

} // namespace TepSonic

#endif // TEPSONIC_ABSTRACTPLUGIN_H
