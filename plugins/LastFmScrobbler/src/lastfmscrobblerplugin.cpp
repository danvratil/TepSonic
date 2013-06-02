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
#include "playlist/playlistbrowser.h"
#include "player.h"

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
    m_translator = new QTranslator(this);
#ifndef APPLEBUNDLE
    // standard unix/windows
    QString dataDir = QLatin1String(PKGDATADIR);
    QString localeDir = dataDir + QDir::separator() + "tepsonic" + QDir::separator() +  "locale" + QDir::separator() + "lastfmscrobbler";
#else
    // mac's bundle. Special stuff again.
    QString localeDir = QCoreApplication::applicationDirPath() + "/../Resources/lastfmscrobbler";
#endif

    m_translator->load("lastfmscrobbler_"+locale,localeDir);
    qApp->installTranslator(m_translator);

    connect(Player::instance(), SIGNAL(trackChanged(Player::MetaData)),
            this, SLOT(trackChanged(Player::MetaData)));
    connect(Player::instance(), SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(playerStatusChanged(Phonon::State,Phonon::State)));
}

LastFmScrobblerPlugin::~LastFmScrobblerPlugin()
{
    delete m_translator;
}

void LastFmScrobblerPlugin::init()
{
    QSettings settings(QString(_CONFIGDIR) + QDir::separator() + "lastfmscrobbler.conf",QSettings::IniFormat,this);
    QString key = settings.value("key", QString()).toString();

    LastFm::Global::api_key = "824f0af8fbc9ca2dd16091ad47817988";
    LastFm::Global::secret_key = "15545c2b44b3e3108a73bb0ad4bc23ea";
    LastFm::Global::session_key = key;

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
    m_configWidget = new Ui::LastFmScrobblerConfig();
    m_configWidget->setupUi(parentWidget);

    connect(m_configWidget->authButton, SIGNAL(clicked()),
            this, SLOT(authenticate()));
}


void LastFmScrobblerPlugin::setupMenu(QMenu *menu, Plugins::MenuTypes menuType)
{
    if (menuType == Plugins::PlaylistPopup) {
        QMenu *pmenu = new QMenu(tr("Last.fm"), menu);
        pmenu->addAction(tr("Love track"), this, SLOT(loveTrack()));
        pmenu->setProperty("menuType", menuType);
        menu->addSeparator();
        menu->addMenu(pmenu);
    }

    if (menuType == Plugins::TrayMenu) {
        QMenu *pmenu = new QMenu(tr("Last.fm"), menu);
        pmenu->addAction(tr("Love current track"), this, SLOT(loveTrack()));
        pmenu->setProperty("menuType", menuType);
        QAction *quitAction = menu->actions().last();
        menu->insertSeparator(quitAction);
        menu->insertMenu(quitAction, pmenu);
        menu->insertSeparator(quitAction);
    }
}

void LastFmScrobblerPlugin::trackChanged(Player::MetaData trackData)
{
    // Submit the old track
    if (m_scrobbler->currentTrack())
        m_scrobbler->currentTrack()->scrobble();

    uint stamp = QDateTime::currentDateTime().toTime_t();

    LastFm::Track *track = new LastFm::Track(m_scrobbler);
    track->setArtist(trackData.artist);
    track->setTrackTitle(trackData.title);
    track->setAlbum(trackData.album);
    track->setTrackLength(int(trackData.length/1000));
    track->setGenre(trackData.genre);
    track->setTrackNumber(trackData.trackNumber);
    track->setPlaybackStart(stamp);
    m_scrobbler->setCurrentTrack(track);

    // Set "Now playing"
    track->nowPlaying();
}

void LastFmScrobblerPlugin::playerStatusChanged(Phonon::State newState, Phonon::State oldState)
{
    if ((oldState == Phonon::PausedState) && (newState == Phonon::PlayingState))
        if (m_scrobbler->currentTrack())
            m_scrobbler->currentTrack()->pause(false);

    if ((oldState == Phonon::PlayingState) && (newState == Phonon::PausedState))
        if (m_scrobbler->currentTrack())
            m_scrobbler->currentTrack()->pause(true);

}

void LastFmScrobblerPlugin::initScrobbler()
{

    qDebug() << "Initializing scrobbler";
    m_scrobbler = new LastFm::Scrobbler();
    connect(m_scrobbler, SIGNAL(error(QString,int)),
            this, SIGNAL(error(QString)));
    connect(m_scrobbler, SIGNAL(gotSessionKey(QString)),
            this, SLOT(gotSessionKey(QString)));

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

    qDebug() << "Recieved token; waiting 60 seconds before initializing scrobbler";

    // Open browser with Last.fm auth web site so user can confirm the token
    QDesktopServices::openUrl(QUrl("http://www.last.fm/api/auth/?api_key="+LastFm::Global::api_key+"&token="+token));
    m_configWidget->label->setText(tr("Now click 'Allow' in your browser, then you can close the browser and this window too."));

    // Wait 60 seconds (that should be enough time for browser to start and load page and for
    // user to click "Allow" button on the page), then re-initialize the scrobbler with
    // new token
    QTimer::singleShot(60000, this, SLOT(initScrobbler()));
}

void LastFmScrobblerPlugin::gotSessionKey(QString session)
{
    QSettings settings(QString(_CONFIGDIR) + QDir::separator() + "lastfmscrobbler.conf",QSettings::IniFormat,this);
    settings.setValue("key", session);

    LastFm::Global::session_key = session;
}


void LastFmScrobblerPlugin::loveTrack()
{
    // The "Love" QAction
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) return;

    // The "Last.fm" menu
    QMenu *lastfmmenu = qobject_cast<QMenu*>(action->parent());
    if (!lastfmmenu) return;


    // Signal from playlist popup
    if (lastfmmenu->property("menuType") == Plugins::PlaylistPopup) {

        // The parent menu
        QMenu *popupmenu = qobject_cast<QMenu*>(lastfmmenu->parent());
        if (!popupmenu) return;

        // Pointer to QModelIndex stored in menu property
        QModelIndex itemIndex = *(QModelIndex*)popupmenu->property("playlistItem").value<void *>();

        // From playlist, we can love any track, so we need to get more info about the track
        QString trackname = itemIndex.sibling(itemIndex.row(), PlaylistBrowser::TracknameColumn).data().toString();
        QString artist = itemIndex.sibling(itemIndex.row(), PlaylistBrowser::InterpretColumn).data().toString();

        // Now we construct the track
        LastFm::Track *track = new LastFm::Track(m_scrobbler);
        track->setTrackTitle(trackname);
        track->setArtist(artist);

        track->love();
        connect(track, SIGNAL(loved()),
                track, SLOT(deleteLater()));
    }

    // Signal from tray menu item
    if (lastfmmenu->property("menuType") == Plugins::TrayMenu) {

        // From tray, we can love only current track
        if (m_scrobbler->currentTrack())
            m_scrobbler->currentTrack()->love();
    }



}

Q_EXPORT_PLUGIN2(tepsonic_lastfmscrobbler, LastFmScrobblerPlugin)
