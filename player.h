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


#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <Phonon/MediaSource>
#include <Phonon/AudioOutput>
#include <Phonon/MediaObject>

struct MetaData {
    QString filename;
    QString artist;
    QString title;
    QString album;
    qint64 length;
    int trackNumber;
};

class Player: public QObject
{
    Q_OBJECT
    Q_ENUMS(RepeatMode)

  public:
    enum RepeatMode { RepeatOff, RepeatTrack, RepeatAll };
    Player();
    ~Player();

    MetaData currentMetaData();
    RepeatMode repeatMode() { return _repeatMode; }
    bool randomMode() { return _randomMode; }
    Phonon::MediaSource currentSource() { return _phononPlayer->currentSource(); }
    Phonon::State playerState() { return _phononPlayer->state(); }
    QString errorString() { return _phononPlayer->errorString(); }
    Phonon::MediaObject* mediaObject() { return _phononPlayer; }
    Phonon::AudioOutput* audioOutput() { return _audioOutput; }

  private:
    RepeatMode _repeatMode;
    bool _randomMode;
    Phonon::MediaObject *_phononPlayer;
    Phonon::AudioOutput *_audioOutput;

 private slots:
    void emitFinished();
    void emitTrackChanged();

 public slots:
    void setRepeatMode(RepeatMode);
    void setRandomMode(bool);
    void play() { _phononPlayer->play(); }
    void pause() { _phononPlayer->pause(); }
    void stop();
    void setTrack(const QString fileName);
    void setTrack(const QString fileName, bool autoPlay);

 signals:
    void repeatModeChanged(RepeatMode repeatMode);
    void randomModeChanged(bool randomMode);
    void trackChanged(MetaData metadata);
    void trackFinished(MetaData metadata);
    void trackFinished();
    void stateChanged(Phonon::State newState, Phonon::State oldState);
    void trackPositionChanged(qint64 newPos);

};

#endif // PLAYER_H
