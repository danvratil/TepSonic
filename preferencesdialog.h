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

#include <QDirModel>
#include <QPluginLoader>
#include <QTreeWidgetItem>
#include <QSettings>

namespace Ui {
    class PreferencesDialog;
}

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
          \param parent dialog's parent widget
        */
        PreferencesDialog(QWidget *parent = 0);

        //! Destructor
        ~PreferencesDialog();

        //! Create new tab with settings for given plugin
        /*!
          When called a new item is added to the toolbox and plugin's configuration interface is
          created on it's widget using plugin's settingsWidget() method
          \param plugin pointer to QPluginLoader
        */
        void addPlugin(QPluginLoader *plugin);

    signals:
        //! Informs main window that rebuilding collections was requested
        void rebuildCollectionsRequested();

    protected:
        //! When language is changed the window is retranslated
        virtual void changeEvent(QEvent *e);

    private:
        //! Dialog interface
        Ui::PreferencesDialog *_ui;


    private slots:
        //! Called when rebuildCollectionsNowBtn is clicked and emmits rebuildCollectionsRequested() signal
        void on_rebuildCollectionsNowBtn_clicked();

        //! Called when addPathButton is clicked
        /*!
          A file dialog to add folder is showed and user can select a folder that will be added to list of
          collections source folders
        */
        void on_addPathButton_clicked();

        //! Called when removePathButton is clicked
        /*!
          Removes selected path from list of collection source folders
        */
        void on_removePathButton_clicked();

        //! Called when collectionsStorageEngine_combo selection is changed
        /*!
          This enables or disables configuration of connection to MySQL server
          when MySQL is selected in to combo
        */
        void on_collectionsStorageEngine_combo_currentIndexChanged(QString newIndex);

        //! Called when the dialog is closed via "Cancel" button
        void on_buttonBox_rejected();

        //! Called when the dialog is closed via "OK" button
        /*!
          The current configuration is written to the settings file and also plugins' method
          settingsAccepted() is called so plugins could store their settings
        */
        void on_buttonBox_accepted();
};

#endif // PREFERENCESDIALOG_H
