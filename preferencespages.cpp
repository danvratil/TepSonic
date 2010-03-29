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


#include "preferencespages.h"

#include <QFileDialog>

PreferencesPages::Player::Player(QWidget *parent):
        QWidget(parent)
{
    ui = new Ui::Player();
    ui->setupUi(this);
}

PreferencesPages::Collections::Collections(QWidget *parent):
        QWidget(parent)
{
    ui = new Ui::Collections();
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);
    connect(ui->dbEngineCombo,SIGNAL(currentIndexChanged(QString)),
            this,SLOT(on_dbEngineCombo_currentIndexChanged(QString)));
    _collectionsSourceChanged = false;
}

PreferencesPages::Plugins::Plugins(QWidget *parent):
        QWidget(parent)
{
    ui = new Ui::Plugins();
    ui->setupUi(this);
    ui->tabs->setCurrentIndex(0);
}

void PreferencesPages::Collections::on_dbEngineCombo_currentIndexChanged(QString currEngine)
{
    if (currEngine == "SQLite") {
        ui->mysqlSettings->setDisabled(true);
    } else {
        ui->mysqlSettings->setEnabled(true);
    }
}

void PreferencesPages::Collections::on_addPathButton_clicked()
{
    QString dirName = QFileDialog::getExistingDirectory(this,
                                                        tr("Add directory"),
                                                        QString(),
                                                        QFileDialog::ShowDirsOnly);
     if(!dirName.isEmpty()) {
         ui->collectionsPathsList->addItem(dirName);
     }
     _collectionsSourceChanged = true;
}

void PreferencesPages::Collections::on_removePathButton_clicked()
{
    foreach (QListWidgetItem *item, ui->collectionsPathsList->selectedItems()) {
        delete ui->collectionsPathsList->takeItem(ui->collectionsPathsList->row(item));
    }
    _collectionsSourceChanged = true;
}

void PreferencesPages::Collections::on_removeAllPathsButton_clicked()
{
    ui->collectionsPathsList->clear();
    _collectionsSourceChanged = true;
}

#include "moc_preferencespages.cpp"

void PreferencesPages::Collections::on_pushButton_clicked()
{
    emit collectionsSourceChanged();
}
