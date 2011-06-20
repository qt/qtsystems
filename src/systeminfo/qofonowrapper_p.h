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

#ifndef QOFONOWRAPPER_P_H
#define QOFONOWRAPPER_P_H

#include <QtCore/qobject.h>
#include <QtDBus/qdbuscontext.h>
#include <QtDBus/qdbusextratypes.h>

#include <qnetworkinfo.h>

#if !defined(QT_NO_OFONO)

QT_BEGIN_NAMESPACE

struct QOfonoProperties
{
    QDBusObjectPath path;
    QVariantMap properties;
};
Q_DECLARE_METATYPE(QOfonoProperties)

typedef QList<QOfonoProperties> QOfonoPropertyMap;
Q_DECLARE_METATYPE(QOfonoPropertyMap)

class QOfonoWrapper : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    QOfonoWrapper(QObject *parent = 0);

    bool isOfonoAvailable();

    // Manager Interface
    QStringList allModems();

    // Network Registration Interface
    int signalStrength(const QString &modemPath);
    QList<QDBusObjectPath> allOperators(const QString &modemPath);
    QNetworkInfo::CellDataTechnology currentCellDataTechnology(const QString &modemPath);
    QNetworkInfo::NetworkStatus networkStatus(const QString &modemPath);
    QString cellId(const QString &modemPath);
    QString currentMcc(const QString &modemPath);
    QString currentMnc(const QString &modemPath);
    QString lac(const QString &modemPath);
    QString operatorName(const QString &modemPath);

    // SIM Manager Interface
    QString homeMcc(const QString &modemPath);
    QString homeMnc(const QString &modemPath);
    QString imsi(const QString &modemPath);

    // Modem Interface
    QString imei(const QString&modemPath);

Q_SIGNALS:
    void cellIdChanged(int interface, const QString &id);
    void currentCellDataTechnologyChanged(int interface, QNetworkInfo::CellDataTechnology tech);
    void currentMobileCountryCodeChanged(int interface, const QString &mcc);
    void currentMobileNetworkCodeChanged(int interface, const QString &mnc);
    void currentNetworkModeChanged(QNetworkInfo::NetworkMode mode);
    void locationAreaCodeChanged(int interface, const QString &lac);
    void networkNameChanged(QNetworkInfo::NetworkMode mode, int interface, const QString &name);
    void networkSignalStrengthChanged(QNetworkInfo::NetworkMode mode, int interface, int strength);
    void networkStatusChanged(QNetworkInfo::NetworkMode mode, int interface, QNetworkInfo::NetworkStatus status);

protected:
    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);

private Q_SLOTS:
    void onOfonoPropertyChanged(const QString &property, const QDBusVariant &value);

private:
    int available;

    QNetworkInfo::CellDataTechnology technologyStringToEnum(const QString &technology);
    QNetworkInfo::NetworkMode technologyToMode(const QString &technology);
    QNetworkInfo::NetworkStatus statusStringToEnum(const QString &status);
    QString currentTechnology(const QString &modemPath);
};

QT_END_NAMESPACE

#endif // QT_NO_OFONO

#endif // QOFONOWRAPPER_P_H
