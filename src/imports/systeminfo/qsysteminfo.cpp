/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>

#include "qbatteryinfo.h"
#include "qdeclarativedeviceinfo_p.h"
#include "qdeclarativenetworkinfo_p.h"
#if defined(Q_OS_LINUX)
#include "qdeclarativeinputdevicemodel_p.h"
#endif
#include <qscreensaver.h>
#include "qinputinfo.h"

QT_BEGIN_NAMESPACE

class QSystemInfoDeclarativeModule : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface" FILE "systeminfo.json")

public:
    virtual void registerTypes(const char *uri)
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtSystemInfo"));

        int major = 5;
        int minor = 0;
        qmlRegisterType<QBatteryInfo>(uri, major, minor, "BatteryInfo");
        qmlRegisterType<QDeclarativeDeviceInfo>(uri, major, minor, "DeviceInfo");
        qmlRegisterType<QDeclarativeNetworkInfo>(uri, major, minor, "NetworkInfo");
        qmlRegisterType<QScreenSaver>(uri, major, minor, "ScreenSaver");

#if defined(Q_OS_LINUX)
        minor = 5;
        qmlRegisterType<QInputInfoManager>(uri, major, minor, "InputDeviceManager");
        qmlRegisterType<QDeclarativeInputDeviceModel>(uri, major, minor, "InputDeviceModel");
        qmlRegisterType<QInputDevice>(uri, major, minor, "InputInfo");
#endif
    }
};

QT_END_NAMESPACE

#include "qsysteminfo.moc"

/*!
    \qmltype ScreenSaver
    \instantiates QScreenSaver
    \inqmlmodule QtSystemInfo
    \ingroup qml-systeminfo
    \brief The ScreenSaver element provides information about the screen saver.
*/

/*!
    \qmlproperty bool ScreenSaver::screenSaverEnabled

    This property holds whether or not the screen saver is enabled.

    On certain platforms, if screen saver is disabled, deep system sleep won't be automatically triggered,
    and the display won't be automatically turned off, etc.
*/
