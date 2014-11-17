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

#ifndef TEPSONIC_METADATA_H
#define TEPSONIC_METADATA_H

#include <QString>
#include <QSharedDataPointer>
#include <QList>

#include <taglib/fileref.h>

#include "tepsonic-core-export.h"

class QSqlQuery;

namespace TepSonic
{

class TEPSONIC_CORE_EXPORT MetaData
{
  public:
    typedef QList<MetaData> List;

    MetaData();
    MetaData(const QSqlQuery &query);
    MetaData(const TagLib::FileRef &file);
    MetaData(const MetaData &other);
    ~MetaData();
    MetaData& operator=(const MetaData &other);
    MetaData& operator=(MetaData &&other);

    bool isNull() const;

    QString fileName() const;
    void setFileName(const QString &fileName);
    QString artist() const;
    void setArtist(const QString &artist);
    QString title() const;
    void setTitle(const QString &title);
    QString album() const;
    void setAlbum(const QString &album);
    QString genre() const;
    void setGenre(const QString &genre);
    /**
     * Returns length of the track in milliseconds
     */
    qint64 length() const;
    void setLength(qint64 msecs);
    QString formattedLength() const;
    uint trackNumber() const;
    void setTrackNumber(uint trackNumber);
    uint year() const;
    void setYear(uint year);
    int bitrate() const;
    void setBitrate(int bitrate);

  private:
    class Private;
    QSharedDataPointer<Private> d;
};

} // namespace TepSonic

#endif // TEPSONIC_METADATA_H
