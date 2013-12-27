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

#include "auth.h"
#include "lastfm.h"
#include "scrobbler.h"

#include <QUrlQuery>
#include <QDomDocument>
#include <QDebug>

using namespace LastFm;

Auth::Auth(Scrobbler *scrobbler):
    QObject(scrobbler),
    m_scrobbler(scrobbler)
{
}

/** Documentation: http://www.last.fm/api/show?service=265 */
void Auth::getToken()
{
    QUrlQuery query;

    QUrl url(QLatin1String("http://ws.audioscrobbler.com/2.0/"));
    query.addQueryItem(QLatin1String("api_key"), Global::api_key);
    query.addQueryItem(QLatin1String("method"), QLatin1String("auth.getToken"));
    query.addQueryItem(QLatin1String("api_sig"), Scrobbler::getRequestSignature(query));
    url.setQuery(query);
    const QNetworkRequest request(url);

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    connect(nam, &QNetworkAccessManager::finished,
            this, &Auth::slotGotToken);
    connect(nam ,&QNetworkAccessManager::finished,
            nam, &QNetworkAccessManager::deleteLater);
    nam->get(request);

    qDebug() << "Requesting new token...";
}

void Auth::slotGotToken(QNetworkReply *reply)
{
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 200) {
        qDebug() << "Token request failed, http error" << status;
        m_scrobbler->raiseError(-1);
        return;
    }

    // Read the XML document
    QDomDocument document;
    document.setContent(reply->readAll());

    // <lfm status="ok/failed">
    const QDomElement lfm = document.documentElement();
    if (lfm.attribute(QLatin1String("status")) == QLatin1String("ok")) {
        const QDomElement token = lfm.childNodes().at(0).toElement();
        if (token.tagName() == QLatin1String("token")) {
            Q_EMIT gotToken(token.childNodes().at(0).nodeValue());
        }
        qDebug() << "Successfully recieved new token";
    } else {
        // <error code="errCode">
        const QDomElement error = lfm.childNodes().at(0).toElement();
        if (error.tagName() == QLatin1String("error")) {
            m_scrobbler->raiseError(error.attribute(QLatin1String("code")).toInt());
        }

        qDebug() << "Failed to obtain new token: " << error.nodeValue();
    }
}

void Auth::getSession()
{
    QUrlQuery query;

    QUrl url(QLatin1String("http://ws.audioscrobbler.com/2.0/"));
    query.addQueryItem(QLatin1String("api_key"), Global::api_key);
    query.addQueryItem(QLatin1String("method"), QLatin1String("auth.getSession"));
    query.addQueryItem(QLatin1String("token"), Global::token);
    query.addQueryItem(QLatin1String("api_sig"), Scrobbler::getRequestSignature(query));
    url.setQuery(query);

    const QNetworkRequest request(url);
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    connect(nam, &QNetworkAccessManager::finished,
            this, &Auth::slotGotSession);
    connect(nam, &QNetworkAccessManager::finished,
            nam, &QNetworkAccessManager::deleteLater);
    nam->get(request);

    qDebug() << "Requesting new session key...";
}

void Auth::slotGotSession(QNetworkReply *reply)
{
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 200) {
        qDebug() << "Session key request failed, http error" << status;
        qDebug() << reply->readAll();
        m_scrobbler->raiseError(-1);
        return;
    }

    // Read the XML document
    QDomDocument document;
    document.setContent(reply->readAll());

    // <lfm status="ok/failed">
    const QDomElement lfm = document.documentElement();
    if (lfm.attribute(QLatin1String("status")) == QLatin1String("ok")) {

        const QDomElement key = lfm.elementsByTagName(QLatin1String("key")).at(0).toElement();
        const QDomElement name = lfm.elementsByTagName(QLatin1String("name")).at(0).toElement();
        Q_EMIT gotSession(key.childNodes().at(0).nodeValue(),
                        name.childNodes().at(0).nodeValue());
        Global::session_key = key.childNodes().at(0).nodeValue();

        qDebug() << "Successfully recieved new session key";

    } else {

        // <error code="errCode">
        const QDomElement error = lfm.elementsByTagName(QLatin1String("error")).at(0).toElement();
        m_scrobbler->raiseError(error.attribute(QLatin1String("code")).toInt());

        qDebug() << "Failed to obtain new session key:" << error.nodeValue();
    }
}
