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
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QLocale>
#include <QLibraryInfo>
#include <QTextCodec>
#include <QTranslator>
#include <QString>
#include "player.h"
#include "mainwindow.h"
#include "pluginsmanager.h"

#include "constants.h"

#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication tepsonic(argc, argv);
    tepsonic.setApplicationName("TepSonic");
    tepsonic.setOrganizationName("Dan Vrátil");
    tepsonic.setApplicationVersion(TEPSONIC_VERSION);

    QString locale = QLocale::system().name();

    // Qt translations
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + locale,
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    tepsonic.installTranslator(&qtTranslator);

    // App translations
    QTranslator translator;
    QString dataDir = QLatin1String(PKGDATADIR);
    QString localeDir = dataDir + QDir::separator() + "tepsonic" + QDir::separator() + "locale" + QDir::separator() + "tepsonic";
    // If app was not "installed" use the app directory
    if (!QFile::exists(localeDir)) {
        localeDir = qApp->applicationDirPath() + QDir::separator() +  "tepsonic" + QDir::separator() + "locale" + QDir::separator() + "tepsonic";
    }
    translator.load(locale,localeDir);
    tepsonic.installTranslator(&translator);

    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    Player *player = new Player();
    MainWindow mainWindow(player);
    PluginsManager *pluginsManager = new PluginsManager(&mainWindow,player);
    mainWindow.setPluginsManager(pluginsManager);

    mainWindow.show();


    for (int i=1; i<tepsonic.arguments().count(); i++) {
        qDebug() << tepsonic.arguments().at(i);
        QFileInfo param(tepsonic.arguments().at(i));
        if ((param.isFile()) && (param.exists())) {
            mainWindow.addPlaylistItem(param.absoluteFilePath());
            // If this is a first file added, start playback immediatelly
            if (i==1) {
                player->setTrack(param.absoluteFilePath());
                player->play();
            }
        }

    }



    return tepsonic.exec();
}
