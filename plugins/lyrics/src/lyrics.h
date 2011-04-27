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

#ifndef LYRICS_H
#define LYRICS_H

#include "abstractplugin.h"

#include <QTranslator>
#include <QNetworkReply>

#include <QScrollArea>
#include <QLabel>
#include <QListWidget>
#include <QSplitter>
#include <QGridLayout>

#include <QResizeEvent>

class LyricsSrollArea;

class LyricsPlugin : public AbstractPlugin
{
    Q_OBJECT
    public:
        //! Constructor
        /*!
          Creates the plugin.
        */
        LyricsPlugin();

        //! Destructor
        ~LyricsPlugin();

        //! Initialize the plugins
        /*!
          Loads settings
        */
        void init();

        //! Prepares the plugin to be disabled
        void quit();

        bool setupPane(QWidget *widget, QString *label);

        void settingsWidget(QWidget *parentWidget) {};

        void setupMenu(QMenu *menu, Plugins::MenuTypes menuType) {};

    public slots:

        //! Notification about new track
        /*!
          \param trackData meta data of the new track
        */
        void trackChanged(Player::MetaData trackData);


    private:
        QTranslator *_translator;

        void setError(int err) {};

        int findMatchingElement(QString html, int start_pos);

        LyricsSrollArea *m_scrollArea;

        QLabel *m_label;

        QListWidget *m_listWidget;

        QSplitter *m_splitter;

        QGridLayout *m_layout;

    private slots:
        void loadLyrics(QModelIndex);

        void lyricsInfoRetrieved(QNetworkReply*);

        void lyricsPageRetrieved(QNetworkReply*);
};

class LyricsSrollArea: public QScrollArea
{
    Q_OBJECT

    public:
        LyricsSrollArea (QWidget *parent):
            QScrollArea(parent) {};

    protected:
        void resizeEvent(QResizeEvent *);

};

#endif // LYRICS_H
