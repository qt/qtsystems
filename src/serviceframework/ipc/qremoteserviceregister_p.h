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

#ifndef QREMOTESERVICEREGISTER_P_H
#define QREMOTESERVICEREGISTER_P_H

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


public slots:
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
    static QObject* proxyForService(const QRemoteServiceRegister::Entry& entry, const QString& location);
    static QRemoteServiceRegisterPrivate* constructPrivateObject(QObject *parent);
};

QT_END_NAMESPACE

#endif
