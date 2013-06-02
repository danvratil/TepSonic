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



#ifndef METADATAEDITOR_H
#define METADATAEDITOR_H

#include <QtGui/QDialog>
#include <QtGui/QCompleter>

#include "ui_metadataeditor.h"

class MetadataEditor : public QDialog
{
    Q_OBJECT

public:
    explicit MetadataEditor(QWidget *parent = 0);
    ~MetadataEditor();

    QString trackTitle() const {
        return m_ui->trackTitleEdit->text();
    }
    void setTrackTitle(const QString &title) {
        m_ui->trackTitleEdit->setText(title);
    }

    QString artist() const {
        return m_ui->artistEdit->text();
    }
    void setArtist(const QString &artist) {
        m_ui->artistEdit->setText(artist);
    }

    QString album() const {
        return m_ui->albumEdit->text();
    }
    void setAlbum(const QString &album) {
        m_ui->albumEdit->setText(album);
    }

    int year() const {
        return m_ui->yearEdit->value();
    }
    void setYear(int year) {
        m_ui->yearEdit->setValue(year);
    }

    QString genre() const {
        return m_ui->genreEdit->text();
    }
    void setGenre(const QString &genre) {
        m_ui->genreEdit->setText(genre);
    }

    int trackNumber() const {
        return m_ui->trackNumberEdit->text().toInt();
    }
    void setTrackNumber(int trackNumber) {
        m_ui->trackNumberEdit->setText(QString::number(trackNumber));
    }

    QString filename() const {
        return m_filename;
    }
    void setFilename(const QString &filename);

protected:
    void resizeEvent(QResizeEvent *);


private:
    Ui::MetadataEditor *m_ui;

    QString m_filename;

    QCompleter *m_artistCompleter;
    QCompleter *m_albumCompleter;
    QCompleter *m_genreCompleter;

};

#endif // METADATAEDITOR_H
