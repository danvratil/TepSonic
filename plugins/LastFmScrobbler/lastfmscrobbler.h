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

#include <QDebug>

class LastFmScrobbler : public AbstractPlugin
{
    Q_OBJECT
    public:
        LastFmScrobbler();
        ~LastFmScrobbler();
        void settingsWidget(QWidget *parentWidget);
        QString pluginName();

    public slots:
        void settingsAccepted();
        void trackChanged(MetaData trackData);
        void trackFinished(MetaData trackData);
        void playerStatusChanged(Phonon::State newState, Phonon::State oldState) { }
        void trackPositionChanged(qint64 newPos);

    private:
        QUrl prepareHandshakeURL(QString username, QString password);
        void login();
        void scrobble(MetaData metadata);

        Ui::LastFmScrobblerConfig *_configWidget;

        QString _token;
        QString _nowPlayingURL;
        QString _submissionURL;
        qint64 _played;

    private slots:
        void on_testLoginButton_clicked();
        void testLoginFinished(QNetworkReply *reply);
        void loginFinished(QNetworkReply *reply);
        void scrobblingFinished(QNetworkReply *reply);
};

#include "moc_abstractplugin.cpp"

#endif // LASTFMSCROBBLER_H
