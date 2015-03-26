/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the test suite of the Qt Toolkit.
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
import QtTest 1.0
import QtServiceFramework 5.0

/*
    This test is a test of both a naughty SFW object and of the SFW QML API
*/

TestCase {
    id: testCase

    name: "Service Object"

    property QtObject targetObject: testObject
    property QtObject sfwObj: empty

    Item {
        id: empty
    }


    ServiceLoader {
        id: testObject
    }

    ServiceLoader {
        id: testObject2
        asynchronous: false
    }

    ServiceLoader {
        id: testObject3
    }

    SignalSpy {id: statusChangedSpy;        target: targetObject; signalName: "statusChanged"}
    SignalSpy {id: interfaceNameChangedSpy; target: targetObject; signalName: "interfaceNameChanged"}
    SignalSpy {id: descriptorChangedSpy;    target: targetObject; signalName: "serviceDescriptorChanged"}
    SignalSpy {id: serviceObjectChangedSpy; target: targetObject; signalName: "serviceObjectChanged"}
    function resetSpies() {
        statusChangedSpy.clear()
        interfaceNameChangedSpy.clear()
        descriptorChangedSpy.clear()
        serviceObjectChangedSpy.clear()
    }

    ServiceFilter {
        id: testList
    }

    SignalSpy {id:serviceListNameSpy;           target: testList; signalName: "serviceNameChanged" }
    SignalSpy {id:serviceListInterfaceSpy;      target: testList; signalName: "interfaceNameChanged" }
    SignalSpy {id:serviceListMajorVersionSpy;   target: testList; signalName: "majorVersionChanged" }
    SignalSpy {id:serviceListMinorVerisonSpy;   target: testList; signalName: "minorVersionChanged" }
    SignalSpy {id:serviceListMonitoeRegSpy;     target: testList; signalName: "monitorServiceRegistrationsChanged" }
    SignalSpy {id:serviceListServicesSpy;       target: testList; signalName: "serviceDescriptionListChanged" }

    function resetListSpies() {
        serviceListNameSpy.clear()
        serviceListInterfaceSpy.clear()
        serviceListMajorVersionSpy.clear()
        serviceListMinorVerisonSpy.clear()
        serviceListServicesSpy.clear()
    }

    function init() {
        resetListSpies()
    }

    function test_service_Async_basic() {
        // always run first, check a pristine object
        compare(serviceObjectChangedSpy.count, 0)
        compare(interfaceNameChangedSpy.count, 0)
        compare(statusChangedSpy.count, 0)
        compare(descriptorChangedSpy.count, 0)

        compare(testObject.status, ServiceLoader.Null)
        compare(testObject.errorString(), "")

        testObject.interfaceName = "com.nokia.qt.tests.serviceobject"
        compare(interfaceNameChangedSpy.count, 1)

        compare(statusChangedSpy.count, 1)
        compare(testObject.status, ServiceLoader.Loading)

        serviceObjectChangedSpy.wait(1000)

        compare(testObject.errorString(), "")
        compare(testObject.status, ServiceLoader.Ready)
        compare(statusChangedSpy.count, 2)
        compare(serviceObjectChangedSpy.count, 1)

        var obj = testObject.serviceObject

        verify(obj != 0, "Verify we received a valid object")

    }

    function test_service_Sync_basic() {
        targetObject = testObject2
        resetSpies()

        // always run first, check a pristine object
        compare(serviceObjectChangedSpy.count, 0)
        compare(interfaceNameChangedSpy.count, 0)
        compare(statusChangedSpy.count, 0)
        compare(descriptorChangedSpy.count, 0)

        compare(testObject2.status, ServiceLoader.Null)
        compare(testObject2.errorString(), "")

        testObject2.interfaceName = "com.nokia.qt.tests.serviceobject"
        compare(interfaceNameChangedSpy.count, 1)

        compare(serviceObjectChangedSpy.count, 1)
        compare(testObject2.errorString(), "")
        compare(statusChangedSpy.count, 1)
        compare(testObject2.status, ServiceLoader.Ready)

        var obj = testObject2.serviceObject

        verify(obj != 0, "Verify we received a valid object")
    }


    function test_service_invalid_and_unset() {
        resetSpies()
        targetObject = testObject3

        compare(testObject3.serviceObject, null)
        compare(testObject3.status, ServiceLoader.Null)

        resetSpies()

        testObject3.interfaceName = "com.nokia.qt.tests.invalid.name.that.should.never.be.registered.we.really.really.hope"
        compare(interfaceNameChangedSpy.count, 1)

        statusChangedSpy.wait(1000)
        statusChangedSpy.wait(1000)
        compare(statusChangedSpy.count, 2)
        compare(testObject3.status, ServiceLoader.Error)
        compare(serviceObjectChangedSpy.count, 0)
        compare(testObject3.serviceObject, null)
        //TODO: Verify correct error string

        testObject3.interfaceName = "com.nokia.qt.tests.serviceobject"
        compare(interfaceNameChangedSpy.count, 2)

        statusChangedSpy.wait(1000)
        statusChangedSpy.wait(1000)
        compare(statusChangedSpy.count, 4)
        compare(testObject3.status, ServiceLoader.Ready)
        compare(serviceObjectChangedSpy.count, 1)

        testObject3.interfaceName = ""
        compare(interfaceNameChangedSpy.count, 3)
        compare(testObject3.serviceObject, null)
        compare(serviceObjectChangedSpy.count, 2)
        compare(testObject3.status, ServiceLoader.Null)
        compare(statusChangedSpy.count, 5)
    }

    function test_service_loading_creation() {

        //
        // Test multiple creation
        //
        targetObject = testObject;
        testObject.interfaceName = testObject2.interfaceName = testObject3.interfaceName = ""
        resetSpies()
        testObject.interfaceName =  "com.nokia.qt.tests.serviceobject"
        testObject2.interfaceName = "com.nokia.qt.tests.serviceobject"
        testObject3.interfaceName = "com.nokia.qt.tests.serviceobject"
        serviceObjectChangedSpy.wait(1000)

        if (testObject.status == ServiceLoader.Error)
            dumpSpies()

        compare(testObject.status, ServiceLoader.Ready)

        var obj = testObject.serviceObject

        verify(obj != null, "Verify we have not received a null object")

        var o2 = testObject2.serviceObject

        verify(o2 != null, "Verify we have received a second valid object")
    }

    function test_service_loading_dblookup() {
        //
        // test simultaneous objects with failures
        //
        targetObject = testObject;
        testObject.interfaceName = testObject2.interfaceName = testObject3.interfaceName = ""
        resetSpies()
        testObject.interfaceName = "com.nokia.qt.tests.serviceobject"
        testObject2.interfaceName = "com.nokia.qt.tests.serviceobject2"
        testObject3.interfaceName = "com.nokia.qt.tests.serviceobject3"

        serviceObjectChangedSpy.wait(1000)

        compare(testObject.status, ServiceLoader.Ready)
        compare(testObject2.status, ServiceLoader.Error)
        compare(testObject3.status, ServiceLoader.Error)

        var obj = testObject.serviceObject

        verify(obj != null, "Verify we have not received a null object")

    }

    function test_service_z_errors_async() {
        targetObject = testObject;
        testObject.interfaceName = ""
        testObject.asynchronous = true
        resetSpies()
        testObject.interfaceName = "com.nokia.qt.tests.serviceobject"

        serviceObjectChangedSpy.wait(1000)
        resetSpies()

        var obj = testObject.serviceObject

        verify(obj != null, "Can get a non-null pointer to the service")

        obj.slotWithoutArg()

        obj.closeClientSockets()

        statusChangedSpy.wait(1000)
        verify(obj.status, ServiceLoader.Error)

        // try using the use the object now
        try {
            obj.slotWithoutArg()
        }
        catch(err) {
            verify(err.message.search("has no method"), "should fail properly");
        }
    }

    Item {
        id: dataTester
        signal done

        property bool enabled
        property int loops: 10

        onEnabledChanged: if (enabled == true) { sfwObj.slotWithArg(1) }

        Connections {
            target: sfwObj == null ? null : sfwObj
            ignoreUnknownSignals: true
            onStateChanged: {
                if (dataTester.loops-- > 0) {
                    sfwObj.slotWithArg(1)
                } else {
                    dataTester.enabled = false
                    dataTester.done()
                }
            }
        }
    }

    SignalSpy {id: dataTestSpy; target: dataTester; signalName: "done" }

    function test_service_mover() {
        testObject.interfaceName = ""
        resetSpies()
        testObject.interfaceName = "com.nokia.qt.tests.serviceobject"

        serviceObjectChangedSpy.wait(1000)

        sfwObj = testObject.serviceObject

        verify(sfwObj != null, "Can't use a null object for testing")

        dataTester.loops = 100
        dataTester.enabled = true

        dataTestSpy.wait(1000)
    }

    Item {
        id: dataTesterSignals
        signal done

        property bool enabled
        property int loops: 10

        onEnabledChanged: if (enabled == true) { sfwObj.slotWithArg(loops) }

        Connections {
            target: ((sfwObj == null) && dataTesterSignals.enabled) ? empty : sfwObj
            ignoreUnknownSignals: true
            onStateChanged: {
                if (dataTester.loops-- <= 0) {
                    dataTesterSignals.enabled = false
                    dataTesterSignals.done()
                }
            }
        }
    }

    SignalSpy {id: dataTestSignalsSpy; target: dataTesterSignals; signalName: "done" }

    function test_service_mover_signals() {
        testObject.interfaceName = ""
        resetSpies()
        testObject.interfaceName = "com.nokia.qt.tests.serviceobject"

        serviceObjectChangedSpy.wait(1000)

        sfwObj = testObject.serviceObject

        verify(sfwObj != null, "Can't use a null object for testing")

        dataTesterSignals.loops = 10
        dataTesterSignals.enabled = true

        dataTestSignalsSpy.wait(1000)
    }


    function test_service_z_errors_sync() {
        targetObject = testObject
        testObject.interfaceName = ""
        testObject.asynchronous = false
        resetSpies()
        testObject.interfaceName = "com.nokia.qt.tests.serviceobject"

        statusChangedSpy.wait(1000)

        var obj = testObject.serviceObject

        console.log("5" + obj + " " + testObject.errorString() + " : " + testObject.status)
        verify(obj != null, "Can not get a valid pointer to the service")

        obj.slotWithoutArg()

        var b = obj.closeClientSocketsBlocking()

        statusChangedSpy.wait(1000)

        // try using the use the object now
        try {
            console.log("9")
            obj.slotWithoutArg()
            console.log("10")
        }
        catch(err) {
            verify(err.message.search("has no method"), "should fail properly");
        }
        console.log("11")

        resetSpies()
        testObject.interfaceName = "com.nokia.qt.tests.serviceobject"
        console.log("12")
        serviceObjectChangedSpy.wait(1000)
        console.log("13")

        // old object should be destroyed, try the new one
        var b = testObject.serviceObject
        console.log("14")

        verify(b != null, "Must get a valid object back")
        verify(b.slotWithOk() == true, "")
        console.log("15")

    }

    function test_service_list_A_empty() {
        compare(serviceListInterfaceSpy.count, 0)
        compare(serviceListMajorVersionSpy.count, 0)
        compare(serviceListMinorVerisonSpy.count, 0)
        compare(serviceListMonitoeRegSpy.count, 0)
        compare(serviceListNameSpy.count, 0)

        verify(testList.serviceDescriptions.length > 0, "Make sure our service is listed")
    }

    function test_service_list_basics() {

        resetListSpies()

        testList.interfaceName = "com.nokia.qt.tests.serviceobject"

        serviceListServicesSpy.wait(1000)

        console.log("Test list2 is a: " + testList.serviceDescriptions.length + " " + serviceListServicesSpy.count);

        compare(testList.serviceDescriptions.length, 1, "Should only select our service")
        compare(serviceListServiceSpy.count, 1, "Only 1 signal")
        compare(serviceListNameSpy.count, 0, "No signal")

        testList.interfaceName = "com.nokia.qt.tests.invalid.name.that.should.never.be.registered"

        serviceListServicesSpy.wait(1000)
        compare(testList.serviceDescriptions.length, 0, "Should select no serviceDescriptions")

        resetListSpies()
        testList.interfaceName = "com.nokia.qt.tests.serviceobject"
        serviceListServicesSpy.wait(1000)
        resetListSpies()

        testList.majorVersion = 2
        compare(testList.majorVersion, 2, "Major version should change")
        compare(serviceListMajorVersionSpy.count, 1, "Expect 1 signal on change")
        serviceListServicesSpy.wait(1000)

        resetListSpies()
        testList.majorVersion = 0
        testList.minorVersion = 0
        testList.interfaceName = "com.nokia.qt.tests.serviceobject"
        serviceListServicesSpy.wait(1000)
        compare(testList.serviceDescriptions.length, 1, "Should only select our service")
        resetListSpies()

        testList.majorVersion = 1
        testList.minorVersion = 1
        compare(testList.minorVersion, 1, "Minor versino should change")
        compare(serviceListMinorVerisonSpy.count, 1, "Expect 1 signal on change")
        console.log("length is " + testList.serviceDescriptions.length)
        serviceListServicesSpy.wait(1000)

    }

    function dumpSpies() {

        console.log("********")
        console.log(serviceObjectChangedSpy)
        console.log("validChangedSpy.count " + validChangedSpy.count)
        console.log("serviceObjectChanged.count " + serviceObjectChangedSpy.count)
        console.log("interfacenamedChanged.count " + interfaceNameChangedSpy.count)
        console.log("serviceNameChanged.count " + serviceNameChangedSpy.count)
        console.log("majorVersionChanged.count " + majorVersionChangedSpy.count)
        console.log("minorVersionChange.count " + minorVersionChangedSpy.count)
        console.log("errorChanged.count " + errorChangedSpy.count)
        console.log("lastError " + testObject.error)
        console.log("***********")

    }
}
