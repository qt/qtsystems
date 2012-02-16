/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qnetworkservicewrapper_p.h"

#include <QServiceManager>
#include <QServiceFilter>

#if !defined(QT_NO_SFW_NETREG)

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(const QString, NETWORK_REGISTRATION_MANAGER, (QStringLiteral("com.nokia.mt.NetworkRegistrationManager")))
Q_GLOBAL_STATIC_WITH_ARGS(const QServiceFilter, NETWORK_REGISTRATION_MANAGER_FILTER, (*NETWORK_REGISTRATION_MANAGER()))

QNetworkServiceWrapper::QNetworkServiceWrapper(QObject *parent)
    : QObject(parent)
    , serviceManager(new QServiceManager(this))
    , watchInterfaceCount(false)
    , watchSignalStrengths(false)
    , watchTechnologies(false)
    , watchStatuses(false)
    , watchCellIds(false)
    , watchCurrentMccs(false)
    , watchCurrentMncs(false)
    , watchLacs(false)
    , watchOperatorNames(false)
    , watchNetworkModes(false)
{
    initServiceInterfaces();

    connect(serviceManager, SIGNAL(serviceAdded(const QString&,QService::Scope)),
            this, SLOT(onServiceAdded(const QString&,QService::Scope)));
    connect(serviceManager, SIGNAL(serviceRemoved(const QString&,QService::Scope)),
            this, SLOT(onServiceRemoved(const QString&,QService::Scope)));
}

void QNetworkServiceWrapper::initServiceInterfaces()
{
    QList<QServiceInterfaceDescriptor> serviceInterfaceList = serviceManager->findInterfaces(*NETWORK_REGISTRATION_MANAGER_FILTER());
    int interfaceIndex = 0;
    foreach (const QServiceInterfaceDescriptor &serviceInterface, serviceInterfaceList) {
        allNetworkManagerInterfaces.insert(interfaceIndex, serviceInterface);
        interfaceIndex++;
    }
}

bool QNetworkServiceWrapper::loadNetworkManagerInterface(int interfaceIndex)
{
    if (!loadedNetworkManagerInterfaces.contains(interfaceIndex)) {
        if (!allNetworkManagerInterfaces.contains(interfaceIndex))
            return false;
        QObject *serviceInterfaceObject = serviceManager->loadInterface(allNetworkManagerInterfaces.value(interfaceIndex));
        if (!serviceInterfaceObject)
            return false;
        loadedNetworkManagerInterfaces.insert(interfaceIndex, serviceInterfaceObject);
    }
    return true;
}

int QNetworkServiceWrapper::getNetworkInterfaceCount()
{
    return allNetworkManagerInterfaces.size();
}

int QNetworkServiceWrapper::getSignalStrength(int interfaceIndex)
{
    if (!watchSignalStrengths) {
        uint signalStrength = 0;
        if (loadNetworkManagerInterface(interfaceIndex))
            QMetaObject::invokeMethod(loadedNetworkManagerInterfaces.value(interfaceIndex), "signalBars", Q_RETURN_ARG(uint, signalStrength));
        return int(signalStrength * 5);
    } else
        return signalStrengths[interfaceIndex] * 5;
}

QNetworkInfo::CellDataTechnology QNetworkServiceWrapper::getCurrentCellDataTechnology(int interfaceIndex)
{
    if (!watchTechnologies) {
        QString technology;
        if (loadNetworkManagerInterface(interfaceIndex))
            QMetaObject::invokeMethod(loadedNetworkManagerInterfaces.value(interfaceIndex), "technology", Q_RETURN_ARG(QString, technology));
        return technologyStringToEnum(technology);
    } else
        return currentCellDataTechnologies[interfaceIndex];
}

QNetworkInfo::NetworkStatus QNetworkServiceWrapper::getNetworkStatus(int interfaceIndex)
{
    if (!watchStatuses) {
        QString networkstatus;
        if (loadNetworkManagerInterface(interfaceIndex))
            QMetaObject::invokeMethod(loadedNetworkManagerInterfaces.value(interfaceIndex), "registrationStatus", Q_RETURN_ARG(QString, networkstatus));
        return statusStringToEnum(networkstatus);
    } else
        return networkStatuses[interfaceIndex];
}

QString QNetworkServiceWrapper::getCellId(int interfaceIndex)
{
    if (!watchCellIds) {
        uint cellId = 0;
        if (loadNetworkManagerInterface(interfaceIndex))
            QMetaObject::invokeMethod(loadedNetworkManagerInterfaces.value(interfaceIndex), "cellId", Q_RETURN_ARG(uint, cellId));
        return QString::number(cellId);
    } else
        return cellIds[interfaceIndex];
}

QString QNetworkServiceWrapper::getCurrentMcc(int interfaceIndex)
{
    if (!watchCurrentMccs) {
        QString currentMcc;
        if (loadNetworkManagerInterface(interfaceIndex))
            QMetaObject::invokeMethod(loadedNetworkManagerInterfaces.value(interfaceIndex), "mobileCountryCode", Q_RETURN_ARG(QString, currentMcc));
        return currentMcc;
    } else
        return currentMccs[interfaceIndex];
}

QString QNetworkServiceWrapper::getCurrentMnc(int interfaceIndex)
{
    if (!watchCurrentMncs) {
        QString currentMnc;
        if (loadNetworkManagerInterface(interfaceIndex))
            QMetaObject::invokeMethod(loadedNetworkManagerInterfaces.value(interfaceIndex), "mobileNetworkCode", Q_RETURN_ARG(QString, currentMnc));
        return currentMnc;
    } else
        return currentMncs[interfaceIndex];
}

QString QNetworkServiceWrapper::getLac(int interfaceIndex)
{
    if (!watchLacs) {
        uint lac = 0;
        if (loadNetworkManagerInterface(interfaceIndex))
            QMetaObject::invokeMethod(loadedNetworkManagerInterfaces.value(interfaceIndex), "locationAreaCode", Q_RETURN_ARG(uint, lac));
        return QString::number(lac);
    } else
        return lacs[interfaceIndex];
}

QString QNetworkServiceWrapper::getOperatorName(int interfaceIndex)
{
    if (!watchOperatorNames) {
        QString name;
        if (loadNetworkManagerInterface(interfaceIndex))
            QMetaObject::invokeMethod(loadedNetworkManagerInterfaces.value(interfaceIndex), "providerName", Q_RETURN_ARG(QString, name));
        return name;
    } else
        return operatorNames[interfaceIndex];
}

QString QNetworkServiceWrapper::getHomeMcc(int interfaceIndex)
{
    QString homeMcc;
    if (loadNetworkManagerInterface(interfaceIndex))
        QMetaObject::invokeMethod(loadedNetworkManagerInterfaces.value(interfaceIndex), "homeMobileCountryCode", Q_RETURN_ARG(QString, homeMcc));

    return homeMcc;
}

QString QNetworkServiceWrapper::getHomeMnc(int interfaceIndex)
{
    QString homeMnc;
    if (loadNetworkManagerInterface(interfaceIndex))
        QMetaObject::invokeMethod(loadedNetworkManagerInterfaces.value(interfaceIndex), "homeMobileNetworkCode", Q_RETURN_ARG(QString, homeMnc));

    return homeMnc;
}

QString QNetworkServiceWrapper::getImsi(int interfaceIndex)
{
    QString imsi;

    if (loadNetworkManagerInterface(interfaceIndex))
        QMetaObject::invokeMethod(loadedNetworkManagerInterfaces.value(interfaceIndex), "identity", Q_RETURN_ARG(QString, imsi));

    return imsi;
}

QNetworkInfo::NetworkMode QNetworkServiceWrapper::getNetworkMode(int interfaceIndex)
{
    if (!watchNetworkModes) {
        QString technology;
        if (loadNetworkManagerInterface(interfaceIndex))
            QMetaObject::invokeMethod(loadedNetworkManagerInterfaces.value(interfaceIndex), "technology", Q_RETURN_ARG(QString, technology));
        return technologyToMode(technology);
    } else
        return networkModes[interfaceIndex];
}

QNetworkInfo::NetworkMode QNetworkServiceWrapper::getCurrentNetworkMode(QNetworkInfo::NetworkStatus status)
{
    QList<int> interfaceIndexes = allNetworkManagerInterfaces.keys();
    QNetworkInfo::NetworkMode mode = QNetworkInfo::UnknownMode;
    foreach (const int interfaceIndex, interfaceIndexes) {
        QNetworkInfo::NetworkStatus actualStatus = getNetworkStatus(interfaceIndex);
        QNetworkInfo::NetworkMode actualMode = getNetworkMode(interfaceIndex);
        if (actualStatus == status) {
            switch (actualMode) {
            case QNetworkInfo::LteMode:
                mode = actualMode;
            case QNetworkInfo::WcdmaMode:
                if (mode != QNetworkInfo::LteMode)
                    mode = actualMode;
            case QNetworkInfo::CdmaMode:
                if (mode != QNetworkInfo::LteMode && mode != QNetworkInfo::WcdmaMode)
                    mode = actualMode;
            case QNetworkInfo::GsmMode:
                if (mode != QNetworkInfo::LteMode && mode != QNetworkInfo::WcdmaMode
                        && mode != QNetworkInfo::CdmaMode)
                    mode = actualMode;
            case QNetworkInfo::TdscdmaMode:
                if (mode != QNetworkInfo::LteMode && mode != QNetworkInfo::WcdmaMode
                        && mode != QNetworkInfo::CdmaMode && mode != QNetworkInfo::GsmMode)
                    mode = actualMode;
            default:
                break;
            }
        }
    }
    return mode;
}

void QNetworkServiceWrapper::connectNotify(const char *signal)
{
    QList<int> interfaceIndexes;
    interfaceIndexes = allNetworkManagerInterfaces.keys();

    if (strcmp(signal, SIGNAL(networkInterfaceCountChanged(QNetworkInfo::NetworkMode,int))) == 0
            && !watchInterfaceCount) {
        watchInterfaceCount = true;
    } else if (strcmp(signal, SIGNAL(currentCellDataTechnologyChanged(int,QNetworkInfo::CellDataTechnology))) == 0
               && !watchTechnologies) {
        currentCellDataTechnologies.clear();
        foreach (const int interfaceIndex, interfaceIndexes) {
            currentCellDataTechnologies[interfaceIndex] = getCurrentCellDataTechnology(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(technologyChanged(const QString&)), this, SLOT(onTechnologyChanged(const QString&)));
        }
        watchTechnologies = true;
    } else if (strcmp(signal, SIGNAL(networkSignalStrengthChanged(QNetworkInfo::NetworkMode,int))) == 0
               && !watchSignalStrengths) {
        signalStrengths.clear();
        foreach (const int interfaceIndex, interfaceIndexes) {
            // ensure networkMode slot is connected to technologyChanged signal so it is easier to pass current mode in networkSignalStrengthChanged signal
            if (!watchNetworkModes) {
                networkModes[interfaceIndex] = getNetworkMode(interfaceIndex);
                connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(technologyChanged(const QString&)), this, SLOT(onNetworkModeChanged(QString)));
            }
            signalStrengths[interfaceIndex] = getSignalStrength(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(signalBarsChanged(int)), this, SLOT(onSignalStrengthChanged(int)));
        }
        watchNetworkModes = true;
        watchSignalStrengths = true;
    } else if (strcmp(signal, SIGNAL(networkStatusChanged(QNetworkInfo::NetworkMode,int,QNetworkInfo::NetworkStatus))) == 0
               && !watchStatuses) {
        networkStatuses.clear();
        foreach (const int interfaceIndex, interfaceIndexes) {
            // ensure networkMode slot is connected to technologyChanged signal so it is easier to pass current mode in networkStatusChanged signal
            if (!watchNetworkModes) {
                networkModes[interfaceIndex] = getNetworkMode(interfaceIndex);
                connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(technologyChanged(const QString&)), this, SLOT(onNetworkModeChanged(QString)));
            }
            networkStatuses[interfaceIndex] = getNetworkStatus(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(registrationStatusChanged(const QString&)), this, SLOT(onNetworkStatusChanged(const QString&)));
        }
        watchNetworkModes = true;
        watchStatuses = true;
    } else if (strcmp(signal, SIGNAL(currentMobileCountryCodeChanged(int,QString))) == 0
               && !watchCurrentMccs) {
        currentMccs.clear();
        foreach (const int interfaceIndex, interfaceIndexes) {
            currentMccs[interfaceIndex] = getCurrentMcc(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(mobileCountryCodeChanged(const QString&)), this, SLOT(onCurrentMccChanged(const QString&)));
        }
        watchCurrentMccs = true;
    } else if (strcmp(signal, SIGNAL(currentMobileNetworkCodeChanged(int,QString))) == 0
               && !watchCurrentMncs) {
        currentMncs.clear();
        foreach (const int interfaceIndex, interfaceIndexes) {
            currentMncs[interfaceIndex] = getCurrentMnc(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(mobileNetworkCodeChanged(const QString&)), this, SLOT(onCurrentMncChanged(const QString&)));
        }
        watchCurrentMncs = true;
    } else if (strcmp(signal, SIGNAL(cellIdChanged(int,QString))) == 0
               && !watchCellIds) {
        cellIds.clear();
        foreach (const int interfaceIndex, interfaceIndexes) {
            cellIds[interfaceIndex] = getCellId(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(cellIdChanged(uint)), this, SLOT(onCellIdChanged(uint)));
        }
        watchCellIds = true;
    } else if (strcmp(signal, SIGNAL(locationAreaCodeChanged(int,QString))) == 0
               && !watchLacs) {
        lacs.clear();
        foreach (const int interfaceIndex, interfaceIndexes) {
            lacs[interfaceIndex] = getLac(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(locationAreaCodeChanged(uint)), this, SLOT(onLocationAreaCodeChanged(uint)));
        }
        watchLacs = true;
    } else if (strcmp(signal, SIGNAL(networkNameChanged(QNetworkInfo::NetworkMode,int,QString))) == 0
               && !watchOperatorNames) {
        operatorNames.clear();
        foreach (const int interfaceIndex, interfaceIndexes) {
            // ensure networkMode slot is connected to technologyChanged signal so it is easier to pass current mode in networkNameChanged signal
            if (!watchNetworkModes) {
                networkModes[interfaceIndex] = getNetworkMode(interfaceIndex);
                connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(technologyChanged(const QString&)), this, SLOT(onNetworkModeChanged(QString)));
            }
            operatorNames[interfaceIndex] = getOperatorName(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(providerNameChanged(const QString&)), this, SLOT(onOperatorNameChanged(const QString&)));
        }
        watchNetworkModes = true;
        watchOperatorNames = true;
    }
}


void QNetworkServiceWrapper::disconnectNotify(const char *signal)
{
    QList<int> interfaceIndexes;
    interfaceIndexes = allNetworkManagerInterfaces.keys();

    if (strcmp(signal, SIGNAL(networkInterfaceCountChanged(QNetworkInfo::NetworkMode,int))) == 0
            && watchInterfaceCount) {
        watchInterfaceCount = false;
    } else if (strcmp(signal, SIGNAL(networkSignalStrengthChanged(QNetworkInfo::NetworkMode,int))) == 0
               && watchSignalStrengths) {
        foreach (const int interfaceIndex, interfaceIndexes)
            disconnect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(signalBarsChanged(int)), this, SLOT(onSignalStrengthChanged(int)));
        watchSignalStrengths = false;
    } else if (strcmp(signal, SIGNAL(networkStatusChanged(QNetworkInfo::NetworkMode,int,QNetworkInfo::NetworkStatus))) == 0
               && watchStatuses) {
        foreach (const int interfaceIndex, interfaceIndexes)
            disconnect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(registrationStatusChanged(const QString&)), this, SLOT(onNetworkStatusChanged(const QString&)));
        watchStatuses = false;
    } else if (strcmp(signal, SIGNAL(currentMobileCountryCodeChanged(int,QString))) == 0
               && watchCurrentMccs) {
        foreach (const int interfaceIndex, interfaceIndexes)
            disconnect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(mobileCountryCodeChanged(const QString&)), this, SLOT(onCurrentMccChanged(const QString&)));
        watchCurrentMccs = false;
    } else if (strcmp(signal, SIGNAL(currentMobileNetworkCodeChanged(int,QString))) == 0
               && watchCurrentMncs) {
        foreach (const int interfaceIndex, interfaceIndexes)
            disconnect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(mobileNetworkCodeChanged(const QString&)), this, SLOT(onCurrentMncChanged(const QString&)));
        watchCurrentMncs = false;
    } else if (strcmp(signal, SIGNAL(cellIdChanged(int,QString))) == 0
               && watchCellIds) {
        foreach (const int interfaceIndex, interfaceIndexes)
            disconnect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(cellIdChanged(uint)), this, SLOT(onCellIdChanged(uint)));
        watchCellIds = false;
    } else if (strcmp(signal, SIGNAL(locationAreaCodeChanged(int,QString))) == 0
               && watchLacs) {
        foreach (const int interfaceIndex, interfaceIndexes)
            disconnect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(locationAreaCodeChanged(uint)), this, SLOT(onLocationAreaCodeChanged(uint)));
        watchLacs = false;
    } else if (strcmp(signal, SIGNAL(networkNameChanged(QNetworkInfo::NetworkMode,int,QString))) == 0
               && watchOperatorNames) {
        foreach (const int interfaceIndex, interfaceIndexes)
            disconnect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(providerNameChanged(const QString&)), this, SLOT(onOperatorNameChanged(const QString&)));
        watchOperatorNames = false;
    } else if (strcmp(signal, SIGNAL(currentCellDataTechnologyChanged(int,QNetworkInfo::CellDataTechnology))) == 0
               && watchTechnologies) {
        foreach (const int interfaceIndex, interfaceIndexes)
            disconnect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(technologyChanged(const QString&)), this, SLOT(onTechnologyChanged(const QString&)));
        watchTechnologies = false;
    }

    if (watchNetworkModes && !watchSignalStrengths && !watchStatuses && !watchOperatorNames) {
        foreach (const int interfaceIndex, interfaceIndexes)
            disconnect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(technologyChanged(const QString&)), this, SLOT(onNetworkModeChanged(QString&)));
        watchNetworkModes = false;
    }
}

void QNetworkServiceWrapper::onServiceAdded(const QString &serviceName, QService::Scope scope)
{
    Q_UNUSED(scope)

    int interfaceIndex = -1;
    QServiceFilter filter;
    filter.setServiceName(serviceName);
    filter.setInterface(*NETWORK_REGISTRATION_MANAGER());
    QList<QServiceInterfaceDescriptor> serviceInterfaces = serviceManager->findInterfaces(filter);
    if (!serviceInterfaces.isEmpty()) {
        QList<int> interfaceIndexes = allNetworkManagerInterfaces.keys();
        if (!interfaceIndexes.isEmpty()) {
            interfaceIndex = interfaceIndexes.last() + 1;
            allNetworkManagerInterfaces.insert(interfaceIndex, serviceInterfaces[0]);
        }
    }

    if (interfaceIndex > -1) {
        if (watchNetworkModes) {
            networkModes[interfaceIndex] = getNetworkMode(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(technologyChanged(const QString&)), this, SLOT(onNetworkModeChanged(QString&)));
        }
        if (watchSignalStrengths) {
            signalStrengths[interfaceIndex] = getSignalStrength(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(signalBarsChanged(int)), this, SLOT(onSignalStrengthChanged(int)));
        }
        if (watchTechnologies) {
            currentCellDataTechnologies[interfaceIndex] = getCurrentCellDataTechnology(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(technologyChanged(const QString&)), this, SLOT(onTechnologyChanged(const QString&)));
        }
        if (watchCellIds) {
            cellIds[interfaceIndex] = getCellId(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(locationAreaCodeChanged(uint)), this, SLOT(onLocationAreaCodeChanged(uint)));
        }
        if (watchStatuses) {
            networkStatuses[interfaceIndex] = getNetworkStatus(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(registrationStatusChanged(const QString&)), this, SLOT(onNetworkStatusChanged(const QString&)));
        }
        if (watchCurrentMccs) {
            currentMccs[interfaceIndex] = getCurrentMcc(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(mobileCountryCodeChanged(const QString&)), this, SLOT(onCurrentMccChanged(const QString&)));
        }
        if (watchCurrentMncs) {
            currentMncs[interfaceIndex] = getCurrentMnc(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(mobileNetworkCodeChanged(const QString&)), this, SLOT(onCurrentMncChanged(const QString&)));
        }
        if (watchLacs) {
            lacs[interfaceIndex] = getLac(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(locationAreaCodeChanged(uint)), this, SLOT(onLocationAreaCodeChanged(uint)));
        }
        if (watchOperatorNames) {
            operatorNames[interfaceIndex] = getOperatorName(interfaceIndex);
            connect(loadedNetworkManagerInterfaces.value(interfaceIndex), SIGNAL(providerNameChanged(const QString&)), this, SLOT(onOperatorNameChanged(const QString&)));
        }
    }

    if (watchInterfaceCount) {
        int interfaceCount = allNetworkManagerInterfaces.size();
        emit networkInterfaceCountChanged(QNetworkInfo::GsmMode, interfaceCount);
        emit networkInterfaceCountChanged(QNetworkInfo::TdscdmaMode, interfaceCount);
        emit networkInterfaceCountChanged(QNetworkInfo::CdmaMode, interfaceCount);
        emit networkInterfaceCountChanged(QNetworkInfo::WcdmaMode, interfaceCount);
        emit networkInterfaceCountChanged(QNetworkInfo::LteMode, interfaceCount);
    }
}

void QNetworkServiceWrapper::onServiceRemoved(const QString &serviceName, QService::Scope scope)
{
    Q_UNUSED(scope)

    int interfaceIndex = -1;
    QList<int> interfaceIndexes = allNetworkManagerInterfaces.keys();
    foreach (const int interface, interfaceIndexes) {
        if (serviceName == allNetworkManagerInterfaces.value(interface).serviceName()) {
            interfaceIndex = interface;
            break;
        }
    }

    if (interfaceIndex > -1) {
        loadedNetworkManagerInterfaces.remove(interfaceIndex);
        allNetworkManagerInterfaces.remove(interfaceIndex);
    }

    if (watchInterfaceCount) {
        int interfaceCount =  allNetworkManagerInterfaces.count();
        emit networkInterfaceCountChanged(QNetworkInfo::GsmMode, interfaceCount);
        emit networkInterfaceCountChanged(QNetworkInfo::TdscdmaMode, interfaceCount);
        emit networkInterfaceCountChanged(QNetworkInfo::CdmaMode, interfaceCount);
        emit networkInterfaceCountChanged(QNetworkInfo::WcdmaMode, interfaceCount);
        emit networkInterfaceCountChanged(QNetworkInfo::LteMode, interfaceCount);
    }
}

void QNetworkServiceWrapper::onSignalStrengthChanged(const int strength)
{
    int interfaceIndex = -1;
    QObject *signalSender = sender();
    if (signalSender)
        interfaceIndex = loadedNetworkManagerInterfaces.key(signalSender, -1);
    if (interfaceIndex > -1)
        emit (networkSignalStrengthChanged(networkModes[interfaceIndex], interfaceIndex, strength * 5));
}

void QNetworkServiceWrapper::onTechnologyChanged(const QString &technology)
{
    int interfaceIndex = -1;
    QObject *signalSender = sender();
    if (signalSender)
        interfaceIndex = loadedNetworkManagerInterfaces.key(signalSender, -1);
    if (interfaceIndex > -1)
        emit (currentCellDataTechnologyChanged(interfaceIndex, technologyStringToEnum(technology)));
}

void QNetworkServiceWrapper::onNetworkStatusChanged(const QString &status)
{
    int interfaceIndex = -1;
    QObject *signalSender = sender();
    if (signalSender)
        interfaceIndex = loadedNetworkManagerInterfaces.key(signalSender, -1);
    if (interfaceIndex > -1)
        emit (networkStatusChanged(networkModes[interfaceIndex], interfaceIndex, statusStringToEnum(status)));
}

void QNetworkServiceWrapper::onCellIdChanged(const uint cellid)
{
    int interfaceIndex = -1;
    QObject *signalSender = sender();
    if (signalSender)
        interfaceIndex = loadedNetworkManagerInterfaces.key(signalSender, -1);
    if (interfaceIndex > -1)
        emit (cellIdChanged(interfaceIndex, QString::number(cellid)));
}

void QNetworkServiceWrapper::onCurrentMccChanged(const QString &currentMcc)
{
    int interfaceIndex = -1;
    QObject *signalSender = sender();
    if (signalSender)
        interfaceIndex = loadedNetworkManagerInterfaces.key(signalSender, -1);
    if (interfaceIndex > -1)
        emit (currentMobileCountryCodeChanged(interfaceIndex, currentMcc));
}

void QNetworkServiceWrapper::onCurrentMncChanged(const QString &currentMnc)
{
    int interfaceIndex = -1;
    QObject *signalSender = sender();
    if (signalSender)
        interfaceIndex = loadedNetworkManagerInterfaces.key(signalSender, -1);
    if (interfaceIndex > -1)
        emit (currentMobileNetworkCodeChanged(interfaceIndex, currentMnc));
}

void QNetworkServiceWrapper::onLocationAreaCodeChanged(const uint lac)
{
    int interfaceIndex = -1;
    QObject *signalSender = sender();
    if (signalSender)
        interfaceIndex = loadedNetworkManagerInterfaces.key(signalSender, -1);
    if (interfaceIndex > -1)
        emit (locationAreaCodeChanged(interfaceIndex, QString::number(lac)));
}

void QNetworkServiceWrapper::onOperatorNameChanged(const QString &name)
{
    int interfaceIndex = -1;
    QObject *signalSender = sender();
    if (signalSender)
        interfaceIndex = loadedNetworkManagerInterfaces.key(signalSender, -1);
    if (interfaceIndex > -1)
        emit (networkNameChanged(networkModes[interfaceIndex], interfaceIndex, name));
}

void QNetworkServiceWrapper::onNetworkModeChanged(const QString &technology)
{
    int interfaceIndex = -1;
    QObject *signalSender = sender();
    if (signalSender)
        interfaceIndex = loadedNetworkManagerInterfaces.key(signalSender, -1);
    if (interfaceIndex > -1)
        networkModes[interfaceIndex] = technologyToMode(technology);
}

QNetworkInfo::CellDataTechnology QNetworkServiceWrapper::technologyStringToEnum(const QString &technology)
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

QNetworkInfo::NetworkMode QNetworkServiceWrapper::technologyToMode(const QString &technology)
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

QNetworkInfo::NetworkStatus QNetworkServiceWrapper::statusStringToEnum(const QString &status)
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

QT_END_NAMESPACE

#endif // QT_NO_SFW_NETREG
