/*******************************************************************************
Copyright (c) 1983-2016 Advantech Co., Ltd.
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
*    SynchronousOneWaveformAO.c
*
* Example Category:
*    AO
*
* Description:
*    This example demonstrates how to use Synchronous One Waveform AO  voltage function.
*
* Instructions for Running:
*    1  Set the 'deviceDescription' for opening the device. 
*	  2  Set the 'profilePath' to save the profile path of being initialized device. 
*    3  Set the 'channelStart' as the first channel for analog data Output.
*    4  Set the 'channelCount' to decide how many sequential channels to output analog data.
*
* I/O Connections Overview:
*    Please refer to your hardware reference manual.
*
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../../../inc/bdaqctrl.h"
#include "../inc/compatibility.h"
//-----------------------------------------------------------------------------------
// Configure the following parameters before running the demo
//-----------------------------------------------------------------------------------
#define ONE_WAVE_POINT_COUNT 2048
int32 samples = ONE_WAVE_POINT_COUNT;

#define  deviceDescription  L"DemoDevice,BID#0"
const wchar_t* profilePath = L"../../profile/DemoDevice.xml";
int32 startChannel = 0;
int32 channelCount = 2;

typedef enum tagWaveStyle{Sine, Sawtooth, Square}WaveStyle;

//function GenerateWaveform: generate one waveform for each selected analog data output channel 
ErrorCode GenerateWaveform(BufferedAoCtrl *, int32, int32, double *, int32, WaveStyle);

void waitAnyKey()
{
   do {SLEEP(1);} while (!kbhit());
}

int main(int argc, char* argv[])
{
   ErrorCode ret = Success;
   ScanChannel * scanChannel = NULL;
   int32 waveBufferLen = 0;
   double * scaledWaveForm = NULL;
   wchar_t enumString[256];

   // Step 1: Create a BfdAoCtrl for buffered AO function.
   BufferedAoCtrl * bfdAoCtrl = BufferedAoCtrl_Create();
   do 
   {  // Step 2: Select a device by device number or device description and specify the access mode.
      // in this example we use ModeWrite mode so that we can fully control the device, including configuring, sampling, etc.
      DeviceInformation devInfo;
      devInfo.DeviceNumber = -1;
      devInfo.DeviceMode = ModeWrite;
      devInfo.ModuleIndex = 0;
      wcscpy(devInfo.Description, deviceDescription);
      ret = BufferedAoCtrl_setSelectedDevice(bfdAoCtrl, &devInfo);
      CHK_RESULT(ret);
      ret = BufferedAoCtrl_LoadProfile(bfdAoCtrl, profilePath);//Loads a profile to initialize the device.
      CHK_RESULT(ret);
      
      // Step 3: Set necessary parameters.
      scanChannel = BufferedAoCtrl_getScanChannel(bfdAoCtrl);
      ret = ScanChannel_setChannelStart(scanChannel, startChannel);
      CHK_RESULT(ret);
      ret = ScanChannel_setChannelCount(scanChannel, channelCount);
      CHK_RESULT(ret);
      ret = ScanChannel_setSamples(scanChannel, samples); 
      CHK_RESULT(ret);

      // Step 4: prepare the buffered AO. 
      ret = BufferedAoCtrl_Prepare(bfdAoCtrl);
      CHK_RESULT(ret);
      // Generate waveform data
      waveBufferLen = channelCount*ONE_WAVE_POINT_COUNT*sizeof(double);
      scaledWaveForm = (double *)malloc(waveBufferLen);
      if (NULL == scaledWaveForm)
      {
         printf( "Insufficient memory available\n" );
         break;
      }
      ret = GenerateWaveform(bfdAoCtrl, startChannel, channelCount, scaledWaveForm, channelCount*ONE_WAVE_POINT_COUNT, Sine);
      CHK_RESULT(ret);
      ret = BufferedAoCtrl_SetDataF64(bfdAoCtrl, channelCount*ONE_WAVE_POINT_COUNT, scaledWaveForm);
      free(scaledWaveForm);
      CHK_RESULT(ret);

      // Step 5: Start Synchronous One Waveform AO, 'Synchronous' indicates using synchronous mode,
      // which means the method will not return until the operation is completed.
      printf("Synchronous finite acquisition is in progress.\n");
      printf("Please wait, until acquisition complete.\n");

      ret = BufferedAoCtrl_RunOnce(bfdAoCtrl);
      CHK_RESULT(ret);
      printf("\nBuffered AO is over, any key to quit !\n");
   } while (FALSE);

   // Step 6: Close device, release any allocated resource.
   BufferedAoCtrl_Dispose(bfdAoCtrl);

   // If something wrong in this execution, print the error code on screen for tracking.
   if (BioFailed(ret))
   {
      AdxEnumToString(L"ErrorCode", (int32)ret, 256, enumString);
      printf("Some error occurred. And the last error code is 0x%X. [%ls]\n", ret, enumString);
   }
   waitAnyKey();
   return 0;
}

ErrorCode GenerateWaveform(BufferedAoCtrl * bfdAoCtrl, int32 channelStart, int32 channelCount, double * waveBuffer, int32 SamplesCount, WaveStyle style)
{
   ErrorCode ret                = Success;
   int32     channel            = 0;
   int32     channelCountMax    = 0;
   int32     oneWaveSamplesCount = SamplesCount / channelCount;
   int32     i                  = 0;

   MathInterval   ranges[64];
   ValueRange     valRange;
   AnalogChannel * aoChannel = NULL;

   channelCountMax = AoFeatures_getChannelCountMax(BufferedAoCtrl_getFeatures(bfdAoCtrl));
   for (i = 0; i < channelCountMax; i++)
   {
      aoChannel = (AnalogChannel *)Array_getItem(BufferedAoCtrl_getChannels(bfdAoCtrl), i);

      valRange  = AoChannel_getValueRange(aoChannel);
      if (V_ExternalRefBipolar == valRange || valRange == V_ExternalRefUnipolar)
      {
         if (AoFeatures_getExternalRefAntiPolar(BufferedAoCtrl_getFeatures(bfdAoCtrl)))
         {
            double referenceValue;
            if (valRange == V_ExternalRefBipolar)
            {
               referenceValue = AoChannel_getExtRefBipolar(aoChannel);
               if (referenceValue >= 0)
               {
                  ranges[i].Max = referenceValue;
                  ranges[i].Min = 0 - referenceValue;
               } 
               else
               {
                  ranges[i].Max = 0 - referenceValue;
                  ranges[i].Min = referenceValue;
               }
            } 
            else
            {
               referenceValue = AoChannel_getExtRefUnipolar(aoChannel);
               if (referenceValue > 0)
               {
                  ranges[i].Max = 0;
                  ranges[i].Min = 0 - referenceValue;
               } 
               else
               {
                  ranges[i].Max = 0 - referenceValue;
                  ranges[i].Min = 0;
               }
            }
         } 
         else
         {
            double referenceValue;
            if (valRange == V_ExternalRefBipolar)
            {
               referenceValue = AoChannel_getExtRefBipolar(aoChannel);
               if (referenceValue >= 0)
               {
                  ranges[i].Max = referenceValue;
                  ranges[i].Min = 0 - referenceValue;
               } 
               else
               {
                  ranges[i].Max = 0 - referenceValue;
                  ranges[i].Min = referenceValue;
               }
            } 
            else
            {
               referenceValue = AoChannel_getExtRefUnipolar(aoChannel);
               if (referenceValue >= 0)
               {
                  ranges[i].Max = referenceValue;
                  ranges[i].Min = 0;
               } 
               else
               {
                  ranges[i].Max = 0;
                  ranges[i].Min = referenceValue;
               }
            }
         }
      }
      else
      {
         ret = AdxGetValueRangeInformation(valRange, 0, NULL, &ranges[i], NULL);
         if (BioFailed(ret))
         {
            return ret;
         }
      }
   }

   //generate waveform data and put them into the buffer which the parameter 'waveBuffer' give in, the Amplitude these waveform
   for (i = 0; i < oneWaveSamplesCount; i++)
   {
      int32 j;
      for (j = channelStart; j < channelStart + channelCount; j++)
      {
         double amplitude;
         double offset;

         channel = j % channelCountMax;
         amplitude = (ranges[channel].Max - ranges[channel].Min) / 2;
         offset = (ranges[channel].Max + ranges[channel].Min) / 2;

         switch (style)
         {
         case Sine:
            *waveBuffer++ = amplitude*(sin((double)i*2.0*( 3.14159 )/oneWaveSamplesCount )) + offset;
            break;
         case Sawtooth:
            if ((i >= 0) && (i < (oneWaveSamplesCount / 4.0)))
            {
               *waveBuffer++ = amplitude*(sin((double)i*2.0*( 3.14159 )/oneWaveSamplesCount )) + offset;
            }
            else
            {
               if ((i >= (oneWaveSamplesCount / 4.0)) && (i < 3 *(oneWaveSamplesCount / 4.0)))
               {
                  *waveBuffer++ = amplitude* ((2.0*(oneWaveSamplesCount/4.0)-i)/(oneWaveSamplesCount/4.0)) + offset;
               } 
               else
               {
                  *waveBuffer++ = amplitude* ((i-oneWaveSamplesCount)/(oneWaveSamplesCount/4.0)) + offset;
               }
            }
            break;
         case Square:
            if ((i >= 0) && (i < (oneWaveSamplesCount /2)))
            {
               *waveBuffer++ = amplitude * 1 +offset;
            } 
            else
            {
               *waveBuffer++ = amplitude * (-1) + offset;
            }
            break;
         default:
            printf("invalid wave style,generate waveform error !");
            ret = ErrorUndefined;
         }
      }
   }
   return ret;
}