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

#include "qofonowrapper_p.h"

#include <QtDBus/qdbusconnection.h>
#include <QtDBus/qdbusconnectioninterface.h>
#include <QtDBus/qdbusmetatype.h>
#include <QtDBus/qdbusreply.h>

#if !defined(QT_NO_OFONO)

QT_BEGIN_NAMESPACE

static const QString OFONO_SERVICE(QString::fromUtf8("org.ofono"));
static const QString OFONO_MANAGER_INTERFACE(QString::fromUtf8("org.ofono.Manager"));
static const QString OFONO_MANAGER_PATH(QString::fromUtf8("/"));
static const QString OFONO_MODEM_INTERFACE(QString::fromUtf8("org.ofono.Modem"));
static const QString OFONO_NETWORK_REGISTRATION_INTERFACE(QString::fromUtf8("org.ofono.NetworkRegistration"));
static const QString OFONO_SIM_MANAGER_INTERFACE(QString::fromUtf8("org.ofono.SimManager"));

struct QOfonoProperty
{
    QDBusObjectPath path;
    QVariantMap properties;
};
Q_DECLARE_METATYPE(QOfonoProperty)

typedef QList<QOfonoProperty> QOfonoPropertyMap;
Q_DECLARE_METATYPE(QOfonoPropertyMap)

QDBusArgument &operator<<(QDBusArgument &argument, const QOfonoProperty &prop)
{
    argument.beginStructure();
    argument << prop.path << prop.properties;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QOfonoProperty &prop)
{
    argument.beginStructure();
    argument >> prop.path >> prop.properties;
    argument.endStructure();
    return argument;
}

/*!
    \internal
    \class QOfonoWrapper
    \brief QOfonoWrapper is a wrapper for OFONO DBus APIs.
*/

int QOfonoWrapper::available = -1;

QOfonoWrapper::QOfonoWrapper(QObject *parent)
    : QObject(parent)
    , watchAllModems(false)
    , watchProperties(false)
{
    qDBusRegisterMetaType<QOfonoProperty>();
    qDBusRegisterMetaType<QOfonoPropertyMap>();
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
    if (watchAllModems)
        return allModemPaths;
    else
        return getAllModems();
}

// Network Registration Interface
int QOfonoWrapper::signalStrength(const QString &modemPath)
{
    if (watchProperties)
        return signalStrengths.value(modemPath);
    else
        return getSignalStrength(modemPath);
}

QNetworkInfo::CellDataTechnology QOfonoWrapper::currentCellDataTechnology(const QString &modemPath)
{
    if (watchProperties)
        return currentCellDataTechnologies.value(modemPath);
    else
        return getCurrentCellDataTechnology(modemPath);
}

QNetworkInfo::NetworkStatus QOfonoWrapper::networkStatus(const QString &modemPath)
{
    if (watchProperties)
        return networkStatuses.value(modemPath);
    else
        return getNetworkStatus(modemPath);
}

QString QOfonoWrapper::cellId(const QString &modemPath)
{
    if (watchProperties)
        return cellIds.value(modemPath);
    else
        return getCellId(modemPath);
}

QString QOfonoWrapper::currentMcc(const QString &modemPath)
{
    if (watchProperties)
        return currentMccs.value(modemPath);
    else
        return getCurrentMcc(modemPath);
}

QString QOfonoWrapper::currentMnc(const QString &modemPath)
{
    if (watchProperties)
        return currentMncs.value(modemPath);
    else
        return getCurrentMnc(modemPath);
}

QString QOfonoWrapper::lac(const QString &modemPath)
{
    if (watchProperties)
        return lacs.value(modemPath);
    else
        return getLac(modemPath);
}

QString QOfonoWrapper::operatorName(const QString &modemPath)
{
    if (watchProperties)
        return operatorNames.value(modemPath);
    else
        return getOperatorName(modemPath);
}

QNetworkInfo::NetworkMode QOfonoWrapper::networkMode(const QString& modemPath)
{
    return technologyToMode(currentTechnology(modemPath));
}

// SIM Manager Interface
QString QOfonoWrapper::homeMcc(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_SIM_MANAGER_INTERFACE, QString::fromUtf8("GetProperties")));

    return reply.value().value(QString::fromAscii("MobileCountryCode")).toString();
}

QString QOfonoWrapper::homeMnc(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_SIM_MANAGER_INTERFACE, QString::fromUtf8("GetProperties")));

    return reply.value().value(QString::fromAscii("MobileNetworkCode")).toString();
}

QString QOfonoWrapper::imsi(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_SIM_MANAGER_INTERFACE, QString::fromUtf8("GetProperties")));

    return reply.value().value(QString::fromAscii("SubscriberIdentity")).toString();
}

// Modem Interface
QString QOfonoWrapper::imei(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_MODEM_INTERFACE, QString::fromUtf8("GetProperties")));

    return reply.value().value(QString::fromAscii("Serial")).toString();
}

void QOfonoWrapper::connectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(networkInterfaceCountChanged(QNetworkInfo::NetworkMode,int))) == 0) {
        allModemPaths = getAllModems();
        QDBusConnection::systemBus().connect(OFONO_SERVICE, OFONO_MANAGER_PATH, OFONO_MANAGER_INTERFACE,
                                             QString::fromUtf8("ModemAdded"),
                                             this, SLOT(onOfonoModemAdded(QDBusObjectPath)));
        QDBusConnection::systemBus().connect(OFONO_SERVICE, OFONO_MANAGER_PATH, OFONO_MANAGER_INTERFACE,
                                             QString::fromUtf8("ModemRemoved"),
                                             this, SLOT(onOfonoModemRemoved(QDBusObjectPath)));
        watchAllModems = true;
    } else if (strcmp(signal, SIGNAL(currentMobileCountryCodeChanged(int,QString))) == 0
               || strcmp(signal, SIGNAL(currentMobileNetworkCodeChanged(int,QString))) == 0
               || strcmp(signal, SIGNAL(cellIdChanged(int,QString))) == 0
               || strcmp(signal, SIGNAL(currentCellDataTechnologyChanged(int,QNetworkInfo::CellDataTechnology))) == 0
               || strcmp(signal, SIGNAL(locationAreaCodeChanged(int,QString))) == 0
               || strcmp(signal, SIGNAL(networkNameChanged(QNetworkInfo::NetworkMode,QString))) == 0
               || strcmp(signal, SIGNAL(networkSignalStrengthChanged(QNetworkInfo::NetworkMode,int))) == 0
               || strcmp(signal, SIGNAL(networkStatusChanged(QNetworkInfo::NetworkMode,QNetworkInfo::NetworkStatus))) == 0) {
        signalStrengths.clear();
        currentCellDataTechnologies.clear();
        networkStatuses.clear();
        cellIds.clear();
        currentMccs.clear();
        currentMncs.clear();
        lacs.clear();
        operatorNames.clear();
        QStringList modems = allModems();
        foreach (const QString &modem, modems) {
            signalStrengths[modem] = getSignalStrength(modem);
            currentCellDataTechnologies[modem] = getCurrentCellDataTechnology(modem);
            networkStatuses[modem] = getNetworkStatus(modem);
            cellIds[modem] = getCellId(modem);
            currentMccs[modem] = getCurrentMcc(modem);
            currentMncs[modem] = getCurrentMnc(modem);
            lacs[modem] = getLac(modem);
            operatorNames[modem] = getOperatorName(modem);
            QDBusConnection::systemBus().connect(OFONO_SERVICE,
                                                 modem,
                                                 OFONO_NETWORK_REGISTRATION_INTERFACE,
                                                 QString::fromUtf8("PropertyChanged"),
                                                 this, SLOT(onOfonoPropertyChanged(QString,QDBusVariant)));
        }
        watchProperties = true;
    }
}

void QOfonoWrapper::disconnectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(networkInterfaceCountChanged(QNetworkInfo::NetworkMode,int))) == 0) {
        QDBusConnection::systemBus().disconnect(OFONO_SERVICE, OFONO_MANAGER_PATH, OFONO_MANAGER_INTERFACE,
                                                QString::fromUtf8("ModemAdded"),
                                                this, SLOT(onOfonoModemAdded(QDBusObjectPath)));
        QDBusConnection::systemBus().disconnect(OFONO_SERVICE, OFONO_MANAGER_PATH, OFONO_MANAGER_INTERFACE,
                                                QString::fromUtf8("ModemRemoved"),
                                                this, SLOT(onOfonoModemRemoved(QDBusObjectPath)));
        watchAllModems = false;
    } else if (strcmp(signal, SIGNAL(currentMobileCountryCodeChanged(int,QString))) == 0
               || strcmp(signal, SIGNAL(currentMobileNetworkCodeChanged(int,QString))) == 0
               || strcmp(signal, SIGNAL(cellIdChanged(int,QString))) == 0
               || strcmp(signal, SIGNAL(currentCellDataTechnologyChanged(int,QNetworkInfo::CellDataTechnology))) == 0
               || strcmp(signal, SIGNAL(locationAreaCodeChanged(int,QString))) == 0
               || strcmp(signal, SIGNAL(networkNameChanged(QNetworkInfo::NetworkMode,QString))) == 0
               || strcmp(signal, SIGNAL(networkSignalStrengthChanged(QNetworkInfo::NetworkMode,int))) == 0
               || strcmp(signal, SIGNAL(networkStatusChanged(QNetworkInfo::NetworkMode,QNetworkInfo::NetworkStatus))) == 0) {
        QStringList modems = allModems();
        foreach (const QString &modem, modems) {
            QDBusConnection::systemBus().disconnect(OFONO_SERVICE,
                                                    modem,
                                                    OFONO_NETWORK_REGISTRATION_INTERFACE,
                                                    QString::fromUtf8("PropertyChanged"),
                                                    this, SLOT(onOfonoPropertyChanged(QString,QDBusVariant)));
        }
    }
}

void QOfonoWrapper::onOfonoModemAdded(const QDBusObjectPath &path)
{
    allModemPaths.append(path.path());
    emit networkInterfaceCountChanged(QNetworkInfo::GsmMode, allModemPaths.size());
    emit networkInterfaceCountChanged(QNetworkInfo::CdmaMode, allModemPaths.size());
    emit networkInterfaceCountChanged(QNetworkInfo::WcdmaMode, allModemPaths.size());
    emit networkInterfaceCountChanged(QNetworkInfo::LteMode, allModemPaths.size());
}

void QOfonoWrapper::onOfonoModemRemoved(const QDBusObjectPath &path)
{
    allModemPaths.removeOne(path.path());
    emit networkInterfaceCountChanged(QNetworkInfo::GsmMode, allModemPaths.size());
    emit networkInterfaceCountChanged(QNetworkInfo::CdmaMode, allModemPaths.size());
    emit networkInterfaceCountChanged(QNetworkInfo::WcdmaMode, allModemPaths.size());
    emit networkInterfaceCountChanged(QNetworkInfo::LteMode, allModemPaths.size());
}

void QOfonoWrapper::onOfonoPropertyChanged(const QString &property, const QDBusVariant &value)
{
    if (!calledFromDBus())
        return;

    int interface = allModems().indexOf(message().path());

    if (property == QString::fromUtf8("MobileCountryCode"))
        emit currentMobileCountryCodeChanged(interface, value.variant().toString());
    else if (property == QString::fromUtf8("MobileNetworkCode"))
        emit currentMobileNetworkCodeChanged(interface, value.variant().toString());
    else if (property == QString::fromUtf8("CellId"))
        emit cellIdChanged(interface, value.variant().toString());
    else if (property == QString::fromUtf8("Technology"))
        emit currentCellDataTechnologyChanged(interface, technologyStringToEnum(value.variant().toString()));
    else if (property == QString::fromUtf8("LocationAreaCode"))
        emit locationAreaCodeChanged(interface, value.variant().toString());
    else if (property == QString::fromUtf8("Name"))
        emit networkNameChanged(technologyToMode(currentTechnology(message().path())), interface, value.variant().toString());
    else if (property == QString::fromUtf8("Strength"))
        emit networkSignalStrengthChanged(technologyToMode(currentTechnology(message().path())), interface, value.variant().toInt());
    else if (property == QString::fromUtf8("Status"))
        emit networkStatusChanged(technologyToMode(currentTechnology(message().path())), interface, statusStringToEnum(value.variant().toString()));
}

QNetworkInfo::CellDataTechnology QOfonoWrapper::technologyStringToEnum(const QString &technology)
{
    if (technology == QString::fromUtf8("edge"))
        return QNetworkInfo::EdgeDataTechnology;
    else if (technology == QString::fromUtf8("umts"))
        return QNetworkInfo::UmtsDataTechnology;
    else if (technology == QString::fromUtf8("hspa"))
        return QNetworkInfo::HspaDataTechnology;
    else
        return QNetworkInfo::UnknownDataTechnology;
}

QNetworkInfo::NetworkMode QOfonoWrapper::technologyToMode(const QString &technology)
{
    if (technology == QString::fromUtf8("lte")) {
        return QNetworkInfo::LteMode;
    } else if (technology == QString::fromUtf8("hspa")) {
        return QNetworkInfo::WcdmaMode;
    } else if (technology == QString::fromUtf8("gsm")
               || technology == QString::fromUtf8("edge")
               || technology == QString::fromUtf8("umts")) {
        return QNetworkInfo::GsmMode;
    } else {
        return QNetworkInfo::CdmaMode;
    }
}

QNetworkInfo::NetworkStatus QOfonoWrapper::statusStringToEnum(const QString &status)
{
    if (status == QString::fromUtf8("unregistered"))
        return QNetworkInfo::NoNetworkAvailable;
    else if (status == QString::fromUtf8("registered"))
        return QNetworkInfo::HomeNetwork;
    else if (status == QString::fromUtf8("searching"))
        return QNetworkInfo::Searching;
    else if (status == QString::fromUtf8("denied"))
        return QNetworkInfo::Denied;
    else if (status == QString::fromUtf8("roaming"))
        return QNetworkInfo::Roaming;
    else
        return QNetworkInfo::UnknownStatus;
}

QString QOfonoWrapper::currentTechnology(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromUtf8("GetProperties")));

    return reply.value().value(QString::fromUtf8("Technology")).toString();
}

// Manager Interface
QStringList QOfonoWrapper::getAllModems()
{
    QDBusReply<QOfonoPropertyMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, OFONO_MANAGER_PATH, OFONO_MANAGER_INTERFACE, QString::fromUtf8("GetModems")));

    QStringList modems;
    if (reply.isValid()) {
        foreach (const QOfonoProperty &property, reply.value())
            modems << property.path.path();
    }
    return modems;
}

// Network Registration Interface
int QOfonoWrapper::getSignalStrength(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromUtf8("GetProperties")));

    return reply.value().value(QString::fromAscii("Strength")).toInt();
}

QNetworkInfo::CellDataTechnology QOfonoWrapper::getCurrentCellDataTechnology(const QString &modemPath)
{
    return technologyStringToEnum(currentTechnology(modemPath));
}

QNetworkInfo::NetworkStatus QOfonoWrapper::getNetworkStatus(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromUtf8("GetProperties")));

    return statusStringToEnum(reply.value().value(QString::fromAscii("Status")).toString());
}

QString QOfonoWrapper::getCellId(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromUtf8("GetProperties")));

    return reply.value().value(QString::fromAscii("CellId")).toString();
}

QString QOfonoWrapper::getCurrentMcc(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromUtf8("GetProperties")));

    return reply.value().value(QString::fromAscii("MobileCountryCode")).toString();
}

QString QOfonoWrapper::getCurrentMnc(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromUtf8("GetProperties")));

    return reply.value().value(QString::fromAscii("MobileNetworkCode")).toString();
}

QString QOfonoWrapper::getLac(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromUtf8("GetProperties")));

    return reply.value().value(QString::fromAscii("LocationAreaCode")).toString();
}

QString QOfonoWrapper::getOperatorName(const QString &modemPath)
{
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(OFONO_SERVICE, modemPath, OFONO_NETWORK_REGISTRATION_INTERFACE, QString::fromUtf8("GetProperties")));

    return reply.value().value(QString::fromAscii("Name")).toString();
}

QT_END_NAMESPACE

#endif // QT_NO_OFONO
