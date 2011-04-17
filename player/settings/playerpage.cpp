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

#include "playerpage.h"
#include "ui_playerpage.h"

#include <QStandardItem>
#include <Phonon/BackendCapabilities>

using namespace SettingsPages;

PlayerPage::PlayerPage(QWidget *parent):
        QWidget(parent)
{
    ui = new Ui::PlayerPage();
    ui->setupUi(this);

    m_devicesModel = new QStandardItemModel();

    QList<Phonon::AudioOutputDevice> devices = Phonon::BackendCapabilities::availableAudioOutputDevices();
    for (int i = 0; i < devices.length(); i++)
    {
        QStandardItem *item = new QStandardItem(devices.at(i).name());
        item->setData(QVariant(devices.at(i).index()), Qt::UserRole);
        m_devicesModel->appendRow(item);
    }
    ui->outputDevicesList->setModel (m_devicesModel);
}

PlayerPage::~PlayerPage()
{
    delete m_devicesModel;
}

QModelIndex PlayerPage::getOutputDeviceModelIndex(int deviceIndex)
{
    for (int i = 0; i < m_devicesModel->rowCount(); i++)
    {
        if (m_devicesModel->item(i, 0)->data(Qt::UserRole).toInt() == deviceIndex)
            return m_devicesModel->index(i, 0);
    }

    return QModelIndex();
}

int PlayerPage::getOutputDeviceIndex (QModelIndex index)
{
    return m_devicesModel->data(index, Qt::UserRole).toInt();
}
