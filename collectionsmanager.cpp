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

#include "collectionsmanager.h"
#include "collectionbrowser.h"
#include "collectionsupdater.h"

#include <QDebug>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMap>

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>

CollectionsManager::CollectionsManager(QSettings *settings, QObject *parent):
        QObject(parent)
{
    _settings = settings;

    QString driver;
    switch (_settings->value("Collections/StorageEngine",0).toInt()) {
        case 1:
           driver = "QMYSQL";
           break;
        default:
        case 0:
           driver = "QSQLITE";
           break;
    }

    sqlDb = QSqlDatabase::addDatabase(driver);
    if (driver == "QMYSQL") {
        _settings->beginGroup("Collections");
        _settings->beginGroup("MySQL");
        sqlDb.setHostName(_settings->value("Server","localhost").toString());
        sqlDb.setUserName(_settings->value("Username",QString()).toString());
        sqlDb.setPassword(_settings->value("Password",QString()).toString());
        sqlDb.setDatabaseName(_settings->value("Database","tepsonic").toString());
        _settings->endGroup();
        _settings->endGroup();
    } else {
        sqlDb.setDatabaseName(QString(QDir::homePath()).append("/.tepsonic/collection.db"));
    }

    if (!sqlDb.open()) {
        qDebug() << "Failed to connect to database!" << sqlDb.lastError().text();
    }

    if (driver=="QMYSQL") {
        QSqlQuery query("SHOW TABLES;");
        if (query.value(0).toString()!="Tracks") {
            initDb(CollectionsManager::MySQL);
        }
    }
}

CollectionsManager::~CollectionsManager()
{
    sqlDb.close();
    sqlDb.removeDatabase(sqlDb.database(sqlDb.connectionName()).databaseName());
}

void CollectionsManager::updateCollections()
{
    _updater = new CollectionsUpdater(_settings);
    connect(_updater,SIGNAL(collectionsUpdated(bool)),this,SLOT(collectionsUpdated(bool)));
    _updater->run();
}

void CollectionsManager::collectionsUpdated(bool updated)
{
    if (updated) {
        emit collectionChanged();
    }
}

void CollectionsManager::initDb(CollectionsManager::DBType dbType)
{
    switch (dbType) {
        case CollectionsManager::MySQL: {
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
                            ") ENGINE=MyISAM DEFAULT CHARSET=utf8;");
        } break;
        case CollectionsManager::SQLite:
            break;
    }
}
