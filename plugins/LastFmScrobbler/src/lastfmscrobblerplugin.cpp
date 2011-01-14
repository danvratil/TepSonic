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

#include "lastfmscrobblerplugin.h"
#include "constants.h"

#include "lastfmlib/lastfmscrobbler.h"
#include "lastfmlib/submissioninfo.h"

#include <QObject>
#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QString>
#include <QWidget>
#include <QTranslator>

// Exports pluginName method
#ifdef Q_WS_WIN
#define NAME_EXPORT __declspec(dllexport)
#define ID_EXPORT __declspec(dllexport)
#else
#define NAME_EXPORT
#define ID_EXPORT
#endif

extern "C" NAME_EXPORT QString pluginName()
{
    return "Last.fm plugin";
}

extern "C" ID_EXPORT QString pluginID()
{
    return "lastfm";
}


LastFmScrobblerPlugin::LastFmScrobblerPlugin()
{
    setHasConfigUI(true);

    QString locale = QLocale::system().name();
    _translator = new QTranslator(this);
#ifndef APPLEBUNDLE
    // standard unix/windows
    QString dataDir = QLatin1String(PKGDATADIR);
    QString localeDir = dataDir + QDir::separator() + "tepsonic" + QDir::separator() +  "locale" + QDir::separator() + "lastfmscrobbler";
#else
    // mac's bundle. Special stuff again.
    QString localeDir = QCoreApplication::applicationDirPath() + "/../Resources/lastfmscrobbler";
#endif

    _translator->load("lastfmscrobbler_"+locale,localeDir);
    qApp->installTranslator(_translator);

    _scrobbler = NULL;
}

void LastFmScrobblerPlugin::init()
{
    QSettings settings(QString(_CONFIGDIR) + QDir::separator() + "lastfmscrobbler.conf",QSettings::IniFormat,this);

    // Login
    _scrobbler = new LastFmScrobbler(QString("tps").toStdString(),
                                     QString("1.0").toStdString(),
                                     settings.value("username","").toString().toStdString(),
                                     settings.value("password","").toString().toStdString(),
                                     false,
                                     false);

    settings.beginGroup("Proxy");
    if (settings.value("enabled",false).toBool()) {
        _scrobbler->setProxy(settings.value("server",QString()).toString().toStdString(),
                             settings.value("port",0).toUInt(),
                             settings.value("username",QString()).toString().toStdString(),
                             settings.value("password",QString()).toString().toStdString());
    }
    settings.endGroup();

     _scrobbler->authenticate();
}

void LastFmScrobblerPlugin::quit()
{
    _scrobbler->finishedPlaying();

    delete _scrobbler;
}

void LastFmScrobblerPlugin::settingsWidget(QWidget *parentWidget)
{
    _configWidget = new Ui::LastFmScrobblerConfig();
    _configWidget->setupUi(parentWidget);

    QSettings settings(QString(_CONFIGDIR) + QDir::separator() + "lastfmscrobbler.conf",QSettings::IniFormat,this);
    _configWidget->usernameEdit->setText(settings.value("username",QString()).toString());
    _configWidget->passwordEdit->setText(settings.value("password",QString()).toString());
    settings.beginGroup("Proxy");
    _configWidget->useProxyCheckBox->setChecked(settings.value("enabled",false).toBool());
    _configWidget->proxyServerEdit->setText(settings.value("server",QString()).toString());
    _configWidget->proxyPortEdit->setText(settings.value("port",0).toString());
    _configWidget->proxyUsernameEdit->setText(settings.value("username",QString()).toString());
    _configWidget->proxyPasswordEdit->setText(settings.value("password",QString()).toString());
    settings.endGroup();

    _configWidget->proxyGroup->setEnabled( _configWidget->useProxyCheckBox->isChecked() );
    connect(_configWidget->useProxyCheckBox,SIGNAL(toggled(bool)),
            _configWidget->proxyGroup,SLOT(setEnabled(bool)));
}

void LastFmScrobblerPlugin::trackFinished(Player::MetaData trackdata)
{
    _scrobbler->finishedPlaying();
}

void LastFmScrobblerPlugin::trackChanged(Player::MetaData trackData)
{
    SubmissionInfo info = SubmissionInfo(trackData.artist.toStdString(),
                                         trackData.title.toStdString());
    // The length is in milliseconds, lastfm accepts seconds!
    info.setTrackLength(trackData.length/1000);
    info.setTrackNr(trackData.trackNumber);

    _scrobbler->startedPlaying(info);
}

void LastFmScrobblerPlugin::trackPaused(bool paused)
{
    _scrobbler->pausePlaying(paused);
}

void LastFmScrobblerPlugin::settingsAccepted()
{
    QSettings settings(_CONFIGDIR + QDir::separator() + "lastfmscrobbler.conf",QSettings::IniFormat,this);
    settings.setValue("username",_configWidget->usernameEdit->text());
    settings.setValue("password",_configWidget->passwordEdit->text());
    settings.beginGroup("Proxy");
    settings.setValue("enabled",_configWidget->useProxyCheckBox->isChecked());
    settings.setValue("server",_configWidget->proxyServerEdit->text());
    settings.setValue("port",_configWidget->proxyPortEdit->text().toUInt());
    settings.setValue("username",_configWidget->proxyUsernameEdit->text());
    settings.setValue("password",_configWidget->proxyPasswordEdit->text());
    settings.endGroup();


    // Reinitialize the scrobbler with new credentials
    delete _scrobbler;
    _scrobbler = new LastFmScrobbler(_configWidget->usernameEdit->text().toStdString(),
                                     _configWidget->passwordEdit->text().toStdString(),
                                     false,
                                     false);

    if (_configWidget->useProxyCheckBox->isChecked()) {
        _scrobbler->setProxy(_configWidget->proxyServerEdit->text().toStdString(),
                             _configWidget->proxyPortEdit->text().toUInt(),
                             _configWidget->proxyUsernameEdit->text().toStdString(),
                             _configWidget->proxyPasswordEdit->text().toStdString());
    }

    _scrobbler->authenticate();

}

Q_EXPORT_PLUGIN2(tepsonic_lastfmscrobbler, LastFmScrobblerPlugin)
