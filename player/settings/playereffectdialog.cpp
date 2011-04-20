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

#include "playereffectdialog.h"

PlayerEffectDialog::PlayerEffectDialog(Phonon::Effect* effect)
{
    setWindowTitle(tr("Effect settings"));
    setAttribute(Qt::WA_DeleteOnClose);

    m_effectWidget = new Phonon::EffectWidget(effect, this);

    m_layout = new QVBoxLayout(this);
    m_layout->addWidget(m_effectWidget);
    setLayout(m_layout);
}

PlayerEffectDialog::~PlayerEffectDialog()
{
    delete m_effectWidget;
    delete m_layout;
}