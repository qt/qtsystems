/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdevicekitservice_linux_p.h"

QT_BEGIN_NAMESPACE

#define UPOWER_SERVICE        "org.freedesktop.UPower"
#define UPOWER_PATH           "/org/freedesktop/UPower"
#define UPOWER_DEVICE_SERVICE "org.freedesktop.UPower.Device"
#define UPOWER_DEVICE_PATH    "/org/freedesktop/UPower/Device"

QUPowerInterface::QUPowerInterface(QObject *parent)
    : QDBusAbstractInterface(QLatin1String(UPOWER_SERVICE)
    , QLatin1String(UPOWER_PATH)
    , UPOWER_SERVICE
    , QDBusConnection::systemBus()
    , parent)
{
}

QUPowerInterface::~QUPowerInterface()
{
}

QList<QDBusObjectPath> QUPowerInterface::enumerateDevices()
{
    QDBusPendingReply<QList<QDBusObjectPath> > reply = call(QLatin1String("EnumerateDevices"));
    reply.waitForFinished();
    if (reply.isError())
        qDebug() << reply.error().message();
    return reply.value();
}

void QUPowerInterface::connectNotify(const QMetaMethod &signal)
{
    static const QMetaMethod addedSignal = QMetaMethod::fromSignal(&QUPowerInterface::deviceAdded);
    static const QMetaMethod removedSignal = QMetaMethod::fromSignal(&QUPowerInterface::deviceRemoved);

    if (signal == addedSignal) {
        if (!connection().connect(QStringLiteral(UPOWER_SERVICE),
                                  QStringLiteral(UPOWER_PATH),
                                  QStringLiteral(UPOWER_SERVICE),
                                  QStringLiteral("DeviceAdded"),
                                  this, SIGNAL(onDeviceAdded(QDBusObjectPath)))) {
            qDebug() << "Error"<<connection().lastError().message();
        }

    }
    if (signal == removedSignal) {
        if (!connection().connect(QStringLiteral(UPOWER_SERVICE),
                                  QStringLiteral(UPOWER_PATH),
                                  QStringLiteral(UPOWER_SERVICE),
                                  QStringLiteral("DeviceRemoved"),
                                  this, SIGNAL(onDeviceRemoved(QDBusObjectPath)))) {
            qDebug() << "Error"<<connection().lastError().message();
        }
    }
}

void QUPowerInterface::disconnectNotify(const QMetaMethod &signal)
{
    static const QMetaMethod addedSignal = QMetaMethod::fromSignal(&QUPowerInterface::deviceAdded);
    static const QMetaMethod removedSignal = QMetaMethod::fromSignal(&QUPowerInterface::deviceRemoved);

    if (signal == addedSignal) {
        if (!connection().disconnect(QStringLiteral(UPOWER_SERVICE),
                                  QStringLiteral(UPOWER_PATH),
                                  QStringLiteral(UPOWER_SERVICE),
                                  QStringLiteral("DeviceAdded"),
                                  this, SLOT(onDeviceAdded(QDBusObjectPath)))) {
            qDebug() << "Error"<<connection().lastError().message();
        }

    }
    if (signal == removedSignal) {
        if (!connection().disconnect(QStringLiteral(UPOWER_SERVICE),
                                  QStringLiteral(UPOWER_PATH),
                                  QStringLiteral(UPOWER_SERVICE),
                                  QStringLiteral("DeviceRemoved"),
                                  this, SLOT(onDeviceRemoved(QDBusObjectPath)))) {
            qDebug() << "Error"<<connection().lastError().message();
        }
    }

}

void QUPowerInterface::onDeviceAdded(const QDBusObjectPath &path)
{
    Q_EMIT deviceAdded(path.path());
}

void QUPowerInterface::onDeviceRemoved(const QDBusObjectPath &path)
{
    Q_EMIT deviceRemoved(path.path());
}

QUPowerDeviceInterface::QUPowerDeviceInterface(const QString &dbusPathName, QObject *parent)
    : QDBusAbstractInterface(QLatin1String(UPOWER_SERVICE)
    , dbusPathName
    , UPOWER_DEVICE_SERVICE
    , QDBusConnection::systemBus()
    , parent)
{
    pMap = getProperties();
}

QUPowerDeviceInterface::~QUPowerDeviceInterface()
{
}

QVariantMap QUPowerDeviceInterface::getProperties()
{
    QDBusMessage propGetMsg = QDBusMessage::createMethodCall(UPOWER_SERVICE, path(), QStringLiteral("org.freedesktop.DBus.Properties"), QLatin1String("GetAll"));
    QList<QVariant> arguments;
    arguments << QLatin1String("org.freedesktop.UPower.Device");
    propGetMsg.setArguments(arguments);
    QDBusMessage propReply = QDBusConnection::systemBus().call(propGetMsg);

    if (propReply.type() == QDBusMessage::ErrorMessage) {
        // don't throw away the existing map if the call fails
        qWarning() << "QUPowerDeviceInterface: error getting properties: " << propReply.errorMessage();
        return pMap;
    }

    QList<QVariant> args = propReply.arguments();

    if (!args.length()) {
        qWarning() << "QUPowerDeviceInterface: Got an empty property reply";
        pMap = QVariantMap();
        return pMap;
    }

    pMap = args[0].toMap();
    return pMap;
}

QVariant QUPowerDeviceInterface::getProperty(const QString &property)
{
    return pMap.value(property);
}

quint16 QUPowerDeviceInterface::type()
{
    return getProperty(QStringLiteral("Type")).toUInt();
}

bool QUPowerDeviceInterface::isPowerSupply()
{
    return getProperty(QStringLiteral("PowerSupply")).toBool();
}

bool QUPowerDeviceInterface::isOnline()
{
    return getProperty(QStringLiteral("Online")).toBool();
}

double QUPowerDeviceInterface::currentEnergy()
{
    return getProperty(QStringLiteral("Energy")).toDouble();
}

double QUPowerDeviceInterface::energyWhenFull()
{
    return getProperty(QStringLiteral("EnergyFull")).toDouble();
}

double QUPowerDeviceInterface::energyDischargeRate()
{
    return getProperty(QStringLiteral("EnergyRate")).toDouble();
}

double QUPowerDeviceInterface::voltage()
{
    return getProperty(QStringLiteral("Voltage")).toDouble();
}

qint64 QUPowerDeviceInterface::timeToFull()
{
    return getProperty(QStringLiteral("TimeToFull")).toUInt();
}

double QUPowerDeviceInterface::percentLeft()
{
    return getProperty(QStringLiteral("Percentage")).toDouble();
}

quint16 QUPowerDeviceInterface::state()
{
    return getProperty(QStringLiteral("State")).toUInt();
}

void QUPowerDeviceInterface::connectNotify(const QMetaMethod &signal)
{
    static const QMetaMethod changedSignal = QMetaMethod::fromSignal(&QUPowerDeviceInterface::changed);
    static const QMetaMethod propertyChangedSignal = QMetaMethod::fromSignal(&QUPowerDeviceInterface::propertyChanged);

    if (signal == changedSignal) {
        if (!connection().connect(QLatin1String(UPOWER_SERVICE),
                                  path(),
                                  QLatin1String(UPOWER_DEVICE_SERVICE),
                                  QLatin1String("Changed"),
                                  this, SIGNAL(changed()))) {
            qDebug() << "Error" << connection().lastError().message();
        }
    }
    if (signal == propertyChangedSignal) {
        if (!connection().connect(QLatin1String(UPOWER_SERVICE),
                                  path(),
                                  QLatin1String(UPOWER_DEVICE_SERVICE),
                                  QLatin1String("Changed"),
                                  this, SIGNAL(propChanged()))) {
            qDebug() << "Error" << connection().lastError().message();
        }
    }
}

void QUPowerDeviceInterface::disconnectNotify(const QMetaMethod &signal)
{
    static const QMetaMethod changedSignal = QMetaMethod::fromSignal(&QUPowerDeviceInterface::changed);
    static const QMetaMethod propertyChangedSignal = QMetaMethod::fromSignal(&QUPowerDeviceInterface::propertyChanged);
    if (signal == changedSignal) {
        connection().disconnect(QLatin1String(UPOWER_SERVICE),
                                path(),
                                QLatin1String(UPOWER_DEVICE_SERVICE),
                                QLatin1String("Changed"),
                                this, SIGNAL(changed()));
    }
    if (signal == propertyChangedSignal) {
        connection().disconnect(QLatin1String(UPOWER_SERVICE),
                                path(),
                                QLatin1String(UPOWER_DEVICE_SERVICE),
                                QLatin1String("Changed"),
                                this, SIGNAL(propChanged()));
    }
}

void QUPowerDeviceInterface::propChanged()
{
    QVariantMap map = pMap;
    QMapIterator<QString, QVariant> i(getProperties());

    while (i.hasNext()) {
        i.next();
        if (pMap.value(i.key()) != map.value(i.key())) {
            Q_EMIT propertyChanged(i.key(), QVariant::fromValue(i.value()));
        }
    }
}

QString QUPowerDeviceInterface::nativePath()
{
    return getProperty(QStringLiteral("NativePath")).toString();
}

QT_END_NAMESPACE
