/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
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
