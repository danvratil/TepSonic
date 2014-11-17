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

#include "player.h"
#include "actionmanager.h"
#include "playlist.h"
#include "settings.h"

#include <QtCore/QSettings>

#include <QFileInfo>
#include <QStringList>
#include <QStack>
#include <QAction>
#include <phonon/mediaobject.h>
#include <phonon/path.h>
#include <phonon/audiooutput.h>
#include <phonon/mediasource.h>
#include <phonon/backendcapabilities.h>
#include <phonon/effect.h>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>

using namespace TepSonic;

class Player::Private
{
  public:
    Private();
    ~Private();
    void loadEffects();

    static Player *sInstance;

    Playlist *playlist;
    int currentTrack;
    int stopTrack;
    QStack<int> previousRandomTracks;

    RepeatMode repeatMode;
    bool randomMode;

    Phonon::MediaObject *phononPlayer;
    Phonon::AudioOutput *audioOutput;
    Phonon::Path phononPath;

    QList<Phonon::Effect *> effects;
};

Player* Player::Private::sInstance = 0;

Player::Private::Private():
    playlist(new Playlist),
    currentTrack(-1),
    stopTrack(-1),
    repeatMode(Player::RepeatOff),
    randomMode(false)
{
}

Player::Private::~Private()
{
    delete playlist;
    delete phononPlayer;
    delete audioOutput;

    // Free all effects
    qDeleteAll(effects);
}

void Player::Private::loadEffects()
{
    const QString configFile = Settings::configDir() + QLatin1String("/main.conf");
    const QSettings settings(configFile, QSettings::IniFormat);
    const QList<Phonon::EffectDescription> availableEffects = Phonon::BackendCapabilities::availableAudioEffects();
    for (int i = 0; i < availableEffects.count(); i++) {
        Phonon::Effect *effect = new Phonon::Effect(availableEffects.at(i));
        effects.append(effect);
        const bool state = settings.value(QLatin1String("Preferences/Effects/") + availableEffects.at(i).name(), 0).toBool();
        if (state) {
            phononPath.insertEffect(effect);
        }
    }
}

Player* Player::instance()
{
    if (Private::sInstance == 0) {
        Private::sInstance = new Player();
    }

    return Private::sInstance;
}

Player::Player():
    d(new Private)
{
    d->phononPlayer = new Phonon::MediaObject();
    d->audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory);
    setDefaultOutputDevice();
    d->phononPath = Phonon::createPath(d->phononPlayer, d->audioOutput);

    // By default tick every 1 second
    d->phononPlayer->setTickInterval(1000);

    connect(d->phononPlayer, &Phonon::MediaObject::finished,
            this, &Player::trackFinished);
    connect(d->phononPlayer, &Phonon::MediaObject::finished,
            this, &Player::onTrackFinished);
    connect(d->phononPlayer, &Phonon::MediaObject::stateChanged,
            this, &Player::stateChanged);
    connect(d->phononPlayer, &Phonon::MediaObject::tick,
            this, &Player::trackPositionChanged);
    connect(d->playlist, &Playlist::modelReset,
            this, &Player::onPlaylistReset);
    connect(ActionManager::instance()->action(QLatin1String("PlayerRandomOn")), &QAction::toggled,
            [=](bool toggled) { setRandomMode(toggled); });
    connect(ActionManager::instance()->action(QLatin1String("PlayerRepeatAll")), &QAction::toggled,
            [=](bool toggled) { if (toggled) { setRepeatMode(RepeatAll); } });
    connect(ActionManager::instance()->action(QLatin1String("PlayerRepeatTrack")), &QAction::toggled,
            [=](bool toggled) { if (toggled) { setRepeatMode(RepeatTrack); } });
    connect(ActionManager::instance()->action(QLatin1String("PlayerRepeatOff")), &QAction::toggled,
            [=](bool toggled) { if (toggled) { setRepeatMode(RepeatOff); } });

    d->randomMode = false;
    d->repeatMode = RepeatOff;

    d->loadEffects();
}

Player::~Player()
{
    if (d->phononPlayer->state() == Phonon::PlayingState) {
        d->phononPlayer->stop();
    }

    delete d;
}

int Player::currentTrack() const
{
    return d->currentTrack;
}

void Player::setCurrentTrack(int index)
{
    d->currentTrack = index;
    if (d->currentTrack > -1) {
        const MetaData currentTrack = d->playlist->track(index);
        const Phonon::MediaSource source(QUrl::fromLocalFile(currentTrack.fileName()));
        d->phononPlayer->setCurrentSource(source);
    }
    Q_EMIT trackChanged(index);
}

int Player::stopTrack() const
{
    return d->stopTrack;
}

void Player::setStopTrack(int index)
{
    d->stopTrack = index;
    Q_EMIT stopTrackChanged(index);
}

void Player::setRandomMode(bool randomMode)
{
    if (d->randomMode == randomMode) {
        return;
    }

    d->randomMode = randomMode;
    QString action;
    if (d->randomMode) {
        action = QStringLiteral("PlayerRandomOn");
    } else {
        action = QStringLiteral("PlayerRandomOff");
    }
    ActionManager::instance()->action(action)->setChecked(true);
    Q_EMIT randomModeChanged(randomMode);
}

bool Player::randomMode() const
{
    return d->randomMode;
}

void Player::setRepeatMode(RepeatMode repeatMode)
{
    if (d->repeatMode == repeatMode) {
        return;
    }

    d->repeatMode = repeatMode;
    QString action;
    switch (d->repeatMode) {
        case RepeatAll:
            action = QStringLiteral("PlayerRepeatAll");
            break;
        case Player::RepeatTrack:
            action = QStringLiteral("PlayerRepeatTrack");
            break;
        case Player::RepeatOff:
            action = QStringLiteral("PlayerRepeatOff");
            break;
    }
    ActionManager::instance()->action(action)->setChecked(true);
    Q_EMIT repeatModeChanged(repeatMode);
}

Player::RepeatMode Player::repeatMode() const
{
    return d->repeatMode;
}

void Player::play()
{
    if (d->phononPlayer->currentSource().fileName().isEmpty()) {
        if (d->currentTrack < 0) {
            if (d->playlist->rowCount() == 0) {
                return;
            }
            setCurrentTrack(0);
        } else {
            const MetaData currentTrack = d->playlist->track(d->currentTrack);
            const Phonon::MediaSource source(QUrl::fromLocalFile(currentTrack.fileName()));
            d->phononPlayer->setCurrentSource(source);
        }
    }

    d->phononPlayer->play();
}

void Player::pause()
{
    d->phononPlayer->pause();
}

void Player::stop()
{
    d->phononPlayer->stop();
    d->phononPlayer->setCurrentSource(Phonon::MediaSource());
}

void Player::next()
{
    int index = -1;
    const int rowCount = d->playlist->rowCount();

    if (d->randomMode) {
        if (d->currentTrack > -1) {
            d->previousRandomTracks.push(d->currentTrack);
        }
        do {
          index  = qrand() % rowCount;
        } while (index == d->currentTrack);
    } else {
        index = d->currentTrack;
        if (index < 0) {
            index = 0;
        } else if (index == rowCount - 1) {
            if (d->repeatMode == RepeatAll) {
                index = 0;
            } else {
                stop();
                return;
            }
        } else {
            ++index;
        }
    }

    setCurrentTrack(index);
    play();
}

void Player::previous()
{
    int index = -1;
    if (d->randomMode) {
        if (!d->previousRandomTracks.isEmpty()) {
            index = d->previousRandomTracks.pop();
        } else {
            do {
                index = qrand() % d->playlist->rowCount();
            } while (index == d->currentTrack);
        }
    } else {
        index = d->currentTrack;
        if (index < 0) {
            index = -1;
        } else if (index == 0) {
            if (d->repeatMode == RepeatAll) {
                index = d->playlist->rowCount() - 1;
            } else {
                stop();
                return;
            }
        } else {
            --index;
        }
    }

    setCurrentTrack(index);
    play();
}

QString Player::errorString() const
{
    return d->phononPlayer->errorString();
}

Phonon::State Player::playerState() const
{
    return d->phononPlayer->state();
}

Phonon::AudioOutput* Player::audioOutput() const
{
    return d->audioOutput;
}

Phonon::MediaObject* Player::mediaObject() const
{
    return d->phononPlayer;
}

QList< Phonon::Effect* > Player::effects() const
{
    return d->effects;
}

Playlist* Player::playlist() const
{
    return d->playlist;
}

void Player::setDefaultOutputDevice()
{
    const QString configFile = Settings::configDir() + QLatin1String("/main.conf");
    const QSettings settings(configFile, QSettings::IniFormat);
    const int index = settings.value(QLatin1String("Preferences/OutputDevice")).toInt();

    const QList<Phonon::AudioOutputDevice> devices = Phonon::BackendCapabilities::availableAudioOutputDevices();
    for (int i = 0; i < devices.length(); i++) {
        if (devices.at(i).index() == index) {
            qDebug() << "Changing output audio device to" << devices.at(i).name();
            d->audioOutput->setOutputDevice(devices.at(i));
        }
    }
}

void Player::enableEffect(Phonon::Effect *effect, bool enable)
{
    if (enable && !d->phononPath.effects().contains(effect)) {
        d->phononPath.insertEffect(effect);
    } else if (!enable && d->phononPath.effects().contains(effect)) {
        d->phononPath.removeEffect(effect);
    }
}

void Player::onPlaylistReset()
{
    d->previousRandomTracks.clear();
    setCurrentTrack(-1);
    setStopTrack(-1);
}

void Player::onTrackFinished()
{
    if (d->repeatMode == RepeatTrack) {
        play();
    } else {
        next();
    }
}
