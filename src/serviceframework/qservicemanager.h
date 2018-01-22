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

#ifndef QSERVICEMANAGER_H
#define QSERVICEMANAGER_H

#include "qserviceframeworkglobal.h"

#include "qservice.h"
#include "qserviceinterfacedescriptor.h"
#include "qservicefilter.h"

#include <QObject>
#include <QList>
#include <QStringList>
#include <QDebug>

QT_BEGIN_NAMESPACE

class QServiceContext;
class QServiceFilter;
class QServiceManagerPrivate;
class QServiceReply;
class Q_SERVICEFW_EXPORT QServiceManager : public QObject
{
    Q_OBJECT

public:

   enum Error {
        NoError,
        StorageAccessError,
        InvalidServiceLocation,
        InvalidServiceXml,
        InvalidServiceInterfaceDescriptor,
        ServiceAlreadyExists,
        ImplementationAlreadyExists,
        PluginLoadingFailed,
        ComponentNotFound,
        ServiceCapabilityDenied,
        UnknownError = 100
    };

    explicit QServiceManager(QObject *parent = Q_NULLPTR);
    explicit QServiceManager(QService::Scope scope, QObject *parent = Q_NULLPTR);
    ~QServiceManager();

    QService::Scope scope() const;

    QStringList findServices(const QString& interfaceName = QString()) const;
    QList<QServiceInterfaceDescriptor> findInterfaces(const QServiceFilter& filter = QServiceFilter()) const;
    QList<QServiceInterfaceDescriptor> findInterfaces(const QString& serviceName) const;

    QObject* loadInterface(const QString& interfaceName);
    QObject* loadInterface(const QServiceInterfaceDescriptor& descriptor);

    bool isInterfaceRunning(const QString& interfaceName);
    bool isInterfaceRunning(const QServiceInterfaceDescriptor& descriptor);

    template <class T>
    T* loadLocalTypedInterface(const QString& interfaceName);

    template <class T>
    T* loadLocalTypedInterface(const QServiceInterfaceDescriptor& descriptor);

    QServiceReply *loadInterfaceRequest(const QString& interfaceName);
    QServiceReply *loadInterfaceRequest(const QServiceInterfaceDescriptor& descriptor);

    bool addService(const QString& xmlFilePath);
    bool addService(QIODevice* xmlDevice);
    bool removeService(const QString& serviceName);

    bool setInterfaceDefault(const QString &service, const QString &interfaceName);
    bool setInterfaceDefault(const QServiceInterfaceDescriptor& descriptor);

    QServiceInterfaceDescriptor interfaceDefault(const QString& interfaceName) const;

    QServiceManager::Error error() const;

    bool event(QEvent *);

protected:
    void connectNotify(const QMetaMethod &signal);
    void disconnectNotify(const QMetaMethod &signal);

Q_SIGNALS:
    void serviceAdded(const QString& serviceName, QService::Scope scope);
    void serviceRemoved(const QString& serviceName, QService::Scope scope);
    void errorChanged();

private:
    QObject *loadInterProcessService(const QServiceInterfaceDescriptor &descriptor,
                                          const QString &location) const;
    QObject *loadInProcessService(const QServiceInterfaceDescriptor &descriptor,
                                  const QString &serviceFilePath) const;

    static QString resolveLibraryPath(const QString &libNameOrPath);

    friend class QServiceManagerPrivate;
    friend class QServiceOperationProcessor;
    QServiceManagerPrivate* d;
};

template <class T>
Q_INLINE_TEMPLATE T* QServiceManager::loadLocalTypedInterface(const QString& interfaceName)
{
    return loadLocalTypedInterface<T>(interfaceDefault(interfaceName));
}

template <class T>
Q_OUTOFLINE_TEMPLATE T* QServiceManager::loadLocalTypedInterface(const QServiceInterfaceDescriptor& descriptor)
{
    T* instance = Q_NULLPTR;
    if (descriptor.isValid()) {
        QObject* obj = loadInterface(descriptor);
        if (!obj) return Q_NULLPTR;

        //TODO this should really be
        //instance = qobject_cast<T *>(loadInterface(descriptor, context, session));
        //check why qobject_cast fails
        const char* templateClassName = T::staticMetaObject.className();
        const QMetaObject* source = obj->metaObject();
        do {
            if (strcmp(templateClassName,source->className())==0) {
                instance = static_cast<T *>(obj);
                break;
            }
            source = source->superClass();
        } while (source);
        if (!instance)
            delete obj;
    }
    return instance;
}

QT_END_NAMESPACE

#endif
