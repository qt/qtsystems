/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#include "contextkitlayer_p.h"
#include <contextpropertyinfo.h>

#include <QCoreApplication>
#include <QMetaType>
#include <QChar>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusArgument>

// dbus types
Q_DECLARE_METATYPE(float)

QDBusArgument &operator<<(QDBusArgument &argument, const float &f)
{
    argument.beginStructure();
    argument << (double)f;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, float &f)
{
    double d;
    argument.beginStructure();
    argument >> d;
    argument.endStructure();
    f = (float)d;
    return argument;
}

Q_DECLARE_METATYPE(QChar)

QDBusArgument &operator<<(QDBusArgument &argument, const QChar &ch)
{
    argument.beginStructure();
    argument << ch.unicode();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QChar &ch)
{
    ushort sh;
    argument.beginStructure();
    argument >> sh;
    argument.endStructure();
    ch = QChar(sh);
    return argument;
}

QT_BEGIN_NAMESPACE

using namespace QValueSpace;

ContextKitPath::ContextKitPath()
{
}

ContextKitPath::ContextKitPath(QString path)
{
    initFromPath(path);
}

void ContextKitPath::initFromPath(QString path)
{
    bool hasSlash = path.contains(QLatin1Char('/'));
    bool hasDot = path.contains(QLatin1Char('.'));
    bool startingSlash = path.startsWith(QLatin1Char('/'));

    if (startingSlash)
        path = path.mid(1);

    if (path == "" || path == "/") {
        m_type = SlashPath;
    } else if ((hasSlash && !hasDot) || (hasSlash && startingSlash)) {
        parts = path.split(QLatin1Char('/'));
        m_type = SlashPath;
    } else if (hasDot && !hasSlash) {
        parts = path.split(QLatin1Char('.'));
        m_type = DotPath;
    } else {
        m_type = SlashPath;
        parts.append(path);
    }
}

ContextKitPath::ContextKitPath(QString path, PathType type)
{
    initFromPath(path);
    m_type = type;
}

ContextKitPath::ContextKitPath(const ContextKitPath &other)
{
    parts = other.parts;
    m_type = other.m_type;
}

ContextKitPath &ContextKitPath::operator=(const ContextKitPath &other)
{
    parts = other.parts;
    m_type = other.m_type;
    return *this;
}

bool ContextKitPath::isRegistered() const
{
    ContextPropertyInfo pi(toNative());
    return pi.declared();
}

bool ContextKitPath::isProvided() const
{
    ContextPropertyInfo pi(toNative());
    return pi.provided();
}

ContextKitPath ContextKitPath::operator+(const ContextKitPath &other) const
{
    ContextKitPath p(*this);
    foreach (QString pt, other.parts)
        p.parts.append(pt);
    return p;
}

ContextKitPath ContextKitPath::operator+(const QString &str) const
{
    ContextKitPath s(str);
    return *this + s;
}

ContextKitPath ContextKitPath::operator-(const ContextKitPath &other) const
{
    if (other.parts.size() > parts.size()) return ContextKitPath();
    if (other.parts.size() == 0) return ContextKitPath(*this);
    if (parts.size() == 0) return ContextKitPath();

    ContextKitPath p(*this);
    ContextKitPath q(other);
    while (p.parts.size() > 0 && q.parts.size() > 0 &&
           p.parts.first() == q.parts.first()) {
        p.parts.removeFirst();
        q.parts.removeFirst();
    }
    return p;
}

QString ContextKitPath::toNative() const
{
    if (m_type == SlashPath)
        return "/" + parts.join("/");
    else if (m_type == DotPath)
        return parts.join(".");
    else
        return QString();
}

QString ContextKitPath::toQtPath() const
{
    return "/" + parts.join("/");
}

bool ContextKitPath::operator==(const ContextKitPath &other) const
{
    if (parts.size() != other.parts.size())
        return false;
    if (m_type != other.m_type)
        return false;

    for (int i = 0; i < parts.size(); i++)
        if (other.parts.at(i) != parts.at(i))
            return false;

    return true;
}

bool ContextKitPath::includes(ContextKitPath &other) const
{
    // can't include things smaller than you
    if (other.parts.size() < parts.size())
        return false;

    // can't include paths of different type
    if (other.m_type != m_type)
        return false;

    // root path includes all others
    if (parts.size() == 0)
        return true;

    for (int i = 0; i < parts.size(); i++)
        if (other.parts.at(i) != parts.at(i))
            return false;

    return true;
}

// borrowed from qt/corelib/io/qsettings_mac
static QString comify(const QString &organization)
{
    for (int i = organization.size() - 1; i >= 0; --i) {
        QChar ch = organization.at(i);
        if (ch == QLatin1Char('.') || ch == QChar(0x3002) || ch == QChar(0xff0e)
                || ch == QChar(0xff61)) {
            QString suffix = organization.mid(i + 1).toLower();
            if (suffix.size() == 2 || suffix == QLatin1String("com")
                    || suffix == QLatin1String("org") || suffix == QLatin1String("net")
                    || suffix == QLatin1String("edu") || suffix == QLatin1String("gov")
                    || suffix == QLatin1String("mil") || suffix == QLatin1String("biz")
                    || suffix == QLatin1String("info") || suffix == QLatin1String("name")
                    || suffix == QLatin1String("pro") || suffix == QLatin1String("aero")
                    || suffix == QLatin1String("coop") || suffix == QLatin1String("museum")) {
                QString result = organization;
                result.replace(QLatin1Char('/'), QLatin1Char(' '));
                return result;
            }
            break;
        }
        int uc = ch.unicode();
        if ((uc < 'a' || uc > 'z') && (uc < 'A' || uc > 'Z'))
            break;
    }

    QString domain;
    for (int i = 0; i < organization.size(); ++i) {
        QChar ch = organization.at(i);
        int uc = ch.unicode();
        if ((uc >= 'a' && uc <= 'z') || (uc >= '0' && uc <= '9')) {
            domain += ch;
        } else if (uc >= 'A' && uc <= 'Z') {
            domain += ch.toLower();
        } else {
           domain += QLatin1Char(' ');
        }
    }
    domain = domain.simplified();
    domain.replace(QLatin1Char(' '), QLatin1Char('-'));
    if (!domain.isEmpty())
        domain.append(QLatin1String(".com"));
    return domain;
}

void ContextKitHandle::insertRead(const ContextKitPath &path)
{
    if (!readProps.contains(path.toQtPath())) {
        ContextProperty *prop = new ContextProperty(path.toNative());
        readProps.insert(path.toQtPath(), prop);
    }
}

void ContextKitHandle::updateSubtrees()
{
    QList<QString> keys = ContextRegistryInfo::instance()->listKeys();
    foreach (const QString &k, keys) {
        ContextKitPath p(k);
        if (path.includes(p))
            insertRead(p);
    }
}

ContextKitHandle::ContextKitHandle(ContextKitHandle *parent, const QString &path,
                                   QValueSpace::LayerOptions opts)
{
    QString javaPackageName;
    int curPos = 0;
    int nextDot;

    QString domainName = comify(QCoreApplication::organizationDomain());
    if (domainName.isEmpty()) {
        domainName = QLatin1String("unknown-organization.nokia.com");
        qWarning("No organization name specified, registering on DBUS with "
                 "'com.nokia.unknown-organization'");
    }

    while ((nextDot = domainName.indexOf(QLatin1Char('.'), curPos)) != -1) {
        javaPackageName.prepend(domainName.mid(curPos, nextDot - curPos));
        javaPackageName.prepend(QLatin1Char('.'));
        curPos = nextDot + 1;
    }
    javaPackageName.prepend(domainName.mid(curPos));
    javaPackageName = javaPackageName.toLower();
    if (curPos == 0)
        javaPackageName.prepend(QLatin1String("com."));

    javaPackageName += QLatin1Char('.');
    QString app = QCoreApplication::applicationName();
    if (app.size() < 1 || app.at(0) == QLatin1Char(' ')) {
        javaPackageName += "unknown-application";
        qWarning("No application name specified, registering on DBUS as "
                 "'unknown-application'");
    } else {
        app.replace(QRegExp("[^0-9a-zA-Z]"), "");
        javaPackageName += app;
    }

    service = new ContextProvider::Service(QDBusConnection::SessionBus,
                                           javaPackageName, false);

    subscribed = true;

    if (opts & QValueSpace::PermanentLayer)
        this->path = ContextKitPath(path, ContextKitPath::DotPath);
    else
        this->path = ContextKitPath(path, ContextKitPath::SlashPath);

    if (parent)
        this->path = parent->path + this->path;

    updateSubtrees();
    connect(ContextRegistryInfo::instance(), SIGNAL(changed()),
            this, SLOT(updateSubtrees()));
}

ContextKitHandle::~ContextKitHandle()
{
    service->stop();
    foreach (ContextProperty *prop, readProps.values()) {
        prop->unsubscribe();
        delete prop;
    }
    foreach (ContextProvider::Property *prop, writeProps.values())
        delete prop;
    readProps.clear();
    writeProps.clear();
    service->start();
    delete service;
}

bool ContextKitHandle::value(const QString &path, QVariant *data)
{
    ContextKitPath p = this->path;
    if (!path.isEmpty()) p = p + path;

    if (!p.isRegistered() || !p.isProvided())
        return false;

    ContextProperty *prop = readProps.value(p.toQtPath());
    if (prop) {
        if (!subscribed)
            prop->subscribe();

        prop->waitForSubscription();
        *data = prop->value();

        if (!subscribed)
            prop->unsubscribe();

        return true;
    } else {
        return false;
    }
}

bool ContextKitHandle::setValue(const QString &path, const QVariant &data)
{
    ContextKitPath p = this->path;
    if (!path.isEmpty()) p = p + path;

    if (!p.isRegistered())
        return false;

    ContextProvider::Property *prop = writeProps.value(p.toQtPath());
    if (!prop)
        prop = insertWrite(p);

    prop->setValue(data);
    return true;
}

bool ContextKitHandle::unsetValue(const QString &path)
{
    ContextKitPath p = this->path;
    if (!path.isEmpty()) p = p + path;

    if (!p.isRegistered())
        return false;

    ContextProvider::Property *prop = writeProps.value(p.toQtPath());
    if (!prop)
        prop = insertWrite(p);

    prop->unsetValue();
    return true;
}

ContextProvider::Property *ContextKitHandle::insertWrite(const ContextKitPath &p)
{
    if (!writeProps.contains(p.toQtPath())) {
        ContextProvider::Property *prop;
        QString pth = p.toNative();

        service->stop();

        prop = new ContextProvider::Property(*service, pth);
        writeProps.insert(p.toQtPath(), prop);
        connect(prop, SIGNAL(firstSubscriberAppeared(QString)),
                this, SLOT(on_firstAppeared(QString)));
        connect(prop, SIGNAL(lastSubscriberDisappeared(QString)),
                this, SLOT(on_lastDisappeared(QString)));

        service->start();

        return prop;
    }
    return NULL;
}

void ContextKitHandle::on_firstAppeared(QString key)
{
    ContextKitPath p(key, path.type());
    QString relPath = (p - path).toQtPath().mid(1);
    emit interestChanged(relPath, true);
}

void ContextKitHandle::on_lastDisappeared(QString key)
{
    ContextKitPath p(key, path.type());
    QString relPath = (p - path).toQtPath().mid(1);
    emit interestChanged(relPath, false);
}

void ContextKitHandle::subscribe()
{
    foreach (ContextProperty *p, readProps.values()) {
        p->subscribe();
        connect(p, SIGNAL(valueChanged()),
                this, SIGNAL(valueChanged()));
    }
    subscribed = true;
}

void ContextKitHandle::unsubscribe()
{
    foreach (ContextProperty *p, readProps.values()) {
        p->unsubscribe();
        disconnect(p, SIGNAL(valueChanged()),
                   this, SIGNAL(valueChanged()));
    }
    subscribed = false;
}

QSet<QString> ContextKitHandle::children()
{
    QSet<QString> kids;

    foreach (const QString &qp, readProps.keys()) {
        ContextKitPath pth(qp);
        pth = pth - this->path;
        if (pth.size() > 0)
            kids.insert(pth.at(0));
    }
    return kids;
}

ContextKitLayer::ContextKitLayer()
{
    qDBusRegisterMetaType<float>();
    qDBusRegisterMetaType<QChar>();
}

ContextKitLayer::~ContextKitLayer()
{
}

ContextKitHandle *ContextKitLayer::handleToCKHandle(Handle handle)
{
    ContextKitHandle *ckh = NULL;
    if (handle != InvalidHandle)
        ckh = reinterpret_cast<ContextKitHandle*>(handle);
    return ckh;
}

void ContextKitLayer::sync()
{
    QCoreApplication::processEvents();
}

bool ContextKitLayer::startup(Type)
{
    return true;
}

QAbstractValueSpaceLayer::Handle ContextKitLayer::item(Handle parent,
                                                       const QString &subPath)
{
    ContextKitHandle *parentHandle = handleToCKHandle(parent);
    ContextKitHandle *h = new ContextKitHandle(parentHandle, subPath,
                                               this->layerOptions());
    return reinterpret_cast<Handle>(h);
}

void ContextKitLayer::removeHandle(Handle handle)
{
    ContextKitHandle *h = handleToCKHandle(handle);
    if (h)
        delete h;
}

void ContextKitLayer::setProperty(Handle handle, Properties properties)
{
    ContextKitHandle *h = handleToCKHandle(handle);
    if (!h)
        return;

    if (properties & Publish)
        connect(h, SIGNAL(valueChanged()),
                this, SLOT(contextHandleChanged()));
    else
        disconnect(h, SIGNAL(valueChanged()),
                   this, SLOT(contextHandleChanged()));
}

void ContextKitLayer::contextHandleChanged()
{
    emit handleChanged(reinterpret_cast<Handle>(sender()));
}

void ContextKitLayer::addWatch(QValueSpacePublisher *pub, Handle handle)
{
    ContextKitHandle *h = handleToCKHandle(handle);
    connect(h, SIGNAL(interestChanged(QString,bool)),
            pub, SIGNAL(interestChanged(QString,bool)));
}

void ContextKitLayer::removeWatches(QValueSpacePublisher *pub, Handle handle)
{
    ContextKitHandle *h = handleToCKHandle(handle);
    disconnect(h, SIGNAL(interestChanged(QString,bool)),
               pub, SIGNAL(interestChanged(QString,bool)));
}

bool ContextKitLayer::value(Handle handle, QVariant *data)
{
    ContextKitHandle *h = handleToCKHandle(handle);
    if (h)
        return h->value("", data);
    else
        return false;
}

bool ContextKitLayer::value(Handle handle, const QString &subPath, QVariant *data)
{
    ContextKitHandle *h = handleToCKHandle(handle);
    if (h)
        return h->value(subPath, data);
    else
        return false;
}

bool ContextKitLayer::setValue(QValueSpacePublisher *, Handle handle,
                               const QString &path, const QVariant &value)
{
    ContextKitHandle *h = handleToCKHandle(handle);
    if (h)
        return h->setValue(path, value);
    else
        return false;
}

bool ContextKitLayer::removeValue(QValueSpacePublisher *, Handle handle,
                                  const QString &path)
{
    ContextKitHandle *h = handleToCKHandle(handle);
    if (h)
        return h->unsetValue(path);
    else
        return false;
}

bool ContextKitLayer::removeSubTree(QValueSpacePublisher *vsp, Handle handle)
{
    ContextKitHandle *h = handleToCKHandle(handle);
    if (!h)
        return false;

    // TODO: fix this up
    foreach (QString kid, h->children())
        if (!h->unsetValue(kid)) return false;

    return true;
}

bool ContextKitLayer::notifyInterest(Handle handle, bool interested)
{
    ContextKitHandle *h = handleToCKHandle(handle);

    if (interested)
        h->subscribe();
    else
        h->unsubscribe();
    return true;
}

QSet<QString> ContextKitLayer::children(Handle handle)
{
    ContextKitHandle *h = handleToCKHandle(handle);
    return h->children();
}

QVALUESPACE_AUTO_INSTALL_LAYER(ContextKitNonCoreLayer)
Q_GLOBAL_STATIC(ContextKitNonCoreLayer, contextKitNonCoreLayer)
ContextKitNonCoreLayer *ContextKitNonCoreLayer::instance()
{
    return contextKitNonCoreLayer();
}

QValueSpace::LayerOptions ContextKitNonCoreLayer::layerOptions() const
{
    return WritableLayer | TransientLayer;
}

QString ContextKitNonCoreLayer::name()
{
    return "ContextKit Non-Core Layer";
}

unsigned int ContextKitNonCoreLayer::order()
{
    return 1;
}

QUuid ContextKitNonCoreLayer::id()
{
    return QVALUESPACE_CONTEXTKITNONCORE_LAYER;
}

QVALUESPACE_AUTO_INSTALL_LAYER(ContextKitCoreLayer)
Q_GLOBAL_STATIC(ContextKitCoreLayer, contextKitCoreLayer)
ContextKitCoreLayer *ContextKitCoreLayer::instance()
{
    return contextKitCoreLayer();
}

QValueSpace::LayerOptions ContextKitCoreLayer::layerOptions() const
{
    return WritableLayer | PermanentLayer;
}

QString ContextKitCoreLayer::name()
{
    return "ContextKit Core Layer";
}

unsigned int ContextKitCoreLayer::order()
{
    return 0;
}

QUuid ContextKitCoreLayer::id()
{
    return QVALUESPACE_CONTEXTKITCORE_LAYER;
}

QT_END_NAMESPACE

