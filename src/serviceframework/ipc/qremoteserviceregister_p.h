/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QREMOTESERVICEREGISTER_P_H
#define QREMOTESERVICEREGISTER_P_H

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

#include "qremoteserviceregister.h"
#include "instancemanager_p.h"
#include "qserviceinterfacedescriptor.h"

QT_BEGIN_NAMESPACE

class ObjectEndPoint;
class QRemoteServiceRegisterPrivate: public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool quitOnLastInstanceClosed READ quitOnLastInstanceClosed WRITE setQuitOnLastInstanceClosed)
public:
    QRemoteServiceRegisterPrivate(QObject* parent);
    virtual ~QRemoteServiceRegisterPrivate();

    virtual void publishServices(const QString& ident ) = 0;

    virtual bool quitOnLastInstanceClosed() const;
    virtual void setQuitOnLastInstanceClosed(const bool quit);

    virtual QRemoteServiceRegister::SecurityFilter setSecurityFilter(QRemoteServiceRegister::SecurityFilter filter);

    void setBaseUserIdentifier(qintptr uid);
    qintptr getBaseUserIdentifier() const;
    bool isBaseUserIdentifierSet() const;

    void setBaseGroupIdentifier(qintptr gid);
    qintptr getBaseGroupIdentifier() const;
    bool isBaseGroupIdentifierSet() const;

    void setSecurityOptions(QRemoteServiceRegister::SecurityAccessOptions options);


public Q_SLOTS:
    // Must be implemented in the subclass
    //void processIncoming();

protected:
    virtual QRemoteServiceRegister::SecurityFilter getSecurityFilter();
    QRemoteServiceRegister::SecurityAccessOptions getSecurityOptions() const;

private:
    bool m_quit;
    QRemoteServiceRegister::SecurityFilter iFilter;
    QRemoteServiceRegister::SecurityAccessOptions securityOptions;
    qintptr userIdentifier;
    bool userIdentifierSet;
    qintptr groupIdentifier;
    bool groupIdentifierSet;

public:
    // These methods apply to IPC services only
    static QObject* proxyForService(const QRemoteServiceRegister::Entry& entry, const QString& location);
    static QRemoteServiceRegisterPrivate* constructPrivateObject(QObject *parent);
    static bool isServiceRunning(const QRemoteServiceRegister::Entry&, const QString& location);
    // Create a private object based on the service type
    static QRemoteServiceRegisterPrivate* constructPrivateObject(QService::Type serviceType, QObject *parent);
};

QT_END_NAMESPACE

#endif //QREMOTESERVICEREGISTER_P_H
