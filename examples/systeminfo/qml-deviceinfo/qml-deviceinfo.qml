/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
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
import QtSystemInfo 5.0
import QtQuick.Window 2.0

Rectangle {
    width: 320; height: 480

    Flickable {
        id: flickable
        anchors.margins: 10
        anchors.leftMargin: 20
        anchors.fill: parent
        contentWidth: column.width; contentHeight: column.height
        Column {
            id: column
            spacing: 10
            Text {
                text: "Device Info:"
                x: -10
                font.bold: true
            }
            Text {
                text: "Manufacturer: " + devinfo.manufacturer()
            }
            Text {
                text: "Product name: " + devinfo.productName()
            }
            Text {
                text: "Model: " + devinfo.model()
            }
            Text {
                text: "Unique device ID: " + devinfo.uniqueDeviceID()
            }
            Text {
                text: "OS version: " + devinfo.version(DeviceInfo.Os)
            }
            Text {
                text: "Firmware version: " + devinfo.version(DeviceInfo.Firmware)
            }
            Text {
                text: "IMEI :" + devinfo.imei(0)
            }
            Text {
                text: "Thermal state: " + devinfo.thermalState
            }
            Text {
                text: "Screen Info:"
                x: -10
                font.bold: true
            }
            Text {
                text: "resolution: " + Screen.width + " x " + Screen.height;
            }
            Text {
                text: "orientation: " + Screen.orientation + " primary " + Screen.primaryOrientation;
            }
        }
    }
    Rectangle {
        id: scrollDecoratorV
        color: "grey"
        opacity: 0.5
        width: 5
        radius: width / 2
        smooth: true
        property real ratio: flickable.height / column.height
        height: flickable.height * ratio
        y: flickable.anchors.topMargin + flickable.contentY * ratio
        anchors.right: parent.right
        anchors.margins: 2
        visible: flickable.height < column.height
    }
    Rectangle {
        id: scrollDecoratorH
        color: "grey"
        opacity: 0.5
        height: 5
        radius: height / 2
        smooth: true
        property real ratio: flickable.width / column.width
        width: flickable.width * ratio
        x: flickable.anchors.topMargin + flickable.contentX * ratio
        anchors.bottom: parent.bottom
        anchors.margins: 2
        visible: flickable.width < column.width
    }

    DeviceInfo {
        id: devinfo;
    }
}
