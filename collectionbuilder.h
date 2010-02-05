#ifndef COLLECTIONBUILDER_H
#define COLLECTIONBUILDER_H

#include <QThread>

class CollectionModel;

class CollectionBuilder : public QThread
{
    Q_OBJECT
    public:
        CollectionBuilder(CollectionModel *model);
        void run();

    private:
        CollectionModel *_model;
};

#endif // COLLECTIONBUILDER_H
