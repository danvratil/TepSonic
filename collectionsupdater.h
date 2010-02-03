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


#ifndef COLLECTIONSUPDATER_H
#define COLLECTIONSUPDATER_H

#include <QThread>
#include <QObject>

#include <QSettings>

class CollectionsUpdater : public QThread
{
    Q_OBJECT
    public:
        CollectionsUpdater(QSettings *settings);
        ~CollectionsUpdater();
        void run();

    private:
        QSettings *_settings;

    signals:
        // Returns TRUE when an update was done, FALSE when the table was not altered
        void collectionsUpdated(bool updated);
};

#endif // COLLECTIONSUPDATER_H
