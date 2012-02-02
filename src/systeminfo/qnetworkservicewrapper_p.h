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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QNETWORKSERVICEWRAPPER_P_H
#define QNETWORKSERVICEWRAPPER_P_H

#include <qnetworkinfo.h>

#include <QObject>
#include <QStringList>
#include <QServiceInterfaceDescriptor>

#if !defined(QT_NO_SFW_NETREG)

QT_BEGIN_NAMESPACE

class QServiceManager;
class QServiceFilter;

class QNetworkServiceWrapper : public QObject
{
    Q_OBJECT

public:
    QNetworkServiceWrapper(QObject *parent = 0);

    int getNetworkInterfaceCount();
    int getSignalStrength(int interfaceIndex);
    QNetworkInfo::CellDataTechnology getCurrentCellDataTechnology(int interfaceIndex);
    QNetworkInfo::NetworkStatus getNetworkStatus(int interfaceIndex);
    QString getCellId(int interfaceIndex);
    QString getCurrentMcc(int interfaceIndex);
    QString getCurrentMnc(int interfaceIndex);
    QString getHomeMcc(int interfaceIndex);
    QString getHomeMnc(int interfaceIndex);
    QString getLac(int interfaceIndex);
    QNetworkInfo::NetworkMode getNetworkMode(int interfaceIndex);
    QNetworkInfo::NetworkMode getCurrentNetworkMode(QNetworkInfo::NetworkStatus status);
    QString getImsi(int interfaceIndex);
    QString getOperatorName(int interfaceIndex);

Q_SIGNALS:
    void cellIdChanged(int interface, const QString &id);
    void currentCellDataTechnologyChanged(int interface, QNetworkInfo::CellDataTechnology tech);
    void currentMobileCountryCodeChanged(int interface, const QString &mcc);
    void currentMobileNetworkCodeChanged(int interface, const QString &mnc);
    void currentNetworkModeChanged(QNetworkInfo::NetworkMode mode);
    void locationAreaCodeChanged(int interface, const QString &lac);
    void networkInterfaceCountChanged(QNetworkInfo::NetworkMode mode, int count);
    void networkNameChanged(QNetworkInfo::NetworkMode mode, int interface, const QString &name);
    void networkSignalStrengthChanged(QNetworkInfo::NetworkMode mode, int interface, int strength);
    void networkStatusChanged(QNetworkInfo::NetworkMode mode, int interface, QNetworkInfo::NetworkStatus status);

protected:
    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);

private Q_SLOTS:
    void onServiceAdded(const QString &serviceName, QService::Scope scope);
    void onServiceRemoved(const QString &serviceName, QService::Scope scope);
    void onSignalStrengthChanged(const int strength);
    void onTechnologyChanged(const QString &technology);
    void onNetworkStatusChanged(const QString &status);
    void onCellIdChanged(const uint cellid);
    void onCurrentMccChanged(const QString &currentMcc);
    void onCurrentMncChanged(const QString &currentMnc);
    void onLocationAreaCodeChanged(const uint lac);
    void onOperatorNameChanged(const QString& name);
    void onNetworkModeChanged(const QString &technology);

private:
    QServiceManager *serviceManager;
    QMap<int, QServiceInterfaceDescriptor> allNetworkManagerInterfaces;
    QMap<int, QObject*> loadedNetworkManagerInterfaces;

    bool watchInterfaceCount;
    bool watchSignalStrengths;
    bool watchTechnologies;
    bool watchStatuses;
    bool watchCellIds;
    bool watchCurrentMccs;
    bool watchCurrentMncs;
    bool watchLacs;
    bool watchOperatorNames;
    bool watchNetworkModes;
    QMap<int, int> signalStrengths;
    QMap<int, QNetworkInfo::CellDataTechnology> currentCellDataTechnologies;
    QMap<int, QNetworkInfo::NetworkStatus> networkStatuses;
    QMap<int, QString> cellIds;
    QMap<int, QString> currentMccs;
    QMap<int, QString> currentMncs;
    QMap<int, QString> lacs;
    QMap<int, QString> operatorNames;
    QMap<int, QNetworkInfo::NetworkMode> networkModes;

    void initServiceInterfaces();
    bool loadNetworkManagerInterface(int interfaceIndex);
    QNetworkInfo::CellDataTechnology technologyStringToEnum(const QString &technology);
    QNetworkInfo::NetworkMode technologyToMode(const QString &technology);
    QNetworkInfo::NetworkStatus statusStringToEnum(const QString &status);
};

QT_END_NAMESPACE

#endif // QT_NO_SFW_NETREG

#endif // QNETWORKSERVICEWRAPPER_P_H
