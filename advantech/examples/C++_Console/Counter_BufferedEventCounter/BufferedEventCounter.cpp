/*******************************************************************************
Copyright (c) 1983-2017 Advantech Co., Ltd.
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
*     BufferedEventCounter.cpp
*
* Example Category:
*    Counter
*
* Description:
*    This example demonstrates how to use Buffered Event Counter function.
*
* Instructions for Running:
*    1  Set the 'deviceDescription' for opening the device. 
*	  2  Set the 'profilePath' to save the profile path of being initialized device. 
*
* I/O Connections Overview:
*    Please refer to your hardware reference manual.
*
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "../inc/compatibility.h"
#include "../../../inc/bdaqctrl.h"
using namespace Automation::BDaq;
//-----------------------------------------------------------------------------------
// Configure the following parameters before running the demo
//-----------------------------------------------------------------------------------
#define     deviceDescription L"PCIE-1813,BID#15"
const wchar_t* profilePath = L"../../profile/PCIE-1813.xml";
const int32  sectionLength = 1024;
const int32  sectionCount  = 0;

// user buffer size should be equal or greater than raw data buffer length, because data ready count
// is equal or more than smallest section of raw data buffer and up to raw data buffer length.
// users can set 'USER_BUFFER_SIZE' according to demand.
#define    USER_BUFFER_SIZE   sectionLength
int32      userDataBuffer[USER_BUFFER_SIZE];

inline void waitAnyKey()
{
   do{SLEEP(1);} while(!kbhit());
} 

void BDAQCALL OnDataReadyEvent(void * sender, BfdCntrEventArgs * args, void *userParam)
{	
	int32 returnedCount = 0;
	BufferedEventCounterCtrl *ctrl = (BufferedEventCounterCtrl *)sender;
   int32 getDataCount = MinValue(USER_BUFFER_SIZE, args->Count);
	ErrorCode ret = ctrl->GetData(args->Channel, getDataCount, userDataBuffer, 0, &returnedCount);
	// in this demo, we show only the first sample of each channel's new data
   if(!BioFailed(ret)) {
      printf("Buffered Event Counting get data count is  %d\n", returnedCount);
      printf("the first sample for Channel %d is: %d\n", args->Channel, userDataBuffer[0]);
   }
}
// This function is used to deal with 'Overrun' Event.
void BDAQCALL OnOverRunEvent(void * sender, BfdCntrEventArgs * args, void *userParam)
{
	printf("Buffered Event Counting Overrun: channel = %d, offset = %d, count = %d\n", args->Channel, args->Offset, args->Count);
}
// This function is used to deal with 'CacheOverflow' Event.
void BDAQCALL OnCacheOverflowEvent(void * sender, BfdCntrEventArgs * args, void *userParam)
{
	printf("Buffered Event Counting Cache Overflow: channel = %d, offset = %d, count = %d\n", args->Channel, args->Offset, args->Count);
}
// This function is used to deal with 'Stopped' Event.
void BDAQCALL OnStoppedEvent(void * sender, BfdCntrEventArgs * args, void *userParam)
{
   printf("Buffered Event Counting stopped: channel = %d, offset = %d, count = %d\n", args->Channel, args->Offset, args->Count);
}

int main(int argc, char* argv[])
{
   ErrorCode ret = Success;
   // Step 1: Create a 'BufferedEventCounterCtrl' for Buffered Event Counter function.
   BufferedEventCounterCtrl* bfdEventCounterCtrl = BufferedEventCounterCtrl::Create();

   // Step 2: Set the notification event Handler by which we can known the state of operation effectively.
   bfdEventCounterCtrl->addDataReadyHandler(OnDataReadyEvent, NULL);
   bfdEventCounterCtrl->addOverrunHandler(OnOverRunEvent, NULL);
   bfdEventCounterCtrl->addCacheOverflowHandler(OnCacheOverflowEvent, NULL);
   bfdEventCounterCtrl->addStoppedHandler(OnStoppedEvent, NULL);
   do
   {
      // Step 3: Select a device by device number or device description and specify the access mode.
      // in this example we use ModeWrite mode so that we can fully control the device, including configuring, sampling, etc.
      DeviceInformation devInfo(deviceDescription);
      ret = bfdEventCounterCtrl->setSelectedDevice(devInfo);
      CHK_RESULT(ret);
      ret = bfdEventCounterCtrl->LoadProfile(profilePath);//Loads a profile to initialize the device.
      CHK_RESULT(ret);

      // Step 4: Set necessary parameters.
      int32 chCount     = 4;
      int8  chEnabled[] = {1, 0, 0, 0};
      ret = bfdEventCounterCtrl->setChannelMap(chCount, chEnabled);
	   CHK_RESULT(ret);

      ret = bfdEventCounterCtrl->setSampleClkSrc(SigExtDigClock);
      CHK_RESULT(ret);

      Record* record = bfdEventCounterCtrl->getRecord();
      {
         ret = record->setSectionCount(sectionCount); //The 0 means setting 'streaming' mode
         CHK_RESULT(ret);
         ret = record->setSectionLength(sectionLength);
         CHK_RESULT(ret);
      }

      // Setting trigger
      int32 trgCount = bfdEventCounterCtrl->getFeatures()->getTriggerCount();
      if(trgCount) {
         Trigger* trigger = bfdEventCounterCtrl->getTrigger(); 
         ret = trigger->setSource(SignalNone); // The "SignalNone" means doing without trigger.
         CHK_RESULT(ret);
      }

      // Step 5: Start Buffered Event Counter 
      ret = bfdEventCounterCtrl->Prepare();
      CHK_RESULT(ret);

	   ret= bfdEventCounterCtrl->setEnabled(true);
	   CHK_RESULT(ret);

      // Step 6: Read counting value: connect the input signal to channels you selected to get counter value.
      printf("Buffered Event Counter is in progress...\nConnect the input signal to channels you selected to get counter value!\n");
	   printf("any key will stop buffered event counter!\n\n");
	   while ( !kbhit()) {
         SLEEP(1);
      }

	   // step 7: Stop the operation if it is running.
	   ret = bfdEventCounterCtrl->setEnabled(false);
	   CHK_RESULT(ret);
   }while(false);

   // Step 8: Close device, release any allocated resource.
	bfdEventCounterCtrl->Dispose();

	// If something wrong in this execution, print the error code on screen for tracking.
   if(BioFailed(ret))
   {
      wchar_t enumString[256];
      AdxEnumToString(L"ErrorCode", (int32)ret, 256, enumString);
      printf("Some error occurred. And the last error code is 0x%X. [%ls]\n", ret, enumString);
      waitAnyKey();
   }  
   return 0;
}
