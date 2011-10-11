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
#include <QtGui/qpixmap.h>

QT_BEGIN_NAMESPACE

QDisplayInfoPrivate::QDisplayInfoPrivate(QDisplayInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
{
}

int QDisplayInfoPrivate::brightness(int screen)
{
    QString sysfsPath(QString::fromAscii("/sys/class/backlight/"));
    const QStringList dirs = QDir(sysfsPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (dirs.size() <= screen)
        return -1;

    bool ok = false;
    int max = 0;
    int actual = 0;
    sysfsPath += dirs.at(screen);
    QFile brightness(sysfsPath + QString::fromAscii("/max_brightness"));
    if (brightness.open(QIODevice::ReadOnly)) {
        max = brightness.readAll().simplified().toInt(&ok);
        if (!ok || max == 0)
            return -1;
        brightness.close();

        brightness.setFileName(sysfsPath + QString::fromAscii("/actual_brightness"));
        if (brightness.open(QIODevice::ReadOnly)) {
            actual = brightness.readAll().simplified().toInt(&ok);
            if (!ok)
                return -1;

            return actual * 100 / max;
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
    Q_UNUSED(screen);
    return QDisplayInfo::BacklightUnknown;
}

void QDisplayInfoPrivate::connectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(orientationChanged(int,Qt::ScreenOrientation))) == 0) {
//        watchOrientation = true;
//        int count = QGuiApplication::screens().size();
//        for (int i = 0; i < count; ++i)
//            currentOrientation[i] = getOrientation(i);
//        connect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onDesktopWidgetResized(int)));
    }
}

void QDisplayInfoPrivate::disconnectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(orientationChanged(int,Qt::ScreenOrientation))) == 0) {
//        watchOrientation = false;
//        currentOrientation.clear();
//        disconnect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onDesktopWidgetResized(int)));
    }
}

QT_END_NAMESPACE
