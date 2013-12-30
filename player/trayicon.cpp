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

#include "trayicon.h"
#include "player.h"
#include "mainwindow.h"

#include <QEvent>
#include <QFileInfo>
#include <QWheelEvent>


TrayIcon::TrayIcon(MainWindow *parent):
    QSystemTrayIcon(parent)
{
    connect(this, &TrayIcon::activated,
            [=](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::Trigger) {
                    parent->toggleWindowVisible();
                }
            });

    setIcon(QIcon(QStringLiteral(":/icons/mainIcon")));
    setVisible(true);
    setContextMenu(ActionManager::instance()->menu(QStringLiteral("TrayIconMenu")));
    setToolTip(tr("Player is stopped"));
}

void TrayIcon::playerStatusChanged(Phonon::State newState, Phonon::State oldState)
{
    Q_UNUSED(oldState);

    const MetaData metadata = Player::instance()->currentMetaData();

    QString playing;
    if (metadata.title().isEmpty()) {
        playing = QFileInfo(metadata.fileName()).fileName();
    } else {
        playing = metadata.title();
    }
    if (!metadata.artist().isEmpty()) {
        playing = metadata.artist() + QLatin1String(" - ") + playing;
    }

    switch (newState) {
        case Phonon::PlayingState:
            setToolTip(tr("Playing: ") + playing);
            break;
        case Phonon::PausedState:
            setToolTip(tr("%1 [paused]").arg(toolTip()));
            break;
        case Phonon::StoppedState:
            setToolTip(tr("Player is stopped"));
            break;
        default:
            //...
            break;
    }
}


bool TrayIcon::event(QEvent *event)
{
    if (event->type() == QEvent::Wheel) {
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
        Player *player = Player::instance();
        if (wheelEvent->delta() > 0) {
            player->audioOutput()->setMuted(false);
            if (player->audioOutput()->volume() < 1) {
                player->audioOutput()->setVolume(player->audioOutput()->volume() + 0.1);
            } else {
                player->audioOutput()->setVolume(1);
            }
        } else {
            // not a typo
            if (player->audioOutput()->volume() > 0.01) {
                player->audioOutput()->setVolume(player->audioOutput()->volume() - 0.1);
            } else {
                player->audioOutput()->setMuted(true);
            }
        }
        event->accept();
        return true;
    } else if (event->type() == QEvent::ToolTip) {
        event->accept();
        return true;
    }

    return false;
}
