/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
** $QT_END_LICENSE$
**
***************************************************************************/

#ifndef QDECLARATIVESERVICE_H
#define QDECLARATIVESERVICE_H

#include <QtCore>
#include <qserviceinterfacedescriptor.h>
#include <qservicemanager.h>
#include <qdeclarative.h>
#include <qdeclarativelist.h>

QT_BEGIN_NAMESPACE

class QDeclarativeService : public QObject, public QDeclarativeParserStatus {
    Q_OBJECT
    Q_INTERFACES(QDeclarativeParserStatus)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName NOTIFY interfaceNameChanged)
    Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName NOTIFY serviceNameChanged)
    Q_PROPERTY(int majorVersion READ majorVersion WRITE setMajorVersion NOTIFY majorVersionChanged)
    Q_PROPERTY(int minorVersion READ minorVersion WRITE setMinorVersion NOTIFY minorVersionChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
    Q_PROPERTY(QObject* serviceObject READ serviceObject NOTIFY serviceObjectChanged)
    Q_PROPERTY(QString error READ lastError NOTIFY error)

public:
    QDeclarativeService();
    ~QDeclarativeService();

    void setInterfaceDesc(const QServiceInterfaceDescriptor& desc);
    QServiceInterfaceDescriptor interfaceDesc() const;

    void setInterfaceName(const QString& interface);
    QString interfaceName() const;
    QString serviceName() const;
    void setServiceName(const QString &service);
    int majorVersion() const;
    void setMajorVersion(int version);
    int minorVersion() const;
    void setMinorVersion(int version);
    QString lastError() const;

    bool isValid() const;
    QObject* serviceObject();

    //Derived from QDeclarativeParserStatus
    virtual void classBegin();
    virtual void componentComplete();

Q_SIGNALS:
    void validChanged();
    void serviceObjectChanged();
    void interfaceNameChanged();
    void serviceNameChanged();
    void majorVersionChanged();
    void minorVersionChanged();

    void error(const QString &errorString);

private slots:
    void IPCFault(QService::UnrecoverableIPCError);

private:
    void updateDescriptor();
    void setServiceObject(QObject *object);

    QPointer<QObject> m_serviceInstance;
    QServiceManager* m_serviceManager;

    QServiceInterfaceDescriptor m_descriptor;

    int m_minor;
    int m_major;
    QString m_service;
    QString m_interface;

    QString m_error;

    bool m_componentComplete;
};


class QDeclarativeServiceList : public QObject, public QDeclarativeParserStatus {
    Q_OBJECT
    Q_INTERFACES(QDeclarativeParserStatus)
    Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName NOTIFY serviceNameChanged)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName NOTIFY interfaceNameChanged)
    Q_PROPERTY(int majorVersion READ majorVersion WRITE setMajorVersion NOTIFY majorVersionChanged)
    Q_PROPERTY(int minorVersion READ minorVersion WRITE setMinorVersion NOTIFY minorVersionChanged)
    Q_PROPERTY(bool dynamicUpdates READ dynamicUpdates WRITE setDynamicUpdates NOTIFY dynamicUpdatesChanged)
    Q_PROPERTY(QDeclarativeListProperty<QDeclarativeService> services READ services NOTIFY resultsChanged)

    Q_PROPERTY(MatchRule versionMatch READ versionMatch WRITE setVersionMatch NOTIFY versionMatchChanged)
    Q_ENUMS(MatchRule)

public:
    enum MatchRule {
        Minimum = 0,
        Exact
    };

    QDeclarativeServiceList();
    ~QDeclarativeServiceList();

    QDeclarativeListProperty<QDeclarativeService> services();

    void setServiceName(const QString& service);
    QString serviceName() const;

    void setInterfaceName(const QString& interface);
    QString interfaceName() const;

    void setMinorVersion(int minor);
    int minorVersion() const;

    void setMajorVersion(int major);
    int majorVersion() const;

    void setDynamicUpdates(bool updates);
    bool dynamicUpdates() const;

    void setVersionMatch(QDeclarativeServiceList::MatchRule match);
    QDeclarativeServiceList::MatchRule versionMatch() const;

    void listUpdated();

    //Derived from QDeclarativeParserStatus
    virtual void classBegin();
    virtual void componentComplete();

Q_SIGNALS:
    void resultsChanged();
    void servicesChanged(const QDeclarativeListProperty<QDeclarativeService>&);
    void serviceNameChanged();
    void interfaceNameChanged();
    void minorVersionChanged();
    void majorVersionChanged();
    void versionMatchChanged();
    void dynamicUpdatesChanged();

protected slots:
    void servicesAddedRemoved();
    void updateFilterResults();

private:
    QList<QDeclarativeService *> m_services;
    QList<QServiceInterfaceDescriptor> m_currentList;
    QServiceManager* serviceManager;
    QString m_service;
    QString m_interface;
    int m_major;
    int m_minor;
    QDeclarativeServiceList::MatchRule m_match;
    bool m_componentComplete;
    bool m_dynamicUpdates;

    static void s_append(QDeclarativeListProperty<QDeclarativeService> *prop, QDeclarativeService *service);
    static int s_count(QDeclarativeListProperty<QDeclarativeService> *prop);
    static QDeclarativeService* s_at(QDeclarativeListProperty<QDeclarativeService> *prop, int index);
    static void s_clear(QDeclarativeListProperty<QDeclarativeService> *prop);


};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeService);
QML_DECLARE_TYPE(QDeclarativeServiceList);

#endif
