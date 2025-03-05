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
*    PWMOutput.py
*
* Example Category:
*    Counter
*
* Description:
*    This example demonstrates how to use PWM Output function.
*
* Instructions for Running:
*    1  Set the 'deviceDescription' for opening the device.
*    2  Set the 'profilePath' to save the profile path of being initialized device.
*    3  Set the 'channelStart' as the start channel of the counter to operate
*    4  Set the 'channelCount' as the channel count of the counter to operate.
*    5  set the 'pulseWidth' to decide the period of pulse for selected channel.
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
from Automation.BDaq.PwModulatorCtrl import PwModulatorCtrl

deviceDescription = "DemoDevice,BID#0"
profilePath = u"../../profile/DemoDevice.xml"
channelStart = 0
channelCount = 1

pulseWidth = PulseWidth(0.07, 0.03)

def AdvPwModulator():
    # Step 1: Create a 'PmModulatorCtrl' for PWMOutput function
    # Select a device by device number or device description and specify the access mode.
    # In this example we use ModeWrite mode so that we can fully control the device,
    # including configuring, sampling, etc.
    pmModulatorCtrl = PwModulatorCtrl(deviceDescription)

    pmModulatorCtrl.loadProfile = profilePath   # Load a profile to initialize the device

    # Step 2: Set necessary parameters
    # get channel max num for PWMOutput
    channelCountMax = pmModulatorCtrl.features.channelCountMax

    # set start channel num and channel count for PWMOutput
    pmModulatorCtrl.channelStart = channelStart
    pmModulatorCtrl.channelCount = channelCount

    # set pulseWidth value
    for i in range(channelStart, channelStart + channelCount):
        pmModulatorCtrl.channels[i % channelCountMax].pulseWidth = pulseWidth

    # Step 4: start PWMOutput
    print("PWMOutput is in progress.. test signal to the Out pin !")
    print("Any key to quit !")
    pmModulatorCtrl.enabled = True

    while not kbhit():
        time.sleep(1)

    # Step 5: Stop PWMOutput
    pmModulatorCtrl.enabled = False
    
    # Step 6: Close device and release any allocated resource.
    pmModulatorCtrl.dispose()

    return 0


if __name__ == '__main__':
    AdvPwModulator()
