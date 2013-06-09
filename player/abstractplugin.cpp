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

#include "abstractplugin.h"

class AbstractPlugin::Private
{
  public:
    bool hasConfigUi;
    QString pluginName;
};

AbstractPlugin::AbstractPlugin(QObject *parent):
    QObject(parent),
    d(new Private)
{
    d->hasConfigUi = false;
}

AbstractPlugin::~AbstractPlugin()
{
    delete d;
}

bool AbstractPlugin::hasConfigUI() const
{
    return d->hasConfigUi;
}

void AbstractPlugin::setHasConfigUI(bool hasConfigUi)
{
    d->hasConfigUi = hasConfigUi;
}

void AbstractPlugin::configUI(QWidget *parentWidget)
{
    Q_ASSERT(!d->hasConfigUi);

    Q_UNUSED(parentWidget);
}

QString AbstractPlugin::pluginName() const
{
    return d->pluginName;
}

void AbstractPlugin::setPluginName(const QString &pluginName)
{
    d->pluginName = pluginName;
}

void AbstractPlugin::settingsAccepted()
{
}

void AbstractPlugin::quit()
{
}

void AbstractPlugin::setupMenu(QMenu *menu, AbstractPlugin::MenuTypes menuType)
{
    Q_UNUSED(menu);
    Q_UNUSED(menuType);
}

bool AbstractPlugin::setupPane(QWidget *widget, QString &label)
{
    Q_UNUSED(widget);
    Q_UNUSED(label);

    return false;
}

void AbstractPlugin::emitError(const QString &errorMsg)
{
    Q_EMIT error(errorMsg);
}

#include "moc_abstractplugin.cpp"
