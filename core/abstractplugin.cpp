/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <me@dvratil.cz>
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

using namespace TepSonic;

AbstractPlugin::AbstractPlugin(QObject *parent):
    QObject(parent)
{
}

AbstractPlugin::~AbstractPlugin()
{
}

void AbstractPlugin::configUI(QWidget *parentWidget)
{
    Q_UNUSED(parentWidget);
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
