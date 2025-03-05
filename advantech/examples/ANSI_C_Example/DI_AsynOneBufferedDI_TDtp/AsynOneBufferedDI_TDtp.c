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
*    AsynOneBufferedDI_TDtp.c
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
#include <stdio.h>
#include <stdlib.h>
#include "../../../inc/bdaqctrl.h"
#include "../inc/compatibility.h"
//-----------------------------------------------------------------------------------
// Configure the following parameters before running the demo
//-----------------------------------------------------------------------------------
#define deviceDescription  L"iDAQ-731,BID#0"
const wchar_t* profilePath = L"../../profile/iDAQ-731_0.xml";

int8    portEnabled[] = {1, 0, 0, 0};
int32   convClkRate   = 1000;

#define EnabledCount   1 // The value depend on portEnabled
#define SectionLength  1024
#define SectionCount   1

// user buffer size should be equal or greater than raw data buffer length, because data ready count
// is equal or more than smallest section of raw data buffer and up to raw data buffer length.
// users can set 'USER_BUFFER_LENGTH' according to demand.
#define  USER_BUFFER_LENGTH  SectionLength * EnabledCount * SectionCount
int8     dataBuf[USER_BUFFER_LENGTH]; 

// Set trigger parameters
TriggerAction triggerAction     = DelayToStop;
ActiveSignal  triggerEdge       = RisingEdge;
int           triggerDelayCount = 1000;
double        triggerLevel      = 3.0;

//for trigger1 parameters 
TriggerAction trigger1Action     = DelayToStop;
ActiveSignal  trigger1Edge       = RisingEdge;
int           trigger1DelayCount = 1000;
double        trigger1Level      = 3.0;

// set which trigger be used for this demo, trigger0(0) or trigger1(1).
int           triggerUsed = 0; // 0: trigger0, 1: trigger1

void BDAQCALL OnStoppedEvent(void * sender, BfdDiEventArgs * args, void *userParam);

void waitAnyKey()
{
   do {SLEEP(1);} while (!kbhit());
}

int main(int argc, char* argv[])
{
   ErrorCode      ret          = Success;
   ConvertClock*  convClk      = NULL;
   ScanPort*      scanPort     = NULL;   
   IArray*        dioPorts     = NULL;
   IArray*        srcs         = NULL;
   DioFeatures*   diFeatures   = NULL;
   Trigger*       trigger      = NULL;
   Trigger*       trigger1     = NULL;
   int32          trgCount     = 0;   
   
   int32 i         = 0;
   int32 bufLen    = 0;
   int32 canSetDir = 0;
   wchar_t enumString[256] = {0};

   // Step 1: Create a 'Buffered DI Control' for Buffered DI function.
   BufferedDiCtrl* bfdDiCtrl = BufferedDiCtrl_Create();

   // Step 2: Set the notification event Handler by which we can known the state of operation effectively.
   BufferedDiCtrl_addStoppedHandler(bfdDiCtrl, OnStoppedEvent, NULL);
   
   do 
   {
      // Step 3: Select a device by device number or device description and specify the access mode.
      // in this example we use ModeWrite mode so that we can fully control the device, including configuring, sampling, etc.
      DeviceInformation devInfo;
      devInfo.DeviceNumber = -1;
      devInfo.DeviceMode   = ModeWrite;
      devInfo.ModuleIndex  = 0;
      wcscpy(devInfo.Description, deviceDescription);
      ret = BufferedDiCtrl_setSelectedDevice(bfdDiCtrl, &devInfo);      
      CHK_RESULT(ret);
      ret = BufferedDiCtrl_LoadProfile(bfdDiCtrl, profilePath);//Loads a profile to initialize the device.
      CHK_RESULT(ret);

      // Step 4: Set necessary parameters.
      bufLen = BufLength(portEnabled);
      diFeatures = BufferedDiCtrl_getFeatures(bfdDiCtrl);
      canSetDir = DioFeatures_getPortProgrammable(diFeatures);
      if (canSetDir)
      {
         dioPorts = BufferedDiCtrl_getPorts(bfdDiCtrl);
         for (i = 0; i < bufLen; ++i) {
            if (portEnabled[i] == 0) { continue; }
            ret = DioPort_setDirectionMask((DioPort*)Array_getItem(dioPorts, i), Input);
            CHK_RESULT(ret);         
         }      
         CHK_RESULT(ret);
      }

      scanPort = BufferedDiCtrl_getScanPort(bfdDiCtrl);
      ret = ScanPort_setPortMap(scanPort, bufLen, portEnabled);
      CHK_RESULT(ret);

      ret = ScanPort_setSectionLength(scanPort, SectionLength);
      CHK_RESULT(ret);
      ret = ScanPort_setSectionCount(scanPort, SectionCount); //The sectionCount is nonzero value, which means 'One Buffered' mode.
      CHK_RESULT(ret);      

      convClk = BufferedDiCtrl_getConvertClock(bfdDiCtrl);
      ret = ConvertClock_setSource(convClk, SigInternalClock);
      CHK_RESULT(ret);
      ret = ConvertClock_setRate(convClk, convClkRate);
      CHK_RESULT(ret);      
      
		//Step 5: Trigger parameters setting
      diFeatures = BufferedDiCtrl_getFeatures(bfdDiCtrl);
      trgCount = DioFeatures_getDiTriggerCount(diFeatures);
		//for trigger0     
      if (triggerUsed == 0 && trgCount > 0){
         trigger = BufferedDiCtrl_getTrigger(bfdDiCtrl);
         ret = Trigger_setAction(trigger, triggerAction);
         CHK_RESULT(ret);
		   /******************************************************************************************/
		   /*The different kinds of devices have different trigger source. The details see manual.
		   /******************************************************************************************/                  
         ret = Trigger_setSource(trigger, SigExtDigTrigger0);
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
         trigger1 = BufferedDiCtrl_getTrigger1(bfdDiCtrl);
         ret = Trigger_setAction(trigger1, trigger1Action);
         CHK_RESULT(ret);                  
         ret = Trigger_setSource(trigger1, SigExtDigTrigger1);
         CHK_RESULT(ret);
         ret = Trigger_setDelayCount(trigger1, trigger1DelayCount);
         CHK_RESULT(ret);
         ret = Trigger_setEdge(trigger1, trigger1Edge);
         CHK_RESULT(ret);
         /***********************************************************************************/
		   /* If the triggerSource is 'SigExtDigTrigger', 'setLevel' will not work.*/
		   /* If not, please uncomment it.
		   /***********************************************************************************/
         //ret =  Trigger_setLevel(trigger1, trigger1Level);
         //CHK_RESULT(ret);
      } else {
         printf("The device can not support trigger function! \n any key to quit.");
         break;
      }       

      // Step 6: start Asynchronous Buffered DI, 'Asynchronous' means the method returns immediately
      // after the acquisition has been started. The StoppedHandler's 'StoppedEvent' method will be called
      // after the acquisition is completed.
      printf("Asynchronous finite acquisition is in progress.\n");      
      ret = BufferedDiCtrl_Prepare(bfdDiCtrl);
      CHK_RESULT(ret);
      ret = BufferedDiCtrl_Start(bfdDiCtrl);
      CHK_RESULT(ret);

      // Step 7: The device is acquiring data.
      do
      {
         SLEEP(1);
      }while(!kbhit());

      // step 8: stop the operation if it is running.
      ret = BufferedDiCtrl_Stop(bfdDiCtrl);
      CHK_RESULT(ret);
   } while (FALSE);

   // Step 9: close device, release any allocated resource before quit.
   BufferedDiCtrl_Dispose(bfdDiCtrl);

   // If something wrong in this execution, print the error code on screen for tracking.
   if(BioFailed(ret))
   {
      AdxEnumToString(L"ErrorCode", (int32)ret, 256, enumString);
      printf("Some error occurred. And the last error code is 0x%X. [%ls]\n", ret, enumString);
      waitAnyKey();// wait any key to quit!
   }
   return 0;
}

//The function is used to deal with 'Stoppped Event'
void BDAQCALL OnStoppedEvent(void * sender, BfdAiEventArgs * args, void *userParam)
{
   int   data = 0;
   int32 i = 0, getDataCount = 0, returnedCount = 0;
   int32 remainingCount = args->Count;
   BufferedDiCtrl* bfdDiCtrl = (BufferedDiCtrl*)sender;

   do
   {
      getDataCount = MinValue(USER_BUFFER_LENGTH, remainingCount);
      printf("\nGetDataCount[ %d ]\n", getDataCount);
      BufferedDiCtrl_GetData(bfdDiCtrl, getDataCount, dataBuf, 0, &returnedCount, NULL, NULL, NULL);      
      remainingCount -= returnedCount;
   } while (remainingCount > 0);

   // Show each channel's new data
   printf("the port data:\n");
   for (i = 0; i < BufLength(portEnabled); ++i) {
      if (portEnabled[i] == 0) { continue; }
      data = dataBuf[i];
      printf("Port %d: 0x%x \n", i, data);      
   }
}