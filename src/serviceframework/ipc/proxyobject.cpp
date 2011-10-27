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
****************************************************************************/

#include "proxyobject_p.h"
#include <private/qmetaobjectbuilder_p.h>
#include "qremoteserviceregisterentry_p.h"

#include <QDebug>

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
        QMetaObjectBuilder sup;

        QMetaObject *remote = builder.toMetaObject();

        builder.setSuperClass(QServiceProxyBase::metaObject());

        QMetaObject *local = builder.toMetaObject();

        d->remoteToLocal = new int[local->methodCount()];
        d->localToRemote = new int[local->methodCount()];

        for (int i = 0; i < local->methodCount(); i++){
            const QMetaMethod m = local->method(i);
            int r = remote->indexOfMethod(m.signature());
            d->localToRemote[i] = r;
            if (r > 0)
                d->remoteToLocal[r] = i;
        }

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

        const int returnType = QMetaType::type(method.typeName());

        //process arguments
        const QList<QByteArray> pTypes = method.parameterTypes();
        const int pTypesCount = pTypes.count();
        QVariantList args ;
        if (pTypesCount > 10) {
            qWarning() << "Cannot call" << method.signature() << ". More than 10 parameter.";
            return id;
        }
        for (int i=0; i < pTypesCount; i++) {
            const QByteArray& t = pTypes[i];

            int variantType = QVariant::nameToType(t);
            if (variantType == QVariant::UserType)
                variantType = QMetaType::type(t);

            if (t == "QVariant") {  //ignore whether QVariant is declared as metatype
                args << *reinterpret_cast<const QVariant(*)>(a[i+1]);
            } else if ( variantType == 0 ){
                qWarning("%s: argument %s has unknown type. Use qRegisterMetaType to register it.",
                        method.signature(), t.data());
                return id;
            } else {
                args << QVariant(variantType, a[i+1]);
            }
        }

        //QVariant looks the same as Void type. we need to distinguish them
        if (returnType == QMetaType::Void && strcmp(method.typeName(),"QVariant") ) {
            d->endPoint->invokeRemote(d->localToRemote[metaIndex], args, returnType);
        } else {
            //TODO: ugly but works
            //add +1 if we have a variant return type to avoid triggering of void
            //code path
            //invokeRemote() parameter list needs review
            QVariant result = d->endPoint->invokeRemote(d->localToRemote[metaIndex], args,
                    returnType==0 ? returnType+1: returnType);
            if (result.type() != QVariant::Invalid){
                if (returnType != 0 && strcmp(method.typeName(),"QVariant")) {
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
            int pType = property.type();
            if (pType == QVariant::UserType)
                pType = QMetaType::type(property.typeName());

            QVariant arg;
            if ( c == QMetaObject::WriteProperty ) {
                if (pType == QVariant::Invalid && QByteArray(property.typeName()) == "QVariant")
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
};

QServiceProxyBase::QServiceProxyBase(ObjectEndPoint *endpoint, QObject *parent)
    : QObject(parent), d(0)
{

    d = new QServiceProxyBasePrivate();
    d->meta = 0;
    d->endPoint = endpoint;
    d->ipcfailure = -1;

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
