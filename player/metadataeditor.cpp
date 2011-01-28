/*
 * TEPSONIC
 * Copyright 2011 Dan Vratil <vratil@progdansoft.com>
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

#include "metadataeditor.h"
#include "ui_metadataeditor.h"

#include <QFontMetrics>
#include <QFileInfo>

MetadataEditor::MetadataEditor(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::MetadataEditor)
{
    m_ui->setupUi(this);
}

MetadataEditor::~MetadataEditor()
{
    delete m_ui;
}


void MetadataEditor::setFilename(QString filename)
{
    QFileInfo fi(filename);
    m_filename = fi.absoluteFilePath();

    resizeEvent(0);
}


void MetadataEditor::resizeEvent(QResizeEvent *e)
{
    QFontMetrics fm(font());
    m_ui->box->setTitle(fm.elidedText(m_filename, Qt::ElideMiddle, m_ui->box->width()));
}

