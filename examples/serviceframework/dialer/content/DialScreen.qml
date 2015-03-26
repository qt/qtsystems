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

//Layout of the DialScreen control
//------------------------------------
//|DialScreen                        |
//| ---------------------  ____  numberPad
//| |dialNumber         | /          |
//| ---------------------/          _|    hangUpButton
//| --------------------/- ------- / |
//| |      |      |      | |     |/  |
//| |   1  |   2  |   3  | |     |   |
//| |      |      |      | |     |   |
//| ---------------------- |     |   |
//| |      |      |      | |     |   |
//| |   4  |   5  |   6  | |     |   |
//| |      |      |      | -------   |
//| ----------------------          _|    callButton
//| |      |      |      | ------- / |
//| |   7  |   8  |   9  | |     |/  |
//| |      |      |      | |     |   |
//| ---------------------- |     |   |
//| |      |      |      | |     |   |
//| |   *  |   0  |   #  | |     |   |
//| |      |      |      | |     |   |
//| ---------------------- -------   |
//|                                  |
//------------------------------------

//! [0]
Item {
    width: childrenRect.width
    height: childrenRect.height
    property string dialString
    signal dial(string numberToDial)
    signal hangup
    //! [0]

    Rectangle {
        id: dialNumber
        height: dialText.height + 5
        width: numberPad.width
        anchors.top: parent.top
        anchors.left: parent.left
        color: "white"
        radius: 5
        border.width: 3
        border.color: "black"

        Text {
            id: dialText
            text: dialString
            anchors.centerIn: parent
        }
    }

    Grid {
        id: numberPad
        width: childrenRect.width
        height: childrenRect.height
        anchors.top: dialNumber.bottom
        anchors.left: parent.left
        anchors.topMargin: 5
        columns: 3
        spacing: 5

        DialButton { buttonText: "1"; onClicked: { dialString += "1";} }
        DialButton { buttonText: "2"; onClicked: { dialString += "2";} }
        DialButton { buttonText: "3"; onClicked: { dialString += "3";} }
        DialButton { buttonText: "4"; onClicked: { dialString += "4";} }
        DialButton { buttonText: "5"; onClicked: { dialString += "5";} }
        DialButton { buttonText: "6"; onClicked: { dialString += "6";} }
        DialButton { buttonText: "7"; onClicked: { dialString += "7";} }
        DialButton { buttonText: "8"; onClicked: { dialString += "8";} }
        DialButton { buttonText: "9"; onClicked: { dialString += "9";} }
        DialButton { buttonText: "*"; onClicked: { dialString += "*";} }
        DialButton { buttonText: "0"; onClicked: { dialString += "0";} }
        DialButton { buttonText: "#"; onClicked: { dialString += "#";} }
    }

    //! [1]
    DialButton {
        id: hangUpButton
        height: { (numberPad.height / 2) - 2 }
        width: 50
        anchors.top: numberPad.top
        anchors.left: numberPad.right
        anchors.leftMargin: 5
        color: "crimson"
        onClicked: {
            dialString = ""
            hangup()
        }
        //! [1]
        Image {
            anchors.centerIn: parent
            source: "hangup.png"
            transformOrigin: "Center"
        }
    }

    //! [2]
    DialButton {
        id: callButton
        width: 50
        height: {(numberPad.height/2) -2}
        anchors.top: hangUpButton.bottom
        anchors.left: numberPad.right
        anchors.leftMargin: 5
        anchors.topMargin: 4
        color: "mediumseagreen"
        onClicked: {
            if (dialString != "") {
                dial(dialString)
                dialString = ""
            }
        }
        //! [2]

        Image {
            anchors.centerIn: parent
            source: "call.png"
            transformOrigin: "Center"
        }
    }
}
