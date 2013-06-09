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
#include "constants.h"

#include <QtCore/QSettings>

#include <QFileInfo>
#include <QStringList>
#include <phonon/mediaobject.h>
#include <phonon/path.h>
#include <phonon/audiooutput.h>
#include <phonon/mediasource.h>
#include <phonon/backendcapabilities.h>
#include <phonon/effect.h>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>

Player* Player::s_instance = 0;

Player* Player::instance()
{
    if (s_instance == 0) {
        s_instance = new Player();
    }

    return s_instance;
}

Player::Player()
{
    m_phononPlayer = new Phonon::MediaObject();
    m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory);
    setDefaultOutputDevice();
    m_phononPath = Phonon::createPath(m_phononPlayer,
                                      m_audioOutput);

    // By default tick every 1 second
    m_phononPlayer->setTickInterval(1000);

    connect(m_phononPlayer, SIGNAL(finished()),
            this, SLOT(emitFinished()));
    connect(m_phononPlayer, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SIGNAL(stateChanged(Phonon::State,Phonon::State)));
    connect(m_phononPlayer, SIGNAL(tick(qint64)),
            this, SIGNAL(trackPositionChanged(qint64)));
    //connect(m_phononPlayer,SIGNAL(currentSourceChanged(Phonon::MediaSource)),this,SLOT(emitTrackChanged()));

    m_randomMode = false;
    m_repeatMode = RepeatOff;

    loadEffects();
}

Player::~Player()
{
    if (m_phononPlayer->state() == Phonon::PlayingState)
        m_phononPlayer->stop();

    delete m_phononPlayer;
    delete m_audioOutput;

    // Free all effects
    qDeleteAll(m_effects);
    m_effects.clear();
}

void Player::setTrack(const QString &fileName, bool autoPlay)
{
    // Stop current track
    m_phononPlayer->stop();

    if (QFileInfo(fileName).isFile()) {
        m_phononPlayer->setCurrentSource(Phonon::MediaSource(fileName));
    }

    Q_EMIT trackChanged(currentMetaData());
    if (autoPlay==true) {
        play();
    }
}

void Player::setRandomMode(bool randomMode)
{
    if (m_randomMode != randomMode) {
        m_randomMode = randomMode;
        Q_EMIT randomModeChanged(randomMode);
    }
}

void Player::setRepeatMode(RepeatMode repeatMode)
{
    if (m_repeatMode != repeatMode) {
        m_repeatMode = repeatMode;
        Q_EMIT repeatModeChanged(repeatMode);
    }
}

void Player::pause()
{
    m_phononPlayer->pause();
    Q_EMIT trackPaused((m_phononPlayer->state() == Phonon::PausedState));
}

Player::MetaData Player::currentMetaData() const
{
    Player::MetaData data;

    const QString filename = m_phononPlayer->currentSource().fileName();
    if ((!QFileInfo(filename).exists()) ||
        (m_phononPlayer->currentSource().type()==Phonon::MediaSource::Invalid)) {
        return data;
    }

    TagLib::FileRef f(filename.toUtf8().constData());
    data.artist = f.tag()->artist().toCString(true);
    data.title = f.tag()->title().toCString(true);
    data.album = f.tag()->album().toCString(true);
    data.trackNumber = f.tag()->track();
    // Length in msecs
    data.length = f.audioProperties()->length()*1000;
    data.filename = filename;

    return data;
}

void Player::stop()
{
    m_phononPlayer->stop();
    // Empty the source
    m_phononPlayer->setCurrentSource(Phonon::MediaSource());
    Q_EMIT trackChanged(MetaData());
}

void Player::emitFinished()
{
    // Emit both signals - with parameter and without!!!!
    Q_EMIT trackFinished(currentMetaData());
    Q_EMIT trackFinished();
}

void Player::setDefaultOutputDevice()
{
    const QSettings settings(QString(_CONFIGDIR).append("/main.conf"),QSettings::IniFormat);
    const int index = settings.value("Preferences/OutputDevice", 0).toInt();

    QList<Phonon::AudioOutputDevice> devices = Phonon::BackendCapabilities::availableAudioOutputDevices();
    for (int i = 0; i < devices.length(); i++)
        if (devices.at(i).index() == index) {
            qDebug() << "Changing output audio device to" << devices.at(i).name();
            m_audioOutput->setOutputDevice(devices.at(i));
        }
}

void Player::loadEffects()
{
    const QSettings settings(QString(_CONFIGDIR).append("/main.conf"), QSettings::IniFormat);
    const QList<Phonon::EffectDescription> effects = Phonon::BackendCapabilities::availableAudioEffects();
    for (int i = 0; i < effects.count(); i++)
    {
        Phonon::Effect *effect = new Phonon::Effect(effects.at(i));
        m_effects.append(effect);
        const bool state = settings.value("Preferences/Effects/"+effects.at(i).name(), 0).toBool();
        if (state) {
            m_phononPath.insertEffect(effect);
        }
    }
}

void Player::enableEffect(Phonon::Effect *effect, bool enable)
{
    if (enable && !m_phononPath.effects().contains(effect)) {
        m_phononPath.insertEffect(effect);
    } else if (!enable && m_phononPath.effects().contains(effect)) {
        m_phononPath.removeEffect(effect);
    }
}

#include "moc_player.cpp"
