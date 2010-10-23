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

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QtGui/QDialog>
#include <QAbstractButton>
#include <QDirModel>
#include <QPluginLoader>
#include <QTreeWidgetItem>
#include <QSettings>
#include <QListWidgetItem>

namespace Ui {
    class PreferencesDialog;
}

namespace PreferencesPages {
    class Player;
    class Plugins;
    class Collections;
    class Shortcuts;
}

class MainWindow;


//! PreferencesDialog is a dialog window with application settings
/*!
  PreferencesDialog is subclassed from QDialog (is modal) and provides
  window with TepSonic configuration.
  Before the dialog is opened main window calls method addPlugin() and so configuration interface
  for plugins is added.
*/
class PreferencesDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(PreferencesDialog)

    public:
        //! Constructor
        /*!
          In constructor UI is prepared and current preferences are loaded from settings file.
          \param parent pointer to main window
        */
        PreferencesDialog(MainWindow *parent = 0);

        //! Destructor
        ~PreferencesDialog();

    signals:
        //! Informs main window that rebuilding collections was requested
        void rebuildCollections();

    protected:
        //! When language is changed the window is retranslated
        virtual void changeEvent(QEvent *e);

    private:
        //! Dialog interface
        Ui::PreferencesDialog *_ui;

        //! Player page
        PreferencesPages::Player *_player;

        //! Collections page
        PreferencesPages::Collections *_collections;

        //! Plugins page
        PreferencesPages::Plugins *_plugins;

        //! Shorcuts settings page
        PreferencesPages::Shortcuts *_shortcuts;

        //! Main window
        MainWindow *_parent;

    private slots:

        //! Called when button in the left list is clicked
        void on_pagesButtons_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);

        //! Called when the dialog is closed via "OK" button
        /*!
          The current configuration is written to the settings file and also plugins' method
          settingsAccepted() is called so plugins could store their settings
        */
        void dialogAccepted();

        //! Saves current confugration and then emits signal rebuildCollections()
        void emitRebuildCollections();

        void enablePlugin(int);

        void disablePlugin(int);
};

#endif // PREFERENCESDIALOG_H
