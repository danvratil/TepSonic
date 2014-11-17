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

#include "mprismediaplayer2.h"

#include <phonon/BackendCapabilities>

MPRISMediaPlayer2::MPRISMediaPlayer2(QObject* parent):
    DBusAbstractAdaptor(parent)
{
}

MPRISMediaPlayer2::~MPRISMediaPlayer2()
{
}

bool MPRISMediaPlayer2::canQuit() const
{
    return false;
}

bool MPRISMediaPlayer2::canRaise() const
{
    return false;
}

bool MPRISMediaPlayer2::hasTrackList() const
{
    return false;
}

QString MPRISMediaPlayer2::identity() const
{
    return QLatin1String("TepSonic");
}

bool MPRISMediaPlayer2::canSetFullscreen() const
{
    return false;
}

bool MPRISMediaPlayer2::fullscreen() const
{
    return false;
}

QStringList MPRISMediaPlayer2::supportedUriSchemes() const
{
    return QStringList() << QLatin1String("file");
}

QStringList MPRISMediaPlayer2::supportedMimeTypes() const
{
    return Phonon::BackendCapabilities::availableMimeTypes();
}

void MPRISMediaPlayer2::Quit()
{
}

void MPRISMediaPlayer2::Raise()
{
}
