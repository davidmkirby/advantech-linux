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
*    SynchronousOneWaveformAO.py
*
* Example Category:
*    AO
*
* Description:
*    This example demonstrates how to use Synchronous One Waveform AO voltage function.
*
* Instructions for Running:
*    1  Set the 'deviceDescription' for opening the device.
*    2  Set the 'profilePath' to save the profile path of being initialized device.
*    3  Set the 'channelStart' as the first channel for analog data Output.
*    4  Set the 'channelCount' to decide how many sequential channels to output analog data.
*
* I/O Connections Overview:
*    Please refer to your hardware reference manual.
*
******************************************************************************/
"""
import math

from Automation.BDaq import *
from Automation.BDaq.Utils import CreateArray
from Automation.BDaq.BufferedAoCtrl import BufferedAoCtrl
from Automation.BDaq.BDaqApi import AdxGetValueRangeInformation, BioFailed, AdxEnumToString

ONE_WAVE_POINT_COUNT = 2048

samples = ONE_WAVE_POINT_COUNT

deviceDescription = "DemoDevice,BID#0"
profilePath = u"../../profile/DemoDevice.xml"
startChannel = 0
channelCount = 2

class WaveStyle(object):
    Sine = 0
    Sawtooth = 1
    Square = 2

def GenerateWaveform(bfdAoCtrlObj, channelStart, channelCount, samplesCount, waveStyle):
    ret = ErrorCode.Success
    waveBuffer = [0.0] * samplesCount

    mathIntervalRanges = CreateArray(MathInterval, 64)   # ranges是64个MathInterval成员的数组
    channelCountMax = bfdAoCtrlObj.features.channelCountMax

    for i in range(channelCountMax):
        aoChannel = bfdAoCtrlObj.channels[i]
        valRange = aoChannel.valueRange

        if ValueRange.V_ExternalRefBipolar == valRange or valRange == ValueRange.V_ExternalRefUnipolar:
            if bfdAoCtrlObj.features.externalRefAntiPolar:
                if valRange == ValueRange.V_ExternalRefBipolar:
                    referenceValue = aoChannel.extRefBipolar
                    if referenceValue >= 0:
                        mathIntervalRanges[i].Max = referenceValue
                        mathIntervalRanges[i].Min = 0 - referenceValue
                    else:
                        mathIntervalRanges[i].Max = 0 - referenceValue
                        mathIntervalRanges[i].Min = referenceValue
                else:
                    referenceValue = aoChannel.extRefUnipolar
                    if referenceValue >= 0:
                        mathIntervalRanges[i].Max = 0
                        mathIntervalRanges[i].Min = 0 - referenceValue
                    else:
                        mathIntervalRanges[i].Max = 0 - referenceValue
                        mathIntervalRanges[i].Min = 0
            else:
                if valRange == ValueRange.V_ExternalRefBipolar:
                    referenceValue = aoChannel.extRefBipolar
                    if referenceValue >= 0:
                        mathIntervalRanges[i].Max = referenceValue
                        mathIntervalRanges[i].Min = 0 - referenceValue
                    else:
                        mathIntervalRanges[i].Max = 0 - referenceValue
                        mathIntervalRanges[i].Min = referenceValue
                else:
                    referenceValue = aoChannel.extRefUnipolar
                    if referenceValue >= 0:
                        mathIntervalRanges[i].Max = referenceValue
                        mathIntervalRanges[i].Min = 0
                    else:
                        mathIntervalRanges[i].Max = 0
                        mathIntervalRanges[i].Min = referenceValue
        else:
            ret = AdxGetValueRangeInformation(valRange, 0, None, mathIntervalRanges[i], None)
            if BioFailed(ret):
                return ret, None

    # generate waveform data and put them into the buffer which the parameter 'waveBuffer' give in,
    # the Amplitude these waveform

    oneWaveSamplesCount = samplesCount // channelCount
    waveBufferIndex = 0

    for i in range(oneWaveSamplesCount):
        for j in range(channelStart, channelStart + channelCount):
            channel = j % channelCountMax
            amplitude = (mathIntervalRanges[channel].Max - mathIntervalRanges[channel].Min) / 2
            offset = (mathIntervalRanges[channel].Max + mathIntervalRanges[channel].Min) / 2

            if waveStyle == WaveStyle.Sine:
                waveBuffer[waveBufferIndex] = amplitude * (math.sin((i * 2.0 * 3.14159) / oneWaveSamplesCount)) + offset
                waveBufferIndex += 1
            elif waveStyle == WaveStyle.Sawtooth:
                if (i >= 0) and (i < (oneWaveSamplesCount / 4.0)):
                    waveBuffer[waveBufferIndex] = amplitude * (math.sin((i * 2.0 * 3.14159) / oneWaveSamplesCount)) + offset
                    waveBufferIndex += 1
                else:
                    if (i >= (oneWaveSamplesCount / 4.0)) and (i < 3 * (oneWaveSamplesCount / 4.0)):
                        waveBuffer[waveBufferIndex] = amplitude * ((2.0 * (oneWaveSamplesCount / 4.0) - i) / (oneWaveSamplesCount / 4.0)) + offset
                        waveBufferIndex += 1
                    else:
                        waveBuffer[waveBufferIndex] = amplitude * ((i - oneWaveSamplesCount) / (oneWaveSamplesCount / 4.0)) + offset
                        waveBufferIndex += 1
            elif waveStyle == WaveStyle.Square:
                if (i >= 0) and (i < (oneWaveSamplesCount / 2)):
                    waveBuffer[waveBufferIndex] = amplitude * 1 + offset
                    waveBufferIndex += 1
                else:
                    waveBuffer[waveBufferIndex] = amplitude * (-1) + offset
                    waveBufferIndex += 1
            else:
                print("invalid wave style, generate waveform error!")
                ret = ErrorCode.ErrorUndefined
    return ret, waveBuffer

def AdvBufferedAO():
    ret = ErrorCode.Success

    # Step 1: Create a BfdAoCtrl for buffered AO function
    # Select a device by device number or device description and specify the access mode.
    # In the example we use ModeWrite mode so that we can fully control the device,
    # including configuring, sampling, etc.
    bfdAoCtrlObj = BufferedAoCtrl(deviceDescription)

    for loop in range(1):
        bfdAoCtrlObj.loadProfile = profilePath   # Loads a profile to initialize the device

        # Step 2: Set necessary parameters
        # get scan channel instance and set the start channel number and scan channel count
        bfdAoCtrlObj.scanChannel.channelStart = startChannel
        bfdAoCtrlObj.scanChannel.channelCount = channelCount

        # set samples number
        bfdAoCtrlObj.scanChannel.samples = samples

        # Step 3: prepare the buffered AO
        ret = bfdAoCtrlObj.prepare()
        if BioFailed(ret):
            break

        # Generate waveform data
        ret, waveformBuf = GenerateWaveform(bfdAoCtrlObj, startChannel, channelCount,  channelCount * ONE_WAVE_POINT_COUNT, WaveStyle.Sine)
        if BioFailed(ret):
            break

        ret = bfdAoCtrlObj.setDataF64(channelCount * ONE_WAVE_POINT_COUNT, waveformBuf)
        if BioFailed(ret):
            break

        # Step 4: Start a Synchronous One Waveform AO, 'Synchronous' indicates using synchronous mode,
        # which means the method will not return until the operation is completed.
        print("Synchronous finite acquisition is in progress.")
        print("Please wait, until acquisition complete.")
        ret = bfdAoCtrlObj.runOnce()
        if BioFailed(ret):
            break
        print("Buffered AO is over!")

    bfdAoCtrlObj.dispose()

    if BioFailed(ret):
        enumStr = AdxEnumToString("ErrorCode", ret.value, 256)
        print("Some error occurred. And the last error code is %#x. [%s]" % (ret.value, enumStr))

    return 0


if __name__ == '__main__':
    AdvBufferedAO()
