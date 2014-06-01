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

#ifndef TEPSONIC_ACTIONMANAGER_H
#define TEPSONIC_ACTIONMANAGER_H

#include <QMap>
#include <QString>
#include <QIcon>
#include <QKeySequence>

#include "tepsonic-core-export.h"

class QAction;
class QMenu;

namespace TepSonic
{

class TEPSONIC_CORE_EXPORT ActionManager
{
  public:
    static ActionManager *instance();
    ~ActionManager();

    QAction* action(const QString &name) const;
    QAction* addAction(const QString &name, QAction *action);
    QAction* addAction(const QString &name, const QString &title,
                          const QIcon &icon = QIcon(),
                          const QKeySequence &shortcut = QKeySequence());

    QMenu* menu(const QString &menu) const;
    void addMenu(const QString &name, QMenu *menu);

  private:
    ActionManager();

    class Private;
    Private * const d;
};

} // namespace TepSonic

#endif // TEPSONIC_ACTIONMANAGER_H