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


#ifndef COLLECTIONSPAGE_H
#define COLLECTIONSPAGE_H

#include <QWidget>
#include "ui_collectionspage.h"

namespace Ui {
    class CollectionsPage;
}

namespace SettingsPages {


    class CollectionsPage: public QWidget
    {
        Q_OBJECT
        public:
            CollectionsPage(QWidget *parent = 0);
            ::Ui::CollectionsPage *ui;
            bool collectionsSourceChanged() {
                return m_collectionsSourceChanged;
            }

        private:
            bool m_collectionsSourceChanged;

        private slots:
            void removeAllPaths();
            void removePath();
            void addPath();
            void changeEngine(QString );

        signals:
            void rebuildCollections();
    };

}

#endif // COLLECTIONSPAGE_H
