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

#ifndef LASTFM_CACHE_H
#define LASTFM_CACHE_H

#include <QObject>
#include <QVector>

class QNetworkReply;
class QTimer;

namespace LastFm {

class Scrobbler;
class Track;

class Cache : public QObject
{
    Q_OBJECT

  public:
    Cache(LastFm::Scrobbler *scrobbler);
    ~Cache();

  public Q_SLOTS:
    void add(LastFm::Track *track);
    void submit();

  private Q_SLOTS:
    void submitted(QNetworkReply *reply);

  private:
    void loadCache();

    QVector<LastFm::Track*> m_cache;
    LastFm::Scrobbler *m_scrobbler;

    QTimer *m_resubmitTimer;

    // Let's have access to track's private items
    friend class Track;
};

}

#endif // LASTFM_CACHE_H
