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

#include "changeshortcutdialog.h"
#include "ui_changeshortcutdialog.h"

using namespace SettingsPages;

ChangeShortcutDialog::ChangeShortcutDialog(const QModelIndex &index, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ChangeShortcutDialog),
    m_index(index)
{
    m_ui->setupUi(this);
    connect(m_ui->buttonBox, SIGNAL(rejected()),
            this, SLOT(close()));
    connect(m_ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    m_ui->currentShortcut->setText(index.sibling(index.row(), 1).data().toString());

    // List legal modifiers
    m_modifiers << Qt::ControlModifier << Qt::AltModifier << Qt::MetaModifier << Qt::ShiftModifier;
}

ChangeShortcutDialog::~ChangeShortcutDialog()
{
    delete m_ui;
}

void ChangeShortcutDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ChangeShortcutDialog::keyReleaseEvent(QKeyEvent *e)
{
    if (m_modifiers.contains(e->modifiers())) {
        m_shortcut = QKeySequence(e->modifiers() + e->key());
        m_ui->currentShortcut->setText(m_shortcut.toString(QKeySequence::NativeText));
    }
}

void ChangeShortcutDialog::accept()
{
    Q_EMIT shortcutChanged(m_index, m_shortcut);
    close();
}
