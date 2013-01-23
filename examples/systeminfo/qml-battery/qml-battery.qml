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
    width: Screen.width
    height: Screen.height

    MouseArea {
        anchors.fill: parent
        onClicked: { Qt.quit() }
    }

    Row {
        anchors.centerIn: parent
        BatteryInfo {
            id: batinfo

            monitorChargerType: true
            monitorCurrentFlow: true
            monitorRemainingCapacity: true
            monitorRemainingChargingTime: true
            monitorVoltage: true
            monitorChargingState: true
            monitorBatteryStatus: true

            onChargerTypeChanged: {
                if (type == 1) {
                    chargertype.text = "Wall Charger"
                } else if (type == 2) {
                    chargertype.text = "USB Charger"
                } else if (type == 3) {
                    chargertype.text = "Variable Current Charger"
                } else {
                    chargertype.text = "Unknown Charger"
                }
            }

            onCurrentFlowChanged: {
                /* battery parameter skipped */
                currentflow.text = flow + " mA"
            }

            onRemainingCapacityChanged: {
                /* battery parameter skipped */
                remainingcapacity.text = capacity + getEnergyUnit()
                updateBatteryLevel()
            }

            onRemainingChargingTimeChanged: {
                /* battery parameter skipped */
                remainingchargingtime.text = seconds + " s"
            }

            onVoltageChanged: {
                /* battery parameter skipped */
                voltagetext.text = voltage + " mV"
            }

            onChargingStateChanged: {
                /* battery parameter skipped */
                if (state == 1) {
                    chargeState.text = "Not Charging"
                } else if (state == 2) {
                    chargeState.text = "Charging"
                } else if (state == 3) {
                chargeState.text = "Discharging"
                } else {
                    chargeState.text = "Unknown"
                }
            }

            onBatteryStatusChanged: {
                /* battery parameter skipped */
                if (status == 1) {
                    batStat.text = "Empty"
                } else if (status == 2) {
                    batStat.text = "Low"
                } else if (status == 3) {
                    batStat.text = "Ok"
                } else if (status == 4) {
                    batStat.text = "Full"
                } else {
                    batStat.text = "Unknown"
                }
            }

            function getEnergyUnit() {
                              if (energyUnit == 1) {
                                  return " mAh"
                              } else if (energyUnit == 2) {
                                  return " mWh"
                            } else {
                                  return " ???"
                            }
            }

            function updateBatteryLevel() {
                var battery = 0
                level.text = (100/batinfo.maximumCapacity(battery)*batinfo.remainingCapacity(battery)).toFixed(1) + "%"
            }

            Component.onCompleted: {
                var battery = 0
                onChargerTypeChanged(chargerType)
                onCurrentFlowChanged(battery, currentFlow(battery))
                onRemainingCapacityChanged(battery, remainingCapacity(battery))
                onRemainingChargingTimeChanged(battery, remainingChargingTime(battery))
                onVoltageChanged(battery, voltage(battery))
                onChargingStateChanged(battery, chargingState(battery))
                onBatteryStatusChanged(battery, batteryStatus(battery))
                maximum.text = maximumCapacity(battery) + getEnergyUnit()
            }
        }

          Column {
              Text { text: "Battery level:" }
              Text { text: "Current flow:" }
              Text { text: "Maximum capacity:" }
              Text { text: "Remaining capacity:" }
              Text { text: "Battery state:" }
              Text { text: "Remaining charging time:" }
              Text { text: "Voltage:" }
              Text { text: "Charge state:" }
              Text { text: "Charger type:" }
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
          }
    }
}
