/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystemKit module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qofonowrapper_p.h"

#include <QtDBus/qdbusconnection.h>
#include <QtDBus/qdbusconnectioninterface.h>
#include <QtDBus/qdbusreply.h>

#if !defined(QT_NO_OFONO)

#define OFONO_SERVICE                        "org.ofono"
#define OFONO_MANAGER_INTERFACE              "org.ofono.Manager"
#define OFONO_MANAGER_PATH                   "/"
#define OFONO_MODEM_INTERFACE                "org.ofono.Modem"
#define OFONO_NETWORK_REGISTRATION_INTERFACE "org.ofono.NetworkRegistration"
#define OFONO_SIM_MANAGER_INTERFACE          "org.ofono.SimManager"

QT_BEGIN_NAMESPACE

/*!
    \internal
    \class QOfonoWrapper
    \brief QOfonoWrapper is a wrapper for OFONO DBus APIs.
*/

QOfonoWrapper::QOfonoWrapper(QObject *parent)
    : QObject(parent)
    , available(-1)
{
}

/*!
    \internal

    Returns true if OFONO is available, or false otherwise.

    Note that it only does the real checking when called for the first time, which might cost some
    time.
*/
bool QOfonoWrapper::isOfonoAvailable()
{
    // -1: Don't know if OFONO is available or not.
    //  0: OFONO is not available.
    //  1: OFONO is available.
    if (-1 == available) {
        if (QDBusConnection::systemBus().isConnected()) {
            QDBusReply<bool> reply = QDBusConnection::systemBus().interface()->isServiceRegistered(OFONO_SERVICE);
            if (reply.isValid())
                available = reply.value();
            else
                available = 0;
        }
    }

    return available;
}

// Manager Interface
QDBusObjectPath QOfonoWrapper::currentModem()
{
    QList<QDBusObjectPath> modems = allModems();
    foreach (const QDBusObjectPath &modem, modems) {
        QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                    QDBusMessage::createMethodCall(QLatin1String(OFONO_SERVICE), modem.path(), OFONO_MODEM_INTERFACE, "GetProperties"));
        if (reply.value().value("Powered").toBool())
            return modem;
    }
    return QDBusObjectPath();
}

QList<QDBusObjectPath> QOfonoWrapper::allModems()
{
    QDBusReply<QOfonoPropertyMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(QLatin1String(OFONO_SERVICE), QLatin1String(OFONO_MANAGER_PATH), OFONO_MANAGER_INTERFACE, "GetModems"));

    QList<QDBusObjectPath> modems;
    if (reply.isValid()) {
        foreach (const QOfonoProperties &property, reply.value())
            modems << property.path;
    }
    return modems;
}

// Network Registration Interface
int QOfonoWrapper::signalStrength(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(QLatin1String(OFONO_SERVICE), modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, "GetProperties"));

    return reply.value().value("Strength").toInt();
}

QList<QDBusObjectPath> QOfonoWrapper::allOperators(const QString &modemPath)
{
    QDBusReply<QOfonoPropertyMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(QLatin1String(OFONO_SERVICE), modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, "GetOperators"));

    QList<QDBusObjectPath> operators;
    if (reply.isValid()) {
        foreach (const QOfonoProperties &property, reply.value())
            operators << property.path;
    }
    return operators;
}

QNetworkInfo::CellDataTechnology QOfonoWrapper::currentCellDataTechnology(const QString &modemPath)
{
    return technologyStringToEnum(currentTechnology(modemPath));
}

QNetworkInfo::NetworkStatus QOfonoWrapper::networkStatus(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(QLatin1String(OFONO_SERVICE), modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, "GetProperties"));

    return statusStringToEnum(reply.value().value("Status").toString());
}

QString QOfonoWrapper::cellId(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(QLatin1String(OFONO_SERVICE), modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, "GetProperties"));

    return reply.value().value("CellId").toString();
}

QString QOfonoWrapper::currentMcc(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(QLatin1String(OFONO_SERVICE), modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, "GetProperties"));

    return reply.value().value("MobileCountryCode").toString();
}

QString QOfonoWrapper::currentMnc(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(QLatin1String(OFONO_SERVICE), modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, "GetProperties"));

    return reply.value().value("MobileNetworkCode").toString();
}

QString QOfonoWrapper::lac(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(QLatin1String(OFONO_SERVICE), modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, "GetProperties"));

    return reply.value().value("LocationAreaCode").toString();
}

QString QOfonoWrapper::operatorName(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(QLatin1String(OFONO_SERVICE), modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, "GetProperties"));

    return reply.value().value("Name").toString();
}

// SIM Manager Interface
QString QOfonoWrapper::homeMcc(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(QLatin1String(OFONO_SERVICE), modemPath, OFONO_SIM_MANAGER_INTERFACE, "GetProperties"));

    return reply.value().value("MobileCountryCode").toString();
}

QString QOfonoWrapper::homeMnc(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(QLatin1String(OFONO_SERVICE), modemPath, OFONO_SIM_MANAGER_INTERFACE, "GetProperties"));

    return reply.value().value("MobileNetworkCode").toString();
}

QString QOfonoWrapper::imsi(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(QLatin1String(OFONO_SERVICE), modemPath, OFONO_SIM_MANAGER_INTERFACE, "GetProperties"));

    return reply.value().value("SubscriberIdentity").toString();
}

void QOfonoWrapper::connectNotify(const char *signal)
{
    // We only connect with the OFONO D-Bus signals once
    if (receivers(signal) == 1)
        return;

    if (strcmp(signal, SIGNAL(currentMobileCountryCodeChanged(int,QString))) == 0
        || strcmp(signal, SIGNAL(currentMobileNetworkCodeChanged(int,QString))) == 0
        || strcmp(signal, SIGNAL(cellIdChanged(int,QString))) == 0
        || strcmp(signal, SIGNAL(currentCellDataTechnologyChanged(int,QNetworkInfo::CellDataTechnology))) == 0
//        || strcmp(signal, SIGNAL(currentNetworkModeChanged(QNetworkInfo::NetworkMode))) == 0
        || strcmp(signal, SIGNAL(locationAreaCodeChanged(int,QString))) == 0
        || strcmp(signal, SIGNAL(networkNameChanged(QNetworkInfo::NetworkMode,QString))) == 0
        || strcmp(signal, SIGNAL(networkSignalStrengthChanged(QNetworkInfo::NetworkMode,int))) == 0
        || strcmp(signal, SIGNAL(networkStatusChanged(QNetworkInfo::NetworkMode,QNetworkInfo::NetworkStatus))) == 0) {
        QList<QDBusObjectPath> modems = allModems();
        foreach (const QDBusObjectPath &modem, modems) {
            QDBusConnection::systemBus().connect(QLatin1String(OFONO_SERVICE),
                                                 modem.path(),
                                                 QLatin1String(OFONO_NETWORK_REGISTRATION_INTERFACE),
                                                 QLatin1String("PropertyChanged"),
                                                 this, SLOT(onOfonoPropertyChanged(QString,QDBusVariant)));
        }
    }
}

void QOfonoWrapper::disconnectNotify(const char *signal)
{
    Q_UNUSED(signal)

    // We can only disconnect with the OFONO D-Bus signals, when there is no receivers for the signal.
    if (receivers(SIGNAL(currentMobileCountryCodeChanged(int,QString))) > 0
        || receivers(SIGNAL(currentMobileNetworkCodeChanged(int,QString))) > 0
        || receivers(SIGNAL(cellIdChanged(int,QString))) > 0
        || receivers(SIGNAL(currentCellDataTechnologyChanged(int,QNetworkInfo::CellDataTechnology))) > 0
//        || receivers(SIGNAL(currentNetworkModeChanged(QNetworkInfo::NetworkMode))) > 0
        || receivers(SIGNAL(locationAreaCodeChanged(int,QString))) > 0
        || receivers(SIGNAL(networkNameChanged(QNetworkInfo::NetworkMode,QString))) > 0
        || receivers(SIGNAL(networkSignalStrengthChanged(QNetworkInfo::NetworkMode,int))) > 0
        || receivers(SIGNAL(networkStatusChanged(QNetworkInfo::NetworkMode,QNetworkInfo::NetworkStatus))) > 0) {
        return;
    }

    QList<QDBusObjectPath> modems = allModems();
    foreach (const QDBusObjectPath &modem, modems) {
        QDBusConnection::systemBus().disconnect(QLatin1String(OFONO_SERVICE),
                                                modem.path(),
                                                QLatin1String(OFONO_NETWORK_REGISTRATION_INTERFACE),
                                                QLatin1String("PropertyChanged"),
                                                this, SLOT(onOfonoPropertyChanged(QString,QDBusVariant)));
    }
}

void QOfonoWrapper::onOfonoPropertyChanged(const QString &property, const QDBusVariant &value)
{
    if (!calledFromDBus())
        return;

    // TODO: multiple-interface support
    if (property == "MobileCountryCode")
        Q_EMIT currentMobileCountryCodeChanged(0, value.variant().toString());
    else if (property == "MobileNetworkCode")
        Q_EMIT currentMobileNetworkCodeChanged(0, value.variant().toString());
    else if (property == "CellId")
        Q_EMIT cellIdChanged(0, value.variant().toString());
    else if (property == "Technology")
        Q_EMIT currentCellDataTechnologyChanged(0, technologyStringToEnum(value.variant().toString()));
    else if (property == "LocationAreaCode")
        Q_EMIT locationAreaCodeChanged(0, value.variant().toString());
    else if (property == "Name")
        Q_EMIT networkNameChanged(technologyToMode(currentTechnology(message().path())), value.variant().toString());
    else if (property == "Strength")
        Q_EMIT networkSignalStrengthChanged(technologyToMode(currentTechnology(message().path())), value.variant().toInt());
    else if (property == "Status")
        Q_EMIT networkStatusChanged(technologyToMode(currentTechnology(message().path())), statusStringToEnum(value.variant().toString()));
}

QNetworkInfo::CellDataTechnology QOfonoWrapper::technologyStringToEnum(const QString &technology)
{
    if (technology == "edge")
        return QNetworkInfo::EdgeDataTechnology;
    else if (technology == "umts")
        return QNetworkInfo::UmtsDataTechnology;
    else if (technology == "hspa")
        return QNetworkInfo::HspaDataTechnology;
    else
        return QNetworkInfo::UnknownDataTechnology;
}

QNetworkInfo::NetworkMode QOfonoWrapper::technologyToMode(const QString &technology)
{
    if (technology == "umts")
        return QNetworkInfo::WcdmaMode;
    else if (technology == "lte")
        return QNetworkInfo::LteMode;
    else
        return QNetworkInfo::GsmMode;
}

QNetworkInfo::NetworkStatus QOfonoWrapper::statusStringToEnum(const QString &status)
{
    if (status == "unregistered")
        return QNetworkInfo::NoNetworkAvailable;
    else if (status == "registered")
        return QNetworkInfo::HomeNetwork;
    else if (status == "searching")
        return QNetworkInfo::Searching;
    else if (status == "denied")
        return QNetworkInfo::Denied;
    else if (status == "roaming")
        return QNetworkInfo::Roaming;
    else
        return QNetworkInfo::UnknownStatus;
}

QString QOfonoWrapper::currentTechnology(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(QLatin1String(OFONO_SERVICE), modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, "GetProperties"));
    return reply.value().value("Technology").toString();
}

QT_END_NAMESPACE

#endif // QT_NO_OFONO
