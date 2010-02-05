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
#include <QMutex>
#include <QSettings>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>


DatabaseManager::DatabaseManager(QString connectionName)
{
    _connectionName = connectionName;
}

DatabaseManager::~DatabaseManager()
{
    sqlDb.close();
    sqlDb.removeDatabase(_connectionName);
}

void DatabaseManager::connectToDB()
{
    QMutex mutex;
    mutex.lock();

    if (QSqlDatabase::contains(_connectionName)) {
        qDebug() << "Connection with name " << _connectionName << " exists.";
        mutex.unlock();
        return;
    }


    QSettings settings(QString(QDir::homePath()).append("/.tepsonic/main.conf"),QSettings::IniFormat,this);

    QString driver;
    switch (settings.value("Collections/StorageEngine",0).toInt()) {
        case 1:
           driver = "QMYSQL";
           break;
        default:
        case 0:
           driver = "QSQLITE";
           break;

    }
    sqlDb = QSqlDatabase::addDatabase(driver,_connectionName);
    if (driver == "QMYSQL") {
        settings.beginGroup("Collections");
        settings.beginGroup("MySQL");
        sqlDb.setHostName(settings.value("Server","localhost").toString());
        sqlDb.setUserName(settings.value("Username",QString()).toString());
        sqlDb.setPassword(settings.value("Password",QString()).toString());
        sqlDb.setDatabaseName(settings.value("Database","tepsonic").toString());
        settings.endGroup();
        settings.endGroup();
    } else {
        sqlDb.setDatabaseName(QString(QDir::homePath()).append("/.tepsonic/collection.db"));
    }

    if (!sqlDb.open()) {
        qDebug() << "Failed to connect to database!" << sqlDb.lastError().text();
        return;
    }

    if (driver=="QMYSQL") {
        QSqlQuery query("SHOW TABLES;",sqlDb);
        if (query.size()>0) query.first();
        if (query.value(0).toString()!="Tracks") {
            initDb(DatabaseManager::MySQL);
        }
    }

    mutex.unlock();
    return;
}

void DatabaseManager::initDb(DatabaseManager::DBType dbType)
{
    qDebug() << "Initializing database structure";
    switch (dbType) {
        case DatabaseManager::MySQL: {
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
        case DatabaseManager::SQLite:
            break;
    }
}
