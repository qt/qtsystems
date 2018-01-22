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

#ifndef QSERVICEFILTER_H
#define QSERVICEFILTER_H

#include "qserviceframeworkglobal.h"
#include <QStringList>

QT_BEGIN_NAMESPACE
class QDataStream;

#ifdef QT_SFW_SERVICEDATABASE_GENERATE
#undef Q_SERVICEFW_EXPORT
#define Q_SERVICEFW_EXPORT
#endif

class QServiceFilterPrivate;
class Q_SERVICEFW_EXPORT QServiceFilter
{
public:
    enum VersionMatchRule {
        ExactVersionMatch = 0,
        MinimumVersionMatch
    };

    enum CapabilityMatchRule {
        MatchMinimum = 0,
        MatchLoadable
    };

    QServiceFilter();
    ~QServiceFilter();
    QServiceFilter(const QServiceFilter& other);
    explicit QServiceFilter(const QString& interfaceName,
                   const QString& version = QString(),
                   QServiceFilter::VersionMatchRule rule = QServiceFilter::MinimumVersionMatch);

    QServiceFilter& operator=(const QServiceFilter& other);

    void setInterface(const QString& interfaceName, const QString& version = QString(),
            QServiceFilter::VersionMatchRule rule = QServiceFilter::MinimumVersionMatch);
    void setServiceName(const QString& serviceName);


    QString serviceName() const;
    QString interfaceName() const;
    int majorVersion() const;
    int minorVersion() const;
    VersionMatchRule versionMatchRule() const;

    QStringList customAttributes() const;
    QString customAttribute(const QString& which) const;
    void setCustomAttribute(const QString& key, const QString& value);
    void clearCustomAttribute(const QString &key = QString());

    void setCapabilities(QServiceFilter::CapabilityMatchRule, const QStringList& capabilities = QStringList() );
    QStringList capabilities() const;
    CapabilityMatchRule capabilityMatchRule() const;

private:
    QServiceFilterPrivate *d;
    friend class QServiceManager;
    //friend class ServiceDatabase;
#ifndef QT_NO_DATASTREAM
    friend Q_SERVICEFW_EXPORT QDataStream &operator<<(QDataStream &, const QServiceFilter &);
    friend Q_SERVICEFW_EXPORT QDataStream &operator>>(QDataStream &, QServiceFilter &);
#endif
};

#ifndef QT_NO_DATASTREAM
Q_SERVICEFW_EXPORT QDataStream &operator<<(QDataStream &, const QServiceFilter &);
Q_SERVICEFW_EXPORT QDataStream &operator>>(QDataStream &, QServiceFilter &);
#endif

QT_END_NAMESPACE

#endif //QSERVICEFILTER_H
