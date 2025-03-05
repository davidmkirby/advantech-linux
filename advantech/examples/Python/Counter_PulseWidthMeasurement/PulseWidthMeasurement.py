#!/usr/bin/python
# -*- coding:utf-8 -*-


"""
/*******************************************************************************
Copyright (c) 1983-2021 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED.
================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log: $
--------------------------------------------------------------------------------
$NoKeywords:  $
*/
/******************************************************************************
*
* Windows Example:
*     PulseWidthMeasurement.py
*
* Example Category:
*    Counter
*
* Description:
*    This example demonstrates how to use Pulse Width Measurement function.
*
* Instructions for Running:
*    1  Set the 'deviceDescription' for opening the device.
*    2  Set the 'profilePath' to save the profile path of being initialized device.
*    3  Set the 'channelStart' as the start channel of the counter to operate
*    4  Set the 'channelCount' as the channel count of the counter to operate.
*
* I/O Connections Overview:
*    Please refer to your hardware reference manual.
*
******************************************************************************/
"""
import time, sys
sys.path.append('..')
from CommonUtils import kbhit

from Automation.BDaq import *
from Automation.BDaq.PwMeterCtrl import PwMeterCtrl
from Automation.BDaq.BDaqApi import BioFailed, AdxEnumToString

deviceDescription = "DemoDevice,BID#0"
profilePath = u"../../profile/DemoDevice.xml"
startChannel = 0
channelCount = 1

def AdvPwMeter():
    ret = ErrorCode.Success

    # Step 1: Create a 'PwMeterCtrl' for PulseWidthMeasurement function
    # Select a device by device number or device description and specify the access mode.
    # In this example we use ModeWrite mode so that we can fully control the device,
    # including configuring, sampling, etc.
    pwMeterCtrl = PwMeterCtrl(deviceDescription)
    for _ in range(1):
        pwMeterCtrl.loadProfile = profilePath

        # Step 2: Set necessary parameters
        # set the channel start number and channel count for PulseWidthMeasurement
        pwMeterCtrl.channelStart = startChannel
        pwMeterCtrl.channelCount = channelCount

        # Step 3: Start PulseWidthMeasurement
        pwMeterCtrl.enabled = True

        # Step 4: Get Pulse Width value
        print("Pulse Width Measurement is in progress... any key to quit !")
        while not kbhit():
            time.sleep(1)
            ret, pwValueList = pwMeterCtrl.read()
            if BioFailed(ret):
                break
            for i, pwValue in enumerate(pwValueList):
                print("Channel %u Current Pulse Width:HiPeriod=%f s  LoPeriod=%f s" % (startChannel + i, pwValue.HiPeriod, pwValue.LoPeriod))

        # Step 5: Stop PulseWidthMeasurement
        pwMeterCtrl.enabled = False

    # Step 6: Close device and release any allocated resource
    pwMeterCtrl.dispose()

    # If something wrong in this execution, print the error code on screen for tracking.
    if BioFailed(ret):
        enumStr = AdxEnumToString("ErrorCode", ret.value, 256)
        print("Some error occurred. And the last error code is 0x%X. [%s]" % (ret.value, enumStr))
    return 0


if __name__ == '__main__':
    AdvPwMeter()
