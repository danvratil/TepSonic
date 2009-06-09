#ifndef PLAYLISTBROWSER_H
#define PLAYLISTBROWSER_H

#include <QTreeWidget>

class PlaylistBrowser : public QTreeWidget
{
    public:
        PlaylistBrowser(QWidget* = 0);

    protected:
        void dropEvent(QDropEvent*);
        void dragEnterEvent(QDragEnterEvent*);
        void dragMoveEvent(QDragMoveEvent*);
};

#endif // PLAYLISTBROWSER_H
