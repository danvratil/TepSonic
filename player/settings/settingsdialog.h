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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtGui/QDialog>
#include <QAbstractButton>
#include <QDirModel>
#include <QPluginLoader>
#include <QTreeWidgetItem>
#include <QSettings>
#include <QListWidgetItem>

namespace Ui {
    class SettingsDialog;
}

namespace SettingsPages {
    class PlayerPage;
    class PluginsPage;
    class CollectionsPage;
    class ShortcutsPage;
}

class MainWindow;


//! SettingsDialog is a dialog window with application settings
/*!
  SettingsDialog is subclassed from QDialog (is modal) and provides
  window with TepSonic configuration.
*/
class SettingsDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(SettingsDialog)

    public:
        //! Constructor
        /*!
          In constructor UI is prepared and current preferences are loaded from settings file.
          \param parent pointer to main window
        */
        SettingsDialog(MainWindow *parent = 0);

        //! Destructor
        ~SettingsDialog();

    signals:
        //! Informs main window that rebuilding collections was requested
        void rebuildCollections();

    protected:
        //! When language is changed the window is retranslated
        virtual void changeEvent(QEvent *e);

    private:
        //! Dialog interface
        Ui::SettingsDialog *_ui;

        //! Player page
        SettingsPages::PlayerPage *_player;

        //! Collections page
        SettingsPages::CollectionsPage *_collections;

        //! Plugins page
        SettingsPages::PluginsPage *_plugins;

        //! Shorcuts settings page
        SettingsPages::ShortcutsPage *_shortcuts;

        //! Main window
        MainWindow *_parent;

    private slots:

        //! Called when button in the left list is clicked
        void changePage(QListWidgetItem* current, QListWidgetItem* previous);

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
