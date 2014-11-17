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

#ifndef LASTFM_AUTH_H
#define LASTFM_AUTH_H

#include <QtCore/QObject>

class QNetworkReply;

namespace LastFm {

class Scrobbler;

class Auth : public QObject
{
    Q_OBJECT

  public:
    Auth(LastFm::Scrobbler *scrobbler);
    ~Auth() {}

  public Q_SLOTS:
    void getToken();
    void getSession();

  private Q_SLOTS:
    void slotGotToken(QNetworkReply *reply);
    void slotGotSession(QNetworkReply *reply);

  private:
    LastFm::Scrobbler *m_scrobbler;

  Q_SIGNALS:
    void gotToken(const QString &token);
    void gotSession(const QString &key, const QString &username);

};

}

#endif // LASTFM_AUTH_H
