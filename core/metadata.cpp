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

#include "metadata.h"
#include "utils.h"

#include <QSharedData>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QDebug>

#include <taglib/fileref.h>

using namespace TepSonic;

class MetaData::Private : public QSharedData
{
  public:
    Private();
    Private(const QSqlQuery &query);
    Private(const TagLib::FileRef &file);
    Private(const Private &other);

    QString filename;  /*! < Name of file */
    QString artist;    /*! < Track artist */
    QString title;     /*! < Track title */
    QString album;     /*! < Album name */
    QString genre;     /*! < Genre string */
    qint64 length;     /*! < Length of song in milliseconds */
    QString formattedLength; /* ! < Formatted time (mm:ss) */
    uint trackNumber;   /*! < Track number */
    uint year;         /*! < Year of release */
    int bitrate;      /*! < bitrate */
};

MetaData::Private::Private():
    QSharedData(),
    length(0),
    trackNumber(0),
    year(0),
    bitrate(0)
{
}

MetaData::Private::Private(const MetaData::Private &other):
    QSharedData(other),
    filename(other.filename),
    artist(other.artist),
    title(other.title),
    album(other.album),
    genre(other.genre),
    length(other.length),
    formattedLength(other.formattedLength),
    trackNumber(other.trackNumber),
    year(other.year),
    bitrate(other.bitrate)
{
}

MetaData::Private::Private(const QSqlQuery &query):
    QSharedData(),
    length(0),
    trackNumber(0),
    year(0),
    bitrate(0)
{
    if (!query.isValid()) {
        qWarning() << "Trying to construct MetaData from invalid QSqlQuery";
        return;
    }

    const QSqlRecord record = query.record();
    filename = record.value(QLatin1Literal("filename")).toString();
    artist = record.value(QLatin1Literal("artist")).toString();
    title = record.value(QLatin1Literal("trackname")).toString();
    album = record.value(QLatin1Literal("album")).toString();
    genre = record.value(QLatin1Literal("genre")).toString();
    length = record.value(QLatin1Literal("length")).toLongLong();
    formattedLength = Utils::formatMilliseconds(length);
    trackNumber = record.value(QLatin1Literal("track")).toInt();
    year = record.value(QLatin1Literal("year")).toUInt();
    bitrate = record.value(QLatin1Literal("bitrate")).toInt();
}

MetaData::Private::Private(const TagLib::FileRef &file):
    QSharedData(),
    length(0),
    trackNumber(0),
    year(0),
    bitrate(0)
{
    if (file.isNull()) {
        qWarning() << "Trying to construct MetaData from invalid FileRef";
        return;
    }
    filename = QString::fromUtf8(file.file()->name());
    artist = QString::fromLatin1(file.tag()->artist().toCString(true));
    title = QString::fromLatin1(file.tag()->title().toCString(true));
    album = QString::fromLatin1(file.tag()->album().toCString(true));
    genre = QString::fromLatin1(file.tag()->genre().toCString(true));
    length = file.audioProperties()->length() * 1000;
    formattedLength = Utils::formatMilliseconds(length);
    trackNumber = file.tag()->track();
    year = file.tag()->year();
    bitrate = file.audioProperties()->bitrate();
}

MetaData::MetaData():
    d(new Private)
{
}

MetaData::MetaData(const QSqlQuery &query):
    d(new Private(query))
{
}

MetaData::MetaData(const TagLib::FileRef &file):
    d(new Private(file))
{
}

MetaData::MetaData(const MetaData &other):
    d(other.d)
{
}

MetaData& MetaData::operator=(const MetaData &other)
{
    if (d != other.d) {
        d = other.d;
    }

    return *this;
}

MetaData& MetaData::operator=(MetaData &&other)
{
    qSwap(d, other.d);
    return *this;
}

MetaData::~MetaData()
{
}

bool MetaData::isNull() const
{
    return d->filename.isEmpty();
}

QString MetaData::fileName() const
{
    return d->filename;
}

void MetaData::setFileName(const QString &fileName)
{
    d->filename = fileName;
}


QString MetaData::artist() const
{
    return d->artist;
}

void MetaData::setArtist(const QString &artist)
{
    d->artist = artist;
}

QString MetaData::title() const
{
    return d->title;
}

void MetaData::setTitle(const QString &title)
{
    d->title = title;
}

QString MetaData::album() const
{
    return d->album;
}

void MetaData::setAlbum(const QString &album)
{
    d->album = album;
}

QString MetaData::genre() const
{
    return d->genre;
}

void MetaData::setGenre(const QString &genre)
{
    d->genre = genre;
}

qint64 MetaData::length() const
{
    return d->length;
}

void MetaData::setLength(qint64 length)
{
    d->length = length;
    d->formattedLength = Utils::formatMilliseconds(length);
}

QString MetaData::formattedLength() const
{
    return d->formattedLength;
}

uint MetaData::trackNumber() const
{
    return d->trackNumber;
}

void MetaData::setTrackNumber(uint trackNumber)
{
    d->trackNumber = trackNumber;
}

uint MetaData::year() const
{
    return d->year;
}

void MetaData::setYear(uint year)
{
    d->year = year;
}

int MetaData::bitrate() const
{
    return d->bitrate;
}

void MetaData::setBitrate(int bitrate)
{
    d->bitrate = bitrate;
}


