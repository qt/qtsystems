/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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
***************************************************************************/

#ifndef QDECLARATIVESERVICE_H
#define QDECLARATIVESERVICE_H

#include <QtCore>
#include <qserviceinterfacedescriptor.h>
#include <qservicemanager.h>
#include <qdeclarative.h>
#include <qdeclarativelist.h>

QT_BEGIN_NAMESPACE

class QDeclarativeService : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName NOTIFY interfaceNameChanged)
    Q_PROPERTY(QString serviceName READ serviceName NOTIFY serviceNameChanged)
    Q_PROPERTY(int majorVersion READ majorVersion NOTIFY majorVersionChanged)
    Q_PROPERTY(int minorVersion READ minorVersion NOTIFY minorVersionChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
    Q_PROPERTY(QObject* serviceObject READ serviceObject NOTIFY serviceObjectChanged)

public:
    QDeclarativeService();
    ~QDeclarativeService();

    void setInterfaceDesc(const QServiceInterfaceDescriptor& desc);
    QServiceInterfaceDescriptor interfaceDesc() const;

    void setInterfaceName(const QString& interface);
    QString interfaceName() const;
    QString serviceName() const;
    int majorVersion() const;
    int minorVersion() const;

    bool isValid() const;
    QObject* serviceObject();

Q_SIGNALS:
    void validChanged();
    void serviceObjectChanged();
    void interfaceNameChanged();
    void serviceNameChanged();
    void majorVersionChanged();
    void minorVersionChanged();

private:
    QObject* serviceInstance;
    QServiceManager* serviceManager;
    QServiceInterfaceDescriptor m_descriptor;
};


class QDeclarativeServiceList : public QObject, public QDeclarativeParserStatus {
    Q_OBJECT
    Q_INTERFACES(QDeclarativeParserStatus)
    Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName NOTIFY serviceNameChanged)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName NOTIFY interfaceNameChanged)
    Q_PROPERTY(int majorVersion READ majorVersion WRITE setMajorVersion NOTIFY majorVersionChanged)
    Q_PROPERTY(int minorVersion READ minorVersion WRITE setMinorVersion NOTIFY minorVersionChanged)
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

    void setVersionMatch(QDeclarativeServiceList::MatchRule match);
    QDeclarativeServiceList::MatchRule versionMatch() const;

    void listUpdated();

    //Derived from QDeclarativeParserStatus
    virtual void classBegin();
    virtual void componentComplete();

private:
    QList<QDeclarativeService *> m_services;
    QServiceManager* serviceManager;
    QString m_service;
    QString m_interface;
    int m_major;
    int m_minor;
    QDeclarativeServiceList::MatchRule m_match;
    bool m_componentComplete;

    void updateFilterResults();

    static void s_append(QDeclarativeListProperty<QDeclarativeService> *prop, QDeclarativeService *service);
    static int s_count(QDeclarativeListProperty<QDeclarativeService> *prop);
    static QDeclarativeService* s_at(QDeclarativeListProperty<QDeclarativeService> *prop, int index);
    static void s_clear(QDeclarativeListProperty<QDeclarativeService> *prop);

Q_SIGNALS:
    void resultsChanged();
    void servicesChanged(const QDeclarativeListProperty<QDeclarativeService>&);
    void serviceNameChanged();
    void interfaceNameChanged();
    void minorVersionChanged();
    void majorVersionChanged();
    void versionMatchChanged();

};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeService);
QML_DECLARE_TYPE(QDeclarativeServiceList);

#endif
