/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import QtSystemInfo 5.0
import QtQuick.Particles 2.0

Rectangle {
    width: 360
    height: 360
    property alias batlevel: batinfo.battlevel;
    property int speed: level2Speed(batlevel);
    property bool hasBattery: (batinfo.batteryStatus != -1)

    MouseArea {
        anchors.fill: parent
        onClicked: {
            Qt.quit();
        }
    }

    BatteryInfo {
        id: batinfo;

        property int battlevel: remainingCapacityPercent;
        property string oldstate;

        monitorChargerTypeChanges: true
        monitorChargingStateChanges: true
        monitorBatteryStatusChanges: true
        monitorRemainingCapacityPercentChanges: true
        monitorRemainingCapacityChanges: true
        monitorRemainingChargingTimeChanges: true
        monitorCurrentFlowChanges: true

        onChargerTypeChanged:  {
//! [battery2-info1]
            if (batinfo.chargerType == -1) {
                chargertype.text = "Unknown Charger"
            }
            if (batinfo.chargerType == 0) {
                chargertype.text = "No Charger"
            }
            if (batinfo.chargerType == 1) {
                chargertype.text = "Wall Charger"
            }
            if (batinfo.chargerType == 2) {
                chargertype.text = "USB Charger"
            }
            if (batinfo.chargerType == 3) {
                chargertype.text = "USB Charger"
            }
            if (batinfo.chargerType == 5) {
                chargertype.text = "Variable Current Charger"
            }
        }

        onChargingStateChanged: {
            getPowerState();
        }
        onBatteryStatusChanged: {
            if (batinfo.batteryStatus == -1) {
                batStat.text = "Battery Unknown"
            }
            if (batinfo.batteryStatus == 0) {
                batStat.text = "Battery Empty"
            }
            if (batinfo.batteryStatus == 1) {
                batStat.text = "Battery Critical"
            }
            if (batinfo.batteryStatus == 3) {
                batStat.text = "Battery Low"
                img.width = 20; img.height = 20;
            }
            if (batinfo.batteryStatus == 4) {
                batStat.text = "Battery Ok"
            }
            if (batinfo.batteryStatus == 5) {
                batStat.text = "Battery Full"
            }
        }
//! [battery2-doBatteryLevelChange]
        onRemainingCapacityPercentChanged: doBatteryLevelChange(level)
//! [battery2-doBatteryLevelChange]
        onRemainingChargingTimeChanged: { chargeTime.text = "Time to full: "+ minutesToFull() +" minutes"; }

        property alias batState : batinfo.chargingState

        Component.onCompleted: getPowerState();
    }

    function minutesToFull() {
        if (batinfo.remainingChargingTime > 0) {
          return (batinfo.remainingChargingTime/60.00)
        }
        return 0;
    }

    function level2Speed(level) {
        if (level > 90) {
            return 1000;
        } else if (level > 70) {
            return 1500;
        } else if (level > 60) {
            return 2000;
        } else if (level > 50) {
            return 2500;
        } else if (level > 40) {
            return 3000;
        } else if (level > 10) {
            return 3500;
        } else if (level < 11) {
            return 4000;
        }
    }

//! [battery2-info2]
    function doBatteryLevelChange(level) {
        leveltext.text = "Level: "+ level +"%"
        floorParticles.burst(level);
        batlevel = level;
        batinfo.oldstate = img.state;
        img.state = "levelchange"
        //img.state = batinfo.oldstate;
        getPowerState();
    }
//! [battery2-info2]

    function getPowerState() {
        if (batinfo.chargingState == 0) {
            chargeState.text = "Charging State: Not Charging"
            if (batinfo.chargerType == 0) {
                img.state = "Battery"
                batinfo.oldstate = img.state;
            } else {
                img.state = "WallPower"
                batinfo.oldstate = img.state;
            }
        }
        if (batinfo.chargingState == 1) {
            chargeState.text = "Charging State: Charging"
            img.state = "Charging"
            batinfo.oldstate = img.state;
        }
    }

    Text {
        id: leveltext
        anchors.centerIn: parent
//! [battery2-level]
        text: "Level: "+batinfo.remainingCapacityPercent +"%"
//! [battery2-level]
    }
    Text {
        id: voltagetext
        anchors{ horizontalCenter: leveltext.horizontalCenter; top: leveltext.bottom}
        text: "Voltage: "+ batinfo.voltage +" mV"
    }
    Text {
        id: nomCap
        anchors{ horizontalCenter: leveltext.horizontalCenter; top: voltagetext.bottom}
        text: "Nominal Capacity: "+ batinfo.nominalCapacity +" "+getEnergyUnit()
    }
    Text {
        id: remCap
        anchors{ horizontalCenter: leveltext.horizontalCenter; top: nomCap.bottom}
        text: "Remaining Capacity: "+ batinfo.remainingCapacity +" "+getEnergyUnit()
    }
    Text {
        id: chargeTime
        anchors{ horizontalCenter: leveltext.horizontalCenter; top: remCap.bottom}
        text: "Time to full: "+ minutesToFull() +" minutes";

    }
    Text {
        id: curFLow
        anchors{ horizontalCenter: leveltext.horizontalCenter; top: chargeTime.bottom}
        text: "Current Energy: "+ batinfo.currentFlow +" mA"
    }

    function getEnergyUnit() {
        if (batinfo.energyMeasurementUnit == -1) {
            return "Unknown energy unit"
        }
        if (batinfo.energyMeasurementUnit == 0) {
            return "mAh"
        }
        if (batinfo.energyMeasurementUnit == 1) {
            return "mWh"
        }
    }

    Text {
        id: batStat
        anchors{ horizontalCenter: leveltext.horizontalCenter; top: curFLow.bottom}
        text: {
            if (batinfo.batteryStatus == -1) {
                batStat.text = "Battery Unknown"
            }
            if (batinfo.batteryStatus == 0) {
                batStat.text = "Battery Empty"
            }
            if (batinfo.batteryStatus == 1) {
                batStat.text = "Battery Critical"
            }
            if (batinfo.batteryStatus == 2) {
                batStat.text = "Battery Very Low"
            }
            if (batinfo.batteryStatus == 3) {
                batStat.text = "Battery Low"
            }
            if (batinfo.batteryStatus == 4) {
                batStat.text = "Battery Ok"
            }
            if (batinfo.batteryStatus == 5) {
                batStat.text = "Battery Full"
            }
        }
    }

    Text {
        id: chargertype
        anchors{ horizontalCenter: leveltext.horizontalCenter; top: batStat.bottom}
        text: {
            if (batinfo.chargerType == -1) {
                chargertype.text = "Unknown Charger"
            }
            if (batinfo.chargerType == 0) {
                chargertype.text = "No Charger"
            }
            if (batinfo.chargerType == 1) {
                chargertype.text = "Wall Charger"
            }
            if (batinfo.chargerType == 2) {
                chargertype.text = "USB Charger"
            }
            if (batinfo.chargerType == 3) {
                chargertype.text = "USB 500 mA Charger"
            }
            if (batinfo.chargerType == 4) {
                chargertype.text = "USB 100 mA Charger"
            }
            if (batinfo.chargerType == 5) {
                chargertype.text = "Variable Current Charger"
            }
        }
    }
    Text {
        id: chargeState
        anchors{ horizontalCenter: leveltext.horizontalCenter; top: chargertype.bottom}
        text: {
            if (batinfo.chargingState == -1) {
                chargeState.text = "Charging Unknown"
            }
            if (batinfo.chargingState == 0) {
                chargeState.text = "Not Charging"
            }
            if (batinfsysteminfoo.chargingState == 1) {
                chargeState.text = "Charging"
            }
        }
    }
/////////////////////////

    Particles {
        id: floorParticles
        anchors { horizontalCenter: screen.horizontalCenter; }
        y: screen.height
        width: 1
        height: 1
        source: "images/blueStar.png"
        lifeSpan: 1000
        count: batlevel
        angle: 270
        angleDeviation: 45
        velocity: 50
        velocityDeviation: 60
        ParticleMotionGravity {
            yattractor: 1000
            xattractor: 0
            acceleration: 5
        }
    }

    function particleState() {
        if (img.state == "Battery") {
            particles.burst(50,200);
        }
    }


    Image {
        id: img;
        source: "images/blueStone.png"
        smooth: true
        width: batinfo.battlevel; height: batinfo.battlevel;
        anchors {
            horizontalCenter: screen.horizontalCenter;
        }
        y: screen.height - img.height;
        Particles {
            id: particles
            width:1; height:1; anchors.centerIn: parent;
            emissionRate: 0;
            lifeSpan: 700; lifeSpanDeviation: 300;
            angle: 0; angleDeviation: 360;
            velocity: 100; velocityDeviation:30;
            source:"images/blueStar.png";
        }

        states: [
        State {
            name: "WallPower"
            when: deviceinfo.currentPowerState == 2
            StateChangeScript { script: particles.burst(50); }
            PropertyChanges {
                target: img; opacity: 1; source : "images/blueStone.png";
                anchors.horizontalCenter: undefined
                y: 0;  x: (screen.width / 2) - (img.width / 2)
            }
            PropertyChanges { target: floorParticles; count:0 }

        },
        State {
            name: "Charging"
            when: deviceinfo.currentPowerState == 3
            StateChangeScript { script: particles.burst(50); }
            PropertyChanges { target: img; y:screen.height
            }
            PropertyChanges {
                target: img; opacity: 1; source : "images/yellowStone.png";
                anchors.horizontalCenter: parent.horizontalCenter;
            }
            PropertyChanges { target: floorParticles; count:0 }
        },

        State {
            name: "Battery"
            when: deviceinfo.currentPowerState == 1
            StateChangeScript { script: particles.burst(50); }
            PropertyChanges {
                target: img; source : "images/redStone.png";
                anchors.horizontalCenter: parent.horizontalCenter;
            }
            PropertyChanges { target: floorParticles; count: batlevel }
        },

//! [battery2-level2]
        State {
            name: "levelchange"
//! [battery2-level2]
            PropertyChanges {
                target: yAnim
                running: false;
            }
//! [battery2-level3]
            PropertyChanges {
                target: bubblebounceanim
                from: screen.height
                to: screen.height - (screen.height * (batlevel / 100 ))
            }
//! [battery2-level3]
            PropertyChanges {
                target: yAnim
                running: true;
            }
        }
        ]


        transitions: [
        Transition {
            from: "*"
            to: "WallPower"
            NumberAnimation{ property: "y"; to: 0; duration: 750; easing.type: Easing.InOutQuad; }
        },
        Transition {
            from: "WallPower"
            to: "*"
            NumberAnimation{ property: "y"; to: screen.height; duration: 2000; easing.type: Easing.InOutQuad; }
        }
        ]

        SequentialAnimation on y {
            id: yAnim
            loops: Animation.Infinite
            running: img.state != "WallPower"
            NumberAnimation {
                id: bubblebounceanim;
                from: screen.height; to: screen.height - (screen.height * (batlevel / 100 ))
                easing.type: Easing.OutBounce; duration: speed
            }
            ScriptAction { script: particleState() }
            PauseAnimation { duration: 750 }
        }

        SequentialAnimation on x {
            running: img.state == "WallPower"
            loops: Animation.Infinite
            id: xanim
            NumberAnimation { target: img; property: "x"; to: screen.width - img.width; duration: 1500;
                easing.type: Easing.InOutQuad;  }
            NumberAnimation { target: img; property: "x"; to: 0; duration: 1500;
                easing.type: Easing.InOutQuad;}
        }
    }





}
