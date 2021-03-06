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

#include "metadataeditor.h"
#include "ui_metadataeditor.h"

#include <core/databasemanager.h>

#include <QFontMetrics>
#include <QSqlQuery>
#include <QFileInfo>
#include <QDebug>

using namespace TepSonic;

MetadataEditor::MetadataEditor(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::MetadataEditor),
    m_artistCompleter(0),
    m_albumCompleter(0),
    m_genreCompleter(0)
{
    m_ui->setupUi(this);

    DatabaseManager *dbManager = DatabaseManager::instance();
    if (dbManager->connectionAvailable()) {

        QStringList list;

        {
            QSqlQuery query(QLatin1String("SELECT interpret FROM interprets ORDER BY interpret ASC;"), dbManager->sqlDb());
            while (query.next()) {
                list.append(query.value(0).toString());
            }
        }
        m_artistCompleter = new QCompleter(list, this);
        m_artistCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        m_ui->artistEdit->setCompleter(m_artistCompleter);
        list.clear();

        {
            QSqlQuery query(QLatin1String("SELECT album FROM albums ORDER BY album ASC;"), dbManager->sqlDb());
            while (query.next()) {
                list.append(query.value(0).toString());
            }
        }
        m_albumCompleter = new QCompleter(list, this);
        m_albumCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        m_ui->albumEdit->setCompleter(m_albumCompleter);
        list.clear();

        {
            QSqlQuery query(QLatin1String("SELECT genre FROM genres ORDER BY genre ASC;"), dbManager->sqlDb());
            while (query.next()) {
                list.append(query.value(0).toString());
            }
        }
        m_genreCompleter = new QCompleter(list, this);
        m_genreCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        m_ui->genreEdit->setCompleter(m_genreCompleter);
    }
}

MetadataEditor::~MetadataEditor()
{
    delete m_ui;

    delete m_artistCompleter;
    delete m_albumCompleter;
    delete m_genreCompleter;
}

MetaData MetadataEditor::metaData()
{
    m_metaData.setAlbum(m_ui->albumEdit->text());
    m_metaData.setArtist(m_ui->artistEdit->text());
    m_metaData.setGenre(m_ui->genreEdit->text());
    m_metaData.setTrackNumber(m_ui->trackNumberEdit->text().toUInt());
    m_metaData.setTitle(m_ui->trackTitleEdit->text());
    m_metaData.setYear(m_ui->yearEdit->value());
    return m_metaData;
}

void MetadataEditor::setMetaData(const MetaData &metaData)
{
    m_metaData = metaData;
    m_ui->albumEdit->setText(metaData.album());
    m_ui->artistEdit->setText(metaData.artist());
    m_ui->genreEdit->setText(metaData.genre());
    m_ui->trackNumberEdit->setText(QString::number(metaData.trackNumber()));
    m_ui->trackTitleEdit->setText(metaData.title());
    m_ui->yearEdit->setValue(metaData.year());
}

void MetadataEditor::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);

    m_ui->box->setTitle(fontMetrics().elidedText(m_metaData.fileName(), Qt::ElideMiddle, m_ui->box->width()));
}
