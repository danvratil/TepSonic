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

#include "trayicon.h"

#include <QEvent>
#include <QWheelEvent>

TrayIcon::TrayIcon(QObject *parent):
    QSystemTrayIcon(parent)
{
    setParent(parent);
}

TrayIcon::TrayIcon(const QIcon &icon, QObject* parent):
    QSystemTrayIcon(parent)
{
    setIcon(icon);
    setParent(parent);
}

bool TrayIcon::event(QEvent *event)
{
    if (event->type() == QEvent::Wheel) {
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
        emit mouseWheelScrolled(wheelEvent->delta());
        event->accept();
        return true;
    }
    if (event->type() == QEvent::ToolTip) {
        event->accept();
        return true;
    }

    return false;
}
