/*
 * TEPSONIC
 * Copyright 2013 Dan Vrátil <dan@progdan.cz>
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

#include "mprismediaplayer2player.h"
#include "player.h"

#include <QtCore/QCryptographicHash>

#include <QtDBus/QDBusObjectPath>

MPRISMediaPlayer2Player::MPRISMediaPlayer2Player(QObject* parent):
    DBusAbstractAdaptor(parent)
{
    connect(Player::instance(), SIGNAL(randomModeChanged(bool)),
            this, SLOT(playerRandomModeChanged()));
    connect(Player::instance(), SIGNAL(repeatModeChanged(Player::RepeatMode)),
            this, SLOT(playerRepeatModeChanged()));
    connect(Player::instance(), SIGNAL(trackChanged(MetaData)),
            this, SLOT(playerTrackChanged()));
    connect(Player::instance()->audioOutput(), SIGNAL(volumeChanged(qreal)),
            this, SLOT(playerVolumeChanged()));
    connect(Player::instance(), SIGNAL(trackPositionChanged(qint64)),
            this, SLOT(playerPositionChanged()));
    connect(Player::instance(), SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(playerStateChanged()));
}

MPRISMediaPlayer2Player::~MPRISMediaPlayer2Player()
{
}

void MPRISMediaPlayer2Player::playerPositionChanged()
{
    signalPropertyChange(QLatin1String("Position"), position());
}

void MPRISMediaPlayer2Player::playerRandomModeChanged()
{
    signalPropertyChange(QLatin1String("Shuffle"), shuffle());
}

void MPRISMediaPlayer2Player::playerRepeatModeChanged()
{
    signalPropertyChange(QLatin1String("Loop"), loopStatus());
}

void MPRISMediaPlayer2Player::playerTrackChanged()
{
    signalPropertyChange(QLatin1String("Metadata"), metadata());
}

void MPRISMediaPlayer2Player::playerVolumeChanged()
{
    signalPropertyChange(QLatin1String("Volume"), volume());
}

void MPRISMediaPlayer2Player::playerStateChanged()
{
    signalPropertyChange(QLatin1String("PlaybackStatus"), playbackStatus());
}

QString MPRISMediaPlayer2Player::playbackStatus() const
{
    switch (Player::instance()->playerState()) {
        case Phonon::PlayingState:
            return QLatin1String("Playing");
        case Phonon::PausedState:
            return QLatin1String("Paused");
        case Phonon::StoppedState:
            return QLatin1String("Stopped");
        default:
            return QString();
    }
}

QString MPRISMediaPlayer2Player::loopStatus() const
{
    switch (Player::instance()->repeatMode()) {
        case Player::RepeatAll:
            return QLatin1String("Playlist");
        case Player::RepeatTrack:
            return QLatin1String("Track");
        case Player::RepeatOff:
            return QLatin1String("None");
    }

    return QString();
}

void MPRISMediaPlayer2Player::setLoopStatus(const QString& loopStatus)
{
    if (loopStatus == QLatin1String("Playlist")) {
        Player::instance()->setRepeatMode(Player::RepeatAll);
    } else if (loopStatus == QLatin1String("Track")) {
        Player::instance()->setRepeatMode(Player::RepeatTrack);
    } else if (loopStatus == QLatin1String("None")) {
        Player::instance()->setRepeatMode(Player::RepeatOff);
    }
}

double MPRISMediaPlayer2Player::rate() const
{
    return 1.0;
}

void MPRISMediaPlayer2Player::setRate(double rate)
{
    Q_UNUSED(rate);
}

double MPRISMediaPlayer2Player::minimumRate() const
{
    return 1.0;
}

double MPRISMediaPlayer2Player::maximumRate() const
{
    return 1.0;
}

bool MPRISMediaPlayer2Player::shuffle() const
{
    return Player::instance()->randomMode();
}

void MPRISMediaPlayer2Player::setShuffle(bool shuffle)
{
    Player::instance()->setRandomMode(shuffle);
}

static QDBusObjectPath trackPath(const QString &path)
{
    return QDBusObjectPath(QString::fromLatin1("/org/tepsonic/track/%1")
        .arg(QString::fromLatin1(QCryptographicHash::hash(path.toLatin1(), QCryptographicHash::Md5).toHex())));
}

QVariantMap MPRISMediaPlayer2Player::metadata() const
{
    const MetaData metaData = Player::instance()->currentMetaData();

    QVariantMap map;
    map[QLatin1String("mpris:trackId")] = QVariant::fromValue<QDBusObjectPath>(trackPath(metaData.fileName()));
    map[QLatin1String("xesam:album")] = metaData.album();
    map[QLatin1String("xesam:albumArtist")] = metaData.artist();
    map[QLatin1String("xesam:artist")] = metaData.artist();
    map[QLatin1String("xesam:genre")] = metaData.genre();
    map[QLatin1String("xesam:length")] = metaData.length();
    map[QLatin1String("xesam:title")] = metaData.title();
    map[QLatin1String("xesam:trackNumber")] = metaData.trackNumber();
    map[QLatin1String("xesam:url")] = QString::fromLatin1(QUrl::toPercentEncoding(metaData.fileName()));

    return map;
}

double MPRISMediaPlayer2Player::volume() const
{
    return Player::instance()->audioOutput()->volume();
}

void MPRISMediaPlayer2Player::setVolume(double volume)
{
    Player::instance()->audioOutput()->setVolume(volume);
}

qlonglong MPRISMediaPlayer2Player::position() const
{
    return Player::instance()->mediaObject()->currentTime();
}

bool MPRISMediaPlayer2Player::canGoNext() const
{
    return true;
}

bool MPRISMediaPlayer2Player::canGoPrevious() const
{
    return true;
}

bool MPRISMediaPlayer2Player::canPause() const
{
    return Player::instance()->playerState() == Phonon::PlayingState;
}

bool MPRISMediaPlayer2Player::canPlay() const
{
    return Player::instance()->playerState() != Phonon::PlayingState;
}

bool MPRISMediaPlayer2Player::canSeek() const
{
    return true;
}

bool MPRISMediaPlayer2Player::canControl() const
{
    return true;
}

void MPRISMediaPlayer2Player::Next()
{
}

void MPRISMediaPlayer2Player::Previous()
{
}

void MPRISMediaPlayer2Player::Pause()
{
    Player::instance()->pause();
}

void MPRISMediaPlayer2Player::Play()
{
    Player::instance()->play();
}

void MPRISMediaPlayer2Player::PlayPause()
{
    if (Player::instance()->playerState() == Phonon::PlayingState) {
        Player::instance()->pause();
    } else if (Player::instance()->playerState() == Phonon::PausedState) {
        Player::instance()->play();
    }
}

void MPRISMediaPlayer2Player::Seek(qlonglong offset)
{
    Player::instance()->mediaObject()->seek(offset);
}

void MPRISMediaPlayer2Player::SetPosition(const QDBusObjectPath& track,
                                          qlonglong position)
{
    const QString filename = Player::instance()->currentSource().fileName();
    const QString path = QString::fromLatin1("/org/tepsonic/track/%1")
        .arg(QString::fromLatin1(QCryptographicHash::hash(filename.toLatin1(), QCryptographicHash::Md5).toHex()));
    if (track.path() != path) {
        return;
    }

    Seek(position);
}

void MPRISMediaPlayer2Player::OpenUri(const QString& uri)
{
    Q_UNUSED(uri);
}

void MPRISMediaPlayer2Player::Stop()
{
    Player::instance()->stop();
}
