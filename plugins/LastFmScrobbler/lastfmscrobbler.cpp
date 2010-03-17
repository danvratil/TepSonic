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
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QString>
#include <QWidget>
#include <QPushButton>
#include <QFormLayout>
#include <QHttpRequestHeader>

LastFmScrobbler::LastFmScrobbler()
{
    qDebug() << "Last.fm plugin loaded!";

    login();
}

LastFmScrobbler::~LastFmScrobbler()
{
    qDebug() << "LastFmScrobbler destroyed";
}

QString LastFmScrobbler::pluginName()
{
    return "Last.fm plugin";
}

void LastFmScrobbler::settingsWidget(QWidget *parentWidget)
{
    _configWidget = new Ui::LastFmScrobblerConfig();
    _configWidget->setupUi(parentWidget);

    QSettings settings(QDir::homePath().append("/.tepsonic/lastfmscrobbler.conf"),QSettings::IniFormat,this);
    _configWidget->usernameEdit->setText(settings.value("username",QString()).toString());
    _configWidget->passwordEdit->setText(settings.value("password",QString()).toString());
    connect(_configWidget->testLoginButton,SIGNAL(clicked()),this,SLOT(on_testLoginButton_clicked()));
    connect(_configWidget->testLoginButton,SIGNAL(clicked(bool)),_configWidget->testLoginButton,SLOT(setDisabled(bool)));
}

void LastFmScrobbler::trackFinished(MetaData trackdata)
{
    if ((_played >= 240000) && (_played >= trackdata.length/2)) {
        scrobble(trackdata);
    }
}

void LastFmScrobbler::trackPositionChanged(qint64 newPos)
{
    _played = newPos;
}

void LastFmScrobbler::trackChanged(MetaData trackData)
{
    Q_UNUSED(trackData);
    _played = 0;
}

void LastFmScrobbler::settingsAccepted()
{
    QSettings settings(QDir::homePath().append("/.tepsonic/lastfmscrobbler.conf"),QSettings::IniFormat,this);
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

    QString uri = "hs=true&p=1.2.1&c=tst&v=1.0&u=" + username
                    + "&t=" + timestamp
                    + "&a=" + token;

    return QUrl("http://post.audioscrobbler.com/?"+uri);
}

void LastFmScrobbler::login()
{
    QSettings settings(QDir::homePath().append("/.tepsonic/lastfmscrobbler.conf"),QSettings::IniFormat,this);
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
    if (statusCode!=200) {
        emit error(tr("Last.fm plugin: Invalid server response. Failed to login"));
    } else {
        QString status = reply->readLine(1024);
        if (status.startsWith("OK")) {
            _token = QString(reply->readLine(256)).remove("\n");
            _nowPlayingURL = QString(reply->readLine(256)).remove("\n");
            _submissionURL = QString(reply->readLine(256)).remove("\n");
        } else if (status.startsWith("BADAUTH")) {
            emit error("Last.fm plugin: "+tr("Bad username or login"));
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

void LastFmScrobbler::scrobble(MetaData metadata)
{
    QString data("s="+_token);

    data.append("&a[0]="+QUrl::toPercentEncoding(metadata.artist.toUtf8()));
    data.append("&t[0]="+QUrl::toPercentEncoding(metadata.title.toUtf8()));
    data.append("&i[0]="+QString::number(QDateTime::currentDateTime().toTime_t()-metadata.length));
    data.append("&o[0]=R");
    data.append("&r[0]=");
    data.append("&l[0]="+QString::number((int)(metadata.length/1000)));
    data.append("&b[0]="+QUrl::toPercentEncoding(metadata.album.toUtf8()));
    data.append("&n[0]="+QString::number(metadata.trackNumber));
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
    } else {
        QString status = reply->readLine(1024);
        if (status.startsWith("OK")) {
            qDebug() << "Track was sucessfully scrobbled";
        } else if (status.startsWith("BADSESSION")) {
            emit error("Last.fm plugin: "+tr("Failed to scrobble due invalid token. Trying to obtain a new one..."));
            login();
        } else if (status.startsWith("FAILED")) {
            emit error("Last.fm plugin: "+status.remove("FAILED ",Qt::CaseSensitive));
        } else {
            emit error("Last.fm plugin: "+tr("Unknown error"));
        }
    }
}

#include "moc_lastfmscrobbler.cpp"

Q_EXPORT_PLUGIN2(tepsonic_lastfmscrobbler, LastFmScrobbler)


