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

/** typedef Task Table */
typedef struct{
	osThreadId_t   *pOsId;
	osThreadFunc_t TaskFunc;
	void           *argment;
	osThreadAttr_t Attr;
}stOSdefTable_t;

#if (defined(__GNUC__)) && (defined(__ARM_ARCH_7M__))
#define ALLOCATE_IN_DTCM __attribute__((section(".bss.$SRAM_DTC*"))) 
#else
#define ALLOCATE_IN_DTCM
#endif

/** Task Handle */
ALLOCATE_IN_DTCM alignas(32) osThreadId_t g_InitialTaskHandle;
ALLOCATE_IN_DTCM alignas(32) osThreadId_t g_ConsoleTaskHandle;
ALLOCATE_IN_DTCM alignas(32) osThreadId_t g_StorageTaskHandle;
ALLOCATE_IN_DTCM alignas(32) osThreadId_t g_LanTaskHandle;

/** Task Control Block (STATIC ALLOCATION)*/
ALLOCATE_IN_DTCM alignas(32) static StaticTask_t s_InitialTaskTCB;
ALLOCATE_IN_DTCM alignas(32) static StaticTask_t s_ConsoleTaskTCB;
ALLOCATE_IN_DTCM alignas(32) static StaticTask_t s_StorageTaskTCB;
ALLOCATE_IN_DTCM alignas(32) static StaticTask_t s_LanTaskTCB;

/** Task Stack (STATIC ALLOCATION)*/
ALLOCATE_IN_DTCM alignas(32) static uint32_t s_InitialTaskStack[8192/sizeof(uint32_t)];
ALLOCATE_IN_DTCM alignas(32) static uint32_t s_ConsoleTaskStack[8192/sizeof(uint32_t)];
ALLOCATE_IN_DTCM alignas(32) static uint32_t s_StorageTaskStack[8192/sizeof(uint32_t)];
ALLOCATE_IN_DTCM alignas(32) static uint32_t s_LanTaskStack[8192/sizeof(uint32_t)];

/** Task Table */
static const stOSdefTable_t s_stTaskTable[] = {
	{
		&g_InitialTaskHandle,
		(osThreadFunc_t)InitialTask,
		NULL,
		{"InitialTask", osThreadDetached, &s_InitialTaskTCB, sizeof(s_InitialTaskTCB), s_InitialTaskStack, sizeof(s_InitialTaskStack), osPriorityLow, 0, 0},
	},
	{
		&g_ConsoleTaskHandle,
		(osThreadFunc_t)ConsoleTask,
		NULL,
		{"ConsoleTask", osThreadDetached, &s_ConsoleTaskTCB, sizeof(s_ConsoleTaskTCB), s_ConsoleTaskStack, sizeof(s_ConsoleTaskStack), osPriorityBelowNormal, 0, 0},
	},
	{	/** StorageTask1 */
		&g_StorageTaskHandle,
		(osThreadFunc_t)StorageTask,
		(void*)enUSDHC1,
		{"StorageTask1", osThreadDetached, &s_StorageTaskTCB, sizeof(s_StorageTaskTCB), s_StorageTaskStack, sizeof(s_StorageTaskStack), osPriorityNormal, 0, 0},
	},
	{	/** LanTask */
		&g_StorageTaskHandle,
		(osThreadFunc_t)LanTask,
		NULL,
		{"LanTask", osThreadDetached, &s_LanTaskTCB, sizeof(s_LanTaskTCB), s_LanTaskStack, sizeof(s_LanTaskStack), osPriorityBelowNormal, 0, 0},
	},
	// Terminate
	{	
		NULL,
		NULL,
		NULL,
		{0},
	},
};


void CreateTask(void){
	uint32_t i=0;

	while(s_stTaskTable[i].pOsId != NULL){
		*s_stTaskTable[i].pOsId = osThreadNew(s_stTaskTable[i].TaskFunc, s_stTaskTable[i].argment, &s_stTaskTable[i].Attr);
		i++;
	}
}

typedef struct{
	EventGroupHandle_t *pEGHandle;
	StaticEventGroup_t *pEGBuffer;
}stEventGroupTable_t;

/** Event Group Handle */
ALLOCATE_IN_DTCM alignas(32) EventGroupHandle_t g_xLPUARTEventGroup[1+enLPUART_MAX];
ALLOCATE_IN_DTCM alignas(32) EventGroupHandle_t g_xFSReadyEventGroup;
/** Event Group Buffer */
ALLOCATE_IN_DTCM alignas(32) static StaticEventGroup_t s_xLPUARTEventGroupBuffer[1+enLPUART_MAX];
ALLOCATE_IN_DTCM alignas(32) static StaticEventGroup_t s_xFSReadyEventGroupBuffer;
static stEventGroupTable_t s_stEventGroupTable[] = {
	{&g_xLPUARTEventGroup[enLPUART1], &s_xLPUARTEventGroupBuffer[enLPUART1]},
	{&g_xLPUARTEventGroup[enLPUART2], &s_xLPUARTEventGroupBuffer[enLPUART2]},
	{&g_xLPUARTEventGroup[enLPUART3], &s_xLPUARTEventGroupBuffer[enLPUART3]},
	{&g_xLPUARTEventGroup[enLPUART4], &s_xLPUARTEventGroupBuffer[enLPUART4]},
	{&g_xLPUARTEventGroup[enLPUART5], &s_xLPUARTEventGroupBuffer[enLPUART5]},
	{&g_xLPUARTEventGroup[enLPUART6], &s_xLPUARTEventGroupBuffer[enLPUART6]},
	{&g_xLPUARTEventGroup[enLPUART7], &s_xLPUARTEventGroupBuffer[enLPUART7]},
	{&g_xLPUARTEventGroup[enLPUART8], &s_xLPUARTEventGroupBuffer[enLPUART8]},

	{&g_xFSReadyEventGroup, &s_xFSReadyEventGroupBuffer},
	{NULL, NULL},
};

void CreateEventGroup(void){	/** CMSIS RTOS2にすること */
	uint32_t i=0;

	while(s_stEventGroupTable[i].pEGHandle != NULL){
		*s_stEventGroupTable[i].pEGHandle = xEventGroupCreateStatic(s_stEventGroupTable[i].pEGBuffer);
		i++;
	}
}


typedef struct{
	SemaphoreHandle_t *pSMHandle;
	StaticSemaphore_t *pSMBuffer;
}stBinarySemaphoreTable_t;

ALLOCATE_IN_DTCM alignas(32) SemaphoreHandle_t g_xLPUARTRxSemaphore[1+enLPUART_MAX] = {NULL};
ALLOCATE_IN_DTCM alignas(32) SemaphoreHandle_t g_xLPUARTTxSemaphore[1+enLPUART_MAX] = {NULL};
ALLOCATE_IN_DTCM alignas(32) static StaticSemaphore_t s_xLPUARTRxSemaphoreBuffer[1+enLPUART_MAX];
ALLOCATE_IN_DTCM alignas(32) static StaticSemaphore_t s_xLPUARTTxSemaphoreBuffer[1+enLPUART_MAX];

static stBinarySemaphoreTable_t s_stBinarySemaphoreTable[] = {
	{&g_xLPUARTRxSemaphore[enLPUART1], &s_xLPUARTRxSemaphoreBuffer[enLPUART1]},
	{&g_xLPUARTRxSemaphore[enLPUART2], &s_xLPUARTRxSemaphoreBuffer[enLPUART2]},
	{&g_xLPUARTRxSemaphore[enLPUART3], &s_xLPUARTRxSemaphoreBuffer[enLPUART3]},
	{&g_xLPUARTRxSemaphore[enLPUART4], &s_xLPUARTRxSemaphoreBuffer[enLPUART4]},
	{&g_xLPUARTRxSemaphore[enLPUART5], &s_xLPUARTRxSemaphoreBuffer[enLPUART5]},
	{&g_xLPUARTRxSemaphore[enLPUART6], &s_xLPUARTRxSemaphoreBuffer[enLPUART6]},
	{&g_xLPUARTRxSemaphore[enLPUART7], &s_xLPUARTRxSemaphoreBuffer[enLPUART7]},
	{&g_xLPUARTRxSemaphore[enLPUART8], &s_xLPUARTRxSemaphoreBuffer[enLPUART8]},

	{&g_xLPUARTTxSemaphore[enLPUART1], &s_xLPUARTTxSemaphoreBuffer[enLPUART1]},
	{&g_xLPUARTTxSemaphore[enLPUART2], &s_xLPUARTTxSemaphoreBuffer[enLPUART2]},
	{&g_xLPUARTTxSemaphore[enLPUART3], &s_xLPUARTTxSemaphoreBuffer[enLPUART3]},
	{&g_xLPUARTTxSemaphore[enLPUART4], &s_xLPUARTTxSemaphoreBuffer[enLPUART4]},
	{&g_xLPUARTTxSemaphore[enLPUART5], &s_xLPUARTTxSemaphoreBuffer[enLPUART5]},
	{&g_xLPUARTTxSemaphore[enLPUART6], &s_xLPUARTTxSemaphoreBuffer[enLPUART6]},
	{&g_xLPUARTTxSemaphore[enLPUART7], &s_xLPUARTTxSemaphoreBuffer[enLPUART7]},
	{&g_xLPUARTTxSemaphore[enLPUART8], &s_xLPUARTTxSemaphoreBuffer[enLPUART8]},
	{NULL, NULL},
};

void CreateBinarySemaphore(void){	/** CMSIS RTOS2にすること */
	uint32_t i=0;

	while(s_stBinarySemaphoreTable[i].pSMHandle != NULL){
		*s_stBinarySemaphoreTable[i].pSMHandle = xSemaphoreCreateBinaryStatic(s_stBinarySemaphoreTable[i].pSMBuffer);
		
		i++;
	}
}


typedef struct{
	osMessageQueueId_t *pID;
	uint32_t	u32BlockNum;
	uint32_t	u32SizeofBlock;
	osMessageQueueAttr_t stAttr;	/** 今のところ使い道がない */
}stQueueTable_t;

ALLOCATE_IN_DTCM alignas(32) osMessageQueueId_t g_StorageTaskQueueId = NULL;

static stQueueTable_t s_stQueueTable[] = {
	{&g_StorageTaskQueueId, 32, sizeof(stTaskMsgBlock_t), {0}},
	{NULL, 0, 0, {0}},
};

void CreateQueue(void){
	uint32_t i=0;

	while(s_stQueueTable[i].pID != NULL){
		*s_stQueueTable[i].pID = osMessageQueueNew(s_stQueueTable[i].u32BlockNum, s_stQueueTable[i].u32SizeofBlock, NULL);
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

ALLOCATE_IN_DTCM alignas(32) StreamBufferHandle_t g_sbhLPUARTTx[1+enLPUART_MAX] = {NULL};
ALLOCATE_IN_DTCM alignas(32) static uint8_t s_u8StorageLPUARTTx[1+enLPUART_MAX][1024+1];	/** +1 はマニュアルの指示 */
ALLOCATE_IN_DTCM alignas(32) static StaticStreamBuffer_t s_ssbLPUARTTx[1+enLPUART_MAX];
ALLOCATE_IN_DTCM alignas(32) StreamBufferHandle_t g_sbhLPUARTRx[1+enLPUART_MAX] = {NULL};
ALLOCATE_IN_DTCM alignas(32) static uint8_t s_u8StorageLPUARTRx[1+enLPUART_MAX][1024+1];	/** +1 はマニュアルの指示 */
ALLOCATE_IN_DTCM alignas(32) static StaticStreamBuffer_t s_ssbLPUARTRx[1+enLPUART_MAX];

ALLOCATE_IN_DTCM alignas(32) StreamBufferHandle_t g_sbhStorageTask[enNumOfSD] = {NULL, NULL};
ALLOCATE_IN_DTCM alignas(32) static uint8_t s_StorageTaskStorage[enNumOfSD][sizeof(stTaskMsgBlock_t) * 32 + 1];	/** +1 はマニュアルの指示 */
ALLOCATE_IN_DTCM alignas(32) static StaticStreamBuffer_t s_ssbStorageTaskStreamBuffer[enNumOfSD];

ALLOCATE_IN_DTCM alignas(32) StreamBufferHandle_t g_sbhLanTask = NULL;
ALLOCATE_IN_DTCM alignas(32) static uint8_t s_LanTaskStorage[sizeof(stTaskMsgBlock_t) * 32 + 1];	/** +1 はマニュアルの指示 */
ALLOCATE_IN_DTCM alignas(32) static StaticStreamBuffer_t s_ssbSLanTaskStreamBuffer;

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
