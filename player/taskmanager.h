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

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>
#include <QThreadPool>
#include <QModelIndex>

#include <core/player.h>

class CollectionBuilder;

class TaskManager : public QObject
{
    Q_OBJECT

  public:
    static TaskManager *instance();
    void destroy();

    ~TaskManager();

  Q_SIGNALS:
    void collectionsRebuilt();
    void taskDone();
    void taskStarted(const QString &action);

  public Q_SLOTS:
    void rebuildCollections(const QString &folder = QString());

  private:
    TaskManager();
    static TaskManager *s_instance;

    QThreadPool *m_collectionsThreadPool;
};

#endif // TASKMANAGER_H
