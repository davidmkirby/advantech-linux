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
*     ContinueCompare.c
*
* Example Category:
*    Counter
*
* Description:
*    This example demonstrates how to use Continue Compare Counter function.
*
* Instructions for Running:
*    1  Set the 'deviceDescription' for opening the device. 
*	  2  Set the 'profilePath' to save the profile path of being initialized device. 
*    3  Set the 'channelStart' as the start channel of the counter to operate
*	  4  Set the 'channelCount' as the channel count of the counter to operate.
*
* I/O Connections Overview:
*    Please refer to your hardware reference manual.
*
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "../../../inc/bdaqctrl.h"
#include "../inc/compatibility.h"
//-----------------------------------------------------------------------------------
// Configure the following parameters before running the demo
//-----------------------------------------------------------------------------------
#define     deviceDescription L"PCI-1784,BID#0"
const wchar_t* profilePath = L"../../profile/PCI-1784.xml";

int32       channelStart = 0;
int32       channelCount = 1;

void waitAnyKey()
{
   do {SLEEP(1);} while (!kbhit());
}

int comValueTab[2][3] = {{50, 100, 150},{1000, 1304, 1755}};

int conCmpOccursCount = 0;
int tabIndex = 0;
int const evntID[8] = {EvtCntCompareTableEnd0,EvtCntCompareTableEnd1,EvtCntCompareTableEnd2,EvtCntCompareTableEnd3,
                       EvtCntCompareTableEnd4,EvtCntCompareTableEnd5,EvtCntCompareTableEnd6,EvtCntCompareTableEnd7};
int const evntCompID[8] = {EvtCntPatternMatch0,EvtCntPatternMatch1,EvtCntPatternMatch2,EvtCntPatternMatch3,
                           EvtCntPatternMatch4,EvtCntPatternMatch5,EvtCntPatternMatch6,EvtCntPatternMatch7};
void BDAQCALL OnUdCntrEvent(void * sender, UdCntrEventArgs * args, void *userParam);

int main(int argc, char* argv[])
{
   ErrorCode ret = Success;
   IArray * channels = 0;
   UdChannel * udChannel = 0;
   int index = 0;
   int32 value = 0;
   wchar_t enumString[256];

   // Step 1: Create a 'UdCounterCtrl' for UpDown Counter function.
   UdCounterCtrl * udCounterCtrl = UdCounterCtrl_Create();

   // Step 2: Set the notification event Handler by which we can known the state of operation effectively.
   UdCounterCtrl_addUdCntrEventHandler(udCounterCtrl, OnUdCntrEvent, NULL);
   
   do 
   {
      // Step 3: Select a device by device number or device description and specify the access mode.
      // in this example we use ModeWrite mode so that we can fully control the device, including configuring, sampling, etc.
      DeviceInformation devInfo;
      devInfo.DeviceNumber = -1;
      devInfo.DeviceMode = ModeWrite;
      devInfo.ModuleIndex = 0;
      wcscpy(devInfo.Description, deviceDescription);
      ret = UdCounterCtrl_setSelectedDevice(udCounterCtrl, &devInfo);
      CHK_RESULT(ret);
      ret = UdCounterCtrl_LoadProfile(udCounterCtrl, profilePath);//Loads a profile to initialize the device.
      CHK_RESULT(ret);

      // Step 4: Set necessary parameters
      ret = UdCounterCtrl_setChannelStart(udCounterCtrl, channelStart);
      CHK_RESULT(ret);
      ret = UdCounterCtrl_setChannelCount(udCounterCtrl, channelCount);
      CHK_RESULT(ret);

      // Step 5: Set counting type for UpDown Counter
		/******************************************************************************************************************/
		/*In this example, we use the PCIE-1784 and set 'PulseDirection' as the default CountingType.The details see manual.
		/******************************************************************************************************************/
      channels = UdCounterCtrl_getChannels(udCounterCtrl);
      for (index = channelStart; index < channelStart + channelCount; index++)
      {
         udChannel = (UdChannel *)Array_getItem(channels, index);
         ret = UdChannel_setCountingType(udChannel, PulseDirection);
         CHK_RESULT(ret);
      }
      CHK_RESULT(ret);
      
      //Step 6: Set compare table
      ret = UdCounterCtrl_CompareAppendTable(udCounterCtrl, channelStart, 3, comValueTab[tabIndex]);
      CHK_RESULT(ret);
      
      // Step 7: Start UpDown Counter 
      ret = UdCounterCtrl_setEnabled(udCounterCtrl, TRUE);
      CHK_RESULT(ret);
      
      // Step 8: Read counting value: connect the input signal to channels you selected to get UpDown counter value.
      printf("UnDown counter is in progress...\nconnect the input signal to ");
      printf("any key will stop UnDown counter!\n\n");
      while ( !kbhit())
      {
         SLEEP(1);//get event UpDown count value per second
         ret = UdCounterCtrl_Read(udCounterCtrl, 1, &value);
         CHK_RESULT(ret);
         printf("channel %d Current UpDown count: %d\n\n ", channelStart, value);
      }
      
      // Step 9: stop UpDown Counter
      ret = UdCounterCtrl_CompareClear(udCounterCtrl, channelStart);
      CHK_RESULT(ret);
      ret = UdCounterCtrl_setEnabled(udCounterCtrl, FALSE);
      CHK_RESULT(ret);
   } while (FALSE);
   
   // Step 10: Close device and release any allocated resource.
   UdCounterCtrl_Dispose(udCounterCtrl);
   
   // If something wrong in this execution, print the error code on screen for tracking.
   if(BioFailed(ret))
   {
      AdxEnumToString(L"ErrorCode", (int32)ret, 256, enumString);
      printf("Some error occurred. And the last error code is 0x%X. [%ls]\n", ret, enumString);
      waitAnyKey();
   }  
   return 0;
}
// This function is used to deal with 'Cntr' Event, we should overwrite the virtual function CntrEvent.
void BDAQCALL OnUdCntrEvent(void * sender, UdCntrEventArgs * args, void *userParam)
{
   UdCounterCtrl * udCounterCtrl = (UdCounterCtrl *)sender;
   int channel = UdCounterCtrl_getChannelStart(udCounterCtrl);

   if (evntCompID[channel] == args->Id)
   {
      printf("Channel %d Compare occurs %d time(times)\n", channel, ++conCmpOccursCount);
      printf("Compare value is %d\n\n",comValueTab[(conCmpOccursCount-1) / 3][(conCmpOccursCount-1) % 3]);
   }
   else if (evntID[channel] == args->Id)//CompareAppendTable set success
   {
      printf("Channel %d Compare end\n", channel);
      if(++tabIndex < 2)
      {
         UdCounterCtrl_CompareAppendTable(udCounterCtrl, channel, 3, comValueTab[tabIndex]);
      }
   }
}
