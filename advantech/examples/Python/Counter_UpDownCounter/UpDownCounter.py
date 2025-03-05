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
*     UpDownCounter.py
*
* Example Category:
*    Counter
*
* Description:
*    This example demonstrates how to use UpDown Counter function.
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

from Automation.BDaq import ErrorCode, DeviceInformation, AccessMode
from Automation.BDaq import CountingType
from Automation.BDaq.UdCounterCtrl import UdCounterCtrl
from Automation.BDaq.BDaqApi import AdxEnumToString, BioFailed

deviceDescription = "DemoDevice,BID#0"
profilePath = u"../../profile/DemoDevice.xml"
channelStart = 0
channelCount = 1

def AdvUdCounter():
    # Step 1: Create a 'UdCounterCtrl' for UpDown Counter function
    # Select a device by device number or device description and specify the access mode.
    # In this example we use ModeWrite mode so that we can fully control the device,
    # including configuring, sampling, etc.
    udCounterCtrl = UdCounterCtrl(deviceDescription)

    udCounterCtrl.loadProfile = profilePath

    # Step 2: Set necessary parameters
    # set start channel number and channel count for UpDown Counter
    udCounterCtrl.channelStart = channelStart
    udCounterCtrl.channelCount = channelCount

    # Step 3: Set counting type for UpDown Counter
    for i in range(channelStart, channelStart + channelCount):
        udCounterCtrl.channels[i].countingType = CountingType.PulseDirection

    # Step 4: Start UpDown Counter
    udCounterCtrl.enabled = True

    # Step 5: Read counting value: connect the input signal to channels you selected to get UpDown counter value.
    print("UpDown counter is in progress.. connect the input signal")
    print("Any key will stop UpDown counter!")
    while not kbhit():
        time.sleep(1)
        ret, value = udCounterCtrl.read()
        if BioFailed(ret):
            break
        print(" channel %u Current UpDown count: %u" % (channelStart, value[0]))

    # Step 6: stop UpDown Counter
    udCounterCtrl.enabled = False

    # Step 7: Close device and release any allocated resource
    udCounterCtrl.dispose()

    # If something wrong in this execution, print the error code on screen for tracking
    if BioFailed(ret):
        enumStr = AdxEnumToString("ErrorCode", ret.value, 256)
        print("Some error occurred. And the last error code is %#x. [%s]" % (ret.value, enumStr))

    return 0


if __name__ == '__main__':
    AdvUdCounter()
