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
#include "constants.h"

#include <QDebug>
#include <QDir>
#include <QSettings>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>


DatabaseManager::DatabaseManager(QString connectionName)
{
    _connectionName = connectionName;
    _sqlDb = NULL;

    QSettings settings(QString(_CONFIGDIR).append("/main.conf"),QSettings::IniFormat,this);
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
    if (_sqlDb != NULL)
        delete _sqlDb;
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
        _sqlDb = new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL",_connectionName));
        _sqlDb->setHostName(_server);
        _sqlDb->setUserName(_username);
        _sqlDb->setPassword(_password);
        _sqlDb->setDatabaseName(_db);
    } break;
    case SQLite: {
        _sqlDb = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE",_connectionName));
        _sqlDb->setDatabaseName(QString(_CONFIGDIR).append("/collection.db"));
    } break;
    }

    if (!_sqlDb->open()) {
        qDebug() << "Failed to establish '" << _sqlDb->connectionName() <<"' connection to database!";
        qDebug() << "Reason: " << _sqlDb->lastError().text();
        _sqlDb = NULL;
        return false;
    }

    // We want to use UTF8!!!
    if (_driverType == MySQL ) {
        QSqlQuery query("SET CHARACTER SET utf8;",*_sqlDb);
    }

    QStringList tables = _sqlDb->tables(QSql::AllTables);
    if (!(tables.contains("albums") &&
          tables.contains("genres") &&
          tables.contains("interprets") &&
          tables.contains("tracks") &&
          tables.contains("years") &&
          tables.contains("view_tracks"))) {
        initDb();
    }

    return true;
}

void DatabaseManager::initDb()
{
    qDebug() << "Initializing database structure";
    switch (_driverType) {
    case MySQL: {
        QSqlQuery query(*_sqlDb);

        query.exec("DROP TABLE IF EXISTS `albums`,`genres`,`interprets`,`tracks`,`years`;");
        query.exec("DROP VIEW `view_tracks`;");

        query.exec("CREATE TABLE `albums` (" \
                   "   `id` int(11) NOT NULL AUTO_INCREMENT," \
                   "   `album` varchar(250) NOT NULL," \
                   "   `tracksCnt` int(11) NOT NULL DEFAULT '0'," \
                   "   `totalLength` int(11) NOT NULL DEFAULT '0'," \
                   "   PRIMARY KEY (`id`)" \
                   ") ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;");

        query.exec("CREATE TABLE `genres` (" \
                   "   `id` int(11) NOT NULL AUTO_INCREMENT," \
                   "   `genre` varchar(80) NOT NULL," \
                   "   PRIMARY KEY (`id`)" \
                   ") ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;");

        query.exec("CREATE TABLE `interprets` (" \
                   "   `id` int(11) NOT NULL AUTO_INCREMENT," \
                   "   `interpret` varchar(300) NOT NULL," \
                   "   `albumsCnt` int(11) NOT NULL DEFAULT '0'," \
                   "   `totalLength` int(11) NOT NULL DEFAULT '0'," \
                   "   PRIMARY KEY (`id`)" \
                   ") ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;");

        query.exec("CREATE TABLE `tracks` (" \
                   "   `id` int(11) NOT NULL AUTO_INCREMENT," \
                   "   `filename` text NOT NULL," \
                   "   `trackname` varchar(300) NOT NULL," \
                   "   `track` int(11) NOT NULL," \
                   "   `length` int(11) unsigned NOT NULL," \
                   "   `interpret` int(11) NOT NULL," \
                   "   `album` int(11) NOT NULL," \
                   "   `genre` int(11) NOT NULL," \
                   "   `year` int(11) NOT NULL," \
                   "   `mtime` int(11) unsigned NOT NULL," \
                   "   PRIMARY KEY (`id`)," \
                   "   KEY `interpret` (`interpret`,`album`,`year`,`genre`)" \
                   ") ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;");

        query.exec("CREATE TABLE `years` (" \
                   "   `id` int(11) NOT NULL AUTO_INCREMENT," \
                   "   `year` int(11) NOT NULL," \
                   "   PRIMARY KEY (`id`)" \
                   ") ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;");

        query.exec("CREATE VIEW `view_tracks` AS" \
                   "   SELECT `tracks`.`id` AS `id`," \
                   "          `tracks`.`filename` AS `filename`," \
                   "          `tracks`.`trackname` AS `trackname`," \
                   "          `tracks`.`track` AS `track`," \
                   "          `tracks`.`length` AS `length`," \
                   "          `tracks`.`album` AS `albumID`," \
                   "          `tracks`.`interpret` AS `interpretID`," \
                   "          `interprets`.`interpret` AS `interpret`," \
                   "          `genres`.`genre` AS `genre`," \
                   "          `albums`.`album` AS `album`," \
                   "          `years`.`year` AS `year`" \
                   "   FROM `tracks`" \
                   "   LEFT JOIN `interprets` ON `tracks`.`interpret` = `interprets`.`id`" \
                   "   LEFT JOIN `genres` ON `tracks`.`genre` = `genres`.`id`" \
                   "   LEFT JOIN `albums` ON `tracks`.`album` = `albums`.`id`" \
                   "   LEFT JOIN `years` ON `tracks`.`year` = `years`.`id`;");

    } break;
    case SQLite: {
        QSqlQuery query(*_sqlDb);

        query.exec("DROP TABLE IF EXISTS `albums`;");
        query.exec("DROP TABLE IF EXISTS `genres`;");
        query.exec("DROP TABLE IF EXISTS ``interprets`;");
        query.exec("DROP TABLE IF EXISTS `tracks`;");
        query.exec("DROP TABLE IF EXISTS `years`;");
        query.exec("DROP VIEW IF EXISTS `view_tracks`;");

        query.exec("CREATE TABLE `genres` (" \
                   "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                   "    `genre` TEXT NOT NULL);");

        query.exec("CREATE TABLE `interprets` (" \
                   "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                   "    `interpret` TEXT NOT NULL," \
                   "    `albumsCnt` INTEGER NOT NULL DEFAULT(0)," \
                   "    `totalLength` INTEGER NOT NULL DEFAULT(0));");

        query.exec("CREATE TABLE `albums` (" \
                   "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                   "    `album` TEXT NOT NULL," \
                   "    `tracksCnt` INTEGER NOT NULL DEFAULT(0)," \
                   "    `totalLength` INTEGER NOT NULL DEFAULT(0));");

        query.exec("CREATE TABLE `tracks` (" \
                   "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                   "    `filename` TEXT NOT NULL," \
                   "    `trackname` TEXT NOT NULL," \
                   "    `track` INTEGER," \
                   "    `length` INTEGER NULL," \
                   "    `interpret` INTEGER," \
                   "    `album` INTEGER," \
                   "    `genre` INTEGER," \
                   "    `year` INTEGER, "\
                   "    `mtime` INTEGER);");

        query.exec("CREATE TABLE `years` (" \
                   "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                   "    `year` INTEGER NOT NULL);");

        query.exec("CREATE VIEW `view_tracks` AS" \
                   "    SELECT `tracks`.`id`," \
                   "           `tracks`.`filename`," \
                   "           `tracks`.`trackname`," \
                   "           `tracks`.`track`," \
                   "           `tracks`.`length`," \
                   "           `tracks`.`album` AS `albumID`," \
                   "           `tracks`.`interpret` AS `interpretID`," \
                   "           `interprets`.`interpret`," \
                   "           `genres`.`genre`," \
                   "           `albums`.`album`," \
                   "           `years`.`year`" \
                   "    FROM `tracks`" \
                   "    LEFT JOIN `interprets` ON `tracks`.`interpret` = `interprets`.`id`" \
                   "    LEFT JOIN `genres` ON `tracks`.`genre` = `genres`.`id`" \
                   "    LEFT JOIN `albums` ON `tracks`.`album` = `albums`.`id`" \
                   "    LEFT JOIN `years` ON `tracks`.`year` = `years`.`id`;");

    } break;
    }
}
