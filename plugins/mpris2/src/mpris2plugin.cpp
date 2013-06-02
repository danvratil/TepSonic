/*
 * TEPSONIC
 * Copyright 2013 Dan Vrátil <dan@progdan.cz>
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

#include <QtDBus/QDBusConnection>

#include <unistd.h>

#include "mprismediaplayer2.h"
#include "mprismediaplayer2player.h"

// Exports pluginName method
#ifdef Q_WS_WIN
#define NAME_EXPORT __declspec(dllexport)
#define ID_EXPORT __declspec(dllexport)
#else
#define NAME_EXPORT
#define ID_EXPORT
#endif

extern "C" NAME_EXPORT QString pluginName()
{
    return "MPRIS2 plugin";
}

extern "C" ID_EXPORT QString pluginID()
{
    return "mpris2";
}

MPRIS2Plugin::MPRIS2Plugin()
{
}

MPRIS2Plugin::~MPRIS2Plugin()
{
}

void MPRIS2Plugin::init()
{
    QString serviceName("org.mpris.MediaPlayer2.TepSonic");

    bool success = QDBusConnection::sessionBus().registerService(serviceName);

    if (!success) {
        serviceName = serviceName + ".instance" + QString::number(getpid());
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

void MPRIS2Plugin::quit()
{
}

void MPRIS2Plugin::settingsWidget(QWidget* parentWidget)
{
    Q_UNUSED(parentWidget);
}

void MPRIS2Plugin::setupMenu(QMenu* menu, Plugins::MenuTypes menuType)
{
    Q_UNUSED(menu);
    Q_UNUSED(menuType);
}

Q_EXPORT_PLUGIN2(tepsonic_mpris2plugin, MPRIS2Plugin)
