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

#ifndef CHANGESHORTCUTDIALOG_H
#define CHANGESHORTCUTDIALOG_H

#include <QDialog>
#include <QKeySequence>
#include <QKeyEvent>
#include <QModelIndex>

namespace Ui
{
class ChangeShortcutDialog;
}

namespace SettingsPages
{

class ChangeShortcutDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit ChangeShortcutDialog(const QModelIndex &index, QWidget *parent = 0);
    ~ChangeShortcutDialog();

  protected:
    void changeEvent(QEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

  private:
    ::Ui::ChangeShortcutDialog *m_ui;
    QModelIndex m_index;
    QKeySequence m_shortcut;

    QList<Qt::KeyboardModifiers> m_modifiers;

  private Q_SLOTS:
    void accept();

  Q_SIGNALS:
    void shortcutChanged(const QModelIndex &index, const QKeySequence &newshortcut);
};

}

#endif // CHANGESHORTCUTDIALOG_H
