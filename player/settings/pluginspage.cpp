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

#include "pluginspage.h"
#include "ui_pluginspage.h"

using namespace SettingsPages;

PluginsPage::PluginsPage(QWidget *parent):
        QWidget(parent)
{
    ui = new Ui::PluginsPage();
    ui->setupUi(this);
    ui->tabs->setCurrentIndex(0);
    connect(ui->pluginsList, SIGNAL(itemChanged(QListWidgetItem*)),
            this, SLOT(pluginsListItemChanged(QListWidgetItem*)));
}

void PluginsPage::pluginsListItemChanged(QListWidgetItem *item)
{
    if (item->checkState()==2) {
        emit pluginEnabled(ui->pluginsList->row(item));
    } else {
        emit pluginDisabled(ui->pluginsList->row(item));
    }
}

