/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystemKit module of the Qt Toolkit.
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

#ifndef QSTORAGEINFO_H
#define QSTORAGEINFO_H

#include "qsysteminfo_p.h"

#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class QStorageInfoPrivate;

class Q_SYSTEMINFO_EXPORT QStorageInfo : public QObject
{
    Q_OBJECT

    Q_ENUMS(DriveType)

    Q_PROPERTY(QStringList allLogicalDrives READ allLogicalDrives NOTIFY logicalDriveChanged)

public:
    enum DriveType {
        UnknownDrive = 0,
        InternalDrive,
        RemovableDrive,
        RemoteDrive,
        CdromDrive,
        InternalFlashDrive,
        RamDrive
    };

    QStorageInfo(QObject *parent = 0);
    virtual ~QStorageInfo();

    static QStringList allLogicalDrives();

    Q_INVOKABLE qlonglong availableDiskSpace(const QString &drive) const;
    Q_INVOKABLE qlonglong totalDiskSpace(const QString &drive) const;
    Q_INVOKABLE QString uriForDrive(const QString &drive) const;
    Q_INVOKABLE QStorageInfo::DriveType driveType(const QString &drive) const;

Q_SIGNALS:
    void logicalDriveChanged(const QString &drive, bool added);

private:
    Q_DISABLE_COPY(QStorageInfo)
    QStorageInfoPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(QStorageInfo)
};

QT_END_NAMESPACE
QT_END_HEADER

#endif // QSTORAGEINFO_H
