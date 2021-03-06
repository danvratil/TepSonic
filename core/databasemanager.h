/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <me@dvratil.cz>
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

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSettings>
#include <QSqlDatabase>

#include "tepsonic-core-export.h"

// Current DB revision
/* When connecting to DB, we automatically checked for DB revision. When the numbers
   don't match, the DB is dropped and collections are rebuilt
*/
#define _DBREVISION "8"

namespace TepSonic
{

// TODO: Kill this class and manage collections directly through collectionsModel
class TEPSONIC_CORE_EXPORT DatabaseManager
{

  public:
    static DatabaseManager* instance();
    ~DatabaseManager();

    const QSqlDatabase& sqlDb() const {
        return m_sqlDb;
    }

    bool connectionAvailable() {
        return m_connectionAvailable;
    }

    void forceConnectionAvailable(bool forceState = true) {
        m_connectionAvailable = forceState;
    }

  private:
    explicit DatabaseManager();
    static DatabaseManager *s_instance;

    void initDb();
    void connectToDB();

    QString m_dbFile;
    QSqlDatabase m_sqlDb;

    bool m_connectionAvailable;

};

} // namespace TepSonic

#endif // DATABASEMANAGER_H
