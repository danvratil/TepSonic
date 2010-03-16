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

#include "lastfmscrobbler.h"

#include <QObject>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QString>
#include <QWidget>

LastFmScrobbler::LastFmScrobbler()
{
    qDebug() << "LastFmScrobbler Loaded!";
}

LastFmScrobbler::~LastFmScrobbler()
{
    qDebug() << "LastFmScrobbler destroyed";
}

QString LastFmScrobbler::getName()
{
    return "lastfm!";
}

QWidget* LastFmScrobbler::settingsWidget()
{
    qDebug() << "LastFmScrobbler settings widget requested";

    QWidget *settingsWidget;
    _configWidget->setupUi(settingsWidget);
    QSettings settings(QDir::homePath().append("/.tepsonic/lastfmscrobbler.conf"));
    _configWidget->usernameEdit->setText(settings.value("username",QString()).toString());
    _configWidget->passwordEdit->setText(settings.value("passowrd",QString()).toString());

    return settingsWidget;
}

void LastFmScrobbler::trackChanged(QString filename)
{
    qDebug() << "LastFmScrobbler tack changed notification accepted!";
}

void LastFmScrobbler::settingsAccepted()
{
    QSettings settings(QDir::homePath().append("/.tepsonic/lastfmscrobbler.conf"));
    settings.setValue("username",_configWidget->usernameEdit->text());
    settings.setValue("password",_configWidget->passwordEdit->text());
}

#include "moc_lastfmscrobbler.cpp"

Q_EXPORT_PLUGIN2(tepsonic_lastfmscrobbler, LastFmScrobbler)
