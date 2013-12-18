/*
 * TEPSONIC
 * Copyright 2010 David Watzke <david@watzke.cz>
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

#include "supportedformats.h"
#include <phonon/backendcapabilities.h>

#include <QDebug>

QStringList SupportedFormats::extensionsList()
{
    const QStringList availableMimeTypes = Phonon::BackendCapabilities::availableMimeTypes();
    const QString mimes = availableMimeTypes.join(QLatin1String(" "));

    QStringList checkList;
    checkList << QLatin1String("ac3") << QLatin1String("flac")
              << QLatin1String("mp3") << QLatin1String("mp4")
              << QLatin1String("ogg") << QLatin1String("wav")
              << QLatin1String("wma");

    //aiff, wavpack, musepack, mpeg2

    QStringList list;

    Q_FOREACH(const QString & ext, checkList) {
        if (mimes.contains(ext)) {
            list << QString::fromLatin1("*.%1").arg(ext);
        }
    }

    if (mimes.contains(QLatin1String("aiff"))) {
        list << QLatin1String("*.aif") << QLatin1String("*.aiff");
    }

    if (mimes.contains(QLatin1String("ms-asf"))) {
        list << QLatin1String("*.asf");
    }

    if (mimes.contains(QLatin1String("mpeg2"))) {
        list << QLatin1String("*.mp2");
    }

    if (mimes.contains(QLatin1String("musepack"))) {
        list << QLatin1String("*.mpc");
    }

    if (mimes.contains(QLatin1String("realaudio"))) {
        list << QLatin1String("*.ra") << QLatin1String("*.ram");
    }

    if (mimes.contains(QLatin1String("wavpack"))) {
        list << QLatin1String("*.wv");
    }

    // falback
    if (list.isEmpty()) {
        list << QLatin1String("*.flac") << QLatin1String("*.mp3")
             << QLatin1String("*.ogg") << QLatin1String("*.wav");
    }

    return list;
}
