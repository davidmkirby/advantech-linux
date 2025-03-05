/*******************************************************************************
Copyright (c) 1983-2020 Advantech Co., Ltd.
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
*    StreamingDI_TDtr.cpp
*
* Example Category:
*    DI
*
* Description:
*    This example demonstrates how to use Streaming DI with Trigger Delay to Start function.
*
* Instructions for Running:
*    1. Set the 'deviceDescription' which can get from system device manager for opening the device.
*	  2. Set the 'profilePath' to save the profile path of being initialized device.
*    3. Set the 'portEnabled' to decide which port to be used.
*    4. Set the 'sectionLength' as the length of data section for Buffered DI.
*	  5. Set the 'sectionCount' as the count of data section for Buffered DI.
*    6. Set the 'trigger parameters' to decide trigger property.
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
#define       deviceDescription  L"iDAQ-731,BID#0"
const wchar_t* profilePath = L"../../profile/iDAQ-731_0.xml";

int8         portEnabled[] = {1, 0, 0, 0};
const int32  enabledCount  = 1; // The value depend on portEnabled

int32        convClkRate   = 1000;
const int32  sectionLength = 1024;
const int32  sectionCount  = 0;	

int32 trgUsed = 0; // 0: trigger0, 1: trigger1

// user buffer size should be equal or greater than raw data buffer length, because data ready count
// is equal or more than smallest section of raw data buffer and up to raw data buffer length.
// users can set 'USER_BUFFER_SIZE' according to demand.
#define  USER_BUFFER_LENGTH  sectionLength * enabledCount
int8     dataBuf[USER_BUFFER_LENGTH]; 

// Set trigger parameters
TriggerAction triggerAction = DelayToStart;
ActiveSignal  triggerEdge = RisingEdge;
int    triggerDelayCount = 600;
double triggerLevel = 3.5;

//for trigger1 parameters 
TriggerAction trigger1Action = DelayToStart;
ActiveSignal  trigger1Edge = RisingEdge;
int    trigger1DelayCount = 600;
double trigger1Level = 3.5;

inline void waitAnyKey()
{
   do{SLEEP(1);} while(!kbhit());
} 
// This function is used to deal with 'DataReady' Event.
void BDAQCALL OnDataReadyEvent(void * sender, BfdDiEventArgs * args, void *userParam)
{
   int32 returnedCount = 0;
   int32 remainingCount = args->Count;
   BufferedDiCtrl* bfdDICtrl = (BufferedDiCtrl*)sender;

   do
   {
      int32 getDataCount = MinValue(USER_BUFFER_LENGTH, remainingCount);
      printf("\nGetDataCount[ %d ]\n", getDataCount);
      bfdDICtrl->GetData(getDataCount, dataBuf, 0, &returnedCount);
      remainingCount -= returnedCount;
   } while (remainingCount > 0);

   // Show each channel's new data   
   printf("the port data:\n");
   for (int32 i = 0; i < BufLength(portEnabled); ++i) {
      if (portEnabled[i] == 0) { continue; }
      int8 data = dataBuf[i];
      printf("Port %d: 0x%x \n", i, data);      
   }
}
// This function is used to deal with 'Overrun' Event.
void BDAQCALL OnOverRunEvent(void * sender, BfdDiEventArgs * args, void *userParam)
{
   printf("Buffered DI Overrun: offset = %d, count = %d\n", args->Offset, args->Count);
}
// This function is used to deal with 'CacheOverflow' Event.
void BDAQCALL OnOverCacheOverflowEvent(void * sender, BfdDiEventArgs * args, void *userParam)
{
   printf("Buffered DI Cache Overflow: offset = %d, count = %d\n", args->Offset, args->Count);
}
// This function is used to deal with 'Stopped' Event.
void BDAQCALL OnStoppedEvent(void * sender, BfdDiEventArgs * args, void *userParam)
{
   int32 returnedCount = 0, returnedSumCount = 0;
   int32 remainingCount = args->Count;
   BufferedDiCtrl* bfdDICtrl = (BufferedDiCtrl*)sender;

   do
   {
      int32 getDataCount = MinValue(USER_BUFFER_LENGTH, remainingCount);
      printf("\nGetDataCount[ %d ]\n", getDataCount);
      bfdDICtrl->GetData(getDataCount, dataBuf, 0, &returnedCount);
      remainingCount -= returnedCount;
      returnedSumCount += returnedCount;
   } while (remainingCount > 0);

   printf(" Buffered DI Stopped Event get data count is  %d£¬argsCount is %d\n", returnedSumCount, args->Count);
}

int main(int argc, char* argv[])
{
   ErrorCode        ret = Success;

   // Step 1: Create a 'BufferedDiCtrl' for buffered DI function.
   BufferedDiCtrl* bfdDiCtrl = BufferedDiCtrl::Create();

	// Step 2: Set the notification event Handler by which we can known the state of operation effectively.
   bfdDiCtrl->addDataReadyHandler(OnDataReadyEvent, NULL);
   bfdDiCtrl->addOverrunHandler(OnOverRunEvent, NULL);
   bfdDiCtrl->addCacheOverflowHandler(OnOverRunEvent, NULL);
   bfdDiCtrl->addStoppedHandler(OnStoppedEvent, NULL);
   do
   {
      // Step 3: Select a device by device number or device description and specify the access mode.
      // in this example we use ModeWrite mode so that we can fully control the device, including configuring, sampling, etc.
      DeviceInformation devInfo(deviceDescription);
      ret = bfdDiCtrl->setSelectedDevice(devInfo);
      CHK_RESULT(ret);
      ret = bfdDiCtrl->LoadProfile(profilePath);//Loads a profile to initialize the device.
      CHK_RESULT(ret);
		
      // Step 4: Set necessary parameters.
      int32 bufLen = BufLength(portEnabled);
      bool canSetDir = bfdDiCtrl->getFeatures()->getPortProgrammable();
      if (canSetDir)
      {
         Array<DioPort>* dioPorts = bfdDiCtrl->getPorts();
         for (int32 i = 0; i < bufLen; ++i) {
            if (portEnabled[i] == 0) { continue; }
            ret = dioPorts->getItem(i).setDirectionMask(Input);// Set DIO ports direction;
            CHK_RESULT(ret);
         }      
         CHK_RESULT(ret);
      }

      ScanPort* scanPort = bfdDiCtrl->getScanPort();
      ret = scanPort->setPortMap(bufLen, portEnabled);
      CHK_RESULT(ret);

      ret = scanPort->setSectionLength(sectionLength);
      CHK_RESULT(ret);
      ret = scanPort->setSectionCount(sectionCount);//The 0 means setting 'streaming' mode.
      CHK_RESULT(ret);

      ConvertClock* convClk = bfdDiCtrl->getConvertClock();
      ret = convClk->setSource(SigInternalClock);
      CHK_RESULT(ret);
      ret = convClk->setRate(convClkRate);
      CHK_RESULT(ret);
		
		//Step 5: Trigger parameters setting
      //for trigger0
      int32 trgCount = bfdDiCtrl->getFeatures()->getDiTriggerCount();      
		if (trgCount > 0 && trgUsed == 0){
         Trigger* trigger = bfdDiCtrl->getTrigger();
			ret = trigger->setAction(triggerAction);
         CHK_RESULT(ret);
			/******************************************************************************************/
			/*The different kinds of devices have different trigger source. The details see manual.
			/*In this example, we use the iDAQ-731 and set 'SigPFP0' as the default trigger source.
			/******************************************************************************************/
         Array<SignalDrop>*  srcs = bfdDiCtrl->getFeatures()->getDiTriggerSources();
			int sourceCount = srcs->getLength();//Uncomment this line, user can get the count of supported trigger source.
         ret = trigger->setSource(SigPFP0);
         CHK_RESULT(ret);
			ret = trigger->setDelayCount(triggerDelayCount);
         CHK_RESULT(ret);
			ret = trigger->setEdge(triggerEdge);
         CHK_RESULT(ret);
         /***********************************************************************************/
			/* If the triggerSource is 'SigPFP', 'setLevel' will not work.*/
			/* If not, please uncomment it.
			/***********************************************************************************/
			//ret = trigger->setLevel(triggerLevel);
         //CHK_RESULT(ret);
		}else if(trgCount > 1 && trgUsed == 1){
         //for trigger1
			Trigger* trigger1 = bfdDiCtrl->getTrigger1();
         ret = trigger1->setAction(trigger1Action);
         CHK_RESULT(ret);
         Array<SignalDrop>*  srcs = bfdDiCtrl->getFeatures()->getDiTrigger1Sources();
         ret = trigger1->setSource(SigPFP1);
         CHK_RESULT(ret);
         ret = trigger1->setDelayCount(trigger1DelayCount) ;
         CHK_RESULT(ret);
         ret = trigger1->setEdge(trigger1Edge);
         CHK_RESULT(ret);
         /***********************************************************************************/
			/* If the triggerSource is 'SigPFP', 'setLevel' will not work.*/
			/* If not, please uncomment it.
			/***********************************************************************************/
         //ret = trigger1->setLevel(trigger1Level);
         //CHK_RESULT(ret);
      } else {
         printf("The device can not support trigger function! \n any key to quit.");
         break;
      }      

      // Step 6: The operation has been started.
      // We can get samples via event handlers.
		ret = bfdDiCtrl->Prepare();
		CHK_RESULT(ret);
      ret = bfdDiCtrl->Start();
      CHK_RESULT(ret);

      // Step 7: The device is acquiring data.
		printf("Streaming DI is in progress.\nplease wait...  any key to quit!\n\n");
      do
      {
         SLEEP(1);
      }	while(!kbhit());

      // step 8: Stop the operation if it is running.
      ret = bfdDiCtrl->Stop(); 
      CHK_RESULT(ret);
   }while(false);

	// Step 9: Close device, release any allocated resource.
	bfdDiCtrl->Dispose(); 

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



