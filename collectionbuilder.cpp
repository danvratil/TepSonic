#include "collectionbuilder.h"
#include "collectionmodel.h"
#include "databasemanager.h"

#include <QDebug>

#include <QModelIndex>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

CollectionBuilder::CollectionBuilder(CollectionModel *model)
{
    _model = model;
}

void CollectionBuilder::run()
{
    qDebug() << "Updating CollectionBrowser...";

    DatabaseManager dbManager("updateCollectionBrowserConnection");
    if (!dbManager.connectToDB()) {
        return;
    }
    QSqlDatabase sqlConn = QSqlDatabase::database("updateCollectionBrowserConnection");

    _model->removeRows(0,_model->rowCount(QModelIndex()),QModelIndex());

    QModelIndex albumsParent;
    QModelIndex tracksParent;
    {
        QSqlQuery artistsQuery("SELECT artist FROM Tracks GROUP BY artist ORDER BY artist ASC",sqlConn);
        while (artistsQuery.next()) {
            albumsParent = _model->addChild(QModelIndex(),artistsQuery.value(0).toString(),QString());
            QSqlQuery albumsQuery("SELECT album FROM Tracks WHERE artist='"+artistsQuery.value(0).toString()+"' GROUP BY album ORDER BY album ASC;",sqlConn);
            while (albumsQuery.next()) {
                tracksParent = _model->addChild(albumsParent,albumsQuery.value(0).toString(),QString());
                QSqlQuery tracksQuery("SELECT title,filename FROM Tracks WHERE album='"+albumsQuery.value(0).toString()+"' AND artist='"+artistsQuery.value(0).toString()+"' ORDER BY trackNo ASC;",sqlConn);
                while (tracksQuery.next()) {
                    _model->addChild(tracksParent,tracksQuery.value(0).toString(),tracksQuery.value(1).toString());
                }
            }
        }
    }
    qDebug() << "CollectionBrowser updated";
}
