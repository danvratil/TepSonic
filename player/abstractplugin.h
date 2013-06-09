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

#ifndef ABSTRACTPLUGIN_H
#define ABSTRACTPLUGIN_H

#include <QObject>

class QWidget;
class QMenu;

class AbstractPlugin : public QObject
{
    Q_OBJECT

  public:
    enum MenuTypes {
        MainMenu = 0,
        TrayMenu = 1,
        PlaylistPopup = 2,
        CollectionsPopup = 3
    };

    explicit AbstractPlugin(QObject *parent = 0);
    virtual ~AbstractPlugin();

    virtual void init() = 0;
    virtual void quit();

    virtual void configUI(QWidget *parentWidget);
    bool hasConfigUI() const;
    void setHasConfigUI(bool hasConfigUi);

    void setPluginName(const QString &pluginName);
    QString pluginName() const;

    virtual void setupMenu(QMenu *menu, AbstractPlugin::MenuTypes menuType);
    virtual bool setupPane(QWidget *widget, QString &label);

  public Q_SLOTS:
    virtual void settingsAccepted();

  protected:
    void emitError(const QString &error);

  Q_SIGNALS:
#if !defined(Q_MOC_RUN) && !defined(DOXYGEN_SHOULD_SKIP_THIS) && !defined(IN_IDE_PARSER)
  private:
#endif
    void error(const QString &msg);

  private:
    class Private;
    Private * const d;
    friend class Private;

};

#endif // ABSTRACTPLUGIN_H
