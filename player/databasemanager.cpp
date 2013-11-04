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

#include "databasemanager.h"
#include "constants.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QMutex>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>


DatabaseManager* DatabaseManager::s_instance = 0;

DatabaseManager *DatabaseManager::instance()
{
    static QMutex dbLock;

    QMutexLocker locker(&dbLock);
    if (s_instance == 0) {
        s_instance = new DatabaseManager();
    }

    return s_instance;
}


DatabaseManager::DatabaseManager()
{
    QSettings settings(QString(_CONFIGDIR).append(QLatin1String("/main.conf")), QSettings::IniFormat);
    m_driverType = (DriverTypes)settings.value(QLatin1String("Collections/StorageEngine"), 0).toInt();
    settings.beginGroup(QLatin1String("Collections"));
    settings.beginGroup(QLatin1String("MySQL"));
    m_server = settings.value(QLatin1String("Server"), QLatin1String("127.0.0.1")).toString();
    m_username = settings.value(QLatin1String("Username"), QString()).toString();
    m_password = settings.value(QLatin1String("Password"), QString()).toString();
    m_db = settings.value(QLatin1String("Database"), QLatin1String("tepsonic")).toString();
    settings.endGroup();
    settings.endGroup();

    connectToDB();
}

void DatabaseManager::connectToDB()
{
    switch (m_driverType) {
    case MySQL: {
        m_sqlDb = QSqlDatabase::addDatabase(QLatin1String("QMYSQL"));
        m_sqlDb.setHostName(m_server);
        m_sqlDb.setUserName(m_username);
        m_sqlDb.setPassword(m_password);
        m_sqlDb.setDatabaseName(m_db);
    }
    break;
    case SQLite: {
        m_sqlDb = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"));
        // If the DB file does not exist, try to create it
        if (!QFile::exists(QString(_CONFIGDIR) + QDir::separator() + QLatin1String("collection.db"))) {
            // First check, if ~/.config/tepsonic exists, eventually create it
            QDir configdir;
            if (!configdir.exists(QString(_CONFIGDIR)))
                configdir.mkdir(QString(_CONFIGDIR));
            // Now check if the DB file exist and try to create it
            QFile file(QString(_CONFIGDIR) + QDir::separator() + QLatin1String("collection.db"));
            if (!file.open(QIODevice::WriteOnly)) {
                qDebug() << "Failed to create new database file!";
                return;
            }
        }
        m_sqlDb.setDatabaseName(QString(_CONFIGDIR) + QDir::separator() + QLatin1String("collection.db"));
    }
    break;
    }

    if (!m_sqlDb.open()) {
        qDebug() << "Failed to establish '" << m_sqlDb.connectionName() << "' connection to database!";
        qDebug() << "Reason: " << m_sqlDb.lastError().text();
        m_connectionAvailable = false;
        return;
    }

    // We want to use UTF8!!!
    if (m_driverType == MySQL) {
        QSqlQuery query(QLatin1String("SET CHARACTER SET utf8;"), m_sqlDb);
    }

    QStringList tables = m_sqlDb.tables(QSql::AllTables);
    if (!(tables.contains(QLatin1String("albums")) &&
            tables.contains(QLatin1String("genres")) &&
            tables.contains(QLatin1String("interprets")) &&
            tables.contains(QLatin1String("tracks")) &&
            tables.contains(QLatin1String("years")) &&
            tables.contains(QLatin1String("db_rev")) &&
            tables.contains(QLatin1String("view_tracks")))) {
        initDb();
    }

    // Now check if DB revision match
    QSqlQuery query(QLatin1String("SELECT `revision` FROM `db_rev` LIMIT 1;"), m_sqlDb);
    query.next();
    if (query.value(0).toString() != QLatin1String(_DBREVISION)) {
        qDebug() << "Database revisions don't match: Found revision" << query.value(0).toString() << ", expected revision " << _DBREVISION;
        qDebug() << "Collections will be rebuilt";
        initDb();
    }

    m_connectionAvailable = true;
    return;
}

void DatabaseManager::initDb()
{
    qDebug() << "Initializing database structure";
    switch (m_driverType) {
    case MySQL: {
        QSqlQuery query(m_sqlDb);

        query.exec(QLatin1String("DROP TABLE IF EXISTS `albums`,`genres`,`interprets`,`tracks`,`years`,`db_rev`;"));
        query.exec(QLatin1String("DROP VIEW IF EXISTS `view_tracks`,`view_various_artists`;"));

        query.exec(QLatin1String("CREATE TABLE `albums` ("
                   "   `id` int(11) NOT NULL AUTO_INCREMENT,"
                   "   `album` varchar(250) NOT NULL,"
                   "   `tracksCnt` int(11) NOT NULL DEFAULT '0',"
                   "   `totalLength` int(11) NOT NULL DEFAULT '0',"
                   "   `showInVA int(1) NOT NULL DEFAULT '0',"
                   "   PRIMARY KEY (`id`)"
                   ") ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;"));

        query.exec(QLatin1String("CREATE TABLE `genres` ("
                   "   `id` int(11) NOT NULL AUTO_INCREMENT,"
                   "   `genre` varchar(80) NOT NULL,"
                   "   PRIMARY KEY (`id`)"
                   ") ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;"));

        query.exec(QLatin1String("CREATE TABLE `interprets` ("
                   "   `id` int(11) NOT NULL AUTO_INCREMENT,"
                   "   `interpret` varchar(300) NOT NULL,"
                   "   `albumsCnt` int(11) NOT NULL DEFAULT '0',"
                   "   `totalLength` int(11) NOT NULL DEFAULT '0',"
                   "   PRIMARY KEY (`id`)"
                   ") ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;"));

        query.exec(QLatin1String("CREATE TABLE `tracks` ("
                   "   `id` int(11) NOT NULL AUTO_INCREMENT,"
                   "   `filename` text NOT NULL,"
                   "   `trackname` varchar(300) NOT NULL,"
                   "   `track` int(11) NOT NULL,"
                   "   `length` int(11) unsigned NOT NULL,"
                   "   `interpret` int(11) NOT NULL,"
                   "   `album` int(11) NOT NULL,"
                   "   `genre` int(11) NOT NULL,"
                   "   `year` int(11) NOT NULL,"
                   "   `bitrate` int(11)  NOT NULL,"
                   "   `mtime` int(11) unsigned NOT NULL,"
                   "   PRIMARY KEY (`id`),"
                   "   KEY `interpret` (`interpret`,`album`,`year`,`genre`)"
                   ") ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;"));

        query.exec(QLatin1String("CREATE TABLE `years` ("
                   "   `id` int(11) NOT NULL AUTO_INCREMENT,"
                   "   `year` int(11) NOT NULL,"
                   "   PRIMARY KEY (`id`)"
                   ") ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;"));

        QString sql = QLatin1String("CREATE TABLE `db_rev` ("
                      "   `revision` int(11) NOT NULL DEFAULT " _DBREVISION " "
                      ") ENGINE=MyISAM DEFAULT CHARSET=utf8;");
        query.exec(sql);
        query.exec(QLatin1String("INSERT INTO `db_rev` VALUES('" _DBREVISION "');"));

        query.exec(QLatin1String("CREATE VIEW `view_tracks` AS"
                   "   SELECT `tracks`.`id` AS `id`,"
                   "          `tracks`.`filename` AS `filename`,"
                   "          `tracks`.`trackname` AS `trackname`,"
                   "          `tracks`.`track` AS `track`,"
                   "          `tracks`.`length` AS `length`,"
                   "          `tracks`.`album` AS `albumID`,"
                   "          `tracks`.`interpret` AS `interpretID`,"
                   "          `interprets`.`interpret` AS `interpret`,"
                   "          `tracks`.`bitrate` AS `bitrate`,"
                   "          `genres`.`genre` AS `genre`,"
                   "          `albums`.`album` AS `album`,"
                   "          `years`.`year` AS `year`"
                   "   FROM `tracks`"
                   "   LEFT JOIN `interprets` ON `tracks`.`interpret` = `interprets`.`id`"
                   "   LEFT JOIN `genres` ON `tracks`.`genre` = `genres`.`id`"
                   "   LEFT JOIN `albums` ON `tracks`.`album` = `albums`.`id`"
                   "   LEFT JOIN `years` ON `tracks`.`year` = `years`.`id`;"));
    }
    break;
    case SQLite: {
        QSqlQuery query(m_sqlDb);

        query.exec(QLatin1String("DROP TABLE IF EXISTS `albums`;"));
        query.exec(QLatin1String("DROP TABLE IF EXISTS `genres`;"));
        query.exec(QLatin1String("DROP TABLE IF EXISTS `interprets`;"));
        query.exec(QLatin1String("DROP TABLE IF EXISTS `tracks`;"));
        query.exec(QLatin1String("DROP TABLE IF EXISTS `years`;"));
        query.exec(QLatin1String("DROP TABLE IF EXISTS `db_rev`;"));
        query.exec(QLatin1String("DROP VIEW IF EXISTS `view_tracks`;"));
        query.exec(QLatin1String("DROP VIEW IF EXISTS `view_various_artists`;"));

        query.exec(QLatin1String("CREATE TABLE `genres` ("
                   "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                   "    `genre` TEXT NOT NULL);"));

        query.exec(QLatin1String("CREATE TABLE `interprets` ("
                   "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                   "    `interpret` TEXT NOT NULL,"
                   "    `albumsCnt` INTEGER NOT NULL DEFAULT(0),"
                   "    `totalLength` INTEGER NOT NULL DEFAULT(0));"));

        query.exec(QLatin1String("CREATE TABLE `albums` ("
                   "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                   "    `album` TEXT NOT NULL,"
                   "    `tracksCnt` INTEGER NOT NULL DEFAULT(0),"
                   "    `totalLength` INTEGER NOT NULL DEFAULT(0),"
                   "    `showInVA` INTEGER NOT NULL DEFAULT(0));"));

        query.exec(QLatin1String("CREATE TABLE `tracks` ("
                   "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                   "    `filename` TEXT NOT NULL,"
                   "    `trackname` TEXT NOT NULL,"
                   "    `track` INTEGER,"
                   "    `length` INTEGER NULL,"
                   "    `interpret` INTEGER,"
                   "    `album` INTEGER,"
                   "    `genre` INTEGER,"
                   "    `year` INTEGER,"
                   "    `bitrate` INTEGER, "
                   "    `mtime` INTEGER);"));

        query.exec(QLatin1String("CREATE TABLE `years` ("
                   "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                   "    `year` INTEGER NOT NULL);"));

        QString sql = QLatin1String("CREATE TABLE `db_rev` ("
                      "    `revision` INTEGER NOT NULL DEFAULT(" _DBREVISION "));");
        query.exec(sql);
        query.exec(QLatin1String("INSERT INTO `db_rev` VALUES('" _DBREVISION "');"));

        query.exec(QLatin1String("CREATE VIEW `view_tracks` AS"
                   "    SELECT `tracks`.`id`,"
                   "           `tracks`.`filename`,"
                   "           `tracks`.`trackname`,"
                   "           `tracks`.`track`,"
                   "           `tracks`.`length`,"
                   "           `tracks`.`album` AS `albumID`,"
                   "           `tracks`.`interpret` AS `interpretID`,"
                   "           `interprets`.`interpret`,"
                   "           `tracks`.`bitrate`, "
                   "           `genres`.`genre`,"
                   "           `albums`.`album`,"
                   "           `years`.`year`"
                   "    FROM `tracks`"
                   "    LEFT JOIN `interprets` ON `tracks`.`interpret` = `interprets`.`id`"
                   "    LEFT JOIN `genres` ON `tracks`.`genre` = `genres`.`id`"
                   "    LEFT JOIN `albums` ON `tracks`.`album` = `albums`.`id`"
                   "    LEFT JOIN `years` ON `tracks`.`year` = `years`.`id`;"));
    }
    break;
    }
}
