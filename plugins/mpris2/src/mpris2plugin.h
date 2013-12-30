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

#ifndef MPRIS2PLUGIN_H
#define MPRIS2PLUGIN_H

#include <core/abstractplugin.h>

class QDBusAbstractAdaptor;

class MPRIS2Plugin : public TepSonic::AbstractPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.progdan.tepsonic.plugins.MPRIS2"
                      FILE "MPRIS2Plugin.json")

  public:
    explicit MPRIS2Plugin();
    virtual ~MPRIS2Plugin();

    virtual void init();

  private:
    QDBusAbstractAdaptor *m_mediaPlayer2;
    QDBusAbstractAdaptor *m_mediaPlayer2Player;
};

#endif // MPRIS2PLUGIN_H
