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

#include "infopanel.h"
#include <QTimer>

InfoPanel::InfoPanel(QWidget *parent = 0)
        :QWidget(parent)
{

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    layout = new QHBoxLayout(this);

    switch (mode) {
        case ErrorMode:
            icon = new QIcon(":/icons/quit");
            break;
        case InfoMode:
            icon = new QIcon(":/icons/about");
            break;
    }

    iconButton = new QPushButton(this);
    iconButton->setIcon(*icon);
    iconButton->setText("");
    layout->addWidget(iconButton);

    spacer = new QSpacerItem(100,0,QSizePolicy::Expanding,QSizePolicy::Fixed);
    layout->addSpacerItem(spacer);

    label = new QLabel(this);
    layout->addWidget(label);

}

void InfoPanel::setMode(InfoPanel::PanelMode newMode)
{
    mode = newMode;
}

void InfoPanel::setMessage(QString msg)
{
    label->setText(msg);
}

void InfoPanel::show()
{
    QTimer::singleShot(5000,this,SLOT(hide()));
    // = show
    this->setHidden(false);
}
