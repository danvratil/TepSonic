/*
 * TEPSONIC
 * Copyright 2009 Dan Vratil <vratil@progdansoft.com>
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

#include <QtGui/QApplication>
#include <QList>
#include "player.h"
#include "mainwindow.h"
#include "pluginsmanager.h"

int main(int argc, char *argv[])
{
    QApplication tepsonic(argc, argv);
    tepsonic.setApplicationName("TepSonic");
    tepsonic.setOrganizationName("Dan Vrátil");
    tepsonic.setApplicationVersion("0.90");

    Player *player = new Player();
    MainWindow mainWindow(player);
    PluginsManager *pluginsManager = new PluginsManager(&mainWindow,player);
    mainWindow.setPluginsManager(pluginsManager);

    mainWindow.show();
    return tepsonic.exec();
}
