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
*    PollingStreamingAI_Retrigger.cpp
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to use Polling Streaming AI with Retrigger function.
*
* Instructions for Running:
*    1. Set the 'deviceDescription' for opening the device.
*	  2. Set the 'profilePath' to save the profile path of being initialized device. 
*    3. Set the 'startChannel' as the first channel for scan analog samples  
*    4. Set the 'channelCount' to decide how many sequential channels to scan analog samples.
*    5. Set the 'sectionLength' as the length of data section for Buffered AI.
*	  6. Set the 'sectionCount' as the count of data section for Buffered AI.
*	  7. Set the 'Cycles'to decide the count of a series of data which is continuous in time domain.
*	  8. Set the 'clockRate'as the sampling count per second in Hz.
*    9. Set the 'trigger parameters' to decide trigger property.
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
#define       deviceDescription  L"PCIE-1840,BID#15"
const wchar_t* profilePath = L"../../profile/PCIE-1840.xml";
int32         startChannel = 0;
const int32   channelCount = 1;
const int32   sectionLength = 10240;
const int32   sectionCount = 0;
const int32   clockRate = 10000;
const int32   cycles = 2;
// user buffer size should be equal or greater than raw data buffer length, because data ready count
// is equal or more than smallest section of raw data buffer and up to raw data buffer length.
// users can set 'USER_BUFFER_SIZE' according to demand.
#define       USER_BUFFER_SIZE   channelCount*sectionLength
double userDataBuffer[USER_BUFFER_SIZE] = {0};

// Set trigger parameters
SignalDrop triggerSource = SigExtDigTrigger0;
TriggerAction triggerAction = DelayToStart;
ActiveSignal  triggerEdge = RisingEdge;
int           triggerDelayCount = 600;
double        triggerLevel = 3.0;

//for trigger1 parameters
SignalDrop trigger1Source = SigExtDigTrigger1;
TriggerAction trigger1Action = DelayToStop;
ActiveSignal  trigger1Edge = RisingEdge;
int           trigger1DelayCount = 600;
double        trigger1Level = 3.0;

inline void waitAnyKey()
{
   do{SLEEP(1);} while(!kbhit());
} 

int main(int argc, char* argv[])
{
   ErrorCode        ret = Success;

   // Step 1: Create a 'WaveformAiCtrl' for buffered AI function.
   WaveformAiCtrl * wfAiCtrl = WaveformAiCtrl::Create();

	do
	{
		// Step 3: Select a device by device number or device description and specify the access mode.
		// in this example we use ModeWrite mode so that we can fully control the device, including configuring, sampling, etc.
		DeviceInformation devInfo(deviceDescription);
		ret = wfAiCtrl->setSelectedDevice(devInfo);
		CHK_RESULT(ret);
		ret = wfAiCtrl->LoadProfile(profilePath);//Loads a profile to initialize the device. 
		CHK_RESULT(ret);
		if (!wfAiCtrl->getFeatures()->getRetriggerable())
		{
			printf("This device don't support the retrigger fuction!");
			break;
		}

		// Step 4: Set necessary parameters. 
      Conversion * conversion = wfAiCtrl->getConversion();
      ret = conversion->setChannelStart(startChannel);
      CHK_RESULT(ret);
      ret = conversion->setChannelCount(channelCount);
      CHK_RESULT(ret);
		ret = conversion->setClockRate(clockRate);
		CHK_RESULT(ret);
		Record * record = wfAiCtrl->getRecord();
      ret = record->setSectionCount(sectionCount);//The 0 means setting 'streaming' mode.
      CHK_RESULT(ret);
      ret = record->setSectionLength(sectionLength);
      CHK_RESULT(ret);
		ret = record->setCycles(cycles);
		CHK_RESULT(ret);
		
      int32 trgCount = wfAiCtrl->getFeatures()->getTriggerCount();
      bool  retriggerable = wfAiCtrl->getFeatures()->getRetriggerable();
      if (trgCount > 1 && retriggerable)
      {
         //for trigger0
         Trigger* trigger = wfAiCtrl->getTrigger();
         ret = trigger->setAction(triggerAction);
         CHK_RESULT(ret);
         /*************************************************************************************************/
         /*The different kinds of devices have different trigger source. The details see manual.
         /*In this example, we use the PCIE-1840 and set 'SigExtDigTrigger0' as the default trigger0 source.
         /**************************************************************************************************/
         ret = trigger->setSource(triggerSource);
         CHK_RESULT(ret);
         ret = trigger->setDelayCount(triggerDelayCount);
         CHK_RESULT(ret);
         ret = trigger->setEdge(triggerEdge);
         CHK_RESULT(ret);
         /***********************************************************************************/
         /* If the triggerSource is 'DigitalSignal', 'setLevel' will not work.*/
         /* If not, please uncomment it.
         /***********************************************************************************/
         //ret = trigger->setLevel(triggerLevel);
         //CHK_RESULT(ret);

         //for trigger1
		   Trigger* trigger1 = wfAiCtrl->getTrigger1();
         ret = trigger1->setAction(trigger1Action);
         CHK_RESULT(ret);
		   /******************************************************************************************/
		   /*Set 'SigExtDigTrigger1' as the default trigger1 source. The details see manual.
		   /******************************************************************************************/
         ret = trigger1->setSource(trigger1Source);
         CHK_RESULT(ret);
		   ret = trigger1->setDelayCount(trigger1DelayCount);
		   CHK_RESULT(ret);
		   ret = trigger1->setEdge(trigger1Edge);
		   CHK_RESULT(ret);
		   /***********************************************************************************/
		   /* If the triggerSource is 'DigitalSignal', 'setLevel' will not work.*/
		   /* If not, please uncomment it.
		   /***********************************************************************************/
		   //ret = trigger1->setLevel(trigger1Level);
		   //CHK_RESULT(ret);
      }
      
      // Step 5: the operation has been started.
      ret = wfAiCtrl->Prepare();
		CHK_RESULT(ret);
		ret = wfAiCtrl->Start();
      CHK_RESULT(ret);
      
      // Step 6: The device is acquiring data with Polling Style.
		printf("Polling infinite acquisition is in progress.\n");
      int32 recordIndex = 0;
      int32 returnedCount = 0;
		do
      {
         ret = wfAiCtrl->GetData(USER_BUFFER_SIZE, userDataBuffer, -1, &returnedCount);//The timeout value is -1, meaning infinite waiting. 
			CHK_RESULT(ret);
			printf("Polling Streaming AI get data count is  %d\n", returnedCount);
			printf("the first sample for each Channel are:\n");
			for (int i = 0; i < channelCount; i++)
			{
				printf("channel %d: %10.6f \n", (i + startChannel), userDataBuffer[i]);
			}

         if (ret == WarningRecordEnd)
         {
				printf("recordIndex %ld is finished\n", ++recordIndex);   
         }
      } while(ret != WarningFuncStopped);
		printf("Acquisition has completed!\n");

      // step 7: Stop the operation if it is running.
      ret = wfAiCtrl->Stop(); 
      CHK_RESULT(ret);
   }while(false);

   // Step 8: Close device, release any allocated resource.
   wfAiCtrl->Dispose();
	
	// If something wrong in this execution, print the error code on screen for tracking.
   if(BioFailed(ret))
   {
      wchar_t enumString[256];
      AdxEnumToString(L"ErrorCode", (int32)ret, 256, enumString);
      printf("Some error occurred. And the last error code is 0x%X. [%ls]\n", ret, enumString);
      waitAnyKey();// wait any key to quit!
   }
   return 0;
}
