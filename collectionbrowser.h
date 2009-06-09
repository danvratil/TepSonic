#ifndef COLLECTIONBROWSER_H
#define COLLECTIONBROWSER_H

#include <QTreeWidget>

class CollectionBrowser : public QTreeWidget
{
    public:
        CollectionBrowser(QWidget* = 0);

    private:
        QPoint dragStartPosition;

    protected:
        void mousePressEvent(QMouseEvent*);
        void mouseMoveEvent(QMouseEvent*);
};

#endif // COLLECTIONBROWSER_H
