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

#include "mpris2plugin.h"

#include <QDBusConnection>
#include <QtPlugin>

//getpid()
#include <unistd.h>


#include "mprismediaplayer2.h"
#include "mprismediaplayer2player.h"

MPRIS2Plugin::MPRIS2Plugin()
{
}

MPRIS2Plugin::~MPRIS2Plugin()
{
}

void MPRIS2Plugin::init()
{
    QString serviceName = QString::fromLatin1("org.mpris.MediaPlayer2.TepSonic");

    bool success = QDBusConnection::sessionBus().registerService(serviceName);

    if (!success) {
        serviceName = serviceName + QLatin1String(".instance") + QString::number(getpid());
        success = QDBusConnection::sessionBus().registerService(serviceName);
    }

    if (success) {
        m_mediaPlayer2 = new MPRISMediaPlayer2(this);
        m_mediaPlayer2Player = new MPRISMediaPlayer2Player(this);

        QDBusConnection::sessionBus().registerObject(
            QLatin1String("/org/mpris/MediaPlayer2"),
            this, QDBusConnection::ExportAdaptors);
    }
}
