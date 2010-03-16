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

PreferencesDialog::PreferencesDialog(QSettings* settings, QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::PreferencesDialog)
{
    _ui->setupUi(this);
    _ui->toolBox->setCurrentIndex(0);

    _settings = settings;

    _settings->beginGroup("Collections");
    _ui->enableCollectionsCheckbox->setChecked(_settings->value("EnableCollections",true).toBool());
    _ui->autoRebuildCheckbox->setChecked(_settings->value("AutoRebuildAfterStart",true).toBool());
    _ui->pathsList->addItems(_settings->value("SourcePaths",QStringList()).toStringList());
    _ui->collectionsStorageEngine_combo->setCurrentIndex(_settings->value("StorageEngine",0).toInt());
    _settings->beginGroup("MySQL");
    _ui->mysqlServer_edit->setText(_settings->value("Server","127.0.0.1").toString());
    _ui->mysqlUsername_edit->setText(_settings->value("Username",QString()).toString());
    _ui->mysqlPassword_edit->setText(_settings->value("Password",QString()).toString());
    _ui->mysqlDatabase_edit->setText(_settings->value("Database",QString()).toString());
    _settings->endGroup();
    _settings->endGroup();
    _settings->beginGroup("Preferences");
    _ui->rememberLastSessionCheckbox->setChecked(_settings->value("RestoreSession",true).toBool());
    _settings->endGroup();

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
    _settings->beginGroup("Collections");
    _settings->setValue("EnableCollections",_ui->enableCollectionsCheckbox->isChecked());
    _settings->setValue("AutoRebuildAfterStart",_ui->autoRebuildCheckbox->isChecked());
    QStringList items;
    for (int i = 0; i < _ui->pathsList->count(); i++) {
        items.append(_ui->pathsList->item(i)->text());
    }
    _settings->setValue("SourcePaths",items);
    _settings->setValue("StorageEngine",_ui->collectionsStorageEngine_combo->currentIndex());
    _settings->beginGroup("MySQL");
    _settings->setValue("Server",_ui->mysqlServer_edit->text());
    _settings->setValue("Username",_ui->mysqlUsername_edit->text());
    // I'd like to have the password encrypted (but not hashed!) - I don't like password in plaintext...
    _settings->setValue("Password",_ui->mysqlPassword_edit->text());
    _settings->setValue("Database",_ui->mysqlDatabase_edit->text());
    _settings->endGroup(); // MySQL group
    _settings->endGroup(); // Collections group
    _settings->beginGroup("Preferences");
    _settings->setValue("RestoreSession",_ui->rememberLastSessionCheckbox->isChecked());
    _settings->endGroup(); // Preferences group

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
    int newItem = _ui->toolBox->addItem(pluginWidget,QIcon(),static_cast<AbstractPlugin*>(plugin->instance())->getName());
    static_cast<AbstractPlugin*>(plugin->instance())->settingsWidget(pluginWidget);
    connect(this,SIGNAL(accepted()),static_cast<AbstractPlugin*>(plugin->instance()),SLOT(settingsAccepted()));
}
