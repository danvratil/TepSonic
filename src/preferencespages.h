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


#ifndef PREFERENCESPAGES_H
#define PREFERENCESPAGES_H

#include <QWidget>
#include "ui_player.h"
#include "ui_collections.h"
#include "ui_plugins.h"

namespace PreferencesPages {

    class Player: public QWidget
    {
        Q_OBJECT
        public:
            Player(QWidget *parent = 0);
            Ui::Player *ui;
    };

    class Collections: public QWidget
    {
        Q_OBJECT
        public:
            Collections(QWidget *parent = 0);
            Ui::Collections *ui;
            bool collectionsSourceChanged() { return _collectionsSourceChanged; }

        private:
            bool _collectionsSourceChanged;

        private slots:
            void on_pushButton_clicked();
            void on_removeAllPathsButton_clicked();
            void on_removePathButton_clicked();
            void on_addPathButton_clicked();
            void on_dbEngineCombo_currentIndexChanged(QString );

        signals:
            void rebuildCollections();
    };

    class Plugins: public QWidget
    {
        Q_OBJECT
        public:
            Plugins(QWidget *parent = 0);
            Ui::Plugins *ui;
    };

}

#endif // PREFERENCESPAGES_H
