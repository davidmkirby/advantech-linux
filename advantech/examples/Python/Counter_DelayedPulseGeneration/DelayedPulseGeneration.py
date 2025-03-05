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
*    DelayedPulseGeneration.py
*
* Example Category:
*    Counter
*
* Description:
*    This example demonstrates how to use Delayed Pulse Generation function.
*
* Instructions for Running:
*    1  Set the 'deviceDescription' for opening the device.
*    2  Set the 'profilePath' to save the profile path of being initialized device.
*    3  Set the 'channelStart' as the start channel of the counter to operate
*    4  Set the 'channelCount' as the channel count of the counter to operate.
*    5  set the 'delayCount' to decide delay time for selected channel.
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
from Automation.BDaq.OneShotCtrl import OneShotCtrl

deviceDescription = "DemoDevice,BID#0"
profilePath = u"../../profile/DemoDevice.xml"
startChannel = 0
channelCount = 1
delayCount = 50

def AdvOneShot():
    # Step 1: Create a 'OneShotCtrl' for Delayed Pulse Generation function
    # Select a device by device number or device description and specify the access mode.
    # In this example we use ModeWrite mode so that we can fully control the device,
    # including configuring, sampling, etc.
    oneShotCtrl = OneShotCtrl(deviceDescription)

    oneShotCtrl.loadProfile = profilePath

    # Step 2: Set necessary parameters
    #  # get channel max number for one shot
    channelCountMax = oneShotCtrl.features.channelCountMax

    # set start channel number and channel count for one shot
    oneShotCtrl.channelStart = startChannel
    oneShotCtrl.channelCount = channelCount

    # set all channels
    for i in range(startChannel, startChannel + channelCount):
        oneShotCtrl.channels[i % channelCountMax].delayCount = delayCount

    # Step 3: Start DelayedPulseGeneration
    print("Delayed Pulse Generation is in progress..\nGive a low level signal to Gate pin and Test the pulse signal on the Out pin!")
    print("Any key to quit !")
    oneShotCtrl.enabled = True

    while not kbhit():
        time.sleep(1)

    # Step 4: Stop DelayedPulseGeneration
    oneShotCtrl.enabled = False

    # Step 5: Close device and release any allocated resource
    oneShotCtrl.dispose()

    return 0


if __name__ == "__main__":
    AdvOneShot()
