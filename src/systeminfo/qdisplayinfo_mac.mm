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

#include "qdisplayinfo_mac_p.h"
#include <QtCore/private/qcore_mac_p.h>

#include <QtCore/QDir>
#include <QtCore/QSettings>

#include <IOKit/graphics/IOGraphicsLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>

#include <CoreServices/CoreServices.h>
#include <QDebug>

CGDirectDisplayID getCGId(int screen)
{
    CGDirectDisplayID displayId[16];
    CGDisplayCount count;
    CGDisplayErr error = CGGetOnlineDisplayList(16,displayId, & count);
    if (error == kCGErrorSuccess) {
        return displayId[screen];
    }
    return CGMainDisplayID();
}

static int GetIntFromDictionaryForKey(CFDictionaryRef desc, CFStringRef key)
{
    CFNumberRef value;
    int resultNumber = 0;
    if ((value = (const __CFNumber*)CFDictionaryGetValue(desc,key)) == NULL
            || CFGetTypeID(value) != CFNumberGetTypeID())
        return 0;
    CFNumberGetValue(value, kCFNumberIntType, &resultNumber);
    return resultNumber;
}

CGDisplayErr GetDisplayDPI(CFDictionaryRef displayModeDict,CGDirectDisplayID displayID,
    double *horizontalDPI, double *verticalDPI)
{
    CGDisplayErr displayError = kCGErrorFailure;
    io_connect_t ioPort;
    CFDictionaryRef displayDict;

    ioPort = CGDisplayIOServicePort(displayID);
    if (ioPort != MACH_PORT_NULL) {
        displayDict = IOCreateDisplayInfoDictionary(ioPort, 0);
        if (displayDict != NULL) {
            const double mmPerInch = 25.4;
            double horizontalSizeInInches = (double)GetIntFromDictionaryForKey(displayDict, CFSTR(kDisplayHorizontalImageSize)) / mmPerInch;
            double verticalSizeInInches = (double)GetIntFromDictionaryForKey(displayDict, CFSTR(kDisplayVerticalImageSize)) / mmPerInch;

            CFRelease(displayDict);

            *horizontalDPI = (double)GetIntFromDictionaryForKey(displayModeDict, kCGDisplayWidth) / horizontalSizeInInches;
            *verticalDPI = (double)GetIntFromDictionaryForKey(displayModeDict, kCGDisplayHeight) / verticalSizeInInches;
            displayError = CGDisplayNoErr;
        }
    }
    return displayError;
}

QT_BEGIN_NAMESPACE

QDisplayInfoPrivate::QDisplayInfoPrivate(QDisplayInfo *parent)
    : q_ptr(parent)
{
    qDebug() << __FUNCTION__;

}

int QDisplayInfoPrivate::brightness(int screen)
{
    CGDisplayErr dErr;
    io_service_t service;
    CFStringRef key = CFSTR(kIODisplayBrightnessKey);

    float brightness = 0.0;
    int displayBrightness = -1;
    service = CGDisplayIOServicePort(getCGId(screen));
    dErr = IODisplayGetFloatParameter(service, kNilOptions, key, &brightness);
    displayBrightness = (int)(brightness * 100);
    return displayBrightness;
}

int QDisplayInfoPrivate::contrast(int screen)
{
    Q_UNUSED(screen);
    QString accessplist = QDir::homePath() + "/Library/Preferences/com.apple.universalaccess.plist";
    QSettings accessSettings(accessplist, QSettings::NativeFormat);
    accessSettings.value("contrast").toFloat();
    return accessSettings.value("contrast").toFloat();
}

QDisplayInfo::BacklightState QDisplayInfoPrivate::backlightState(int screen)
{
    int bright = brightness(screen);
    if (bright == 0) {
        return QDisplayInfo::BacklightOff;
    } else if (bright > 1 && bright < 99) {
        return QDisplayInfo::BacklightOff;
       } else {
        return QDisplayInfo::BacklightOn;
    }
    return QDisplayInfo::BacklightUnknown;
}

int QDisplayInfoPrivate::colorDepth(int screen) const
{
    qDebug() << __FUNCTION__;
    long bitsPerPixel = 0;
#ifndef MAC_SDK_10_5
    CGDisplayModeRef mode = CGDisplayCopyDisplayMode(getCGId(screen));
    bitsPerPixel = QCFString::toQString(CGDisplayModeCopyPixelEncoding(mode)).toLong();
#else
    bitsPerPixel = CGDisplayBitsPerPixel(getCGId(screen));
#endif
    return (int)bitsPerPixel;
}

int QDisplayInfoPrivate::dpiX(int screen) const
{
    int dpi=0;
    if (screen < 16 && screen > -1) {
        double horizontalDPI, verticalDPI;

        // TODO CGDisplayCopyDisplayMode depreciated

        CGDisplayErr displayError = GetDisplayDPI(CGDisplayCurrentMode(kCGDirectMainDisplay), kCGDirectMainDisplay, &horizontalDPI, &verticalDPI);
        if (displayError == CGDisplayNoErr) {
            dpi = horizontalDPI;
        }
    }
    return dpi;
}

int QDisplayInfoPrivate::dpiY(int screen) const
{
    int dpi=0;
    if (screen < 16 && screen > -1) {
        double horizontalDPI, verticalDPI;

        CGDisplayErr displayError = GetDisplayDPI(CGDisplayCurrentMode(kCGDirectMainDisplay),  kCGDirectMainDisplay, &horizontalDPI, &verticalDPI);
        if (displayError == CGDisplayNoErr) {
            dpi = verticalDPI;
        }
    }
    return dpi;

}

int QDisplayInfoPrivate::physicalHeight(int screen) const
{
    int height=0;
    if (screen < 16 && screen > -1) {
        CGSize size = CGDisplayScreenSize(getCGId(screen));
        height = size.height;
    }
    return height;

}

int QDisplayInfoPrivate::physicalWidth(int screen) const
{
    int width=0;
    if (screen < 16 && screen > -1) {
        CGSize size = CGDisplayScreenSize(getCGId(screen));
        width = size.width;
    }
    return width;

}


QT_END_NAMESPACE

