/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the test suite of the Qt Toolkit.
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

#include <qservicemanager.h>
#include "serviceobject_target.h"

#include <qcoreapplication.h>

#ifdef Q_OS_LINUX
#include <sys/socket.h>
#include <sys/un.h>
#endif

ServiceObjectTarget::ServiceObjectTarget(QObject *parent)
    : QObject(parent)
{
//    qWarning() << "Target created";
}

ServiceObjectTarget::~ServiceObjectTarget()
{
//    qWarning() << "Target destroyed";
}

void ServiceObjectTarget::slotWithArg(const QString &number)
{
    for (int i = 0; i < number.toInt(); i++) {
        emit stateChanged();
    }
    m_state = number.toInt();
}

void ServiceObjectTarget::slotWithoutArg()
{
    qWarning() << "Target without arg";
}

bool ServiceObjectTarget::slotWithOk()
{
    return true;
}

void ServiceObjectTarget::timerEvent(QTimerEvent *event)
{

}

int ServiceObjectTarget::state() const
{
    return m_state;
}

void ServiceObjectTarget::closeClientSockets()
{
    closeClientSocketsBlocking();
}

bool ServiceObjectTarget::closeClientSocketsBlocking()
{
#ifdef Q_OS_UNIX
    for (int i = 0; i < 1024; i++) {
        struct sockaddr_un un;
        socklen_t len = sizeof(struct sockaddr_un);
        if (-1 != ::getsockname(i, (struct sockaddr*)&un, &len)) {
            struct sockaddr_un unp;
            socklen_t lenp = sizeof(struct sockaddr_un);
            if (-1 != ::getpeername(i, (struct sockaddr*)&unp, &lenp)) {
                // found a client socket
                qDebug() << "Socket peer" << i << un.sun_path;
                close(i);
            }
        }
    }
#endif
}

int main(int argc, char **argv)
{


    QCoreApplication app(argc, argv);

    //register the unique service
    QRemoteServiceRegister* serviceRegister = new QRemoteServiceRegister();
    QRemoteServiceRegister::Entry entry =
            serviceRegister->createEntry<ServiceObjectTarget>(
                "ServiceObjectTarget", "com.nokia.qt.tests.serviceobject", "1.0");
    entry.setInstantiationType(QRemoteServiceRegister::PrivateInstance);
    serviceRegister->setQuitOnLastInstanceClosed(false);
    serviceRegister->publishEntries("com.nokia.qt.tests.serviceobject");

    QServiceManager *mgr = new QServiceManager();
    if (!mgr->addService(":/xml/serviceobject.xml")) {
        qWarning() << "failed to register service" << mgr->error();
    }

    app.exec();
}
