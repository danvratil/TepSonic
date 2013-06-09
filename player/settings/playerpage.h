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

#ifndef PLAYERPAGE_H
#define PLAYERPAGE_H

#include <QStandardItemModel>

#include <Phonon/Effect>

#include "settingspage.h"

namespace Ui
{
class PlayerPage;
}

namespace SettingsPages
{

class PlayerPage : public SettingsPage
{
    Q_OBJECT

  public:
    explicit PlayerPage(QWidget *parent = 0);
    ~PlayerPage();

    QModelIndex getOutputDeviceModelIndex(int deviceIndex);
    int getOutputDeviceIndex(const QModelIndex &index) const;
    bool outputDeviceChanged() const;

    int effectsCount() const {
        return m_effectsModel->rowCount();
    }

  public Q_SLOTS:
    void loadSettings(QSettings *settings);
    void saveSettings(QSettings *settings);

  private:
    ::Ui::PlayerPage *m_ui;

    QStandardItemModel *m_devicesModel;
    QStandardItemModel *m_effectsModel;

    int m_oldOutputDeviceIndex;

  private Q_SLOTS:
    void setEffectDescription(const QModelIndex &effect);
    void showEffectSettings(const QModelIndex &effect);

  Q_SIGNALS:
    void effectEnabled(const Phonon::EffectDescription &description);
    void effectDiasabled(Phonon::Effect *effect);
};
}

#endif // PLAYERPAGE_H
