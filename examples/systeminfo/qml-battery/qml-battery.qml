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
    width: 640
    height: 640

    MouseArea {
        anchors.fill: parent
        onClicked: { Qt.quit() }
    }

    Row {
        anchors.centerIn: parent
        BatteryInfo {
            id: batinfo

            onChargerTypeChanged: {
                if (type == BatteryInfo.WallCharger) {
                    chargertype.text = "Wall Charger"
                } else if (type == BatteryInfo.USBCharger) {
                    chargertype.text = "USB Charger"
                } else if (type == BatteryInfo.VariableCurrentCharger) {
                    chargertype.text = "Variable Current Charger"
                } else {
                    chargertype.text = "Unknown Charger"
                }
            }

            onCurrentFlowChanged: {
                currentflow.text = flow + " mA"
            }

            onRemainingCapacityChanged: {
                remainingcapacity.text = capacity
                updateBatteryLevel()
            }

            onRemainingChargingTimeChanged: {
                remainingchargingtime.text = seconds + " s"
            }

            onVoltageChanged: {
                voltagetext.text = voltage + " mV"
            }

            onChargingStateChanged: {
                if (state == BatteryInfo.Charging) {
                    chargeState.text = "Charging"
                } else if (state == BatteryInfo.IdleChargingState) {
                    chargeState.text = "Idle Charging"
                } else if (state == BatteryInfo.Discharging) {
                    chargeState.text = "Discharging"
                } else {
                    chargeState.text = "Unknown"
                }
            }

            onLevelStatusChanged: {
                if (levelStatus == BatteryInfo.LevelEmpty) {
                    batStat.text = "Empty"
                } else if (levelStatus == BatteryInfo.LevelLow) {
                    batStat.text = "Low"
                } else if (levelStatus == BatteryInfo.LevelOk) {
                    batStat.text = "Ok"
                } else if (levelStatus == BatteryInfo.LevelFull) {
                    batStat.text = "Full"
                } else {
                    batStat.text = "Unknown"
                }
            }

            onHealthChanged: {
                if (health == BatteryInfo.HealthOk)
                    healthState.text = "Ok"
                else if (health == BatteryInfo.HealthBad)
                    healthState.text = "Bad"
                else
                    healthState.text = "Unknown"
            }

            function updateBatteryLevel() {
                level.text = (100 / batinfo.maximumCapacity * batinfo.remainingCapacity).toFixed(1) + "%"
            }

            Component.onCompleted: {
                onChargerTypeChanged(batinfo.chargerType)
                onCurrentFlowChanged(batinfo.currentFlow)
                onRemainingCapacityChanged(batinfo.remainingCapacity)
                onRemainingChargingTimeChanged(batinfo.remainingChargingTime)
                onVoltageChanged(batinfo.voltage)
                onChargingStateChanged(batinfo.chargingState)
                onLevelStatusChanged(batinfo.levelStatus)
                maximum.text = batinfo.maximumCapacity
                onHealthChanged(batinfo.health)
            }
        }

          Column {
              Text { text: "Battery level:" }
              Text { text: "Current flow:" }
              Text { text: "Maximum capacity:" }
              Text { text: "Remaining capacity:" }
              Text { text: "Level status:" }
              Text { text: "Remaining charging time:" }
              Text { text: "Voltage:" }
              Text { text: "Charge state:" }
              Text { text: "Charger type:" }
              Text { text: "Health:" }

          }
          Column {
              Text { id: level }
              Text { id: currentflow }
              Text { id: maximum }
              Text { id: remainingcapacity }
              Text { id: batStat }
              Text { id: remainingchargingtime }
              Text { id: voltagetext }
              Text { id: chargeState }
              Text { id: chargertype }
              Text { id: healthState }
          }
    }
}
