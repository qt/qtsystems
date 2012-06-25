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
import "content"

Rectangle {
    width: 480; height: 854
    color: "#343434"

    StorageInfo {
        id: storageinfo
        onLogicalDriveChanged: updateList;
    }

    function getTotalSizeText(name) {
        var totalSpace = storageinfo.totalDiskSpace(name);
        if (totalSpace / 1024 < 1024)
            return Math.round((totalSpace / 1024) * 100) / 100 + " kb avail.";
        else if (totalSpace/1024/1024 < 1024)
            return Math.round((totalSpace / 1024 / 1024) * 100) / 100 + " Mb avail.";
        else if (totalSpace/1024/1024/1024 < 1024)
            return Math.round((totalSpace / 1024 / 1024 / 1024) * 100) / 100 + " Gb avail.";
        return "";
    }

    function getAvailableSizeText(name) {
        var dspace = storageinfo.availableDiskSpace(name);
        if (dspace /1024 < 1024)
            return Math.round((dspace / 1024) * 100) / 100 + " kb / ";
        else if (dspace/1024/1024 < 1024)
            return Math.round((dspace / 1024 / 1024) * 100) / 100 + " Mb / ";
        else if (dspace/1024/1024/1024 < 1024)
            return Math.round((dspace / 1024 / 1024 / 1024) * 100) / 100 + " Gb / ";
        return "";
    }

    function getPercent(name) {
        return Math.round(100 - ((storageinfo.availableDiskSpace(name) / storageinfo.totalDiskSpace(name)) * 100))
    }

    property alias storageList: storageinfo.allLogicalDrives;

    function updateList() {
        driveList:storageList: storageinfo.allLogicalDrives;
    }

    Component {
        id: listItem

        Row {
            id:row
            spacing: 10
            ProgressBar {
                width: 120; height: 25
                maxval: getPercent(name.text)
                value: getPercent(name.text)
                NumberAnimation on value { duration: 1500; from: 0; to: getPercent(name.text); loops: 1 }
                ColorAnimation on color { duration: 1500; from: "lightsteelblue"; to: "thistle"; loops:1}
                ColorAnimation on secondColor { duration: 1500; from: "steelblue"; to: "#CD96CD"; loops: 1 }
            }
            Text { id: name; text: modelData; color: "white";}
            Text { text: getAvailableSizeText(name.text) + getTotalSizeText(name.text); color: "white";}
        }
    }

    ListView {
        anchors.fill: parent
        model: storageList
        delegate: listItem
    }
}
