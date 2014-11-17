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

#ifndef LASTFM_H
#define LASTFM_H

#include <QObject>
#include <QNetworkReply>
#include <QTimer>
#include <QUrlQuery>

namespace LastFm {

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
}

#endif // LASTFM_H
