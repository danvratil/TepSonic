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


bool DatabaseManager::m_static_connectionAvailable;

DatabaseManager::DatabaseManager(QString connectionName)
{
    m_connectionName = connectionName;
    m_sqlDb = NULL;
    m_static_connectionAvailable = false;

    QSettings settings(QString(_CONFIGDIR).append("/main.conf"),
                       QSettings::IniFormat);
    m_driverType = (DriverTypes)settings.value("Collections/StorageEngine",0).toInt();
    settings.beginGroup("Collections");
    settings.beginGroup("MySQL");
    m_server = settings.value("Server","127.0.0.1").toString();
    m_username = settings.value("Username",QString()).toString();
    m_password = settings.value("Password",QString()).toString();
    m_db = settings.value("Database","tepsonic").toString();
    settings.endGroup();
    settings.endGroup();;

}

DatabaseManager::~DatabaseManager()
{
    delete m_sqlDb;
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool DatabaseManager::connectToDB()
{
    if (QSqlDatabase::contains(m_connectionName)) {
        qDebug() << "Connection with name " << m_connectionName << " exists.";
        m_static_connectionAvailable = true;
        return true;
    }

    switch (m_driverType) {
    case MySQL: {
        m_sqlDb = new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL", m_connectionName));
        m_sqlDb->setHostName(m_server);
        m_sqlDb->setUserName(m_username);
        m_sqlDb->setPassword(m_password);
        m_sqlDb->setDatabaseName(m_db);
    } break;
    case SQLite: {
        m_sqlDb = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", m_connectionName));
        m_sqlDb->setDatabaseName(QString(_CONFIGDIR).append("/collection.db"));
    } break;
    }

    if (!m_sqlDb->open()) {
        qDebug() << "Failed to establish '" << m_sqlDb->connectionName() <<"' connection to database!";
        qDebug() << "Reason: " << m_sqlDb->lastError().text();
        m_sqlDb = NULL;
        m_static_connectionAvailable = false;
        return false;
    }

    // We want to use UTF8!!!
    if (m_driverType == MySQL ) {
        QSqlQuery query("SET CHARACTER SET utf8;", *m_sqlDb);
    }

    QStringList tables = m_sqlDb->tables(QSql::AllTables);
    if (!(tables.contains("albums") &&
          tables.contains("genres") &&
          tables.contains("interprets") &&
          tables.contains("tracks") &&
          tables.contains("years") &&
          tables.contains("db_rev") &&
          tables.contains("view_tracks"))) {
        initDb();
    }

    // Now check if DB revision match
    QSqlQuery query("SELECT `revision` FROM `db_rev` LIMIT 1;", *m_sqlDb);
    query.next();
    QString rev = _DBREVISION;
    if (query.value(0).toString() != rev) {
        qDebug() << "Database revisions don't match: Found revision" << query.value(0).toString() << ", expected revision " << rev;
        qDebug() << "Collections will be rebuilt";
        initDb();
    }

    m_static_connectionAvailable = true;
    return true;
}

void DatabaseManager::initDb()
{
    qDebug() << "Initializing database structure";
    switch (m_driverType) {
    case MySQL: {
        QSqlQuery query(*m_sqlDb);

        query.exec("DROP TABLE IF EXISTS `albums`,`genres`,`interprets`,`tracks`,`years`,`db_rev`;");
        query.exec("DROP VIEW `view_tracks`,`view_various_artists`;");

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
                   "   `bitrate` int(11)  NOT NULL," \
                   "   `mtime` int(11) unsigned NOT NULL," \
                   "   PRIMARY KEY (`id`)," \
                   "   KEY `interpret` (`interpret`,`album`,`year`,`genre`)" \
                   ") ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;");

        query.exec("CREATE TABLE `years` (" \
                   "   `id` int(11) NOT NULL AUTO_INCREMENT," \
                   "   `year` int(11) NOT NULL," \
                   "   PRIMARY KEY (`id`)" \
                   ") ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;");

        QString rev = _DBREVISION;
        QString sql = "CREATE TABLE `db_rev` (" \
                      "   `revision` int(11) NOT NULL DEFAULT "+rev+" " \
                      ") ENGINE=MyISAM DEFAULT CHARSET=utf8;";
        query.exec(sql);
        query.exec("INSERT INTO `db_rev` VALUES('"+rev+"');");

        query.exec("CREATE VIEW `view_tracks` AS" \
                   "   SELECT `tracks`.`id` AS `id`," \
                   "          `tracks`.`filename` AS `filename`," \
                   "          `tracks`.`trackname` AS `trackname`," \
                   "          `tracks`.`track` AS `track`," \
                   "          `tracks`.`length` AS `length`," \
                   "          `tracks`.`album` AS `albumID`," \
                   "          `tracks`.`interpret` AS `interpretID`," \
                   "          `interprets`.`interpret` AS `interpret`," \
                   "          `tracks`.`bitrate` AS `bitrate`," \
                   "          `genres`.`genre` AS `genre`," \
                   "          `albums`.`album` AS `album`," \
                   "          `years`.`year` AS `year`" \
                   "   FROM `tracks`" \
                   "   LEFT JOIN `interprets` ON `tracks`.`interpret` = `interprets`.`id`" \
                   "   LEFT JOIN `genres` ON `tracks`.`genre` = `genres`.`id`" \
                   "   LEFT JOIN `albums` ON `tracks`.`album` = `albums`.`id`" \
                   "   LEFT JOIN `years` ON `tracks`.`year` = `years`.`id`;");

        query.exec("CREATE VIEW `view_various_artists` AS" \
                   "  SELECT `tracks`.`album`," \
                   "         COUNT(DISTINCT `interpret`) AS `interpretsCnt`," \
                   "         SUM(`length`) AS `totalLength`," \
                   "         COUNT(`id`) AS `tracksCnt`" \
                   "  FROM `tracks`" \
                   "  GROUP BY `album`" \
                   "  HAVING `interpretsCnt` > 1;");

    } break;
    case SQLite: {
        QSqlQuery query(*m_sqlDb);

        query.exec("DROP TABLE IF EXISTS `albums`;");
        query.exec("DROP TABLE IF EXISTS `genres`;");
        query.exec("DROP TABLE IF EXISTS ``interprets`;");
        query.exec("DROP TABLE IF EXISTS `tracks`;");
        query.exec("DROP TABLE IF EXISTS `years`;");
        query.exec("DROP TABLE IF EXISTS `db_rev`;");
        query.exec("DROP VIEW IF EXISTS `view_tracks`;");
        query.exec("DROP VIEW IF EXISTS `view_various_artists`;");

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
                   "    `bitrate` INTEGER, " \
                   "    `mtime` INTEGER);");

        query.exec("CREATE TABLE `years` (" \
                   "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                   "    `year` INTEGER NOT NULL);");

        QString rev = _DBREVISION;
        QString sql = "CREATE TABLE `db_rev` (" \
                      "    `revision` INTEGER NOT NULL DEFAULT("+rev+"));";
        query.exec(sql);
        query.exec("INSERT INTO `db_rev` VALUES('"+rev+"');");

        query.exec("CREATE VIEW `view_tracks` AS" \
                   "    SELECT `tracks`.`id`," \
                   "           `tracks`.`filename`," \
                   "           `tracks`.`trackname`," \
                   "           `tracks`.`track`," \
                   "           `tracks`.`length`," \
                   "           `tracks`.`album` AS `albumID`," \
                   "           `tracks`.`interpret` AS `interpretID`," \
                   "           `interprets`.`interpret`," \
                   "           `tracks`.`bitrate`, "\
                   "           `genres`.`genre`," \
                   "           `albums`.`album`," \
                   "           `years`.`year`" \
                   "    FROM `tracks`" \
                   "    LEFT JOIN `interprets` ON `tracks`.`interpret` = `interprets`.`id`" \
                   "    LEFT JOIN `genres` ON `tracks`.`genre` = `genres`.`id`" \
                   "    LEFT JOIN `albums` ON `tracks`.`album` = `albums`.`id`" \
                   "    LEFT JOIN `years` ON `tracks`.`year` = `years`.`id`;");

        query.exec("CREATE VIEW `view_various_artists` AS" \
                   "  SELECT `tracks`.`album`," \
                   "         COUNT(DISTINCT `interpret`) AS `interpretsCnt`," \
                   "         SUM(`length`) AS `totalLength`," \
                   "         COUNT(`id`) AS `tracksCnt`" \
                   "  FROM `tracks`" \
                   "  GROUP BY `album`" \
                   "  HAVING `interpretsCnt` > 1;");

    } break;
    }
}
