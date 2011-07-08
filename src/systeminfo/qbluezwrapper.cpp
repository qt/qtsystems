/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbluezwrapper_p.h"

#include <QtDBus/qdbusconnection.h>
#include <QtDBus/qdbusconnectioninterface.h>
#include <QtDBus/qdbusreply.h>

#if !defined(QT_NO_BLUEZ)

QT_BEGIN_NAMESPACE

static const QString BLUEZ_SERVICE(QString::fromAscii("org.bluez"));
static const QString BLUEZ_MANAGER_INTERFACE(QString::fromAscii("org.bluez.Manager"));
static const QString BLUEZ_MANAGER_PATH(QString::fromAscii("/"));
static const QString BLUEZ_ADAPTER_INTERFACE(QString::fromAscii("org.bluez.Adapter"));
static const QString BLUEZ_DEVICE_INTERFACE(QString::fromAscii("org.bluez.Device"));

static const QString GET_PROPERTIES(QString::fromAscii("GetProperties"));

// TODO check where these magic numbers are defined
static const int BLUEZ_INPUT_DEVICE_CLASS(9536);

/*!
    \internal
    \class QBluezWrapper
    \brief QBluezWrapper is a wrapper for BLUEZ DBus APIs.
*/

int QBluezWrapper::available = -1;

QBluezWrapper::QBluezWrapper(QObject *parent)
    : QObject(parent)
{
}

/*!
    \internal

    Returns true if BLUEZ is available, or false otherwise.

    Note that it only does the real checking when called for the first time, which might cost some
    time.
*/
bool QBluezWrapper::isBluezAvailable()
{
    // -1: Don't know if BLUEZ is available or not.
    //  0: BLUEZ is not available.
    //  1: BLUEZ is available.
    if (-1 == available) {
        if (QDBusConnection::systemBus().isConnected()) {
            QDBusReply<bool> reply = QDBusConnection::systemBus().interface()->isServiceRegistered(BLUEZ_SERVICE);
            if (reply.isValid())
                available = reply.value();
            else
                available = 0;
        }
    }

    return available;
}

bool QBluezWrapper::hasInputDevice()
{
    QStringList devices = allDevices();
    foreach (const QString &device, devices) {
        QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                    QDBusMessage::createMethodCall(BLUEZ_SERVICE, device, BLUEZ_DEVICE_INTERFACE, GET_PROPERTIES));
        if (reply.value().value(QString::fromAscii("Class")).toInt() == BLUEZ_INPUT_DEVICE_CLASS
            && reply.value().value(QString::fromAscii("Connected")).toBool()) {
            return true;
        }
    }
    return false;
}

void QBluezWrapper::connectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(wirelessKeyboardConnected(bool))) == 0) {
        QStringList devices = allDevices();
        foreach (const QString &device, devices) {
            QDBusConnection::systemBus().connect(BLUEZ_SERVICE,
                                                 device,
                                                 BLUEZ_DEVICE_INTERFACE,
                                                 QString::fromAscii("PropertyChanged"),
                                                 this, SLOT(onBluezPropertyChanged(QString,QDBusVariant)));
        }
    }
}

void QBluezWrapper::disconnectNotify(const char *signal)
{
    Q_UNUSED(signal)

    // We can only disconnect with the BLUEZ D-Bus signals, when there is no receivers for the signal.
    if (receivers(SIGNAL(wirelessKeyboardConnected(bool))) > 0)
        return;

    QStringList devices = allDevices();
    foreach (const QString &device, devices) {
        QDBusConnection::systemBus().disconnect(BLUEZ_SERVICE,
                                                device,
                                                BLUEZ_DEVICE_INTERFACE,
                                                QString::fromAscii("PropertyChanged"),
                                                this, SLOT(onBluezPropertyChanged(QString,QDBusVariant)));
    }
}

void QBluezWrapper::onBluezPropertyChanged(const QString &property, const QDBusVariant &value)
{
    if (!calledFromDBus() || property != QString::fromAscii("Connected"))
        return;

    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(BLUEZ_SERVICE, message().path(), BLUEZ_DEVICE_INTERFACE, GET_PROPERTIES));
    if (reply.isValid() && reply.value().value(QString::fromAscii("Class")).toInt() == BLUEZ_INPUT_DEVICE_CLASS)
        Q_EMIT wirelessKeyboardConnected(value.variant().toBool());
}

QStringList QBluezWrapper::allDevices()
{
    QStringList deviceList;

    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(BLUEZ_SERVICE, BLUEZ_MANAGER_PATH, BLUEZ_MANAGER_INTERFACE, GET_PROPERTIES));
    QList<QDBusObjectPath> adapters = qdbus_cast<QList<QDBusObjectPath> >(reply.value().value(QString::fromAscii("Adapters")));
    foreach (const QDBusObjectPath &adapter, adapters) {
        reply = QDBusConnection::systemBus().call(
                    QDBusMessage::createMethodCall(BLUEZ_SERVICE, adapter.path(), BLUEZ_ADAPTER_INTERFACE, GET_PROPERTIES));
        QList<QDBusObjectPath> devices = qdbus_cast<QList<QDBusObjectPath> >(reply.value().value(QString::fromAscii("Devices")));
        foreach (const QDBusObjectPath &device, devices)
            deviceList << device.path();
    }

    return deviceList;
}

QT_END_NAMESPACE

#endif // QT_NO_BLUEZ
