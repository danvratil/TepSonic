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
#include "lastfmlib/lastfmexceptions.h"
#include "lastfmlib/lastfmclient.h"
#include "lastfmlib/nowplayinginfo.h"
#include "lastfmlib/submissioninfo.h"

#include <QObject>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QSettings>
#include <QString>
#include <QWidget>
#include <QPushButton>
#include <QFormLayout>
#include <QTranslator>

LastFmScrobblerPlugin::LastFmScrobblerPlugin()
{
    setPluginName("Last.fm plugin");
    setHasConfigUI(true);

    QString locale = QLocale::system().name();
    _translator = new QTranslator(this);
    QString dataDir = QLatin1String(PKGDATADIR);
    QString localeDir = dataDir + QDir::separator() + "tepsonic" + QDir::separator() +  "locale" + QDir::separator() + "lastfmscrobbler";
    _translator->load(locale,localeDir);
    qApp->installTranslator(_translator);

}

void LastFmScrobblerPlugin::init()
{
    _cache.clear();
    qDebug() << "Initialized";

    // Load cache first
    QSettings settings(QString(_CONFIGDIR) + QDir::separator() + "lastfmscrobbler.conf",QSettings::IniFormat,this);
    int size = settings.beginReadArray("Cache");
    for (int i = 0; i < size; i++) {
        settings.setArrayIndex(i);
        LastFmScrobblerPlugin::MetaData metadata;
        metadata.trackInfo.album = settings.value("album").toString();
        metadata.trackInfo.artist = settings.value("artist").toString();
        metadata.trackInfo.filename = settings.value("filename").toString();
        metadata.trackInfo.length = settings.value("length").toUInt();
        metadata.trackInfo.title = settings.value("title").toString();
        metadata.trackInfo.trackNumber = settings.value("trackNumber").toInt();
        metadata.playbackStart = settings.value("playbackStart").toUInt();
        _cache.append(metadata);
     }
     settings.endArray();

     // Login
     _scrobbler = new LastFmScrobbler(QString("tps").toStdString(),
                                      QString("1.0").toStdString(),
                                      settings.value("username","").toString().toStdString(),
                                      settings.value("password","").toString().toStdString(),
                                      false,
                                      false);
     _scrobbler->authenticate();

}

void LastFmScrobblerPlugin::quit()
{
    _scrobbler->finishedPlaying();

    delete _scrobbler;

    saveCache();

    _cache.clear();
}

void LastFmScrobblerPlugin::settingsWidget(QWidget *parentWidget)
{
    _configWidget = new Ui::LastFmScrobblerConfig();
    _configWidget->setupUi(parentWidget);

    QSettings settings(QString(_CONFIGDIR) + QDir::separator() + "lastfmscrobbler.conf",QSettings::IniFormat,this);
    _configWidget->usernameEdit->setText(settings.value("username",QString()).toString());
    _configWidget->passwordEdit->setText(settings.value("password",QString()).toString());
    connect(_configWidget->testLoginButton,SIGNAL(clicked()),this,SLOT(on_testLoginButton_clicked()));
    connect(_configWidget->testLoginButton,SIGNAL(clicked(bool)),_configWidget->testLoginButton,SLOT(setDisabled(bool)));

}

void LastFmScrobblerPlugin::trackFinished(Player::MetaData trackdata)
{
    _scrobbler->finishedPlaying();
}

void LastFmScrobblerPlugin::trackChanged(Player::MetaData trackData)
{
    SubmissionInfo info = SubmissionInfo(trackData.artist.toStdString(),
                                         trackData.title.toStdString());
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
}

void LastFmScrobblerPlugin::on_testLoginButton_clicked()
{
/*    QNetworkAccessManager *testNAM = new QNetworkAccessManager();
    QNetworkRequest nr;
    nr.setUrl(prepareHandshakeURL(_configWidget->usernameEdit->text(),
                                  _configWidget->passwordEdit->text()));
    nr.setRawHeader("Host", "post.audioscrobbler.com");
    connect(testNAM,SIGNAL(finished(QNetworkReply*)),this,SLOT(testLoginFinished(QNetworkReply*)));
    testNAM->get(nr);*/
}

void LastFmScrobblerPlugin::saveCache()
{
    QSettings settings(QString(_CONFIGDIR).append("/lastfmscrobbler.conf"),QSettings::IniFormat,this);
    settings.beginWriteArray("Cache");
    for (int i = 0; i < _cache.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("album", _cache.at(i).trackInfo.album);
        settings.setValue("artist", _cache.at(i).trackInfo.artist);
        settings.setValue("filename", _cache.at(i).trackInfo.filename);
        settings.setValue("length", _cache.at(i).trackInfo.length);
        settings.setValue("title", _cache.at(i).trackInfo.title);
        settings.setValue("trackNumber", _cache.at(i).trackInfo.trackNumber);
        settings.setValue("playbackStart",_cache.at(i).playbackStart);
    }
    settings.endArray();
}

Q_EXPORT_PLUGIN2(tepsonic_lastfmscrobbler, LastFmScrobblerPlugin)


