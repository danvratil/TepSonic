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
        //! Constructor
        /*!
          Loads settings and try to login and obtain token and url
          \sa login()
        */
        LastFmScrobbler();

        //! Destructor
        ~LastFmScrobbler();

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
          Sents a HTTP request to Last.fm with some data contained in \p metadata. When
          request is successfull and servers replies, scrobblingFinished() slot is invoked
          \param metadata meta data of the track to be submitted
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
};

#include "moc_abstractplugin.cpp"

#endif // LASTFMSCROBBLER_H
