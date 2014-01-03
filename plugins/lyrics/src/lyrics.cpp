/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <dan@progdan.cz>
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
#include "lyricsscrollarea.h"

#include <core/player.h>
#include <core/playlist.h>

#include <QDir>
#include <QString>
#include <QLocale>
#include <QPushButton>
#include <QApplication>
#include <QtPlugin>
#include <QSplitter>
#include <QLabel>
#include <QGridLayout>
#include <QUrlQuery>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QListWidgetItem>

using namespace TepSonic;

LyricsPlugin::LyricsPlugin()
{
    const QString locale = QLocale::system().name();
    m_translator = new QTranslator(this);

    const QString dataDir = QLatin1String(PKGDATADIR);
    const QString localeDir = dataDir
            + QDir::separator() + QLatin1String("tepsonic")
            + QDir::separator() +  QLatin1String("locale")
            + QDir::separator() + QLatin1String("lyricsplugin");

    m_translator->load(QLatin1String("tepsonic_lyricsplugin_") + locale, localeDir);
    qApp->installTranslator(m_translator);

    connect(Player::instance(), &Player::trackChanged,
            this, &LyricsPlugin::trackChanged);
}

LyricsPlugin::~LyricsPlugin()
{
}

void LyricsPlugin::init()
{
}

bool LyricsPlugin::setupPane(QWidget *widget, QString &label)
{
    label = tr("Lyrics");

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

void LyricsPlugin::trackChanged()
{
    const int currentTrack = Player::instance()->currentTrack();
    if (currentTrack == -1 ) {
        return;
    }
    const MetaData metaData = Player::instance()->playlist()->track(currentTrack);
    const QByteArray u("http://webservices.lyrdb.com/lookup.php?q="  
                    + QUrl::toPercentEncoding(metaData.artist()) + "|"
                    + QUrl::toPercentEncoding(metaData.title()) + "&for=match&agent=TepSonic");
    const QUrl url(QString::fromLatin1(u));
    QNetworkRequest req(url);
    QNetworkAccessManager *nam = new QNetworkAccessManager();
    connect(nam, &QNetworkAccessManager::finished,
            this, &LyricsPlugin::lyricsInfoRetrieved);
    connect(nam, &QNetworkAccessManager::finished,
            nam, &QNetworkAccessManager::deleteLater);
    nam->get(req);
}

void LyricsPlugin::loadLyrics(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    m_listWidget->setCurrentIndex(index);

    const QString trackID = index.data(Qt::UserRole).toString();

    const QUrl url(QLatin1String("http://webservices.lyrdb.com/getlyr.php?q=") + trackID);
    QNetworkRequest req(url);
    QNetworkAccessManager *nam = new QNetworkAccessManager();
    connect(nam, &QNetworkAccessManager::finished,
            this, &LyricsPlugin::lyricsPageRetrieved);
    connect(nam, &QNetworkAccessManager::finished,
            nam, &QNetworkAccessManager::deleteLater);
    nam->get(req);
}

void LyricsPlugin::lyricsInfoRetrieved(QNetworkReply *reply)
{
    // Parse lyrics URL from the reply
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (status != 200) {
        return;
    }

    m_listWidget->clear();

    const QString tracks = QString::fromLatin1(reply->readAll());
    const QStringList tracks_list = tracks.split(QChar(10));

    const QUrlQuery query(reply->url());
    if ((tracks_list.size() == 0 ||
        (tracks_list.size() == 1 && tracks_list.at(0).isEmpty())) &&
        query.queryItemValue(QLatin1String("for")) != QLatin1String("fullt"))
    {
        const QString track_name = query.queryItemValue(QLatin1String("q")).replace(QLatin1Char('|'), QLatin1String(" - "));
        const QUrl url(QLatin1String("http://webservices.lyrdb.com/lookup.php?q=") + track_name + QLatin1String("&for=fullt&agent=TepSonic"));

        qDebug() << url;

        QNetworkRequest req(url);
        QNetworkAccessManager *nam = new QNetworkAccessManager();
        connect(nam, &QNetworkAccessManager::finished,
                this, &LyricsPlugin::lyricsInfoRetrieved);
        connect(nam, &QNetworkAccessManager::finished,
                nam, &QNetworkAccessManager::deleteLater);        
        nam->get(req);
    }


    Q_FOREACH (const QString &track, tracks_list)
    {
        const QStringList info = track.split(QLatin1Char('\\'));
        if (info.size() != 3) {
            continue;
        }

        QListWidgetItem *item = new QListWidgetItem (info.at(2) + QLatin1String(" - ") + info.at(1), m_listWidget);
        item->setData(Qt::UserRole, info.at(0));
        m_listWidget->addItem(item);
    }

    loadLyrics(m_listWidget->model()->index(0,0));
}

void LyricsPlugin::lyricsPageRetrieved(QNetworkReply *reply)
{
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (status != 200) {
        return;
    }

    if (!m_listWidget->currentItem()) {
        return;
    }

    QString lyrics = QString::fromLatin1(reply->readAll());
    lyrics.prepend(QLatin1String("<h2>") + m_listWidget->currentItem()->text() + QLatin1String("</h2>"));
    lyrics.replace(QChar(10), QLatin1String("<br>"));
    lyrics.append(QLatin1String("<br><br><i>Powered by <a href=\"http://www.lyrdb.com\">LYRDB.com</a></i>"));

    m_label->setText(lyrics);
    m_label->adjustSize();
}
