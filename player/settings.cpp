/*
 * Copyright 2013  Daniel Vrátil <dan@progdan.cz>
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
#include "constants.h"

#include <QSettings>
#include <QTimer>

Settings *Settings::s_instance = 0;

Settings* Settings::instance()
{
    if (s_instance == 0) {
        s_instance = new Settings();
    }

    return s_instance;
}

Settings::Settings():
    QObject()
{
    m_settings = new QSettings(_CONFIGDIR + QLatin1String("/main.conf"), QSettings::IniFormat, this);
    m_writebackTimer = new QTimer(this);
    m_writebackTimer->setInterval(500);
    m_writebackTimer->setSingleShot(true);
    connect(m_writebackTimer, SIGNAL(timeout()),
            this, SLOT(onWritebackTimeout()));
}

Settings::~Settings()
{
    if (m_writebackTimer->isActive()) {
        m_writebackTimer->stop();
    }

    onWritebackTimeout();
}

void Settings::destroy()
{
    delete s_instance;
    s_instance = 0;
}

void Settings::onWritebackTimeout()
{
    m_settings->sync();
}

#define DECLARESETTINGSGETTER(type, getter, group, option, default) \
type Settings::getter() const \
{ \
    m_settings->beginGroup(QLatin1String(group)); \
    const QVariant val = m_settings->value(QLatin1String(option), default); \
    m_settings->endGroup(); \
    return val.value<type>(); \
}

#define DECLARESETTINGSSETTER(type, setter, group, option) \
void Settings::setter(const type &val) \
{ \
    m_settings->beginGroup(QLatin1String(group)); \
    m_settings->setValue(QLatin1String(option), val); \
    m_settings->endGroup(); \
    if (!m_writebackTimer->isActive()) { \
        m_writebackTimer->start(); \
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

DECLAREOPTION(QString, collectionsMySQLServer, setCollectionsMySQLServer,
              "Collections", "MySQL/Server", QLatin1String("127.0.0.1"))
DECLAREOPTION(QString, collectionsMySQLDatabase, setCollectionsMySQLDatabase,
              "Collections", "MySQL/Database", QLatin1String("TepSonic"))
DECLAREOPTION(QString, collectionsMySQLUsername, setCollectionsMySQLUsername,
              "Collections", "MySQL/Username", QString())
DECLAREOPTION(QString, collectionsMySQLPassword, setCollectionsMySQLPassword,
              "Collections", "MySQL/Password", QString())

DECLAREOPTION(QString, collectionsStorageEngine, setCollectionsStorageEngine,
              "Collections", "StorageEngine", QLatin1String("QSQLITE"))


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
