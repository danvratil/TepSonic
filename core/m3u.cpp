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

#include "m3u.h"

#include <QFile>
#include <QDir>
#include <QDebug>

#define M3UHEADER "#EXT3MU"

using namespace TepSonic;

QStringList M3U::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "Failed to open file " << filePath << "for reading";
        return QStringList();
    }

    const QDir playlistDir(QFileInfo(filePath).path());

    const QByteArray header = file.readLine();
    if (!header.startsWith(M3UHEADER)) {
        qWarning() << filePath << "is not a valid M3U playlist file";
        return QStringList();
    }

    QStringList files;
    while (!file.atEnd()) {
        const QByteArray line = file.readLine();
        if (line.isEmpty() || line.startsWith("#")) {
            continue;
        }

        const QString filepath = playlistDir.absoluteFilePath(QString::fromLatin1(line)).remove(QLatin1Char('\n'), Qt::CaseInsensitive);
        if (!filepath.isEmpty()) {
            files.append(filepath);
        }
    }

    file.close();

    return files;
}

void M3U::writeToFile(const QStringList &playlist, const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Failed to open file" << filePath << "for writing";
        return;
    }

    file.write(M3UHEADER "\n");
    Q_FOREACH (const QString &f, playlist) {
        file.write(f.toLatin1() + '\n');
    }

    file.close();
}
