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

#ifndef PLUGINSPAGE_H
#define PLUGINSPAGE_H

#include <QtGui/QListWidgetItem>

#include "settingspage.h"

namespace Ui
{
class PluginsPage;
}

namespace SettingsPages
{

class PluginsPage: public SettingsPage
{
    Q_OBJECT

  public:
    PluginsPage(QWidget *parent = 0);
    ~PluginsPage();

  public Q_SLOTS:
    void loadSettings(QSettings *settings);
    void saveSettings(QSettings *settings);

  private Q_SLOTS:
    void pluginsListItemChanged(QListWidgetItem *item);

  private:
    ::Ui::PluginsPage *m_ui;
};

}

#endif // PLUGINSPAGE_H
