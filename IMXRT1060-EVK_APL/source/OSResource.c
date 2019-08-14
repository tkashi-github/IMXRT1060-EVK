/**
 * @file OSResource.c
 * @brief TODO
 * @author Takashi Kashiwagi
 * @date 2018/10/28
 * @version     0.1
 * @details 
 * --
 * License Type <MIT License>
 * --
 * Copyright 2018 Takashi Kashiwagi
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 * @par Update:
 * - 2018/10/28: Takashi Kashiwagi: v0.1 for IMXRT1060-EVK
 */
#include "OSResource.h"

/** Standard Header */
#include <stdint.h>
#include <stdbool.h>

/* Other Tasks */
#include "Task/InitialTask.h"
#include "Task/ConsoleTask/ConsoleTask.h"
#include "Task/StorageTask/StorageTask.h"
#include "Task/LanTask/LanTask.h"
#include "SensorTask/SensorTask.h"
#include "CameraTask/CameraTask.h"
#include "LcdTask/LcdTask.h"
#include "TouchScreenTask/TouchScreenTask.h"
#include "TempMoniTask/TempMoniTask.h"
#include "Task/SoundTask/SoundTask.h"
#include "Task/PlayCtrl/PlayCtrl.h"

/** typedef Task Table */
typedef struct{
	osThreadId_t   *pOsId;
	osThreadFunc_t TaskFunc;
	void           *argment;
	osThreadAttr_t Attr;
}stOSdefTable_t;


/** Task Handle */
OS_RESOURCE_MACRO_TASK_DEFINE(InitialTask, 8192);
OS_RESOURCE_MACRO_TASK_DEFINE(ConsoleTask, 8192);
OS_RESOURCE_MACRO_TASK_DEFINE(StorageTask, 8192);
OS_RESOURCE_MACRO_TASK_DEFINE(LanTask, 8192);
OS_RESOURCE_MACRO_TASK_DEFINE(LcdTask, 8192);
OS_RESOURCE_MACRO_TASK_DEFINE(TouchScreenTask, 8192);
OS_RESOURCE_MACRO_TASK_DEFINE(TempMoniTask, 8192);
OS_RESOURCE_MACRO_TASK_DEFINE(SoundTask, 8192);
OS_RESOURCE_MACRO_TASK_DEFINE(PlayCtrl, 16384);

/** Task Table */
static const stOSdefTable_t s_stTaskTable[] = {
	OS_RESOURCE_MACRO_TASK_TABLE(InitialTask, NULL, osPriorityLow),
	OS_RESOURCE_MACRO_TASK_TABLE(ConsoleTask, NULL, osPriorityBelowNormal),
	OS_RESOURCE_MACRO_TASK_TABLE(StorageTask, enUSDHC1, osPriorityNormal),
	OS_RESOURCE_MACRO_TASK_TABLE(LanTask, NULL, osPriorityBelowNormal),
	OS_RESOURCE_MACRO_TASK_TABLE(LcdTask, NULL, osPriorityAboveNormal),
	OS_RESOURCE_MACRO_TASK_TABLE(TouchScreenTask, NULL, osPriorityBelowNormal),
	OS_RESOURCE_MACRO_TASK_TABLE(TempMoniTask, NULL, osPriorityBelowNormal),
	OS_RESOURCE_MACRO_TASK_TABLE(SoundTask, NULL, osPriorityHigh),
	OS_RESOURCE_MACRO_TASK_TABLE(PlayCtrl, NULL, osPriorityAboveNormal),

	// Terminate
	{NULL,NULL,NULL,{0}},
};


void CreateTask(void){
	uint32_t i=0;

	while(s_stTaskTable[i].pOsId != NULL){
		*s_stTaskTable[i].pOsId = osThreadNew(s_stTaskTable[i].TaskFunc, s_stTaskTable[i].argment, &s_stTaskTable[i].Attr);
		i++;
	}
}

/** CMSIS RTOS2 Event Flag Table */
typedef struct{
	osEventFlagsId_t *pefID;
	osEventFlagsAttr_t efAttr;
}stEventFlagTable_t;

/** osEventFlagsId_t */
OS_RESOURCE_MACRO_EVENT_DEFINE(LPUART[1+enLPUART_MAX]);
OS_RESOURCE_MACRO_EVENT_DEFINE(FSReady);
OS_RESOURCE_MACRO_EVENT_DEFINE(CameraSensor);
OS_RESOURCE_MACRO_EVENT_DEFINE(SAITx[enNumOfSAI]);
OS_RESOURCE_MACRO_EVENT_DEFINE(SAIRx[enNumOfSAI]);
OS_RESOURCE_MACRO_EVENT_DEFINE(PlayCtrl);
OS_RESOURCE_MACRO_EVENT_DEFINE(SoundTask[enNumOfSoundTask]);

static stEventFlagTable_t s_stEventFlagTable[] = {
	OS_RESOURCE_MACRO_EVENT_TABLE(LPUART[enLPUART1]),
	OS_RESOURCE_MACRO_EVENT_TABLE(LPUART[enLPUART2]),
	OS_RESOURCE_MACRO_EVENT_TABLE(LPUART[enLPUART3]),
	OS_RESOURCE_MACRO_EVENT_TABLE(LPUART[enLPUART4]),
	OS_RESOURCE_MACRO_EVENT_TABLE(LPUART[enLPUART5]),
	OS_RESOURCE_MACRO_EVENT_TABLE(LPUART[enLPUART6]),
	OS_RESOURCE_MACRO_EVENT_TABLE(LPUART[enLPUART7]),
	OS_RESOURCE_MACRO_EVENT_TABLE(LPUART[enLPUART8]),
	OS_RESOURCE_MACRO_EVENT_TABLE(FSReady),
	OS_RESOURCE_MACRO_EVENT_TABLE(CameraSensor),
	OS_RESOURCE_MACRO_EVENT_TABLE(SAITx[enSAI1]),
	OS_RESOURCE_MACRO_EVENT_TABLE(SAITx[enSAI2]),
	OS_RESOURCE_MACRO_EVENT_TABLE(SAIRx[enSAI1]),
	OS_RESOURCE_MACRO_EVENT_TABLE(SAIRx[enSAI2]),
	OS_RESOURCE_MACRO_EVENT_TABLE(PlayCtrl),
	OS_RESOURCE_MACRO_EVENT_TABLE(SoundTask[enSoundTask1]),
	OS_RESOURCE_MACRO_EVENT_TABLE(SoundTask[enSoundTask2]),

	{NULL, {NULL, 0, NULL, 0}},
};

void CreateEventGroup(void){	
	uint32_t i=0;

	while(s_stEventFlagTable[i].pefID != NULL){
		*s_stEventFlagTable[i].pefID = osEventFlagsNew(&s_stEventFlagTable[i].efAttr);
		i++;
	}
}


typedef struct{
	osSemaphoreId_t *pbsId;
	osSemaphoreAttr_t bsAttr;
	uint32_t u32MaxCount;
	uint32_t u32InitCount;
}stBinarySemaphoreTable_t;

OS_RESOURCE_MACRO_SEM_DEFINE(LPUARTRxSemaphore[1+enLPUART_MAX]);
OS_RESOURCE_MACRO_SEM_DEFINE(LPUARTTxSemaphore[1+enLPUART_MAX]);
OS_RESOURCE_MACRO_SEM_DEFINE(StorageTask);
OS_RESOURCE_MACRO_SEM_DEFINE(CameraTask);
OS_RESOURCE_MACRO_SEM_DEFINE(ComboSensor);

static stBinarySemaphoreTable_t s_stBinarySemaphoreTable[] = {
	OS_RESOURCE_MACRO_SEM_TABLE(LPUARTTxSemaphore[enLPUART1], 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(LPUARTTxSemaphore[enLPUART2], 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(LPUARTTxSemaphore[enLPUART3], 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(LPUARTTxSemaphore[enLPUART4], 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(LPUARTTxSemaphore[enLPUART5], 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(LPUARTTxSemaphore[enLPUART6], 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(LPUARTTxSemaphore[enLPUART7], 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(LPUARTTxSemaphore[enLPUART8], 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(LPUARTRxSemaphore[enLPUART1], 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(LPUARTRxSemaphore[enLPUART2], 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(LPUARTRxSemaphore[enLPUART3], 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(LPUARTRxSemaphore[enLPUART4], 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(LPUARTRxSemaphore[enLPUART5], 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(LPUARTRxSemaphore[enLPUART6], 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(LPUARTRxSemaphore[enLPUART7], 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(LPUARTRxSemaphore[enLPUART8], 1, 1),

	OS_RESOURCE_MACRO_SEM_TABLE(StorageTask, 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(CameraTask, 1, 1),
	OS_RESOURCE_MACRO_SEM_TABLE(ComboSensor, 1, 1),
	{NULL, {NULL, 0, NULL, 0}, 0, 0},
};

void CreateBinarySemaphore(void){	/** CMSIS RTOS2にすること */
	uint32_t i=0;

	while(s_stBinarySemaphoreTable[i].pbsId != NULL){
		*s_stBinarySemaphoreTable[i].pbsId = osSemaphoreNew(s_stBinarySemaphoreTable[i].u32MaxCount, 
															s_stBinarySemaphoreTable[i].u32InitCount, 
															&s_stBinarySemaphoreTable[i].bsAttr);
		
		i++;
	}
}


typedef struct{
	osMessageQueueId_t *pID;
	uint32_t	u32SizeofBlock;
	uint32_t	u32BlockNum;
	osMessageQueueAttr_t stAttr;
}stMsgQueueTable_t;

OS_RESOURCE_MACRO_MSGQUEUE_DEFINE(LcdTask, sizeof(stTaskMsgBlock_t), 32);
OS_RESOURCE_MACRO_MSGQUEUE_DEFINE(TouchScreenTask, sizeof(stTaskMsgBlock_t), 32);
OS_RESOURCE_MACRO_MSGQUEUE_DEFINE(PlayCtrl, sizeof(stTaskMsgBlock_t), 32);
OS_RESOURCE_MACRO_MSGQUEUE_DEFINE(SoundTask[enNumOfSoundTask], sizeof(stTaskMsgBlock_t), 32);

static stMsgQueueTable_t s_stMsgQueueTable[] = {
	OS_RESOURCE_MACRO_MSGQUEUE_TABLE(LcdTask, sizeof(stTaskMsgBlock_t), 32),
	OS_RESOURCE_MACRO_MSGQUEUE_TABLE(TouchScreenTask, sizeof(stTaskMsgBlock_t), 32),
	OS_RESOURCE_MACRO_MSGQUEUE_TABLE(PlayCtrl, sizeof(stTaskMsgBlock_t), 32),
	OS_RESOURCE_MACRO_MSGQUEUE_TABLE(SoundTask[enSoundTask1], sizeof(stTaskMsgBlock_t), 32),
	OS_RESOURCE_MACRO_MSGQUEUE_TABLE(SoundTask[enSoundTask2], sizeof(stTaskMsgBlock_t), 32),
	{NULL, 0, 0, {0}},
};


void CreateMsgQueue(void){
	uint32_t i=0;

	while(s_stMsgQueueTable[i].pID != NULL){
		*s_stMsgQueueTable[i].pID = osMessageQueueNew(s_stMsgQueueTable[i].u32BlockNum, s_stMsgQueueTable[i].u32SizeofBlock, &s_stMsgQueueTable[i].stAttr);
		i++;
	}
}

typedef struct{
	StreamBufferHandle_t *pID;
	uint32_t             BufferSize;
	uint32_t             xTriggerLevel;
	uint8_t	             *pu8Buffer;
	StaticStreamBuffer_t *pxStreamBufferStruct;
}stStreamBuffer_t;

DefALLOCATE_BSS_DTCM alignas(32) StreamBufferHandle_t g_sbhLPUARTTx[1+enLPUART_MAX] = {NULL};
DefALLOCATE_BSS_DTCM alignas(32) static uint8_t s_u8StorageLPUARTTx[1+enLPUART_MAX][1024+1];	/** +1 はマニュアルの指示 */
DefALLOCATE_BSS_DTCM alignas(32) static StaticStreamBuffer_t s_ssbLPUARTTx[1+enLPUART_MAX];
DefALLOCATE_BSS_DTCM alignas(32) StreamBufferHandle_t g_sbhLPUARTRx[1+enLPUART_MAX] = {NULL};
DefALLOCATE_BSS_DTCM alignas(32) static uint8_t s_u8StorageLPUARTRx[1+enLPUART_MAX][1024+1];	/** +1 はマニュアルの指示 */
DefALLOCATE_BSS_DTCM alignas(32) static StaticStreamBuffer_t s_ssbLPUARTRx[1+enLPUART_MAX];

DefALLOCATE_BSS_DTCM alignas(32) StreamBufferHandle_t g_sbhStorageTask[enNumOfSD] = {NULL, NULL};
DefALLOCATE_BSS_DTCM alignas(32) static uint8_t s_StorageTaskStorage[enNumOfSD][sizeof(stTaskMsgBlock_t) * 32 + 1];	/** +1 はマニュアルの指示 */
DefALLOCATE_BSS_DTCM alignas(32) static StaticStreamBuffer_t s_ssbStorageTaskStreamBuffer[enNumOfSD];

DefALLOCATE_BSS_DTCM alignas(32) StreamBufferHandle_t g_sbhLanTask = NULL;
DefALLOCATE_BSS_DTCM alignas(32) static uint8_t s_LanTaskStorage[sizeof(stTaskMsgBlock_t) * 32 + 1];	/** +1 はマニュアルの指示 */
DefALLOCATE_BSS_DTCM alignas(32) static StaticStreamBuffer_t s_ssbSLanTaskStreamBuffer;

DefALLOCATE_BSS_DTCM alignas(32) StreamBufferHandle_t g_sbhCameraTask;
DefALLOCATE_BSS_DTCM alignas(32) static uint8_t s_CameraTaskStorage[sizeof(stTaskMsgBlock_t) * 32 + 1];	/** +1 はマニュアルの指示 */
DefALLOCATE_BSS_DTCM alignas(32) static StaticStreamBuffer_t s_ssbSCameraTaskStreamBuffer;

DefALLOCATE_BSS_DTCM alignas(32) StreamBufferHandle_t g_sbhTouchScreenTask;
DefALLOCATE_BSS_DTCM alignas(32) static uint8_t s_u8TouchScreenTaskStorage[sizeof(stTaskMsgBlock_t) * 32 + 1];	/** +1 はマニュアルの指示 */
DefALLOCATE_BSS_DTCM alignas(32) static StaticStreamBuffer_t s_ssbTouchScreenTaskStreamBuffer;

static stStreamBuffer_t s_stStreamBufferTable[] = {
	{
		&g_sbhStorageTask[enUSDHC1], 
		sizeof(stTaskMsgBlock_t)*32+1, sizeof(stTaskMsgBlock_t), 
		s_StorageTaskStorage[enUSDHC1], &s_ssbStorageTaskStreamBuffer[enUSDHC1]
	},		/** 個数はちゃんと見積もること */
	{
		&g_sbhStorageTask[enUSDHC2], 
		sizeof(stTaskMsgBlock_t)*32+1, sizeof(stTaskMsgBlock_t), 
		s_StorageTaskStorage[enUSDHC2], &s_ssbStorageTaskStreamBuffer[enUSDHC2]
	},		/** 個数はちゃんと見積もること */

	{&g_sbhLPUARTTx[enLPUART1], 1024+1, sizeof(TCHAR), s_u8StorageLPUARTTx[enLPUART1], &s_ssbLPUARTTx[enLPUART1]},
	{&g_sbhLPUARTTx[enLPUART2], 1024+1, sizeof(TCHAR), s_u8StorageLPUARTTx[enLPUART2], &s_ssbLPUARTTx[enLPUART2]},
	{&g_sbhLPUARTTx[enLPUART3], 1024+1, sizeof(TCHAR), s_u8StorageLPUARTTx[enLPUART3], &s_ssbLPUARTTx[enLPUART3]},
	{&g_sbhLPUARTTx[enLPUART4], 1024+1, sizeof(TCHAR), s_u8StorageLPUARTTx[enLPUART4], &s_ssbLPUARTTx[enLPUART4]},
	{&g_sbhLPUARTTx[enLPUART5], 1024+1, sizeof(TCHAR), s_u8StorageLPUARTTx[enLPUART5], &s_ssbLPUARTTx[enLPUART5]},
	{&g_sbhLPUARTTx[enLPUART6], 1024+1, sizeof(TCHAR), s_u8StorageLPUARTTx[enLPUART6], &s_ssbLPUARTTx[enLPUART6]},
	{&g_sbhLPUARTTx[enLPUART7], 1024+1, sizeof(TCHAR), s_u8StorageLPUARTTx[enLPUART7], &s_ssbLPUARTTx[enLPUART7]},
	{&g_sbhLPUARTTx[enLPUART8], 1024+1, sizeof(TCHAR), s_u8StorageLPUARTTx[enLPUART8], &s_ssbLPUARTTx[enLPUART8]},

	{&g_sbhLPUARTRx[enLPUART1], 1024+1, sizeof(TCHAR), s_u8StorageLPUARTRx[enLPUART1], &s_ssbLPUARTRx[enLPUART1]},
	{&g_sbhLPUARTRx[enLPUART2], 1024+1, sizeof(TCHAR), s_u8StorageLPUARTRx[enLPUART2], &s_ssbLPUARTRx[enLPUART2]},
	{&g_sbhLPUARTRx[enLPUART3], 1024+1, sizeof(TCHAR), s_u8StorageLPUARTRx[enLPUART3], &s_ssbLPUARTRx[enLPUART3]},
	{&g_sbhLPUARTRx[enLPUART4], 1024+1, sizeof(TCHAR), s_u8StorageLPUARTRx[enLPUART4], &s_ssbLPUARTRx[enLPUART4]},
	{&g_sbhLPUARTRx[enLPUART5], 1024+1, sizeof(TCHAR), s_u8StorageLPUARTRx[enLPUART5], &s_ssbLPUARTRx[enLPUART5]},
	{&g_sbhLPUARTRx[enLPUART6], 1024+1, sizeof(TCHAR), s_u8StorageLPUARTRx[enLPUART6], &s_ssbLPUARTRx[enLPUART6]},
	{&g_sbhLPUARTRx[enLPUART7], 1024+1, sizeof(TCHAR), s_u8StorageLPUARTRx[enLPUART7], &s_ssbLPUARTRx[enLPUART7]},
	{&g_sbhLPUARTRx[enLPUART8], 1024+1, sizeof(TCHAR), s_u8StorageLPUARTRx[enLPUART8], &s_ssbLPUARTRx[enLPUART8]},

	{&g_sbhLanTask, sizeof(s_LanTaskStorage), sizeof(stTaskMsgBlock_t), s_LanTaskStorage, &s_ssbSLanTaskStreamBuffer},
	{&g_sbhCameraTask, sizeof(s_CameraTaskStorage), sizeof(stTaskMsgBlock_t), s_CameraTaskStorage, &s_ssbSCameraTaskStreamBuffer},
	{&g_sbhTouchScreenTask, sizeof(s_u8TouchScreenTaskStorage), sizeof(stTaskMsgBlock_t), s_u8TouchScreenTaskStorage, &s_ssbTouchScreenTaskStreamBuffer},

	{NULL, 0, 0, NULL, NULL},
};
void CreateStreamBuffer(void){
	uint32_t i=0;

	while(s_stStreamBufferTable[i].pID != NULL){
		*s_stStreamBufferTable[i].pID = xStreamBufferCreateStatic(
			s_stStreamBufferTable[i].BufferSize, s_stStreamBufferTable[i].xTriggerLevel, 
			s_stStreamBufferTable[i].pu8Buffer, s_stStreamBufferTable[i].pxStreamBufferStruct);
		i++;
	}
}
