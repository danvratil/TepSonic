/* TEPSONIC
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


#ifndef COLLECTIONSMANAGER_H
#define COLLECTIONSMANAGER_H

#include <QObject>
#include <QSettings>

#include <QtSql/QSqlDatabase>

class CollectionsUpdater;

class CollectionsManager : public QObject
{
    Q_OBJECT
    Q_ENUMS(DBType);
    public:
        enum DBType { SQLite, MySQL };
        CollectionsManager(QSettings *settings, QObject *parent = 0);
        ~CollectionsManager();

        QSqlDatabase sqlDb;

    public slots:
        void updateCollections();
        void collectionsUpdated(bool updated);

    signals:
        void collectionChanged();

    private:
        QSettings *_settings;

        CollectionsUpdater *_updater;

        void initDb(CollectionsManager::DBType dbType);


};

#endif // COLLECTIONSMANAGER_H
