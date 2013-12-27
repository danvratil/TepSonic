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

#include "scrobbler.h"
#include "cache.h"
#include "auth.h"
#include "track.h"
#include "lastfm.h"

#include <QTimer>

using namespace LastFm;

Scrobbler::Scrobbler():
    QObject(),
    m_cache(0),
    m_auth(0),
    m_ready(false),
    m_currentTrack(0)
{
    m_cache = new Cache(this);
    m_auth = new Auth(this);
    connect(m_auth, &Auth::gotSession,
            this, &Scrobbler::setSession);
    connect(m_auth, &Auth::gotSession,
            [=](const QString &key, const QString &username) {
                Q_EMIT gotSessionKey(key); Q_UNUSED(username);
            });

    if (Global::session_key.isEmpty()) {
        m_auth->getSession();
    }
}

Scrobbler::~Scrobbler()
{
    m_ready = false;
}

void Scrobbler::setSession(const QString &key, const QString &username)
{
    Global::session_key = key;
    Global::username = username;
    m_ready = true;
}

void Scrobbler::scrobble()
{
    if (m_currentTrack) {
        m_currentTrack->scrobble();
    } else {
        m_cache->submit();
    }
}

void Scrobbler::scrobble(Track *track)
{
    track->scrobble();
}

bool Scrobbler::ready() const
{
    return m_ready;
}

void Scrobbler::nowPlaying(Track *track)
{
    m_currentTrack = track;
    if (track) {
        track->nowPlaying();
    }
}

void Scrobbler::raiseError(int code)
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
        errmsg = QLatin1String("Invalid service - This service does not exist");
        break;
    case 3:
        errmsg = QLatin1String("Invalid Method - No method with that name in this package");
        break;
    case 4:
        //errmsg = "Authentication Failed - You do not have permissions to access the service";
        errmsg = tr("Authentication Failed - Please try to obtain a new token in TepSonic -> Settings -> Plugins -> Last.Fm");
        break;
    case 5:
        errmsg = QLatin1String("Invalid format - This service doesn't exist in that format");
        break;
    case 6:
        errmsg = QLatin1String("Invalid parameters - Your request is missing a required parameter");
        break;
    case 7:
        errmsg = QLatin1String("Invalid resource specified");
        break;
    case 9:
        // Don't tell anything about this, try to re-authenticate
        //errmsg = "Invalid session key - Please re-authenticate";
        m_auth->getSession();
        return;
        break;
    case 10:
        errmsg = QLatin1String("Invalid API key - You must be granted a valid key by last.fm");
        break;
    case 11: // Service Offline - This service is temporarily offline. Try again later.
    case 16: // There was a temporary error processing your request. Please try again
        QTimer::singleShot(1200000, m_cache, SLOT(submit()));
        return;
        break;
    case 13:
        errmsg = QLatin1String("Invalid method signature supplied");
        break;
    case 26:
        errmsg = QLatin1String("Suspended API key - Access for your account has been suspended, please contact Last.fm");
        break;
    }

    Q_EMIT error(errmsg, code);
}

QString Scrobbler::getRequestSignature(const QUrlQuery &query)
{
    QUrlQuery q(query);
    // sort the list of query items alphabetically
    QList<QPair<QString, QString> > list = q.queryItems();
    qSort(list);

    q.setQueryItems(list);

    // convert name1=value1&name2=value2... to name1value1name2value2
    QString str = q.toString();
    str.remove(q.queryPairDelimiter());
    str.remove(q.queryValueDelimiter());
    str.append(Global::secret_key);
    QByteArray ba;
    ba.append(str.toLatin1());

    return QString::fromLatin1(QCryptographicHash::hash(ba, QCryptographicHash::Md5).toHex());
}

Cache* Scrobbler::cache() const
{
    return m_cache;
}

Track* Scrobbler::currentTrack() const
{
    return m_currentTrack;
}

void Scrobbler::setCurrentTrack(Track* track)
{
    m_currentTrack = track;
}

