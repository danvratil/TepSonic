/*
 * TEPSONIC
 * Copyright 2011 Dan Vratil <vratil@progdansoft.com>
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

#include "lyrics.h"
#include "player.h"

#include <QDir>
#include <QString>
#include <QLocale>
#include <QPushButton>
#include <QApplication>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QListWidgetItem>

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
    return "Lyrics plugin";
}

extern "C" ID_EXPORT QString pluginID()
{
    return "lyrics";
}

void LyricsSrollArea::resizeEvent(QResizeEvent *e)
{
    widget()->adjustSize();
}


LyricsPlugin::LyricsPlugin()
{
    setHasConfigUI(false);

    QString locale = QLocale::system().name();
    _translator = new QTranslator(this);
#ifndef APPLEBUNDLE
    // standard unix/windows
    QString dataDir = QLatin1String(PKGDATADIR);
    QString localeDir = dataDir + QDir::separator() + "tepsonic" + QDir::separator() +  "locale" + QDir::separator() + "lyricsplugin";
#else
    // mac's bundle. Special stuff again.
    QString localeDir = QCoreApplication::applicationDirPath() + "/../Resources/lyricsplugin";
#endif

    _translator->load("lyricsplugin_"+locale,localeDir);
    qApp->installTranslator(_translator);

    connect(Player::instance(), SIGNAL(trackChanged(Player::MetaData)),
            this, SLOT(trackChanged(Player::MetaData)));
}

LyricsPlugin::~LyricsPlugin()
{
    delete m_label;
    delete m_scrollArea;
    delete m_listWidget;
    delete m_splitter;
    delete m_layout;
}

void LyricsPlugin::init()
{


}

void LyricsPlugin::quit()
{


}

bool LyricsPlugin::setupPane(QWidget *widget, QString *label)
{
    *label = QString("Lyrics");

    m_layout = new QGridLayout(widget);

    m_splitter = new QSplitter(widget);

    m_scrollArea = new LyricsSrollArea(widget);
    m_scrollArea->show();
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_splitter->addWidget(m_scrollArea);
    m_splitter->setStretchFactor(0, 5);

    m_label = new QLabel(m_scrollArea);
    m_label->setWordWrap(true);
    m_label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_scrollArea->setWidget(m_label);

    m_listWidget = new QListWidget(widget);
    m_listWidget->show();
    m_splitter->addWidget(m_listWidget);
    m_splitter->setStretchFactor(1, 0);
    connect (m_listWidget, SIGNAL(doubleClicked(QModelIndex)),
             this, SLOT(loadLyrics(QModelIndex)));

    m_layout->addWidget(m_splitter);

    return true;
}

void LyricsPlugin::trackChanged(Player::MetaData trackData)
{
    QUrl url("http://webservices.lyrdb.com/lookup.php?q="+trackData.artist+"|"+trackData.title+"&for=match&agent=TepSonic");

    QNetworkRequest req(url);
    QNetworkAccessManager *nam = new QNetworkAccessManager();
    connect(nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(lyricsInfoRetrieved(QNetworkReply*)));
    connect(nam, SIGNAL(finished(QNetworkReply*)),
            nam, SLOT(deleteLater()));
    nam->get(req);
}

void LyricsPlugin::loadLyrics(QModelIndex index)
{
    if (!index.isValid())
        return;

    m_listWidget->setCurrentIndex(index);

    QString trackID = index.data(Qt::UserRole).toString();

    QUrl url("http://webservices.lyrdb.com/getlyr.php?q="+trackID);
    QNetworkRequest req(url);
    QNetworkAccessManager *nam = new QNetworkAccessManager();
    connect(nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(lyricsPageRetrieved(QNetworkReply*)));
    connect(nam, SIGNAL(finished(QNetworkReply*)),
            nam, SLOT(deleteLater()));
    nam->get(req);
}


void LyricsPlugin::lyricsInfoRetrieved(QNetworkReply *reply)
{
    // Parse lyrics URL from the reply
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (status != 200) {
        setError(status);
        return;
    }

    m_listWidget->clear();

    QString tracks = reply->readAll();
    QStringList tracks_list = tracks.split(QChar(10));

    if ((tracks_list.size() == 0 ||
        (tracks_list.size() == 1 && tracks_list.at(0).isEmpty())) &&
        reply->url().queryItemValue("for") != "fullt")
    {
        QString track_name = reply->url().queryItemValue("q").replace("|", " - ");
        QUrl url("http://webservices.lyrdb.com/lookup.php?q="+track_name+"&for=fullt&agent=TepSonic");

        qDebug() << url;

        QNetworkRequest req(url);
        QNetworkAccessManager *nam = new QNetworkAccessManager();
        connect(nam, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(lyricsInfoRetrieved(QNetworkReply*)));
        connect(nam, SIGNAL(finished(QNetworkReply*)),
                nam, SLOT(deleteLater()));
        nam->get(req);
    }


    foreach (QString track, tracks_list)
    {
        QStringList info = track.split("\\");
        if (info.size() != 3) continue;

        QListWidgetItem *item = new QListWidgetItem (info.at(2)+" - "+info.at(1), m_listWidget);
        item->setData(Qt::UserRole, info.at(0));
        m_listWidget->addItem(item);
    }

    loadLyrics(m_listWidget->model()->index(0,0));
}

void LyricsPlugin::lyricsPageRetrieved(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (status != 200) {
        setError(status);
        return;
    }

    QString lyrics = reply->readAll();
    lyrics.prepend("<h2>"+m_listWidget->currentItem()->text()+"</h2>");
    lyrics.replace(QChar(10), "<br>");
    lyrics.append("<br><br><i>Powered by <a href=\"http://www.lyrdb.com\">LYRDB.com</a></i>");

    m_label->setText(lyrics);
    m_label->adjustSize();
}




Q_EXPORT_PLUGIN2(tepsonic_lyrics, LyricsPlugin)
