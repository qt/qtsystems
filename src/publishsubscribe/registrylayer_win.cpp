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

#include "registrylayer_win_p.h"
#include <QtCore/qdatastream.h>

#if defined(Q_OS_WIN)

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(NonVolatileRegistryLayer, nonVolatileRegistryLayer)
Q_GLOBAL_STATIC(VolatileRegistryLayer, volatileRegistryLayer)

void CALLBACK qVolatileRegistryLayerCallback(PVOID lpParameter, BOOLEAN)
{
    QMetaObject::invokeMethod(volatileRegistryLayer(), "emitHandleChanged", Qt::QueuedConnection,
                              Q_ARG(void *, lpParameter));
}

void CALLBACK qNonVolatileRegistryLayerCallback(PVOID lpParameter, BOOLEAN)
{
    QMetaObject::invokeMethod(nonVolatileRegistryLayer(), "emitHandleChanged",
                              Qt::QueuedConnection, Q_ARG(void *, lpParameter));
}

static QString qConvertPath(const QString &path)
{
    QString fixedPath(path);

    while (fixedPath.endsWith(QLatin1Char('/')))
        fixedPath.chop(1);

    fixedPath.replace(QLatin1Char('/'), QLatin1Char('\\'));

    return fixedPath;
}

VolatileRegistryLayer::VolatileRegistryLayer()
    :   RegistryLayer(QLatin1String("Software/Nokia/QtMobility/volatileContext"), true,
          &qVolatileRegistryLayerCallback)
{
}

VolatileRegistryLayer::~VolatileRegistryLayer()
{
}

QUuid VolatileRegistryLayer::id()
{
    return QVALUESPACE_VOLATILEREGISTRY_LAYER;
}

QValueSpace::LayerOptions VolatileRegistryLayer::layerOptions() const
{
    return QValueSpace::TransientLayer | QValueSpace::WritableLayer;
}

VolatileRegistryLayer *VolatileRegistryLayer::instance()
{
    return volatileRegistryLayer();
}

NonVolatileRegistryLayer::NonVolatileRegistryLayer()
    :   RegistryLayer(QLatin1String("Software/Nokia/QtMobility/nonVolatileContext"), false,
          &qNonVolatileRegistryLayerCallback)
{
}

NonVolatileRegistryLayer::~NonVolatileRegistryLayer()
{
}

QUuid NonVolatileRegistryLayer::id()
{
    return QVALUESPACE_NONVOLATILEREGISTRY_LAYER;
}

QValueSpace::LayerOptions NonVolatileRegistryLayer::layerOptions() const
{
    return QValueSpace::PermanentLayer | QValueSpace::WritableLayer;
}

NonVolatileRegistryLayer *NonVolatileRegistryLayer::instance()
{
    return nonVolatileRegistryLayer();
}

RegistryLayer::RegistryLayer(const QString &basePath, bool volatileKeys, RegistryCallback callback)
    :   localLock(QMutex::Recursive), m_basePath(basePath), m_volatileKeys(volatileKeys),
      m_callback(callback)
{
    // Ensure that the m_basePath key exists and is non-volatile.
    HKEY key;
    RegCreateKeyEx(HKEY_CURRENT_USER,
                   reinterpret_cast<const wchar_t*>(qConvertPath(m_basePath).utf16()),
                   0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &key, 0);

    RegCloseKey(key);
}

RegistryLayer::~RegistryLayer()
{
    QMutableHashIterator<QString, RegistryHandle *> i(handles);
    while (i.hasNext()) {
        i.next();

        RegistryHandle *handle = i.value();

        if (handle->valueHandle)
            removeHandle(Handle(handle));
    }

    i.toFront();
    while (i.hasNext()) {
        i.next();

        removeHandle(Handle(i.value()));
    }
}

bool RegistryLayer::value(Handle handle, QVariant *data)
{
    QMutexLocker locker(&localLock);

    RegistryHandle *rh = registryHandle(handle);
    if (!rh)
        return false;

    return value(InvalidHandle, rh->path, data);
}

bool RegistryLayer::value(Handle handle, const QString &subPath, QVariant *data)
{
    QMutexLocker locker(&localLock);

    if (handle != InvalidHandle && !registryHandle(handle))
        return false;

    QString path(subPath);
    while (path.endsWith(QLatin1Char('/')))
        path.chop(1);
    if (handle != InvalidHandle)
        while (path.startsWith(QLatin1Char('/')))
            path = path.mid(1);

    Handle fullHandle = item(handle, subPath);
    if (fullHandle != InvalidHandle) {
        RegistryHandle *rh = registryHandle(fullHandle);
        if (rh) {
            openRegistryKey(rh);
            if (!rh->valueHandle) {
                handle = fullHandle;
                path.clear();
            }
        }
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
            path = QLatin1String("/");

        handle = item(handle, path);
        createdHandle = true;
    }

    RegistryHandle *rh = registryHandle(handle);

    openRegistryKey(rh);
    if (!hKeys.contains(rh)) {
        if (createdHandle)
            removeHandle(handle);

        return false;
    }

    HKEY key = hKeys.value(rh);

    DWORD regSize = 0;
    long result = RegQueryValueEx(key,
                                  reinterpret_cast<const wchar_t*>(value.utf16()), 0, 0, 0, &regSize);
    if (result == ERROR_FILE_NOT_FOUND) {
        *data = QVariant();
        if (createdHandle)
            removeHandle(handle);
        return false;
    } else if (result != ERROR_SUCCESS) {
        qDebug() << "RegQueryValueEx failed with error" << result;
        *data = QVariant();
        if (createdHandle)
            removeHandle(handle);
        return false;
    }

    BYTE *regData = new BYTE[regSize];
    DWORD regType;

    result = RegQueryValueEx(key,
                             reinterpret_cast<const wchar_t*>(value.utf16()),
                             0, &regType, regData, &regSize);
    if (result != ERROR_SUCCESS) {
        qDebug() << "real RegQueryValueEx failed with error" << result;
        if (createdHandle)
            removeHandle(handle);
        return false;
    }

    switch (regType) {
    case REG_DWORD:
        *data = qVariantFromValue(*reinterpret_cast<uint *>(regData));
        break;
    case REG_QWORD:
        *data = qVariantFromValue(*reinterpret_cast<quint64 *>(regData));
        break;
    case REG_SZ:
        *data = qVariantFromValue(QString::fromWCharArray(reinterpret_cast<wchar_t *>(regData)));
        break;
    case REG_MULTI_SZ: {
        QStringList list;
        wchar_t *temp = reinterpret_cast<wchar_t *>(regData);
        while (*temp != L'\0') {
            QString string = QString::fromWCharArray(temp);
            list << string;
            temp += string.length() + 1;
        }
        *data = qVariantFromValue(list);
        break;
    }
    case REG_BINARY:
        *data = qVariantFromValue(QByteArray(reinterpret_cast<char *>(regData), regSize));
        break;
    case REG_NONE: {
        QDataStream stream(QByteArray::fromRawData(reinterpret_cast<char *>(regData), regSize));
        stream >> *data;
        break;
    }
    default:
        qDebug() << "Unknown REG type" << regType;
        delete[] regData;
        if (createdHandle)
            removeHandle(handle);
        return false;
        break;
    }

    delete[] regData;

    if (createdHandle)
        removeHandle(handle);

    return true;
}

#define MAX_KEY_LENGTH 255
#define MAX_NAME_LENGTH 16383
QSet<QString> RegistryLayer::children(Handle handle)
{
    QMutexLocker locker(&localLock);

    QSet<QString> foundChildren;

    RegistryHandle *rh = registryHandle(handle);
    if (!rh)
        return foundChildren;

    openRegistryKey(rh);
    if (rh->valueHandle || !hKeys.contains(rh))
        return foundChildren;

    HKEY key = hKeys.value(rh);
    int i = 0;
    long result;
    do {
        TCHAR subKey[MAX_KEY_LENGTH];
        DWORD subKeySize = MAX_KEY_LENGTH;

        result = RegEnumKeyEx(key, i, subKey, &subKeySize, 0, 0, 0, 0);
        if (result == ERROR_KEY_DELETED) {
            QMetaObject::invokeMethod(this, "emitHandleChanged", Qt::QueuedConnection,
                                      Q_ARG(void *, key));
            break;
        }
        if (result != ERROR_SUCCESS)
            break;

        foundChildren << QString::fromWCharArray(subKey, subKeySize);
        ++i;
    } while (result == ERROR_SUCCESS);

    i = 0;
    do {
        TCHAR valueName[MAX_NAME_LENGTH];
        DWORD valueNameSize = MAX_NAME_LENGTH;

        result = RegEnumValue(key, i, valueName, &valueNameSize, 0, 0, 0, 0);
        if (result == ERROR_KEY_DELETED) {
            QMetaObject::invokeMethod(this, "emitHandleChanged", Qt::QueuedConnection,
                                      Q_ARG(void *, key));
            break;
        }
        if (result != ERROR_SUCCESS)
            break;

        QString value = QString::fromWCharArray(valueName, valueNameSize);
        if (!value.isEmpty())
            foundChildren << value;

        ++i;
    } while (result == ERROR_SUCCESS);

    return foundChildren;
}

QAbstractValueSpaceLayer::Handle RegistryLayer::item(Handle parent, const QString &path)
{
    QMutexLocker locker(&localLock);

    QString fullPath;

    // Fail on invalid path.
    if (path.isEmpty() || path.contains(QLatin1String("//")))
        return InvalidHandle;

    if (parent == InvalidHandle) {
        fullPath = path;
    } else {
        RegistryHandle *rh = registryHandle(parent);
        if (!rh)
            return InvalidHandle;

        if (path == QLatin1String("/")) {
            fullPath = rh->path;
        } else if (rh->path.endsWith(QLatin1Char('/')) && path.startsWith(QLatin1Char('/')))
            fullPath = rh->path + path.mid(1);
        else if (!rh->path.endsWith(QLatin1Char('/')) && !path.startsWith(QLatin1Char('/')))
            fullPath = rh->path + QLatin1Char('/') + path;
        else
            fullPath = rh->path + path;
    }

    if (handles.contains(fullPath)) {
        RegistryHandle *rh = handles.value(fullPath);
        ++rh->refCount;
        return Handle(rh);
    }

    // Create a new handle for path
    RegistryHandle *rh = new RegistryHandle(fullPath);
    handles.insert(fullPath, rh);

    return Handle(rh);
}

void RegistryLayer::setProperty(Handle handle, Properties properties)
{
    QMutexLocker locker(&localLock);

    RegistryHandle *rh = registryHandle(handle);
    if (!rh)
        return;

    if (properties & QAbstractValueSpaceLayer::Publish) {
        // Enable change notification for handle
        openRegistryKey(rh);
        if (!hKeys.contains(rh)) {
            // The key does not exist. Temporarily use the parent key as a proxy.
            QString path = rh->path;
            while (!path.isEmpty()) {
                rh = registryHandle(item(InvalidHandle, path));
                openRegistryKey(rh);
                if (hKeys.contains(rh)) {
                    notifyProxies.insert(rh, registryHandle(handle));
                    break;
                }
                removeHandle(Handle(rh));

                // root path doesn't exists
                if (path.length() == 1)
                    return;

                int index = path.lastIndexOf(QLatin1Char('/'));
                if (index == 0)
                    path.truncate(1);
                else
                    path.truncate(index);
            }
        }

        HKEY key = hKeys.value(rh);

        if (waitHandles.contains(key))
            return;

        ::HANDLE event = CreateEvent(0, false, false, 0);
        if (event == 0) {
            qDebug() << "CreateEvent failed with error" << GetLastError();
            return;
        }

        DWORD filter = REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_ATTRIBUTES | REG_NOTIFY_CHANGE_LAST_SET;
        long result = RegNotifyChangeKeyValue(key, true, filter, event, true);

        if (result != ERROR_SUCCESS) {
            qDebug() << "RegNotifyChangeKeyValue failed with error" << result;
            return;
        }

        ::HANDLE waitHandle;
        if (!RegisterWaitForSingleObject(&waitHandle, event, m_callback, key,
                                         INFINITE, WT_EXECUTEDEFAULT)) {
            qDebug() << "RegisterWaitForSingleObject failed with error" << GetLastError();
            return;
        }

        //waitHandles.insert(key, QPair<::HANDLE, ::HANDLE>(event, waitHandle));
        waitHandles.insert(key, HandlePair(event, waitHandle));
    }
    if (!(properties & QAbstractValueSpaceLayer::Publish)) {
        // Disable change notification for handle
        if (!hKeys.contains(rh))
            return;

        HKEY key = hKeys.value(rh);

        if (waitHandles.contains(key)) {
            //QPair<::HANDLE, ::HANDLE> wait = waitHandles.take(key);
            HandlePair wait = waitHandles.take(key);

            UnregisterWait(wait.second);
            CloseHandle(wait.first);
        }
    }
}

void RegistryLayer::removeHandle(Handle handle)
{
    QMutexLocker locker(&localLock);

    RegistryHandle *rh = registryHandle(handle);
    if (!rh)
        return;

    if (--rh->refCount)
        return;

    QList<RegistryHandle *> proxies = notifyProxies.keys(rh);
    while (!proxies.isEmpty()) {
        notifyProxies.remove(proxies.first(), rh);
        removeHandle(Handle(proxies.takeFirst()));
    }

    handles.remove(rh->path);

    closeRegistryKey(rh);

    delete rh;
}

void RegistryLayer::closeRegistryKey(RegistryHandle *handle)
{
    QMutexLocker locker(&localLock);

    if (!hKeys.contains(handle))
        return;

    HKEY key = hKeys.take(handle);

    // Check if other handles are using this registry key.
    if (!hKeys.keys(key).isEmpty())
        return;

    if (waitHandles.contains(key)) {
        //QPair<::HANDLE, ::HANDLE> wait = waitHandles.take(key);
        HandlePair wait = waitHandles.take(key);

        UnregisterWait(wait.second);
        CloseHandle(wait.first);
    }

    RegCloseKey(key);
}

static LONG qRegDeleteTree(HKEY hKey, LPCTSTR lpSubKey)
{
    HKEY key;
    long result = RegOpenKeyEx(hKey, lpSubKey, 0, KEY_ALL_ACCESS, &key);
    if (result != ERROR_SUCCESS) {
        if (result == ERROR_FILE_NOT_FOUND)
            return ERROR_SUCCESS;
        else
            return result;
    }

    do {
        TCHAR subKey[MAX_KEY_LENGTH];
        DWORD subKeySize = MAX_KEY_LENGTH;

        result = RegEnumKeyEx(key, 0, subKey, &subKeySize, 0, 0, 0, 0);
        if (result == ERROR_NO_MORE_ITEMS)
            break;

        result = qRegDeleteTree(key, subKey);
    } while (result == ERROR_SUCCESS);

    RegCloseKey(key);

    if (result != ERROR_NO_MORE_ITEMS && result != ERROR_SUCCESS)
        return result;

    return RegDeleteKey(hKey, lpSubKey);
}

bool RegistryLayer::removeRegistryValue(RegistryHandle *handle, const QString &subPath)
{
    QMutexLocker locker(&localLock);

    QString path(subPath);
    while (path.endsWith(QLatin1Char('/')))
        path.chop(1);

    int index = path.lastIndexOf(QLatin1Char('/'), -1);

    bool createdHandle = false;

    RegistryHandle *rh;
    QString value;
    if (index == -1) {
        rh = handle;
        value = path;
    } else {
        // want a value that is in a sub path under handle
        value = path.mid(index + 1);
        path.truncate(index);
        if (path.isEmpty())
            path.append(QLatin1Char('/'));

        rh = registryHandle(item(handle ? Handle(handle) : InvalidHandle, path));

        createdHandle = true;
    }

    openRegistryKey(rh);
    if (!hKeys.contains(rh)) {
        if (createdHandle)
            removeHandle(Handle(rh));

        return false;
    }

    HKEY key = hKeys.value(rh);

    long result = RegDeleteValue(key, reinterpret_cast<const wchar_t*>(value.utf16()));
    if (result == ERROR_FILE_NOT_FOUND) {
        result = qRegDeleteTree(key, reinterpret_cast<const wchar_t*>(value.utf16()));
        if (result == ERROR_SUCCESS) {
            const QString rootPath = rh->path;

            QList<QString> paths = handles.keys();
            while (!paths.isEmpty()) {
                QString p = paths.takeFirst();

                if (p.startsWith(rootPath))
                    closeRegistryKey(handles.value(p));
            }
        }
        if (result != ERROR_SUCCESS)
            qDebug() << "RegDeleteTree failed with error" << result;
    } else if (result != ERROR_SUCCESS) {
        qDebug() << "RegDeleteValue failed with error" << result;
    }

    if (createdHandle)
        removeHandle(Handle(rh));

    return result == ERROR_SUCCESS;
}

bool RegistryLayer::setValue(QValueSpacePublisher *creator, Handle handle, const QVariant &data)
{
    return setValue(creator, handle, QString(), data);
}

bool RegistryLayer::setValue(QValueSpacePublisher *creator, Handle handle, const QString &subPath,
                             const QVariant &data)
{
    QMutexLocker locker(&localLock);

    RegistryHandle *rh = registryHandle(handle);
    if (!rh)
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

        rh = registryHandle(item(Handle(rh), path));
        createdHandle = true;
    }

    if (!value.isEmpty()) {
        QString fullPath = rh->path;
        if (!fullPath.endsWith(QLatin1Char('/')))
            fullPath.append(QLatin1Char('/'));
        fullPath.append(value);

        RegistryHandle *fullHandle = registryHandle(item(InvalidHandle, fullPath));
        if (fullHandle) {
            fullHandle->valueHandle = true;
            removeHandle(Handle(fullHandle));
        }
    }

    if (createRegistryKey(rh)) {
        if (!creators[creator].contains(rh->path))
            creators[creator].append(rh->path);
    }
    if (!hKeys.contains(rh)) {
        if (createdHandle)
            removeHandle(Handle(rh));

        return false;
    }

    HKEY key = hKeys.value(rh);

    long result;

    switch (data.type()) {
    case QVariant::UInt: {
        DWORD temp = data.toUInt();
        result = RegSetValueEx(key, reinterpret_cast<const wchar_t*>(value.utf16()), 0,
                               REG_DWORD, reinterpret_cast<const BYTE *>(&temp), sizeof(DWORD));
        break;
    }
    case QVariant::ULongLong: {
        quint64 temp = data.toULongLong();
        result = RegSetValueEx(key, reinterpret_cast<const wchar_t*>(value.utf16()), 0,
                               REG_QWORD, reinterpret_cast<const BYTE *>(&temp), sizeof(quint64));
        break;
    }
    case QVariant::String: {
        // This may be wrong!
        QString tempString = data.toString();
        int length = tempString.length() + 1;
        wchar_t *temp = new wchar_t[length];
        tempString.toWCharArray(temp);
        temp[length - 1] = L'\0';
        result = RegSetValueEx(key, reinterpret_cast<const wchar_t*>(value.utf16()), 0,
                               REG_SZ, reinterpret_cast<const BYTE *>(temp), length * sizeof(wchar_t));
        delete[] temp;
        break;
    }
    case QVariant::StringList: {
        const QString joined = data.toStringList().join(QString(QLatin1Char('\0')));
        int length = joined.length() + 2;
        wchar_t *temp = new wchar_t[length];
        joined.toWCharArray(temp);
        temp[length - 2] = L'\0';
        temp[length - 1] = L'\0';
        result = RegSetValueEx(key, reinterpret_cast<const wchar_t*>(value.utf16()), 0,
                               REG_MULTI_SZ, reinterpret_cast<const BYTE *>(temp),
                               length * sizeof(wchar_t));
        delete[] temp;
        break;
    }
    case QVariant::ByteArray: {
        QByteArray temp = data.toByteArray();
        result = RegSetValueEx(key, reinterpret_cast<const wchar_t*>(value.utf16()), 0,
                               REG_BINARY, reinterpret_cast<const BYTE *>(temp.constData()),
                               temp.length());
        break;
    }
    default: {
        QByteArray temp;
        QDataStream stream(&temp, QIODevice::WriteOnly | QIODevice::Truncate);
        stream << data;
        result = RegSetValueEx(key, reinterpret_cast<const wchar_t*>(value.utf16()), 0,
                               REG_NONE, reinterpret_cast<const BYTE *>(temp.constData()),
                               temp.length());
        break;
    }
    };

    QString fullPath(rh->path);
    if (fullPath != QLatin1String("/"))
        fullPath.append(QLatin1Char('/'));

    fullPath.append(value);

    if (!creators[creator].contains(fullPath))
        creators[creator].append(fullPath);

    if (createdHandle)
        removeHandle(Handle(rh));

    return result == ERROR_SUCCESS;
}

void RegistryLayer::sync()
{
    QMutexLocker locker(&localLock);

    // Wait for change notification callbacks before returning
    QEventLoop loop;
    connect(this, SIGNAL(handleChanged(quintptr)), &loop, SLOT(quit()));
    bool wait = false;

    QList<HKEY> keys = hKeys.values();
    while (!keys.isEmpty()) {
        HKEY key = keys.takeFirst();

        if (!wait && waitHandles.contains(key))
            wait = true;

        RegFlushKey(key);
    }

    if (wait) {
        locker.unlock();
        QTimer::singleShot(1000, &loop, SLOT(quit()));
        loop.exec();
    }
}

void RegistryLayer::emitHandleChanged(void *k)
{
    QMutexLocker locker(&localLock);

    HKEY key = reinterpret_cast<HKEY>(k);

    QList<RegistryHandle *> changedHandles = hKeys.keys(key);
    if (changedHandles.isEmpty()) {
        if (waitHandles.contains(key)) {
            //QPair<::HANDLE, ::HANDLE> wait = waitHandles.take(key);
            HandlePair wait = waitHandles.take(key);

            UnregisterWait(wait.second);
            CloseHandle(wait.first);
            return;
        }
    }

    while (!changedHandles.isEmpty()) {
        RegistryHandle *handle = changedHandles.takeFirst();
        emit handleChanged(Handle(handle));

        // Emit signal for handles that this handle is proxying for.
        foreach (RegistryHandle *proxied, notifyProxies.values(handle)) {
            openRegistryKey(proxied);
            if (hKeys.contains(proxied)) {
                notifyProxies.remove(handle, proxied);
                removeHandle(Handle(handle));
                setProperty(Handle(proxied), Publish);

                emit handleChanged(Handle(proxied));
            }
        }
    }

    if (waitHandles.contains(key)) {
        //QPair<::HANDLE, ::HANDLE> wait = waitHandles.take(key);
        HandlePair wait = waitHandles.take(key);

        ::HANDLE event = wait.first;

        UnregisterWait(wait.second);

        long result = RegNotifyChangeKeyValue(key, true, REG_NOTIFY_CHANGE_NAME |
                                              REG_NOTIFY_CHANGE_ATTRIBUTES |
                                              REG_NOTIFY_CHANGE_LAST_SET,
                                              event, true);

        if (result != ERROR_SUCCESS) {
            if (result == ERROR_KEY_DELETED || result == ERROR_INVALID_PARAMETER) {
                CloseHandle(event);

                QList<RegistryHandle *> changedHandles = hKeys.keys(key);

                for (int i = 0; i < changedHandles.count(); ++i)
                    hKeys.remove(changedHandles.at(i));

                RegCloseKey(key);

                for (int i = 0; i < changedHandles.count(); ++i)
                    setProperty(Handle(changedHandles.at(i)), Publish);
            } else {
                qDebug() << "RegNotifyChangeKeyValue failed with error" << result;
            }

            return;
        }

        ::HANDLE waitHandle;

        if (!RegisterWaitForSingleObject(&waitHandle, event, m_callback, key,
                                         INFINITE, WT_EXECUTEDEFAULT)) {
            qDebug() << "RegisterWaitForSingleObject failed with error" << GetLastError();
            return;
        }

        //waitHandles.insert(key, QPair<::HANDLE, ::HANDLE>(event, waitHandle));
        waitHandles.insert(key, HandlePair(event, waitHandle));
    }
}

void RegistryLayer::openRegistryKey(RegistryHandle *handle)
{
    QMutexLocker locker(&localLock);

    if (!handle)
        return;

    // Check if HKEY for this handle already exists.
    if (hKeys.contains(handle))
        return;

    const QString fullPath = qConvertPath(m_basePath + handle->path);

    // Attempt to open registry key path
    HKEY key;
    long result = RegOpenKeyEx(HKEY_CURRENT_USER,
                               reinterpret_cast<const wchar_t*>(fullPath.utf16()),
                               0, KEY_ALL_ACCESS, &key);

    if (result == ERROR_SUCCESS) {
        hKeys.insert(handle, key);
    } else if (result == ERROR_FILE_NOT_FOUND) {
        // Key for handle does not exist in Registry, check if it is a value handle.
        int index = handle->path.lastIndexOf(QLatin1Char('/'), -1);

        const QString parentPath = handle->path.left(index);
        const QString valueName = handle->path.mid(index + 1);

        RegistryHandle *parentHandle = registryHandle(item(InvalidHandle, parentPath));
        if (!parentHandle)
            return;

        openRegistryKey(parentHandle);
        if (!hKeys.contains(parentHandle)) {
            removeHandle(Handle(parentHandle));
            return;
        }

        // Check if value exists.
        if (!children(Handle(parentHandle)).contains(valueName)) {
            removeHandle(Handle(parentHandle));
            return;
        }

        handle->valueHandle = true;

        hKeys.insert(handle, hKeys.value(parentHandle));

        removeHandle(Handle(parentHandle));
    }
}

bool RegistryLayer::createRegistryKey(RegistryHandle *handle)
{
    QMutexLocker locker(&localLock);

    // Check if HKEY for this handle already exists.
    if (hKeys.contains(handle))
        return false;

    const QString fullPath = qConvertPath(m_basePath + handle->path);

    // Attempt to open registry key path
    HKEY key;
    DWORD disposition;
    long result = RegCreateKeyEx(HKEY_CURRENT_USER,
                                 reinterpret_cast<const wchar_t*>(fullPath.utf16()),
                                 0, 0,
                                 (m_volatileKeys ? REG_OPTION_VOLATILE : REG_OPTION_NON_VOLATILE),
                                 KEY_ALL_ACCESS, 0, &key, &disposition);

    if (result == ERROR_SUCCESS)
        hKeys.insert(handle, key);

    return disposition == REG_CREATED_NEW_KEY;
}

bool RegistryLayer::removeSubTree(QValueSpacePublisher *creator, Handle handle)
{
    QMutexLocker locker(&localLock);

    RegistryHandle *rh = registryHandle(handle);
    if (!rh)
        return false;

    QList<QString> paths = creators.value(creator);

    while (!paths.isEmpty()) {
        QString item = paths.takeFirst();
        if (!item.startsWith(rh->path))
            continue;

        removeRegistryValue(0, item);
        creators[creator].removeOne(item);

        int index = item.lastIndexOf(QLatin1Char('/'));
        if (index == -1)
            continue;

        item.truncate(index);
        if (!paths.contains(item))
            paths.append(item);
    }

    pruneEmptyKeys(rh);

    return true;
}

void RegistryLayer::pruneEmptyKeys(RegistryHandle *handle)
{
    QMutexLocker locker(&localLock);

    if (!children(Handle(handle)).isEmpty())
        return;

    QString path = handle->path;

    while (path != QLatin1String("/")) {
        int index = path.lastIndexOf(QLatin1Char('/'), -1);

        QString value = path.mid(index + 1);

        path.truncate(index);
        if (path.isEmpty())
            path.append(QLatin1Char('/'));

        RegistryHandle *rh = registryHandle(item(InvalidHandle, path));

        openRegistryKey(rh);
        if (!hKeys.contains(rh)) {
            removeHandle(Handle(rh));
            return;
        }

        HKEY key = hKeys.value(rh);

        long result = RegDeleteKey(key, reinterpret_cast<const wchar_t*>(value.utf16()));
        if (result == ERROR_SUCCESS) {
            QList<QString> paths = handles.keys();
            while (!paths.isEmpty()) {
                const QString p = paths.takeFirst();

                if (p.startsWith(path))
                    closeRegistryKey(handles.value(p));
            }
        } else if (result != ERROR_FILE_NOT_FOUND) {
            return;
        }

        bool hasChildren = !children(Handle(rh)).isEmpty();

        removeHandle(Handle(rh));

        if (hasChildren)
            break;
    }
}

bool RegistryLayer::removeValue(QValueSpacePublisher *creator,
                                Handle handle,
                                const QString &subPath)
{
    QMutexLocker locker(&localLock);

    QString fullPath;

    if (handle == InvalidHandle) {
        fullPath = subPath;
    } else {
        RegistryHandle *rh = registryHandle(handle);
        if (!rh)
            return false;

        if (subPath == QLatin1String("/"))
            fullPath = rh->path;
        else if (rh->path.endsWith(QLatin1Char('/')) && subPath.startsWith(QLatin1Char('/')))
            fullPath = rh->path + subPath.mid(1);
        else if (!rh->path.endsWith(QLatin1Char('/')) && !subPath.startsWith(QLatin1Char('/')))
            fullPath = rh->path + QLatin1Char('/') + subPath;
        else
            fullPath = rh->path + subPath;
    }

    // permanent layer always removes items even if our records show that creator does not own it.
    if (!creators[creator].contains(fullPath) && (layerOptions() & QValueSpace::TransientLayer))
        return false;

    removeRegistryValue(0, fullPath);
    creators[creator].removeOne(fullPath);

    return true;
}

void RegistryLayer::addWatch(QValueSpacePublisher *, Handle)
{
}

void RegistryLayer::removeWatches(QValueSpacePublisher *, Handle)
{
}

bool RegistryLayer::supportsInterestNotification() const
{
    return false;
}

bool RegistryLayer::notifyInterest(Handle, bool)
{
    return false;
}

QT_END_NAMESPACE

#endif // Q_OS_WIN
