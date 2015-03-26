/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Mobility Components.
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
// ![4]
import QtServiceFramework 5.0
// ![4]

//Layout of the ServiceList control
//---------------------------------
//|ServiceList                    |
//| ----------------------------- |
//| |title                      | |
//| ----------------------------- |
//| ----------------------------- |
//| |listFrame                  | |
//| |-------------------------- | |
//| ||serviceListView         | | |
//| ||- listItem              | | |
//| ||- listItem              | | |
//| ||- listItem              | | |
//| |---------------------------| |
//| ----------------------------- |
//---------------------------------

Rectangle {
    id: serviceListControl
    property QtObject dialService
    signal signalSelected
    property bool allowselction: true
    property bool nohighlightlistitem : true

    Text {
        id: title
        width: parent.width
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 5
        anchors.leftMargin: 5
        text: "<b>Select dial service:</b>"
    }

    //! [1]
    Component {
        id: delegateComponent
        //! [1]
        //Rectangle item to draw a list view item
        //This includes 2 line of text:
        // ------------------------------------------
        // |Service: LandDialer (1.0)               |
        // |Interface: com.nokia.qt.examples Dialer |
        // ------------------------------------------
        Rectangle {
            id: listItem
            width: serviceListView.width-20
            height: serviceItemInfo.height + serviceItemInterfaceName.height
            border.color: "black"
            border.width: 1
            opacity: 0.6
            ServiceLoader {
                id: listService
                serviceDescriptor: model.modelData
                //TODO: Async loading handling in case they have *really* fast clicks
            }

            //! [2]
            MouseArea {
                id: listItemMouseRegion
                anchors.fill: parent
                onClicked: {
                   if (serviceListControl.allowselction) {
                            if (serviceListControl.nohighlightlistitem) {
                                serviceListView.highlight = highlight
                                serviceListControl.nohighlightlistitem = false;
                            }
                            serviceListView.currentIndex = index;
                            dialService = listService;
                            signalSelected()
                   }
                }
            }

            Text {
                id: serviceItemInfo
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.topMargin: 5
                anchors.leftMargin: 3
                text: " <b>Service:</b> " + serviceName + "  (" +
                                            majorVersion + "." +
                                            minorVersion + ")"
            }

            Text {
                id: serviceItemInterfaceName
                anchors.top: serviceItemInfo.bottom
                anchors.left: parent.left
                anchors.topMargin: 2
                anchors.leftMargin: 3
                text: " <b>Interface:</b> " + interfaceName;
            }
            //! [2]
        }
    }

    //! [3]
    Component {
        id: highlight

        Rectangle {
            width: childrenRect.width
            border.color: "black"; border.width: 2
            height: 30
            color : "lightsteelblue"
            gradient: Gradient {
                GradientStop {position: 0.0; color: "steelblue"}
                GradientStop {position: 0.5; color: "lightsteelblue"}
                GradientStop {position: 1.0; color: "steelblue"}
            }
        }
    }
    //! [3]

    //! [0]
    ListView {
        id: serviceListView
        height: 50
        width: mainPage.width-10
        anchors.top:  title.bottom
        anchors.left: title.left
        anchors.topMargin: 0
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        model: dialerServiceList.serviceDescriptions
        opacity: 1
        delegate: delegateComponent
//            currentIndex: -1
//            clip: true
    }
    //! [0]

    //! [5]
    ServiceFilter {
        id: dialerServiceList
        interfaceName: "com.nokia.qt.examples.Dialer"
        majorVersion: 1
        minorVersion: 0
    }
    //! [5]

    Connections {
        target: dialerList.dialService
        ignoreUnknownSignals: true
        onError: ipcFailure(errorString);
    }
}
