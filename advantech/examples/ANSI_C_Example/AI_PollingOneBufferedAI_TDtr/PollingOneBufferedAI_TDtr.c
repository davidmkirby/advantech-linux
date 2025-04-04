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
*    PollingOneBufferedAI_TDtr.c
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to use Polling One Buffered AI with Trigger Delay to
*    Start function.
*
* Instructions for Running:
*    1. Set the 'deviceDescription' which can get from system device manager for opening the device. 
*	  2. Set the 'profilePath' to save the profile path of being initialized device. 
*    3. Set the 'startChannel' as the first channel for scan analog samples  
*    4. Set the 'channelCount' to decide how many sequential channels to scan analog samples.
*    5. Set the 'sectionLength' as the length of data section for Buffered AI.
*	  6. Set the 'sectionCount' as the count of data section for Buffered AI.
*    7. Set 'trigger parameters' to decide trigger property. 
*
* I/O Connections Overview:
*    Please refer to your hardware reference manual.
*
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "../../../inc/bdaqctrl.h"
#include "../inc/compatibility.h"
//-----------------------------------------------------------------------------
// Configure the following parameters before running the demo
//-----------------------------------------------------------------------------
#define  deviceDescription  L"DemoDevice,BID#0"
const wchar_t* profilePath = L"../../profile/DemoDevice.xml";
#define  startChannel      0
#define  channelCount      2
#define  sectionLength     1024
#define  sectionCount      1
// user buffer size should be equal or greater than raw data buffer length, because data ready count
// is equal or more than smallest section of raw data buffer and up to raw data buffer length.
// users can set 'USER_BUFFER_SIZE' according to demand.
#define USER_BUFFER_SIZE   channelCount*sectionLength*sectionCount
double Data[USER_BUFFER_SIZE];
int32 returnedCount = 0;

//Set trigger parameter
TriggerAction triggerAction     = DelayToStart;
ActiveSignal  triggerEdge       = RisingEdge;
int           triggerDelayCount = 1000;
double        triggerLevel      = 3.0;

//Set trigger1 parameter
TriggerAction trigger1Action     = DelayToStart;
ActiveSignal  trigger1Edge       = RisingEdge;
int           trigger1DelayCount = 1000;
double        trigger1Level      = 3.0;

// set which trigger be used for this demo, trigger0(0) or trigger1(1).
int           triggerUsed = 0;

void waitAnyKey()
{
   do 
   {SLEEP(1);} while (!kbhit());
}

int main(int argc, char* argv[])
{
   ErrorCode      ret           = Success;
   Conversion *   conversion    = NULL;
   Record *       record        = NULL;
   Trigger *      trigger       = NULL;
   Trigger *      trigger1      = NULL;
   IArray *       srcs          = NULL;
   AiFeatures *   aiFeatures    = NULL;
   int32          i             = 0;
   int32          delayCount    = 0;
   int32          trgCount      = 0;
   wchar_t        enumString[256] ;

   // Step 1: Create a 'WaveformAiCtrl' for buffered AI function.
   WaveformAiCtrl * wfAiCtrl = WaveformAiCtrl_Create();

   do 
   {
      // Step 2: Select a device by device number or device description and specify the access mode.
      // in this example we use ModeWrite mode so that we can fully control the device, including configuring, sampling, etc.
      DeviceInformation devInfo;
      devInfo.DeviceNumber = -1;
      devInfo.DeviceMode   = ModeWrite;
      devInfo.ModuleIndex  = 0;
      wcscpy(devInfo.Description, deviceDescription);
      ret = WaveformAiCtrl_setSelectedDevice(wfAiCtrl, &devInfo);
      CHK_RESULT(ret);
      ret = WaveformAiCtrl_LoadProfile(wfAiCtrl, profilePath);//Loads a profile to initialize the device. 
      CHK_RESULT(ret);
		
		// Step 3: Set necessary parameters. 
      conversion = WaveformAiCtrl_getConversion(wfAiCtrl);
      ret = Conversion_setChannelStart(conversion, startChannel);
      CHK_RESULT(ret);
      ret = Conversion_setChannelCount(conversion, channelCount);
      CHK_RESULT(ret);
		record = WaveformAiCtrl_getRecord(wfAiCtrl);
      ret = Record_setSectionCount(record, sectionCount);//The sectionCount is nonzero value, which means 'One Buffered' mode.
      CHK_RESULT(ret);
      ret = Record_setSectionLength(record, sectionLength);
      CHK_RESULT(ret);

      //Step 4: Trigger parameters setting
      aiFeatures = WaveformAiCtrl_getFeatures(wfAiCtrl);
      trgCount = AiFeatures_getTriggerCount(aiFeatures);
      //for trigger0
      if (triggerUsed == 0)
      {
			if (trgCount)
			{
            trigger = WaveformAiCtrl_getTrigger(wfAiCtrl);
				ret = Trigger_setAction(trigger, triggerAction);
				CHK_RESULT(ret);
				/******************************************************************************************/
				/*The different kinds of devices have different trigger source. The details see manual.
				/*In this example, we use the DemoDevice and set 'Ai channel 0' as the default trigger source.
			   /******************************************************************************************/
				srcs = AiFeatures_getTriggerSources(aiFeatures);
				ret = Trigger_setSource(trigger, *(SignalDrop*)Array_getItem(srcs, 1));//To DemoDevice, the 1 means 'Ai channel 0'.
				CHK_RESULT(ret);
				ret = Trigger_setDelayCount(trigger, triggerDelayCount);
				CHK_RESULT(ret);
				ret = Trigger_setEdge(trigger, triggerEdge);
				CHK_RESULT(ret);
				ret = Trigger_setLevel(trigger, triggerLevel);
				CHK_RESULT(ret);
			}
			else
			{
				printf("The trigger1 can not support by the device! \n any key to quit.");
				break;
			}
      } 
      else if(triggerUsed == 1)
      {
			//for Trigger1
			if (trgCount > 1)
			{
				trigger1 = WaveformAiCtrl_getTrigger1(wfAiCtrl);
				ret = Trigger_setAction(trigger1, trigger1Action);
				CHK_RESULT(ret);
				srcs = AiFeatures_getTrigger1Sources(aiFeatures);
				ret = Trigger_setSource(trigger1, *(SignalDrop*)Array_getItem(srcs, 1));
				CHK_RESULT(ret);
				ret = Trigger_setDelayCount(trigger1, trigger1DelayCount);
				CHK_RESULT(ret);
				ret = Trigger_setEdge(trigger1, trigger1Edge);
				CHK_RESULT(ret);
				ret = Trigger_setLevel(trigger1, trigger1Level);
				CHK_RESULT(ret);
			} 
			else
			{
				printf("The device can not support trigger function! \n any key to quit.");
				break;
			}
      }

      // Step 5: The operation has been started.
		printf("Polling finite acquisition is in progress.\n");
		ret = WaveformAiCtrl_Prepare(wfAiCtrl);
		CHK_RESULT(ret);
		ret = WaveformAiCtrl_Start(wfAiCtrl);
      CHK_RESULT(ret);

      //Step 6: GetData with Polling Style
      ret = WaveformAiCtrl_GetDataF64(wfAiCtrl, USER_BUFFER_SIZE, Data, -1, &returnedCount, NULL, NULL, NULL);//The timeout value is -1, meaning infinite waiting.
      CHK_RESULT(ret);
		printf("Polling One Buffered AI get data count is  %d\n", returnedCount);
		if (ret == Success)
		{
			printf(" The first sample each channel are:\n");
			for (i = 0; i < channelCount; i++)
			{
				printf("channel %d: %10.6f \n", (i + startChannel), Data[i]);   
			}
		}
      printf("Acquisition has completed!\n\n");

      // step 7: stop the operation if it is running.
      ret = WaveformAiCtrl_Stop(wfAiCtrl);  
      CHK_RESULT(ret);
   } while (FALSE);
   
   //Step 8: close device, release any allocated resource before quit.
   WaveformAiCtrl_Dispose(wfAiCtrl);

   // If something wrong in this execution, print the error code on screen for tracking.
   if(BioFailed(ret))
   {
      AdxEnumToString(L"ErrorCode", (int32)ret, 256, enumString);
      printf("Some error occurred. And the last error code is 0x%X. [%ls]\n", ret, enumString);
   }
   waitAnyKey();
   return 0;

}