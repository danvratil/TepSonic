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


# Workaround for CMake not passing QT_NO_DEBUG correctly
#define QT_NO_DEBUG


#include "lastfmscrobbler.h"
#include "constants.h"

#include <QObject>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QLocale>
#include <QSettings>
#include <QString>
#include <QWidget>
#include <QPushButton>
#include <QFormLayout>
#include <QHttpRequestHeader>
#include <QTimer>
#include <QTranslator>

LastFmScrobbler::LastFmScrobbler()
{
    setPluginName("Last.fm plugin");
    setHasConfigUI(true);

    QString locale = QLocale::system().name();
    _translator = new QTranslator(this);
    QString dataDir = QLatin1String(PKGDATADIR);
    QString localeDir = dataDir + QDir::separator() + "locale"+QDir::separator()+"lastfmscrobbler";
    _translator->load(locale,localeDir);
    qApp->installTranslator(_translator);
}

void LastFmScrobbler::init()
{
    _cache.clear();

    // Load cache first
    QSettings settings(QString(_CONFIGDIR).append("/lastfmscrobbler.conf"),QSettings::IniFormat,this);
    int size = settings.beginReadArray("Cache");
    for (int i = 0; i < size; i++) {
        settings.setArrayIndex(i);
        LastFmScrobbler::MetaData metadata;
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

     login();
}

void LastFmScrobbler::quit()
{
    saveCache();

    _token.clear();
    _submissionURL.clear();
    _nowPlayingURL.clear();
    _cache.clear();
}

void LastFmScrobbler::settingsWidget(QWidget *parentWidget)
{
    _configWidget = new Ui::LastFmScrobblerConfig();
    _configWidget->setupUi(parentWidget);

    QSettings settings(QString(_CONFIGDIR).append("/lastfmscrobbler.conf"),QSettings::IniFormat,this);
    _configWidget->usernameEdit->setText(settings.value("username",QString()).toString());
    _configWidget->passwordEdit->setText(settings.value("password",QString()).toString());
    connect(_configWidget->testLoginButton,SIGNAL(clicked()),this,SLOT(on_testLoginButton_clicked()));
    connect(_configWidget->testLoginButton,SIGNAL(clicked(bool)),_configWidget->testLoginButton,SLOT(setDisabled(bool)));

}

void LastFmScrobbler::trackFinished(Player::MetaData trackdata)
{
   /* if ((_played >= 240000) && (_played >= trackdata.length/2)) {
        scrobble(trackdata);
    }*/
    scrobble(trackdata);
}

void LastFmScrobbler::trackPositionChanged(qint64 newPos)
{
    _played = newPos;
}

void LastFmScrobbler::trackChanged(Player::MetaData trackData)
{
    Q_UNUSED(trackData);
    _played = 0;
}

void LastFmScrobbler::settingsAccepted()
{
    QSettings settings(QString(_CONFIGDIR).append("/lastfmscrobbler.conf"),QSettings::IniFormat,this);
    settings.setValue("username",_configWidget->usernameEdit->text());
    settings.setValue("password",_configWidget->passwordEdit->text());
}

void LastFmScrobbler::on_testLoginButton_clicked()
{
    QNetworkAccessManager *testNAM = new QNetworkAccessManager();
    QNetworkRequest nr;
    nr.setUrl(prepareHandshakeURL(_configWidget->usernameEdit->text(),
                                  _configWidget->passwordEdit->text()));
    nr.setRawHeader("Host", "post.audioscrobbler.com");
    connect(testNAM,SIGNAL(finished(QNetworkReply*)),this,SLOT(testLoginFinished(QNetworkReply*)));
    testNAM->get(nr);
}

void LastFmScrobbler::testLoginFinished(QNetworkReply *reply)
{
    // Default color for errors
    _configWidget->errorLabel->setStyleSheet("color: red;");

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode!=200) {
        _configWidget->errorLabel->setText(tr("Invalid server response:")+reply->errorString());
    } else {
        QString status = reply->readLine(1024);
        if (status.startsWith("OK")) {
            _configWidget->errorLabel->setStyleSheet("color: green");
            _configWidget->errorLabel->setText(tr("Test was successful"));
        } else if (status.startsWith("BADAUTH")) {
            _configWidget->errorLabel->setText(tr("Bad username or password"));
        } else  if (status.startsWith("BANNED")) {
            _configWidget->errorLabel->setText(tr("Account is banned"));
        } else if (status.startsWith("BADTIME")) {
            _configWidget->errorLabel->setText(tr("The difference between local and server timestamps is too big. Set up your clock properly first."));
        } else if (status.startsWith("FAILED")) {
            _configWidget->errorLabel->setText(tr("Test failed for following reason:")+status.remove("FAILED ",Qt::CaseSensitive));
        } else {
            _configWidget->errorLabel->setText(tr("Unknown error"));
        }
    }
    // re-enable the "Test" button
    _configWidget->testLoginButton->setEnabled(true);
}

QUrl LastFmScrobbler::prepareHandshakeURL(QString username, QString password)
{
    QString timestamp = QString::number(QDateTime::currentDateTime().toTime_t());
    QCryptographicHash hash(QCryptographicHash::Md5);
    QString passwordHash = QCryptographicHash::hash(password.toUtf8(),
                                                QCryptographicHash::Md5).toHex();
    QString token = QCryptographicHash::hash(QString(passwordHash+timestamp).toUtf8(),
                                             QCryptographicHash::Md5).toHex();

    QString uri;
    uri.append("hs=true");
    uri.append("&p=1.2.1");
    //uri.append("&c="+QCoreApplication::applicationName());
    //uri.append("&v="+QCoreApplication::applicationVersion());
    uri.append("&c=tst");
    uri.append("&v=1.0");
    uri.append("&u=" + username);
    uri.append("&t=" + timestamp);
    uri.append("&a=" + token);

    return QUrl("http://post.audioscrobbler.com/?"+uri);
}

void LastFmScrobbler::login()
{
    QSettings settings(QString(_CONFIGDIR).append("/lastfmscrobbler.conf"),QSettings::IniFormat,this);
    QString username = settings.value("username",QString()).toString();
    QString password = settings.value("password",QString()).toString();
    // We won't try to log in if user didn't fill the credentials
    if ((username.isEmpty()) || (password.isEmpty())) return;
    
    prepareHandshakeURL(username,password);
    QNetworkAccessManager *NAM = new QNetworkAccessManager();
    QNetworkRequest nr;
    nr.setUrl(prepareHandshakeURL(username,password));
    nr.setRawHeader("Host", "post.audioscrobbler.com");
    connect(NAM,SIGNAL(finished(QNetworkReply*)),this,SLOT(loginFinished(QNetworkReply*)));
    NAM->get(nr);
}

void LastFmScrobbler::loginFinished(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "Last.fm plugin login: server replied " << statusCode;
    if (statusCode!=200) {
        emit error(tr("Last.fm plugin: Invalid server response. Failed to login"));
    } else {
        QString status = reply->readLine(1024);
        if (status.startsWith("OK")) {
            _token = QString(reply->readLine(256)).remove("\n");
            _nowPlayingURL = QString(reply->readLine(256)).remove("\n");
            _submissionURL = QString(reply->readLine(256)).remove("\n");
            _failedAttempts = 0;
            // Try to submit all tracks in cache
            submitTrack();
        } else if (status.startsWith("BADAUTH")) {
            emit error("Last.fm plugin: "+tr("Bad username or password"));
        } else  if (status.startsWith("BANNED")) {
            emit error("Last.fm plugin: "+tr("Account is banned"));
        } else if (status.startsWith("BADTIME")) {
            emit error("Last.fm plugin: "+tr("The difference between local and server timestamps is too big. Set up your clock properly first."));
        } else if (status.startsWith("FAILED")) {
            emit error("Last.fm plugin: "+status.remove("FAILED ",Qt::CaseSensitive));
        } else {
            emit error("Last.fm plugin: "+tr("Unknown error"));
        }
    }

}

void LastFmScrobbler::scrobble(Player::MetaData metadata)
{
    LastFmScrobbler::MetaData data;
    data.trackInfo = metadata;
    data.playbackStart = QDateTime::currentDateTime().toUTC().toTime_t()-(int)(metadata.length/1000);

    // Appends last track to cache. The track is removed when sucessfully scrobbled
    _cache.append(data);
    saveCache();

    // Don't even try when not logged in
    if (_submissionURL.isEmpty())
        return;

    /* If this is the only track in front we can expect that everything works and the track
       will be submitted. If there are more tracks then we will suppose that something is wrong
       and we'll wait for timer */
    if (_cache.size() == 1)
        submitTrack();

}

void LastFmScrobbler::submitTrack()
{
    if (_cache.isEmpty()) return;

    LastFmScrobbler::MetaData metadata = _cache.first();

    QString data("s="+_token);

    QString playbackStart = QString::number(metadata.playbackStart);

    data.append("&a[0]="+QUrl::toPercentEncoding(metadata.trackInfo.artist.toUtf8()));
    data.append("&t[0]="+QUrl::toPercentEncoding(metadata.trackInfo.title.toUtf8()));
    data.append("&i[0]="+playbackStart);
    data.append("&o[0]=R");
    data.append("&r[0]=");
    data.append("&l[0]="+QString::number((int)(metadata.trackInfo.length/1000)));
    data.append("&b[0]="+QUrl::toPercentEncoding(metadata.trackInfo.album.toUtf8()));
    data.append("&n[0]="+QString::number(metadata.trackInfo.trackNumber));
    data.append("&m[0]=");

    QNetworkAccessManager *NAM = new QNetworkAccessManager();
    QNetworkRequest nr;
    nr.setUrl(QUrl(_submissionURL));
    nr.setRawHeader("Host",QUrl(_submissionURL).host().toUtf8());
    connect(NAM,SIGNAL(finished(QNetworkReply*)),this,SLOT(scrobblingFinished(QNetworkReply*)));
    NAM->post(nr,data.toUtf8());
}

void LastFmScrobbler::scrobblingFinished(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode!=200) {
        emit error(tr("Last.fm plugin: Invalid server response. Failed to scrobble"));
        _failedAttempts++;
        setupTimer();
    } else {
        QString status = reply->readLine(1024);
        if (status.startsWith("OK")) {
            qDebug() << "Track was sucessfully scrobbled";
            _failedAttempts = 0;
            _cache.takeFirst();
            saveCache();
            // if there are some more tracks to submit then do it
            if (!_cache.isEmpty()) submitTrack();
        } else if (status.startsWith("BADSESSION")) {
            emit error("Last.fm plugin: "+tr("Failed to scrobble due invalid token. Trying to obtain a new one..."));
            login();
            _failedAttempts++;
            setupTimer();
        } else if (status.startsWith("FAILED")) {
            emit error("Last.fm plugin: "+status.remove("FAILED ",Qt::CaseSensitive));
            _failedAttempts++;
            setupTimer();
        } else {
            emit error("Last.fm plugin: "+tr("Unknown error"));
            _failedAttempts++;
            setupTimer();
        }
    }
}

void LastFmScrobbler::setupTimer()
{
    qDebug() << "Settings up timer...next shot in " << (int)(2^((_failedAttempts/2)*_failedAttempts)*5) << " seconds";
    QTimer::singleShot((int)(2^((_failedAttempts/2)*_failedAttempts)*5000),this,SLOT(submitTrack()));
}

void LastFmScrobbler::saveCache()
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

Q_EXPORT_PLUGIN2(tepsonic_lastfmscrobbler, LastFmScrobbler)


