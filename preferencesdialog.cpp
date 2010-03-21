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

#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include "abstractplugin.h"

#include <QFileDialog>
#include <QDebug>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::PreferencesDialog)
{
    _ui->setupUi(this);
    _ui->toolBox->setCurrentIndex(0);

    QSettings settings(QString(QDir::homePath()).append("/.tepsonic/main.conf"),QSettings::IniFormat,this);
    settings.beginGroup("Collections");
    _ui->enableCollectionsCheckbox->setChecked(settings.value("EnableCollections",true).toBool());
    _ui->autoRebuildCheckbox->setChecked(settings.value("AutoRebuildAfterStart",true).toBool());
    _ui->pathsList->addItems(settings.value("SourcePaths",QStringList()).toStringList());
    _ui->collectionsStorageEngine_combo->setCurrentIndex(settings.value("StorageEngine",0).toInt());
    settings.beginGroup("MySQL");
    _ui->mysqlServer_edit->setText(settings.value("Server","127.0.0.1").toString());
    _ui->mysqlUsername_edit->setText(settings.value("Username",QString()).toString());
    _ui->mysqlPassword_edit->setText(settings.value("Password",QString()).toString());
    _ui->mysqlDatabase_edit->setText(settings.value("Database",QString()).toString());
    settings.endGroup();
    settings.endGroup();
    settings.beginGroup("Preferences");
    _ui->rememberLastSessionCheckbox->setChecked(settings.value("RestoreSession",true).toBool());
    settings.endGroup();

}

PreferencesDialog::~PreferencesDialog()
{
    delete _ui;
}

void PreferencesDialog::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        _ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void PreferencesDialog::on_buttonBox_accepted()
{
    QSettings settings(QString(QDir::homePath()).append("/.tepsonic/main.conf"),QSettings::IniFormat,this);
    settings.beginGroup("Collections");
    settings.setValue("EnableCollections",_ui->enableCollectionsCheckbox->isChecked());
    settings.setValue("AutoRebuildAfterStart",_ui->autoRebuildCheckbox->isChecked());
    QStringList items;
    for (int i = 0; i < _ui->pathsList->count(); i++) {
        items.append(_ui->pathsList->item(i)->text());
    }
    settings.setValue("SourcePaths",items);
    settings.setValue("StorageEngine",_ui->collectionsStorageEngine_combo->currentIndex());
    settings.beginGroup("MySQL");
    settings.setValue("Server",_ui->mysqlServer_edit->text());
    settings.setValue("Username",_ui->mysqlUsername_edit->text());
    // I'd like to have the password encrypted (but not hashed!) - I don't like password in plaintext...
    settings.setValue("Password",_ui->mysqlPassword_edit->text());
    settings.setValue("Database",_ui->mysqlDatabase_edit->text());
    settings.endGroup(); // MySQL group
    settings.endGroup(); // Collections group
    settings.beginGroup("Preferences");
    settings.setValue("RestoreSession",_ui->rememberLastSessionCheckbox->isChecked());
    settings.endGroup(); // Preferences group

    emit(accepted());
    this->close();
}

void PreferencesDialog::on_buttonBox_rejected()
{
    this->close();
}

void PreferencesDialog::on_collectionsStorageEngine_combo_currentIndexChanged(QString newIndex)
{
    if (newIndex == "MySQL") {
        _ui->mysqlStorageConfiguration_box->setEnabled(true);
    } else {
        _ui->mysqlStorageConfiguration_box->setDisabled(true);
    }
}

void PreferencesDialog::on_removePathButton_clicked()
{
    _ui->pathsList->takeItem(_ui->pathsList->currentRow());
}

void PreferencesDialog::on_addPathButton_clicked()
{
    QString dirName = QFileDialog::getExistingDirectory(this,
                                                        tr("Add directory"),
                                                        QString(),
                                                        QFileDialog::ShowDirsOnly);
    if(!dirName.isEmpty()) {
        _ui->pathsList->addItem(dirName);
    }
}

void PreferencesDialog::on_rebuildCollectionsNowBtn_clicked()
{
    emit rebuildCollectionsRequested();
}

void PreferencesDialog::addPlugin(QPluginLoader *plugin)
{
    QWidget *pluginWidget = new QWidget();
    _ui->toolBox->addItem(pluginWidget,QIcon(),static_cast<AbstractPlugin*>(plugin->instance())->pluginName());
    static_cast<AbstractPlugin*>(plugin->instance())->settingsWidget(pluginWidget);
    connect(this,SIGNAL(accepted()),static_cast<AbstractPlugin*>(plugin->instance()),SLOT(settingsAccepted()));
}
