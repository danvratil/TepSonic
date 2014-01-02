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

#include "player.h"
#include "actionmanager.h"
#include "constants.h"

#include <QtCore/QSettings>

#include <QFileInfo>
#include <QStringList>
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

    RepeatMode repeatMode;
    bool randomMode;

    Phonon::MediaObject *phononPlayer;
    Phonon::AudioOutput *audioOutput;
    Phonon::Path phononPath;

    QList<Phonon::Effect *> effects;
};

Player* Player::Private::sInstance = 0;

Player::Private::Private():
    repeatMode(Player::RepeatOff),
    randomMode(false)
{
}

Player::Private::~Private()
{
    delete phononPlayer;
    delete audioOutput;

    // Free all effects
    qDeleteAll(effects);
}

void Player::Private::loadEffects()
{
    const QSettings settings(QString(XdgConfigDir).append(QLatin1String("/main.conf")), QSettings::IniFormat);
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
            this, &Player::emitFinished);
    connect(d->phononPlayer, &Phonon::MediaObject::stateChanged,
            this, &Player::stateChanged);
    connect(d->phononPlayer, &Phonon::MediaObject::tick,
            this, &Player::trackPositionChanged);
    //connect(m_phononPlayer,SIGNAL(currentSourceChanged(Phonon::MediaSource)),this,SLOT(emitTrackChanged()));

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

void Player::setTrack(const QString &fileName, bool autoPlay)
{
    // Stop current track
    d->phononPlayer->stop();

    if (QFileInfo(fileName).isFile()) {
        d->phononPlayer->setCurrentSource(Phonon::MediaSource(QUrl::fromLocalFile(fileName)));
    }

    Q_EMIT trackChanged(currentMetaData());
    if (autoPlay == true) {
        play();
    }
}

void Player::setRandomMode(bool randomMode)
{
    if (d->randomMode != randomMode) {
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
}

bool Player::randomMode() const
{
    return d->randomMode;
}

void Player::setRepeatMode(RepeatMode repeatMode)
{
    if (d->repeatMode != repeatMode) {
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
}

Player::RepeatMode Player::repeatMode() const
{
    return d->repeatMode;
}


void Player::play()
{
    d->phononPlayer->play();
}

void Player::pause()
{
    d->phononPlayer->pause();
    Q_EMIT trackPaused((d->phononPlayer->state() == Phonon::PausedState));
}

void Player::stop()
{
    d->phononPlayer->stop();
    // Empty the source
    d->phononPlayer->setCurrentSource(Phonon::MediaSource());
    Q_EMIT trackChanged(MetaData());
}

MetaData Player::currentMetaData() const
{
    // FIXME: Cache the metadata, or get them directly from Playlist!
    const QString filename = d->phononPlayer->currentSource().fileName();
    if ((!QFileInfo(filename).exists()) ||
        (d->phononPlayer->currentSource().type()==Phonon::MediaSource::Invalid)) {
        return MetaData();
    }

    TagLib::FileRef f(filename.toUtf8().constData());
    return MetaData(f);
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

Phonon::MediaSource Player::currentSource() const
{
    return d->phononPlayer->currentSource();
}

QString Player::errorString() const
{
    return d->phononPlayer->errorString();
}

Phonon::State Player::playerState() const
{
    return d->phononPlayer->state();
}



void Player::emitFinished()
{
    // Emit both signals - with parameter and without!!!!
    Q_EMIT trackFinished(currentMetaData());
    Q_EMIT trackFinished();
}

void Player::setDefaultOutputDevice()
{
    const QSettings settings(QString(XdgConfigDir).append(QLatin1String("/main.conf")), QSettings::IniFormat);
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
