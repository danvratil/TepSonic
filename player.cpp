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
#include <Phonon/MediaObject>
#include <Phonon/Path>
#include <Phonon/AudioOutput>
#include <Phonon/MediaSource>

Player::Player()
{
    _phononPlayer = new Phonon::MediaObject();
    _audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory);
    Phonon::createPath(_phononPlayer,_audioOutput);

    connect(_phononPlayer,SIGNAL(finished()),this,SIGNAL(finished()));
    connect(_phononPlayer,SIGNAL(stateChanged(Phonon::State,Phonon::State)),this,SIGNAL(stateChanged(Phonon::State,Phonon::State)));

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
   emit trackChanged(fileName);
}

void Player::setTrack(const QString fileName, bool autoPlay)
{
    setTrack(fileName);
    if (autoPlay==true) {
        _phononPlayer->play();
    }
    emit trackChanged(fileName);
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
    /*data.artist = phononPlayer->metaData("ARTIST").
    data.album = phononPlayer->metaData("ALBUM").at(0);
    data.length = (int)phononPlayer->totalTime()/1000;
    data.title = phononPlayer->metaData("TITLE").at(0);
    data.trackNumber = phononPlayer->metaData("TRACKNUMBER").at(0).toInt();*/

    return data;
}

void Player::stop()
{
    _phononPlayer->stop();
    // Empty the source
    _phononPlayer->setCurrentSource(Phonon::MediaSource());
}



