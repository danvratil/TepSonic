/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <me@dvratil.cz>
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

#ifndef MPRISMEDIAPLAYER_H
#define MPRISMEDIAPLAYER_H

#include "dbusabstractadaptor.h"
#include <QtCore/QStringList>

class MPRISMediaPlayer2 : public DBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")

    Q_PROPERTY(bool CanQuit READ canQuit)
    Q_PROPERTY(bool Fullscreen READ fullscreen)
    Q_PROPERTY(bool CanSetFullscreen READ canSetFullscreen)
    Q_PROPERTY(bool CanRaise READ canRaise)
    Q_PROPERTY(bool HasTrackList READ hasTrackList)
    Q_PROPERTY(QString Identity READ identity)
    Q_PROPERTY(QStringList SupportedUriSchemes READ supportedUriSchemes)
    Q_PROPERTY(QStringList SupportedMimeTypes READ supportedMimeTypes)

  public:
    explicit MPRISMediaPlayer2(QObject *parent);
    virtual ~MPRISMediaPlayer2();

    bool canQuit() const;
    bool canRaise() const;
    bool hasTrackList() const;
    QString identity() const;
    QStringList supportedUriSchemes() const;
    QStringList supportedMimeTypes() const;

    bool canSetFullscreen() const;
    bool fullscreen() const;

  public Q_SLOTS:
    void Raise();
    void Quit();

};

#endif // MPRISMEDIAPLAYER_H
