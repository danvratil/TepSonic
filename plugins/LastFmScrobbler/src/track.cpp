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

#include "track.h"
#include "lastfm.h"
#include "scrobbler.h"
#include "cache.h"

#include <QDateTime>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>

using namespace LastFm;

Track::Track(Scrobbler *scrobbler, const QString &artist,
             const QString &trackTitle, const QString &album, int trackLength,
             const QString &genre, int trackNumber, uint playbackStart):
    QObject(scrobbler),
    m_scrobbler(scrobbler),
    m_artist(artist),
    m_trackTitle(trackTitle),
    m_album(album),
    m_trackLength(trackLength),
    m_genre(genre),
    m_trackNumber(trackNumber),
    m_playbackStart(playbackStart)
{
}

void Track::setArtist(const QString &artist)
{
    m_artist = artist;
}

QString Track::artist() const
{
    return m_artist;
}

void Track::setTrackTitle(const QString &trackTitle)
{
    m_trackTitle = trackTitle;
}

QString Track::trackTitle() const
{
    return m_trackTitle;
}

void Track::setAlbum(const QString &album)
{
    m_album = album;
}

QString Track::album() const
{
    return m_album;
}

void Track::setTrackLength(int trackLength)
{
    m_trackLength = trackLength;
}

int Track::trackLength() const
{
    return m_trackLength;
}

void Track::setGenre(const QString &genre)
{
    m_genre = genre;
}

QString Track::genre() const
{
    return m_genre;
}

void Track::setTrackNumber(int trackNumber)
{
    m_trackNumber = trackNumber;
}

int Track::trackNumber() const
{
    return m_trackNumber;
}

void Track::setPlaybackStart(uint playbackStart)
{
    m_playbackStart = playbackStart;
}

uint Track::playbackStart() const
{
    return m_playbackStart;
}

void Track::scrobble()
{
    m_playbackLength  += QDateTime::currentDateTime().toTime_t() - m_unpauseTime;

    // The track must be played for at least half of it's duration or at least 4 minutes, whatever
    // occurs first and must be longer then 30 seconds to be scrobbled
    if (((m_playbackLength < m_trackLength / 2) && (m_playbackLength < 240)) || (m_trackLength < 30)) {
        qDebug() << "Track" << m_trackTitle << "was not played long enough or is too short, not scrobbling";
        return;
    }

    // Unset the current track - when we are scrobbling, this track is no more current
    m_scrobbler->setCurrentTrack(0);

    // Add itself to the cache
    if (!m_scrobbler->cache()) {
        return;
    }

    m_scrobbler->cache()->add(this);
    m_scrobbler->cache()->submit();
}

void Track::nowPlaying()
{
    QUrl requestUrl(QLatin1String("http://ws.audioscrobbler.com/2.0/"));
    {
        QUrlQuery query;
        query.addQueryItem(QLatin1String("method"), QLatin1String("track.updateNowPlaying"));
        query.addQueryItem(QLatin1String("track"), m_trackTitle);
        requestUrl.setQuery(query);
    }
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QByteArray data;
    QUrlQuery query;
    query.addQueryItem(QLatin1String("album"), m_album);
    query.addQueryItem(QLatin1String("api_key"), Global::api_key);
    query.addQueryItem(QLatin1String("artist"), m_artist);
    query.addQueryItem(QLatin1String("duration"), QString::number(m_trackLength));
    query.addQueryItem(QLatin1String("method"), QLatin1String("track.updateNowPlaying"));
    query.addQueryItem(QLatin1String("sk"), Global::session_key);
    query.addQueryItem(QLatin1String("token"), Global::token);
    query.addQueryItem(QLatin1String("track"), m_trackTitle);
    query.addQueryItem(QLatin1String("trackNumber"), QString::number(m_trackNumber));
    query.addQueryItem(QLatin1String("api_sig"), Scrobbler::getRequestSignature(query));
    // Remove the trailing "?" from the params, since we are doing POST
    data.append(query.toString().toLatin1());

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    connect(nam, &QNetworkAccessManager::finished,
            this, static_cast<void(Track::*)(QNetworkReply*)>(&Track::scrobbled));
    connect(nam, &QNetworkAccessManager::finished,
            nam, &QNetworkAccessManager::deleteLater);

    // Send the date
    nam->post(request, data);

    // Set the track as current
    m_scrobbler->setCurrentTrack(this);

    // We started playing
    m_playbackLength = 0;
    m_unpauseTime = m_playbackStart;
}

void Track::love()
{
    QUrl requestUrl(QLatin1String("http://ws.audioscrobbler.com/2.0/"));
    {
        QUrlQuery query;
        query.addQueryItem(QLatin1String("method"), QLatin1String("track.love"));
        query.addQueryItem(QLatin1String("track"), m_trackTitle);
        requestUrl.setQuery(query);
    }
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QByteArray data;
    QUrlQuery query;
    query.addQueryItem(QLatin1String("api_key"), Global::api_key);
    query.addQueryItem(QLatin1String("artist"), m_artist);
    query.addQueryItem(QLatin1String("method"), QLatin1String("track.love"));
    query.addQueryItem(QLatin1String("sk"), Global::session_key);
    query.addQueryItem(QLatin1String("token"), Global::token);
    query.addQueryItem(QLatin1String("track"), m_trackTitle);
    query.addQueryItem(QLatin1String("api_sig"), Scrobbler::getRequestSignature(query));
    data.append(query.toString().toLatin1());

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    connect(nam, &QNetworkAccessManager::finished,
            this, static_cast<void (Track::*)(QNetworkReply*)>(&Track::scrobbled));
    connect(nam, &QNetworkAccessManager::finished,
            nam, &QNetworkAccessManager::deleteLater);
    // Send the date
    nam->post(request, data);
}

void Track::pause(bool pause)
{
    if (pause) {
        m_playbackLength += QDateTime::currentDateTime().toTime_t() - m_unpauseTime;
    } else {
        m_unpauseTime = QDateTime::currentDateTime().toTime_t();
    }
}

void Track::scrobbled(QNetworkReply *reply)
{
    QDomDocument document;
    document.setContent(reply->readAll());

    const QUrlQuery query(reply->request().url());
    const QString method = query.queryItemValue(QLatin1String("method"));

    const QDomElement lfm = document.documentElement();
    if (lfm.attribute(QLatin1String("status")) == QLatin1String("ok")) {
        qDebug() << method << "for" << query.queryItemValue(QLatin1String("track")) << "successfull";
        if (method == QLatin1String("track.love")) {
            Q_EMIT loved();
        } else {
            Q_EMIT scrobbled();
        }
    } else {
        qDebug() << method << "for" << query.queryItemValue(QLatin1String("track")) << "failed:";
        QDomElement err = lfm.childNodes().at(0).toElement();
        qDebug() << err.childNodes().at(0).nodeValue();

        m_scrobbler->raiseError(err.attribute(QLatin1String("code")).toInt());
    }
}
