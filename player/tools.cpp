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

#include <QString>
#include <QStringList>

#include "tools.h"

QString formatTimestamp(qint64 time)
{
    int weeks = time / 604800;
    int days = (time - (weeks*604800))/ 86400;
    int hours = (time - (weeks*604800) - (days*86400))/ 3600;
    int mins = (time - (weeks*604800) - (days*86400) - (hours*3600))/60;
    int secs = time - (weeks*604800) - (days*86400) - (hours*3600) - (mins*60);

    QString sWeeks;
    QString sDays;
    QString sHours;
    QString sMins;
    QString sSecs;

    if (weeks > 0) {
        sWeeks = QObject::tr("%n week(s)","",weeks).append(" ");
    }
    if (days > 0) {
        sDays = QObject::tr("%n day(s)","",days).append(" ");
    }

    if (hours<10) {
        sHours = QString("0").append(QString::number(hours));
    } else {
        sHours = QString::number(hours);
    }
    if (hours == 0) sHours = QString("00");
    if (mins<10) {
        sMins = QString("0").append(QString::number(mins));
    } else {
        sMins = QString::number(mins);
    }
    if (secs<10) {
        sSecs = QString("0").append(QString::number(secs));
    } else {
        sSecs = QString::number(secs);
    }

    return sWeeks+sDays+sHours+":"+sMins+":"+sSecs;
}

QString formatMilliseconds(qint64 msecs, bool forceHours)
{
    int secs = (int)(msecs/1000);
    int hours = secs/3600;
    int mins = (secs - (hours*3600))/60;
    secs = secs - (hours*3600) - (mins*60);

    QString sHours;
    QString sMins;
    QString sSecs;

    if (hours<10) {
        sHours = QString("0").append(QString::number(hours));
    } else {
        sHours = QString::number(hours);
    }
    if (hours == 0) sHours = QString("00");
    if (mins<10) {
        sMins = QString("0").append(QString::number(mins));
    } else {
        sMins = QString::number(mins);
    }
    if (secs<10) {
        sSecs = QString("0").append(QString::number(secs));
    } else {
        sSecs = QString::number(secs);
    }

    if ((hours > 0) || (forceHours)) 
        return sHours+":"+sMins+":"+sSecs;

    return sMins+":"+sSecs;
}

int formattedLengthToSeconds(QString formattedLength)
{
    int seconds;
    QStringList time = formattedLength.split(":",QString::SkipEmptyParts);
    if (time.size() == 3) {
        seconds = time.at(0).toInt()*3600 + time.at(1).toInt()*60 + time.at(2).toInt();
    } else {
        seconds = time.at(0).toInt()*60 + time.at(1).toInt();
    }

    return seconds;
}
