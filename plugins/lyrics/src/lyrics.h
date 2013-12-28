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


#ifndef LYRICS_H
#define LYRICS_H

#include "abstractplugin.h"
#include "player.h"

#include <QTranslator>
#include <QNetworkReply>

class LyricsSrollArea;
class QLabel;
class QListWidget;
class QSplitter;
class QGridLayout;
class QModelIndex;

class LyricsPlugin : public AbstractPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.progdan.tepsonic.plugins.Lyrics"
                      FILE "Lyrics.json")

  public:
    explicit LyricsPlugin();
    virtual ~LyricsPlugin();

    void init();

    bool setupPane(QWidget *widget, const QString &label);

  public Q_SLOTS:
    void trackChanged(const Player::MetaData &trackData);

  private Q_SLOTS:
    void loadLyrics(const QModelIndex &index);
    void lyricsInfoRetrieved(QNetworkReply *reply);
    void lyricsPageRetrieved(QNetworkReply *reply);

  private:
    void setError(int err);

    QTranslator *m_translator;
    LyricsSrollArea *m_scrollArea;
    QLabel *m_label;
    QListWidget *m_listWidget;
    QSplitter *m_splitter;
    QGridLayout *m_layout;
};

#endif // LYRICS_H
