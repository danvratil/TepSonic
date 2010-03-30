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

#ifndef LASTFMSCROBBLER_H
#define LASTFMSCROBBLER_H

#include "abstractplugin.h"
#include "player.h"

#include "ui_lastfmscrobblerconfig.h"

#include <QObject>
#include <QWidget>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>


//! LastFmScrobbler is a plugin for scrobbling recently played tracks to Last.fm
/*!
  LastFmScrobbler is a plugin for scrobbling recently played tracks to Last.fm
  via their submission API.
*/
class LastFmScrobbler : public AbstractPlugin
{
    Q_OBJECT
    public:
        struct MetaData {
            Player::MetaData trackInfo;
            uint playbackStart;
        };

        //! Constructor
        /*!
          Creates the plugin.
        */
        LastFmScrobbler();

        //! Destructor
        ~LastFmScrobbler() {}

        //! Initialize the plugins
        /*!
          Loads settings, calls login() for the first time
          \sa login()
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

        //! Returns name of the plugin
        /*!
          \return Returns name of the plugin
        */
        QString pluginName();

    public slots:
        //! Notification that configuration dialog was accepted
        /*!
          When settings is accepted, plugin will store the configuration
          into it's own config file
        */
        void settingsAccepted();

        //! Notification about new track
        /*!
          \param trackData meta data of the new track
        */
        void trackChanged(Player::MetaData trackData);

        //! Notification that current track has finished playing
        /*!
          When track has finished playing it is submitted to the Last.fm.
          \param trackData meta data of the recently finished track
          \sa scrobble()
        */
        void trackFinished(Player::MetaData trackData);

        //! Notification about new position in the playback ticked every second
        /*!
          \param newPos position in the playback in milliseconds
        */
        void trackPositionChanged(qint64 newPos);

    private:
        //! Creates URL to be sent to server for handshake
        /*!
          \param username username to use
          \param password password to use (password is not sent in plain)
        */
        QUrl prepareHandshakeURL(QString username, QString password);

        //! Makes a HTTP request on URL prepared by prepareHandshakeURL() method
        /*!
          When the request is sucessfull and server replies, loginFinished() slot is invoked
          \sa loginFinished()
        */
        void login();

        //! Makes a submission to Last.fm server
        /*!
          Appends new track to the queue (cache) and calls submitTrack() to try to submit the track.
          \param metadata meta data of the track to be submitted
          \sa submitTrack()
        */
        void scrobble(Player::MetaData metadata);

        //! Configuration UI
        Ui::LastFmScrobblerConfig *_configWidget;

        //! User token - identifies the session
        QString _token;

        //! URL where "now playing" can be sumitted
        QString _nowPlayingURL;

        //! URL where track submissions are sent
        QString _submissionURL;

        //! Amount of time played
        qint64 _played;

        int _failedAttempts;

        QList<LastFmScrobbler::MetaData> _cache;

    private slots:
        //! Called when testLoginButton is clicked
        /*!
          This attempts to get a session token from Last.fm server using recently
          set username and password.
          When HTTP request on Last.fm server is sucessfull and server replies, testLoginFinished()
          slot is invoked
          \sa testLoginFinished()
        */
        void on_testLoginButton_clicked();

        //! Called when data from request made in on_testLoginButton_clicked() are recieved.
        /*!
          The reply is evaluated and proper message is displayed to user on the configuration dialog
          \param reply reply from Last.fm server
          \sa on_testLoginButton_clicked()
        */
        void testLoginFinished(QNetworkReply *reply);

        //! Called when data from request made in login() are recieved
        /*!
          The reply is then evaluated. When OK, the token and URLs are read and stored to be used for submissions.
          If the reply is not OK an error message is displayed in main window via inherited signal error()
          \param reply reply from Last.fm server
          \sa login(), error()
        */
        void loginFinished(QNetworkReply *reply);

        //! Called when data from request made in scrobble() are recieved
        /*!
          When reply is recieved, the response is evaluated. When there is an error, an error message with description
          recieved from server is displayed in main window via inherited signal error()
          \param reply reply from Last.fm server
          \sa reply(), error()
        */
        void scrobblingFinished(QNetworkReply *reply);

        //! Submit first track in cache
        /*!
          Attempts to submit first track in cache (if there's any) and when successfull, keeps posting until all tracks
          are submitted.
        */
        void submitTrack();

        //! Creates a timer which starts new attempt to submit a track
        /*!
          This method is called when the scrobblers fails to submit a track. First, number of failed attempts is incremented
          and then the timer is set up. The tick interval is growing exponentialy with one half of square of failed attempts
          which makes the interval to be longer and longer.
          The intervals are 7 seconds, 2 minutes, 10 minutes, 30 minutes, 1 hour, 2.7 hours etc...
        */
        void setupTimer();

        //! Saves current cache
        /*!
          Saves current cache to config file. This is performed after every change in cache in order to prevent
          lost of some tracks
        */
        void saveCache();

};

#include "moc_abstractplugin.cpp"

#endif // LASTFMSCROBBLER_H
