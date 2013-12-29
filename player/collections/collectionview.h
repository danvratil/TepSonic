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

#ifndef COLLECTIONVIEW_H
#define COLLECTIONVIEW_H

#include <QTreeView>

class CollectionView : public QTreeView
{
    Q_OBJECT

  public:
    CollectionView(QWidget *parent = 0);
    ~CollectionView();

    void disableCollections();
    void enableCollections();

  protected:
    void startDrag(Qt::DropActions dropActions);
    void keyPressEvent(QKeyEvent *event);

  private:
    void loadAlbum(const QModelIndex &album, QDataStream &stream) const;

};

#endif // COLLECTIONVIEW_H
