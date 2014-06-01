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
#include "settings.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QMutex>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

using namespace TepSonic;

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
    connectToDB();
}

void DatabaseManager::connectToDB()
{
    m_sqlDb = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"));
    // If the DB file does not exist, try to create it
    QFile dbFile(Settings::dataDir() + QLatin1String("/collection.db"));
    if (!dbFile.exists()) {
        // Now check if the DB file exist and try to create it
        if (!dbFile.open(QIODevice::WriteOnly)) {
            qDebug() << "Failed to create new database file!";
            return;
        }
    }
    m_sqlDb.setDatabaseName(dbFile.fileName());

    if (!m_sqlDb.open()) {
        qDebug() << "Failed to establish '" << m_sqlDb.connectionName() << "' connection to database!";
        qDebug() << "Reason: " << m_sqlDb.lastError().text();
        m_connectionAvailable = false;
        return;
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
