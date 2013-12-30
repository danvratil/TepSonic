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

#ifndef LASTFM_TRACK_H
#define LASTFM_TRACK_H

#include <QObject>
#include "metadata.h"

class QNetworkReply;

namespace LastFm {

class Scrobbler;

class Track : public QObject
{
    Q_OBJECT
  public:
    Track(LastFm::Scrobbler *scobbler, const MetaData &metaData, uint playbackStart = 0);

    MetaData metaData() const;

    void setPlaybackStart(uint playbackStart);
    uint playbackStart() const;

  public Q_SLOTS:
    void scrobble();
    void nowPlaying();
    void love();
    void pause(bool pause);

  Q_SIGNALS:
    void scrobbled();
    void loved();

  private Q_SLOTS:
    void scrobbled(QNetworkReply *reply);

  private:
    LastFm::Scrobbler *m_scrobbler;

    /** Track data **/
    MetaData m_metaData;
    uint m_playbackStart;

    /** Helper time counter **/
    int m_playbackLength;
    uint m_unpauseTime;
};

}

#endif // LASTFM_TRACK_H
