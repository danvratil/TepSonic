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

#include "actionmanager.h"

#include <QAction>
#include <QMenu>
#include <QActionGroup>
#include <QDebug>

using namespace TepSonic;

ActionManager* ActionManager::sInstance = 0;

ActionManager* ActionManager::instance()
{
    if (sInstance == 0) {
        sInstance = new ActionManager;
    }

    return sInstance;
}

ActionManager::ActionManager()
{
    initDefaultActions();
    initDefaultMenus();
}

ActionManager::~ActionManager()
{
    qDeleteAll(mActions);
    qDeleteAll(mMenus);
}

QAction* ActionManager::action(const QString &name) const
{
    if (!mActions.contains(name)) {
        qFatal("Fatal error in %s - Unknown action '%s'",
               Q_FUNC_INFO, name.toLocal8Bit().constData());
    }

    return mActions[name];
}

QAction* ActionManager::addAction(const QString &name, QAction *action)
{
    if (mActions.contains(name)) {
        qFatal("Fatal error in %s - Action '%s' already registered",
               Q_FUNC_INFO, name.toLocal8Bit().constData());
    }
    mActions.insert(name, action);
    return action;
}

QAction* ActionManager::addAction(const QString &name, const QString &title,
                                  const QIcon &icon, const QKeySequence &shortcut)
{
    QAction *action = new QAction(icon, title, 0);
    action->setShortcut(shortcut);
    mActions.insert(name, action);
    return action;
}

QMenu* ActionManager::menu(const QString &name) const
{
    if (!mMenus.contains(name)) {
        qFatal("Fatal error in %s - Unknown menu '%s'",
               Q_FUNC_INFO, name.toLocal8Bit().constData());
    }

    return mMenus[name];
}

void ActionManager::addMenu(const QString &name, QMenu *menu)
{
    if (mMenus.contains(name)) {
        qFatal("Fatal error in %s - Menu '%s' already registered",
               Q_FUNC_INFO, name.toLocal8Bit().constData());
    }

    mMenus.insert(name, menu);
}

void ActionManager::initDefaultActions()
{
    QAction *action;
    QActionGroup *group;

    addAction(QStringLiteral("Settings"), QObject::tr("Se&ttings"));
    addAction(QStringLiteral("ToggleWindow"), QObject::tr("&Show/Hide"), QIcon(),
              QKeySequence(Qt::CTRL + Qt::Key_H));
    addAction(QStringLiteral("Quit"), QObject::tr("&Quit"), QIcon(QStringLiteral(":/icons/quit")),
              QKeySequence(Qt::CTRL + Qt::Key_Q));

    addAction(QStringLiteral("PlaylistClear"), QObject::tr("Clear &Playlist"),
              QIcon(QStringLiteral(":/icons/clearList")), QKeySequence(Qt::CTRL + Qt::Key_C));
    addAction(QStringLiteral("PlaylistShuffle"), QObject::tr("Shu&ffle"),
              QIcon(QStringLiteral(":/icons/shuffle")), QKeySequence(Qt::CTRL + Qt::Key_F));
    addAction(QStringLiteral("PlaylistSave"), QObject::tr("&Save"),
              QIcon(QStringLiteral(":/icons/save")), QKeySequence(Qt::CTRL + Qt::Key_S));

    addAction(QStringLiteral("PlayerPreviousTrack"), QObject::tr("P&revious track"),
              QIcon(QStringLiteral(":icons/backward")), QKeySequence(Qt::CTRL + Qt::Key_P));
    addAction(QStringLiteral("PlayerNextTrack"), QObject::tr("&Next track"),
              QIcon(QStringLiteral(":icons/forward")), QKeySequence(Qt::CTRL + Qt::Key_N));
    addAction(QStringLiteral("PlayerStop"), QObject::tr("&Stop"),
              QIcon(QStringLiteral(":icons/stop")), QKeySequence(Qt::CTRL + Qt::Key_B));
    addAction(QStringLiteral("PlayerPlayPause"), QObject::tr("&Play/Pause"),
              QIcon(QStringLiteral(":icons/start")), QKeySequence(Qt::Key_Space));

    group = new QActionGroup(0);
    action = addAction(QStringLiteral("PlayerRandomOn"), QObject::tr("Random &ON"));
    action->setCheckable(true);
    group->addAction(action);
    action = addAction(QStringLiteral("PlayerRandomOff"), QObject::tr("Random &OFF"));
    action->setCheckable(true);
    group->addAction(action);

    group = new QActionGroup(0);
    action = addAction(QStringLiteral("PlayerRepeatTrack"), QObject::tr("Repeat &track"));
    action->setCheckable(true);
    group->addAction(action);
    action = addAction(QStringLiteral("PlayerRepeatAll"), QObject::tr("Repeat &all"));
    action->setCheckable(true);
    group->addAction(action);
    action = addAction(QStringLiteral("PlayerRepeatOff"), QObject::tr("Repeat &off"));
    action->setCheckable(true);
    group->addAction(action);

    addAction(QStringLiteral("AboutTepSonic"), QObject::tr("About &TepSonic"),
              QIcon(QStringLiteral(":/icons/about")));
    addAction(QStringLiteral("AboutQt"), QObject::tr("About &Qt"),
              QIcon(QStringLiteral(":/icons/about")));
    addAction(QStringLiteral("SupportedFormats"), QObject::tr("Supported &Formats"));
    addAction(QStringLiteral("ReportBug"), QObject::tr("Report a &bug"));
}

void ActionManager::initDefaultMenus()
{
    QMenu *menu;

    menu = new QMenu(QObject::tr("&Random"));
    menu->addAction(mActions[QStringLiteral("PlayerRandomOn")]);
    menu->addAction(mActions[QStringLiteral("PlayerRandomOff")]);
    addMenu(QStringLiteral("PlayerRandomSubmenu"), menu);

    menu = new QMenu(QObject::tr("&Repeat"));
    menu->addAction(mActions[QStringLiteral("PlayerRepeatOff")]);
    menu->addAction(mActions[QStringLiteral("PlayerRepeatTrack")]);
    menu->addAction(mActions[QStringLiteral("PlayerRepeatAll")]);
    addMenu(QStringLiteral("PlayerRepeatSubmenu"), menu);

    menu = new QMenu;
    menu->addAction(mActions[QStringLiteral("ToggleWindow")]);
    menu->addSeparator();
    menu->addAction(mActions[QStringLiteral("PlayerPreviousTrack")]);
    menu->addAction(mActions[QStringLiteral("PlayerPlayPause")]);
    menu->addAction(mActions[QStringLiteral("PlayerStop")]);
    menu->addAction(mActions[QStringLiteral("PlayerNextTrack")]);
    menu->addSeparator();
    menu->addMenu(mMenus[QStringLiteral("PlayerRandomSubmenu")]);
    menu->addMenu(mMenus[QStringLiteral("PlayerRepeatSubmenu")]);
    menu->addSeparator();
    menu->addAction(mActions[QStringLiteral("Quit")]);
    addMenu(QStringLiteral("TrayIconMenu"), menu);
}
