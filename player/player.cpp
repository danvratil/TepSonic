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
#include <phonon/MediaObject>
#include <phonon/Path>
#include <phonon/AudioOutput>
#include <phonon/MediaSource>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>

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
    connect(_phononPlayer,SIGNAL(currentSourceChanged(Phonon::MediaSource)),this,SLOT(emitTrackChanged()));

    _randomMode = false;
    _repeatMode = RepeatOff;
}

Player::~Player()
{
}

void Player::setTrack(const QString fileName, bool autoPlay)
{
    if (QFileInfo(fileName).isFile()) {
        _phononPlayer->setCurrentSource(Phonon::MediaSource(fileName));
    }

    emit trackChanged(currentMetaData());
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

Player::MetaData Player::currentMetaData()
{
    Player::MetaData data;

    QString filename = _phononPlayer->currentSource().fileName();

    if ((!QFileInfo(filename).exists()) ||
            (_phononPlayer->currentSource().type()==Phonon::MediaSource::Invalid)) {
        return data;
    }

    TagLib::FileRef f(filename.toUtf8().constData());
    data.artist = f.tag()->artist().toCString();
    data.title = f.tag()->title().toCString();
    data.album = f.tag()->album().toCString();
    data.trackNumber = f.tag()->track();
    // Length in msecs
    data.length = f.audioProperties()->length()*1000;
    data.filename = filename;

    return data;
}

void Player::stop()
{
    _phononPlayer->stop();
    // Empty the source
    _phononPlayer->setCurrentSource(Phonon::MediaSource());
    emit trackChanged(MetaData());
}

void Player::emitFinished()
{
    emit trackFinished(currentMetaData());
    emit trackFinished();
}

void Player::emitTrackChanged()
{
    emit trackChanged(currentMetaData());
}


