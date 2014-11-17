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

#include <QString>
#include <QStringList>
#include <QTime>
#include <QDebug>

#include "utils.h"

using namespace TepSonic;

QString Utils::formatTimestamp(qint64 secs, bool omitHour)
{
    int weeks = secs / 604800;
    int days = (secs - (weeks * 604800)) / 86400;

    QString string;

    if (weeks > 0) {
        string += QObject::tr("%n week(s)", "",weeks) + QLatin1Char(' ');
    }
    if (days > 0) {
        string += QObject::tr("%n day(s)", "",days) + QLatin1Char(' ');
    }

    QTime time(0, 0, 0);
    time = time.addSecs(secs);

    if (!omitHour || time.hour() > 0) {
        return string + time.toString(QLatin1String("hh:mm:ss"));
    } else {
        return string + time.toString(QLatin1String("mm:ss"));
    }

    return string;
}

QString Utils::formatMilliseconds(qint64 msecs, bool forceHours)
{
    QTime time(0, 0, 0);
    time = time.addMSecs(msecs);
    if (forceHours || time.hour() > 0) {
        return time.toString(QLatin1String("hh:mm:ss"));
    } else {
        return time.toString(QLatin1String("mm:ss"));
    }
}

int Utils::formattedLengthToSeconds(const QString &formattedLength)
{
    QTime time(0, 0, 0);
    const QStringList times = formattedLength.split(QLatin1Char(':'), QString::SkipEmptyParts);
    if (times.size() == 3) {
        time = QTime::fromString(formattedLength, QLatin1String("hh:mm:ss"));
    } else {
        time = QTime::fromString(formattedLength, QLatin1String("mm:ss"));
    }

    return -time.msecsTo(QTime());
}
