/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <dan@progdan.cz>
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
#include <Phonon/EffectWidget>

using namespace SettingsPages;

PlayerPage::PlayerPage(QWidget *parent):
    SettingsPage(parent)
{
    m_ui = new Ui::PlayerPage();
    m_ui->setupUi(this);

    m_devicesModel = new QStandardItemModel();

    QList<Phonon::AudioOutputDevice> devices = Phonon::BackendCapabilities::availableAudioOutputDevices();
    for (int i = 0; i < devices.length(); i++) {
        QStandardItem *item = new QStandardItem(devices.at(i).name());
        item->setData(QVariant(devices.at(i).index()), Qt::UserRole);
        m_devicesModel->appendRow(item);
    }
    m_ui->outputDevicesList->setModel(m_devicesModel);


    m_effectsModel = new QStandardItemModel();
    const QList<Phonon::Effect *> effects = Player::instance()->effects();
    for (int i = 0; i < effects.count(); i++) {
        Phonon::EffectDescription ed = effects.at(i)->description();
        QStandardItem *item = new QStandardItem(ed.name());
        item->setData(QVariant(ed.index()), Qt::UserRole);
        item->setData(QVariant(quintptr(effects.at(i))), Qt::UserRole + 1);
        item->setCheckable(true);
        m_effectsModel->appendRow(item);
    }
    m_ui->playerEffectsList->setModel(m_effectsModel);
    connect(m_ui->playerEffectsList, SIGNAL(clicked(QModelIndex)),
            this, SLOT(setEffectDescription(QModelIndex)));
    connect(m_ui->playerEffectsList, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(showEffectSettings(QModelIndex)));
}

PlayerPage::~PlayerPage()
{
    delete m_ui;
    delete m_devicesModel;
    delete m_effectsModel;
}

QModelIndex PlayerPage::getOutputDeviceModelIndex(int deviceIndex)
{
    for (int i = 0; i < m_devicesModel->rowCount(); i++) {
        if (m_devicesModel->item(i, 0)->data(Qt::UserRole).toInt() == deviceIndex) {
            return m_devicesModel->index(i, 0);
        }
    }

    return QModelIndex();
}

int PlayerPage::getOutputDeviceIndex(const QModelIndex &index) const
{
    return m_devicesModel->data(index, Qt::UserRole).toInt();
}

bool PlayerPage::outputDeviceChanged() const
{
    return (getOutputDeviceIndex(m_ui->outputDevicesList->currentIndex()) != m_oldOutputDeviceIndex);
}

void PlayerPage::setEffectDescription(const QModelIndex &effect)
{
    Phonon::Effect *peffect = (Phonon::Effect *)effect.data(Qt::UserRole + 1).toLongLong();
    const Phonon::EffectDescription description = peffect->description();
    m_ui->effectDescription->setText(description.description());
}

void PlayerPage::showEffectSettings(const QModelIndex &effect)
{
    int index = effect.data(Qt::UserRole).toInt();

    const QList<Phonon::EffectDescription> effects = Phonon::BackendCapabilities::availableAudioEffects();
    for (int i = 0; i < effects.length(); i++) {
        if (effects.at(i).index() == index) {
            // Dialog deletes itself automatically when closed
            PlayerEffectDialog *playerEffectDialog = new PlayerEffectDialog((Phonon::Effect *)(effect.data(Qt::UserRole + 1).toLongLong()));
            playerEffectDialog->open();
        }
    }
}

void PlayerPage::loadSettings(QSettings *settings)
{
    settings->beginGroup(QLatin1String("Preferences"));
    m_ui->restorePreviousSessionCheckBox->setChecked(settings->value(QLatin1String("RestoreSession"), true).toBool());
    m_ui->outputDevicesList->setCurrentIndex(getOutputDeviceModelIndex(settings->value(QLatin1String("OutputDevice"), 0).toInt()));

    m_oldOutputDeviceIndex = settings->value(QLatin1String("OutputDevice"), 0).toInt();

    const QList<Phonon::Effect *> effects = Player::instance()->effects();
    for (int i = 0; i < effects.count(); i++) {
        const Phonon::EffectDescription ed = effects.at(i)->description();
        const bool state = settings->value(QLatin1String("Effects/") + ed.name(), false).toBool();
        if (state) {
            qobject_cast<QStandardItemModel *>(m_ui->playerEffectsList->model())->item(i, 0)->setCheckState(Qt::Checked);
        }
    }
    settings->endGroup();
}

void PlayerPage::saveSettings(QSettings *settings)
{
    settings->beginGroup(QLatin1String("Preferences"));
    settings->setValue(QLatin1String("RestoreSession"), m_ui->restorePreviousSessionCheckBox->isChecked());
    settings->setValue(QLatin1String("OutputDevice"), getOutputDeviceIndex(m_ui->outputDevicesList->currentIndex()));

    const QList<Phonon::Effect *> effects = Player::instance()->effects();
    for (int i = 0; i < effects.count(); i++) {
        Phonon::EffectDescription ed = effects.at(i)->description();
        settings->setValue(QLatin1String("Effects/") + ed.name(),
                           qobject_cast<QStandardItemModel *>(m_ui->playerEffectsList->model())->item(i, 0)->checkState() == Qt::Checked);
        Player::instance()->enableEffect(effects.at(i),
                                         (qobject_cast<QStandardItemModel *>(m_ui->playerEffectsList->model())->item(i, 0)->checkState() == Qt::Checked));
    }
    settings->endGroup();
}

#include "moc_playerpage.cpp"
