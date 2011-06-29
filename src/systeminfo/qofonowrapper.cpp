/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystems module of the Qt Toolkit.
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

static const QString OFONO_SERVICE(QString::fromAscii("org.ofono"));
static const QString OFONO_MANAGER_INTERFACE(QString::fromAscii("org.ofono.Manager"));
static const QString OFONO_MANAGER_PATH(QString::fromAscii("/"));
static const QString OFONO_MODEM_INTERFACE(QString::fromAscii("org.ofono.Modem"));
static const QString OFONO_NETWORK_REGISTRATION_INTERFACE(QString::fromAscii("org.ofono.NetworkRegistration"));
static const QString OFONO_SIM_MANAGER_INTERFACE(QString::fromAscii("org.ofono.SimManager"));

QT_BEGIN_NAMESPACE

/*!
    \internal
    \class QOfonoWrapper
    \brief QOfonoWrapper is a wrapper for OFONO DBus APIs.
*/

int QOfonoWrapper::available = -1;

QOfonoWrapper::QOfonoWrapper(QObject *parent)
    : QObject(parent)
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
QStringList QOfonoWrapper::allModems()
{
    QDBusReply<QOfonoPropertyMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, OFONO_MANAGER_PATH, OFONO_MANAGER_INTERFACE, QString::fromAscii("GetModems")));

    QStringList modems;
    if (reply.isValid()) {
        foreach (const QOfonoProperties &property, reply.value())
            modems << property.path.path();
    }
    return modems;
}

// Network Registration Interface
int QOfonoWrapper::signalStrength(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromAscii("GetProperties")));

    return reply.value().value(QString::fromAscii("Strength")).toInt();
}

QList<QDBusObjectPath> QOfonoWrapper::allOperators(const QString &modemPath)
{
    QDBusReply<QOfonoPropertyMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromAscii("GetOperators")));

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
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromAscii("GetProperties")));

    return statusStringToEnum(reply.value().value(QString::fromAscii("Status")).toString());
}

QString QOfonoWrapper::cellId(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromAscii("GetProperties")));

    return reply.value().value(QString::fromAscii("CellId")).toString();
}

QString QOfonoWrapper::currentMcc(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromAscii("GetProperties")));

    return reply.value().value(QString::fromAscii("MobileCountryCode")).toString();
}

QString QOfonoWrapper::currentMnc(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromAscii("GetProperties")));

    return reply.value().value(QString::fromAscii("MobileNetworkCode")).toString();
}

QString QOfonoWrapper::lac(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromAscii("GetProperties")));

    return reply.value().value(QString::fromAscii("LocationAreaCode")).toString();
}

QString QOfonoWrapper::operatorName(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromAscii("GetProperties")));

    return reply.value().value(QString::fromAscii("Name")).toString();
}

// SIM Manager Interface
QString QOfonoWrapper::homeMcc(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_SIM_MANAGER_INTERFACE, QString::fromAscii("GetProperties")));

    return reply.value().value(QString::fromAscii("MobileCountryCode")).toString();
}

QString QOfonoWrapper::homeMnc(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_SIM_MANAGER_INTERFACE, QString::fromAscii("GetProperties")));

    return reply.value().value(QString::fromAscii("MobileNetworkCode")).toString();
}

QString QOfonoWrapper::imsi(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_SIM_MANAGER_INTERFACE, QString::fromAscii("GetProperties")));

    return reply.value().value(QString::fromAscii("SubscriberIdentity")).toString();
}

// Modem Interface
QString QOfonoWrapper::imei(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_MODEM_INTERFACE, QString::fromAscii("GetProperties")));

    return reply.value().value(QString::fromAscii("Serial")).toString();
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
        QStringList modems = allModems();
        foreach (const QString &modem, modems) {
            QDBusConnection::systemBus().connect(OFONO_SERVICE,
                                                 modem,
                                                 OFONO_NETWORK_REGISTRATION_INTERFACE,
                                                 QString::fromAscii("PropertyChanged"),
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

    QStringList modems = allModems();
    foreach (const QString &modem, modems) {
        QDBusConnection::systemBus().disconnect(OFONO_SERVICE,
                                                modem,
                                                OFONO_NETWORK_REGISTRATION_INTERFACE,
                                                QString::fromAscii("PropertyChanged"),
                                                this, SLOT(onOfonoPropertyChanged(QString,QDBusVariant)));
    }
}

void QOfonoWrapper::onOfonoPropertyChanged(const QString &property, const QDBusVariant &value)
{
    if (!calledFromDBus())
        return;

    int interface = allModems().indexOf(message().path());

    if (property == QString::fromAscii("MobileCountryCode"))
        Q_EMIT currentMobileCountryCodeChanged(interface, value.variant().toString());
    else if (property == QString::fromAscii("MobileNetworkCode"))
        Q_EMIT currentMobileNetworkCodeChanged(interface, value.variant().toString());
    else if (property == QString::fromAscii("CellId"))
        Q_EMIT cellIdChanged(interface, value.variant().toString());
    else if (property == QString::fromAscii("Technology"))
        Q_EMIT currentCellDataTechnologyChanged(interface, technologyStringToEnum(value.variant().toString()));
    else if (property == QString::fromAscii("LocationAreaCode"))
        Q_EMIT locationAreaCodeChanged(interface, value.variant().toString());
    else if (property == QString::fromAscii("Name"))
        Q_EMIT networkNameChanged(technologyToMode(currentTechnology(message().path())), interface, value.variant().toString());
    else if (property == QString::fromAscii("Strength"))
        Q_EMIT networkSignalStrengthChanged(technologyToMode(currentTechnology(message().path())), interface, value.variant().toInt());
    else if (property == QString::fromAscii("Status"))
        Q_EMIT networkStatusChanged(technologyToMode(currentTechnology(message().path())), interface, statusStringToEnum(value.variant().toString()));
}

QNetworkInfo::CellDataTechnology QOfonoWrapper::technologyStringToEnum(const QString &technology)
{
    if (technology == QString::fromAscii("edge"))
        return QNetworkInfo::EdgeDataTechnology;
    else if (technology == QString::fromAscii("umts"))
        return QNetworkInfo::UmtsDataTechnology;
    else if (technology == QString::fromAscii("hspa"))
        return QNetworkInfo::HspaDataTechnology;
    else
        return QNetworkInfo::UnknownDataTechnology;
}

QNetworkInfo::NetworkMode QOfonoWrapper::technologyToMode(const QString &technology)
{
    if (technology == QString::fromAscii("umts"))
        return QNetworkInfo::WcdmaMode;
    else if (technology == QString::fromAscii("lte"))
        return QNetworkInfo::LteMode;
    else
        return QNetworkInfo::GsmMode;
}

QNetworkInfo::NetworkStatus QOfonoWrapper::statusStringToEnum(const QString &status)
{
    if (status == QString::fromAscii("unregistered"))
        return QNetworkInfo::NoNetworkAvailable;
    else if (status == QString::fromAscii("registered"))
        return QNetworkInfo::HomeNetwork;
    else if (status == QString::fromAscii("searching"))
        return QNetworkInfo::Searching;
    else if (status == QString::fromAscii("denied"))
        return QNetworkInfo::Denied;
    else if (status == QString::fromAscii("roaming"))
        return QNetworkInfo::Roaming;
    else
        return QNetworkInfo::UnknownStatus;
}

QString QOfonoWrapper::currentTechnology(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromAscii("GetProperties")));
    return reply.value().value(QString::fromAscii("Technology")).toString();
}

QT_END_NAMESPACE

#endif // QT_NO_OFONO
