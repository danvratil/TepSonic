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

#include "lastfmscrobblerplugin.h"
#include "lastfm.h"
#include "auth.h"
#include "scrobbler.h"
#include "track.h"

#include <core/playlist.h>
#include <core/player.h>
#include <core/settings.h>

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
#include <QtPlugin>

using namespace TepSonic;

LastFmScrobblerPlugin::LastFmScrobblerPlugin():
    AbstractPlugin(),
    m_scrobbler(0),
    m_auth(0)
{
    QString locale = QLocale::system().name();
    m_translator = new QTranslator(this);

    // standard unix/windows
    const QString dataDir = QLatin1String(PKGDATADIR);
    const QString localeDir = QLatin1String(PKGDATADIR) + QLatin1String("/tepsonic/locale");
    m_translator->load(QLatin1String("tepsonic_lastfmscrobbler_") + locale, localeDir);
    qApp->installTranslator(m_translator);

    connect(Player::instance(), SIGNAL(trackChanged()),
            this, SLOT(trackChanged()));
    connect(Player::instance(), SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(playerStatusChanged(Phonon::State,Phonon::State)));
}

LastFmScrobblerPlugin::~LastFmScrobblerPlugin()
{
    delete m_translator;
}

void LastFmScrobblerPlugin::init()
{
    const QString configFile = Settings::configDir() + QLatin1String("/lastfmscrobbler.conf");
    QSettings settings(configFile, QSettings::IniFormat, this);
    LastFm::Global::api_key = QLatin1String("824f0af8fbc9ca2dd16091ad47817988");
    LastFm::Global::secret_key = QLatin1String("15545c2b44b3e3108a73bb0ad4bc23ea");
    LastFm::Global::session_key = settings.value(QLatin1String("key"), QString()).toString();
    LastFm::Global::token = settings.value(QLatin1String("token"), QString()).toString();

    initScrobbler();
}

void LastFmScrobblerPlugin::quit()
{
    // Submit what's in cache
    m_scrobbler->scrobble();

    delete m_scrobbler;
}

void LastFmScrobblerPlugin::configUI(QWidget *parentWidget)
{
    m_configWidget = new Ui::LastFmScrobblerConfig();
    m_configWidget->setupUi(parentWidget);

    connect(m_configWidget->authButton, SIGNAL(clicked()),
            this, SLOT(authenticate()));
}

void LastFmScrobblerPlugin::setupMenu(QMenu *menu, AbstractPlugin::MenuTypes menuType)
{
    if (menuType == AbstractPlugin::PlaylistPopup) {
        QMenu *pmenu = new QMenu(tr("Last.fm"), menu);
        pmenu->addAction(tr("Love track"), this, SLOT(loveTrack()));
        pmenu->setProperty("menuType", menuType);
        menu->addSeparator();
        menu->addMenu(pmenu);
    }

    if (menuType == AbstractPlugin::TrayMenu) {
        QMenu *pmenu = new QMenu(tr("Last.fm"), menu);
        pmenu->addAction(tr("Love current track"), this, SLOT(loveTrack()));
        pmenu->setProperty("menuType", menuType);
        QAction *quitAction = menu->actions().last();
        menu->insertSeparator(quitAction);
        menu->insertMenu(quitAction, pmenu);
        menu->insertSeparator(quitAction);
    }
}

void LastFmScrobblerPlugin::trackChanged()
{
    // Submit the old track
    if (m_scrobbler->currentTrack()) {
        m_scrobbler->currentTrack()->scrobble();
    }

    uint stamp = QDateTime::currentDateTime().toTime_t();

    int currentTrack = Player::instance()->currentTrack();
    if (currentTrack == -1) {
        return;
    }

    MetaData metaData = Player::instance()->playlist()->track(currentTrack);
    LastFm::Track *track = new LastFm::Track(m_scrobbler, metaData, stamp);
    m_scrobbler->setCurrentTrack(track);

    // Set "Now playing"
    track->nowPlaying();
}

void LastFmScrobblerPlugin::playerStatusChanged(Phonon::State newState, Phonon::State oldState)
{
    if ((oldState == Phonon::PausedState) && (newState == Phonon::PlayingState))  {
        if (m_scrobbler->currentTrack()) {
            m_scrobbler->currentTrack()->pause(false);
        }
    }

    if ((oldState == Phonon::PlayingState) && (newState == Phonon::PausedState)) {
        if (m_scrobbler->currentTrack()) {
            m_scrobbler->currentTrack()->pause(true);
        }
    }
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

void LastFmScrobblerPlugin::gotToken(const QString &token)
{
    if (token.isEmpty()) {
        qDebug() << "LastFmScrobbler: Got empty token!";
        return;
    }

    LastFm::Global::token = token;

    qDebug() << "Recieved token; waiting 60 seconds before initializing scrobbler";

    // Open browser with Last.fm auth web site so user can confirm the token
    QDesktopServices::openUrl(QUrl(QLatin1String("http://www.last.fm/api/auth/?api_key=") 
                                    + LastFm::Global::api_key + QLatin1String("&token=") + token));
    m_configWidget->label->setText(tr("Now click 'Allow' in your browser, then you can close the browser and this window too."));

    // Wait 60 seconds (that should be enough time for browser to start and load page and for
    // user to click "Allow" button on the page), then re-initialize the scrobbler with
    // new token
    QTimer::singleShot(60000, this, SLOT(initScrobbler()));
}

void LastFmScrobblerPlugin::gotSessionKey(const QString &session)
{
    const QString configFile = Settings::configDir() + QLatin1String("/lastfmscrobbler.conf");
    QSettings settings(configFile, QSettings::IniFormat,this);
    settings.setValue(QLatin1String("key"), session);

    LastFm::Global::session_key = session;
}


void LastFmScrobblerPlugin::loveTrack()
{
    // The "Love" QAction
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) {
        return;
    }

    // The "Last.fm" menu
    QMenu *lastfmmenu = qobject_cast<QMenu*>(action->parent());
    if (!lastfmmenu) {
        return;
    }


    // Signal from playlist popup
    if (lastfmmenu->property("menuType") == AbstractPlugin::PlaylistPopup) {
        // The parent menu
        QMenu *popupmenu = qobject_cast<QMenu*>(lastfmmenu->parent());
        if (!popupmenu) {
            return;
        }

        // Pointer to QModelIndex stored in menu property
        const QModelIndex itemIndex = *(QModelIndex*)popupmenu->property("playlistItem").value<void *>();
        // Now we construct the track
        const MetaData metaData = itemIndex.data(Playlist::MetaDataRole).value<MetaData>();
        LastFm::Track *track = new LastFm::Track(m_scrobbler, metaData);

        track->love();
        connect(track, SIGNAL(loved()),
                track, SLOT(deleteLater()));
    }

    // Signal from tray menu item
    if (lastfmmenu->property("menuType") == AbstractPlugin::TrayMenu) {
        // From tray, we can love only current track
        if (m_scrobbler->currentTrack()) {
            m_scrobbler->currentTrack()->love();
        }
    }
}
