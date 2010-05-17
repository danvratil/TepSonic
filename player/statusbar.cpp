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

#include "statusbar.h"

StatusBar::StatusBar(QWidget *parent) :
    QStatusBar(parent)
{
    _actionLabel = 0;
    _progressBar = 0;
}


void StatusBar::setProgressBar(QString action, int position, int maxPosition)
{
    if (_actionLabel==0) {
        _actionLabel = new QLabel(action,this);
        _actionLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);
        insertWidget(0,_actionLabel,1);
    }
    _actionLabel->setText(action);

    if (_progressBar==0) {
        _progressBar = new QProgressBar(this);
        _progressBar->setMinimumWidth(100);
        _progressBar->setMaximumWidth(100);
        _progressBar->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);
        insertWidget(0,_progressBar,1);
        _progressBar->setMinimum(0);
        _progressBar->setMaximum(maxPosition);
    }
    _progressBar->setMaximum(maxPosition);
    _progressBar->setValue(position);

    if (position == maxPosition) {
        removeWidget(_actionLabel);
        removeWidget(_progressBar);
        delete _actionLabel;
        delete _progressBar;
        _actionLabel = 0;
        _progressBar = 0;
    }
}

void StatusBar::cancelAction()
{
    removeWidget(_actionLabel);
    removeWidget(_progressBar);
    delete _actionLabel;
    delete _progressBar;
    _actionLabel = 0;
    _progressBar = 0;
}

void StatusBar::showWorkingBar(QString action)
{
    setProgressBar(action,1,0);
}
