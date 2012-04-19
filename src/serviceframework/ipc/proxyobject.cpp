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

#include "proxyobject_p.h"
#include <private/qmetaobjectbuilder_p.h>
#include "qremoteserviceregisterentry_p.h"
#include "qservicedebuglog_p.h"

#include <qtimer.h>
#include <qcoreevent.h>

#include <QDebug>
#include <QCoreApplication>

QT_BEGIN_NAMESPACE

class QServiceProxyPrivate
{
public:
    QByteArray metadata;
    QMetaObject* meta;
    ObjectEndPoint* endPoint;
    int *localToRemote;
    int *remoteToLocal;

};

QServiceProxy::QServiceProxy(const QByteArray& metadata, ObjectEndPoint* endPoint, QObject* parent)
    : QServiceProxyBase(endPoint, parent)
{
    Q_ASSERT(endPoint);
    d = new QServiceProxyPrivate();
    d->metadata = metadata;
    d->meta = 0;
    d->endPoint = endPoint;
    d->localToRemote = 0;
    d->remoteToLocal = 0;

    QDataStream stream(d->metadata);
    QMetaObjectBuilder builder;
    QMap<QByteArray, const QMetaObject*> refs;

    builder.deserialize(stream, refs);
    if (stream.status() != QDataStream::Ok) {
        qWarning() << "Invalid metaObject for service received";
    } else {
        QMetaObject *remote = builder.toMetaObject();

        builder.setSuperClass(QServiceProxyBase::metaObject());

        QMetaObject *local = builder.toMetaObject();

        d->remoteToLocal = new int[local->methodCount()];
        d->localToRemote = new int[local->methodCount()];

        for (int i = 0; i < local->methodCount(); i++){
            const QMetaMethod m = local->method(i);
            int r = remote->indexOfMethod(m.methodSignature().constData());
            d->localToRemote[i] = r;
            if (r > 0)
                d->remoteToLocal[r] = i;
        }

#if defined(QT_SFW_IPC_DEBUG) && defined(QT_SFW_IPC_DEBUG_VERBOSE)
        QString mapping = QString::fromLatin1("%%% QWE Doing lookup table for ") + endPoint->objectName();
        for (int i = 0; i < local->methodCount(); i++){
            const QMetaMethod m = local->method(i);
            int r = d->localToRemote[i];
            mapping.append(QString::fromLatin1("\n%%%Mapping %1 from %2 to %3").arg(QString::fromLatin1(m.signature())).arg(i).arg(r));
        }
        QServiceDebugLog::instance()->appendToLog(mapping);
#endif

        d->meta = local;

        endPoint->setLookupTable(d->localToRemote, d->remoteToLocal);
    }
}

QServiceProxy::~QServiceProxy()
{
    delete[] d->remoteToLocal;
    delete[] d->localToRemote;
    if (d->meta)
        qFree(d->meta);
    delete d;
}

//provide custom Q_OBJECT implementation
const QMetaObject* QServiceProxy::metaObject() const
{
    return d->meta;
}

int QServiceProxy::qt_metacall(QMetaObject::Call c, int id, void **a)
{
    id = QServiceProxyBase::qt_metacall(c, id, a);
    if (id < 0 || !d->meta)
        return id;

    if (c == QMetaObject::InvokeMetaMethod) {

        const int mcount = d->meta->methodCount() - d->meta->methodOffset();
        const int metaIndex = id + d->meta->methodOffset();

        QMetaMethod method = d->meta->method(metaIndex);

        const int returnType = method.returnType();

        //process arguments
        const QList<QByteArray> pTypes = method.parameterTypes();
        const int pTypesCount = pTypes.count();
        QVariantList args ;
        if (pTypesCount > 10) {
            qWarning() << "Cannot call" << method.methodSignature() << ". More than 10 parameter.";
            return id;
        }
        for (int i=0; i < pTypesCount; i++) {
            const QByteArray& t = pTypes[i];

            int variantType = QMetaType::type(t);

            if (variantType == QMetaType::QVariant) {  //ignore whether QVariant is declared as metatype
                args << *reinterpret_cast<const QVariant(*)>(a[i+1]);
            } else if ( variantType == 0 ){
                qWarning("%s: argument %s has unknown type. Use qRegisterMetaType to register it.",
                        method.methodSignature().constData(), t.data());
                return id;
            } else {
                args << QVariant(variantType, a[i+1]);
            }
        }

        if (returnType == QMetaType::Void) {

            QServiceDebugLog::instance()->appendToLog(
                        QString::fromLatin1("--> non-blocking method %1 for %2")
                        .arg(QString::fromLatin1(method.methodSignature()))
                        .arg(d->endPoint->objectName()));

            d->endPoint->invokeRemote(d->localToRemote[metaIndex], args, returnType);
        } else {
            //TODO: invokeRemote() parameter list needs review

            QServiceDebugLog::instance()->appendToLog(
                        QString::fromLatin1("--> non-BLOCKING method %1 for %2")
                        .arg(QString::fromLatin1(method.methodSignature()))
                        .arg(d->endPoint->objectName()));

            QVariant result = d->endPoint->invokeRemote(d->localToRemote[metaIndex], args, returnType);
            if (result.type() != QVariant::Invalid){
                if (returnType != QMetaType::QVariant) {
                    QByteArray buffer;
                    QDataStream stream(&buffer, QIODevice::ReadWrite);
                    QMetaType::save(stream, returnType, result.constData());
                    stream.device()->seek(0);
                    QMetaType::load(stream, returnType, a[0]);
                } else {
                    if (a[0]) *reinterpret_cast< QVariant*>(a[0]) = result;
                }
            }
        }
        id-=mcount;
    } else if ( c == QMetaObject::ReadProperty
            || c == QMetaObject::WriteProperty
            || c == QMetaObject::ResetProperty ) {
        const int pCount = d->meta->propertyCount() - d->meta->propertyOffset();
        const int metaIndex = id + d->meta->propertyOffset();
        QMetaProperty property = d->meta->property(metaIndex);
        if (property.isValid()) {
            int pType = property.userType();

            QVariant arg;
            if ( c == QMetaObject::WriteProperty ) {

                QServiceDebugLog::instance()->appendToLog(
                            QString::fromLatin1("--> proerty WRITE operation %1 for %2")
                            .arg(QString::fromLatin1(property.name()))
                            .arg(d->endPoint->objectName()));


                if (pType == QMetaType::QVariant)
                    arg =  *reinterpret_cast<const QVariant(*)>(a[0]);
                else if (pType == 0) {
                    qWarning("%s: property %s has unkown type", property.name(), property.typeName());
                    return id;
                } else {
                    arg = QVariant(pType, a[0]);
                }
            }
            QVariant result;
            if (c == QMetaObject::ReadProperty) {

                QServiceDebugLog::instance()->appendToLog(
                            QString::fromLatin1("--> proerty READ operation %1 for %2")
                            .arg(QString::fromLatin1(property.name()))
                            .arg(d->endPoint->objectName()));

                result = d->endPoint->invokeRemoteProperty(metaIndex, arg, pType, c);
                //wrap result for client
                if (pType != 0) {
                    QByteArray buffer;
                    QDataStream stream(&buffer, QIODevice::ReadWrite);
                    QMetaType::save(stream, pType, result.constData());
                    stream.device()->seek(0);
                    QMetaType::load(stream, pType, a[0]);
                } else {
                    if (a[0]) *reinterpret_cast< QVariant*>(a[0]) = result;
                }
            } else {
                d->endPoint->invokeRemoteProperty(metaIndex, arg, pType, c);
            }
        }
        id-=pCount;
    } else if ( c == QMetaObject::QueryPropertyDesignable
            || c == QMetaObject::QueryPropertyScriptable
            || c == QMetaObject::QueryPropertyStored
            || c == QMetaObject::QueryPropertyEditable
            || c == QMetaObject::QueryPropertyUser )
    {
        //Nothing to do?
        //These values are part of the transferred meta object already
    } else {
        //TODO
        qWarning() << "MetaCall type" << c << "not yet handled";
    }
    return id;
}

void *QServiceProxy::qt_metacast(const char* className)
{
    if (!className) return 0;
    //this object should not be castable to anything but it's super type
    return QServiceProxyBase::qt_metacast(className);
}

class QServiceProxyBasePrivate
{
public:
    QServiceProxy *p;
    QMetaObject* meta;
    ObjectEndPoint* endPoint;
    int ipcfailure;
    int timerId;
};

QServiceProxyBase::QServiceProxyBase(ObjectEndPoint *endpoint, QObject *parent)
    : QObject(parent), d(0)
{

    d = new QServiceProxyBasePrivate();
    d->meta = 0;
    d->endPoint = endpoint;
    d->ipcfailure = -1;
    d->timerId = startTimer(1000);

    QMetaObjectBuilder sup;
    sup.setClassName("QServiceProxyBase");
    QMetaMethodBuilder b = sup.addSignal("errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)");
    d->ipcfailure = b.index();
    d->meta = sup.toMetaObject();

}

QServiceProxyBase::~QServiceProxyBase()
{
    if (d->meta)
        qFree(d->meta);
    delete d;
}

void QServiceProxyBase::connectNotify(const char *signal)
{
    if (d->timerId > 0 && strcmp(SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)), signal) == 0) {
        killTimer(d->timerId);
        d->timerId = -1;
    }
}

void QServiceProxyBase::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == d->timerId) {
        qWarning() << this << "Someone is using SFW incorrectly. No one connected to errorUnrecoverableIPCFault for class" << this->metaObject()->className() << "in" << qApp->applicationFilePath();
        killTimer(d->timerId);
        d->timerId = -1;
    } else {
        QObject::timerEvent(e);
    }
}

//provide custom Q_OBJECT implementation
const QMetaObject* QServiceProxyBase::metaObject() const
{
    return d->meta;
}

void *QServiceProxyBase::qt_metacast(const char* className)
{
    if (!className) return 0;
    //this object should not be castable to anything but QObject
    return QObject::qt_metacast(className);
}

int QServiceProxyBase::qt_metacall(QMetaObject::Call c, int id, void **a)
{
    id = QObject::qt_metacall(c, id, a);
    if (id < 0 || !d->meta)
        return id;

    if (c == QMetaObject::InvokeMetaMethod) {
        // ipcfailure is the local offset, so is id
        if (id == d->ipcfailure) {
            QMetaObject::activate(this, d->meta, d->ipcfailure, a);
        }

        const int mcount = d->meta->methodCount() - d->meta->methodOffset();
        id-=mcount;
    }
    return id;
}

QT_END_NAMESPACE
