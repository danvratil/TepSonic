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


#ifndef ADDBOOKMARKDLG_H
#define ADDBOOKMARKDLG_H

#include <QDialog>

namespace Ui {
    class AddBookmarkDlg;
}

class AddBookmarkDlg : public QDialog
{
    Q_OBJECT
    public:
        explicit AddBookmarkDlg(QString path, QWidget *parent = 0);

    signals:
        void accepted(QString title, QString path);

    private slots:
        void emitAccepted();

    private:
        Ui::AddBookmarkDlg *m_ui;

};

#endif // ADDBOOKMARKDLG_H
