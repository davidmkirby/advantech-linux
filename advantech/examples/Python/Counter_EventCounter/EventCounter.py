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
*     EventCounter.py
*
* Example Category:
*    Counter
*
* Description:
*    This example demonstrates how to use Event Counter function.
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
from Automation.BDaq.EventCounterCtrl import EventCounterCtrl
from Automation.BDaq.BDaqApi import BioFailed, AdxEnumToString

deviceDescription = "DemoDevice,BID#0"
profilePath = u"../../profile/DemoDevice.xml"
startChannel = 0
channelCount = 1

def AdvEventCounter():
    ret = ErrorCode.Success

    # Step 1: Create a 'EventCounterCtrl' for event counter function
    # Select a device by device number or device description and specify the access mode.
    # In this example we use ModeWrite mode so that we can fully control the device,
    # including configuring, sampling, etc.
    eventCounterCtrl = EventCounterCtrl(deviceDescription)
    for _ in range(1):
        eventCounterCtrl.loadProfile = profilePath

        # Step 2: Set necessary parameters
        # set start channel number and channel count for event counter
        eventCounterCtrl.channelStart = startChannel
        eventCounterCtrl.channelCount = channelCount

        # Step 3: Start EventCounter
        eventCounterCtrl.enabled = True

        # Step 4: Read counting value: connect the input signal to channels you selected to get event counter value
        print("Event counter is in progress...")
        print("Connect the input signal to CNT%d_CLK pin if you choose external clock!" % startChannel)
        print("Any key will stop event counter!")
        while not kbhit():
            ret, data = eventCounterCtrl.read()
            if BioFailed(ret):
                break
            print("channel %u Currnt Event count: %u" % (startChannel, data[0]))
            time.sleep(1)

        # Step 5: stop EventCounter
        eventCounterCtrl.enabled = False

    # Step 6: Close device and release any allocated resource
    eventCounterCtrl.dispose()

    # If something wrong in this execution, print the error code on screen for tracking
    if BioFailed(ret):
        enumStr = AdxEnumToString("ErrorCode", ret.value, 256)
        print("Some error occurred, And the last error code is %#x, [%s]" % (ret.value, enumStr))
    return 0


if __name__ == '__main__':
    AdvEventCounter()
