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

#include "cache.h"
#include "scrobbler.h"
#include "track.h"
#include "lastfm.h"
#include <core/settings.h>

#include <QDomDocument>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QDir>

using namespace LastFm;
using namespace TepSonic;

Cache::Cache(Scrobbler *scrobbler):
    QObject(scrobbler),
    m_scrobbler(scrobbler),
    m_resubmitTimer(0)
{
    loadCache();
}

Cache::~Cache()
{
    // Save the cache
    const QString cacheFile = Settings::dataDir() + QLatin1String("/lastfmcache.xml");
    QFile file(cacheFile);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    QDomDocument document;

    QDomElement rootElement = document.createElement(QLatin1String("tracks"));
    document.appendChild(rootElement);

    Q_FOREACH (Track *track, m_cache) {
        QDomElement trackElement = document.createElement(QLatin1String("track"));
        const MetaData metaData = track->metaData();
        trackElement.setAttribute(QLatin1String("track"), metaData.title());
        trackElement.setAttribute(QLatin1String("artist"), metaData.artist());
        trackElement.setAttribute(QLatin1String("album"), metaData.album());
        trackElement.setAttribute(QLatin1String("genre"), metaData.genre());
        trackElement.setAttribute(QLatin1String("length"), metaData.length());
        trackElement.setAttribute(QLatin1String("trackNumber"), metaData.trackNumber());
        trackElement.setAttribute(QLatin1String("playbackStart"), track->playbackStart());

        rootElement.appendChild(trackElement);
    }

    // Save the stream
    QTextStream ts(&file);
    ts << document.toString();

    // Delete all tracks in cache
    qDeleteAll(m_cache);

    if (m_resubmitTimer) {
        delete m_resubmitTimer;
    }
}

void Cache::add(Track *track)
{
    qDebug() << "Adding track" << track->metaData().title() << "to cache";
    m_cache.append(track);
}

void Cache::submit()
{
    QUrl requestUrl(QLatin1String("http://ws.audioscrobbler.com/2.0/"));
    {
        QUrlQuery query;
        query.addQueryItem(QLatin1String("method"), QLatin1String("track.scrobble"));
        requestUrl.setQuery(query);
    }
    QNetworkRequest request(requestUrl);
    QByteArray data;
    QUrlQuery params;

    const int items_count = qMin(m_cache.size() - 1, 40);

    qDebug() << "Scrobbling " << items_count << " tracks";

    params.addQueryItem(QLatin1String("api_key"), Global::api_key);
    params.addQueryItem(QLatin1String("method"), QLatin1String("track.scrobble"));
    params.addQueryItem(QLatin1String("sk"), Global::session_key);
    params.addQueryItem(QLatin1String("token"), Global::token);
    for (int i = 0; i <= items_count; ++i) {
        Track *track = m_cache.at(i);
        if (track) {
            const MetaData metaData = track->metaData();
            params.addQueryItem(QString::fromLatin1("album[%1]").arg(i), metaData.album());
            params.addQueryItem(QString::fromLatin1("artist[%1]").arg(i), metaData.artist());
            params.addQueryItem(QString::fromLatin1("duration[%1]").arg(i), QString::number(metaData.length() / 1000));
            params.addQueryItem(QString::fromLatin1("timestamp[%1]").arg(i), QString::number(track->playbackStart()));
            params.addQueryItem(QString::fromLatin1("track[%1]").arg(i), metaData.title());
            params.addQueryItem(QString::fromLatin1("trackNumber[%1]").arg(i), QString::number(metaData.trackNumber()));
        }
    }
    params.addQueryItem(QLatin1String("api_sig"), Scrobbler::getRequestSignature(params));
    request.setAttribute(QNetworkRequest::User, QVariant(items_count));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    data.append(params.toString().toLatin1());

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    // When request is done, destroy the QNetworkAccessManager
    connect(nam, &QNetworkAccessManager::finished,
            this, &Cache::submitted);
    connect(nam, &QNetworkAccessManager::finished,
            nam, &QNetworkAccessManager::deleteLater);

    // Send the date
    nam->post(request, data);
}

void Cache::submitted(QNetworkReply *reply)
{
    QDomDocument document;
    document.setContent(reply->readAll());

    const QUrlQuery query(reply->request().url());
    const QString method = query.queryItemValue(QLatin1String("method"));
    const QDomElement lfm = document.documentElement();
    if (lfm.attribute(QLatin1String("status")) == QLatin1String("ok")) {
        qDebug() << method << "for tracks in cache successfull, removing them from cache";
        const int items_count = reply->request().attribute(QNetworkRequest::User).toInt();
        for (int i = 0; i < items_count; i++) {
            delete m_cache.at(i);
            m_cache.removeAt(i);
        }
        qDebug() << "There are now" << m_cache.size() << "items in cache";

        // If we were able to submit track BEFORE the timer or this is timer's event, then
        // destroy the timer so that it won't tick anymore
        if (m_resubmitTimer) {
            delete m_resubmitTimer;
            m_resubmitTimer = 0;
        }

    } else {
        qDebug() << method << "for  cache failed:";
        QDomElement err = lfm.childNodes().at(0).toElement();
        qDebug() << err.childNodes().at(0).nodeValue();

        m_scrobbler->raiseError(err.attribute(QLatin1String("code")).toInt());

        m_resubmitTimer = new QTimer();
        // let's try again in 2 minutes
        m_resubmitTimer->setInterval(120000);
        connect(m_resubmitTimer, SIGNAL(timeout()),
                this, SLOT(submit()));
        connect(m_resubmitTimer, SIGNAL(timeout()),
                m_resubmitTimer, SLOT(deleteLater()));
    }
}

void Cache::loadCache()
{
    qDeleteAll(m_cache);
    m_cache.clear();

    const QString configFile = Settings::dataDir() + QLatin1String("/lastfmcache.xml");
    QFile file(configFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QDomDocument document;
    document.setContent(&file);

    const QDomElement rootElement = document.documentElement();

    // Is the root element <tracks> ?
    if (rootElement.tagName() != QLatin1String("tracks")) {
        return;
    }

    QDomNode trackNode = rootElement.firstChild();
    while ( !trackNode.isNull() )
    {
        const QDomElement trackEl = trackNode.toElement();

        // Is the element valid?
        if (trackEl.isNull()) {
            trackNode = trackNode.nextSibling();
            continue;
        }

        // Is it a <track> element?
        if (trackEl.tagName() != QLatin1String("track")) {
            trackNode = trackNode.nextSibling();
            continue;
        }

        MetaData metaData;
        metaData.setArtist(trackEl.attribute(QLatin1String("artist")));
        metaData.setTitle(trackEl.attribute(QLatin1String("track")));
        metaData.setAlbum(trackEl.attribute(QLatin1String("album")));
        metaData.setLength(trackEl.attribute(QLatin1String("length")).toInt() * 1000);
        metaData.setGenre(trackEl.attribute(QLatin1String("genre")));
        metaData.setTrackNumber(trackEl.attribute(QLatin1String("trackNumber")).toInt());
        const uint playbackStart = trackEl.attribute(QLatin1String("playbackStart")).toInt();
        if (metaData.artist().isEmpty() || metaData.title().isEmpty()
            || metaData.album().isEmpty() || metaData.length() == 0 || playbackStart == 0)
        {
            trackNode = trackNode.nextSibling();
            continue;
        }

        m_cache.append(new Track(m_scrobbler, metaData, playbackStart));
        trackNode = trackNode.nextSibling();
    }
}
