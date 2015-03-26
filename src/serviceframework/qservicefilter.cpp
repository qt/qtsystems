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

#include <QRegExp>
#include <QStringList>
#include <QDebug>
#ifndef QT_NO_DATASTREAM
#include <qdatastream.h>
#endif

#include "qservicefilter.h"

QT_BEGIN_NAMESPACE

class QServiceFilterPrivate
{
public:
    QString interface;
    QString service;
    int majorVersion;
    int minorVersion;
    QServiceFilter::VersionMatchRule matchingRule;
    QHash<QString,QString> customAttributes;
    QStringList capabilities;
    QServiceFilter::CapabilityMatchRule capMatchingRule;
};


/*!
    \class QServiceFilter

    \ingroup servicefw
    \inmodule QtServiceFramework
    \brief The QServiceFilter class defines criteria for defining a sub-set of
    all available services.

    A QServiceFilter can be used to constrain the number of services when searching
    for services. Only those services that match all filter criteria are returned
    by \l QServiceManager::findInterfaces().


    \sa QServiceInterfaceDescriptor, QServiceManager
*/

/*!
    \enum QServiceFilter::VersionMatchRule

    This enum describes how interface version matching is performed.

    \value ExactVersionMatch    The filter matches any interface implementation that implements
                                the exact version provided.
    \value MinimumVersionMatch  The filter matches any interface implementation that implements
                                either the given major/minor version or any subsequent version.
*/

/*!
    \enum QServiceFilter::CapabilityMatchRule

    This enum describes the capability/permission matching rules. Some platforms restrict what services clients
    can access using "capabilities" or permissions. Services with more capabilities require
    more privileged clients. Platforms without capabilities may ignore this type of matching
    rule as the default behavior is to ignore any capability restrictions.

    This is a brief example. Assuming that the system knows the services S1 - S6 which require capabilities as stated below:
    \table
        \header     \li Service  \li Required capabilities
        \row        \li S1       \li \{\}
        \row        \li S2       \li \{A\}
        \row        \li S3       \li \{A,B\}
        \row        \li S4       \li \{A,B,C,D\}
        \row        \li S5       \li \{A,D\}
        \row        \li S6       \li \{F\}
    \endtable

    The matching rules would apply as follows:

    \table
        \header     \li Matching rule    \li Filter's capabilities    \li Matching services
        \row        \li MatchLoadable    \li \{\}                       \li S1
        \row        \li MatchLoadable    \li \{A\}                      \li S1, S2
        \row        \li MatchLoadable    \li \{A,B,C\}                  \li S1, S2, S3
        \row        \li MatchMinimum     \li \{\}                       \li S1, S2, S3, S4, S5, S6
        \row        \li MatchMinimum     \li \{A\}                      \li S2, S3, S4, S5
        \row        \li MatchMinimum     \li \{A,B,C\}                  \li S4
    \endtable

    \value MatchMinimum     The filter matches any service that requires at least the given
                            filter capabilities. This may mean that the returned services
                            may require more capabilities than the specified ones.
                            Such a search is equivalent to a wildcard match if the passed filter's capability list is empty. In mathematical set notation
                            this rule is equivalent to Cap\sub{(Filter)} \\ Cap\sub{(Service)} = {}. This is the default matching rule.
    \value MatchLoadable    The filter matches any service that could be loaded by the client.
                            Using this matching rule guarantees that the returned services do not
                            require more capabilites than specified by this rule. It includes services
                            with no capability requirements. If this rule
                            is provided alongside an empty capability search list the returned
                            services do not require any capabilities and thus can be accessed
                            by any client. The equivalent set notation is Cap\sub{(Service)} \\ Cap\sub{(Filter)} = {}.
*/

/*!
    Creates a new filter object that matches all service implementations.
*/
QServiceFilter::QServiceFilter()
{
    d = new QServiceFilterPrivate();
    d->majorVersion = -1;
    d->minorVersion = -1;
    d->matchingRule = QServiceFilter::MinimumVersionMatch;
    d->capMatchingRule = QServiceFilter::MatchMinimum;
}

/*!
    Creates a copy of QServiceFilter object contained in \a other.
*/
QServiceFilter::QServiceFilter(const QServiceFilter& other)
{
    d = new QServiceFilterPrivate();
    (*this) = other;
}

/*!
    \fn  QServiceFilter::QServiceFilter(const QString& interfaceName, const QString& version, QServiceFilter::VersionMatchRule rule)

    Creates a new filter object that matches all service
    implementations implementing \a interfaceName that match the specified
    \a version using the given \a rule.
*/
QServiceFilter::QServiceFilter(const QString& interfaceName, const QString& version, QServiceFilter::VersionMatchRule rule)
{
    d = new QServiceFilterPrivate();
    d->majorVersion = -1;
    d->minorVersion = -1;
    d->matchingRule = QServiceFilter::MinimumVersionMatch;
    d->capMatchingRule = QServiceFilter::MatchMinimum;
    setInterface(interfaceName, version, rule);
}

/*!
    Destroys this instance of QServiceFilter.
*/
QServiceFilter::~QServiceFilter()
{
    delete d;
}

/*!
    \fn  QServiceFilter& QServiceFilter::operator=(const QServiceFilter& other)

    Copies the content of the QServiceFilter object contained in
    \a other into this one.
*/
QServiceFilter& QServiceFilter::operator=(const QServiceFilter& other)
{
    if (&other == this)
        return *this;

    d->interface = other.d->interface;
    d->service = other.d->service;
    d->majorVersion = other.d->majorVersion;
    d->minorVersion = other.d->minorVersion;
    d->matchingRule = other.d->matchingRule;
    d->customAttributes = other.d->customAttributes;
    d->capabilities = other.d->capabilities;
    d->capMatchingRule = other.d->capMatchingRule;

    return *this;
}

/*!
    \fn  void QServiceFilter::setServiceName(const QString& serviceName)

    The filter only matches implementations which are provided by the service
    specified by \a serviceName.

    If the \a serviceName is empty the filter matches any service.
*/
void QServiceFilter::setServiceName(const QString& serviceName)
{
    d->service = serviceName;
}

/*!
    \fn  void QServiceFilter::setInterface(const QString &interfaceName, const QString& version, QServiceFilter::VersionMatchRule rule)

    Sets the filter to match any interface implementation that implements
    \a interfaceName with version \a version. The version is matched
    according to the given \a rule. If \a version is not set, the filter matches any version of the
    interface implementation.

    This method does nothing if \a version is not a valid version string or
    if \a interfaceName is empty.

    A valid version string has the format x.y whereby x and y are positive integer
    numbers.
*/
void QServiceFilter::setInterface(const QString &interfaceName, const QString& version, QServiceFilter::VersionMatchRule rule)
{
    //unset interface name
    if (interfaceName.isEmpty() && version.isEmpty())
    {
        d->interface = interfaceName;
        d->majorVersion = d->minorVersion = -1;
        d->matchingRule = rule;
        return;
    }

    if (interfaceName.isEmpty()) {
        qWarning() << "Empty interface name. Ignoring filter details";
        return;
    }

    if (version.isEmpty()) {
        d->majorVersion = d->minorVersion = -1;
        d->matchingRule = rule;
        d->interface = interfaceName;
        return;
    }

    // Match x.y as version format.
    // This differs from regex in servicemetadata in that 0.x versions are
    // accepted for the search filter.
    QRegExp rx(QLatin1String("^(0+|[1-9][0-9]*)\\.(0+|[1-9][0-9]*)$"));
    int pos = rx.indexIn(version);
    QStringList list = rx.capturedTexts();
    bool success = false;
    int temp_major = -1;
    int temp_minor = -1;
    if (pos == 0 && list.count() == 3
            && rx.matchedLength() == version.length() )
    {
        temp_major = list[1].toInt(&success);
        if ( success ) {
            temp_minor = list[2].toInt(&success);
       }
    }

    if (success) {
        d->majorVersion = temp_major;
        d->minorVersion = temp_minor;
        d->interface = interfaceName;
        d->matchingRule = rule;
    } else {
        qWarning() << "Invalid version tag" << version << ". Ignoring filter details.";
    }
}

/*!
    \fn  QString QServiceFilter::serviceName() const

    Returns the service name for this filter.

    \sa setServiceName()
*/
QString QServiceFilter::serviceName() const
{
    return d->service;
}

/*!
    \fn  QString QServiceFilter::interfaceName() const

    Returns the interface name for this filter.

    \sa setInterface()
*/
QString QServiceFilter::interfaceName() const
{
    return d->interface;
}

/*!
    \fn  int QServiceFilter::majorVersion() const

    Returns the major interface version for this filter.

    \sa setInterface()
*/
int QServiceFilter::majorVersion() const
{
    return d->majorVersion;
}

/*!
    \fn  int QServiceFilter::minorVersion() const

    Returns the minor interface version for this filter.

    \sa setInterface()
*/
int QServiceFilter::minorVersion() const
{
    return d->minorVersion;
}

/*!
    \fn  void QServiceFilter::setCustomAttribute(const QString& key, const QString& value)

    The filter only matches implementations which have the custom attribute
    \a key with the given \a value. Such constraints are specified via the
    \e{<customproperty>} tag within the service xml.

    \sa customAttribute(), clearCustomAttribute()
*/
void QServiceFilter::setCustomAttribute(const QString& key, const QString& value)
{
    d->customAttributes.insert(key, value);
}

/*!
    \fn  QString QServiceFilter::customAttribute(const QString& key) const

    Returns the value for the custom attribute \a key; otherwise
    returns a null string.

    \sa setCustomAttribute(), clearCustomAttribute()
*/
QString QServiceFilter::customAttribute(const QString& key) const
{
    return d->customAttributes.value(key);
}

/*!
    \fn  void QServiceFilter::clearCustomAttribute(const QString &key)

    Clears the custom attribute \a key from the filter's set of constraints.
    If \a key is empty all custom attributes are cleared.

    \sa setCustomAttribute()
*/
void QServiceFilter::clearCustomAttribute(const QString &key)
{
    if (key.isEmpty())
        d->customAttributes.clear();
    else
        d->customAttributes.remove(key);
}

/*!
    \fn  QServiceFilter::VersionMatchRule QServiceFilter::versionMatchRule() const

    Returns the version match rule for this filter.

    \sa setInterface()
*/
QServiceFilter::VersionMatchRule QServiceFilter::versionMatchRule() const
{
    return d->matchingRule;
}

/*!
    \fn  QList<QString> QServiceFilter::customAttributes() const

    Returns the list of custom keys which have been added to the filter.
*/
QStringList QServiceFilter::customAttributes() const
{
    return d->customAttributes.keys();
}

/*!
    \fn  void QServiceFilter::setCapabilities(QServiceFilter::CapabilityMatchRule rule, const QStringList& capabilities )

    Sets the list of \a capabilities which are used to constrain
    searches for services. The capabilities are matched according
    to the given \a rule.

    \sa capabilities()
*/
void QServiceFilter::setCapabilities(QServiceFilter::CapabilityMatchRule rule, const QStringList& capabilities )
{
    d->capMatchingRule = rule;
    d->capabilities = capabilities;
}

/*!
    \fn  QStringList QServiceFilter::capabilities() const

    Returns the list of capabilities which are used to limit services searches.

    The filter matches any services that requires the given or less
    capabilities and thus enabling clients to query for services
    for which they have the required capabilties.

    \sa setCapabilities(), capabilityMatchRule()
*/
QStringList QServiceFilter::capabilities() const
{
    return d->capabilities;
}

/*!
    Returns the capability matching rule for this filter.

    \sa setCapabilities(), capabilities()
*/
QServiceFilter::CapabilityMatchRule QServiceFilter::capabilityMatchRule() const
{
    return d->capMatchingRule;
}

#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &out, const QServiceFilter &sf)
    \relates QServiceFilter

    Writes service filter \a sf to the stream \a out and returns a reference
    to the stream.
*/
QDataStream &operator<<(QDataStream &out, const QServiceFilter &sf)
{
    const quint32 magicNumber = 0x78AFAFA;
    const qint32 mj = sf.d->majorVersion;
    const qint32 mn = sf.d->minorVersion;
    const qint8 versionrule = (qint32) sf.d->matchingRule;
    const qint8 caprule = (qint8) sf.d->capMatchingRule;
    const quint16 majorVersion = 1;
    const quint16 minorVersion = 0;

    out << magicNumber
        << majorVersion
        << minorVersion
        << sf.d->interface
        << sf.d->service
        << mj
        << mn
        << versionrule
        << sf.d->customAttributes
        << caprule
        << sf.d->capabilities;
    return out;
}

/*!
    \fn QDataStream &operator>>(QDataStream &in, QServiceFilter &sf)
    \relates QServiceFilter

    Reads a service filter into \a sf from the stream \a in and returns a
    reference to the stream.
*/
QDataStream &operator>>(QDataStream &in, QServiceFilter &sf)
{
    const quint32 magicNumber = 0x78AFAFA;
    qint32 mj, mn;
    qint8 versionrule, caprule;

    quint32 storedMagicNumber;
    in >> storedMagicNumber;
    if (storedMagicNumber != magicNumber) {
        qWarning() << Q_FUNC_INFO << "Datastream doesn't provide serialized QServiceFilter";
        return in;
    }

    const quint16 currentMajorVersion = 1;
    quint16 majorVersion = 0;
    quint16 minorVersion = 0;

    in >> majorVersion >> minorVersion;
    if (majorVersion != currentMajorVersion) {
        qWarning() << "Unknown serialization format for QServiceFilter.";
        return in;
    }
    //Allow all minor versions.

    in  >> sf.d->interface
        >> sf.d->service
        >> mj
        >> mn
        >> versionrule
        >> sf.d->customAttributes
        >> caprule
        >> sf.d->capabilities;

    sf.d->majorVersion = mj;
    sf.d->minorVersion = mn;
    sf.d->matchingRule = (QServiceFilter::VersionMatchRule) versionrule;
    sf.d->capMatchingRule = (QServiceFilter::CapabilityMatchRule) caprule;

    return in;
}
#endif //QT_NO_DATASTREAM


QT_END_NAMESPACE

