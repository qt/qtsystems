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

#include "qserviceinterfacedescriptor_p.h"
#ifndef QT_NO_DATASTREAM
#include <qdatastream.h>
#endif

#include <QDebug>
#include <QStringList>

QT_BEGIN_NAMESPACE

/*!
    \class QServiceInterfaceDescriptor
    \ingroup servicefw
    \inmodule QtServiceFramework
    \brief The QServiceInterfaceDescriptor class identifies a service implementation.

    A service can implement multiple interfaces and each interface can have multiple implementations.
    The QServiceInterfaceDescriptor class enscapsulates this information, as illustrated
    by the diagram below.

    \image qserviceinterfacedescriptor.png Service-Interface-Implementation

    The major version tag indicates the interface version and the minor version tag identifies the implementation
    version. Subsequent versions of the same interface must be binary compatible to previous versions
    of the same interface.

    In the above example service A and B implement the interface \e com.nokia.qt.x.
    In fact Service A provides two different implementations for the very same interface.
    This is indicated by the changed minor version number. Although Service B is
    using the same interface it's implementation actually utilizes the second version of
    the interface \e com.nokia.qt.x. Binary compatibility guarantees that clients
    who know version 1 can utilize version 2. If an existing interface has to be changed
    in a non-compatible way a new interface (name) is required.

    \section1 Namespaces

    A QServiceInterfaceDescriptor (the quadruble of service name,
    interface name, interface version and implementation version) uniquely
    identifies a service implementation on a device. Interface names follow
    the java namespace convention.

    The namespace \e com.nokia.qt.* is reserved for future Qt development.

    \sa QServiceFilter, QServiceManager
*/

/*!
    \enum QServiceInterfaceDescriptor::Attribute

    This enum describes the possible attribute types which can be attached
    to a QServiceInterfaceDescriptor.

    \value      Capabilities            The capabilities attribute is a QStringList and
                                        describes the capabilities that a service client
                                        would require to use the service if capability
                                        checks are enforced.
    \value      Location                This attribute points to either the location
                                        where the plug-in providing this service is stored or
                                        where the name of the service IPC path is found.
                                        If the service is plug-in based the location is the
                                        name and/or path of the plugin. If the service is
                                        IPC based the location is the name of the socket address.
    \value      ServiceDescription      This attribute provides a general description for
                                        the service.
    \value      InterfaceDescription    This attribute provides a description for the interface
                                        implementation.
    \value      ServiceType             This attribute specifies the QService::Type that the
                                        service is being provided.
*/

/*!
    Creates a new QServiceInterfaceDescriptor.
*/
QServiceInterfaceDescriptor::QServiceInterfaceDescriptor()
    :  d(0)
{
}

/*!
    Destroys the QServiceInterfaceDescriptor object.
*/
QServiceInterfaceDescriptor::~QServiceInterfaceDescriptor()
{
    if (d)
        delete d;
}

/*!
    Creates a copy of QServiceInterfaceDescriptor contained in \a other.
*/
QServiceInterfaceDescriptor::QServiceInterfaceDescriptor(const QServiceInterfaceDescriptor& other)
    : d(0)
{
    (*this) = other; //use assignment operator
}

/*!
    \fn  QServiceInterfaceDescriptor& QServiceInterfaceDescriptor::operator=(const QServiceInterfaceDescriptor& other)

    Copies the content of the QServiceInterfaceDescriptor object contained
    in \a other into this one.
*/
QServiceInterfaceDescriptor& QServiceInterfaceDescriptor::operator=(const QServiceInterfaceDescriptor& other)
{
    if (&other == this)
        return *this;

    if ( !other.isValid() ) {
        if (d)
            delete d;
        d = 0;
        return *this;
    }

    if (!d)
        d = new QServiceInterfaceDescriptorPrivate;

    (*d) = *(other.d);
    return *this;
}

/*!
    \fn bool QServiceInterfaceDescriptor::operator==(const QServiceInterfaceDescriptor& other) const

    Compares a QServiceInterfaceDescriptor to \a other. Returns true if they
    are equal and false otherwise.
*/
bool QServiceInterfaceDescriptor::operator==(const QServiceInterfaceDescriptor& other) const
{
    if (isValid() ^ other.isValid())
        return false;

    if (!d)
        return true;

    if ((*d) == *(other.d))
        return true;
    return false;
}

/*!
    \fn bool QServiceInterfaceDescriptor::operator!=(const QServiceInterfaceDescriptor& other) const

    Compares a QServiceInterfaceDescriptor to \a other. Returns true
    if they are not equal and false otherwise.
*/

/*!
    \fn bool QServiceInterfaceDescriptor::isValid() const

    Returns true if this descriptor is valid; otherwise returns false.
*/
bool QServiceInterfaceDescriptor::isValid() const
{
    return d ? true : false;
}

/*!
    \fn  bool QServiceInterfaceDescriptor::scope() const

    Returns true if this implementation is provided for all users on the system.

    \sa QService::Scope
*/
QService::Scope QServiceInterfaceDescriptor::scope() const
{
    return d ? d->scope : QService::UserScope;
}

/*!
    \fn  QString QServiceInterfaceDescriptor::serviceName() const

    Returns the name of service that provides this implementation.
*/
QString QServiceInterfaceDescriptor::serviceName() const
{
    return d ? d->serviceName : QString();
}

/*!
    \fn  QString QServiceInterfaceDescriptor::interfaceName() const

    Returns the name of the interface that is implemented.
*/
QString QServiceInterfaceDescriptor::interfaceName() const
{
    return d ? d->interfaceName : QString();
}

/*!
    \fn  int QServiceInterfaceDescriptor::majorVersion() const

    Returns the version of the interface.

    Subsequent versions of an interface are binary compatible
    to previous versions of the same interface. If an interface
    is broken it must use a new interface name.
*/
int QServiceInterfaceDescriptor::majorVersion() const
{
    return d ? d->major : -1;
}

/*!
    \fn  int QServiceInterfaceDescriptor::minorVersion() const

    Returns the version of the implementation.
*/
int QServiceInterfaceDescriptor::minorVersion() const
{
    return d ? d->minor : -1;
}

/*!
    \fn  QVariant QServiceInterfaceDescriptor::attribute(QServiceInterfaceDescriptor::Attribute which) const

    Returns the value for the attribute \a which; otherwise returns
    an invalid QVariant.
*/
QVariant QServiceInterfaceDescriptor::attribute(QServiceInterfaceDescriptor::Attribute which) const
{
    if (d)
        return d->attributes.value(which);
    return QVariant();
}

/*!
    \fn  QString QServiceInterfaceDescriptor::customAttribute(const QString& which) const

    Returns the value for the custom attribute \a which; otherwise
    returns a null string.
*/
QString QServiceInterfaceDescriptor::customAttribute(const QString& which) const
{
    if (d)
        return d->customAttributes[which];
    return QString();
}

/*!
  Returns a list of custom attributes attached to the service.
 */
QStringList QServiceInterfaceDescriptor::customAttributes() const
{
    if (d)
        return d->customAttributes.keys();
    return QStringList();
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QServiceInterfaceDescriptor &desc)
{
    if (desc.isValid()) {
        QString serviceInterface = QString(QLatin1String("%1 %2.%3")).arg(desc.interfaceName())
                .arg(desc.majorVersion() < 0 ? '?' : desc.majorVersion())
                .arg(desc.minorVersion() < 0 ? '?' : desc.minorVersion());
        dbg.nospace() << "QServiceInterfaceDescriptor(";
        dbg.nospace() << "service=" << desc.serviceName() << ", ";
        dbg.nospace() << "interface=" << serviceInterface;
        dbg.nospace() << ")";
    } else {
        dbg.nospace() << "QServiceInterfaceDescriptor(invalid)";
    }
    return dbg.space();
}
#endif

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &out, const QServiceInterfaceDescriptor::Attribute &k)
{
    out << qint8(k);
    return out;
}

QDataStream &operator>>(QDataStream &in, QServiceInterfaceDescriptor::Attribute &k)
{
    quint8 key;
    in >> key;
    k = (QServiceInterfaceDescriptor::Attribute)key;
    return in;
}
/*!
    \fn QDataStream &operator<<(QDataStream &out, const QServiceInterfaceDescriptor &dc)
    \relates QServiceInterfaceDescriptor

    Writes service interface descriptor \a dc to the stream \a out and returns a reference
    to the stream.
*/

QDataStream &operator<<(QDataStream &out, const QServiceInterfaceDescriptor &dc)
{
    const quint32 magicNumber = 0x77AFAFA;
    const quint16 majorVersion = 1;
    const quint16 minorVersion = 0;
    const qint8 valid = dc.isValid();
    out << magicNumber << majorVersion << minorVersion;
    out << valid;
    if (valid) {
       out << dc.d->serviceName;
       out << dc.d->interfaceName;
       out << dc.d->major;
       out << dc.d->minor;
       out << dc.d->attributes;
       out << dc.d->customAttributes;
       out << (qint8)dc.d->scope;
    }
    return out;
}

/*!
    \fn QDataStream &operator>>(QDataStream &in, QServiceInterfaceDescriptor &dc)
    \relates QServiceInterfaceDescriptor

    Reads a service interface descriptor into \a dc from the stream \a in and returns a
    reference to the stream.
*/
QDataStream &operator>>(QDataStream &in, QServiceInterfaceDescriptor &dc)
{
    const quint32 magicNumber = 0x77AFAFA;
    quint32 storedMagicNumber;
    in >> storedMagicNumber;
    if (storedMagicNumber != magicNumber) {
        qWarning() << "Datastream doesn't provide searialized QServiceInterfaceDescriptor";
        return in;
    }

    const quint16 currentMajorVersion = 1;
    quint16 majorVersion = 0;
    quint16 minorVersion = 0;

    in >> majorVersion >> minorVersion;
    if (majorVersion != currentMajorVersion) {
        qWarning() << "Unknown serialization format for QServiceInterfaceDescriptor.";
        return in;
    }
    //Allow all minor versions.

    qint8 valid;
    in >> valid;
    if (valid) {
        if (!dc.isValid())
            dc.d = new QServiceInterfaceDescriptorPrivate;
        in >> dc.d->serviceName;
        in >> dc.d->interfaceName;
        in >> dc.d->major;
        in >> dc.d->minor;
        in >> dc.d->attributes;
        in >> dc.d->customAttributes;
        in >> valid;
        dc.d->scope = (QService::Scope) valid;
    } else { //input stream contains invalid descriptor
        //use assignment operator
        dc = QServiceInterfaceDescriptor();
    }

    return in;
}
#endif //QT_NO_DATASTREAM



QT_END_NAMESPACE

