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

#ifndef LASTFM_SCROBBLER_H
#define LASTFM_SCROBBLER_H

#include <QObject>

class QUrlQuery;

namespace LastFm {

class Track;
class Cache;
class Auth;

class Scrobbler : public QObject
{
    Q_OBJECT
  public:
    Scrobbler();
    ~Scrobbler();

    void scrobble();
    void scrobble(LastFm::Track *track);

    bool ready() const;
    void raiseError(int code);

    static QString getRequestSignature(const QUrlQuery &request);
    LastFm::Cache* cache() const;

    void nowPlaying(LastFm::Track *track);

    LastFm::Track* currentTrack() const;
    void setCurrentTrack(LastFm::Track* track);

  Q_SIGNALS:
    void error(const QString &message, int code);
    void gotSessionKey(const QString &key);

  private Q_SLOTS:
    void setSession(const QString &key, const QString &username);

  private:
    LastFm::Cache *m_cache;
    LastFm::Auth *m_auth;
    bool m_ready;
    LastFm::Track *m_currentTrack;
};

}

#endif // LASTFM_SCROBBLER_H
