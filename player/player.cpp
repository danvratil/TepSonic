/*
 * TEPSONIC
 * Copyright 2009 Dan Vratil <vratil@progdansoft.com>
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

#include <QFileInfo>
#include <QStringList>
#include <phonon/mediaobject.h>
#include <phonon/path.h>
#include <phonon/audiooutput.h>
#include <phonon/mediasource.h>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>

Player::Player()
{
    m_phononPlayer = new Phonon::MediaObject();
    m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory);
    Phonon::createPath(m_phononPlayer,
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
}

Player::~Player()
{
}

void Player::setTrack(const QString fileName, bool autoPlay)
{
    if (QFileInfo(fileName).isFile()) {
        m_phononPlayer->setCurrentSource(Phonon::MediaSource(fileName));
    }

    emit trackChanged(currentMetaData());
    if (autoPlay==true) {
        play();
    }
}

void Player::setRandomMode(bool randomMode)
{
    if (m_randomMode != randomMode) {
        m_randomMode = randomMode;
        emit randomModeChanged(randomMode);
    }
}

void Player::setRepeatMode(RepeatMode repeatMode)
{
    if (m_repeatMode != repeatMode) {
        m_repeatMode = repeatMode;
        emit repeatModeChanged(repeatMode);
    }
}

void Player::pause()
{
    m_phononPlayer->pause();
    emit trackPaused((m_phononPlayer->state() == Phonon::PausedState));
}

Player::MetaData Player::currentMetaData()
{
    Player::MetaData data;

    QString filename = m_phononPlayer->currentSource().fileName();

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
    emit trackChanged(MetaData());
}

void Player::emitFinished()
{
    // Emit both signals - with parameter and without!!!!
    emit trackFinished(currentMetaData());
    emit trackFinished();
}
