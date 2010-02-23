#include "collectionsupdater.h"
#include "collectionmodel.h"
#include "databasemanager.h"

#include <QDebug>

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMap>
#include <QModelIndex>
#include <QRegExp>
#include <QSettings>
#include <QStringList>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>


CollectionsUpdater::CollectionsUpdater(CollectionModel *model)
{
    _model = model;
}

void CollectionsUpdater::run()
{
    // This does not take any effect on Linux but on Windows it will not freeze the whole system :-)
    setPriority(QThread::LowPriority);

    DatabaseManager dbManager("collectionsUpdateConnection");
    if (!dbManager.connectToDB()) {
        return;
    }
    QSqlDatabase sqlConn = QSqlDatabase::database("collectionsUpdateConnection");

    qDebug() << "Starting collections update...";
    QSettings settings(QDir::homePath().append("/.tepsonic/main.conf"),QSettings::IniFormat);

    // Store all paths that will be searched
    QStringList paths = settings.value("Collections/SourcePaths",QStringList()).toStringList();
    QFileInfo fileInfo;
    // List of files in database
    QMap<QString,uint> dbFiles;
    QStringList toBeUpdated;
    QStringList toBeRemoved;
    QStringList filters;

    bool anythingUpdated = false;

    {
        QSqlQuery query("SELECT filename,mtime FROM Tracks;",sqlConn);
        qDebug() << sqlConn.lastError().text();
        while (query.next()) {
            dbFiles[query.value(0).toString()] = query.value(1).toUInt();
        }
    }

    filters << "*.mp3" << "*.mp4" << "*.wav" << "*.flac";
    int filesProcessed = 0;
    for (int i = 0; i < paths.count(); i++) {
        QDir dirlist(paths.at(i));
        dirlist.setNameFilters(filters);

        QDirIterator dirIterator(dirlist,QDirIterator::Subdirectories);
        while (dirIterator.hasNext()) {
            fileInfo = dirIterator.fileInfo();
            if (fileInfo.isFile()) {
                // If the file is not in database OR mtime are different then the file will be updated
                if ((!dbFiles.contains(fileInfo.filePath())) ||
                    (dbFiles[fileInfo.filePath()]!=fileInfo.lastModified().toTime_t())) {
                    toBeUpdated << fileInfo.filePath();
                }
                /* Now set the mtime to 0 which indicates that we have already checked this file. When checking
                   is done, we can assume that all pairs with value > 0 are files that are stored in DB but are not
                   anymore in the list of files on harddrive and therefor should be removed from DB */
                dbFiles[fileInfo.filePath()]=0;
            }
            dirIterator.next();
            filesProcessed++;
        }
    }
    // Find all pairs with value > 0 and put the list of filenames to toBeRemoved stringlist
    QMap<QString,uint>::iterator dbFilesIterator = dbFiles.begin();
    while (dbFilesIterator != dbFiles.end()) {
        if (dbFilesIterator.value()>0) {
            toBeRemoved << dbFilesIterator.key();
        }
        ++dbFilesIterator;
    }
    dbFiles.clear(); // Memory saving...

    // First remove files that are to be removed
    if (toBeRemoved.count() > 0) {
        QSqlQuery query("DELETE FROM Tracks WHERE filename IN ('"+toBeRemoved.join("','")+"');",sqlConn);
        qDebug() << sqlConn.lastError().text();
        if (query.numRowsAffected() > 0) {
            anythingUpdated = true;
        }
    }
    toBeRemoved.clear(); // Memory saving...

    QStringList values;
    // Next we will prepare set of input data for each file that should be updated
    QRegExp rx("\'");
    if (toBeUpdated.count() > 0) {
        QVariantList filenames, trackNos, artists, albums, titles, mtimes;
        for (int i = 0; i < toBeUpdated.count(); i++) {
            TagLib::FileRef f(toBeUpdated.at(i).toUtf8());
            int trackNo = f.tag()->track();
            QString filename = toBeUpdated.at(i);
            QString artist = f.tag()->artist().toCString(true);
            QString album = f.tag()->album().toCString(true);
            QString title = f.tag()->title().toCString(true);
            filename.replace(rx,"\\'");
            artist.replace(rx,"\\'");
            album.replace(rx,"\\'");
            title.replace(rx,"\\'");
            if (artist.isEmpty()) artist = tr("Unkown artist");
            if (album.isEmpty()) album = tr("Unknown album");
            if (title.isEmpty()) title = QFileInfo(toBeUpdated.at(i).toUtf8()).baseName();
            uint mtime = QFileInfo(toBeUpdated.at(i)).lastModified().toTime_t();
            filenames << filename;
            trackNos << trackNo;
            artists << artist;
            albums << album;
            titles << title;
            mtimes << mtime;
        }
        QSqlQuery query(sqlConn);
        query.prepare("REPLACE INTO Tracks (filename,trackNo,artist,album,title,mtime)"\
                      "VALUES (?, ?, ?, ?, ?, ?)");
        query.addBindValue(filenames);
        query.addBindValue(trackNos);
        query.addBindValue(artists);
        query.addBindValue(albums);
        query.addBindValue(titles);
        query.addBindValue(mtimes);
        query.execBatch();
        if (query.numRowsAffected() > 0) {
            anythingUpdated = true;
        }
        qDebug() << query.lastError().text();

    }
    values.clear();

    qDebug() << "Collections were updated";

    if (anythingUpdated) {
        emit(collectionsChanged());
    }
}
