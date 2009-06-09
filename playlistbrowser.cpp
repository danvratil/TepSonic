/*
 * Playlist Browser
 *    QTreeWidget with implemented DROP action
 *
 * Thanks to David Watzke
 *
 */

#include "playlistbrowser.h"

#include <QDropEvent>

PlaylistBrowser::PlaylistBrowser(QWidget* parent): QTreeWidget(parent)
{
    setAcceptDrops(true);
}

// Event: item is dragged over the widget
void PlaylistBrowser::dragEnterEvent(QDragEnterEvent *event)
{
         // Just accept the proposed action
        event->acceptProposedAction();
}

// Event: drag has moved above the widget
void PlaylistBrowser::dragMoveEvent(QDragMoveEvent* event)
{
        // Again - accept the proposed action
        event->acceptProposedAction();
}

 // Drop event (item is dropped on the widget)
void PlaylistBrowser::dropEvent(QDropEvent* event)
{

         //TODO: Accept QTreeWidgetItem
         // Get text from informations about the event
        QString text = event->mimeData()->text();


        // TODO: QTreeWidgetItem was accepted - add it into the playlist browser
        // If the text is not emtpy add it to the list
        /*if(!text.isEmpty())
                addItem(event->mimeData()->text());*/
}
