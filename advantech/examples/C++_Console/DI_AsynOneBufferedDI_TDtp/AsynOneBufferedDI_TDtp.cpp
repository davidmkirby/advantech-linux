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
*    AsynOneBufferedDI_TDtp.cpp
*
* Example Category:
*    DI
*
* Description:
*    This example demonstrates how to use Asynchronous One Buffered DI with Trigger 
*    Delay to Stop function.
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
const int32  sectionCount  = 1;

// user buffer size should be equal or greater than raw data buffer length, because data ready count
// is equal or more than smallest section of raw data buffer and up to raw data buffer length.
// users can set 'USER_BUFFER_SIZE' according to demand.
#define  USER_BUFFER_LENGTH  sectionLength * enabledCount * sectionCount
int8     dataBuf[USER_BUFFER_LENGTH]; 

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
int triggerUsed = 0;// 0: trigger0, 1: trigger1

inline void waitAnyKey()
{
   do{SLEEP(1);} while(!kbhit());
} 

// This function is used to deal with 'StoppedEvent'.
void BDAQCALL OnStoppedEvent(void * sender, BfdDiEventArgs * args, void *userParam)
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

int main(int argc, char* argv[])
{
   ErrorCode ret = Success;

   // Step 1: Create a 'BufferedDiCtrl' for buffered DI function.
   BufferedDiCtrl* bfdDiCtrl = BufferedDiCtrl::Create();

	// Step 2: Set the notification event Handler by which we can known the state of operation effectively.
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
      ret = scanPort->setSectionCount(sectionCount);//The sectionCount is nonzero value, which means 'One Buffered' mode.
      CHK_RESULT(ret);

      ConvertClock* convClk = bfdDiCtrl->getConvertClock();
      ret = convClk->setSource(SigInternalClock);
      CHK_RESULT(ret);
      ret = convClk->setRate(convClkRate);
      CHK_RESULT(ret);
      
      //Step 5: Trigger parameters setting
      int32 trgCount = bfdDiCtrl->getFeatures()->getDiTriggerCount();
      //for trigger0
      if (trgCount > 0 && triggerUsed == 0){
         Trigger* trigger = bfdDiCtrl->getTrigger();
	      ret = trigger->setAction(triggerAction);
	      CHK_RESULT(ret);
	      /*************************************************************************************************/
	      /*The different kinds of devices have different trigger source. The details see manual.
	      /*In this example, we use the iDAQ-731 and set 'SigPFP0' as the default trigger0 source.
	      /**************************************************************************************************/
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
      }else if(trgCount > 1 && triggerUsed == 1){
         //for trigger1
         Trigger* trigger1 = bfdDiCtrl->getTrigger1();
         ret = trigger1->setAction(trigger1Action);
         CHK_RESULT(ret);
         /******************************************************************************************/
         /*Set 'SigPFP1' as the default trigger1 source. The details see manual.
         /******************************************************************************************/
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
      }else {
         printf("The device can not support trigger function! \n any key to quit.");
         break;
      }     

      // Step 6: start Asynchronous Buffered DI, 'Asynchronous' means the method returns immediately
      // after the acquisition has been started. The StoppedHandler's 'StoppedEvent' method will be called
      // after the acquisition is completed.
      printf("Asynchronous finite acquisition is in progress.\n");
      ret = bfdDiCtrl->Prepare();
      CHK_RESULT(ret);
      ret = bfdDiCtrl->Start();
      CHK_RESULT(ret);

      // Step 7: The device is acquiring data.
      do
      {
         SLEEP(1);
      }while(!kbhit());

      // step 8: stop the operation if it is running.
      ret = bfdDiCtrl->Stop();  
      CHK_RESULT(ret);
   }
   while(false);
   
   // Step 9: close device, release any allocated resource before quit.
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