/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QREMOTESERVICEREGISTER_H
#define QREMOTESERVICEREGISTER_H

#include "qserviceframeworkglobal.h"
#include <QObject>
#include <QQueue>
#include <QHash>
#include <QDebug>
#include <QExplicitlySharedDataPointer>

QT_BEGIN_NAMESPACE

class QRemoteServiceRegisterPrivate;
class QRemoteServiceRegisterEntryPrivate;

class Q_SERVICEFW_EXPORT QRemoteServiceRegister : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool quitOnLastInstanceClosed READ quitOnLastInstanceClosed WRITE setQuitOnLastInstanceClosed)
public:

    enum InstanceType {
        GlobalInstance = 0,
        PrivateInstance
    };

    typedef QObject *(*CreateServiceFunc)();

    class Q_SERVICEFW_EXPORT Entry {
    public:
        Entry();
        Entry(const Entry &);
        Entry &operator=(const Entry &);

        ~Entry();

        bool operator==(const Entry &) const;
        bool operator!=(const Entry &) const;

        bool isValid() const;

        QString interfaceName() const;
        QString serviceName() const;
        QString version() const;

        void setInstantiationType(QRemoteServiceRegister::InstanceType type);
        QRemoteServiceRegister::InstanceType instantiationType() const;

    private:
        QExplicitlySharedDataPointer<QRemoteServiceRegisterEntryPrivate> d;

        const QMetaObject* metaObject() const;

        friend class QRemoteServiceRegisterPrivate;
        friend class QRemoteServiceRegister;
        friend class InstanceManager;
        friend class QServiceManager;
#ifndef QT_NO_DATASTREAM
        friend Q_SERVICEFW_EXPORT QDataStream &operator<<(QDataStream &, const QRemoteServiceRegister::Entry &);
        friend Q_SERVICEFW_EXPORT QDataStream &operator>>(QDataStream &, QRemoteServiceRegister::Entry &);
#endif
    };

    QRemoteServiceRegister(QObject* parent = 0);
    ~QRemoteServiceRegister();

    template <typename T>
    Entry createEntry(const QString& serviceName,
                    const QString& interfaceName, const QString& version);

    void publishEntries(const QString& ident );

    bool quitOnLastInstanceClosed() const;
    void setQuitOnLastInstanceClosed(const bool quit);

    typedef bool (*SecurityFilter)(const void *message);
    SecurityFilter setSecurityFilter(SecurityFilter filter);

Q_SIGNALS:
    void allInstancesClosed();
    void instanceClosed(const QRemoteServiceRegister::Entry& entry);

private:

    Entry createEntry(const QString& serviceName,
                    const QString& interfaceName, const QString& version,
                    CreateServiceFunc cptr, const QMetaObject* meta);

    QRemoteServiceRegisterPrivate* d;
};

struct QRemoteServiceRegisterCredentials {
    int fd;
    int pid;
    int uid;
    int gid;
};

inline uint qHash(const QRemoteServiceRegister::Entry& e) {
    //Only consider version, iface and service name -> needs to sync with operator==
    return ( qHash(e.serviceName()) + qHash(e.interfaceName()) + qHash(e.version()) );
}

#ifndef QT_NO_DATASTREAM
Q_SERVICEFW_EXPORT QDataStream& operator>>(QDataStream& s, QRemoteServiceRegister::Entry& entry);
Q_SERVICEFW_EXPORT QDataStream& operator<<(QDataStream& s, const QRemoteServiceRegister::Entry& entry);
#endif

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QRemoteServiceRegister::Entry& entry);
#endif

template <typename T>
QObject* qServiceTypeConstructHelper()
{
    return new T;
}

template <typename T>
QRemoteServiceRegister::Entry QRemoteServiceRegister::createEntry(const QString& serviceName,
                const QString& interfaceName, const QString& version)
{

    QRemoteServiceRegister::CreateServiceFunc cptr = qServiceTypeConstructHelper<T>;
    return createEntry(serviceName, interfaceName, version, cptr, &T::staticMetaObject);
}


QT_END_NAMESPACE
#endif //QREMOTESERVICEREGISTER_H
