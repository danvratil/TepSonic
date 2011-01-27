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

#include "lastfmscrobblerplugin.h"
#include "constants.h"
#include "lastfm.h"

#include <QObject>
#include <QDir>
#include <QDesktopServices>
#include <QDebug>
#include <QSettings>
#include <QString>
#include <QWidget>
#include <QTranslator>
#include <QTimer>
#include <QDateTime>

// Exports pluginName method
#ifdef Q_WS_WIN
#define NAME_EXPORT __declspec(dllexport)
#define ID_EXPORT __declspec(dllexport)
#else
#define NAME_EXPORT
#define ID_EXPORT
#endif

extern "C" NAME_EXPORT QString pluginName()
{
    return "Last.fm plugin";
}

extern "C" ID_EXPORT QString pluginID()
{
    return "lastfm";
}


LastFmScrobblerPlugin::LastFmScrobblerPlugin():
        m_scrobbler(0),
        m_auth(0)
{
    setHasConfigUI(true);

    QString locale = QLocale::system().name();
    _translator = new QTranslator(this);
#ifndef APPLEBUNDLE
    // standard unix/windows
    QString dataDir = QLatin1String(PKGDATADIR);
    QString localeDir = dataDir + QDir::separator() + "tepsonic" + QDir::separator() +  "locale" + QDir::separator() + "lastfmscrobbler";
#else
    // mac's bundle. Special stuff again.
    QString localeDir = QCoreApplication::applicationDirPath() + "/../Resources/lastfmscrobbler";
#endif

    _translator->load("lastfmscrobbler_"+locale,localeDir);
    qApp->installTranslator(_translator);

}

void LastFmScrobblerPlugin::init()
{
    QSettings settings(QString(_CONFIGDIR) + QDir::separator() + "lastfmscrobbler.conf",QSettings::IniFormat,this);
    QString token = settings.value("token", QString()).toString();

    LastFm::Global::api_key = "824f0af8fbc9ca2dd16091ad47817988";
    LastFm::Global::secret_key = "15545c2b44b3e3108a73bb0ad4bc23ea";
    LastFm::Global::token = token;

    if (!token.isEmpty())
        initScrobbler();

}

void LastFmScrobblerPlugin::quit()
{
    // Submit what's in cache
    m_scrobbler->scrobble();

    delete m_scrobbler;
}

void LastFmScrobblerPlugin::settingsWidget(QWidget *parentWidget)
{
    _configWidget = new Ui::LastFmScrobblerConfig();
    _configWidget->setupUi(parentWidget);

    connect(_configWidget->authButton, SIGNAL(clicked()),
            this, SLOT(authenticate()));
}

void LastFmScrobblerPlugin::trackFinished(Player::MetaData trackdata)
{
    // And try to submit the cache, whatever's in it
    if (m_scrobbler->currentTrack())
        m_scrobbler->scrobble(m_scrobbler->currentTrack());
}

void LastFmScrobblerPlugin::trackChanged(Player::MetaData trackData)
{
    uint stamp = QDateTime::currentDateTime().toTime_t();

    LastFm::Track *track = new LastFm::Track(m_scrobbler);
    track->setArtist(trackData.artist);
    track->setTrackTitle(trackData.title);
    track->setAlbum(trackData.album);
    track->setTrackLength(trackData.length);
    track->setGenre(trackData.genre);
    track->setTrackNumber(trackData.trackNumber);
    track->setPlaybackStart(stamp);

    qDebug() << "Created track " << track << track->trackTitle() << " in " << this;
    // Set "Now playing"
    m_scrobbler->nowPlaying(track);
}

void LastFmScrobblerPlugin::initScrobbler()
{

    qDebug() << "Initializing scrobbler";
    m_scrobbler = new LastFm::Scrobbler();
    connect(m_scrobbler, SIGNAL(error(QString,int)),
            this, SIGNAL(error(QString)));

}

void LastFmScrobblerPlugin::authenticate()
{
    m_auth = new LastFm::Auth(m_scrobbler);
    connect(m_auth, SIGNAL(gotToken(QString)),
            this, SLOT(gotToken(QString)));
    m_auth->getToken();
}

void LastFmScrobblerPlugin::gotToken(QString token)
{
    if (token.isEmpty()) {
        qDebug() << "LastFmScrobbler: Got empty token!";
        return;
    }

    LastFm::Global::token = token;
    QSettings settings(QString(_CONFIGDIR) + QDir::separator() + "lastfmscrobbler.conf",QSettings::IniFormat,this);
    settings.setValue("token", token);

    qDebug() << "Recieved token; waiting 60 seconds before initializing scrobbler";

    // Open browser with Last.fm auth web site so user can confirm the token
    QDesktopServices::openUrl(QUrl("http://www.last.fm/api/auth/?api_key="+LastFm::Global::api_key+"&token="+token));
    _configWidget->label->setText(tr("Now click 'Allow' in your browser, then you can close the browser and this window too."));

    // Wait 60 seconds (that should be enough time for browser to start and load page and for
    // user to click "Allow" button on the page), then re-initialize the scrobbler with
    // new token
    QTimer::singleShot(60000, this, SLOT(initScrobbler()));
}

Q_EXPORT_PLUGIN2(tepsonic_lastfmscrobbler, LastFmScrobblerPlugin)
