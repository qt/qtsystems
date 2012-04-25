/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdisplayinfo_linux_p.h"

#if !defined(QT_NO_JSONDB)
#include "qjsondbwrapper_p.h"
#endif // QT_NO_JSONDB

#include <QtCore/qdir.h>
#include <QtCore/qmap.h>
#include <QtCore/qmetaobject.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(const QString, BACKLIGHT_SYSFS_PATH, (QStringLiteral("/sys/class/backlight/")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, GRAPHICS_SYSFS_PATH, (QStringLiteral("/sys/class/graphics/")))

QDisplayInfoPrivate::QDisplayInfoPrivate(QDisplayInfo *parent)
    : q_ptr(parent)
#if !defined(QT_NO_JSONDB)
    , backlightStateWatcher(false)
    , jsondbWrapper(0)
#endif // QT_NO_JSONDB
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
#if !defined(QT_NO_JSONDB)
    if (backlightStates.size() == 0)
        initScreenMap();
    if (backlightStates[screen] == QDisplayInfo::BacklightUnknown) {
        int actualBrightness = brightness(screen);

        if (actualBrightness > 0)
            backlightStates[screen] = QDisplayInfo::BacklightOn;
        else if (actualBrightness == 0)
            backlightStates[screen] = QDisplayInfo::BacklightOff;
        if (!jsondbWrapper) {
            jsondbWrapper = new QJsonDbWrapper(this);
            connect(jsondbWrapper, SIGNAL(backlightStateChanged(int, QDisplayInfo::BacklightState)), this, SLOT(onBacklightStateChanged(int, QDisplayInfo::BacklightState)) , Qt::UniqueConnection);
        }
    }
    return backlightStates[screen];
#else
    int actualBrightness = brightness(screen);

    if (actualBrightness > 0)
        return QDisplayInfo::BacklightOn;
    else if (actualBrightness == 0)
        return QDisplayInfo::BacklightOff;
    else
        return QDisplayInfo::BacklightUnknown;
#endif // QT_NO_JSONDB
}


#if !defined(QT_NO_JSONDB)
void QDisplayInfoPrivate::initScreenMap()
{
    const QStringList dirs = QDir(*GRAPHICS_SYSFS_PATH()).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < dirs.size(); i++)
       backlightStates.insert(i, QDisplayInfo::BacklightUnknown);
}

void QDisplayInfoPrivate::onBacklightStateChanged(int screen, QDisplayInfo::BacklightState state)
{
    if (backlightStates[screen] != state) {
        backlightStates[screen] = state;
        if (backlightStateWatcher)
           emit backlightStateChanged(screen, state);
    }
}

void QDisplayInfoPrivate::connectNotify(const QMetaMethod &signal)
{
    static const QMetaMethod backlightStateChangedSignal = QMetaMethod::fromSignal(&QDisplayInfoPrivate::backlightStateChanged);
    if (!backlightStateWatcher && signal == backlightStateChangedSignal) {
       backlightStateWatcher = true;
       if (backlightStates.size() == 0)
          backlightState(0);
    }
}

void QDisplayInfoPrivate::disconnectNotify(const QMetaMethod &signal)
{
    static const QMetaMethod backlightStateChangedSignal = QMetaMethod::fromSignal(&QDisplayInfoPrivate::backlightStateChanged);
    if (signal == backlightStateChangedSignal && jsondbWrapper) {
        backlightStateWatcher = false;
    }
}
#endif // QT_NO_JSONDB

QT_END_NAMESPACE
