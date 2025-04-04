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
*    This example demonstrates how to use Streaming AI with saving function.
*
* Instructions for Running:
*    1. Set the 'deviceDescription' for opening the device. 
*	  2. Set the 'profilePath' to save the profile path of being initialized device. 
*    3. Set the 'startChannel' as the first channel for scan analog samples  
*    4. Set the 'channelCount' to decide how many sequential channels to scan analog samples.
*    5. Set the 'sectionLength' as the length of data section for Buffered AI.
*	  6. Set the 'sectionCount' as the count of data section for Buffered AI.

* I/O Connections Overview:
*    Please refer to your hardware reference manual.
*
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include "../inc/compatibility.h"
#include "../../../inc/bdaqctrl.h"

#include <Windows.h>
#include "stdafx.h"
#include <tchar.h>

using namespace Automation::BDaq;
using namespace std;
//-----------------------------------------------------------------------------------
// Configure the following parameters before running the demo
//-----------------------------------------------------------------------------------
#define       deviceDescription  L"DemoDevice,BID#0"
const wchar_t* profilePath = L"../../profile/DemoDevice.xml";
int32         startChannel = 0;
const int32   channelCount = 1;
const int32   sectionLength = 1024;
const int32   sectionCount = 0;

DWORD    SingleSavingFileSize = 1024  * sizeof(double);
DWORD    RequirementFileSize  = 10240 * sizeof(double);

HANDLE   hFile;
double * buffer = NULL;
DWORD    WrittenBytes;
DWORD    RealFileSize;

// user buffer size should be equal or greater than raw data buffer length, because data ready count
// is equal or more than smallest section of raw data buffer and up to raw data buffer length.
// users can set 'USER_BUFFER_SIZE' according to demand.
#define       USER_BUFFER_SIZE   channelCount*sectionLength
double        Data[USER_BUFFER_SIZE]; 

inline void waitAnyKey()
{
   do{SLEEP(1);} while(!kbhit());
} 

inline void openFile()
{
      hFile = CreateFile(
      _T("..\\C++_Data.bin"),
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
      printf("Allocate buffer fail(error %d)\n",GetLastError());
   }
}

// This function is used to deal with 'DataReady' Event. 
// Notice:
// 1.It is not recommended to process or print too much information in this function,
// which can easily cause event blockage.
// 2.In theory, the amount of data argsCount returned by a DataReady should be equal to sectionLen * channelCount.
// However, in reality, when the dataReady callback function is executed is also affected by system load,
// so it cannot be guaranteed that the amount of data argsCount in each DataReady is equal to sectionLen * channelCount.
void BDAQCALL OnDataReadyEvent(void * sender, BfdAiEventArgs * args, void *userParam)
{
   static int i = 1;
   int32 returnedCount = 0;
   int32 returnedSumCount = 0;
   int32 remainingCount = args->Count;
   WaveformAiCtrl * waveformAiCtrl = (WaveformAiCtrl *)sender;

   printf("Executed %d time.\n\n", i++);
   do 
   {
      int32 getDataCount = MinValue(USER_BUFFER_SIZE, remainingCount);
      waveformAiCtrl->GetData(getDataCount, Data, 0, &returnedCount);
      remainingCount -= returnedCount;
      returnedSumCount += returnedCount;

      printf("Ready to save the data...\n\n");
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
void BDAQCALL OnOverRunEvent(void * sender, BfdAiEventArgs * args, void *userParam)
{
   printf("Streaming AI Overrun: offset = %d, count = %d\n", args->Offset, args->Count);
}
//The function is used to deal with 'CacheOverflow' Event.
void BDAQCALL OnCacheOverflowEvent(void * sender, BfdAiEventArgs * args, void *userParam)
{
   printf(" Streaming AI Cache Overflow: offset = %d, count = %d\n", args->Offset, args->Count);
}
//The function is used to deal with 'Stopped' Event.
void BDAQCALL OnStoppedEvent(void * sender, BfdAiEventArgs * args, void *userParam)
{
   printf("Streaming AI stopped: offset = %d, count = %d\n", args->Offset, args->Count);
}

int main(int argc, char* argv[])
{
   ErrorCode        ret = Success;

   // Step 1: Create a 'WaveformAiCtrl' for buffered AI function.
   WaveformAiCtrl * wfAiCtrl = WaveformAiCtrl::Create();

   //Step 2: Open file
   openFile();

   // Step 3: Set the notification event Handler by which we can known the state of operation effectively.
   wfAiCtrl->addDataReadyHandler(OnDataReadyEvent, NULL);
   wfAiCtrl->addOverrunHandler(OnOverRunEvent, NULL);
   wfAiCtrl->addCacheOverflowHandler(OnCacheOverflowEvent, NULL);
   wfAiCtrl->addStoppedHandler(OnStoppedEvent, NULL);
   do
   {
      // Step 4: Select a device by device number or device description and specify the access mode.
      // in this example we use ModeWrite mode so that we can fully control the device, including configuring, sampling, etc.
      DeviceInformation devInfo(deviceDescription);
      ret = wfAiCtrl->setSelectedDevice(devInfo);
      CHK_RESULT(ret);
      ret = wfAiCtrl->LoadProfile(profilePath);//Loads a profile to initialize the device.
      CHK_RESULT(ret);

      // Step 5: Set necessary parameters.
      Conversion * conversion = wfAiCtrl->getConversion();
      ret = conversion->setChannelStart(startChannel);
      CHK_RESULT(ret);
      ret = conversion->setChannelCount(channelCount);
      CHK_RESULT(ret);
		Record * record = wfAiCtrl->getRecord();
      ret = record->setSectionCount(sectionCount);//The 0 means setting 'streaming' mode.
      CHK_RESULT(ret);
      ret = record->setSectionLength(sectionLength);
      CHK_RESULT(ret);

      // Step 6: The operation has been started.
      // We can get samples via event handlers.
		ret = wfAiCtrl->Prepare();
		CHK_RESULT(ret);
		ret = wfAiCtrl->Start();
      CHK_RESULT(ret);

      // Step 7: The device is acquiring data.
      printf("Streaming AI is in progress.\nplease wait...  any key to quit!\n\n");
      do
      {
         SLEEP(1);
      }	while((RealFileSize < RequirementFileSize) ? true : false);
      printf("Saving completely!\n");

      // step 8: Stop the operation if it is running.
      ret = wfAiCtrl->Stop(); 
      CHK_RESULT(ret);
   }while(false);

   // Step 9: Close device, release any allocated resource.
   wfAiCtrl->Dispose();
   VirtualFree(buffer, SingleSavingFileSize, MEM_RELEASE);
   CloseHandle(hFile);

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