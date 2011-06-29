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

#include "qstorageinfo_linux_p.h"

#include <QtCore/qfile.h>
#include <QtCore/qsocketnotifier.h>

#include <errno.h>
#include <mntent.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/statfs.h>

#if !defined(QT_NO_BLKID)
#include <blkid/blkid.h>
#endif // QT_NO_BLKID

QT_BEGIN_NAMESPACE

QStorageInfoPrivate::QStorageInfoPrivate(QStorageInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
    , inotifyWatcher(-1)
    , inotifyFileDescriptor(-1)
    , notifier(0)
{
}

QStorageInfoPrivate::~QStorageInfoPrivate()
{
    cleanupWatcher();
}

qlonglong QStorageInfoPrivate::availableDiskSpace(const QString &drive)
{
    struct statfs statistics;
    if (statfs(drive.toLatin1(), &statistics) == 0) {
        qlonglong blockSize = statistics.f_bsize;
        qlonglong availBlocks = statistics.f_bavail;
        return availBlocks * blockSize;
    }

    return -1;
}

qlonglong QStorageInfoPrivate::totalDiskSpace(const QString &drive)
{
    struct statfs statistics;
    if (statfs(drive.toLatin1(), &statistics) == 0) {
        qlonglong blockSize = statistics.f_bsize;
        qlonglong totalBlocks = statistics.f_blocks;
        return totalBlocks * blockSize;
    }

    return -1;
}

QString QStorageInfoPrivate::uriForDrive(const QString &drive)
{
    QString uri;

#if !defined(QT_NO_BLKID)
    FILE *fsDescription = setmntent(_PATH_MOUNTED, "r");
    mntent *entry = NULL;
    while ((entry = getmntent(fsDescription)) != NULL) {
        if (drive != QString::fromAscii(entry->mnt_dir))
            continue;

        int fd = open(entry->mnt_fsname, O_RDONLY);
        if (fd == -1)
            break;

        uint64_t size;
        const char *label;
        blkid_probe probe = blkid_new_probe();
        if (blkid_probe_set_request(probe, BLKID_PROBREQ_UUID) == 0
            && ioctl(fd, BLKGETSIZE64, &size) == 0
            && blkid_probe_set_device(probe, fd, 0, size) == 0
            && blkid_do_safeprobe(probe) == 0
            && blkid_probe_lookup_value(probe, "UUID", &label, NULL) == 0) {
            uri = QString::fromAscii(label);
        }
        blkid_free_probe(probe);
        close(fd);
        break;
    }

    endmntent(fsDescription);
#else
    Q_UNUSED(drive)
#endif // QT_NO_BLKID

    return uri;
}

QStringList QStorageInfoPrivate::allLogicalDrives()
{
    // No need to update the list if someone is listening to the signal, as it will be updated in that case
    if (inotifyWatcher == -1)
        updateLogicalDrives();

    return logicalDrives;
}

QStorageInfo::DriveType QStorageInfoPrivate::driveType(const QString &drive)
{
    QStorageInfo::DriveType type = QStorageInfo::UnknownDrive;
    FILE *fsDescription = setmntent(_PATH_MOUNTED, "r");
    mntent *entry = NULL;
    while ((entry = getmntent(fsDescription)) != NULL) {
        if (drive != QString::fromAscii(entry->mnt_dir))
            continue;

        if (strcmp(entry->mnt_type, "binfmt_misc") == 0
            || strcmp(entry->mnt_type, "debugfs") == 0
            || strcmp(entry->mnt_type, "devpts") == 0
            || strcmp(entry->mnt_type, "devtmpfs") == 0
            || strcmp(entry->mnt_type, "fusectl") == 0
            || strcmp(entry->mnt_type, "none") == 0
            || strcmp(entry->mnt_type, "proc") == 0
            || strcmp(entry->mnt_type, "ramfs") == 0
            || strcmp(entry->mnt_type, "securityfs") == 0
            || strcmp(entry->mnt_type, "sysfs") == 0
            || strcmp(entry->mnt_type, "tmpfs") == 0) {
            type = QStorageInfo::RamDrive;
            break;
        }

        if (strcmp(entry->mnt_type, "cifs") == 0
            || strcmp(entry->mnt_type, "ncpfs") == 0
            || strcmp(entry->mnt_type, "nfs") == 0
            || strcmp(entry->mnt_type, "nfs4") == 0
            || strcmp(entry->mnt_type, "smbfs") == 0) {
            type = QStorageInfo::RemoteDrive;
            break;
        }

        if (strcmp(entry->mnt_type, "iso9660") == 0) {
            type = QStorageInfo::CdromDrive;
            break;
        }

        // Now need to guess if it's InternalDrive, RemovableDrive, or InternalFlashDrive
        bool isMmc = false;
        QString fsName(QString::fromAscii(entry->mnt_fsname));
        if (fsName.contains(QString::fromAscii("mapper"))) {
            struct stat status;
            stat(entry->mnt_fsname, &status);
            fsName = QString::fromAscii("/sys/block/dm-%1/removable").arg(status.st_rdev & 0377);
        } else {
            fsName = fsName.section(QString::fromAscii("/"), 2, 3);
            if (!fsName.isEmpty()) {
                if (fsName.left(3) == QString::fromAscii("mmc"))
                    isMmc = true;

                if (fsName.length() > 3) {
                    fsName.chop(1);
                    if (fsName.right(1) == QString::fromAscii("p"))
                        fsName.chop(1);
                }
                fsName = QString::fromAscii("/sys/block/") + fsName + QString::fromAscii("/removable");
            }
        }
        QFile removable(fsName);
        char isRemovable;
        if (!removable.open(QIODevice::ReadOnly) || 1 != removable.read(&isRemovable, 1))
            break;
        if (isRemovable == '0') {
            if (isMmc)
                type = QStorageInfo::InternalFlashDrive;
            else
                type = QStorageInfo::InternalDrive;
        } else {
            type = QStorageInfo::RemovableDrive;
        }
        break;
    }

    endmntent(fsDescription);
    return type;
}

void QStorageInfoPrivate::connectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(logicalDriveChanged(QString,bool))) == 0) {
        updateLogicalDrives();
        setupWatcher();
    }
}

void QStorageInfoPrivate::disconnectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(logicalDriveChanged(QString,bool))) == 0)
        cleanupWatcher();
}

void QStorageInfoPrivate::cleanupWatcher()
{
    if (notifier) {
        delete notifier;
        notifier = 0;
    }

    if (inotifyWatcher != -1) {
        inotify_rm_watch(inotifyFileDescriptor, inotifyWatcher);
        inotifyWatcher = -1;
    }

    if (inotifyFileDescriptor != -1) {
        close(inotifyFileDescriptor);
        inotifyFileDescriptor = -1;
    }
}

void QStorageInfoPrivate::setupWatcher()
{
    if (inotifyFileDescriptor == -1
        && (inotifyFileDescriptor = inotify_init()) == -1) {
        return;
    }

    if (inotifyWatcher == -1
        && (inotifyWatcher = inotify_add_watch(inotifyFileDescriptor, _PATH_MOUNTED, IN_MODIFY)) == -1) {
        close(inotifyFileDescriptor);
        return;
    }

    if (notifier == 0) {
        notifier = new QSocketNotifier(inotifyFileDescriptor, QSocketNotifier::Read);
        connect(notifier, SIGNAL(activated(int)), this, SLOT(onInotifyActivated()));
    }
}

void QStorageInfoPrivate::updateLogicalDrives()
{
    FILE *fsDescription = setmntent(_PATH_MOUNTED, "r");
    mntent *entry = NULL;
    logicalDrives.clear();
    while ((entry = getmntent(fsDescription)) != NULL)
        logicalDrives << QString::fromAscii(entry->mnt_dir);
    endmntent(fsDescription);
}

void QStorageInfoPrivate::onInotifyActivated()
{
    inotify_event event;
    if (read(inotifyFileDescriptor, (void *)&event, sizeof(event)) > 0
        && event.wd == inotifyWatcher) {
        // Have to do this, otherwise I can't get further notification
        inotify_rm_watch(inotifyFileDescriptor, inotifyWatcher);
        inotifyWatcher = inotify_add_watch(inotifyFileDescriptor, _PATH_MOUNTED, IN_MODIFY);

        QStringList oldLogicalDrives = logicalDrives;
        updateLogicalDrives();

        foreach (const QString &drive, oldLogicalDrives) {
            if (!logicalDrives.contains(drive))
                Q_EMIT logicalDriveChanged(drive, false);
        }

        foreach (const QString &drive, logicalDrives) {
            if (!oldLogicalDrives.contains(drive))
                Q_EMIT logicalDriveChanged(drive, true);
        }
    }
}

QT_END_NAMESPACE
