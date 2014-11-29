/****************************************************************************
**
** Copyright (C) 2016 Canonical, Ltd. and/or its subsidiary(-ies).
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/
import QtQuick 2.0
import QtSystemInfo 5.5

Rectangle {
    width: 900
    height: 480

    ListView {
        anchors.fill: parent
        id: inputList
        model: deviceModel
        delegate: deviceDelegate
        anchors.margins: 6
    }

    Component {
        id: deviceDelegate

        Rectangle {
            id: contentRect
            anchors { left: parent.left; right: parent.right }
            height: 20
            Row {
                id: row
                spacing: 10
                Text { text: name }
                Text { text: identifier }
                Text { text: typeToString(types) }
            }
        }
    }
    InputDeviceModel {
        id: deviceModel
        filter: InputInfo.Keyboard | InputInfo.Mouse | InputInfo.TouchScreen
        onCountChanged: {
         console.log("new count of filtered devices: " + devices)
        }
    }

    InputInfo {
        id: systemInputs
    }

    InputDeviceManager {
        id: deviceManager
        filter: InputInfo.Mouse | InputInfo.Keyboard
        onReady: {
            console.log("number of mouse and keyboard devices: " + deviceManager.count)
            console.log("row count " + deviceModel.count)
        }
        onDeviceAdded: {
            console.log(inputDevice.properties.name+" " + inputDevice.properties.types +" "+InputInfo.Mouse)
            // does not seem to work
            if (inputDevice.properties.types & InputInfo.Mouse) {
                console.log("mouse added")
            }
            if (inputDevice.properties.types & InputInfo.Keyboard) {
                console.log("keyboard added")
            }
        }
        onDeviceRemoved: {
            console.log("device removed "+deviceId)
        }
    }

    function typeToString(flags) {
        var typeString = ""
        if (flags & InputInfo.Button)
            typeString += "Button, "
        if (flags & InputInfo.Mouse)
            typeString += "Mouse, "
        if (flags & InputInfo.TouchPad)
            typeString += "TouchPad, "
        if (flags & InputInfo.TouchScreen)
            typeString += "TouchScreen, "
        if (flags & InputInfo.Keyboard)
            typeString += "Keyboard, "
        if (flags & InputInfo.Switch)
            typeString += "Switch "

        console.log(typeString)

        return typeString
    }
}
