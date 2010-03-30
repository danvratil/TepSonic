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

#include "databasemanager.h"

#include <QDebug>
#include <QDir>
#include <QSettings>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>


DatabaseManager::DatabaseManager(QString connectionName)
{
    _connectionName = connectionName;

    QSettings settings(QString(QDir::homePath()).append("/.tepsonic/main.conf"),QSettings::IniFormat,this);
    _driverType = (DriverTypes)settings.value("Collections/StorageEngine",0).toInt();
    settings.beginGroup("Collections");
    settings.beginGroup("MySQL");
    _server = settings.value("Server","127.0.0.1").toString();
    _username = settings.value("Username",QString()).toString();
    _password = settings.value("Password",QString()).toString();
    _db = settings.value("Database","tepsonic").toString();
    settings.endGroup();
    settings.endGroup();;

}

DatabaseManager::~DatabaseManager()
{
    sqlDb.close();
    QSqlDatabase::removeDatabase(_connectionName);
}

bool DatabaseManager::connectToDB()
{
    if (QSqlDatabase::contains(_connectionName)) {
        qDebug() << "Connection with name " << _connectionName << " exists.";
        return true;
    }

    switch (_driverType) {
        case MySQL: {
            sqlDb = QSqlDatabase::addDatabase("QMYSQL",_connectionName);
            sqlDb.setHostName(_server);
            sqlDb.setUserName(_username);
            sqlDb.setPassword(_password);
            sqlDb.setDatabaseName(_db);
        } break;
        case SQLite: {
            sqlDb = QSqlDatabase::addDatabase("QSQLITE",_connectionName);
            sqlDb.setDatabaseName(QString(QDir::homePath()).append("/.tepsonic/collection.db"));
        } break;
    }

    if (!sqlDb.open()) {
        qDebug() << "Failed to establish '" << sqlDb.connectionName() <<"' connection to database!";
        qDebug() << "Reason: " << sqlDb.lastError().text();
        return false;
    }

    // We want to use UTF8!!!
    if (_driverType == MySQL ) {
        QSqlQuery query("SET CHARACTER SET utf8;",sqlDb);
    }

    if (!sqlDb.tables(QSql::Tables).contains("Tracks",Qt::CaseSensitive)) {
        initDb();
    }

    return true;
}

void DatabaseManager::initDb()
{
    qDebug() << "Initializing database structure";
    switch (_driverType) {
        case MySQL: {
            QSqlQuery query("CREATE TABLE IF NOT EXISTS `Tracks` (" \
                            "   `filename` varchar(300) NOT NULL," \
                            "   `trackNo` int(11) NOT NULL DEFAULT '0'," \
                            "   `artist` varchar(120) NOT NULL," \
                            "   `album` varchar(120) NOT NULL," \
                            "   `title` varchar(300) NOT NULL," \
                            "   `mtime` int(10) unsigned NOT NULL DEFAULT '0'," \
                            "   PRIMARY KEY (`filename`)," \
                            "   KEY `artist` (`artist`)," \
                            "   KEY `album` (`album`)"  \
                            ") ENGINE=MyISAM DEFAULT CHARSET=utf8;",sqlDb);
        } break;
        case SQLite: {
            QSqlQuery query("CREATE TABLE Tracks("\
                            "   filename TEXT," \
                            "   trackNo INTEGER," \
                            "   artist TEXT," \
                            "   album TEXT," \
                            "   title TEXT," \
                            "   mtime INTEGER,"\
                            "   PRIMARY KEY(filename ASC));",sqlDb);
        } break;
    }
}