/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <dan@progdan.cz>
 * Contributors: Petr Vaněk
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


#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "pluginsmanager.h"
#include "mainwindow.h"
#include "constants.h"
#include "player.h"

#include "playerpage.h"
#include "collectionspage.h"
#include "pluginspage.h"
#include "shortcutspage.h"
#include "ui_playerpage.h"
#include "ui_collectionspage.h"
#include "ui_pluginspage.h"
#include "ui_shortcutspage.h"

#include "abstractplugin.h"

#include <QFileDialog>
#include <QStandardItemModel>
#include <QDebug>

enum {
    PLAYER_PAGE,
    COLLECTIONS_PAGE,
    PLUGINS_PAGE,
    SHORTCUTS_PAGE
};

SettingsDialog::SettingsDialog(MainWindow *parent):
    m_ui(new Ui::SettingsDialog),
    m_parent(parent)
{
    m_ui->setupUi(this);
    m_ui->pagesButtons->item(0)->setSelected(true);

    connect(m_ui->pagesButtons, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(changePage(QListWidgetItem *, QListWidgetItem *)));

    m_pages.insert(PLAYER_PAGE, new SettingsPages::PlayerPage);
    m_pages.insert(COLLECTIONS_PAGE, new SettingsPages::CollectionsPage);
    m_pages.insert(PLUGINS_PAGE, new SettingsPages::PluginsPage);
    m_pages.insert(SHORTCUTS_PAGE, new SettingsPages::ShortcutsPage);

    QSettings settings(QString(_CONFIGDIR) + QLatin1String("/main.conf"), QSettings::IniFormat, this);
    foreach (SettingsPage * page, m_pages) {
        m_ui->pages->addWidget(page);
        page->loadSettings(&settings);
    }

    connect(m_pages[COLLECTIONS_PAGE], SIGNAL(rebuildCollections()),
            this, SLOT(emitRebuildCollections()));

    connect(m_ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(dialogAccepted()));
}

SettingsDialog::~SettingsDialog()
{
    qDeleteAll(m_pages);
    delete m_ui;
}

void SettingsDialog::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void SettingsDialog::dialogAccepted()
{
    QSettings settings(QString(_CONFIGDIR) + QLatin1String("/main.conf"), QSettings::IniFormat);
    Q_FOREACH (SettingsPage * page, m_pages) {
        page->saveSettings(&settings);
    }

    if (qobject_cast<SettingsPages::CollectionsPage *>(m_pages[COLLECTIONS_PAGE])->collectionsSourceChanged()) {
        Q_EMIT rebuildCollections();
    }

    if (qobject_cast<SettingsPages::PlayerPage *>(m_pages[PLAYER_PAGE])->outputDeviceChanged()) {
        Q_EMIT outputDeviceChanged();
    }

    accept();

    this->close();
}

void SettingsDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    m_ui->pages->setCurrentIndex(m_ui->pagesButtons->row(current));
}

void SettingsDialog::emitRebuildCollections()
{
    // Save current state of collections configurations and emit rebuilding

    QSettings settings(QString(_CONFIGDIR) + QLatin1String("/main.conf"), QSettings::IniFormat);
    m_pages[COLLECTIONS_PAGE]->saveSettings(&settings);

    Q_EMIT rebuildCollections();
}

#include "moc_settingsdialog.cpp"
