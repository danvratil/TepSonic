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


#include "changeshortcutdialog.h"
#include "ui_changeshortcutdialog.h"

ChangeShortcutDialog::ChangeShortcutDialog(QModelIndex index, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeShortcutDialog)
{
    _index = index;


    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(rejected()),
            this, SLOT(close()));
    connect(ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    ui->currentShortcut->setText(index.sibling(index.row(),1).data().toString());

    // List legal modifiers
    _modifiers << Qt::ControlModifier << Qt::AltModifier << Qt::MetaModifier << Qt::ShiftModifier;

}

ChangeShortcutDialog::~ChangeShortcutDialog()
{
    delete ui;
}

void ChangeShortcutDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ChangeShortcutDialog::keyReleaseEvent(QKeyEvent *e)
{
    if (_modifiers.contains(e->modifiers())) {
        _shortcut = QKeySequence(e->modifiers() + e->key());
        ui->currentShortcut->setText(_shortcut.toString(QKeySequence::NativeText));
    }
}

void ChangeShortcutDialog::accept()
{
    emit shortcutChanged(_index, _shortcut);
    close();
}
