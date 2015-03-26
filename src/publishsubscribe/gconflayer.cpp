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

#if !defined(QT_NO_GCONFLAYER)

#include "gconflayer_p.h"

#include <QtCore/QDataStream>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(GConfLayer, gConfLayer)

GConfLayer::GConfLayer()
{
    GConfItem *gconfItem = new GConfItem(QString::fromLatin1("/"), true, this);
    connect(gconfItem, SIGNAL(subtreeChanged(QString,QVariant)), this, SLOT(notifyChanged(QString,QVariant)));
}

GConfLayer::~GConfLayer()
{
    QMutableHashIterator<QString, GConfHandle *> i(m_handles);
    while (i.hasNext()) {
        i.next();
        doRemoveHandle(Handle(i.value()));
    }
}

QUuid GConfLayer::id()
{
    return QVALUESPACE_GCONF_LAYER;
}

QValueSpace::LayerOptions GConfLayer::layerOptions() const
{
    return QValueSpace::PermanentLayer | QValueSpace::WritableLayer;
}

GConfLayer *GConfLayer::instance()
{
    return gConfLayer();
}

bool GConfLayer::value(Handle handle, QVariant *data)
{
    QMutexLocker locker(&m_mutex);
    GConfHandle *sh = gConfHandle(handle);
    if (!sh)
        return false;

    return getValue(InvalidHandle, sh->path, data);
}

bool GConfLayer::value(Handle handle, const QString &subPath, QVariant *data)
{
    QMutexLocker locker(&m_mutex);
    return getValue(handle, subPath, data);
}

bool GConfLayer::getValue(Handle handle, const QString &subPath, QVariant *data)
{
    if (handle != InvalidHandle && !gConfHandle(handle))
        return false;

    QString path(subPath);
    while (path.endsWith(QLatin1Char('/')))
        path.chop(1);

    if (handle != InvalidHandle) {
        while (path.startsWith(QLatin1Char('/')))
            path = path.mid(1);
    }

    int index = path.lastIndexOf(QLatin1Char('/'), -1);
    bool createdHandle = false;

    QString value;
    if (index == -1) {
        value = path;
    } else {
        // want a value that is in a sub path under handle
        value = path.mid(index + 1);
        path.truncate(index);

        if (path.isEmpty())
            path.append(QLatin1Char('/'));

        handle = getItem(handle, path);
        createdHandle = true;
    }

    GConfHandle *sh = gConfHandle(handle);
    if (!sh)
        return false;

    QString fullPath(sh->path);
    if (fullPath != QLatin1String("/") && !value.isEmpty())
        fullPath.append(QLatin1Char('/'));

    fullPath.append(value);

    GConfItem gconfItem(fullPath);
    QVariant readValue = gconfItem.value();
    switch (readValue.type()) {
    case QVariant::Invalid:
    case QVariant::Bool:
    case QVariant::Int:
    case QVariant::Double:
    case QVariant::StringList:
    case QVariant::List:
        *data = readValue;
        break;
    case QVariant::String: {
        QString readString = readValue.toString();
        QDataStream readStream(QByteArray::fromBase64(readString.toLatin1()));
        QVariant serializedValue;
        readStream >> serializedValue;
        if (serializedValue.isValid())
            *data = serializedValue;
        else
            *data = readValue;
        break;
    }
    default:
        break;
    }

    if (createdHandle)
        doRemoveHandle(handle);

    return data->isValid();
}

QSet<QString> GConfLayer::children(Handle handle)
{
    QMutexLocker locker(&m_mutex);

    GConfHandle *sh = gConfHandle(handle);
    if (!sh)
        return QSet<QString>();

    GConfItem gconfItem(sh->path);

    QSet<QString> ret;
    foreach (const QString &child, gconfItem.listEntries() + gconfItem.listDirs()) {
        const int index = child.lastIndexOf(QLatin1Char('/'), -1);
        ret += child.mid(index + 1);
    }

    return ret;
}

QAbstractValueSpaceLayer::Handle GConfLayer::item(Handle parent, const QString &subPath)
{
    QMutexLocker locker(&m_mutex);
    return getItem(parent, subPath);
}

QAbstractValueSpaceLayer::Handle GConfLayer::getItem(Handle parent, const QString &subPath)
{
    QString fullPath;

    // Fail on invalid path.
    if (subPath.isEmpty() || subPath.contains(QLatin1String("//")))
        return InvalidHandle;

    if (parent == InvalidHandle) {
        fullPath = subPath;
    } else {
        GConfHandle *sh = gConfHandle(parent);
        if (!sh)
            return InvalidHandle;

        if (subPath == QLatin1String("/"))
            fullPath = sh->path;
        else if (sh->path.endsWith(QLatin1Char('/')) && subPath.startsWith(QLatin1Char('/')))
            fullPath = sh->path + subPath.mid(1);
        else if (!sh->path.endsWith(QLatin1Char('/')) && !subPath.startsWith(QLatin1Char('/')))
            fullPath = sh->path + QLatin1Char('/') + subPath;
        else
            fullPath = sh->path + subPath;
    }

    if (m_handles.contains(fullPath)) {
        GConfHandle *sh = m_handles.value(fullPath);
        ++sh->refCount;
        return Handle(sh);
    }

    // Create a new handle for path
    GConfHandle *sh = new GConfHandle(fullPath);
    m_handles.insert(fullPath, sh);

    return Handle(sh);
}

void GConfLayer::setProperty(Handle handle, Properties properties)
{
    QMutexLocker locker(&m_mutex);

    GConfHandle *sh = gConfHandle(handle);
    if (!sh)
        return;

    QString basePath = sh->path;
    if (!basePath.endsWith(QLatin1Char('/')))
        basePath += QLatin1Char('/');

    if (properties & QAbstractValueSpaceLayer::Publish)
        m_monitoringHandles.insert(sh);
    else
        m_monitoringHandles.remove(sh);
}

void GConfLayer::removeHandle(Handle handle)
{
    QMutexLocker locker(&m_mutex);
    doRemoveHandle(handle);
}

void GConfLayer::doRemoveHandle(Handle handle)
{
    GConfHandle *sh = gConfHandle(handle);
    if (!sh)
        return;

    if (--sh->refCount)
        return;

    m_monitoringHandles.remove(sh);
    m_handles.remove(sh->path);

    delete sh;
}

bool GConfLayer::setValue(QValueSpacePublisher */*creator*/, Handle handle, const QString &subPath, const QVariant &data)
{
    QMutexLocker locker(&m_mutex);

    GConfHandle *sh = gConfHandle(handle);
    if (!sh)
        return false;

    QString path(subPath);
    while (path.endsWith(QLatin1Char('/')))
        path.chop(1);

    int index = path.lastIndexOf(QLatin1Char('/'), -1);

    bool createdHandle = false;

    QString value;
    if (index == -1) {
        value = path;
    } else {
        // want a value that is in a sub path under handle
        value = path.mid(index + 1);
        path.truncate(index);

        if (path.isEmpty())
            path.append(QLatin1Char('/'));

        sh = gConfHandle(getItem(Handle(sh), path));
        createdHandle = true;
    }

    QString fullPath(sh->path);
    if (fullPath != QLatin1String("/") && !value.isEmpty())
        fullPath.append(QLatin1Char('/'));

    fullPath.append(value);

    GConfItem gconfItem(fullPath);
    switch (data.type()) {
    case QVariant::Invalid:
    case QVariant::Bool:
    case QVariant::Int:
    case QVariant::Double:
    case QVariant::String:
    case QVariant::StringList:
    case QVariant::List:
        gconfItem.set(data);
        break;
    default:
        QByteArray byteArray;
        QDataStream writeStream(&byteArray, QIODevice::WriteOnly);
        writeStream << data;
        QString serializedValue(QString::fromLatin1(byteArray.toBase64().data()));
        gconfItem.set(serializedValue);
    }

    if (createdHandle)
        doRemoveHandle(Handle(sh));

    return true;
}

void GConfLayer::sync()
{
    //Not needed
}

bool GConfLayer::removeSubTree(QValueSpacePublisher * /*creator*/, Handle /*handle*/)
{
    //Not needed
    return false;
}

bool GConfLayer::removeValue(QValueSpacePublisher */*creator*/, Handle handle, const QString &subPath)
{
    QMutexLocker locker(&m_mutex);

    QString fullPath;

    GConfHandle *sh = gConfHandle(handle);
    if (!sh)
        return false;

    if (handle == InvalidHandle) {
        fullPath = subPath;
    } else {
        if (subPath == QLatin1String("/"))
            fullPath = sh->path;
        else if (sh->path.endsWith(QLatin1Char('/')) && subPath.startsWith(QLatin1Char('/')))
            fullPath = sh->path + subPath.mid(1);
        else if (!sh->path.endsWith(QLatin1Char('/')) && !subPath.startsWith(QLatin1Char('/')))
            fullPath = sh->path + QLatin1Char('/') + subPath;
        else
            fullPath = sh->path + subPath;
    }

    GConfItem gconfItem(fullPath);
    gconfItem.recursiveUnset();

    return true;
}

void GConfLayer::addWatch(QValueSpacePublisher *, Handle)
{
    //Not needed
}

void GConfLayer::removeWatches(QValueSpacePublisher *, Handle)
{
    //Not needed
}

bool GConfLayer::supportsInterestNotification() const
{
    return false;
}

bool GConfLayer::notifyInterest(Handle, bool)
{
    //Not needed
    return false;
}

void GConfLayer::notifyChanged(const QString &key, const QVariant & /*value*/)
{
    foreach (GConfHandle *handle, m_monitoringHandles.values()) {
        if (key.startsWith(handle->path))
            emit handleChanged(Handle(handle));
    }
}

QT_END_NAMESPACE

#endif // QT_NO_GCONFLAYER
