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

#include "player.h"
#include "plugininterface.h"

#include <QObject>
#include <phonon/mediaobject.h>

class QString;
class QWidget;
class QMenu;
class PluginsManager;


//! The AbstractPlugin class provides interface to be implemented by plugins
/*!
  When creating plugins for TepSonic, plugins must implement a few default functions
  including settingsWidget() and pluginName().
  Implementation of slots is optional. But at least settingsAccepted() should be implemented
  if the plugin has a settings interface.
*/
class AbstractPlugin : public QObject, public PluginInterface
{
    // Plugins manager is our friend!
    friend class PluginsManager;

    Q_OBJECT
    Q_INTERFACES(PluginInterface);
    Q_PROPERTY(bool hasConfigUI
               READ hasConfigUI
               WRITE setHasConfigUI);
    public:

        //! Initializes the plugin
        /*!
          The plugin should not be initiated in the constructor. The constructor is only for setting plugin's name
          and hasConfigUI. The initialization itself (like loading settings, connection to network etc) should be done
          here. This allows user to disable the plugin (init() is not called) but TepSonic still knows about the plugin
          without initializing it.
        */
        virtual void init() = 0;

        //! Uninitializes the plugin
        /*!
          The plugin should stop doing it's work but it won't be destroyed. The plugin will be formally disabled.
        */
        virtual void quit() { }

        //! Initilizes plugin's settings UI on given parentWidget. This is a pure virtual method.
        /*!
          When the Settings dialog is opened it requests PluginManager to give it a list of plugins. Then it
          appends a new QWidget to the list of pages and ask the plugin to initialize it's settings UI on it
          by calling settingsWidget() and passing pointer to the new QWidget.
          \param parentWidget pointer to QWidget that plugin UI will be initialized on
        */
        virtual void settingsWidget(QWidget *parentWidget) = 0;

        //! Returns state of hasConfigUI property
        /*!
          \return Returns state of hasConfigUI property.
        */
        bool hasConfigUI() const { return _hasConfigUI; }

        //! Changes state of hasConfigUI property
        /*!
          \param state new state of the property
        */
        void setHasConfigUI(const bool &state) { _hasConfigUI = state; }

        //! Allows plugin to setup custom menu to given menu. The type of menu is set in menuType.
        virtual void setupMenu(QMenu *menu, Plugins::MenuTypes menuType) = 0;

    public slots:
        //! Called when Settings dialog is accepted
        /*!
          When user accepts the Settings dialog (by clicking on "OK" button) this slot is called. It is good to implement
          saving of configuration to a file.
        */
        virtual void settingsAccepted() {}

        //! Notifies about changing current track
        /*!
          Provides information about new track that was recently set to Player. It recieves MetaData with informations
          about the new track.
          \param metadata Meta data from the current track
        */
        virtual void trackChanged(Player::MetaData metadata) { Q_UNUSED(metadata) }

        //! Notifies that current track was played.
        /*!
          This slot notifies plugin that current track was finished. The slot is called when playback reaches
          the end of the current track, not when the playback was stopped by user or changed for another.
          \param metadata Meta data from the current track
          \sa trackChanged()
        */
        virtual void trackFinished(Player::MetaData metadata) { Q_UNUSED(metadata) }

        //! Notifies about change of player's status
        /*!
          Provides information about change of state of the player. New state and previous (old) state are passed.
          \param newState new state of the player
          \param oldState previous state of the player
        */
        virtual void playerStatusChanged(Phonon::State newState, Phonon::State oldState) { Q_UNUSED(newState) Q_UNUSED(oldState) }

        //! Informs about position of the playback
        /*!
          This slot is fired every second and provides information about current position in the track.
          \param newPos gives current position from beginning of the track in milliseconds
        */
        virtual void trackPositionChanged(qint64 newPos) { Q_UNUSED(newPos) }

        //! Informs that the playback has been (un)paused
        /*!
          When TRUE is passed as an argument, the playback has been paused. FALSE means, that
          playback has been unpaused
          \param paused paused (true) or unpaused (false)
        */
        virtual void trackPaused(bool paused) { Q_UNUSED(paused) }

    signals:
        //! Signalize failure of the plugin to main window
        /*!
          Plugin can emit this signal when an failure that user should be informed about occurs. Additionally it can
          pass a string with short description of the error (eg. Failed to connect to...). The string should be prefixed
          with name of the plugin as it's not done automatically
          \param msg short description of the error
        */
        void error(QString msg);

    private:
        //! Has config UI?
        bool _hasConfigUI;

        //! Is the plugin initialized?
        bool _initialized;

};

#endif // ABSTRACTPLUGIN_H
