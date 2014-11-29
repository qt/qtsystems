/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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
