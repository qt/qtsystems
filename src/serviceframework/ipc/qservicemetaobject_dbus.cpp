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
****************************************************************************/

#include "qservicemetaobject_dbus_p.h"
#include <private/qmetaobjectbuilder_p.h>
#include "qsignalintercepter_p.h"
#include <QDebug>

#include <stdlib.h>

QT_BEGIN_NAMESPACE

class ServiceMetaSignalIntercepter : public QSignalIntercepter
{

public:
    ServiceMetaSignalIntercepter(QObject* sender, const QByteArray& signal, QServiceMetaObjectDBus* parent)
        : QSignalIntercepter(sender, signal, parent), serviceDBus(parent)
    {

    }

    void setMetaIndex(int index)
    {
        metaIndex = index;
    }

protected:
    void activated( const QList<QVariant>& args )
    {
        serviceDBus->activateMetaSignal(metaIndex, args);
    }
private:
    QServiceMetaObjectDBus* serviceDBus;
    int metaIndex;
};


class QServiceMetaObjectDBusPrivate
{
public:
    QObject *service;
    const QMetaObject *serviceMeta;
    const QMetaObject *dbusMeta;
};

QServiceMetaObjectDBus::QServiceMetaObjectDBus(QObject* service, bool signalsObject)
    : QDBusAbstractAdaptor(service)
{
    // Register our DBus custom type object
    qRegisterMetaType<QServiceUserTypeDBus>();
    qDBusRegisterMetaType<QServiceUserTypeDBus>();

    // Generate our DBus meta object
    d = new QServiceMetaObjectDBusPrivate();
    d->service = service;
    d->serviceMeta = service->metaObject();
    d->dbusMeta = dbusMetaObject(signalsObject);

    // Relay signals from the service to the constructed DBus meta object
    connectMetaSignals(signalsObject);
}

QServiceMetaObjectDBus::~QServiceMetaObjectDBus()
{
    if (d->dbusMeta)
        free(const_cast<QMetaObject*>(d->dbusMeta));

    delete d;
}

/*!
    Relays all signals from the service object to the converted DBus service adaptor so
    that when a service signal is emitted the DBus object will emit the counterpart signal
*/
void QServiceMetaObjectDBus::connectMetaSignals(bool signalsObject) {
    if (!signalsObject) {
        // Automatically relay signals from service object to adaptor
        setAutoRelaySignals(true);

        // Connect signals with custom arguments
        int methodCount = d->serviceMeta->methodCount();
        for (int i = 0; i < methodCount; i++) {
            QMetaMethod mm = d->serviceMeta->method(i);


            if (mm.methodType() == QMetaMethod::Signal) {
                QByteArray sig(mm.methodSignature());
                bool customType = false;
                const QList<QByteArray> pTypes = mm.parameterTypes();
                const int pTypesCount = pTypes.count();

                // Ignore all QObject calls
                const QMetaObject *mo = QObject::metaObject();
                int qobjectIndex = mo->indexOfMethod(sig);
                if (qobjectIndex >= 0)
                    continue;

                // Detects custom types as passed arguments
                for (int arg = 0; arg < pTypesCount; arg++) {
                    const QByteArray& type = pTypes[arg];
                    int variantType = QMetaType::type(type);
                    if (variantType >= QMetaType::User || variantType == QMetaType::QVariant) {
                        sig.replace(QByteArray(type), QByteArray("QDBusVariant"));
                        customType = true;
                    }
                }

                // Connects the service signal to the corresponding DBus service signal
                if (customType) {
                    QByteArray signal = mm.methodSignature();
                    ServiceMetaSignalIntercepter *intercept =
                        new ServiceMetaSignalIntercepter(d->service, "2"+signal, this);
                    intercept->setMetaIndex(i);
                }
            }
        }
    }
}

/*!
    Relays the activation to the DBus object signal with the given id and arguments list when the
    intercepter for signals with custom arguments has been activated. This bypasses the metacall
    which usually does the relaying for signals with standard arguments since no pre-connection
    conversions are required.
*/
void QServiceMetaObjectDBus::activateMetaSignal(int id, const QVariantList& args)
{
    QMetaMethod method = d->serviceMeta->method(id);

    // Converts the argument list to values supported by the QtDBus type system
    QVariantList convertedList = args;
    QByteArray sig(method.methodSignature());
    QList<QByteArray> params = method.parameterTypes();

    for (int i = 0; i < params.size(); i++) {
        QVariant dbusVariant = args[i];

        // Convert custom types
        const QByteArray& type = params[i];
        int variantType = QMetaType::type(type);
        if (variantType >= QMetaType::User || variantType == QMetaType::QVariant) {
            if (variantType != QMetaType::QVariant) {
                // Wrap custom types in a QDBusVariant of the type name and
                // a buffer of its variant-wrapped data
                QByteArray buffer;
                QDataStream stream(&buffer, QIODevice::ReadWrite | QIODevice::Append);
                stream << args[i];

                QServiceUserTypeDBus customType;
                customType.typeName = type;
                customType.variantBuffer = buffer;

                QDBusVariant replacement(QVariant::fromValue(customType));
                convertedList.replace(i, QVariant::fromValue(replacement));
            }

            sig.replace(QByteArray(type), QByteArray("QDBusVariant"));
        }
    }

    // Activate the DBus object signal call
    const int numArgs = convertedList.size();
    QVarLengthArray<void *, 32> a( numArgs+1 );
    a[0] = 0;

    const QList<QByteArray> pTypes = method.parameterTypes();
    for ( int arg = 0; arg < numArgs; ++arg ) {
        if (pTypes.at(arg) == "QVariant")
            a[arg+1] = (void *)&( convertedList[arg] );
        else
            a[arg+1] = (void *)( convertedList[arg].data() );
    }

    int dbusIndex = d->dbusMeta->indexOfSignal(sig);
    QMetaObject::activate(this, dbusIndex, a.data());
}


/*!
    Build a metaobject that represents the service object as a valid service that
    satisfies the QtDBus type system.
*/
const QMetaObject* QServiceMetaObjectDBus::dbusMetaObject(bool signalsObject) const
{
    // Create a meta-object to represent all the contents of our service on DBus
    QMetaObjectBuilder builder;

    builder.setClassName(d->serviceMeta->className());
    builder.setSuperClass(d->serviceMeta->superClass()); // needed?

    const QMetaObject* mo = d->serviceMeta;
    while (mo && strcmp(mo->className(), "QObject")) {
        // Add our methods, signals and slots
        for (int i = mo->methodOffset(); i < mo->methodCount(); i++) {
            QMetaMethod mm = mo->method(i);

            if (signalsObject && mm.methodType() != QMetaMethod::Signal)
                continue;

            // Convert QVariant and custom return types to QDBusVariants
            QByteArray ret(mm.typeName());
            const QByteArray& type = mm.typeName();
            int variantType = QMetaType::type(type);
            if (variantType >= QMetaType::User || variantType == QMetaType::QVariant) {
                ret = QByteArray("QDBusVariant");
            }

            // Convert QVariant and custom argument types to QDBusVariants
            QByteArray sig(mm.methodSignature());
            const QList<QByteArray> pTypes = mm.parameterTypes();
            const int pTypesCount = pTypes.count();
            for (int i=0; i < pTypesCount; i++) {
                const QByteArray& type = pTypes[i];
                int variantType = QMetaType::type(type);
                if (variantType >= QMetaType::User || variantType == QMetaType::QVariant) {
                    sig.replace(QByteArray(type), QByteArray("QDBusVariant"));
                }
            }

            // Add a MetaMethod with converted signature to our builder
            QMetaMethodBuilder method;
            switch (mm.methodType()) {
                case QMetaMethod::Method:
                    method = builder.addMethod(sig);
                    break;
                case QMetaMethod::Slot:
                    method = builder.addSlot(sig);
                    break;
                case QMetaMethod::Signal:
                    method = builder.addSignal(sig);
                    break;
                default:
                    break;
            }

            // Make sure our built MetaMethod is identical, excluding conversion
            method.setReturnType(ret);
            method.setParameterNames(mm.parameterNames());
            method.setTag(mm.tag());
            method.setAccess(mm.access());
            method.setAttributes(mm.attributes());
        }

        if (signalsObject)
            return builder.toMetaObject();

        // Add our property accessor methods
        // NOTE: required because read/reset properties over DBus require adaptors
        //       otherwise a metacall won't be invoked as QMetaObject::ReadProperty
        //       or QMetaObject::ResetProperty
        QMetaMethodBuilder readProp;
        readProp = builder.addMethod(QByteArray("propertyRead(QString)"));
        readProp.setReturnType(QByteArray("QDBusVariant"));
        QList<QByteArray> params;
        params << QByteArray("name");
        readProp.setParameterNames(params);

        QMetaMethodBuilder resetProp;
        resetProp = builder.addMethod(QByteArray("propertyReset(QString)"));
        QList<QByteArray> paramsReset;
        paramsReset << QByteArray("name");
        resetProp.setParameterNames(paramsReset);


        // Add our properties/enums
        int propCount = d->serviceMeta->propertyCount();
        for (int i=0; i<propCount; i++) {
            QMetaProperty mp = d->serviceMeta->property(i);

            QMetaPropertyBuilder property = builder.addProperty(mp.name(), mp.typeName());
            property.setReadable(mp.isReadable());
            property.setWritable(mp.isWritable());
            property.setResettable(mp.isResettable());
            property.setDesignable(mp.isDesignable());
            property.setScriptable(mp.isScriptable());
            property.setStored(mp.isStored());
            property.setEditable(mp.isEditable());
            property.setUser(mp.isUser());
            property.setStdCppSet(mp.hasStdCppSet());
            property.setEnumOrFlag(mp.isEnumType());
        }

        // Needs Enumerators support when QtDBus supports

        mo = mo->superClass();
    }

    // return our constructed dbus metaobject
    return builder.toMetaObject();
}

/*!
    Provide custom Q_OBJECT implementation of the metaObject
*/
const QMetaObject* QServiceMetaObjectDBus::metaObject() const
{
    // Provide our construected DBus service metaobject
    return d->dbusMeta;
}

/*!
    Overrides metacall which relays the DBus service call to the actual service
    meta object. Positive return will indicate that the metacall was unsuccessful
*/
int QServiceMetaObjectDBus::qt_metacall(QMetaObject::Call c, int id, void **a)
{
    int dbusIndex = id;

    // Relay the meta-object call to the service object
    if (c == QMetaObject::InvokeMetaMethod) {
        // METHOD CALL
        QMetaMethod method = d->dbusMeta->method(id);

        const bool isSignal = (method.methodType() == QMetaMethod::Signal);

        ///////////////////// CHECK SPECIAL PROPERTY ///////////////////////
        // This is required because property READ/RESET doesn't function
        // as desired over DBus without the use of adaptors. Temporary
        // methods propertyRead and propertyReset are added to the published
        // meta object and relay the correct property call
        QString methodName(QLatin1String(method.methodSignature().constData()));
        methodName.truncate(methodName.indexOf(QLatin1String("(")));

        if (methodName == QLatin1String("propertyRead")) {
            QString propertyName = *reinterpret_cast<QString*>(a[1]);
            int index = d->dbusMeta->indexOfProperty(propertyName.toLatin1().constData());
            id = qt_metacall(QMetaObject::ReadProperty, index, a);
            return id;

        } else if (methodName == QLatin1String("propertyReset")) {
            QString propertyName = *reinterpret_cast<QString*>(a[1]);
            int index = d->dbusMeta->indexOfProperty(propertyName.toLatin1().constData());
            id = qt_metacall(QMetaObject::ResetProperty, index, a);
            return id;
        }
        ////////////////////////////////////////////////////////////////////

        // Find the corresponding signature to our service object
        QByteArray sig(method.methodSignature());
        int count = methodName.size() + 1;
        const QList<QByteArray> xTypes = method.parameterTypes();
        const int xTypesCount = xTypes.count();
        for (int i=0; i < xTypesCount; i++) {
            const QByteArray& t = xTypes[i];
            int variantType = QMetaType::type(t);

            // Check for QVariants or custom types, represented as QDBusVariants
            if (t == "QDBusVariant") {
                // Convert QDBusVariant to QVariant
                QVariant convert = QVariant(variantType, a[i+1]);
                QDBusVariant dbusVariant = qvariant_cast<QDBusVariant>(convert);
                QVariant variant = dbusVariant.variant();

                // Is a custom argument if castable to QDBusArgument
                bool hasCustomType = variant.canConvert<QDBusArgument>();
                QByteArray replacement("QVariant");

                // Custom types will have QDBusArgument streaming operators for
                // the QServiceUserTypeDBus object. Extract the real type name
                // and buffered QVariant from this
                if (hasCustomType) {
                    QDBusArgument demarshall = variant.value<QDBusArgument>();
                    QServiceUserTypeDBus userType = qdbus_cast<QServiceUserTypeDBus>(demarshall);
                    *reinterpret_cast<QVariant*>(a[i+1]) = userType.variantBuffer;

                    replacement = userType.typeName;
                }

                // Replace "QDBusVariant" with "QVariant" or custom type name
                sig.replace(count, 12, replacement);
                count += replacement.size();

            } else {
                // Supported type so skip this paramater
                count += t.size();
            }

            // Skips the comma if not last parameter
            if (i < xTypesCount)
                count += 1;
        }

        // Find the corresponding method metaindex to our service object
        id = d->serviceMeta->indexOfMethod(sig);
        QMetaMethod mm = d->serviceMeta->method(id);

        const QList<QByteArray> pTypes = mm.parameterTypes();
        const int pTypesCount = pTypes.count();

        const char* typeNames[] = {0,0,0,0,0,0,0,0,0,0};
        const void* params[] = {0,0,0,0,0,0,0,0,0,0};
        bool hasCustomType = false;

        // Process arguments
        for (int i=0; i < pTypesCount; i++) {
            const QByteArray& t = pTypes[i];
            int variantType = QMetaType::type(t);

            if (variantType >= QMetaType::User) {
                // Custom argument
                QVariant convert = QVariant(QVariant::ByteArray, a[i+1]);
                QByteArray buffer = convert.toByteArray();
                QDataStream stream(&buffer, QIODevice::ReadWrite);

                // Load our buffered variant-wrapped custom type
                QVariant *customType = new QVariant(variantType, (const void*)0);
                QMetaType::load(stream, QMetaType::QVariant, customType);

                typeNames[i] = customType->typeName();
                params[i] = customType->constData();
                hasCustomType = true;
            } else {
                typeNames[i] = t.constData();
                params[i] = a[i+1];
            }
        }

        // Check if this is a signal emit and activate
        if (isSignal) {
            QMetaObject::activate(this, dbusIndex, a);
            return id;
        }

        // Check for custom return types and make the metacall
        const QByteArray& type = mm.typeName();
        int retType = QMetaType::type(type);
        if (retType >= QMetaType::User) {
            // Invoke the object method directly for custom return types
            bool result = false;
            QVariant returnValue = QVariant(retType, (const void*)0);
            QGenericReturnArgument ret(type, returnValue.data());
            result = mm.invoke(d->service, ret,
                    QGenericArgument(typeNames[0], params[0]),
                    QGenericArgument(typeNames[1], params[1]),
                    QGenericArgument(typeNames[2], params[2]),
                    QGenericArgument(typeNames[3], params[3]),
                    QGenericArgument(typeNames[4], params[4]),
                    QGenericArgument(typeNames[5], params[5]),
                    QGenericArgument(typeNames[6], params[6]),
                    QGenericArgument(typeNames[7], params[7]),
                    QGenericArgument(typeNames[8], params[8]),
                    QGenericArgument(typeNames[9], params[9]));

            if (result) {
                // Wrap custom return type in a QDBusVariant of the type
                // and a buffer of its variant-wrapped data
                QByteArray buffer;
                QDataStream stream(&buffer, QIODevice::WriteOnly | QIODevice::Append);
                stream << returnValue;

                QServiceUserTypeDBus customType;
                customType.typeName = type;
                customType.variantBuffer = buffer;

                QDBusVariant replacement(QVariant::fromValue(customType));
                *reinterpret_cast<QDBusVariant*>(a[0]) = replacement;

                // Return negative id to say metacall was handled externally
                return -1;
            }

        } else {
            // Void or standard return types
            if (hasCustomType == true) {
                // Invoke the object method directly for custom arguments
                bool result = false;
                result = mm.invoke(d->service,
                         QGenericArgument(typeNames[0], params[0]),
                         QGenericArgument(typeNames[1], params[1]),
                         QGenericArgument(typeNames[2], params[2]),
                         QGenericArgument(typeNames[3], params[3]),
                         QGenericArgument(typeNames[4], params[4]),
                         QGenericArgument(typeNames[5], params[5]),
                         QGenericArgument(typeNames[6], params[6]),
                         QGenericArgument(typeNames[7], params[7]),
                         QGenericArgument(typeNames[8], params[8]),
                         QGenericArgument(typeNames[9], params[9]));
                if (result) {
                    // Return negative id to say metacall was handled externally
                    return -1;
                }
            } else {
                // Relay standard metacall to service object
                id = d->service->qt_metacall(c, id, a);
            }
        }

    } else {
        // PROPERTY CALL

        // Find the corresponding property metaindex of our service object
        QMetaProperty property = d->dbusMeta->property(id);
        QByteArray name(property.name());
        id = d->serviceMeta->indexOfProperty(name);

        if (c == QMetaObject::ReadProperty) {
            // Convert to DBusVariant
            QMetaProperty mp = d->serviceMeta->property(id);
            QVariant value = mp.read(d->service);
            QDBusVariant replacement(value);
            *reinterpret_cast<QDBusVariant*>(a[0]) = replacement;

            // Return negative id to say metacall was handled externally
            return -1;
        }

        // Metacall our service object property
        id = d->service->qt_metacall(c, id, a);
    }

    return id;
}

void *QServiceMetaObjectDBus::qt_metacast(const char* className)
{
    if (!className) return 0;
    //this object should not be castable to anything but QObject
    return QObject::qt_metacast(className);
}

QDBusArgument &operator<<(QDBusArgument &argument, const QServiceUserTypeDBus &myType)
{
    argument.beginStructure();
    argument << myType.typeName << myType.variantBuffer;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QServiceUserTypeDBus &myType)
{
    argument.beginStructure();
    argument >> myType.typeName >> myType.variantBuffer;
    argument.endStructure();
    return argument;
}

QT_END_NAMESPACE
