/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarativenetworkinfo_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype NetworkInfo
    \instantiates QDeclarativeNetworkInfo
    \inqmlmodule QtSystemInfo
    \ingroup qml-systeminfo
    \brief The NetworkInfo element provides various information about the network status.
 */

/*!
    \internal
 */
QDeclarativeNetworkInfo::QDeclarativeNetworkInfo(QObject *parent)
    : QObject(parent)
    , networkInfo(new QNetworkInfo(this))
    , isMonitorCurrentNetworkMode(false)
    , isMonitorNetworkSignalStrength(false)
    , isMonitorNetworkInterfaceCount(false)
    , isMonitorCurrentCellDataTechnology(false)
    , isMonitorNetworkStatus(false)
    , isMonitorCellId(false)
    , isMonitorCurrentMobileCountryCode(false)
    , isMonitorCurrentMobileNetworkCode(false)
    , isMonitorLocationAreaCode(false)
    , isMonitorNetworkName(false)
{
    connect(networkInfo, SIGNAL(networkInterfaceCountChanged(QNetworkInfo::NetworkMode,int)),
            this, SLOT(_q_networkInterfaceCountChanged(QNetworkInfo::NetworkMode,int)));
    connect(networkInfo, SIGNAL(currentCellDataTechnologyChanged(int,QNetworkInfo::CellDataTechnology)),
            this, SLOT(_q_currentCellDataTechnologyChanged(int,QNetworkInfo::CellDataTechnology)));
    connect(networkInfo, SIGNAL(cellIdChanged(int,QString)), this, SIGNAL(cellIdChanged(int,QString)));
    connect(networkInfo, SIGNAL(currentMobileCountryCodeChanged(int,QString)),
            this, SIGNAL(currentMobileCountryCodeChanged(int,QString)));
    connect(networkInfo, SIGNAL(currentMobileNetworkCodeChanged(int,QString)),
            this, SIGNAL(currentMobileNetworkCodeChanged(int,QString)));
    connect(networkInfo, SIGNAL(locationAreaCodeChanged(int,QString)),
            this, SIGNAL(locationAreaCodeChanged(int,QString)));
}

/*!
    \internal
 */
QDeclarativeNetworkInfo::~QDeclarativeNetworkInfo()
{
}

/*!
    \qmlproperty bool NetworkInfo::monitorCurrentNetworkMode

    This property holds whether or not monitor the change of current network mode.
 */
bool QDeclarativeNetworkInfo::monitorCurrentNetworkMode() const
{
    return isMonitorCurrentNetworkMode;
}

void QDeclarativeNetworkInfo::setMonitorCurrentNetworkMode(bool monitor)
{
    if (monitor != isMonitorCurrentNetworkMode) {
        isMonitorCurrentNetworkMode = monitor;
        if (monitor) {
            connect(networkInfo, SIGNAL(currentNetworkModeChanged(QNetworkInfo::NetworkMode)),
                    this, SIGNAL(currentNetworkModeChanged()));
        } else {
            disconnect(networkInfo, SIGNAL(currentNetworkModeChanged(QNetworkInfo::NetworkMode)),
                       this, SIGNAL(currentNetworkModeChanged()));
        }
        emit monitorCurrentNetworkModeChanged();
    }
}

/*!
    \qmlproperty enumeration NetworkInfo::currentNetworkMode

    This property holds the current network mode. Possible values are:
    \list
    \li NetworkInfo.UnknownMode     - The network is unknown or an error occured.
    \li NetworkInfo.GsmMode         - Global System for Mobile (GSM) network.
    \li NetworkInfo.CdmaMode        - Code Division Multiple Access (CDMA) network.
    \li NetworkInfo.WcdmaMode       - Wideband Code Division Multiple Access (WCDMA) network.
    \li NetworkInfo.WlanMode        - Wireless local area network (WLAN) network.
    \li NetworkInfo.EthernetMode    - Local area network (LAN), or Ethernet network.
    \li NetworkInfo.BluetoothMode   - Bluetooth network.
    \li NetworkInfo.WimaxMode       - Worldwide Interoperability for Microwave Access (WiMAX) network.
    \li NetworkInfo.LteMode         - 3GPP Long Term Evolution (LTE) network.
    \li NetworkInfo.TdscdmaMode     - Time Division Synchronous Code Division Multiple Access (TD-SCDMA) network.
    \endlist
 */
QDeclarativeNetworkInfo::NetworkMode QDeclarativeNetworkInfo::currentNetworkMode() const
{
    return static_cast<QDeclarativeNetworkInfo::NetworkMode>(networkInfo->currentNetworkMode());
}

/*!
    \qmlproperty bool NetworkInfo::monitorNetworkSignalStrength

    This property holds whether or not monitor the change of network signal strength.
 */
bool QDeclarativeNetworkInfo::monitorNetworkSignalStrength() const
{
    return isMonitorNetworkSignalStrength;
}

void QDeclarativeNetworkInfo::setMonitorNetworkSignalStrength(bool monitor)
{
    if (monitor != isMonitorNetworkSignalStrength) {
        isMonitorNetworkSignalStrength = monitor;
        if (monitor) {
            connect(networkInfo, SIGNAL(networkSignalStrengthChanged(QNetworkInfo::NetworkMode,int,int)),
                    this, SLOT(_q_networkSignalStrengthChanged(QNetworkInfo::NetworkMode,int,int)));
        } else {
            disconnect(networkInfo, SIGNAL(networkSignalStrengthChanged(QNetworkInfo::NetworkMode,int,int)),
                       this, SLOT(_q_networkSignalStrengthChanged(QNetworkInfo::NetworkMode,int,int)));
        }
        emit monitorNetworkSignalStrengthChanged();
    }
}

/*!
    \qmlmethod int NetworkInfo::networkSignalStrength(NetworkMode mode, int interface)

    Returns the signal strength of the given \a mode and \a interface. If the information
    is not available, or error occurs, -1 is returned.
 */
int QDeclarativeNetworkInfo::networkSignalStrength(NetworkMode mode, int interface) const
{
    return networkInfo->networkSignalStrength(static_cast<QNetworkInfo::NetworkMode>(mode), interface);
}

/*!
    \qmlsignal NetworkInfo::onNetworkSignalStrengthChanged(NetworkMode mode, int interfaceIndex, int strength)

    This handler is called whenever the signal strength for the \a interfaceIndex of \a mode has changed
    to \a strength. Note that it won't be called until monitorNetworkSignalStrength is set true.

    \sa networkSignalStrength, monitorNetworkSignalStrength
 */
void QDeclarativeNetworkInfo::_q_networkSignalStrengthChanged(QNetworkInfo::NetworkMode mode, int interface, int strength)
{
    emit networkSignalStrengthChanged(static_cast<NetworkMode>(mode), interface, strength);
}

/*!
    \qmlproperty bool NetworkInfo::monitorNetworkInterfaceCount

    This property is obsoleted, and will be removed soon. You don't need to use it at all.
 */
bool QDeclarativeNetworkInfo::monitorNetworkInterfaceCount() const
{
    return isMonitorNetworkInterfaceCount;
}

void QDeclarativeNetworkInfo::setMonitorNetworkInterfaceCount(bool monitor)
{
    if (monitor != isMonitorNetworkInterfaceCount) {
        isMonitorNetworkInterfaceCount = monitor;
        if (monitor) {
            connect(networkInfo, SIGNAL(networkInterfaceCountChanged(QNetworkInfo::NetworkMode,int)),
                    this, SLOT(_q_networkInterfaceCountChanged(QNetworkInfo::NetworkMode,int)));
        } else {
            disconnect(networkInfo, SIGNAL(networkInterfaceCountChanged(QNetworkInfo::NetworkMode,int)),
                       this, SLOT(_q_networkInterfaceCountChanged(QNetworkInfo::NetworkMode,int)));
        }
        emit monitorNetworkInterfaceCountChanged();
    }
}

/*!
    \qmlmethod int NetworkInfo::networkInterfaceCount(NetworkMode mode)

    Returns the interface count of the given \a mode.
 */
int QDeclarativeNetworkInfo::networkInterfaceCount(NetworkMode mode) const
{
    return networkInfo->networkInterfaceCount(static_cast<QNetworkInfo::NetworkMode>(mode));
}

/*!
    \qmlsignal NetworkInfo::onNetworkInterfaceCountChanged(NetworkMode mode, int count)

    This handler is called whenever the number of interfaces of \a mode has changed to \a count.
    Note that it won't called until monitorNetworkInterfaceCount is set true.

    \sa networkInterfaceCount, monitorNetworkInterfaceCount
 */
void QDeclarativeNetworkInfo::_q_networkInterfaceCountChanged(QNetworkInfo::NetworkMode mode, int count)
{
    emit networkInterfaceCountChanged(static_cast<NetworkMode>(mode), count);
}

/*!
    \qmlproperty bool NetworkInfo::monitorCurrentCellDataTechnology

    This property is obsoleted, and will be removed soon. You don't need to use it at all.
 */
bool QDeclarativeNetworkInfo::monitorCurrentCellDataTechnology() const
{
    return isMonitorCurrentCellDataTechnology;
}

void QDeclarativeNetworkInfo::setMonitorCurrentCellDataTechnology(bool monitor)
{
    if (monitor != isMonitorCurrentCellDataTechnology) {
        isMonitorCurrentCellDataTechnology = monitor;
        if (monitor) {
            connect(networkInfo, SIGNAL(currentCellDataTechnologyChanged(int,QNetworkInfo::CellDataTechnology)),
                    this, SLOT(_q_currentCellDataTechnologyChanged(int,QNetworkInfo::CellDataTechnology)));
        } else {
            disconnect(networkInfo, SIGNAL(currentCellDataTechnologyChanged(int,QNetworkInfo::CellDataTechnology)),
                       this, SLOT(_q_currentCellDataTechnologyChanged(int,QNetworkInfo::CellDataTechnology)));
        }
        emit monitorCurrentCellDataTechnologyChanged();
    }
}

/*!
    \qmlmethod CellDataTechnology NetworkInfo::currentCellDataTechnology(int interface)

    Returns the current cell data technology of the given \a interface. Possible values are:
    \list
    \li NetworkInfo.UnknownDataTechnology   - The cellular technology is unknown or an error occured.
    \li NetworkInfo.GprsDataTechnology      - General Packet Radio Service (GPRS) data service.
    \li NetworkInfo.EdgeDataTechnology      - Enhanced Data Rates for GSM Evolution (EDGE) data service.
    \li NetworkInfo.UmtsDataTechnology      - Universal Mobile Telecommunications System (UMTS) data service.
    \li NetworkInfo.HspaDataTechnology      - High Speed Packet Access (HSPA) data service.
    \endlist
 */
int QDeclarativeNetworkInfo::currentCellDataTechnology(int interface) const
{
    return networkInfo->currentCellDataTechnology(interface);
}

/*!
    \qmlsignal NetworkInfo::onCurrentCellDataTechnologyChanged(int interfaceIndex, CellDataTechnology tech)

    This handler is called whenever the cell data technology of \a interfaceIndex has been changed to \a tech.
    Note that the signal won't emit until monitorCurrentCellDataTechnology is set true.

    \sa currentCellDataTechnology, monitorCurrentCellDataTechnology
 */
void QDeclarativeNetworkInfo::_q_currentCellDataTechnologyChanged(int interface, QNetworkInfo::CellDataTechnology tech)
{
    emit currentCellDataTechnologyChanged(interface, static_cast<CellDataTechnology>(tech));
}

/*!
    \qmlproperty bool NetworkInfo::monitorNetworkStatus

    This property holds whether or not monitor the network status.
 */
bool QDeclarativeNetworkInfo::monitorNetworkStatus() const
{
    return isMonitorNetworkStatus;
}

void QDeclarativeNetworkInfo::setMonitorNetworkStatus(bool monitor)
{
    if (monitor != isMonitorNetworkStatus) {
        isMonitorNetworkStatus = monitor;
        if (monitor) {
            connect(networkInfo, SIGNAL(networkStatusChanged(QNetworkInfo::NetworkMode,int,QNetworkInfo::NetworkStatus)),
                    this, SLOT(_q_networkStatusChanged(QNetworkInfo::NetworkMode,int,QNetworkInfo::NetworkStatus)));
        } else {
            disconnect(networkInfo, SIGNAL(networkStatusChanged(QNetworkInfo::NetworkMode,int,QNetworkInfo::NetworkStatus)),
                       this, SLOT(_q_networkStatusChanged(QNetworkInfo::NetworkMode,int,QNetworkInfo::NetworkStatus)));
        }
        emit monitorNetworkStatusChanged();
    }
}

/*!
    \qmlmethod NetworkStatus NetworkInfo::networkStatus(NetworkMode mode, int interface)

    Returns the status of the given \a mode and \a interface. Possible values are:
    \list
    \li NetworkInfo.UnknownStatus        - The status is unknown or an error occured.
    \li NetworkInfo.NoNetworkAvailable   - There is no network available.
    \li NetworkInfo.EmergencyOnly        - The network only allows emergency calls.
    \li NetworkInfo.Searching            - The device is searching or connecting to the network.
    \li NetworkInfo.Busy                 - The network is too busy to be connected.
    \li NetworkInfo.Denied               - The connection to the network has been denied.
    \li NetworkInfo.HomeNetwork          - The device is connected to the home network.
    \li NetworkInfo.Roaming              - The device is connected to some roaming network.
    \endlist
 */
int QDeclarativeNetworkInfo::networkStatus(QDeclarativeNetworkInfo::NetworkMode mode, int interface) const
{
    return networkInfo->networkStatus(static_cast<QNetworkInfo::NetworkMode>(mode), interface);
}

/*!
    \qmlsignal NetworkInfo::onNetworkStatusChanged(NetworkMode mode, int interfaceIndex, NetworkStatus status)

    This handler is called whenever the status of \a mode and \a interfaceIndex has been changed to \a status.
    Note that it won't be called until monitorNetworkStatus is set true.

    \sa networkStatus, monitorNetworkStatus
 */
void QDeclarativeNetworkInfo::_q_networkStatusChanged(QNetworkInfo::NetworkMode mode, int interface, QNetworkInfo::NetworkStatus status)
{
    emit networkStatusChanged(static_cast<NetworkMode>(mode), interface, static_cast<NetworkStatus>(status));
}

/*!
    \qmlproperty bool NetworkInfo::monitorCellId

    This property is obsoleted, and will be removed soon. You don't need to use it at all.
 */
bool QDeclarativeNetworkInfo::monitorCellId() const
{
    return isMonitorCellId;
}

void QDeclarativeNetworkInfo::setMonitorCellId(bool monitor)
{
    if (monitor != isMonitorCellId) {
        isMonitorCellId = monitor;
        if (monitor) {
            connect(networkInfo, SIGNAL(cellIdChanged(int,QString)),
                    this, SIGNAL(cellIdChanged(int,QString)));
        } else {
            disconnect(networkInfo, SIGNAL(cellIdChanged(int,QString)),
                    this, SIGNAL(cellIdChanged(int,QString)));
        }
        emit monitorCellIdChanged();
    }
}

/*!
    \qmlmethod string NetworkInfo::cellId(int interface)

    Returns the cell ID of the given \a interface. If this information
    is not available or error occurs, an empty string is returned.

    \sa onCellIdChanged
 */
QString QDeclarativeNetworkInfo::cellId(int interface) const
{
    return networkInfo->cellId(interface);
}

/*!
    \qmlsignal NetworkInfo::onCellIdChanged(int interfaceIndex, string id)

    This handler is called whenever the cell ID of \a interfaceIndex has been changed to \a id.
    Note that it won't be called unless monitorCellId is set true.

    \sa cellId, monitorCellId
 */

/*!
    \qmlproperty bool NetworkInfo::monitorCurrentMobileCountryCode

    This property is obsoleted, and will be removed soon. You don't need to use it at all.
 */
bool QDeclarativeNetworkInfo::monitorCurrentMobileCountryCode() const
{
    return isMonitorCurrentMobileCountryCode;
}

void QDeclarativeNetworkInfo::setMonitorCurrentMobileCountryCode(bool monitor)
{
    if (monitor != isMonitorCurrentMobileCountryCode) {
        isMonitorCurrentMobileCountryCode = monitor;
        if (monitor) {
            connect(networkInfo, SIGNAL(currentMobileCountryCodeChanged(int,QString)),
                    this, SIGNAL(currentMobileCountryCodeChanged(int,QString)));
        } else {
            disconnect(networkInfo, SIGNAL(currentMobileCountryCodeChanged(int,QString)),
                       this, SIGNAL(currentMobileCountryCodeChanged(int,QString)));
        }
        emit monitorCurrentMobileCountryCodeChanged();
    }
}

/*!
    \qmlmethod string NetworkInfo::currentMobileCountryCode(int interface)

    Returns the current mobile country code of the given \a interface. If this information
    is not available or error occurs, an empty string is returned.
 */
QString QDeclarativeNetworkInfo::currentMobileCountryCode(int interface) const
{
    return networkInfo->currentMobileCountryCode(interface);
}

/*!
    \qmlsignal NetworkInfo::onCurrentMobileCountryCodeChanged(int interfaceIndex, string mcc)

    This handler is called whenever the current mobile country code of \a interfaceIndex has been changed
    to \a mcc. Note that it won't be called unless monitorCurrentMobileCountryCode is set true.

    \sa currentMobileCountryCode, monitorCurrentMobileCountryCode
 */

/*!
    \qmlproperty bool NetworkInfo::monitorCurrentMobileNetworkCode

    This property is obsoleted, and will be removed soon. You don't need to use it at all.
 */
bool QDeclarativeNetworkInfo::monitorCurrentMobileNetworkCode() const
{
    return isMonitorCurrentMobileNetworkCode;
}

void QDeclarativeNetworkInfo::setMonitorCurrentMobileNetworkCode(bool monitor)
{
    if (monitor != isMonitorCurrentMobileNetworkCode) {
        isMonitorCurrentMobileNetworkCode = monitor;
        if (monitor) {
            connect(networkInfo, SIGNAL(currentMobileNetworkCodeChanged(int,QString)),
                    this, SIGNAL(currentMobileNetworkCodeChanged(int,QString)));
        } else {
            disconnect(networkInfo, SIGNAL(currentMobileNetworkCodeChanged(int,QString)),
                       this, SIGNAL(currentMobileNetworkCodeChanged(int,QString)));
        }
        emit monitorCurrentMobileNetworkCodeChanged();
    }
}

/*!
    \qmlmethod string NetworkInfo::currentMobileNetworkCode(int interface)

    Returns the current mobile network code of the given \a interface. If this information
    is not available or error occurs, an empty string is returned.
 */
QString QDeclarativeNetworkInfo::currentMobileNetworkCode(int interface) const
{
    return networkInfo->currentMobileNetworkCode(interface);
}

/*!
    \qmlsignal NetworkInfo::onCurrentMobileNetworkCodeChanged(int interfaceIndex, string mnc)

    This handler is called whenever the current mobile network code of \a interfaceIndex has been changed
    to \a mnc. Note that it won't be called unless monitorCurrentMobileNetworkCode is set true.

    \sa currentMobileNetworkCode, monitorCurrentMobileNetworkCode
 */

/*!
    \qmlproperty bool NetworkInfo::monitorLocationAreaCode

    This property is obsoleted, and will be removed soon. You don't need to use it at all.
 */
bool QDeclarativeNetworkInfo::monitorLocationAreaCode() const
{
    return isMonitorLocationAreaCode;
}

void QDeclarativeNetworkInfo::setMonitorLocationAreaCode(bool monitor)
{
    if (monitor != isMonitorLocationAreaCode) {
        isMonitorLocationAreaCode = monitor;
        if (monitor) {
            connect(networkInfo, SIGNAL(locationAreaCodeChanged(int,QString)),
                    this, SIGNAL(locationAreaCodeChanged(int,QString)));
        } else {
            disconnect(networkInfo, SIGNAL(locationAreaCodeChanged(int,QString)),
                    this, SIGNAL(locationAreaCodeChanged(int,QString)));
        }
        emit monitorLocationAreaCodeChanged();
    }
}

/*!
    \qmlmethod string NetworkInfo::locationAreaCode(int interface)

    Returns the location area code of the given \a interface. If this information
    is not available or error occurs, an empty string is returned.
 */
QString QDeclarativeNetworkInfo::locationAreaCode(int interface) const
{
    return networkInfo->locationAreaCode(interface);
}

/*!
    \qmlsignal NetworkInfo::onLocationAreaCodeChanged(int interfaceIndex, string lac)

    This handler is called whenever the location area code of \a interfaceIndex has been changed to \a lac.
    Note that it won't be called unless monitorLocationAreaCode is set true.

    \sa locationAreaCode, monitorLocationAreaCode
 */

/*!
    \qmlproperty bool NetworkInfo::monitorNetworkName

    This property holds whether or not monitor the change of network names.
 */
bool QDeclarativeNetworkInfo::monitorNetworkName() const
{
    return isMonitorNetworkName;
}

void QDeclarativeNetworkInfo::setMonitorNetworkName(bool monitor)
{
    if (monitor != isMonitorNetworkName) {
        isMonitorNetworkName = monitor;
        if (monitor) {
            connect(networkInfo, SIGNAL(networkNameChanged(QNetworkInfo::NetworkMode,int,QString)),
                    this, SLOT(_q_networkNameChanged(QNetworkInfo::NetworkMode,int,QString)));
        } else {
            disconnect(networkInfo, SIGNAL(networkNameChanged(QNetworkInfo::NetworkMode,int,QString)),
                    this, SLOT(_q_networkNameChanged(QNetworkInfo::NetworkMode,int,QString)));
        }
        emit monitorNetworkNameChanged();
    }
}

/*!
    \qmlmethod string NetworkInfo::networkName(NetworkMode mode, int interface)

    Returns the name of the given \a mode and \a interface. If the information is not available,
    or an error occurs, an empty string is returned.

    In case of WLAN, the SSID is returned; for Ethernet, the domain name is returned if available.
 */
QString QDeclarativeNetworkInfo::networkName(NetworkMode mode, int interface) const
{
    return networkInfo->networkName(static_cast<QNetworkInfo::NetworkMode>(mode), interface);
}

/*!
    \qmlsignal NetworkInfo::onNetworkNameChanged(NetworkMode mode, int interfaceIndex, string name)

    This handler is called whenever the network name of \a mode and \a interfaceIndex has been changed
    to \a name. Note that it won't called until monitorNetworkName is set true.

    \sa networkName, monitorNetworkName
 */
void QDeclarativeNetworkInfo::_q_networkNameChanged(QNetworkInfo::NetworkMode mode, int interface, const QString &name)
{
    emit networkNameChanged(static_cast<NetworkMode>(mode), interface, name);
}

/*!
    \qmlmethod string NetworkInfo::macAddress(NetworkMode mode, int interface)

    Returns the MAC address for \a interface of \a mode. If the MAC address is not available or error
    occurs, an empty string is returned.
*/
QString QDeclarativeNetworkInfo::macAddress(QDeclarativeNetworkInfo::NetworkMode mode, int interface) const
{
    return networkInfo->macAddress(static_cast<QNetworkInfo::NetworkMode>(mode), interface);
}

/*!
    \qmlmethod string NetworkInfo::homeMobileCountryCode(int interface)

    Returns the home Mobile Country Code (MCC) for \a interface. An empty string is returned if the
    information is not available or on error.
*/
QString QDeclarativeNetworkInfo::homeMobileCountryCode(int interface) const
{
    return networkInfo->homeMobileCountryCode(interface);
}

/*!
    \qmlmethod string NetworkInfo::homeMobileNetworkCode(int interface)

    Returns the home Mobile Network Code (MNC) for \a interface. An empty string is returned if the
    information is not available or on error.
*/
QString QDeclarativeNetworkInfo::homeMobileNetworkCode(int interface) const
{
    return networkInfo->homeMobileNetworkCode(interface);
}

/*!
    \qmlmethod string NetworkInfo::imsi(int interface)

    Returns the International Mobile Subscriber Identity (IMSI) for \a interface. If this information is
    not available, or error occurs, an empty string is returned.
*/
QString QDeclarativeNetworkInfo::imsi(int interface) const
{
    return networkInfo->imsi(interface);
}

QT_END_NAMESPACE
