/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the test suite of the Qt Toolkit.
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

import QtQuick 2.0
import QtTest 1.0
import QtPublishSubscribe 5.0

TestCase {
    name: "ReadSettingsObject"
    id: readSettingsObjectTests

    ValueSpaceSubscriber {
        id: appSettingsObject
        path: "/org/qt-project/test/app/read/settings/object"
    }

    ValueSpaceSubscriber {
        id: sysSettingsObject
        path: "/org/qt-project/test/sys/read/settings/object"
    }

    TestTools {
        id: jsonDbHandler
    }

    function init() {
        var settingsObjects = [
            {
                "identifier": "org.qt-project.test.app.read.settings.object",
                "version": "1.0.0",
                "_type": "com.nokia.mt.settings.ApplicationSettings",
                "settings": {
                    "username": "Fred",
                    "password": "Bedrock",
                    "active": false
                }
            },
            {
                "identifier": "org.qt-project.test.sys.read.settings.object",
                "version": "1.0.0",
                "_type": "com.nokia.mt.settings.SystemSettings",
                "settings": {
                    "username": "Noname",
                    "password": "Password",
                    "active": true
                }
            }
        ]

        jsonDbHandler.createObjects(settingsObjects)
    }

    function cleanup() {
        jsonDbHandler.cleanUpJsonDb()
    }

    function test_read_settings_object() {
        var obj = appSettingsObject.value
        verify(obj.identifier === "org.qt-project.test.app.read.settings.object")
        verify(obj.version === "1.0.0")
        verify(obj._type === "com.nokia.mt.settings.ApplicationSettings")
        verify(obj.settings.username === "Fred")
        verify(obj.settings.password === "Bedrock")
        verify(!obj.settings.active)

        obj = sysSettingsObject.value
        verify(obj.identifier === "org.qt-project.test.sys.read.settings.object")
        verify(obj.version === "1.0.0")
        verify(obj._type === "com.nokia.mt.settings.SystemSettings")
        verify(obj.settings.username === "Noname")
        verify(obj.settings.password === "Password")
        verify(obj.settings.active)
    }
}
