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
$Log:  $
--------------------------------------------------------------------------------
$NoKeywords:  $
*/
/******************************************************************************
*
* Windows Example:
*     PollingOneBufferedAI_TDtp.py
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to use Polling One Buffered AI with Trigger Delay
*    to Stop function.
* Instructions for Running:
*    1. Set the 'deviceDescription' which can get from system device manager for opening the device.
*    2. Set the 'profilePath' to save the profile path of being initialized device.
*    3. Set the 'startChannel' as the first channel for scan analog samples
*    4. Set the 'channelCount' to decide how many sequential channels to scan analog samples.
*    5. Set the 'sectionLength' as the length of data section for Buffered AI.
*    6. Set the 'sectionCount' as the count of data section for Buffered AI.
*
* I/O Connections Overview:
*    Please refer to your hardware reference manual.
*
******************************************************************************/
"""
from Automation.BDaq import *
from Automation.BDaq.WaveformAiCtrl import WaveformAiCtrl
from Automation.BDaq.BDaqApi import AdxEnumToString, BioFailed

# Configure the following parameters before running the demo
deviceDescription = "DemoDevice,BID#0"
profilePath = u"../../profile/DemoDevice.xml"
startChannel = 0
channelCount = 2
sectionLength = 1024
sectionCount = 1

# user buffer size should be equal or greater than raw data buffer length, because data ready count
# is equal or more than smallest section of raw data buffer and up to raw data buffer length.
# users can set 'USER_BUFFER_SIZE' according to demand.
USER_BUFFER_SIZE = channelCount * sectionLength * sectionCount

# Set trigger parameters
triggerAction = TriggerAction.DelayToStop
triggerEdge = ActiveSignal.RisingEdge
triggerDelayCount = 1000
triggerLevel = 2.0

# Set trigger1 parameters
trigger1Action = TriggerAction.DelayToStop
trigger1Edge = ActiveSignal.RisingEdge
trigger1DelayCount = 1000
trigger1Level = 2.0

# set which trigger be used for this demo, trigger0(0) or trigger1(1)
triggerUsed = 0

def AdvPollingOneBufferedAI_TDtp():
    ret = ErrorCode.Success

    # Step 1: Create a 'WaveformAiCtrl' for buffered AI function
    # Select a device by device number pr device description and specify the access mode.
    # In this example we use ModeWrite mode so that we can use fully control the device,
    # including configuring, sampling, etc
    wfAiCtrl = WaveformAiCtrl(deviceDescription)
    for _ in range(1):
        wfAiCtrl.loadProfile = profilePath   # Loads a profile to initialize the device

        # Step 2: Set necessary parameters
        # get the Conversion instance and set the start channel and scan channel number
        wfAiCtrl.conversion.channelStart = startChannel
        wfAiCtrl.conversion.channelCount = channelCount

        # Set record count and section length
        # This sectionCount is nonzero value, which means 'One Buffered' Mode
        wfAiCtrl.record.sectionCount = sectionCount
        wfAiCtrl.record.sectionLength = sectionLength

        # Step 3: Trigger parameters setting
        trgCount = wfAiCtrl.features.triggerCount
        # for trigger0
        if triggerUsed == 0:
            if trgCount:
                #################################################################
                # The different kinds of devices have different trigger source. The details see manual.
                # In this example, we use the DemoDevice and set 'AI channel 0' as the default trigger source
                wfAiCtrl.trigger[0].source = wfAiCtrl.features.getTriggerSources(0)[1] # To DemoDevice, the 1 means 'AI channel 0'

                wfAiCtrl.trigger[0].action = triggerAction
                wfAiCtrl.trigger[0].delayCount = triggerDelayCount
                wfAiCtrl.trigger[0].edge = triggerEdge
                wfAiCtrl.trigger[0].level = triggerLevel
            else:
                print("The device can not supported trigger function!")
                break
        elif triggerUsed == 1:
            if trgCount > 1:
                wfAiCtrl.trigger[1].source = wfAiCtrl.features.getTriggerSources(1)[1]

                wfAiCtrl.trigger[1].action = trigger1Action
                wfAiCtrl.trigger[1].delayCount = trigger1DelayCount
                wfAiCtrl.trigger[1].edge = trigger1Edge
                wfAiCtrl.trigger[1].level = trigger1Level
            else:
                print("the trigger1 can not supported by the device!")
                break

        # Step 4: The operation has been started
        print("Polling finite acquisition is in progress!")
        ret = wfAiCtrl.prepare()
        if BioFailed(ret):
            break

        ret = wfAiCtrl.start()
        if BioFailed(ret):
            break

        # Step 5: GetDate with Polling Style
        result = wfAiCtrl.getDataF64(USER_BUFFER_SIZE, -1)  # The timeout value is -1, meaning infinite waiting
        ret, returnedCount, data, = result[0], result[1], result[2]
        if BioFailed(ret):
            break

        if ret == ErrorCode.Success or ret == ErrorCode.WarningFuncStopped:
            print("The first sample each channel are:")
            for i in range(channelCount):
                print("channel %d: %s" % (i + startChannel, data[i]))

        delayCount = wfAiCtrl.trigger[triggerUsed].delayCount
        triggerPointIndex = returnedCount // channelCount - delayCount
        print("trigger point each channel: %d" % triggerPointIndex)
        print("Acquisition has completed!")

        # Step 6: stop the operation if it is running
        ret = wfAiCtrl.stop()

    # Step 7: close device, release any allocated resource before quit
    wfAiCtrl.dispose()

    # If something wrong in this execution, print the error code on screen for tracking
    if BioFailed(ret):
        enumStr = AdxEnumToString("ErrorCode", ret.value, 256)
        print("Some error occurred. And the last error code is %#x. [%s]" % (ret.value, enumStr))
    return 0


if __name__ == '__main__':
    AdvPollingOneBufferedAI_TDtp()
