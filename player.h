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
    QString artist;
    QString title;
    QString album;
    int length;
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
    Phonon::MediaObject *phononPlayer;
    Phonon::AudioOutput *audioOutput;
    void setTrack(const QString fileName);
    void setTrack(const QString fileName, bool autoPlay);
    RepeatMode repeatMode() { return _repeatMode; }
    bool randomMode() { return _randomMode; }
    MetaData currentMetaData();


  private:
    RepeatMode _repeatMode;
    bool _randomMode;

 public slots:
    void setRepeatMode(RepeatMode);
    void setRandomMode(bool);

 signals:
    void repeatModeChanged(RepeatMode repeatMode);
    void randomModeChanged(bool randomMode);
    void trackChanged(QString filename);

};

#endif // PLAYER_H
