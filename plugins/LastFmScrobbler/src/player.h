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
#include <phonon/mediasource.h>
#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>

//! Player class provides simplified API to control the playback
/*!
  Player class initializes PhononPlayer. reads, checks and provides access to many player-related variables
  and properties.
  It provides large API of signals&slots and also direct function calls for controlling the playback.
*/
class Player: public QObject
{
    Q_OBJECT
    Q_ENUMS(RepeatMode)
    Q_PROPERTY(Player::RepeatMode repeatMode
               READ repeatMode
               WRITE setRepeatMode)
    Q_PROPERTY(bool randomMode
               READ randomMode
               WRITE setRandomMode)
    Q_PROPERTY(Player::MetaData metadata
               READ currentMetaData)
    Q_PROPERTY(Phonon::State playerState
               READ playerState)
public:
    //! Structure to pass track's metadata
    struct MetaData {
        QString filename;  /*! < Name of file */
        QString artist;    /*! < Track artist */
        QString title;     /*! < Track title */
        QString album;     /*! < Album name */
        qint64 length;     /*! < Length of song in milliseconds */
        int trackNumber;   /*! < Track number */
    };

    //! Enumeration of repeat modes.
    enum RepeatMode { RepeatOff = 0,    /*! < Disable repeat */
                      RepeatTrack = 1,  /*! < Repeat current track */
                      RepeatAll = 2     /*! < Repeat whole playlist */
                    };

    //! Constructor
    Player();

    //! Destructor
    ~Player();

    //! Returns current track metadata
    /*!
      \return Returns meta data of current track
      \sa trackChanged(), trackFinished()
    */
    Player::MetaData currentMetaData();

    //! Returns current repeat mode
    /*!
      \return Returns current player repeat mode
      \sa setRepeatMode(), repeatModeChanged()
    */
    RepeatMode repeatMode() {
        return _repeatMode;
    }

    //! Returns current random mode state
    /*!
      \return Returns true when random mode is enabled, false when it's disabled
      \sa setRandomMode(), randomModeChanged()
    */
    bool randomMode() {
        return _randomMode;
    }

    //! Returns current media source
    /*!
      \return Returns current player media source
      \sa setTrack()
    */
    Phonon::MediaSource currentSource() {
        return _phononPlayer->currentSource();
    }

    //! Returns current player state
    /*!
      \return Returns current player state
      \sa stateChanged()
    */
    Phonon::State playerState() {
        return _phononPlayer->state();
    }

    //! Returns players error string
    /*!
      \return Returns description of a player failure
    */
    QString errorString() {
        return _phononPlayer->errorString();
    }

    //! Returns pointer to media object
    /*!
      \return Returns pointer to player Phonon MediaObject object
    */
    Phonon::MediaObject* mediaObject() {
        return _phononPlayer;
    }

    //! Returns pointer to audio object
    /*!
      \return Returns pointer to player Phonon AudioOutput object
    */
    Phonon::AudioOutput* audioOutput() {
        return _audioOutput;
    }

private:
    //! Current repeat mode
    RepeatMode _repeatMode;

    //! Current random mode
    bool _randomMode;

    //! Phonon MediaObject
    Phonon::MediaObject *_phononPlayer;

    //! Phonon AudioOutput
    Phonon::AudioOutput *_audioOutput;

private slots:
    //! When a track is finished calls currentSource() and then emits trackFinished(MetaData) and trackFinished() signals
    /*!
      \sa trackFinished(), trackFinished(MetaData)
    */
    void emitFinished();

    //! When a track is changed calls currentSource() and then emits trackChanged(MetaData)
    /*!
      \sa trackChanged()
    */
    void emitTrackChanged();

public slots:
    //! Changes repeat mode to \p repeatMode
    /*!
      \param repeatMode new repeat mode
    */
    void setRepeatMode(RepeatMode repeatMode);

    //! Changes random mode to \p randoMode
    /*!
      \param randomMode new random mode state
    */
    void setRandomMode(bool randomMode);

    //! Starts playback
    /*!
      \sa stateChanged()
    */
    void play() {
        _phononPlayer->play();
    }

    //! Pauses playback
    /*!
      \sa stateChanged()
    */
    void pause() {
        _phononPlayer->pause();
    }

    //! Stops playback and removes current MediaSource
    /*!
      \sa stateChanged()
    */
    void stop();

    //! Sets new track
    /*!
      \param fileName name of new media file to play
      \param autoPlay when true, playback will automatically start after new track is loaded
    */
    void setTrack(const QString fileName, bool autoPlay = false);

signals:
    //! Informs about new repeat mode
    /*!
      \param repeatMode new repeat mode
      \sa repeatMode()
    */
    void repeatModeChanged(Player::RepeatMode repeatMode);

    //! Informs about new random mode
    /*!
      \param randomMode new random mode
      \sa randomMode()
    */
    void randomModeChanged(bool randomMode);

    //! Informs that a new track was set and provides it's metadata
    /*!
      \param metadata meta data of the new track
      \sa setTrack()
    */
    void trackChanged(Player::MetaData metadata);

    //! Informs that end of the current track was reached and provides it's metadata
    /*!
      \param metadata meta data of the just finished track
    */
    void trackFinished(Player::MetaData metadata);

    //! Informs that end of the current track was reached
    void trackFinished();

    //! Informs that player's state has changed and provides informations about new and previous state
    /*!
      \param newState new player state
      \param oldState previous player state
      \sa play(), pause(), stop()
    */
    void stateChanged(Phonon::State newState, Phonon::State oldState);

    //! Ticks every 1 second and informs about current position in track
    /*!
      \param newPos position in track in milliseconds
    */
    void trackPositionChanged(qint64 newPos);

};

#endif // PLAYER_H
