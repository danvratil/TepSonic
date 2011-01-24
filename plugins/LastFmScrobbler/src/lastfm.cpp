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

LastFm::Scrobbler::Scrobbler():
        m_cache(0),
        m_auth(0),
        m_ready(false)
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
    LastFm::Global::secret_key = key;
    LastFm::Global::username = username;
    m_ready = true;
}


void LastFm::Scrobbler::scrobble()
{
    m_cache->submit();
}


void LastFm::Scrobbler::raiseError(int code)
{
    QString errmsg;

    /* Only errors that are expected to occur are set to be translated; errors that
       indicate this scrobbler's internal failure should not never occur and therefor
       the messages don't need to be translated.
       Some errors do not inform user about an error, but just try to submit the track
       again after 60 seconds.
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
        QTimer::singleShot(60000, m_cache, SLOT(submit()));
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
    str.append(LastFm::Global::secret_key);
    QByteArray ba;
    ba.append(str);

    return QString(QCryptographicHash::hash(ba, QCryptographicHash::Md5));

}



LastFm::Auth::Auth(LastFm::Scrobbler *scrobbler):
        m_scrobbler(scrobbler)
{
    m_nam = new QNetworkAccessManager();
}


LastFm::Auth::~Auth()
{
    delete m_nam;
}


/** Documentation: http://www.last.fm/api/show?service=265 */
void LastFm::Auth::getToken()
{
    QNetworkRequest request;
    request.setUrl("http://ws.audioscrobbler.com/2.0/?method=auth.gettoken&api_key="+LastFm::Global::api_key);
    connect(m_nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotGotToken(QNetworkReply*)));
    m_nam->get(request);
}


void LastFm::Auth::slotGotToken(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 200)
    {
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

    } else {

        // <error code="errCode">
        QDomElement error = lfm.childNodes().at(0).toElement();
        if (error.tagName() == "error")
            m_scrobbler->raiseError(error.attribute("code", 0).toInt());
        return;

    }
}


void LastFm::Auth::getSession()
{
    QNetworkRequest request;
    request.setUrl("http://ws.audioscrobbler.com/2.0/?method=auth.getsession&api_key="+LastFm::Global::api_key+"&token="+LastFm::Global::token);
    connect(m_nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotGotSession(QNetworkReply*)));
    m_nam->get(request);
}


void LastFm::Auth::slotGotSession(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 200)
    {
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

    } else {

        // <error code="errCode">
        QDomElement error = lfm.elementsByTagName("error").at(0).toElement();
        m_scrobbler->raiseError(error.attribute("code").toInt());
        return;

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
    // Let the scrobbler do what's neccessary
    m_scrobbler->scrobble();
}


void LastFm::Track::nowPlaying()
{
    QNetworkRequest request;
    request.setUrl(QUrl("http://ws.audioscrobbler.com/2.0/"));

    QByteArray data;
    QUrl params;
    params.addQueryItem("album", m_album);
    params.addQueryItem("api_key", LastFm::Global::api_key);
    params.addQueryItem("artist", m_artist);
    params.addQueryItem("duration", QString::number(m_trackLength));
    params.addQueryItem("method", "updateNowPlaying");
    params.addQueryItem("sk", LastFm::Global::session_key);
    params.addQueryItem("token", LastFm::Global::token);
    params.addQueryItem("track", m_trackTitle);
    params.addQueryItem("trackNumber", QString::number(m_trackNumber));
    params.addQueryItem("api_sig", LastFm::Scrobbler::getRequestSignature(params));
    data.append(params.toString());

    QNetworkAccessManager *nam = new QNetworkAccessManager();
    // When request is done, destroy the QNetworkAccessManager
    connect(nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(deleteLater()));

    // Send the date
    nam->post(request, data);
}


void LastFm::Track::love()
{
    QNetworkRequest request;
    request.setUrl(QUrl("http://ws.audioscrobbler.com/2.0/"));

    QByteArray data;
    QUrl params;
    params.addQueryItem("api_key", LastFm::Global::api_key);
    params.addQueryItem("artist", m_artist);
    params.addQueryItem("method", "track.love");
    params.addQueryItem("sk", LastFm::Global::session_key);
    params.addQueryItem("token", LastFm::Global::token);
    params.addQueryItem("track", m_trackTitle);
    params.addQueryItem("api_sig", LastFm::Scrobbler::getRequestSignature(params));
    data.append(params.toString());

    QNetworkAccessManager *nam = new QNetworkAccessManager();
    // When request is done, destroy the QNetworkAccessManager
    connect(nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(deleteLater()));

    // Send the date
    nam->post(request, data);
}




LastFm::Cache::Cache(LastFm::Scrobbler *scrobbler):
        m_scrobbler(scrobbler)
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


void LastFm::Cache::add(Track *track)
{

    m_cache.append(track);

}


void LastFm::Cache::submit()
{
    QNetworkRequest request;
    request.setUrl(QUrl("http://ws.audioscrobbler.com/2.0/"));

    QByteArray data;
    QUrl params;


    params.addQueryItem("api_key", LastFm::Global::api_key);
    params.addQueryItem("method", "updateNowPlaying");
    params.addQueryItem("sk", LastFm::Global::session_key);
    params.addQueryItem("token", LastFm::Global::token);
    for (int i = 0; i < m_cache.size(); i++)
    {
        params.addQueryItem("album["+QString::number(i)+"]", m_cache.at(i)->album());
        params.addQueryItem("artist["+QString::number(i)+"]", m_cache.at(i)->artist());
        params.addQueryItem("duration["+QString::number(i)+"]", QString::number(m_cache.at(i)->trackLength()));
        params.addQueryItem("timestamp["+QString::number(i)+"]", QString::number(m_cache.at(i)->playbackStart()));
        params.addQueryItem("track["+QString::number(i)+"]", m_cache.at(i)->trackTitle());
        params.addQueryItem("trackNumber["+QString::number(i)+"]", QString::number(m_cache.at(i)->trackNumber()));
    }
    params.addQueryItem("api_sig", LastFm::Scrobbler::getRequestSignature(params));


    data.append(params.toString());

    QNetworkAccessManager *nam = new QNetworkAccessManager();
    // When request is done, destroy the QNetworkAccessManager
    connect(nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(deleteLater()));

    // Send the date
    nam->post(request, data);

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
