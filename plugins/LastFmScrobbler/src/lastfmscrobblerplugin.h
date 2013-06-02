/*
 * TEPSONIC
 * Copyright 2010 Dan Vratil <vratil@progdansoft.com>
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

#include "abstractplugin.h"
#include "player.h"
#include "lastfm.h"

#include "ui_lastfmscrobblerconfig.h"

#include <QObject>
#include <QWidget>
#include <QString>
#include <QTranslator>
#include <QMenu>

//! LastFmScrobbler is a plugin for scrobbling recently played tracks to Last.fm
/*!
  LastFmScrobbler is a plugin for scrobbling recently played tracks to Last.fm
  via their submission API using lastfmlib.
*/
class LastFmScrobblerPlugin : public AbstractPlugin
{
    Q_OBJECT
    public:
        //! Constructor
        /*!
          Creates the plugin.
        */
        LastFmScrobblerPlugin();

        //! Destructor
        ~LastFmScrobblerPlugin();

        //! Initialize the plugins
        /*!
          Loads settings
        */
        void init();

        //! Prepares the plugin to be disabled
        void quit();

        //! Initializes UI on given widget
        /*!
          Installs user interface for configuration via Ui::LastFmScrobblerConfig::setupUI() on
          given parentWidget
          \param parentWidget widget to setup the UI on
        */
        void settingsWidget(QWidget *parentWidget);

        //! Add custom menu items to given menu. */
        void setupMenu(QMenu *menu, Plugins::MenuTypes menuType);

    public slots:

        //! Notification about new track
        /*!
          \param trackData meta data of the new track
        */
        void trackChanged(Player::MetaData trackData);


        //! Notification that player state has changed (paused/unpuased etc)
        void playerStatusChanged(Phonon::State newState, Phonon::State oldState);

        //! Submit track as loved()
        void loveTrack();

    private slots:
        void initScrobbler();

        void authenticate();

        void gotToken(QString token);

        void gotSessionKey(QString session);

    private:
        LastFm::Scrobbler *m_scrobbler;
        LastFm::Auth *m_auth;

        //! Configuration UI
        Ui::LastFmScrobblerConfig *m_configWidget;

        QTranslator *m_translator;

        QString m_token;


    signals:

        void error(QString);

};

#endif // LASTFMSCROBBLERPLUGIN_H
