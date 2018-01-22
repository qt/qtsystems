/****************************************************************************
**
** Copyright (C) 2018 Canonical, Ltd. and/or its subsidiary(-ies).
** Copyright (C) 2018 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
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
