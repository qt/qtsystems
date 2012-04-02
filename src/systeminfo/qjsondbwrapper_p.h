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

#ifndef QJSONDBWRAPPER_P_H
#define QJSONDBWRAPPER_P_H

#include <qdeviceinfo.h>
#include <qdisplayinfo.h>
#include <QtJsonDb/qjsondbconnection.h>
#include <QtJsonDb/qjsondbrequest.h>

QT_BEGIN_NAMESPACE

class QTimer;
class QEventLoop;

class QJsonDbWrapper : public QObject
{
    Q_OBJECT
public:
    QJsonDbWrapper(QObject *parent = 0);
    virtual ~QJsonDbWrapper();

    // DeviceInfo Interface
    QDeviceInfo::LockTypeFlags getActivatedLocks();
    QDeviceInfo::LockTypeFlags getEnabledLocks();
    bool hasFeaturePositioning();
    QString getUniqueDeviceID();
    QString getModel();

    // DeviceProfile Interface
    bool isVibrationActivated();
    int getRingtoneVolume();

Q_SIGNALS:
    void activatedLocksChanged(QDeviceInfo::LockTypeFlags types);
    void enabledLocksChanged(QDeviceInfo::LockTypeFlags types);
    void backlightStateChanged(int screen, QDisplayInfo::BacklightState state);

protected:
    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);

private Q_SLOTS:
    void onJsonDbConnectionError(QtJsonDb::QJsonDbConnection::ErrorCode error, const QString &message);
    void onJsonDbRequestError(QtJsonDb::QJsonDbRequest::ErrorCode error, const QString &message);
    void onJsonDbRequestFinished();
    void onJsonDbWatcherNotificationsAvailable();
    void onJsonDbWatcherNotificationsBacklightStateAvailable();

private:
    QJsonValue getSystemPropertyValue(const QString &objectType, const QString &property, const QString &partition = QStringLiteral(""));
    QJsonValue getSystemSettingValue(const QString &settingId, const QString &setting, const QString &partition = QStringLiteral(""));
    bool hasSystemObject(const QString &objectType);
    bool waitForResponse();

    QtJsonDb::QJsonDbConnection jsonDbConnection;
    QtJsonDb::QJsonDbWatcher *jsonDbWatcher;
    QtJsonDb::QJsonDbWatcher *backlightWatcher;

    QEventLoop *waitLoop;
    QTimer *timer;

    bool watchActivatedLocks;
    bool watchEnabledLocks;
    bool watchBacklightState;
    QDeviceInfo::LockTypeFlags activatedLocks;
    QDeviceInfo::LockTypeFlags enabledLocks;
};

QT_END_NAMESPACE

#endif // QJSONDBWRAPPER_P_H
