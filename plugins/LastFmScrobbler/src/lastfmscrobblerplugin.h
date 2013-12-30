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

#ifndef LASTFMSCROBBLERPLUGIN_H
#define LASTFMSCROBBLERPLUGIN_H

#include <core/abstractplugin.h>
#include <core/player.h>

#include "lastfm.h"
#include "ui_lastfmscrobblerconfig.h"

#include <QObject>
#include <QWidget>
#include <QString>
#include <QTranslator>
#include <QMenu>

namespace LastFm {
    class Scrobbler;
    class Auth;
}

class LastFmScrobblerPlugin : public TepSonic::AbstractPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.progdan.tepsonic.plugins.LastFmScrobbler"
                      FILE "LastFmScrobbler.json")

  public:
    explicit LastFmScrobblerPlugin();
    virtual ~LastFmScrobblerPlugin();

    void init();
    void quit();

    void configUI(QWidget *parentWidget);
    virtual void setupMenu(QMenu *menu, MenuTypes menuType);

  public Q_SLOTS:
    void trackChanged(const TepSonic::MetaData &trackData);
    void playerStatusChanged(Phonon::State newState, Phonon::State oldState);

    //! Submit track as loved()
    void loveTrack();

  private Q_SLOTS:
    void initScrobbler();

    void authenticate();

    void gotToken(const QString &token);

    void gotSessionKey(const QString &session);

  private:
    LastFm::Scrobbler *m_scrobbler;
    LastFm::Auth *m_auth;

    //! Configuration UI
    Ui::LastFmScrobblerConfig *m_configWidget;

    QTranslator *m_translator;

    QString m_token;

};

#endif // LASTFMSCROBBLERPLUGIN_H
