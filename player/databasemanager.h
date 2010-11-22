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

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSettings>
#include <QtSql/QSqlDatabase>


// Current DB revision
/* When connecting to DB, we automatically checked for DB revision. When the numbers
   don't match, the DB is dropped and collections are rebuilt
*/
#define _DBREVISION "7";

//! DatabaseManager handles connection to database
/*!
  DatabaseManager provides simple interface for Collections to establish connection
  to database storage backend.
  It can be easily used by plugins that want to store some data (statistics etc) in database.
  However the DatabaseManager does not take care of presence of the plugin's table in the database. Plugin must
  check this itself.
*/
class DatabaseManager : public QObject
{
    Q_OBJECT
    Q_ENUMS(DriverTypes)

    public:
        //! Enumerates supported database storage backends.
        /*!
          DriverTypes provide enumeration of supported drivers. Manager sets currently
          used driver type according to user settings. The current driver type can be obtained
          by method driverType().
          \sa driverType()
        */
        enum DriverTypes { SQLite, MySQL };

        //! Constructor
        /*!
          Initialize manager, set which db backend will be used
          \param connectionName uniq indentificator of the connection
        */
        DatabaseManager(QString connectionName = "");

        //! Destructor
        ~DatabaseManager();

        //! Attempts to establish connection to database
        /*!
          \return Returns true on success, false when fails to connect.
        */
        bool connectToDB();

        //! Returns type of database storage backend that will be used.
        /*!
          This can be usefull especially for more complex queries that may differ in syntax
          in various databases (SQLite vs MySQL...)
          \return Returns type of database storage backend that is used.
        */
        DriverTypes driverType()  { return m_driverType; };

        //! Returns QSqlDatabase connection object
        QSqlDatabase* sqlDb() { return m_sqlDb; }

        //! Returns wheter connection is now available or not
        static bool connectionAvailable() { return m_static_connectionAvailable; }

        //! Force new state of connection availability
        static void forceConnectionAvailable(bool forceState = true) { m_static_connectionAvailable = forceState; }

    private:
        //! Check if the database contains all required tables and if not creates them
        /*!
          Checks wheter the structure of tables is up-to-date and all tables are present. When not, it's recreated.
        */
        void initDb();

        //! Unique identification of the connection
        QString m_connectionName;

        //! Driver type that is used in this session
        /*!
          \sa driverType()
        */
        DriverTypes m_driverType;

        //! Database server hostname or IP
        QString m_server;

        //! Database server username
        QString m_username;

        //! Database server password
        QString m_password;

        //! Database
        QString m_db;

        //! SQL database
        QSqlDatabase *m_sqlDb;

        //! Is connection available?
        static bool m_static_connectionAvailable;

        bool m_connectionAvailable;


};



#endif // DATABASEMANAGER_H
