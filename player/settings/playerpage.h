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

#ifndef PLAYERPAGE_H
#define PLAYERPAGE_H

#include <QWidget>
#include <QStandardItemModel>
#include <QCloseEvent>

#include <Phonon/Effect>
#include <Phonon/EffectWidget>

namespace Ui {
    class PlayerPage;
}

namespace SettingsPages {



    class PlayerPage : public QWidget
    {
        Q_OBJECT
        public:
            explicit PlayerPage(QWidget *parent = 0);
            ~PlayerPage();
            ::Ui::PlayerPage *ui;

            QModelIndex getOutputDeviceModelIndex(int deviceIndex);
            int getOutputDeviceIndex(QModelIndex index);
            bool outputDeviceChanged();

            int effectsCount() { return m_effectsModel->rowCount(); }

        private:
            QStandardItemModel *m_devicesModel;
            QStandardItemModel *m_effectsModel;

        private slots:
            void setEffectDescription(QModelIndex effect);
            void showEffectSettings(QModelIndex effect);

        signals:
            void effectEnabled(Phonon::EffectDescription description);
            void effectDiasabled(Phonon::Effect *effect);
    };

}

#endif // PLAYERPAGE_H
