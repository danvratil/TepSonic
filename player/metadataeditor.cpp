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
#include "databasemanager.h"

#include <QFontMetrics>
#include <QFileInfo>
#include <QSqlQuery>

#include <QDebug>



MetadataEditor::MetadataEditor(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::MetadataEditor),
    m_artistCompleter(0),
    m_albumCompleter(0),
    m_genreCompleter(0)
{
    m_ui->setupUi(this);

    DatabaseManager dbManager("metadataEditorConnection");
    if (DatabaseManager::connectionAvailable() || dbManager.connectToDB())
    {

        QStringList list;

        {
            QSqlQuery query("SELECT interpret FROM interprets ORDER BY interpret ASC;", *dbManager.sqlDb());
            while (query.next())
                list.append(query.value(0).toString());
        }
        m_artistCompleter = new QCompleter(list, this);
        m_artistCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        m_ui->artistEdit->setCompleter(m_artistCompleter);
        list.clear();

        {
            QSqlQuery query("SELECT album FROM albums ORDER BY album ASC;", *dbManager.sqlDb());
            while (query.next())
                list.append(query.value(0).toString());
        }
        m_albumCompleter = new QCompleter(list, this);
        m_albumCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        m_ui->albumEdit->setCompleter(m_albumCompleter);
        list.clear();

        {
            QSqlQuery query("SELECT genre FROM genres ORDER BY genre ASC;", *dbManager.sqlDb());
            while (query.next())
                list.append(query.value(0).toString());
        }
        m_genreCompleter = new QCompleter(list, this);
        m_genreCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        m_ui->genreEdit->setCompleter(m_genreCompleter);
    }
}

MetadataEditor::~MetadataEditor()
{
    delete m_ui;

    if (m_artistCompleter)
        delete m_artistCompleter;
    if (m_albumCompleter)
        delete m_albumCompleter;
    if  (m_genreCompleter)
        delete m_genreCompleter;
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
