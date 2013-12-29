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

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QLocale>
#include <QLibraryInfo>
#include <QTextCodec>
#include <QTranslator>
#include <QString>
#include <QDebug>

#include "player.h"
#include "mainwindow.h"
#include "pluginsmanager.h"
#include "playlistmodel.h"
#include "taskmanager.h"
#include "constants.h"
#include "settings.h"
#include "trayicon.h"


Player *player;
PluginsManager *pluginsManager;

int main(int argc, char *argv[])
{
    QApplication tepsonic(argc, argv);
    tepsonic.setApplicationName(QLatin1String("TepSonic"));
    tepsonic.setOrganizationName(QLatin1String("Dan Vrátil"));
    tepsonic.setApplicationVersion(QLatin1String(TEPSONIC_VERSION));
    tepsonic.setWindowIcon(QIcon(QStringLiteral(":/icons/mainIcon")));

    QString locale = QLocale::system().name().left(2);

    // Qt translations
    QTranslator qtTranslator;
    qtTranslator.load(QLatin1String("qt_") + locale,
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    tepsonic.installTranslator(&qtTranslator);

    // App translations
    QTranslator translator;

    // standard unix/windows
    QString dataDir = QLatin1String(PKGDATADIR);
    QString localeDir = dataDir + QLatin1String("/tepsonic/locale");

    // If app was not "installed" use the app directory
    if (!QFile::exists(localeDir)) {
        localeDir = qApp->applicationDirPath() + QLatin1String("/tepsonic/locale");
    }

    translator.load(QLatin1String("tepsonic_") + locale, localeDir);
    tepsonic.installTranslator(&translator);

    qRegisterMetaType<Player::MetaData>("Player::MetaData");

    PluginsManager::instance();
    TaskManager::instance();

    MainWindow *mainWindow = new MainWindow;
    mainWindow->show();

    TrayIcon *trayIcon = new TrayIcon(mainWindow);

    Player *player = Player::instance();
    QStringList files;
    for (int i=1; i<tepsonic.arguments().count(); i++) {
        QFileInfo param(tepsonic.arguments().at(i));
        if ((param.isFile()) && (param.exists())) {
            files << param.absoluteFilePath();
        }
    }
    if (!files.isEmpty()) {
        mainWindow->playlistModel()->addFiles(files);
        player->setTrack(files.first());
        player->play();
    }

    int ret = tepsonic.exec();

    delete trayIcon;
    delete mainWindow;

    TaskManager::instance()->destroy();
    Settings::instance()->destroy();

    return ret;
}
