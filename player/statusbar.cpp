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

#include "statusbar.h"

StatusBar::StatusBar(QWidget *parent) :
    QStatusBar(parent)
{
    m_actionLabel = 0;
    m_progressBar = 0;
}


void StatusBar::setProgressBar(const QString &action, int position, int maxPosition)
{
    if (m_actionLabel == 0) {
        m_actionLabel = new QLabel(action, this);
        m_actionLabel->setSizePolicy(QSizePolicy::Minimum,
                                     QSizePolicy::Fixed);
        insertWidget(0, m_actionLabel, 1);
    }
    m_actionLabel->setText(action);

    if (m_progressBar == 0) {
        m_progressBar = new QProgressBar(this);
        m_progressBar->setMinimumWidth(100);
        m_progressBar->setMaximumWidth(100);
        m_progressBar->setSizePolicy(QSizePolicy::Minimum,
                                     QSizePolicy::Fixed);
        insertWidget(0, m_progressBar, 1);
        m_progressBar->setMinimum(0);
        m_progressBar->setMaximum(maxPosition);
    }
    m_progressBar->setMaximum(maxPosition);
    m_progressBar->setValue(position);

    if (position == maxPosition) {
        removeWidget(m_actionLabel);
        removeWidget(m_progressBar);
        delete m_actionLabel;
        delete m_progressBar;
        m_actionLabel = 0;
        m_progressBar = 0;
    }
}

void StatusBar::cancelAction()
{
    removeWidget(m_actionLabel);
    removeWidget(m_progressBar);
    delete m_actionLabel;
    delete m_progressBar;
    m_actionLabel = 0;
    m_progressBar = 0;
}

void StatusBar::showWorkingBar(const QString &action)
{
    setProgressBar(action, 1, 0);
}
