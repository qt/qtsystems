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

#include "qdisplayinfo_linux_p.h"

#include <QtCore/qdir.h>
#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(const QString, BACKLIGHT_SYSFS_PATH, (QStringLiteral("/sys/class/backlight/")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, GRAPHICS_SYSFS_PATH, (QStringLiteral("/sys/class/graphics/")))

QDisplayInfoPrivate::QDisplayInfoPrivate(QDisplayInfo *parent)
    : q_ptr(parent)
{
}

int QDisplayInfoPrivate::brightness(int screen)
{
    QMap<QString, QStringList> brightnessMap;
    brightnessMap.insert(*BACKLIGHT_SYSFS_PATH(), QStringList() << QStringLiteral("/max_brightness") << QStringLiteral("/actual_brightness"));
    brightnessMap.insert(*GRAPHICS_SYSFS_PATH(), QStringList() << QStringLiteral("/backlight_max") << QStringLiteral("/backlight"));

    QStringList sysfsPaths = brightnessMap.keys();
    foreach (const QString &sysfsPath, sysfsPaths) {
        const QStringList dirs = QDir(sysfsPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        if (dirs.size() <= screen)
            continue;
        bool ok = false;
        int max = 0;
        int actual = 0;
        QFile brightness(sysfsPath + dirs.at(screen) + brightnessMap.value(sysfsPath).at(0));
        if (brightness.open(QIODevice::ReadOnly)) {
            max = brightness.readAll().simplified().toInt(&ok);
            if (!ok || max == 0)
                continue;
            brightness.close();

            brightness.setFileName(sysfsPath + dirs.at(screen) + brightnessMap.value(sysfsPath).at(1));
            if (brightness.open(QIODevice::ReadOnly)) {
                actual = brightness.readAll().simplified().toInt(&ok);
                if (!ok)
                    continue;

                return actual * 100 / max;
            }
        }
    }

    return -1;
}

int QDisplayInfoPrivate::contrast(int screen)
{
    Q_UNUSED(screen)
    return -1;
}

QDisplayInfo::BacklightState QDisplayInfoPrivate::backlightState(int screen)
{
    int actualBrightness = brightness(screen);

    if (actualBrightness == 100)
        return QDisplayInfo::BacklightOn;
    else if (actualBrightness == 0)
        return QDisplayInfo::BacklightOff;

    return QDisplayInfo::BacklightUnknown;
}

QT_END_NAMESPACE
