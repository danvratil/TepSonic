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

#ifndef METADATAEDITOR_H
#define METADATAEDITOR_H

#include <QDialog>
#include <QCompleter>

#include "ui_metadataeditor.h"

#include <core/metadata.h>

class MetadataEditor : public QDialog
{
    Q_OBJECT

public:
    explicit MetadataEditor(QWidget *parent = 0);
    ~MetadataEditor();

    TepSonic::MetaData metaData();
    void setMetaData(const TepSonic::MetaData &metaData);

protected:
    void resizeEvent(QResizeEvent *);


private:
    Ui::MetadataEditor *m_ui;

    TepSonic::MetaData m_metaData;

    QCompleter *m_artistCompleter;
    QCompleter *m_albumCompleter;
    QCompleter *m_genreCompleter;

};

#endif // METADATAEDITOR_H
