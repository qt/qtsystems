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
    name: "ReadSetting"
    id: readSettingTests

    ValueSpaceSubscriber {
        id: usernameAppSubscriber
        path: "/org/qt-project/test/app/read/setting/username"
    }

    ValueSpaceSubscriber {
        id: passwordAppSubscriber
        path: "/org/qt-project/test/app/read/setting/password"
    }

    ValueSpaceSubscriber {
        id: activeAppSubscriber
        path: "/org/qt-project/test/app/read/setting/active"
    }

    ValueSpaceSubscriber {
        id: usernameSysSubscriber
        path: "/org/qt-project/test/sys/read/setting/username"
    }

    ValueSpaceSubscriber {
        id: passwordSysSubscriber
        path: "/org/qt-project/test/sys/read/setting/password"
    }

    ValueSpaceSubscriber {
        id: activeSysSubscriber
        path: "/org/qt-project/test/sys/read/setting/active"
    }

    TestTools {
        id: jsonDbHandler
    }

    function init() {
        var settingsObjects = [
            {
                "identifier": "org.qt-project.test.app.read.setting",
                "version": "1.0.0",
                "_type": "com.nokia.mt.settings.ApplicationSettings",
                "settings": {
                    "username": "Fred",
                    "password": "Bedrock",
                    "active": true
                }
            },
            {
                "identifier": "org.qt-project.test.sys.read.setting",
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

    function test_read_setting() {
        verify(usernameAppSubscriber.value === "Fred")
        verify(passwordAppSubscriber.value === "Bedrock")
        verify(activeAppSubscriber.value === true)


        verify(usernameSysSubscriber.value === "Noname")
        verify(passwordSysSubscriber.value === "Password")
        verify(activeSysSubscriber.value === true)
    }
}