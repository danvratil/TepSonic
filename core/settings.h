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

#ifndef TEPSONIC_SETTINGS_H
#define TEPSONIC_SETTINGS_H

#include <QtCore/QObject>
#include <QtCore/QStringList>

class QTimer;
class QSettings;

#include "tepsonic-core-export.h"

namespace TepSonic
{

class TEPSONIC_CORE_EXPORT Settings
{
    Q_DISABLE_COPY(Settings)

  public:
    static Settings *instance();
    ~Settings();

    void destroy();

    static QString configDir();
    static QString dataDir();

#define DECLAREPROPERTY(type, getter, setter) \
    type getter() const; \
    void setter(const type &value);

    /* === Collections === */
    DECLAREPROPERTY(bool, collectionsEnabled, setCollectionsEnabled)
    DECLAREPROPERTY(bool, collectionsAutoRebuild, setCollectionsAutoRebuild)
    DECLAREPROPERTY(QStringList, collectionsSourcePaths, setCollectionsSourcePaths)

    /* === Session === */
    DECLAREPROPERTY(bool, sessionRestore, setSessionRestore)
    DECLAREPROPERTY(QString, sessionFSBrowserPath, setSessionFSBrowserPath)

    /* === Plugins === */
    DECLAREPROPERTY(QStringList, enabledPlugins, setEnabledPlugins)

    /* === Player === */
    DECLAREPROPERTY(bool, playerRandomMode, setPlayerRandomMode)
    DECLAREPROPERTY(int, playerRepeatMode, setPlayerRepeatMode)
    DECLAREPROPERTY(int, playerOutputDevice, setPlayerOutputDevice)
    DECLAREPROPERTY(QVariantMap, playerEffects, setPlayerEffects)

    /* === Shortcuts === */
    DECLAREPROPERTY(QString, shortcutNextTrack, setShortcutNextTrack)
    DECLAREPROPERTY(QString, shortcutPreviousTrack, setShortcutPreviousTrack)
    DECLAREPROPERTY(QString, shortcutPlayPause, setShortcutPlayPause)
    DECLAREPROPERTY(QString, shortcutStop, setShortcutStop)
    DECLAREPROPERTY(QString, shortcutToggleWindow, setShortcutToggleWindow)

    /* === Window === */
    DECLAREPROPERTY(QByteArray, windowGeometry, setWindowGeometry)
    DECLAREPROPERTY(QByteArray, windowState, setWindowState)
    DECLAREPROPERTY(QByteArray, windowSplitterState, setWindowSplitterState)

    /* === Playlist === */
    DECLAREPROPERTY(QVariantList, playlistColumnsStates, setPlaylistColumnsStates)
    DECLAREPROPERTY(QVariantList, playlistColumnsWidths, setPlaylistColumnsWidths)

#undef DECLAREPROPERTY

  private:
    Settings();

    class Private;
    const Private * const d;
};

} // namespace TepSonic

#endif // TEPSONIC_SETTINGS_H
