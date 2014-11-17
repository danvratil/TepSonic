/*
 * Copyright 2013  Daniel Vrátil <me@dvratil.cz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "settings.h"

#include <QSettings>
#include <QTimer>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>

using namespace TepSonic;

class Settings::Private
{
  public:
    Private();
    ~Private();

    static Settings *instance;

    QSettings *settings;
    QTimer *writebackTimer;
};

Settings *Settings::Private::instance = 0;

Settings::Private::Private()
{
    const QString configFile = Settings::configDir() + QLatin1String("/main.conf");
    qDebug() << "Config:" << configFile;
    settings = new QSettings(configFile, QSettings::IniFormat);
    writebackTimer = new QTimer();
    writebackTimer->setInterval(100);
    writebackTimer->setSingleShot(true);
    QObject::connect(writebackTimer, &QTimer::timeout, [=]() { settings->sync(); });
}

Settings::Private::~Private()
{
    if (writebackTimer->isActive()) {
        writebackTimer->stop();
    }

    delete writebackTimer;
    delete settings;
}

Settings* Settings::instance()
{
    if (Private::instance == 0) {
        Private::instance = new Settings();
    }

    return Private::instance;
}

Settings::Settings():
    d(new Private)
{
}

Settings::~Settings()
{
    delete d;
}

void Settings::destroy()
{
    delete Private::instance;
    Private::instance = 0;
}

QString fullStandardPath(QStandardPaths::StandardLocation location)
{
    const QString path = QStandardPaths::writableLocation(location) + QLatin1String("/tepsonic");
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(path);
    }
    return path;
}

QString Settings::configDir()
{
    static QString s_configDir;
    if (s_configDir.isEmpty()) {
        s_configDir = fullStandardPath(QStandardPaths::ConfigLocation);
    }
    return s_configDir;
}

QString Settings::dataDir()
{
    static QString s_dataDir;
    if (s_dataDir.isEmpty()) {
        s_dataDir = fullStandardPath(QStandardPaths::DataLocation);
    }
    return s_dataDir;
}


#define DECLARESETTINGSGETTER(type, getter, group, option, default) \
type Settings::getter() const \
{ \
    d->settings->beginGroup(QLatin1String(group)); \
    const QVariant val = d->settings->value(QLatin1String(option), default); \
    d->settings->endGroup(); \
    return val.value<type>(); \
}

#define DECLARESETTINGSSETTER(type, setter, group, option) \
void Settings::setter(const type &val) \
{ \
    d->settings->beginGroup(QLatin1String(group)); \
    d->settings->setValue(QLatin1String(option), val); \
    d->settings->endGroup(); \
    if (!d->writebackTimer->isActive()) { \
        d->writebackTimer->start(); \
    } \
}

#define DECLAREOPTION(type, getter, setter, group, option, default) \
    DECLARESETTINGSGETTER(type, getter, group, option, default) \
    DECLARESETTINGSSETTER(type, setter, group, option)

DECLAREOPTION(bool, collectionsEnabled, setCollectionsEnabled,
              "Collections", "Enabled", true)
DECLAREOPTION(bool, collectionsAutoRebuild, setCollectionsAutoRebuild,
              "Collections", "AutoRebuild", false)
DECLAREOPTION(QStringList, collectionsSourcePaths, setCollectionsSourcePaths,
              "Collections", "SourcePaths", QStringList())

DECLAREOPTION(bool, sessionRestore, setSessionRestore,
              "Session", "restore", true)
DECLAREOPTION(QString, sessionFSBrowserPath, setSessionFSBrowserPath,
              "Session", "LastFSBPath", QString())

DECLAREOPTION(QStringList, enabledPlugins, setEnabledPlugins,
              "Plugins", "pluginsEnabled", QStringList())

DECLAREOPTION(bool, playerRandomMode, setPlayerRandomMode,
              "Player", "RandomMode", false)
DECLAREOPTION(int, playerRepeatMode, setPlayerRepeatMode,
              "Player", "RepeatMode", 0)
DECLAREOPTION(int, playerOutputDevice, setPlayerOutputDevice,
              "Player", "OutputDevice", -1)
DECLAREOPTION(QVariantMap, playerEffects, setPlayerEffects,
              "Player", "Effects", QVariantMap())

DECLAREOPTION(QString, shortcutNextTrack, setShortcutNextTrack,
              "Shortcuts", "NextTrack", QLatin1String("Meta+N"))
DECLAREOPTION(QString, shortcutPreviousTrack, setShortcutPreviousTrack,
              "Shortcuts", "PreviousTrack", QLatin1String("Meta+B"))
DECLAREOPTION(QString, shortcutPlayPause, setShortcutPlayPause,
              "Shortcuts", "PlayPause", QLatin1String("Meta+P"))
DECLAREOPTION(QString, shortcutStop, setShortcutStop,
              "Shortcuts", "Stop", QLatin1String("Meta+S"))
DECLAREOPTION(QString, shortcutToggleWindow, setShortcutToggleWindow,
              "Shortcuts", "ToggleWindow", QLatin1String("Meta+H"))

DECLAREOPTION(QByteArray, windowGeometry, setWindowGeometry,
              "Window", "Geometry", QByteArray())
DECLAREOPTION(QByteArray, windowState, setWindowState,
              "Window", "State", QByteArray())
DECLAREOPTION(QByteArray, windowSplitterState, setWindowSplitterState,
              "Window", "SplitterState", QByteArray())

DECLAREOPTION(QVariantList, playlistColumnsStates, setPlaylistColumnsStates,
              "Playlist", "ColumnsStates", QVariantList())
DECLAREOPTION(QVariantList, playlistColumnsWidths, setPlaylistColumnsWidths,
              "Playlist", "ColumnsWidths", QVariantList())

#undef DECLARESETTINGSGETTER
#undef DECLARESETTINGSSETTER
#undef DECLAREOPTION
