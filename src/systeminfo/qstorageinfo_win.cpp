/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qstorageinfo_win_p.h"

#include <QtCore/qdir.h>

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
        return QStorageInfo::InternalDrive; // Can't tell between InternalDrive and InternalFlashDrive
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

void QStorageInfoPrivate::connectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(logicalDriveChanged(QString,bool))) == 0) {
    }
}

void QStorageInfoPrivate::disconnectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(logicalDriveChanged(QString,bool))) == 0) {
    }
}

QT_END_NAMESPACE
