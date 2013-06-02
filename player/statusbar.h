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

#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QtGui/QStatusBar>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>

//! StatusBar is displays various informations about state of the application
/*!
  StatusBar is subclassed from QStatusBar and displayes information about length
  of playlist (on right side) and either popups messages (for 5 seconds or so) with
  an error or information or displays progressbar with action label that are describing
  a long-time process (like populating playlist or rebuilding collection) and it's progress
*/
class StatusBar : public QStatusBar
{
    Q_OBJECT

  public:
    //! Constructor
    /*!
      \param parent pointer to parent object
    */
    StatusBar(QWidget *parent = 0);

  private:
    //! Label that describes the action
    QLabel *m_actionLabel;

    //! Progress bar that shows progress of the action
    QProgressBar *m_progressBar;

  public Q_SLOTS:
    //! Updates action description and position of progressbar
    void setProgressBar(const QString &action, int progress, int maxPosition = 100);

    //! Hides action label and progressbar
    void cancelAction();

    //! Shows progress bar that is just informing about work, not showing exact progress
    void showWorkingBar(const QString &action);

};

#endif // STATUSBAR_H
