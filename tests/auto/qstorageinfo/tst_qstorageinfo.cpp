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

#include <QtTest/QtTest>
#include "qstorageinfo.h"

QT_USE_NAMESPACE

class tst_QStorageInfo : public QObject
{
    Q_OBJECT

private slots:
    void tst_diskSpace();
};

void tst_QStorageInfo::tst_diskSpace()
{
    QStringList drives = QStorageInfo::allLogicalDrives();
    QStorageInfo storageInfo;
    foreach (const QString &drive, drives) {
        qlonglong available = storageInfo.availableDiskSpace(drive);
        qlonglong total = storageInfo.totalDiskSpace(drive);
        QVERIFY(available <= total);

        if (available == -1 || total == -1) {
            QVERIFY(QStorageInfo::UnknownUsage == storageInfo.storageUsage(drive));
            continue;
        }
        double percent = (double)available / total;
        if (percent < 0.02)
            QVERIFY(QStorageInfo::CriticalUsage == storageInfo.storageUsage(drive));
        else if (percent < 0.1)
            QVERIFY(QStorageInfo::VeryHighUsage == storageInfo.storageUsage(drive));
        else if (percent < 0.4)
            QVERIFY(QStorageInfo::HighUsage == storageInfo.storageUsage(drive));
        else
            QVERIFY(QStorageInfo::NormalUsage == storageInfo.storageUsage(drive));
    }
}

QTEST_MAIN(tst_QStorageInfo)
#include "tst_qstorageinfo.moc"
