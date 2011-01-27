/*
 * TEPSONIC
 * Copyright 2011 Dan Vratil <vratil@progdansoft.com>
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

#include "lastfm.h"
#include "constants.h"  /* /player/constants.h */

#include <QAbstractNetworkCache>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QDomDocument>
#include <QDomElement>

#include <QCryptographicHash>
#include <QFile>
#include <QTimer>
#include <QDir>
#include <QTextStream>
#include <QDateTime>

#include <QDebug>


namespace LastFm {

    namespace Global {

        QString api_key;
        QString session_key;
        QString secret_key;
        QString token;
        QString username;

    }
}

LastFm::Scrobbler::Scrobbler():
        m_cache(0),
        m_auth(0),
        m_ready(false),
        m_currentTrack(0)
{
    m_cache = new LastFm::Cache(this);
    m_auth = new LastFm::Auth(this);

    if (LastFm::Global::session_key.isEmpty()) {

        m_auth->getSession();
        connect(m_auth, SIGNAL(gotSession(QString,QString)),
                this, SLOT(setSession(QString,QString)));

    }
}


LastFm::Scrobbler::~Scrobbler()
{
    m_ready = false;

    delete m_cache;
    delete m_auth;
}


void LastFm::Scrobbler::setSession(QString key, QString username)
{
    LastFm::Global::session_key = key;
    LastFm::Global::username = username;
    m_ready = true;
}


void LastFm::Scrobbler::scrobble()
{
    if (m_currentTrack)
        m_currentTrack->scrobble();
    else
        m_cache->submit();
}


void LastFm::Scrobbler::scrobble(LastFm::Track *track)
{
    track->scrobble();
}



void LastFm::Scrobbler::nowPlaying(LastFm::Track *track)
{
    m_currentTrack = track;
    if (track)
        track->nowPlaying();
}



void LastFm::Scrobbler::raiseError(int code)
{
    QString errmsg;

    /* Only errors that are expected to occur are set to be translated; errors that
       indicate this scrobbler's internal failure should not never occur and therefor
       the messages don't need to be translated.
       Some errors do not inform user about an error, but just try to submit the track
       again after 120 seconds.
    */
    switch (code)
    {
    case -1:
        errmsg = tr("Invalid server response");
        break;
    case 2:
        errmsg = "Invalid service - This service does not exist";
        break;
    case 3:
        errmsg = "Invalid Method - No method with that name in this package";
        break;
    case 4:
        //errmsg = "Authentication Failed - You do not have permissions to access the service";
        errmsg = tr("Authentication Failed - Please try to obtain a new token in TepSonic -> Settings -> Plugins -> Last.Fm");
        break;
    case 5:
        errmsg = "Invalid format - This service doesn't exist in that format";
        break;
    case 6:
        errmsg = "Invalid parameters - Your request is missing a required parameter";
        break;
    case 7:
        errmsg = "Invalid resource specified";
        break;
    case 9:
        // Don't tell anything about this, try to re-authenticate
        //errmsg = "Invalid session key - Please re-authenticate";
        m_auth->getSession();
        return;
        break;
    case 10:
        errmsg = "Invalid API key - You must be granted a valid key by last.fm";
        break;
    case 11: // Service Offline - This service is temporarily offline. Try again later.
    case 16: // There was a temporary error processing your request. Please try again
        QTimer::singleShot(1200000, m_cache, SLOT(submit()));
        return;
        break;
    case 13:
        errmsg = "Invalid method signature supplied";
        break;
    case 26:
        errmsg = "Suspended API key - Access for your account has been suspended, please contact Last.fm";
        break;
    }

    emit error(errmsg, code);

}


QString LastFm::Scrobbler::getRequestSignature(QUrl request)
{
    // sort the list of query items alphabetically
    QList<QPair<QString, QString> > list = request.queryItems();
    qSort(list);

    QUrl url;
    url.setQueryItems(list);
    // convert name1=value1&name2=value2... to name1value1name2value2
    QString str = url.toString();
    str.remove(url.queryPairDelimiter());
    str.remove(url.queryValueDelimiter());
    str.remove(0,1); // remove trailing "?"
    str.append(LastFm::Global::secret_key);
    QByteArray ba;
    ba.append(str);

    return QString(QCryptographicHash::hash(ba, QCryptographicHash::Md5).toHex());

}



LastFm::Auth::Auth(LastFm::Scrobbler *scrobbler):
        m_scrobbler(scrobbler)
{
}


/** Documentation: http://www.last.fm/api/show?service=265 */
void LastFm::Auth::getToken()
{
    QNetworkRequest request;
    QUrl url;

    url.setUrl("http://ws.audioscrobbler.com/2.0/");
    url.addQueryItem("api_key", LastFm::Global::api_key);
    url.addQueryItem("method", "auth.getToken");
    url.addQueryItem("api_sig", LastFm::Scrobbler::getRequestSignature(url));
    request.setUrl(url.toString());

    QNetworkAccessManager *nam = new QNetworkAccessManager();
    connect(nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotGotToken(QNetworkReply*)));
    nam->get(request);

    qDebug() << "Requesting new token...";
}


void LastFm::Auth::slotGotToken(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 200)
    {
        qDebug() << "Token request failed, http error" << status;
        m_scrobbler->raiseError(-1);
        return;
    }


    // Read the XML document
    QDomDocument document;
    document.setContent(reply->readAll());

    // <lfm status="ok/failed">
    QDomElement lfm = document.documentElement();
    if (lfm.attribute("status", "") == "ok") {

        QDomElement token = lfm.childNodes().at(0).toElement();
        if (token.tagName() == "token")
            emit gotToken(token.childNodes().at(0).nodeValue());

        qDebug() << "Successfully recieved new token";

    } else {

        // <error code="errCode">
        QDomElement error = lfm.childNodes().at(0).toElement();
        if (error.tagName() == "error")
            m_scrobbler->raiseError(error.attribute("code", 0).toInt());

        qDebug() << "Failed to obtain new token: " << error.nodeValue();
    }
}


void LastFm::Auth::getSession()
{
    QNetworkRequest request;
    QUrl url;

    url.setUrl("http://ws.audioscrobbler.com/2.0/");
    url.addQueryItem("api_key", LastFm::Global::api_key);
    url.addQueryItem("method", "auth.getSession");
    url.addQueryItem("token", LastFm::Global::token);
    url.addQueryItem("api_sig", LastFm::Scrobbler::getRequestSignature(url));
    request.setUrl(url.toString());

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    connect(nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotGotSession(QNetworkReply*)));
    nam->get(request);

    qDebug() << "Requesting new session key...";
}


void LastFm::Auth::slotGotSession(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 200)
    {
        qDebug() << "Session key request failed, http error" << status;
        qDebug() << reply->readAll();
        m_scrobbler->raiseError(-1);
        return;
    }

    // Read the XML document
    QDomDocument document;
    document.setContent(reply->readAll());


    // <lfm status="ok/failed">
    QDomElement lfm = document.documentElement();
    if (lfm.attribute("status","") == "ok") {

        QDomElement key = lfm.elementsByTagName("key").at(0).toElement();
        QDomElement name = lfm.elementsByTagName("name").at(0).toElement();
        emit gotSession(key.childNodes().at(0).nodeValue(),
                        name.childNodes().at(0).nodeValue());
        LastFm::Global::session_key = key.childNodes().at(0).nodeValue();

        qDebug() << "Successfully recieved new session key";

    } else {

        // <error code="errCode">
        QDomElement error = lfm.elementsByTagName("error").at(0).toElement();
        m_scrobbler->raiseError(error.attribute("code").toInt());

        qDebug() << "Failed to obtain new session key:" << error.nodeValue();

    }
}



LastFm::Track::Track(LastFm::Scrobbler *scrobbler, QString artist, QString trackTitle,
                     QString album, int trackLength, QString genre, int trackNumber,
                     uint playbackStart):
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


void LastFm::Track::scrobble()
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
    if (!m_scrobbler->cache())
        return;

    m_scrobbler->cache()->add(this);
    m_scrobbler->cache()->submit();
}


void LastFm::Track::nowPlaying()
{
    QNetworkRequest request;

    QUrl requestUrl("http://ws.audioscrobbler.com/2.0/");
    requestUrl.addQueryItem("method", "track.updateNowPlaying");
    requestUrl.addQueryItem("track", m_trackTitle);
    request.setUrl(requestUrl);

    QByteArray data;
    QUrl params;
    params.addQueryItem("album", m_album);
    params.addQueryItem("api_key", LastFm::Global::api_key);
    params.addQueryItem("artist", m_artist);
    params.addQueryItem("duration", QString::number(m_trackLength));
    params.addQueryItem("method", "track.updateNowPlaying");
    params.addQueryItem("sk", LastFm::Global::session_key);
    params.addQueryItem("token", LastFm::Global::token);
    params.addQueryItem("track", m_trackTitle);
    params.addQueryItem("trackNumber", QString::number(m_trackNumber));
    params.addQueryItem("api_sig", LastFm::Scrobbler::getRequestSignature(params));
    // Remove the trailing "?" from the params, since we are doing POST
    data.append(params.toString().remove(0,1));

    QNetworkAccessManager *nam = new QNetworkAccessManager();
    connect(nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(scrobbled(QNetworkReply*)));
    // Send the date
    nam->post(request, data);

    // Set the track as current
    m_scrobbler->setCurrentTrack(this);

    // We started playing
    m_playbackLength = 0;
    m_unpauseTime = m_playbackStart;
}


void LastFm::Track::love()
{
    QNetworkRequest request;
    QUrl requestUrl("http://ws.audioscrobbler.com/2.0/");
    requestUrl.addQueryItem("method", "track.love");
    requestUrl.addQueryItem("track", m_trackTitle);
    request.setUrl(requestUrl);

    QByteArray data;
    QUrl params;
    params.addQueryItem("api_key", LastFm::Global::api_key);
    params.addQueryItem("artist", m_artist);
    params.addQueryItem("method", "track.love");
    params.addQueryItem("sk", LastFm::Global::session_key);
    params.addQueryItem("token", LastFm::Global::token);
    params.addQueryItem("track", m_trackTitle);
    params.addQueryItem("api_sig", LastFm::Scrobbler::getRequestSignature(params));
    data.append(params.toString().remove(0,1));

    QNetworkAccessManager *nam = new QNetworkAccessManager();
    connect(nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(scrobbled(QNetworkReply*)));
    connect(nam, SIGNAL(finished(QNetworkReply*)),
            nam, SLOT(deleteLater()));
    // Send the date
    nam->post(request, data);

}


void LastFm::Track::pause(bool pause)
{
    if (pause) {
        m_playbackLength += QDateTime::currentDateTime().toTime_t() - m_unpauseTime;
    } else {
        m_unpauseTime = QDateTime::currentDateTime().toTime_t();
    }
}


void LastFm::Track::scrobbled(QNetworkReply *reply)
{
    QDomDocument document;
    document.setContent(reply->readAll());


    QString method = reply->request().url().queryItemValue("method");

    QDomElement lfm = document.documentElement();
    if (lfm.attribute("status", "") == "ok") {
        qDebug() << method << "for" << reply->request().url().queryItemValue("track") << "successfull";
        if (method == "track.love")
            emit loved();
        else
            emit scrobbled();

    } else {
        qDebug() << method << "for" << reply->request().url().queryItemValue("track") << "failed:";
        QDomElement err = lfm.childNodes().at(0).toElement();
        qDebug() << err.childNodes().at(0).nodeValue();

        if (err.attribute("code", 0).toInt() == 9)
            qDebug() << "TODO: Request a new session key and send the request again.";
    }
}




LastFm::Cache::Cache(LastFm::Scrobbler *scrobbler):
        m_scrobbler(scrobbler),
        m_resubmitTimer(0)
{
    loadCache();
}


LastFm::Cache::~Cache()
{
    // Submit entire cache
    submit();

    // Save what's left
    QFile file(QString(_CONFIGDIR) + QDir::separator() + "lastfmcache.xml");
    if (!file.open(QIODevice::WriteOnly))
        return;

    QDomDocument document;

    QDomElement rootElement = document.createElement("tracks");
    document.appendChild(rootElement);

    for (int i = 0; i < m_cache.size(); i++)
    {
        QDomElement trackElement = document.createElement("track");

        LastFm::Track *track = m_cache.at(i);
        trackElement.setAttribute("track", track->trackTitle());
        trackElement.setAttribute("artist", track->artist());
        trackElement.setAttribute("album", track->album());
        trackElement.setAttribute("genre", track->genre());
        trackElement.setAttribute("length", track->trackLength());
        trackElement.setAttribute("trackNumber", track->trackNumber());
        trackElement.setAttribute("playbackStart", track->playbackStart());

        rootElement.appendChild(trackElement);
    }

    // Save the stream
    QTextStream ts(&file);
    ts << document.toString();

    // Delete all tracks in cache
    qDeleteAll(m_cache);
}


void LastFm::Cache::add(LastFm::Track *track)
{
    qDebug() << "Adding track" << track->trackTitle() << "to cache";
    if (track)
        m_cache.append(track);

}


void LastFm::Cache::submit()
{
    QNetworkRequest request;
    QUrl requestUrl("http://ws.audioscrobbler.com/2.0/");
    requestUrl.addQueryItem("method", "track.scrobble");
    request.setUrl(requestUrl);
    QByteArray data;
    QUrl params;


    qDebug() << "Scrobbling " << m_cache.size() << " tracks";

    params.addQueryItem("api_key", LastFm::Global::api_key);
    params.addQueryItem("method", "track.scrobble");
    params.addQueryItem("sk", LastFm::Global::session_key);
    params.addQueryItem("token", LastFm::Global::token);
    for (int i = 0; i < m_cache.size(); i++)
    {
        if (m_cache.at(i)) {
            params.addQueryItem("album["+QString::number(i)+"]", m_cache.at(i)->album());
            params.addQueryItem("artist["+QString::number(i)+"]", m_cache.at(i)->artist());
            params.addQueryItem("duration["+QString::number(i)+"]", QString::number(m_cache.at(i)->trackLength()));
            params.addQueryItem("timestamp["+QString::number(i)+"]", QString::number(m_cache.at(i)->playbackStart()));
            params.addQueryItem("track["+QString::number(i)+"]", m_cache.at(i)->trackTitle());
            params.addQueryItem("trackNumber["+QString::number(i)+"]", QString::number(m_cache.at(i)->trackNumber()));
        }
    }
    params.addQueryItem("api_sig", LastFm::Scrobbler::getRequestSignature(params));


    data.append(params.toString().remove(0,1));

    QNetworkAccessManager *nam = new QNetworkAccessManager();
    // When request is done, destroy the QNetworkAccessManager
    connect(nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(submitted(QNetworkReply*)));

    // Send the date
    nam->post(request, data);

}


void LastFm::Cache::submitted(QNetworkReply *reply)
{
    QDomDocument document;
    document.setContent(reply->readAll());

    QString method = reply->request().url().queryItemValue("method");

    QDomElement lfm = document.documentElement();
    if (lfm.attribute("status", "") == "ok") {
        qDebug() << method << "for tracks in cache successfull, purging cache";
        qDeleteAll(m_cache);

        // If we were able to submit track BEFORE the timer or this is timer's event, then
        // destroy the timer so that it won't tick anymore
        if (m_resubmitTimer)
            delete m_resubmitTimer;

    } else {
        qDebug() << method << "for  cache failed:";
        QDomElement err = lfm.childNodes().at(0).toElement();
        qDebug() << err.childNodes().at(0).nodeValue();

        m_scrobbler->raiseError(err.attribute("code", 0).toInt());

        m_resubmitTimer = new QTimer();
        // let's try again in 2 minutes
        m_resubmitTimer->setInterval(120000);
        connect(m_resubmitTimer, SIGNAL(timeout()),
                this, SLOT(submit()));
    }
}


void LastFm::Cache::loadCache()
{
    m_cache.clear();

    QFile file(QString(_CONFIGDIR) + QDir::separator() + "lastfmcache.xml");
    if (!file.open(QIODevice::ReadOnly))
        return;

    QDomDocument document;
    document.setContent(&file);

    QDomElement rootElement = document.documentElement();
    LastFm::Track *track;

    // Is the root element <tracks> ?
    if (rootElement.tagName() != "tracks")
        return;

    QDomNode trackNode = rootElement.firstChild();
    while ( !trackNode.isNull() )
    {
        QDomElement trackEl = trackNode.toElement();

        // Is the element valid?
        if (trackEl.isNull()) {
            trackNode = trackNode.nextSibling();
            continue;
        }

        // Is it a <track> element?
        if (trackEl.tagName() != "track") {
            trackNode = trackNode.nextSibling();
            continue;
        }

        track = new LastFm::Track(m_scrobbler,
                                  trackEl.attribute("artist", ""),
                                  trackEl.attribute("track", ""),
                                  trackEl.attribute("album", ""),
                                  trackEl.attribute("length", 0).toInt(),
                                  trackEl.attribute("genre", ""),
                                  trackEl.attribute("trackNumber", 0).toInt(),
                                  trackEl.attribute("playbackStart", 0).toInt());
        m_cache.append(track);

        trackNode = trackNode.nextSibling();
    }
}
