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

#ifndef TRAYICON_H
#define TRAYICON_H

#include <QSystemTrayIcon>

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

  public:
    explicit TrayIcon(QObject *parent = 0);
    explicit TrayIcon(const QIcon &icon, QObject *parent = 0);

  protected:
    bool event(QEvent *event);

};

#endif // TRAYICON_H
