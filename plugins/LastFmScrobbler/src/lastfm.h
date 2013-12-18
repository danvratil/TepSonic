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

#ifndef LASTFM_H
#define LASTFM_H

#include <QObject>
#include <QNetworkReply>
#include <QTimer>
#include <QUrlQuery>

/** Provides implementation of Last.fm Scrobbling API, version 2.0.
  * I haven't found any library that would support the 2.0 API, so I've
  * written my own.
  * It implementes on things, that are really needed and supported in TepSonic,
  * but can be easily extended.
  */
namespace LastFm {

    // Forward declarations
    class Auth;
    class Track;
    class Cache;
    class Scrobbler;


    /** Global configuration
      * Some of variables must be set manually before doing any scrobbling.
      */
    namespace Global
    {
        /** Application API key */
        extern QString api_key;

        /** Application secret shared key */
        extern QString secret_key;

        /** Username; not actually necessary but could be useful. The username
          * is obtained by Auth::getSession() method and does not need to be
          * set manually
          */
        extern QString username;

        /** User's token. This token is used to authenticate with the Last.fm service and is
          * valid until revoked by user or replaced by another.
          * The token must be obtained for the first time by Auth::getToken() method and then verified
          * via web interface (see http://www.last.fm/api/show?service=265). After this, the token is valid
          * and can be used.
          * This library does not store or load the token automatically, so it must be set before using another
          * services.
          */
        extern QString token;

        /** Session key identifies current session. The session key must be obtained before any other
          * communication with Last.fm begins.
          * Before obtaining the session key, token must be set and validated otherwise it will not work.
          * The key is obtained and stored automatically by constructing LastFm::Auth class.
          */
        extern QString session_key;

    };





    /**
      * This class initializes the scrobbler, handles scrobbling, cache etc.
      * The class must be created before any other communication begins.
      */
    class Scrobbler: public QObject
    {
        Q_OBJECT
        public:
            Scrobbler();

            ~Scrobbler();

            /** Submits the cache scrobbling all it's content.
              * This method should be used instead of LastFm::Track::scrobble()
              */
            void scrobble();

            /** Add given track to cache and scrobble everything what's in cache */
            void scrobble(LastFm::Track *track);


            /** Returns wheter the scrobbler is ready to scrobble or not.
              * The scrobbler is ready, when session key is successfully obtained and set.
              */
            bool ready() { return m_ready; }

            /** Emits error() signal. */
            void raiseError(int code);

            /** Returns signature of given request */
            static QString getRequestSignature(QUrlQuery request);

            /** Returns pointer to scrobbler's cache */
            LastFm::Cache* cache() { return m_cache; }

            /** Scrobble informations about now playing track */
            void nowPlaying(LastFm::Track *track);

            /** Returns pointer to currently playing track **/
            LastFm::Track* currentTrack() { return m_currentTrack; }

            /** Set new current track **/
            void setCurrentTrack(LastFm::Track* track) { m_currentTrack = track; }

        private Q_SLOTS:

            /** Called when LastFm::Auth::gotSession() is emitted */
            void setSession(QString key, QString username);

        private:
            LastFm::Cache *m_cache;
            LastFm::Auth *m_auth;
            bool m_ready;
            LastFm::Track *m_currentTrack;

        Q_SIGNALS:

            void error(QString message, int code);

            void gotSessionKey(QString key);

    };





    /**
      * This class handles the authentication process. It can be used from outside to obtain
      * and parse the token. It is also used internally by LastFm::Scrobbler class to obtain
      * current session key.
      */
    class Auth: public QObject
    {
        Q_OBJECT
        public:
            Auth(LastFm::Scrobbler *scrobbler);

            ~Auth() {}

        public Q_SLOTS:

            /** Requests a new token. This token is valid until revoked or replaced. Use this method,
              * to request the token. When token is recieved, gotToken() signal is emitted passing the
              * new token. After that, the token still has to be validated via web interface.
              */
            void getToken();

            /** For internal use; this method requests session key. The session key is used to identify
              * current session.
              * When key is recieved, the getSession() slot is emitted, providing the key and username.
              */
            void getSession();

        private Q_SLOTS:
            void slotGotToken(QNetworkReply*);

            void slotGotSession(QNetworkReply*);

        private:
            LastFm::Scrobbler *m_scrobbler;

        Q_SIGNALS:

            /** Emitted when a new token is recieved. */
            void gotToken(QString token);

            /** Emitted when a new session key is recieved. */
            void gotSession(QString key, QString username);
    };





    /**
      * This class provides API for submitting, updating Now Playing and loving tracks.
      */
    class Track: public QObject
    {
        Q_OBJECT
        public:
            /** Constructs a new Track. Please note, that it is neccessary to pass pointer to
              * a valid LastFm::Scrobbler object.
              */
            Track(LastFm::Scrobbler *scobbler,
                  QString artist = QString(),
                  QString trackTitle = QString(),
                  QString album = QString(),
                  int trackLength = 0,
                  QString genre = QString(),
                  int trackNumber = 0,
                  uint playbackStart = 0);

            void setArtist(QString artist) { m_artist = artist; }
            QString artist() { return m_artist; }

            void setTrackTitle(QString trackTitle) { m_trackTitle = trackTitle; }
            QString trackTitle() { return m_trackTitle; }

            void setAlbum(QString album) { m_album = album; }
            QString album() { return m_album; }

            void setTrackLength(int trackLength) { m_trackLength = trackLength; }
            int trackLength() { return m_trackLength; }

            void setGenre(QString genre) { m_genre = genre; }
            QString genre() { return m_genre; }

            void setTrackNumber(int trackNumber) { m_trackNumber = trackNumber; }
            int trackNumber() { return m_trackNumber; }

            void setPlaybackStart(uint playbackStart) { m_playbackStart = playbackStart; }
            uint playbackStart() { return m_playbackStart; }



        public Q_SLOTS:

            /** Scrobble the track to Last.fm. When scrobbling fails, the item is held in cache
              * and is submitted after some time.
              * For internal use only. To scrobble the track to Last.fm, use LastFm::Scrobbler::scrobble()
              * method, which will submit whole cache.
              */
            void scrobble();

            /** Update "Now playing" status. The track is also stored in cache, so when the playback is
              * finished, submitting cache will update it to "recently played".
              */
            void nowPlaying();

            /** Set the track as loved.
              * When loving a track, all the informations above must be set.
              * The method works simillary to scrobble() or nowPlaying() methods and
              * it is not necessarry to call any of them to submit the love.
              */
            void love();

            /** Pause (or unpause) the track
              * This is used to count real time for which the track is playing
              */
            void pause(bool pause);


        Q_SIGNALS:
            /** This signal is emitted when the track is scrobbled */
            void scrobbled();

            /** This signal is emitted when the track is loved */
            void loved();

        private:

            LastFm::Scrobbler *m_scrobbler;

            /** Track data **/
            QString m_artist;
            QString m_trackTitle;
            QString m_album;
            int m_trackLength;
            QString m_genre;
            int m_trackNumber;
            uint m_playbackStart;

            /** Helper time counter **/
            int m_playbackLength;
            uint m_unpauseTime;


        private Q_SLOTS:

            void scrobbled(QNetworkReply*);

    };






    /** Cache provides an internal mechanism for saving unscrobbled items.
      * The cache is saved and loaded automatically, so you don't need to take
      * care of anything.
      * Do not use the cache directly as it is not neccessary and will probably not
      * work for you.
      */
    class Cache: public QObject
    {
        Q_OBJECT
        public:
            Cache(LastFm::Scrobbler *scrobbler);

            ~Cache();

        public Q_SLOTS:

            /** Add a track to the cache. */
            void add(LastFm::Track *track);

            /** Try to submit all tracks in cache. */
            void submit();

            /** Called when track are submitted */
            void submitted(QNetworkReply*);

        private:
            void loadCache();

            QList<LastFm::Track*> m_cache;
            LastFm::Scrobbler *m_scrobbler;

            QTimer *m_resubmitTimer;


        // Let's have access to track's private items
        friend class LastFm::Track;

    };

}

#endif // LASTFM_H
