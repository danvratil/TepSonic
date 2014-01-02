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

#ifndef TEPSONIC_PLAYER_H
#define TEPSONIC_PLAYER_H

#include <QObject>

#include <phonon/mediasource.h>
#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>

#include <core/metadata.h>

#include "tepsonic-core-export.h"

namespace TepSonic
{

class TEPSONIC_CORE_EXPORT Player: public QObject
{
    Q_OBJECT
    Q_ENUMS(RepeatMode)
    Q_PROPERTY(Player::RepeatMode repeatMode
               READ repeatMode
               WRITE setRepeatMode)
    Q_PROPERTY(bool randomMode
               READ randomMode
               WRITE setRandomMode)
    Q_PROPERTY(MetaData metadata
               READ currentMetaData)

  public:
    //! Enumeration of repeat modes.
    enum RepeatMode { RepeatOff = 0,    /*! < Disable repeat */
                      RepeatTrack = 1,  /*! < Repeat current track */
                      RepeatAll = 2     /*! < Repeat whole playlist */
                    };

    static Player *instance();

    ~Player();

    MetaData currentMetaData() const;

    RepeatMode repeatMode() const;
    bool randomMode() const;
    Phonon::MediaSource currentSource() const;
    Phonon::State playerState() const;
    QString errorString() const;
    Phonon::MediaObject *mediaObject() const;
    Phonon::AudioOutput *audioOutput() const;
    QList<Phonon::Effect *> effects() const;

  public Q_SLOTS:
    void setRepeatMode(Player::RepeatMode repeatMode);
    void setRandomMode(bool randomMode);
    void play();
    void pause();
    void stop();

    void setTrack(const QString &fileName, bool autoPlay = false);

    void setDefaultOutputDevice();

    void enableEffect(Phonon::Effect *effect, bool enable);

  Q_SIGNALS:
    void repeatModeChanged(Player::RepeatMode repeatMode);
    void randomModeChanged(bool randomMode);
    void trackChanged(const MetaData &metadata);
    void trackFinished(const MetaData &metadata);
    void trackFinished();
    void stateChanged(Phonon::State newState, Phonon::State oldState);
    void trackPositionChanged(qint64 newPos);
    void trackPaused(bool paused);

  private Q_SLOTS:
    void emitFinished();

  private:
    Player();

    class Private;
    Private * const d;
};

} // namespace TepSonic

#endif // TEPSONIC_PLAYER_H
