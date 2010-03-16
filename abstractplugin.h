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


#ifndef ABSTRACTPLUGIN_H
#define ABSTRACTPLUGIN_H

#include "plugininterface.h"

#include <QObject>
#include "player.h"
#include <Phonon/MediaObject>

class QString;
class QWidget;
//class MainWindow;


class AbstractPlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface);
    public:
        virtual QWidget* settingsWidget() = 0;
        virtual QString getName() = 0;
        QString pluginName() { return _pluginName; }

    public slots:
        virtual void settingsAccepted() = 0;
        virtual void trackChanged(QString filename) = 0;
        virtual void playerStatusChanged(Phonon::State newState, Phonon::State oldState) = 0;

    protected:
        void setPluginName(QString name) { _pluginName = name; }

    private:
        QString _pluginName;

};

//#include "moc_abstractplugin.cpp"

#endif // ABSTRACTPLUGIN_H
