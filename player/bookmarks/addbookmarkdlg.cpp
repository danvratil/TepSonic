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

#include "addbookmarkdlg.h"
#include "ui_addbookmarkdlg.h"

AddBookmarkDlg::AddBookmarkDlg(const QString &path, QWidget *parent) :
    QDialog(parent)
{
    m_ui = new Ui::AddBookmarkDlg;
    m_ui->setupUi(this);

    m_ui->titleEdit->setText(path);
    m_ui->pathEdit->setText(path);

    connect(this, SIGNAL(accepted()),
            this, SLOT(emitAccepted()));
}

AddBookmarkDlg::~AddBookmarkDlg()
{
    delete m_ui;
}

void AddBookmarkDlg::emitAccepted()
{
    Q_EMIT accepted(m_ui->titleEdit->text(),
                    m_ui->pathEdit->text());
}

#include "moc_addbookmarkdlg.cpp"
