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
* Windows  Example:
*    PulseOutputwithTimerInterrupt.py
*
* Example Category:
*    Counter
*
* Description:
*    This example demonstrates how to use Pulse Output with Timer Interrupt function.
*
* Instructions for Running:
*    1  Set the 'deviceDescription' for opening the device.
*    2  Set the 'profilePath' to save the profile path of being initialized device.
*    3  Set the 'channelStart' as the start channel of the counter to operate
*    4  Set the 'channelCount' as the channel count of the counter to operate.
*    5  set the 'frequency' to decide the frequency of pulse for selected channel.
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
from Automation.BDaq.TimerPulseCtrl import TimerPulseCtrl

deviceDescription = "DemoDevice,BID#0"
profilePath = u"../../profile/DemoDevice.xml"
startChannel = 0
channelCount = 1
frequency = 10.0

def AdvTimerPulse():
    # Step 1: Create a 'TimerPulseCtrl' for Pulse Output with Timer Interrupt function
    # Select a device by device number or device description and specify the access mode.
    # In this example we use ModeWrite mode so that we can fully control the device,
    # including configuring, sampling, etc.
    timerPulseCtrl = TimerPulseCtrl(deviceDescription)

    timerPulseCtrl.loadProfile = profilePath

    # Step 2: Set necessary parameters
    #  # get channel max number for timer pulse
    channelCountMax = timerPulseCtrl.features.channelCountMax

    # set start channel number and channel count for timer pulse
    timerPulseCtrl.channelStart = startChannel
    timerPulseCtrl.channelCount = channelCount

    # set all channels
    for i in range(startChannel, startChannel + channelCount):
        timerPulseCtrl.channels[i % channelCountMax].frequency = frequency

    # Step 3: Start PulseOutputWithTimerInterrupt
    print("PulseOutputWithTimerInterrupt is in progress..\nTest signal to the Out pin!")
    print("Any key to quit !")
    timerPulseCtrl.enabled = True

    while not kbhit():
        time.sleep(1)

    # Step 4: Stop PulseOutputWithTimerInterrup
    timerPulseCtrl.enabled = False

    # Step 5: Close device and release any allocated resource
    timerPulseCtrl.dispose()

    return 0


if __name__ == "__main__":
    AdvTimerPulse()
