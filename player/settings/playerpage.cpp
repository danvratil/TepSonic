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
#include "playereffectdialog.h"
#include "player.h"

#include <QStandardItem>
#include <Phonon/BackendCapabilities>
#include <Phonon/Effect>
#include <Phonon/EffectDescription>

extern Player *player;

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


    m_effectsModel = new QStandardItemModel();
    QList<Phonon::Effect*> *effects = player->effects();
    for (int i = 0; i < effects->length(); i++)
    {
        Phonon::EffectDescription ed = effects->at(i)->description();
        QStandardItem *item = new QStandardItem(ed.name());
        item->setData(QVariant(ed.index()), Qt::UserRole);
        item->setData(QVariant(quintptr(effects->at(i))), Qt::UserRole+1);
        item->setCheckable(true);
        m_effectsModel->appendRow(item);
    }
    ui->playerEffectsList->setModel(m_effectsModel);
    connect(ui->playerEffectsList, SIGNAL(clicked(QModelIndex)),
            this, SLOT(setEffectDescription(QModelIndex)));
    connect(ui->playerEffectsList, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(showEffectSettings(QModelIndex)));
}

PlayerPage::~PlayerPage()
{
    delete m_devicesModel;
    delete m_effectsModel;
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

void PlayerPage::setEffectDescription(QModelIndex effect)
{
    Phonon::Effect *peffect = (Phonon::Effect*)effect.data(Qt::UserRole+1).toLongLong();
    Phonon::EffectDescription description = peffect->description();
    ui->effectDescription->setText(description.description());
}

void PlayerPage::showEffectSettings(QModelIndex effect)
{
    int index = effect.data(Qt::UserRole).toInt();

    QList<Phonon::EffectDescription> effects = Phonon::BackendCapabilities::availableAudioEffects();
    for (int i = 0; i < effects.length(); i++)
    {
        if (effects.at(i).index() == index) {
            // Dialog deletes itself automatically when closed
            PlayerEffectDialog *playerEffectDialog = new PlayerEffectDialog((Phonon::Effect*)(effect.data(Qt::UserRole+1).toLongLong()));
            playerEffectDialog->open();
        }
    }
}
