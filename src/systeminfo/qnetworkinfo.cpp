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

#include "qnetworkinfo.h"

#if defined(Q_OS_LINUX)
#  include "qnetworkinfo_linux_p.h"
#else
QT_BEGIN_NAMESPACE
class QNetworkInfoPrivate
{
public:
    QNetworkInfoPrivate(QNetworkInfo *) {}

    int networkSignalStrength(QNetworkInfo::NetworkMode) { return -1; }
    QNetworkInfo::CellDataTechnology currentCellDataTechnology(int) { return QNetworkInfo::UnknownDataTechnology; }
    QNetworkInfo::NetworkMode currentNetworkMode() { return QNetworkInfo::UnknownMode; }
    QNetworkInfo::NetworkStatus networkStatus(QNetworkInfo::NetworkMode) { return QNetworkInfo::UnknownStatus; }
    QNetworkInterface interfaceForMode(QNetworkInfo::NetworkMode) { return QNetworkInterface(); }
    QString cellId(int) { return QString(); }
    QString currentMobileCountryCode(int) { return QString(); }
    QString currentMobileNetworkCode(int) { return QString(); }
    QString homeMobileCountryCode(int) { return QString(); }
    QString homeMobileNetworkCode(int) { return QString(); }
    QString imsi(int) { return QString(); }
    QString locationAreaCode(int) { return QString(); }
    QString macAddress(QNetworkInfo::NetworkMode) { return QString(); }
    QString networkName(QNetworkInfo::NetworkMode) { return QString(); }
};
QT_END_NAMESPACE
#endif

QT_BEGIN_NAMESPACE

/*!
    \class QNetworkInfo
    \inmodule QtSystemKit
    \brief The QNetworkInfo class provides various information of the network status.
*/

/*!
    \enum QNetworkInfo::CellDataTechnology
    This enum describes the type of cellular technology.

    \value UnknownDataTechnology   The cellular technology is unknown or on error.
    \value GprsDataTechnology      General Packet Radio Service (GPRS) data service.
    \value EdgeDataTechnology      Enhanced Data Rates for GSM Evolution (EDGE) data service.
    \value UmtsDataTechnology      Universal Mobile Telecommunications System (UMTS) data service.
    \value HspaDataTechnology      High Speed Packet Access (HSPA) data service.
*/

/*!
    \enum QNetworkInfo::NetworkMode
    This enumeration describes the type of the network.

    \value UnknownMode     The network is unknown or on error.
    \value GsmMode         Global System for Mobile (GSM) network.
    \value CdmaMode        Code Division Multiple Access (CDMA) network.
    \value WcdmaMode       Wideband Code Division Multiple Access (WCDMA) network.
    \value WlanMode        Wireless local area network (WLAN) network.
    \value EthernetMode    Local area network (LAN), or Ethernet network.
    \value BluetoothMode   Bluetooth network.
    \value WimaxMode       Worldwide Interoperability for Microwave Access (WiMAX) network.
    \value LteMode         3GPP Long Term Evolution (LTE) network.
*/

/*!
    \enum QNetworkInfo::NetworkStatus
    This enumeration describes the status of the network.

    \value UnknownStatus        The status is unknown or on error.
    \value NoNetworkAvailable   There is no network available.
    \value EmergencyOnly        The network only allows emergency calls.
    \value Searching            The device is searching or connecting to the network.
    \value Busy                 The network is too busy to be connected.
    \value Connected            The device has successfully connected to the network.
    \value Denied               The connection to the network has been denied.
    \value HomeNetwork          The device is on home network. It suggests the Connected status.
    \value Roaming              The device is on roaming network. It suggests the Connected status.
*/

/*!
    \fn void QNetworkInfo::cellIdChanged(int sim, const QString &id)

    This signal is emitted whenever the cell ID for \a sim has changed to \a id.
*/

/*!
    \fn void QNeworkInfo::currentCellDataTechnologyChanged(int sim, QNetworkInfo::CellDataTechnology tech)

    This signal is emitted whenever the current cell data technology for \a sim has changed to \a tech.
*/

/*!
    \fn void QNetworkInfo::currentMobileCountryCodeChanged(int sim, const QString &mcc)

    This signal is emitted whenever the current Mobile Country Code (MCC) for \a sim has changed
    to \a mcc.
*/

/*!
    \fn void QNetworkInfo::currentMobileNetworkCodeChanged(int sim, const QString &mnc)

    This signal is emitted whenever the current Mobile Network Code (MNC) for \a sim has changed
    to \a mnc.
*/

/*!
    \fn void QNetworkInfo::currentNetworkMode(QNetworkInfo::NetworkMode mode)

    This signal is emitted whenever the current network has changed to \a mode.
*/

/*!
    \fn void QNetworkInfo::locationAreaCodeChanged(int sim, const QString &lac)

    This signal is emitted whenever the location area code for \a sim has changed to \a lac.
*/

/*!
    \fn void QNetworkInfo::networkNameChanged(QNetworkInfo::NetworkMode mode, const QString &name)

    This signal is emitted whenever the name for \a mode has changed to \a name.
*/

/*!
    \fn void QNetworkInfo::networkSignalStrengthChanged(QNetworkInfo::NetworkMode mode, int strength)

    This signal is emitted whenever the signal strength for \a mode has changed to \a strength.
*/

/*!
    \fn void QNetworkInfo::networkStatusChanged(QNetworkInfo::NetworkMode mode, QNetworkInfo::NetworkStatus status)

    This signal is emitted whenever the status for \a mode has changed to \a status.
*/

/*!
    Constructs a QNetworkInfo object with the given \a parent.
*/
QNetworkInfo::QNetworkInfo(QObject *parent)
    : QObject(parent)
    , d_ptr(new QNetworkInfoPrivate(this))
{
}

/*!
    Destroys the object
*/
QNetworkInfo::~QNetworkInfo()
{
    delete d_ptr;
}

/*!
    Returns the signal strength for \a mode, in 0 - 100 scale. If the information is not available,
    or error occurs, -1 is returned.
*/
int QNetworkInfo::networkSignalStrength(QNetworkInfo::NetworkMode mode) const
{
    return d_ptr->networkSignalStrength(mode);
}

/*!
    Returns the current cell data technology used for \a sim.
*/
QNetworkInfo::CellDataTechnology QNetworkInfo::currentCellDataTechnology(int sim) const
{
    return d_ptr->currentCellDataTechnology(sim);
}

/*!
    \property QNetworkInfo::currentNetworkMode

    Returns the current active network mode. If there are more than one modes activated, the preferred
    one is returned.
*/
QNetworkInfo::NetworkMode QNetworkInfo::currentNetworkMode() const
{
    return d_ptr->currentNetworkMode();
}

/*!
    Returns the current status for \a mode.
*/
QNetworkInfo::NetworkStatus QNetworkInfo::networkStatus(QNetworkInfo::NetworkMode mode) const
{
    return d_ptr->networkStatus(mode);
}

/*!
    Returns the first found interface for \a mode. If none is found, or it can't be represented
    by QNetworkInterface (e.g. Bluetooth), and empty object is returned.
*/
QNetworkInterface QNetworkInfo::interfaceForMode(QNetworkInfo::NetworkMode mode) const
{
    return d_ptr->interfaceForMode(mode);
}

/*!
    Returns the cell ID of the connected tower or based station for \a sim. If this information
    is not available or error occurs, an empty string is returned.
*/
QString QNetworkInfo::cellId(int sim) const
{
    return d_ptr->cellId(sim);
}

/*!
    Returns the current Mobile Country Code (MCC) for \a sim. An empty string is returned if the
    information is not available or on error.
*/
QString QNetworkInfo::currentMobileCountryCode(int sim) const
{
    return d_ptr->currentMobileCountryCode(sim);
}

/*!
    Returns the current Mobile Network Code (MNC) for \a sim. An empty string is returned if the
    information is not available or on error.
*/
QString QNetworkInfo::currentMobileNetworkCode(int sim) const
{
    return d_ptr->currentMobileNetworkCode(sim);
}

/*!
    Returns the home Mobile Country Code (MCC) for \a sim. An empty string is returned if the
    information is not available or on error.
*/
QString QNetworkInfo::homeMobileCountryCode(int sim) const
{
    return d_ptr->homeMobileCountryCode(sim);
}

/*!
    Returns the home Mobile Network Code (MNC) for \a sim. An empty string is returned if the
    information is not available or on error.
*/
QString QNetworkInfo::homeMobileNetworkCode(int sim) const
{
    return d_ptr->homeMobileNetworkCode(sim);
}

/*!
    Returns the International Mobile Subscriber Identity (IMSI) for \a sim. If this information is
    not available, or error occurs, an empty string is returned.
*/
QString QNetworkInfo::imsi(int sim) const
{
    return d_ptr->imsi(sim);
}

/*!
    Returns the location area code of the current cellular radio network for \a sim. If this information
    is not available or error occurs, an empty string is returned.
*/
QString QNetworkInfo::locationAreaCode(int sim) const
{
    return d_ptr->locationAreaCode(sim);
}

/*!
    Returns the MAC address for \a mode. If the MAC address is not available or error occurs, an
    empty string is returned.
*/
QString QNetworkInfo::macAddress(QNetworkInfo::NetworkMode mode) const
{
    return d_ptr->macAddress(mode);
}

/*!
    Returns the name of the operator for \a mode. If the information is not available, or an error
    occurs, an empty string is returned.

    In case of WLAN, the SSID is returned; for Ethernet, the domain name is returned if available.
*/
QString QNetworkInfo::networkName(QNetworkInfo::NetworkMode mode) const
{
    return d_ptr->networkName(mode);
}

QT_END_NAMESPACE
