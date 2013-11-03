/*
 * Copyright 2012  Alex Merry <alex.merry@kdemail.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "dbusabstractadaptor.h"

#include <QtCore/QDebug>
#include <QtCore/QMetaClassInfo>
#include <QtDBus/QDBusMessage>

DBusAbstractAdaptor::DBusAbstractAdaptor(QObject *parent):
    QDBusAbstractAdaptor(parent)
{
}

void DBusAbstractAdaptor::signalPropertyChange(const QString &property,
                                               const QVariant &value)
{
    if (m_updatedProperties.isEmpty() && m_invalidatedProperties.isEmpty()) {
        QMetaObject::invokeMethod(this, "_m_emitPropertiesChanged",
                                  Qt::QueuedConnection);
    }

    m_updatedProperties[property] = value;
}

void DBusAbstractAdaptor::signalPropertyChange(const QString &property)
{
    if (!m_invalidatedProperties.contains(property)) {
        if (m_updatedProperties.isEmpty() && m_invalidatedProperties.isEmpty()) {
            QMetaObject::invokeMethod(this, "_m_emitPropertiesChanged",
                                      Qt::QueuedConnection);
        }

        m_invalidatedProperties << property;
    }
}

void DBusAbstractAdaptor::_m_emitPropertiesChanged()
{
    if (m_updatedProperties.isEmpty() && m_invalidatedProperties.isEmpty()) {
        return;
    }

    int ifaceIndex = metaObject()->indexOfClassInfo("D-Bus Interface");
    if (ifaceIndex < 0) {
    } else {
        QDBusMessage signal = QDBusMessage::createSignal(QLatin1String("/org/mpris/MediaPlayer2"),
                                QLatin1String("org.freedesktop.DBus.Properties"),
                                QLatin1String("PropertiesChanged"));
        signal << QLatin1String(metaObject()->classInfo(ifaceIndex).value());
        signal << m_updatedProperties;
        signal << m_invalidatedProperties;
        QDBusConnection::sessionBus().send(signal);
    }

    m_updatedProperties.clear();
    m_invalidatedProperties.clear();
}
