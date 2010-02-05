#ifndef COLLECTIONSUPDATER_H
#define COLLECTIONSUPDATER_H

#include <QThread>

class CollectionModel;

class CollectionsUpdater : public QThread
{
    Q_OBJECT
    public:
        CollectionsUpdater(CollectionModel *model);
        void run();

    signals:
        void collectionsChanged();

    private:
        CollectionModel *_model;
};

#endif // COLLECTIONSUPDATER_H
