/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <me@dvratil.cz>
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

class ActionManager::Private
{
  public:
    Private(ActionManager *parent);

    void initDefaultActions();
    void initDefaultMenus();

    QMap<QString, QAction*> actions;
    QMap<QString, QMenu*> menus;

    static ActionManager *sInstance;

  private:
    ActionManager * const q;
};

ActionManager* ActionManager::Private::sInstance = 0;

ActionManager::Private::Private(ActionManager* parent):
    q(parent)
{
}

void ActionManager::Private::initDefaultActions()
{
    QAction *action;
    QActionGroup *group;

    q->addAction(QStringLiteral("Settings"), QObject::tr("Se&ttings"));
    q->addAction(QStringLiteral("ToggleWindow"), QObject::tr("&Show/Hide"), QIcon(),
                 QKeySequence(Qt::CTRL + Qt::Key_H));
    q->addAction(QStringLiteral("Quit"), QObject::tr("&Quit"), QIcon(QStringLiteral(":/icons/quit")),
                 QKeySequence(Qt::CTRL + Qt::Key_Q));

    q->addAction(QStringLiteral("PlaylistClear"), QObject::tr("Clear &Playlist"),
                 QIcon(QStringLiteral(":/icons/clearList")), QKeySequence(Qt::CTRL + Qt::Key_C));
    q->addAction(QStringLiteral("PlaylistShuffle"), QObject::tr("Shu&ffle"),
                 QIcon(QStringLiteral(":/icons/shuffle")), QKeySequence(Qt::CTRL + Qt::Key_F));
    q->addAction(QStringLiteral("PlaylistSave"), QObject::tr("&Save"),
                 QIcon(QStringLiteral(":/icons/save")), QKeySequence(Qt::CTRL + Qt::Key_S));

    q->addAction(QStringLiteral("PlayerPreviousTrack"), QObject::tr("P&revious track"),
                 QIcon(QStringLiteral(":icons/backward")), QKeySequence(Qt::CTRL + Qt::Key_P));
    q->addAction(QStringLiteral("PlayerNextTrack"), QObject::tr("&Next track"),
                 QIcon(QStringLiteral(":icons/forward")), QKeySequence(Qt::CTRL + Qt::Key_N));
    q->addAction(QStringLiteral("PlayerStop"), QObject::tr("&Stop"),
                 QIcon(QStringLiteral(":icons/stop")), QKeySequence(Qt::CTRL + Qt::Key_B));
    q->addAction(QStringLiteral("PlayerPlayPause"), QObject::tr("&Play/Pause"),
                 QIcon(QStringLiteral(":icons/start")), QKeySequence(Qt::Key_Space));

    group = new QActionGroup(0);
    action = q->addAction(QStringLiteral("PlayerRandomOn"), QObject::tr("Random &ON"));
    action->setCheckable(true);
    group->addAction(action);
    action = q->addAction(QStringLiteral("PlayerRandomOff"), QObject::tr("Random &OFF"));
    action->setCheckable(true);
    group->addAction(action);

    group = new QActionGroup(0);
    action = q->addAction(QStringLiteral("PlayerRepeatTrack"), QObject::tr("Repeat &track"));
    action->setCheckable(true);
    group->addAction(action);
    action = q->addAction(QStringLiteral("PlayerRepeatAll"), QObject::tr("Repeat &all"));
    action->setCheckable(true);
    group->addAction(action);
    action = q->addAction(QStringLiteral("PlayerRepeatOff"), QObject::tr("Repeat &off"));
    action->setCheckable(true);
    group->addAction(action);

    q->addAction(QStringLiteral("AboutTepSonic"), QObject::tr("About &TepSonic"),
                 QIcon(QStringLiteral(":/icons/about")));
    q->addAction(QStringLiteral("AboutQt"), QObject::tr("About &Qt"),
                 QIcon(QStringLiteral(":/icons/about")));
    q->addAction(QStringLiteral("SupportedFormats"), QObject::tr("Supported &Formats"));
    q->addAction(QStringLiteral("ReportBug"), QObject::tr("Report a &bug"));
}

void ActionManager::Private::initDefaultMenus()
{
    QMenu *menu;

    menu = new QMenu(QObject::tr("&Random"));
    menu->addAction(actions[QStringLiteral("PlayerRandomOn")]);
    menu->addAction(actions[QStringLiteral("PlayerRandomOff")]);
    q->addMenu(QStringLiteral("PlayerRandomSubmenu"), menu);

    menu = new QMenu(QObject::tr("&Repeat"));
    menu->addAction(actions[QStringLiteral("PlayerRepeatOff")]);
    menu->addAction(actions[QStringLiteral("PlayerRepeatTrack")]);
    menu->addAction(actions[QStringLiteral("PlayerRepeatAll")]);
    q->addMenu(QStringLiteral("PlayerRepeatSubmenu"), menu);

    menu = new QMenu;
    menu->addAction(actions[QStringLiteral("ToggleWindow")]);
    menu->addSeparator();
    menu->addAction(actions[QStringLiteral("PlayerPreviousTrack")]);
    menu->addAction(actions[QStringLiteral("PlayerPlayPause")]);
    menu->addAction(actions[QStringLiteral("PlayerStop")]);
    menu->addAction(actions[QStringLiteral("PlayerNextTrack")]);
    menu->addSeparator();
    menu->addMenu(menus[QStringLiteral("PlayerRandomSubmenu")]);
    menu->addMenu(menus[QStringLiteral("PlayerRepeatSubmenu")]);
    menu->addSeparator();
    menu->addAction(actions[QStringLiteral("Quit")]);
    q->addMenu(QStringLiteral("TrayIconMenu"), menu);
}

ActionManager* ActionManager::instance()
{
    if (Private::sInstance == 0) {
        Private::sInstance = new ActionManager;
    }

    return Private::sInstance;
}

ActionManager::ActionManager():
    d(new Private(this))
{
    d->initDefaultActions();
    d->initDefaultMenus();
}

ActionManager::~ActionManager()
{
    qDeleteAll(d->actions);
    qDeleteAll(d->menus);
    delete d;
}

QAction* ActionManager::action(const QString &name) const
{
    if (!d->actions.contains(name)) {
        qFatal("Fatal error in %s - Unknown action '%s'",
               Q_FUNC_INFO, name.toLocal8Bit().constData());
    }

    return d->actions[name];
}

QAction* ActionManager::addAction(const QString &name, QAction *action)
{
    if (d->actions.contains(name)) {
        qFatal("Fatal error in %s - Action '%s' already registered",
               Q_FUNC_INFO, name.toLocal8Bit().constData());
    }
    d->actions.insert(name, action);
    return action;
}

QAction* ActionManager::addAction(const QString &name, const QString &title,
                                  const QIcon &icon, const QKeySequence &shortcut)
{
    QAction *action = new QAction(icon, title, 0);
    action->setShortcut(shortcut);
    d->actions.insert(name, action);
    return action;
}

QMenu* ActionManager::menu(const QString &name) const
{
    if (!d->menus.contains(name)) {
        qFatal("Fatal error in %s - Unknown menu '%s'",
               Q_FUNC_INFO, name.toLocal8Bit().constData());
    }

    return d->menus[name];
}

void ActionManager::addMenu(const QString &name, QMenu *menu)
{
    if (d->menus.contains(name)) {
        qFatal("Fatal error in %s - Menu '%s' already registered",
               Q_FUNC_INFO, name.toLocal8Bit().constData());
    }

    d->menus.insert(name, menu);
}

