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
***************************************************************************/

#ifndef QDECLARATIVESERVICE_P_H
#define QDECLARATIVESERVICE_P_H

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

#include <QtCore>
#include <qserviceinterfacedescriptor.h>
#include <qservicemanager.h>
#include <qqml.h>
#include <qqmllist.h>

QT_BEGIN_NAMESPACE

class QDeclarativeServiceDescriptor : public QObject, public QServiceInterfaceDescriptor {
    Q_OBJECT
    Q_PROPERTY(QString serviceName READ serviceName CONSTANT)
    Q_PROPERTY(QString interfaceName READ interfaceName CONSTANT)
    Q_PROPERTY(int majorVersion READ majorVersion CONSTANT)
    Q_PROPERTY(int minorVersion READ minorVersion CONSTANT)
    Q_PROPERTY(bool valid READ isValid CONSTANT)

public:
    QDeclarativeServiceDescriptor(QObject* parent = 0) : QObject(parent) {}
    QDeclarativeServiceDescriptor(const QDeclarativeServiceDescriptor& other)
        : QObject(0), QServiceInterfaceDescriptor(other) {}
    QDeclarativeServiceDescriptor(const QServiceInterfaceDescriptor& other)
        : QObject(0), QServiceInterfaceDescriptor(other) {}
    bool operator==(const QServiceInterfaceDescriptor& other) const
    { return QServiceInterfaceDescriptor::operator==(other); }
    QDeclarativeServiceDescriptor& operator=(const QServiceInterfaceDescriptor& other)
    {
        QServiceInterfaceDescriptor::operator=(other);
        return *this;
    }
    QDeclarativeServiceDescriptor& operator=(const QDeclarativeServiceDescriptor& other)
    {
        QServiceInterfaceDescriptor::operator=(other);
        return *this;
    }
};

class QDeclarativeServiceLoader : public QObject, public QQmlParserStatus {
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName NOTIFY interfaceNameChanged)
    // ### Rename descriptor?
    Q_PROPERTY(QDeclarativeServiceDescriptor* serviceDescriptor READ serviceDescriptor WRITE setServiceDescriptor NOTIFY serviceDescriptorChanged RESET resetServiceDescriptor) //Takes precedence over interfaceName if set
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool asynchronous READ asynchronous WRITE setAsynchronous NOTIFY asynchronousChanged)
    // ### Rename object?
    Q_PROPERTY(QObject* serviceObject READ serviceObject NOTIFY serviceObjectChanged)
    Q_ENUMS(Status)

public:
    enum Status {
        Null = 0,
        Ready,
        Loading,
        Error
    };

    QDeclarativeServiceLoader();
    ~QDeclarativeServiceLoader();

    Q_INVOKABLE QString errorString() const
    {
        if (m_status == Error)
            return m_errorString;
        else
            return "";
    }

    QString interfaceName() const
    {
        return m_interfaceName;
    }

    QDeclarativeServiceDescriptor* serviceDescriptor() const
    {
        return m_serviceDescriptor;
    }

    void resetServiceDescriptor()
    {
        setServiceDescriptor(0);
    }

    Status status() const
    {
        return m_status;
    }

    bool asynchronous() const
    {
        return m_asynchronous;
    }

    QObject* serviceObject() const
    {
        return m_serviceObject;
    }

public slots:
    void setInterfaceName(QString arg)
    {
        if (m_interfaceName != arg) {
            m_interfaceName = arg;
            emit interfaceNameChanged(arg);
            if (!m_serviceDescriptor && m_componentComplete)
                startLoading();
        }
    }

    void setServiceDescriptor(QDeclarativeServiceDescriptor* arg)
    {
        if (m_serviceDescriptor != arg) {
            m_serviceDescriptor = arg;
            emit serviceDescriptorChanged(arg);
            if (m_componentComplete)
                startLoading();
        }
    }

    void setAsynchronous(bool arg)
    {
        if (m_asynchronous != arg) {
            m_asynchronous = arg;
            emit asynchronousChanged(arg);
        }
    }

private Q_SLOTS:
    void setStatus(Status arg)
    {
        if (m_status != arg) {
            m_status = arg;
            emit statusChanged(arg);
        }
    }

protected:
    virtual void classBegin() {}
    virtual void componentComplete();

signals:
    void interfaceNameChanged(QString arg);

    void serviceDescriptorChanged(QServiceInterfaceDescriptor* arg);

    void statusChanged(Status arg);

    void asynchronousChanged(bool arg);

    void serviceObjectChanged(QObject* arg);

private slots:
    void startLoading();
    void finishLoading();
    void IPCFault(QService::UnrecoverableIPCError);

private:
    QString m_interfaceName;
    QDeclarativeServiceDescriptor* m_serviceDescriptor;
    Status m_status;
    bool m_asynchronous;
    QObject* m_serviceObject;
    QString m_errorString;

    bool m_componentComplete;
    QServiceManager* m_serviceManager;
    QServiceReply* m_serviceReply;
};

class QDeclarativeServiceFilter : public QObject, public QQmlParserStatus {
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName NOTIFY serviceNameChanged)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName NOTIFY interfaceNameChanged)
    Q_PROPERTY(int majorVersion READ majorVersion WRITE setMajorVersion NOTIFY majorVersionChanged)
    Q_PROPERTY(int minorVersion READ minorVersion WRITE setMinorVersion NOTIFY minorVersionChanged)
    Q_PROPERTY(bool exactVersionMatching READ exactVersionMatching WRITE setExactVersionMatching NOTIFY exactVersionMatchingChanged)
    // ### Rename monitorRegistrations ?
    Q_PROPERTY(bool monitorServiceRegistrations READ monitorServiceRegistrations WRITE setMonitorServiceRegistrations NOTIFY monitorServiceRegistrationsChanged)
    // ### Rename services ?
    Q_PROPERTY(QQmlListProperty<QDeclarativeServiceDescriptor> serviceDescriptions READ serviceDescriptions NOTIFY serviceDescriptionsChanged)
public:
    QDeclarativeServiceFilter(QObject* parent = 0);
    ~QDeclarativeServiceFilter();
    QString serviceName() const
    {
        return m_serviceName;
    }

    QString interfaceName() const
    {
        return m_interfaceName;
    }

    int majorVersion() const
    {
        return m_majorVersion;
    }

    int minorVersion() const
    {
        return m_minorVersion;
    }

    bool exactVersionMatching() const
    {
        return m_exactVersionMatching;
    }

    bool monitorServiceRegistrations() const
    {
        return m_monitorServiceRegistrations;
    }

    QQmlListProperty<QDeclarativeServiceDescriptor> serviceDescriptions()
    {
        return QQmlListProperty<QDeclarativeServiceDescriptor> (this, 0, s_append, s_count, s_at, s_clear);
    }

public slots:
    void setServiceName(QString arg)
    {
        if (m_serviceName != arg) {
            m_serviceName = arg;
            emit serviceNameChanged(arg);
            updateServiceList();
        }
    }

    void setInterfaceName(QString arg)
    {
        if (m_interfaceName != arg) {
            m_interfaceName = arg;
            emit interfaceNameChanged(arg);
            updateServiceList();
        }
    }

    void setMajorVersion(int arg)
    {
        if (m_majorVersion != arg) {
            m_majorVersion = arg;
            emit majorVersionChanged(arg);
            updateServiceList();
        }
    }

    void setMinorVersion(int arg)
    {
        if (m_minorVersion != arg) {
            m_minorVersion = arg;
            emit minorVersionChanged(arg);
            updateServiceList();
        }
    }

    void setExactVersionMatching(bool arg)
    {
        if (m_exactVersionMatching != arg) {
            m_exactVersionMatching = arg;
            emit exactVersionMatchingChanged(arg);
            updateServiceList();
        }
    }

    void setMonitorServiceRegistrations(bool updates);

protected:
    virtual void classBegin() {}
    virtual void componentComplete();
signals:
    void serviceNameChanged(QString arg);

    void interfaceNameChanged(QString arg);

    void majorVersionChanged(int arg);

    void minorVersionChanged(int arg);

    void exactVersionMatchingChanged(bool arg);

    void monitorServiceRegistrationsChanged(bool arg);

    void serviceDescriptionsChanged();

private slots:
    void updateServiceList();
    void servicesAddedRemoved();
private:
    QString m_serviceName;
    QString m_interfaceName;
    int m_majorVersion;
    int m_minorVersion;
    bool m_exactVersionMatching;
    bool m_monitorServiceRegistrations;
    QQmlListProperty<QDeclarativeServiceDescriptor*> m_serviceDescriptions;
    QList<QDeclarativeServiceDescriptor> m_services;

    QServiceManager* m_serviceManager;
    bool m_componentComplete;

    static void s_append(QQmlListProperty<QDeclarativeServiceDescriptor> *prop, QDeclarativeServiceDescriptor *service);
    static int s_count(QQmlListProperty<QDeclarativeServiceDescriptor> *prop);
    static QDeclarativeServiceDescriptor* s_at(QQmlListProperty<QDeclarativeServiceDescriptor> *prop, int index);
    static void s_clear(QQmlListProperty<QDeclarativeServiceDescriptor> *prop);
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeServiceLoader);
QML_DECLARE_TYPE(QDeclarativeServiceDescriptor);
QML_DECLARE_TYPE(QDeclarativeServiceFilter);


#endif // QDECLARATIVESERVICE_P_H
