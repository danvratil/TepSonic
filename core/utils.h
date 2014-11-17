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

#ifndef TEPSONIC_UTILS_H
#define TEPSONIC_UTILS_H

#include <QString>

#include "tepsonic-core-export.h"

namespace TepSonic
{

class TEPSONIC_CORE_EXPORT Utils
{

  public:

    //! Parses timestamp
    /*!
    Parses given timestamp and returns time in format "W week(s) D day(s) hh:mm:ss"
    \param time timestamp to parse
    \return Returns formatted time
    */
    static QString formatTimestamp(qint64 time, bool omitHour = false);


    //! Parses given time in milliseconds
    /*!
    Parses time given in milliseconds and returns it in format "hh:mm:ss or mm:ss when hh=0"
    \param msecs time in milliseconds
    \param forceHours force format hh:mm:ss even when hh=0
    \return Returns formatted time
    */
    static QString formatMilliseconds(qint64 msecs, bool forceHours = false);


    //! Converts formated length ([hh:]mm:ss) to integer
    /*!
    \param formattedLength string in format [hh:]mm:ss
    \return Returns length in seconds
    */
    static int formattedLengthToSeconds(const QString &formattedLength);

};

} // namespace TepSonic

#endif // TEPSONIC_UTILS_H
