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
#include <Phonon/MediaObject>
#include <Phonon/Path>
#include <Phonon/AudioOutput>
#include <Phonon/MediaSource>

Player::Player()
{
    _phononPlayer = new Phonon::MediaObject();
    _audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory);
    Phonon::createPath(_phononPlayer,_audioOutput);

    // By default tick every 1 second
    _phononPlayer->setTickInterval(1000);

    connect(_phononPlayer,SIGNAL(finished()),this,SLOT(emitFinished()));
    connect(_phononPlayer,SIGNAL(stateChanged(Phonon::State,Phonon::State)),this,SIGNAL(stateChanged(Phonon::State,Phonon::State)));
    connect(_phononPlayer,SIGNAL(tick(qint64)),this,SIGNAL(trackPositionChanged(qint64)));
    connect(_phononPlayer,SIGNAL(metaDataChanged()),this,SLOT(emitTrackChanged()));

    _randomMode = false;
    _repeatMode = RepeatOff;
}

Player::~Player()
{
}

void Player::setTrack(const QString fileName)
{
    if (QFileInfo(fileName).isFile()) {
       _phononPlayer->setCurrentSource(Phonon::MediaSource(fileName));
   }
   //emit trackChanged(currentMetaData());
}

void Player::setTrack(const QString fileName, bool autoPlay)
{
    setTrack(fileName);
    if (autoPlay==true) {
        _phononPlayer->play();
    }
}

void Player::setRandomMode(bool randomMode)
{
    if (_randomMode != randomMode) {
        _randomMode = randomMode;
        emit randomModeChanged(randomMode);
    }
}

void Player::setRepeatMode(RepeatMode repeatMode)
{
    if (_repeatMode != repeatMode) {
        _repeatMode = repeatMode;
        emit repeatModeChanged(repeatMode);
    }
}

MetaData Player::currentMetaData()
{
    MetaData data;

    if (_phononPlayer->currentSource().type()==Phonon::MediaSource::Invalid) return data;

    data.filename = _phononPlayer->currentSource().fileName();
    data.artist = _phononPlayer->metaData(Phonon::ArtistMetaData).join(QString());
    data.album = _phononPlayer->metaData(Phonon::AlbumMetaData).join(QString());
    data.length = (qint64)_phononPlayer->totalTime(); //msecs
    data.title = _phononPlayer->metaData(Phonon::TitleMetaData).join(QString());
    data.trackNumber = _phononPlayer->metaData(Phonon::TracknumberMetaData).join(QString()).toInt();

    return data;
}

void Player::stop()
{
    _phononPlayer->stop();
    // Empty the source
    _phononPlayer->setCurrentSource(Phonon::MediaSource());
}

void Player::emitFinished()
{
    emit trackFinished();
    emit trackFinished(currentMetaData());
}

void Player::emitTrackChanged()
{
    emit trackChanged(currentMetaData());
}


