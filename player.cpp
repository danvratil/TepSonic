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
    phononPlayer = new Phonon::MediaObject();
    phononPlayer = new Phonon::MediaObject();
    audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory);
    Phonon::createPath(phononPlayer,audioOutput);

    _randomMode = false;
    _repeatMode = RepeatOff;
}

Player::~Player()
{
}

void Player::setTrack(const QString fileName)
{
    if (QFileInfo(fileName).isFile()) {
       phononPlayer->setCurrentSource(Phonon::MediaSource(fileName));
   }
   emit trackChanged(fileName);
}

void Player::setTrack(const QString fileName, bool autoPlay)
{
    setTrack(fileName);
    if (autoPlay==true) {
        phononPlayer->play();
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
    data.artist = phononPlayer->metaData("ARTIST");
    data.album = phononPlayer->metaData("ALBUM");
    data.length = (int)phononPlayer->totalTime()/1000;
    data.title = phononPlayer->metaData("TITLE");
    data.trackNumber = QString(phononPlayer->metaData("TRACKNUMBER")).toInt();

    return data;
}



