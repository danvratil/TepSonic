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

#include <QtCore/QObject>
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

  public:
    //! Structure to pass track's metadata
    struct MetaData {
        QString filename;  /*! < Name of file */
        QString artist;    /*! < Track artist */
        QString title;     /*! < Track title */
        QString album;     /*! < Album name */
        QString genre;     /*! < Genre string */
        qint64 length;     /*! < Length of song in milliseconds */
        QString formattedLength; /* ! < Formatted time (mm:ss) */
        int trackNumber;   /*! < Track number */
        uint year;         /*! < Year of release */
        int bitrate;      /*! < bitrate */

    };

    //! Enumeration of repeat modes.
    enum RepeatMode { RepeatOff = 0,    /*! < Disable repeat */
                      RepeatTrack = 1,  /*! < Repeat current track */
                      RepeatAll = 2     /*! < Repeat whole playlist */
                    };

    static Player *instance();

    //! Destructor
    ~Player();

    //! Returns current track metadata
    /*!
      \return Returns meta data of current track
      \sa trackChanged(), trackFinished()
    */
    Player::MetaData currentMetaData() const;

    //! Returns current repeat mode
    /*!
      \return Returns current player repeat mode
      \sa setRepeatMode(), repeatModeChanged()
    */
    RepeatMode repeatMode() const {
        return m_repeatMode;
    }

    //! Returns current random mode state
    /*!
      \return Returns true when random mode is enabled, false when it's disabled
      \sa setRandomMode(), randomModeChanged()
    */
    bool randomMode() const {
        return m_randomMode;
    }

    //! Returns current media source
    /*!
      \return Returns current player media source
      \sa setTrack()
    */
    Phonon::MediaSource currentSource() const {
        return m_phononPlayer->currentSource();
    }

    //! Returns current player state
    /*!
      \return Returns current player state
      \sa stateChanged()
    */
    Phonon::State playerState() const {
        return m_phononPlayer->state();
    }

    //! Returns players error string
    /*!
      \return Returns description of a player failure
    */
    QString errorString() const {
        return m_phononPlayer->errorString();
    }

    //! Returns pointer to media object
    /*!
      \return Returns pointer to player Phonon MediaObject object
    */
    Phonon::MediaObject *mediaObject() const {
        return m_phononPlayer;
    }

    //! Returns pointer to audio object
    /*!
      \return Returns pointer to player Phonon AudioOutput object
    */
    Phonon::AudioOutput *audioOutput() const {
        return m_audioOutput;
    }

    //! Returns pointer to list of available effects
    QList<Phonon::Effect *> effects() const {
        return m_effects;
    }

  private:
    explicit Player();
    static Player *s_instance;

    //! Current repeat mode
    RepeatMode m_repeatMode;

    //! Current random mode
    bool m_randomMode;

    //! Phonon MediaObject
    Phonon::MediaObject *m_phononPlayer;

    //! Phonon AudioOutput
    Phonon::AudioOutput *m_audioOutput;

    //! Phonon Path
    Phonon::Path m_phononPath;

    QList<Phonon::Effect *> m_effects;

    //! Loads all effects to m_effects and installs chosen effects according to configuration
    void loadEffects();

  private Q_SLOTS:
    //! When a track is finished calls currentSource() and then emits trackFinished(MetaData) and trackFinished() signals
    /*!
      \sa trackFinished(), trackFinished(MetaData)
    */
    void emitFinished();

  public Q_SLOTS:
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
        m_phononPlayer->play();
    }

    //! Pauses playback
    /*!
      \sa stateChanged()
    */
    void pause();

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
    void setTrack(const QString &fileName, bool autoPlay = false);

    //! Sets default output device
    /*! Default output device is defined as "OutputDevice" in settings file.
     */
    void setDefaultOutputDevice();

    //! Install or uninstall effect from Phonon. When the effect is already loaded, do nothing.
    void enableEffect(Phonon::Effect *effect, bool enable);

  Q_SIGNALS:
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
    void trackChanged(const Player::MetaData &metadata);

    //! Informs that end of the current track was reached and provides it's metadata
    /*!
      \param metadata meta data of the just finished track
    */
    void trackFinished(const Player::MetaData &metadata);

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

    //! Informs that the playback has been (un)paused
    /*!
      Passes true when the playback was paused and false when unpaused
      \param paused paused (true) or unpaused (false)
    */
    void trackPaused(bool paused);

};

#endif // PLAYER_H
