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

#ifndef QSYSTEMINFOCONNECTION_SIMULATOR_P_H
#define QSYSTEMINFOCONNECTION_SIMULATOR_P_H

#include "qsysteminfodata_simulator_p.h"

#include <QObject>

namespace Simulator
{
    class Connection;
    class ConnectionWorker;
    class Version;
}

class SystemInfoConnection : public QObject
{
    Q_OBJECT

public:
    static void ensureSimulatorConnection();
    virtual ~SystemInfoConnection();

private:
    SystemInfoConnection(QObject *parent = 0);
    Q_DISABLE_COPY(SystemInfoConnection)

    bool save() const { return mInitialDataSent; }

private Q_SLOTS:
    void initialSystemInfoDataSent();
    void setBatteryInfoData(const QBatteryInfoData &data);
    void setDeviceInfoData(const QDeviceInfoData &data);
    void setNetworkInfoData(const QNetworkInfoData &data);

Q_SIGNALS:
    void initialDataReceived();

private:
    Simulator::Connection *mConnection;
    Simulator::ConnectionWorker *mWorker;
    bool mInitialDataSent;

    static const QString SERVERNAME;
    static const int PORT;
    static const Simulator::Version VERSION;
};

#endif // QSYSTEMINFOCONNECTION_SIMULATOR_P_H
