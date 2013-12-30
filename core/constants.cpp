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

#include "constants.h"

#include <stdlib.h>
#include <QDir>

namespace TepSonic
{

QString getConfigDir()
{

    QString configdir;

    if (getenv("XDG_CONFIG_HOME") == 0) {
        configdir = QString::fromLatin1(getenv("HOME"));
        configdir.append(QLatin1String("/.config/tepsonic"));
    } else {
        configdir = QString::fromLatin1(getenv("XDG_CONFIG_HOME"));
        configdir.append(QLatin1String("/tepsonic"));
    }

    return configdir;
}

}
