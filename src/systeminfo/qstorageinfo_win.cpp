/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qstorageinfo_win_p.h"

#include <QtCore/qdir.h>
#include <QtCore/qmetaobject.h>

#include <windows.h>

QT_BEGIN_NAMESPACE

QStorageInfoPrivate::QStorageInfoPrivate(QStorageInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
{
}

QStorageInfoPrivate::~QStorageInfoPrivate()
{
}

qlonglong QStorageInfoPrivate::availableDiskSpace(const QString &drive)
{
    qlonglong availableBytes(-1);
    if (!GetDiskFreeSpaceEx((WCHAR *)drive.utf16(), 0, 0, (PULARGE_INTEGER)&availableBytes))
        availableBytes = -1;
    return availableBytes;
}

qlonglong QStorageInfoPrivate::totalDiskSpace(const QString &drive)
{
    qlonglong totalBytes(-1);
    if (!GetDiskFreeSpaceEx((WCHAR *)drive.utf16(), 0, (PULARGE_INTEGER)&totalBytes, 0))
        totalBytes = -1;
    return totalBytes;
}

QString QStorageInfoPrivate::uriForDrive(const QString &drive)
{
    WCHAR uri[50];
    if (GetVolumeNameForVolumeMountPoint((WCHAR *)drive.utf16(), uri, 50))
        return QString::fromUtf16(reinterpret_cast<const unsigned short *>(uri));
    return QString();
}

QStringList QStorageInfoPrivate::allLogicalDrives()
{
    QFileInfoList drives = QDir::drives();
    QStringList drivesList;
    foreach (const QFileInfo &drive, drives)
        drivesList << drive.absoluteFilePath();
    return drivesList;
}

QStorageInfo::DriveType QStorageInfoPrivate::driveType(const QString &drive)
{
    UINT type = GetDriveType((WCHAR *)drive.utf16());
    switch (type) {
    case DRIVE_REMOVABLE:
        return QStorageInfo::RemovableDrive;
    case DRIVE_FIXED:
        return QStorageInfo::InternalDrive;
    case DRIVE_REMOTE:
        return QStorageInfo::RemoteDrive;
    case DRIVE_CDROM:
        return QStorageInfo::CdromDrive;
    case DRIVE_RAMDISK:
        return QStorageInfo::RamDrive;
    case DRIVE_UNKNOWN:
    case DRIVE_NO_ROOT_DIR:
    default:
        return QStorageInfo::UnknownDrive;
    };
}

void QStorageInfoPrivate::connectNotify(const QMetaMethod &signal)
{
    static const QMetaMethod logicalDriveChangedSignal = QMetaMethod::fromSignal(&QStorageInfoPrivate::logicalDriveChanged);
    if (signal == logicalDriveChangedSignal) {
    }
}

void QStorageInfoPrivate::disconnectNotify(const QMetaMethod &signal)
{
    static const QMetaMethod logicalDriveChangedSignal = QMetaMethod::fromSignal(&QStorageInfoPrivate::logicalDriveChanged);
    if (signal == logicalDriveChangedSignal) {
    }
}

QT_END_NAMESPACE
