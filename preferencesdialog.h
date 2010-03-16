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

class PreferencesDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(PreferencesDialog)

    public:
        PreferencesDialog(QSettings* settings, QWidget *parent = 0);
        virtual ~PreferencesDialog();

        void addPlugin(QPluginLoader *plugin);

    signals:
        void rebuildCollectionsRequested();

    protected:
        virtual void changeEvent(QEvent *e);

    private:
        Ui::PreferencesDialog *_ui;

        QSettings *_settings;

    private slots:


    private slots:
        void on_rebuildCollectionsNowBtn_clicked();
        void on_addPathButton_clicked();
        void on_removePathButton_clicked();
        void on_collectionsStorageEngine_combo_currentIndexChanged(QString newIndex);
        void on_buttonBox_rejected();
        void on_buttonBox_accepted();
};

#endif // PREFERENCESDIALOG_H
