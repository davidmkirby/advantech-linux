/*******************************************************************************
Copyright (c) 1983-2020 Advantech Co., Ltd.
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
*    AsynchronousOneBufferedDO.cpp
*
* Example Category
*    DO
* Description:
*    This example demonstrates how to use Asynchronous One Buffered DO with Trigger Delay to Stop function.
*
* Instructions for Running:
*    1. Set the 'deviceDescription' which can get from system device manager for opening the device.
*	  2. Set the 'profilePath' to save the profile path of being initialized device.
*    3. Set the 'portEnabled' to decide which port to be used.
*    4. Set the 'sectionLength' as the length of data section for Buffered DO.
*	  5. Set the 'sectionCount' as the count of data section for Buffered DO.
*    6. Set the 'trigger parameters' to decide trigger property.
*
* I/O Connections Overview:
*    Please refer to your hardware reference manual.
*
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../inc/compatibility.h"
#include "../../../inc/bdaqctrl.h"
using namespace Automation::BDaq;

//-----------------------------------------------------------------------------------
// Configure the following parameters before running the demo
//-----------------------------------------------------------------------------------
#define       deviceDescription  L"iDAQ-731,BID#0"
const wchar_t* profilePath = L"../../profile/iDAQ-731_0.xml";

int8         portEnabled[] = {1, 0, 0, 0};

int32        convClkRate   = 1000;
const int32  sectionLength = 1024;
const int32  sectionCount  = 1;

// Set trigger parameters
TriggerAction triggerAction = DelayToStop;
ActiveSignal  triggerEdge = RisingEdge;
int    triggerDelayCount = 600;
double triggerLevel = 3.5;

//for trigger1 parameters 
TriggerAction trigger1Action = DelayToStop;
ActiveSignal  trigger1Edge = RisingEdge;
int    trigger1DelayCount = 600;
double trigger1Level = 3.5;

// set which trigger be used for this demo, trigger0(0) or trigger1(1).
int32  triggerUsed = 0; // 0: trigger0, 1: trigger1

inline void waitAnyKey()
{
   do{SLEEP(1);} while(!kbhit());
} 
// This function is used to deal with 'Stopped' Event.
void BDAQCALL OnStoppedEvent(void* sender, BfdDoEventArgs* args, void* userParam)
{
   printf("\nBufferedDO stopped: offset = %d, count = %d\n", args->Offset, args->Count);
}

int main(int argc, char* argv[])
{
   ErrorCode ret = Success;
  
   // Step 1: Create a 'BufferedDoCtrl' for buffered DO function.
   BufferedDoCtrl* bfdDoCtrl = BufferedDoCtrl::Create();

	// Step 2: Set the notification event Handler by which we can known the state of operation effectively.
   bfdDoCtrl->addStoppedHandler(OnStoppedEvent, NULL);

   do
   {
      // Step 3: Select a device by device number or device description and specify the access mode.
      // in this example we use ModeWrit mode so that we can fully control the device, including configuring, sampling, etc.
      DeviceInformation devInfo(deviceDescription);
      ret = bfdDoCtrl->setSelectedDevice(devInfo);
      CHK_RESULT(ret);
      ret = bfdDoCtrl->LoadProfile(profilePath);//Loads a profile to initialize the device.
      CHK_RESULT(ret);
      
      // Step 4: Set necessary parameters.
      int32 bufLen = BufLength(portEnabled);
      bool canSetDir = bfdDoCtrl->getFeatures()->getPortProgrammable();
      if (canSetDir)
      {
         Array<DioPort>* dioPorts = bfdDoCtrl->getPorts();
         for (int32 i = 0; i < bufLen; ++i) {
            if (portEnabled[i] == 0) { continue; }
            ret = dioPorts->getItem(i).setDirectionMask(Output);// Set DIO ports direction;
            CHK_RESULT(ret);
         }      
         CHK_RESULT(ret);
      }

      ScanPort* scanPort = bfdDoCtrl->getScanPort();
      ret = scanPort->setPortMap(bufLen, portEnabled);
      CHK_RESULT(ret);

      ret = scanPort->setSectionLength(sectionLength);
      CHK_RESULT(ret);
      ret = scanPort->setSectionCount(sectionCount);//The non-zero means setting 'one-buffered' mode.
      CHK_RESULT(ret);

      ConvertClock* convClk = bfdDoCtrl->getConvertClock();
      ret = convClk->setSource(SigInternalClock);
      CHK_RESULT(ret);
      ret = convClk->setRate(convClkRate);
      CHK_RESULT(ret);

      //Step 5: Trigger parameters setting
      int32 trgCount = bfdDoCtrl->getFeatures()->getDoTriggerCount();
      //for trigger0
      if (trgCount > 0 && triggerUsed == 0){
         Trigger* trigger = bfdDoCtrl->getTrigger();
         ret = trigger->setAction(triggerAction);
         CHK_RESULT(ret);
			/******************************************************************************************/
			/*The different kinds of devices have different trigger source. The details see manual.
			/*In this example, we use the iDAQ-731 and set 'SigPFP0' as the default trigger source.
			/******************************************************************************************/
         Array<SignalDrop>*  srcs = bfdDoCtrl->getFeatures()->getDoTriggerSources();
         //int sourceCount = srcs->getLength();//Uncomment this line, user can get the count of supported trigger source.
         ret = trigger->setSource(SigPFP0);
         CHK_RESULT(ret);
         ret = trigger->setDelayCount(triggerDelayCount) ;
         CHK_RESULT(ret);
         ret = trigger->setEdge(triggerEdge);
         CHK_RESULT(ret);
         /***********************************************************************************/
	      /* If the triggerSource is 'SigPFP', 'setLevel' will not work.*/
	      /* If not, please uncomment it.
	      /***********************************************************************************/
	      //ret = trigger->setLevel(triggerLevel);
         //CHK_RESULT(ret);
      }else if(trgCount > 1 && triggerUsed == 1){
         //for trigger1
         Trigger* trigger1 = bfdDoCtrl->getTrigger1();
         ret = trigger1->setAction(trigger1Action);
         CHK_RESULT(ret);
         Array<SignalDrop>*  srcs = bfdDoCtrl->getFeatures()->getDoTriggerSources();
         ret = trigger1->setSource(SigPFP1);
         CHK_RESULT(ret);
         ret = trigger1->setDelayCount(trigger1DelayCount);
         CHK_RESULT(ret);
         ret = trigger1->setEdge(trigger1Edge);
         CHK_RESULT(ret);
         /***********************************************************************************/
	      /* If the triggerSource is 'SigPFP', 'setLevel' will not work.*/
	      /* If not, please uncomment it.
	      /***********************************************************************************/
         //ret = trigger1->setLevel(trigger1Level);
         //CHK_RESULT(ret);
      }else {
         printf("The device can not support trigger function! \n any key to quit.");
         break;
      }
      
      // Step 6: Prepare the buffered DO. 
      ret = bfdDoCtrl->Prepare();
      CHK_RESULT(ret);

      int32 bufCapacity = bfdDoCtrl->getBufferCapacity();
      int8* userDataBuf = new int8[bufCapacity];
      int8 data         = 0;
      for (int i = 0; i < bufCapacity; i++)
      {
         //fill user data buffer
         userDataBuf[i] = data;
         data = ~data;
      }

      ret = bfdDoCtrl->SetData(bufCapacity, userDataBuf);
      CHK_RESULT(ret);

      delete[] userDataBuf;
      userDataBuf = 0;     

      // Step 6: Start Asynchronous One Buffered DO, 'Asynchronous' means the method returns immediately
      // after the Buffered DO has been started. The StoppedHandler's 'BfdAoEvent' method will be called
      // after the Buffered DO is completed.
      printf("Asynchronous Buffered DO is in progress.\n");
      printf("Please wait... any key to quit !\n");
      ret = bfdDoCtrl->Start();
      CHK_RESULT(ret);

      // Step 7: Do anything you are interesting while the device is outputting data.
      do
      {
         // do something yourself !
         SLEEP(1);
      }while(!kbhit());
    
      // step 8: Stop the operation if it is running.
      ret = bfdDoCtrl->Stop(1);  
      CHK_RESULT(ret);
   }while(false);

	// Step 9: Close device, release any allocated resource.
	bfdDoCtrl->Dispose();

	// If something wrong in this execution, print the error code on screen for tracking.
   if(BioFailed(ret))
   {
      wchar_t enumString[256];
      AdxEnumToString(L"ErrorCode", (int32)ret, 256, enumString);
      printf("Some error occurred. And the last error code is 0x%X. [%ls]\n", ret, enumString);
      waitAnyKey();// Wait any key to quit !
   }
   return 0;
}