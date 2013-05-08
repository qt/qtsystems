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

#include "qdisplayinfo_linux_p.h"

#include <QtCore/qdir.h>
#include <QtCore/qmap.h>
#include <QtCore/qmetaobject.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(const QString, BACKLIGHT_SYSFS_PATH, (QLatin1String("/sys/class/backlight/")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, GRAPHICS_SYSFS_PATH, (QLatin1String("/sys/class/graphics/")))

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

    if (actualBrightness > 0)
        return QDisplayInfo::BacklightOn;
    else if (actualBrightness == 0)
        return QDisplayInfo::BacklightOff;
    else
        return QDisplayInfo::BacklightUnknown;
}

QT_END_NAMESPACE
