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
*    StreamingDI.c
*
* Example Category:
*    DI
*
* Description:
*    This example demonstrates how to use Streaming DI function.
*
* Instructions for Running:
*    1. Set the 'deviceDescription' which can get from system device manager for opening the device.
*	  2. Set the 'profilePath' to save the profile path of being initialized device.
*    3. Set the 'portEnabled' to decide which port to be used.
*    4. Set the 'sectionLength' as the length of data section for Buffered DI.
*	  5. Set the 'sectionCount' as the count of data section for Buffered DI.
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
#define EnabledCount   1 // The value depend on portEnabled

int32   convClkRate   = 1000;
int32   sectionCount  = 0;
#define SectionLength  1024

// user buffer size should be equal or greater than raw data buffer length, because data ready count
// is equal or more than smallest section of raw data buffer and up to raw data buffer length.
// users can set 'USER_BUFFER_LENGTH' according to demand.
#define  USER_BUFFER_LENGTH  SectionLength * EnabledCount
int8     dataBuf[USER_BUFFER_LENGTH]; 

void BDAQCALL OnDataReadyEvent         (void* sender, BfdDiEventArgs* args, void* userParam);
void BDAQCALL OnOverRunEvent           (void* sender, BfdDiEventArgs* args, void* userParam);
void BDAQCALL OnCacheOverflowEvent     (void* sender, BfdDiEventArgs* args, void* userParam);
void BDAQCALL OnStoppedEvent           (void* sender, BfdDiEventArgs* args, void* userParam);

void waitAnyKey()
{
   do {SLEEP(1);} while (!kbhit());
}

int main(int argc, char* argv[])
{
   ErrorCode     ret        = Success;
   ConvertClock* convClk    = NULL;
   ScanPort*     scanPort   = NULL;
   IArray*       dioPorts   = NULL;
   DioFeatures*  diFeatures = NULL;

   int32 i      = 0;
   int32 bufLen = 0;
   int32 canSetDir = 0;
   wchar_t enumString[256] = {0};

   // Step 1: Create a 'Buffered DI Control' for Streaming DI function.
   BufferedDiCtrl* bfdDiCtrl = BufferedDiCtrl_Create();
   
   // Step 2: Set the notification event Handler by which we can known the state of operation effectively.
   BufferedDiCtrl_addDataReadyHandler     (bfdDiCtrl, OnDataReadyEvent,         NULL);
   BufferedDiCtrl_addOverrunHandler       (bfdDiCtrl, OnOverRunEvent,           NULL);
   BufferedDiCtrl_addCacheOverflowHandler (bfdDiCtrl, OnCacheOverflowEvent,     NULL);
   BufferedDiCtrl_addStoppedHandler       (bfdDiCtrl, OnStoppedEvent,           NULL);

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
      ret = ScanPort_setSectionCount(scanPort, sectionCount);//The 0 means setting 'streaming' mode.
      CHK_RESULT(ret);

      convClk = BufferedDiCtrl_getConvertClock(bfdDiCtrl);
      ret = ConvertClock_setSource(convClk, SigInternalClock);
      CHK_RESULT(ret);
      ret = ConvertClock_setRate(convClk, convClkRate);      
      CHK_RESULT(ret);

      // Step 5: The operation has been started.
      ret = BufferedDiCtrl_Prepare(bfdDiCtrl);
		CHK_RESULT(ret);
      ret = BufferedDiCtrl_Start(bfdDiCtrl);
      CHK_RESULT(ret);

      // Step 6: The device is acquiring data.
      printf("Streaming DI is in progress.\nplease wait...  any key to quit!\n\n");
      do
      {
         SLEEP(1);
      }	while (!kbhit());
      
      // step 7: Stop the operation if it is running.
      ret = BufferedDiCtrl_Stop(bfdDiCtrl);
      CHK_RESULT(ret);
   } while (FALSE);

   // Step 9: Close device, release any allocated resource.
   BufferedDiCtrl_Dispose(bfdDiCtrl);

   // If something wrong in this execution, print the error code on screen for tracking.
   if (BioFailed(ret))
   {
      AdxEnumToString(L"ErrorCode", (int32)ret, 256, enumString);
      printf("Some error occurred. And the last error code is 0x%X. [%ls]\n", ret, enumString);
      waitAnyKey();// wait any key to quit!
   }
   return 0;
}
//The function is used to deal with 'DataReady' Event.
void BDAQCALL OnDataReadyEvent (void* sender, BfdDiEventArgs* args, void* userParam)
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
//The function is used to deal with 'OverRun' Event.
void BDAQCALL OnOverRunEvent (void* sender, BfdDiEventArgs* args, void* userParam)
{
   printf(" Buffered DI Overrun: offset = %d, count = %d\n", args->Offset, args->Count);
}
//The function is used to deal with 'CacheOverflow' Event.
void BDAQCALL OnCacheOverflowEvent (void* sender, BfdDiEventArgs* args, void* userParam)
{
   printf(" Buffered DI Cache Overflow: offset = %d, count = %d\n", args->Offset, args->Count);
}
//The function is used to deal with 'Stopped' Event.
void BDAQCALL OnStoppedEvent (void* sender, BfdDiEventArgs* args, void* userParam)
{
   int32 i = 0, getDataCount = 0, returnedCount = 0, returnedSumCount = 0;
   int32 remainingCount = args->Count;
   BufferedDiCtrl* bfdDiCtrl = (BufferedDiCtrl*)sender;

   do 
   {
      getDataCount = MinValue(USER_BUFFER_LENGTH, remainingCount);
      BufferedDiCtrl_GetData(bfdDiCtrl, getDataCount, dataBuf, 0, &returnedCount, NULL, NULL, NULL);
      remainingCount -= returnedCount;
      returnedSumCount += returnedCount;
   } while (remainingCount > 0);
   printf("Buffered DI Stopped Event get data count is  %d，argsCount is %d\n", returnedSumCount, args->Count);
}
