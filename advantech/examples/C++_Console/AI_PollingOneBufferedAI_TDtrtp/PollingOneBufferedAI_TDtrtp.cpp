/*******************************************************************************
Copyright (c) 1983-2016 Advantech Co., Ltd.
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
*    PollingOneBufferedAI_TDtrtp.cpp
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to use Polling One Buffered AI with Trigger Delay to
*    Start and delay to stop function.
*
* Instructions for Running:
*    1. Set the 'deviceDescription' which can get from system device manager for opening the device. 
*	  2. Set the 'profilePath' to save the profile path of being initialized device. 
*    3. Set the 'startChannel' as the first channel for scan analog samples  
*    4. Set the 'channelCount' to decide how many sequential channels to scan analog samples.
*    5. Set the 'sectionLength' as the length of data section for Buffered AI.
*	  6. Set the 'sectionCount' as the count of data section for Buffered AI.
*    7. Set 'trigger parameters' to decide trigger property. 
* I/O Connections Overview:
*    Please refer to your hardware reference manual.
*
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "../inc/compatibility.h"
#include "../../../inc/bdaqctrl.h"
using namespace Automation::BDaq;
//-----------------------------------------------------------------------------------
// Configure the following parameters before running the demo
//-----------------------------------------------------------------------------------
#define      deviceDescription  L"PCIE-1810,BID#0"
wchar_t const *filePath = L"../../profile/PCIE-1810.xml";
int32         startChannel = 0;
const int32   channelCount = 2;
const int32   sectionLength = 1024;
const int32   sectionCount = 1;

#define      USER_BUFFER_SIZE    channelCount*sectionLength*sectionCount
double       Data[USER_BUFFER_SIZE];
int32 returnedCount = 0;

//Set Trigger parameters
TriggerAction triggerAction = DelayToStart;
ActiveSignal  triggerEdge = RisingEdge;
int           triggerDelayCount = 1000;
double        triggerLevel = 3.0;

//Set trigger1 paramater
TriggerAction trigger1Action = DelayToStop;
ActiveSignal  trigger1Edge = RisingEdge;
int           trigger1DelayCount = 1000;
double        trigger1Level = 3.0;

inline void waitAnyKey()
{
   do {SLEEP(1);} while (!kbhit());
}

int main(int argc, char *argv[])
{
   ErrorCode ret = Success;

   // Step 1: Create a 'WaveformAiCtrl' for buffered AI function.
   WaveformAiCtrl* wfAiCtrl = WaveformAiCtrl::Create();

   do 
   {
      // Step 2: Select a device by device number or device description and specify the access mode.
      // in this example we use ModeWrite mode so that we can fully control the device, including configuring, sampling, etc.
      DeviceInformation devInfo(deviceDescription);
      ret = wfAiCtrl->setSelectedDevice(devInfo);
      CHK_RESULT(ret);
      ret = wfAiCtrl->LoadProfile(filePath);//Loads a profile to initialize the device. 
      CHK_RESULT(ret);

      if (!(wfAiCtrl->getFeatures()->getTriggerCount() > 1))
      {
         printf("The device does not support this demo!\n");
         break;
      }
      
      // Step 3: Set necessary parameters for Buffered AI operation, 
      Conversion* conversion = wfAiCtrl->getConversion();
      ret = conversion->setChannelStart(startChannel);
      CHK_RESULT(ret);
      ret = conversion->setChannelCount(channelCount);
      CHK_RESULT(ret);
		Record* record = wfAiCtrl->getRecord();
      ret = record->setSectionLength(sectionLength);
      CHK_RESULT(ret);
      ret = record->setSectionCount(sectionCount);//The sectionCount is nonzero value, which means 'One Buffered' mode.
      CHK_RESULT(ret);

      //Step 4: Trigger parameters setting
      int32 trgCount = wfAiCtrl->getFeatures()->getTriggerCount();
      if (trgCount)
      {
         Trigger* trigger = wfAiCtrl->getTrigger();
         ret = trigger->setAction(triggerAction);
         CHK_RESULT(ret);
			/******************************************************************************************/
			/*The different kinds of devices have different trigger source. The details see manual.
			/*In this example, we use the PCIE-1810 and set 'SigExtDigClock' as the default trigger source.
			/******************************************************************************************/
         Array<SignalDrop>* srcs = wfAiCtrl->getFeatures()->getTriggerSources();
         ret = trigger->setSource(srcs->getItem(1));//To PCIE-1810, the 1 means 'SigExtDigClock'.
         CHK_RESULT(ret);
         ret = trigger->setDelayCount(triggerDelayCount);
         CHK_RESULT(ret);
         ret = trigger->setEdge(triggerEdge);
         CHK_RESULT(ret);
         ret = trigger->setLevel(triggerLevel);
         CHK_RESULT(ret);
      }

      if (trgCount > 1)
      {
         Trigger* trigger1 = wfAiCtrl->getTrigger1();
         ret = trigger1->setAction(trigger1Action);
         CHK_RESULT(ret);
         Array<SignalDrop>* srcs = wfAiCtrl->getFeatures()->getTrigger1Sources();
         ret = trigger1->setSource(srcs->getItem(1));
         CHK_RESULT(ret);
         ret = trigger1->setDelayCount(trigger1DelayCount);
         CHK_RESULT(ret);
         ret = trigger1->setEdge(trigger1Edge);
         CHK_RESULT(ret);
         ret = trigger1->setLevel(trigger1Level);
         CHK_RESULT(ret);
      }
      
      // Step 5: The acquisition has been started. 
      printf("Polling finite acquisition is in progress.\n");
		ret = wfAiCtrl->Prepare();
		CHK_RESULT(ret);
      ret = wfAiCtrl->Start();
      CHK_RESULT(ret);

      //Step 6:GetData with Polling Style
      ret = wfAiCtrl->GetData(USER_BUFFER_SIZE, Data, -1, &returnedCount);//The timeout value is -1, meaning infinite waiting.
      CHK_RESULT(ret);
		printf("Polling One Buffered AI get data count is  %d\n", returnedCount);
		if (ret == Success)
		{
			printf("The first sample each channel are:\n");
			for (int32 i = 0; i < channelCount; i++)
			{
				printf("channel %d: %10.6f \n", (i + startChannel), Data[i]);   
			}
		}
		int32 delayCount = wfAiCtrl->getTrigger1()->getDelayCount();
		int triggerPointIndex = returnedCount/channelCount - delayCount;
		printf("trigger point each channel: %d\n", triggerPointIndex);
		printf("Acquisition has completed!\n");
      
      // step 7: stop the operation if it is running.
      ret = wfAiCtrl->Stop();  
      CHK_RESULT(ret);
   } while (false);
   
   // Step 8: close device, release any allocated resource before quit.
   wfAiCtrl->Dispose();

   // If something wrong in this execution, print the error code on screen for tracking.
   if(BioFailed(ret))
   {
      wchar_t enumString[256];
      AdxEnumToString(L"ErrorCode", (int32)ret, 256, enumString);
      printf("Some error occurred. And the last error code is 0x%X. [%ls]\n", ret, enumString);
   }
   waitAnyKey();
   return 0;
}
