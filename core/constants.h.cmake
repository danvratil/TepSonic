/*
 * TEPSONIC
 * Copyright 2010 - 2013 Daniel Vrátil <dan@progdan.cz>
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

#ifndef TEPSONIC_CONSTANTS_H
#define TEPSONIC_CONSTANTS_H

#include <QString>
#include <stdlib.h>

#include "tepsonic-core-export.h"

#cmakedefine TEPSONIC_VERSION_MAJOR "${TEPSONIC_VERSION_MAJOR}"
#cmakedefine TEPSONIC_VERSION_MINOR "${TEPSONIC_VERSION_MINOR}"
#cmakedefine TEPSONIC_VERSION_PATCH "${TEPSONIC_VERSION_PATCH}"
#cmakedefine TEPSONIC_VERSION "${TEPSONIC_VERSION}"

namespace TepSonic
{

    TEPSONIC_CORE_EXPORT QString getConfigDir();

    // FIXME: Kill this variable
    TEPSONIC_CORE_EXPORT const QString XdgConfigDir = getConfigDir();

} // namespace TepSonic

#endif // TEPSONIC_CONSTANTS_H
