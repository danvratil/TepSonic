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

#include <QtCore/QDebug>

QStringList SupportedFormats::getExtensionList()
{
    const QStringList availableMimeTypes = Phonon::BackendCapabilities::availableMimeTypes();
    const QString mimes = availableMimeTypes.join(" ");

    QStringList checkList;
    checkList << "ac3" << "flac" << "mp3" << "mp4" << "ogg" << "wav" << "wma";

    //aiff, wavpack, musepack, mpeg2

    QStringList list;

    Q_FOREACH(const QString & ext, checkList) {
        if (mimes.contains(ext)) {
            list << QString("*.%1").arg(ext);
        }
    }

    if (mimes.contains("aiff")) {
        list << "*.aif" << "*.aiff";
    }

    if (mimes.contains("ms-asf")) {
        list << "*.asf";
    }

    if (mimes.contains("mpeg2")) {
        list << "*.mp2";
    }

    if (mimes.contains("musepack")) {
        list << "*.mpc";
    }

    if (mimes.contains("realaudio")) {
        list << "*.ra" << "*.ram";
    }

    if (mimes.contains("wavpack")) {
        list << "*.wv";
    }

    // falback
    if (list.isEmpty()) {
        list << "*.flac" << "*.mp3" << "*.ogg" << "*.wav";
    }

    return list;
}
