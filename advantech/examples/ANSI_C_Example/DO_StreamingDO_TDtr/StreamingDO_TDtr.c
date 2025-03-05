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
*    StreamingDO_TDtr.c
*
* Example Category:
*    DO
*
* Description:
*    This example demonstrates how to use Streaming DO with Trigger Delay to Start function.
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
#include "../../../inc/bdaqctrl.h"
#include "../inc/compatibility.h"
//-----------------------------------------------------------------------------------
// Configure the following parameters before running the demo
//-----------------------------------------------------------------------------------
#define deviceDescription  L"iDAQ-731,BID#0"
const wchar_t* profilePath = L"../../profile/iDAQ-731_0.xml";

int8    portEnabled[] = {1, 0, 0, 0};
int32   convClkRate   = 1000;
int32   sectionCount  = 0;
int32   sectionLength = 1024;

void BDAQCALL OnDataTransmittedEvent (void* sender, BfdDoEventArgs* args, void* userParam);
void BDAQCALL OnTransStoppedEvent    (void* sender, BfdDoEventArgs* args, void* userParam);
void BDAQCALL OnStoppedEvent         (void* sender, BfdDoEventArgs* args, void* userParam);

// Set trigger parameters
TriggerAction triggerAction     = DelayToStart;
ActiveSignal  triggerEdge       = RisingEdge;
int           triggerDelayCount = 600;
double        triggerLevel      = 3.0;

//for trigger1 parameters 
TriggerAction trigger1Action     = DelayToStart;
ActiveSignal  trigger1Edge       = RisingEdge;
int           trigger1DelayCount = 600;
double        trigger1Level      = 3.0;

void waitAnyKey()
{
   do {SLEEP(1);} while (!kbhit());
}

int main(int argc, char* argv[])
{
   ErrorCode     ret          = Success;
   ConvertClock* convClk      = NULL;
   ScanPort*     scanPort     = NULL;
   IArray*       dioPorts     = NULL;
   IArray*       srcs         = NULL;
   int8*         dataBuf      = NULL; 
   Trigger*      trigger      = NULL;
   Trigger*      trigger1     = NULL;
   DioFeatures*  doFeatures   = NULL;
   int32         trgCount     = 0;

   int32 i           = 0;
   int32 bufLen      = 0;
   int32 bufCapacity = 0;
   int32 canSetDir   = 0;
   wchar_t enumString[256] = {0};
   int8 data         = 0;

   // set which trigger be used for this demo, trigger0(0) or trigger1(1).
   int32          triggerUsed = 0; // 0: trigger0, 1: trigger1

   
   // Step 1: Create a 'Buffered DO Control' for Streaming DO function.   
   BufferedDoCtrl* bfdDoCtrl = BufferedDoCtrl_Create();
   
   // Step 2: Set the notification event Handler by which we can known the state of operation effectively.   
   BufferedDoCtrl_addDataTransmittedHandler(bfdDoCtrl, OnDataTransmittedEvent,  NULL);   
   BufferedDoCtrl_addTransitStoppedHandler (bfdDoCtrl, OnTransStoppedEvent,     NULL);
   BufferedDoCtrl_addStoppedHandler        (bfdDoCtrl, OnStoppedEvent,          NULL);

   do 
   {
      // Step 3: Select a device by device number or device description and specify the access mode.
      // in this example we use ModeWrite mode so that we can fully control the device, including configuring, sampling, etc.
      DeviceInformation devInfo;
      devInfo.DeviceNumber = -1;
      devInfo.DeviceMode   = ModeWrite;
      devInfo.ModuleIndex  = 0;
      wcscpy(devInfo.Description, deviceDescription);
      ret = BufferedDoCtrl_setSelectedDevice(bfdDoCtrl, &devInfo);
      CHK_RESULT(ret);
      ret = BufferedDoCtrl_LoadProfile(bfdDoCtrl, profilePath);//Loads a profile to initialize the device.
      CHK_RESULT(ret);
     
      // Step 4: Set necessary parameters.
      bufLen = BufLength(portEnabled);
      doFeatures = BufferedDoCtrl_getFeatures(bfdDoCtrl);
      canSetDir = DioFeatures_getPortProgrammable(doFeatures);
      if (canSetDir)
      {
         dioPorts = BufferedDoCtrl_getPorts(bfdDoCtrl);
         for (i = 0; i < bufLen; ++i) {
            if (portEnabled[i] == 0) { continue; }
            ret = DioPort_setDirectionMask((DioPort*)Array_getItem(dioPorts, i), Output);
            CHK_RESULT(ret);         
         }      
         CHK_RESULT(ret);
      }

      scanPort = BufferedDoCtrl_getScanPort(bfdDoCtrl);
      ret = ScanPort_setPortMap(scanPort, bufLen, portEnabled);
      CHK_RESULT(ret);

      ret = ScanPort_setSectionLength(scanPort, sectionLength);
      CHK_RESULT(ret);
      ret = ScanPort_setSectionCount(scanPort, sectionCount);//The 0 means setting 'streaming' mode.
      CHK_RESULT(ret);

      convClk = BufferedDoCtrl_getConvertClock(bfdDoCtrl);
      ret = ConvertClock_setSource(convClk, SigInternalClock);
      CHK_RESULT(ret);
      ret = ConvertClock_setRate(convClk, convClkRate);
      CHK_RESULT(ret);      
      
      //Step 5: Trigger parameters setting      
      doFeatures = BufferedDoCtrl_getFeatures(bfdDoCtrl);      
      trgCount = DioFeatures_getDoTriggerCount(doFeatures);
      
      //for trigger0
      if (triggerUsed == 0 && trgCount > 0){
         trigger = BufferedDoCtrl_getTrigger(bfdDoCtrl);
         ret = Trigger_setAction(trigger, triggerAction);
         CHK_RESULT(ret);
			/******************************************************************************************/
			/*The different kinds of devices have different trigger source. The details see manual.
			/******************************************************************************************/
         ret = Trigger_setSource(trigger, SigPFP0);         
         CHK_RESULT(ret);
         ret = Trigger_setDelayCount(trigger, triggerDelayCount);   
         CHK_RESULT(ret);
         ret = Trigger_setEdge(trigger, triggerEdge);
         CHK_RESULT(ret);
         /***********************************************************************************/
			/* If the triggerSource is 'SigPFP', 'setLevel' will not work.*/
			/* If not, please uncomment it.
			/***********************************************************************************/
         //ret = Trigger_setLevel(trigger, triggerLevel);
         //CHK_RESULT(ret);
      } else if (triggerUsed == 1 && trgCount > 1){
         //for trigger1
         trigger1 = BufferedDoCtrl_getTrigger1(bfdDoCtrl);
         ret = Trigger_setAction(trigger1, trigger1Action);
         CHK_RESULT(ret);
         ret = Trigger_setSource(trigger1, SigPFP1);         
         CHK_RESULT(ret);
         ret = Trigger_setDelayCount(trigger1, trigger1DelayCount);
         CHK_RESULT(ret);
         ret = Trigger_setEdge(trigger1, trigger1Edge);
         CHK_RESULT(ret);
         /***********************************************************************************/
			/* If the triggerSource is 'SigPFP', 'setLevel' will not work.*/
			/* If not, please uncomment it.
			/***********************************************************************************/
         ret =  Trigger_setLevel(trigger1, trigger1Level);
         CHK_RESULT(ret);
       } else {
         printf("The device can not support trigger function! \n any key to quit.");
         break;
       }

      // Step 5: The operation has been started.
      ret = BufferedDoCtrl_Prepare(bfdDoCtrl);
		CHK_RESULT(ret);
      
      bufCapacity = BufferedDoCtrl_getBufferCapacity(bfdDoCtrl);      
      dataBuf = (int8*)malloc(bufCapacity);
      if (NULL == dataBuf)
      {
         printf("Insufficient memory available\n");
         break;
      }
      
      //fill user data buffer
      for (i = 0; i < bufCapacity; i++)
      {         
         dataBuf[i] = data;            
         data = ~data;
      }
      ret = BufferedDoCtrl_SetData(bfdDoCtrl, bufCapacity, dataBuf);            
      CHK_RESULT(ret);
      
      free(dataBuf);

      ret = BufferedDoCtrl_Start(bfdDoCtrl);
      CHK_RESULT(ret);

      // Step 6: The device is acquiring data.
      printf("Streaming DO is in progress.\nplease wait...  any key to quit!\n\n");
      do
      {
         SLEEP(1);
      }	while (!kbhit());
      
      // step 7: Stop the operation if it is running.
      ret = BufferedDoCtrl_Stop(bfdDoCtrl, 1);
      CHK_RESULT(ret);
   } while (FALSE);

   // Step 9: Close device, release any allocated resource.
   BufferedDoCtrl_Dispose(bfdDoCtrl);

   // If something wrong in this execution, print the error code on screen for tracking.
   if (BioFailed(ret))
   {
      AdxEnumToString(L"ErrorCode", (int32)ret, 256, enumString);
      printf("Some error occurred. And the last error code is 0x%X. [%ls]\n", ret, enumString);
      waitAnyKey();// wait any key to quit!
   }
   return 0;
}

// This function is used to deal with 'Stopped' Event.
void BDAQCALL OnStoppedEvent(void * sender, BfdDoEventArgs * args, void *userParam)
{
   printf("\nBuffered DO stopped: offset = %d, count = %d\n", args->Offset, args->Count);
}
// This function is used to deal with 'DataTransmitted' Event.
void BDAQCALL OnDataTransmittedEvent(void * sender, BfdDoEventArgs * args, void *userParam)
{
   printf("\nBuffered DO data transmitted: offset = %d, count = %d\n", args->Offset, args->Count);
}
// This function is used to deal with 'TransStopped' Event.
void BDAQCALL OnTransStoppedEvent(void * sender, BfdDoEventArgs * args, void *userParam)
{
   printf("\nBuffered DO Transmit Stopped: offset = %d, count = %d\n", args->Offset, args->Count);
}
