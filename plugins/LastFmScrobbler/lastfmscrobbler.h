/*
 * TEPSONIC
 * Copyright 2010 Dan Vratil <vratil@progdansoft.com>
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

#ifndef LASTFMSCROBBLER_H
#define LASTFMSCROBBLER_H

#include "abstractplugin.h"

#include "ui_lastfmscrobblerconfig.h"

#include <QObject>
#include <QWidget>
#include <QString>

class LastFmScrobbler : public AbstractPlugin
{
    Q_OBJECT
    public:
        LastFmScrobbler();
        ~LastFmScrobbler();
        const QWidget* settingsWidget();
        QString getName();

    public slots:
        void settingsAccepted();
        void trackChanged(QString filename);
        //void playerStatusChanged(Phonon::State newState, Phonon::State oldState) { }

    private:
        Ui::LastFmScrobblerConfig *_configWidget;
};

#endif // LASTFMSCROBBLER_H
