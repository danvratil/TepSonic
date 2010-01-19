/*
 * TEPSONIC
 * Copyright 2009 Dan Vratil <vratil@progdansoft.com>
 * Copyright 2009 Petr Los <petr_los@centrum.cz>
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


#ifndef INFOPANEL_H
#define INFOPANEL_H

#include <QWidget>

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>

class InfoPanel : public QWidget
{
    Q_OBJECT
    Q_ENUMS(PanelMode)

    public:
        enum PanelMode { ErrorMode, InfoMode };
        InfoPanel(QWidget *parent);
        ~InfoPanel() { }
        void setMessage(QString msg);
        void show();
        void setMode(PanelMode newMode);

    private:

        QHBoxLayout *layout;
        QIcon *icon;
        QLabel *label;
        QPushButton *iconButton;
        QSpacerItem *spacer;

        PanelMode mode;

};

#endif // INFOPANEL_H
