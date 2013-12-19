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

#ifndef COLLECTIONSPAGE_H
#define COLLECTIONSPAGE_H

#include "settingspage.h"
#include "ui_collectionspage.h"

namespace Ui
{
class CollectionsPage;
}

namespace SettingsPages
{

class CollectionsPage: public SettingsPage
{
    Q_OBJECT

  public:
    CollectionsPage(QWidget *parent = 0);
    ~CollectionsPage();

    bool collectionsSourceChanged() {
        return m_collectionsSourceChanged;
    }

  public Q_SLOTS:
    void loadSettings();
    void saveSettings();

  private:
    ::Ui::CollectionsPage *m_ui;

    bool m_collectionsSourceChanged;

  private Q_SLOTS:
    void removeAllPaths();
    void removePath();
    void addPath();
    void changeEngine(int index);

    void collectionStateToggled();
};
}

#endif // COLLECTIONSPAGE_H
