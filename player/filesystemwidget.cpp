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

#include "filesystemwidget.h"
#include "filesystemview.h"

#include <core/settings.h>

#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QDir>

using namespace TepSonic;

FileSystemWidget::FileSystemWidget(QWidget* parent):
    QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    mainLayout->addLayout(buttonLayout);

    mFSView = new FileSystemView(this);
    connect(mFSView, &FileSystemView::pathChanged,
            this, &FileSystemWidget::setPath);

    mBackButton = createButton(QStringLiteral(":/icons/goPrev"), tr("Back"),
                               &FileSystemView::goBack);
    mBackButton->setDisabled(true);
    buttonLayout->addWidget(mBackButton);

    mUpButton = createButton(QStringLiteral(":/icons/goUp"), tr("Up"),
                             &FileSystemView::cdUp);
    mUpButton->setDisabled(true);
    buttonLayout->addWidget(mUpButton);

    mForwardButton = createButton(QStringLiteral(":/icons/goNext"), tr("Forward"),
                                  &FileSystemView::goForward);
    mForwardButton->setDisabled(true);
    buttonLayout->addWidget(mForwardButton);

    mHomeButton = createButton(QStringLiteral(":/icons/goHome"), tr("Home"),
                               &FileSystemView::goHome);
    buttonLayout->addWidget(mHomeButton);

    buttonLayout->addStretch(2);

    mFSPathEditor = new QLineEdit(this);
    connect(mFSPathEditor, &QLineEdit::textChanged,
            this, &FileSystemWidget::setPath);
    mainLayout->addWidget(mFSPathEditor);

    mainLayout->addWidget(mFSView);

    mFSView->setPath(Settings::instance()->sessionFSBrowserPath());
}

FileSystemWidget::~FileSystemWidget()
{
}

QPushButton* FileSystemWidget::createButton(const QString &icon, const QString &title,
                                            void (FileSystemView::*slot)(void))
{
    QPushButton *button = new QPushButton(QIcon(icon), QString(), this);
    button->setToolTip(title);
    connect(button, &QPushButton::clicked, mFSView, slot);

    return button;
}

void FileSystemWidget::setPath(const QString &path)
{
    mFSView->blockSignals(true);
    mFSView->setPath(path);
    mFSView->blockSignals(false);

    mFSPathEditor->blockSignals(true);
    mFSPathEditor->setText(path);
    mFSPathEditor->blockSignals(false);

    mBackButton->setEnabled(mFSView->canGoBack());
    mForwardButton->setEnabled(mFSView->canGoForward());
    mUpButton->setEnabled(mFSView->canGoUp());
}
