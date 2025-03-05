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
*    StreamingAIWithSaving.cpp
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to use Streaming AI function with saving.
*
* Instructions for Running:
*    1. Set the 'deviceDescription' for opening the device. 
*	  2. Set the 'profilePath' to save the profile path of being initialized device. 
*    3. Set the 'startChannel' as the first channel for scan analog samples  
*    4. Set the 'channelCount' to decide how many sequential channels to scan analog samples.
*    5. Set the 'sectionLength' as the length of data section for Buffered AI.
*	  6. Set the 'sectionCount' as the count of data section for Buffered AI.
*
* I/O Connections Overview:
*    Please refer to your hardware reference manual.
*
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "../../../inc/bdaqctrl.h"
#include "../inc/compatibility.h"
#include <Windows.h>
#include "stdafx.h"
#include <tchar.h>
//-----------------------------------------------------------------------------------
// Configure the following parameters before running the demo
//-----------------------------------------------------------------------------------
#define deviceDescription L"DemoDevice,BID#0"
const wchar_t* profilePath = L"../../profile/DemoDevice.xml";
#define startChannel  0
#define channelCount  1
#define sectionLength 1024
#define sectionCount  0
#define SingleSavingFileSize  1024  * sizeof(double)
#define RequirementFileSize   10240 * sizeof(double)
// user buffer size should be equal or greater than raw data buffer length, because data ready count
// is equal or more than smallest section of raw data buffer and up to raw data buffer length.
// users can set 'USER_BUFFER_SIZE' according to demand.
#define USER_BUFFER_SIZE  channelCount*sectionLength
double  Data[USER_BUFFER_SIZE];

void BDAQCALL OnDataReadyEvent         (void * sender, BfdAiEventArgs * args, void *userParam);
void BDAQCALL OnOverRunEvent           (void * sender, BfdAiEventArgs * args, void *userParam);
void BDAQCALL OnCacheOverflowEvent     (void * sender, BfdAiEventArgs * args, void *userParam);
void BDAQCALL OnStoppedEvent           (void * sender, BfdAiEventArgs * args, void *userParam);

HANDLE   hFile;
double * buffer = NULL;
DWORD    WrittenBytes;
DWORD    RealFileSize;

void waitAnyKey()
{
   do {SLEEP(1);} while (!kbhit());
}

void openFile()
{
      hFile = CreateFile(
      _T("..\\C_Data.bin"),
      GENERIC_WRITE | GENERIC_READ, 
      0, 
      NULL,
      CREATE_ALWAYS,
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN,
      NULL);

   if (hFile == INVALID_HANDLE_VALUE)
   {
      printf("Cannot open file (error %d)\n", GetLastError());
   }

   buffer = (double*)VirtualAlloc(0, SingleSavingFileSize, MEM_COMMIT, PAGE_READWRITE);
   if (!buffer)
   {
      printf("Allocated buffer fail(error %d)\n",GetLastError());
   }
}

int main(int argc, char* argv[])
{
   ErrorCode ret = Success;
   Conversion * conversion = NULL;
   Record * record = NULL; 
   wchar_t enumString[256];

   // Step 1: Create a 'WaveformAiCtrl' for Waveform AI function.
   WaveformAiCtrl * wfAiCtrl = WaveformAiCtrl_Create();
   
   //Step 2: open file to save data.
   openFile();

   // Step 3: Set the notification event Handler by which we can known the state of operation effectively.
   WaveformAiCtrl_addDataReadyHandler     (wfAiCtrl, OnDataReadyEvent,         NULL);
   WaveformAiCtrl_addOverrunHandler       (wfAiCtrl, OnOverRunEvent,           NULL);
   WaveformAiCtrl_addCacheOverflowHandler (wfAiCtrl, OnCacheOverflowEvent,     NULL);
   WaveformAiCtrl_addStoppedHandler       (wfAiCtrl, OnStoppedEvent,           NULL);

   do 
   {
      // Step 4: Select a device by device number or device description and specify the access mode.
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

      // Step 5: Set necessary parameters.
      conversion = WaveformAiCtrl_getConversion(wfAiCtrl);
      ret = Conversion_setChannelStart(conversion, startChannel);
      CHK_RESULT(ret);
      ret= Conversion_setChannelCount(conversion, channelCount);
      CHK_RESULT(ret);
		record = WaveformAiCtrl_getRecord(wfAiCtrl);
      ret = Record_setSectionCount(record, sectionCount);//The 0 means setting 'streaming' mode.
      CHK_RESULT(ret);
      ret = Record_setSectionLength(record, sectionLength);
      CHK_RESULT(ret);

		// Step 6: The operation has been started.
		// We can get samples via event handlers.
      ret = WaveformAiCtrl_Prepare(wfAiCtrl);
		CHK_RESULT(ret);
		ret = WaveformAiCtrl_Start(wfAiCtrl);
      CHK_RESULT(ret);

      // Step 7: The device is acquiring data.
      printf("Streaming AI is in progress.\n\nplease wait...  any key to quit!\n\n");
      do
      {
         SLEEP(1);
      }	while((RealFileSize < RequirementFileSize) ? TRUE : FALSE);
      printf("Saving completely!\n\n");

      // step 8: Stop the operation if it is running.
      ret = WaveformAiCtrl_Stop(wfAiCtrl);
      CHK_RESULT(ret);
   } while (FALSE);

   // Step 9: Close device, release any allocated resource.
   WaveformAiCtrl_Dispose(wfAiCtrl);
   VirtualFree(buffer, SingleSavingFileSize, MEM_RELEASE);
   CloseHandle(hFile);
 
   // If something wrong in this execution, print the error code on screen for tracking.
   if (BioFailed(ret))
   {
      AdxEnumToString(L"ErrorCode", (int32)ret, 256, enumString);
      printf("Some error occurred. And the last error code is 0x%X. [%ls]\n", ret, enumString);
      waitAnyKey();// wait any key to quit!
   }
   return 0;
}

// This function is used to deal with 'DataReady' Event. 
// Notice:
// 1.It is not recommended to process or print too much information in this function,
// which can easily cause event blockage.
// 2.In theory, the amount of data argsCount returned by a DataReady should be equal to sectionLen * channelCount.
// However, in reality, when the dataReady callback function is executed is also affected by system load,
// so it cannot be guaranteed that the amount of data argsCount in each DataReady is equal to sectionLen * channelCount.
void BDAQCALL OnDataReadyEvent (void * sender, BfdAiEventArgs * args, void *userParam)
{
   static int i = 1;
   int32 getDataCount = 0, returnedCount = 0, returnedSumCount = 0;
   int32 remainingCount = args->Count;
   WaveformAiCtrl * waveformAiCtrl = (WaveformAiCtrl *)sender;

   printf("Executed %d time.\n\n", i++);
   do 
   {
      getDataCount = MinValue(USER_BUFFER_SIZE, remainingCount);
      WaveformAiCtrl_GetDataF64(waveformAiCtrl, getDataCount, Data, 0, &returnedCount, NULL, NULL, NULL);
      remainingCount -= returnedCount;
      returnedSumCount += returnedCount;

      printf("Ready to save the data...\n");
      memcpy(buffer, Data, returnedCount * sizeof(double));
      if (WriteFile(hFile, buffer, returnedCount * sizeof(double), &WrittenBytes, NULL)){
         printf("Saving has been executed!\n\n");
         RealFileSize += WrittenBytes; 
         printf("The real-time size of file is %d byte\n\n", RealFileSize);         
      }else{
         printf("error!\n\n");
      }
   } while (remainingCount > 0);
}
//The function is used to deal with 'OverRun' Event.
void BDAQCALL OnOverRunEvent (void * sender, BfdAiEventArgs * args, void *userParam)
{
   printf("Streaming AI Overrun: offset = %d, count = %d\n", args->Offset, args->Count);
}
//The function is used to deal with 'CacheOverflow' Event.
void BDAQCALL OnCacheOverflowEvent (void * sender, BfdAiEventArgs * args, void *userParam)
{
   printf(" Streaming AI Cache Overflow: offset = %d, count = %d\n", args->Offset, args->Count);
}
//The function is used to deal with 'Stopped' Event.
void BDAQCALL OnStoppedEvent (void * sender, BfdAiEventArgs * args, void *userParam)
{
   printf("Streaming AI stopped: offset = %d, count = %d\n", args->Offset, args->Count);
}
