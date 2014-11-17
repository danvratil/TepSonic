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

#ifndef TEPSONIC_M3U_H
#define TEPSONIC_M3U_H

#include <QStringList>

#include "tepsonic-core-export.h"

namespace TepSonic
{

namespace M3U
{

    TEPSONIC_CORE_EXPORT void writeToFile(const QStringList &playlist, const QString &file);
    TEPSONIC_CORE_EXPORT QStringList loadFromFile(const QString &file);

} // namespace M3U

} // namespace TepSonic

#endif // TEPSONIC_M3U_H
