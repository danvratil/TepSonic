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


#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include <QSettings>

class SettingsPage: public QWidget {

    Q_OBJECT
    public:
        SettingsPage (QWidget *parent): QWidget(parent) {};

    public slots:
        virtual void loadSettings (QSettings *settings) = 0;

        virtual void saveSettings (QSettings *settings) = 0;
};

#endif // SETTINGSPAGE_H
