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

/** typedef Task Table */
typedef struct{
	osThreadId_t   *pOsId;
	osThreadFunc_t TaskFunc;
	void           *argment;
	osThreadAttr_t Attr;
}stOSdefTable_t;


/** Task Handle */
DefALLOCATE_BSS_DTCM alignas(32) osThreadId_t g_InitialTaskHandle;
DefALLOCATE_BSS_DTCM alignas(32) osThreadId_t g_ConsoleTaskHandle;
DefALLOCATE_BSS_DTCM alignas(32) osThreadId_t g_StorageTaskHandle;
DefALLOCATE_BSS_DTCM alignas(32) osThreadId_t g_LanTaskHandle;
//DefALLOCATE_BSS_DTCM alignas(32) osThreadId_t g_SensorTaskHandle;
//DefALLOCATE_BSS_DTCM alignas(32) osThreadId_t g_CameraTaskHandle;
DefALLOCATE_BSS_DTCM alignas(32) osThreadId_t g_LcdTaskHandle;
DefALLOCATE_BSS_DTCM alignas(32) osThreadId_t g_TouchScreenTaskHandle;

/** Task Control Block (STATIC ALLOCATION)*/
DefALLOCATE_BSS_DTCM alignas(32) static StaticTask_t s_InitialTaskTCB;
DefALLOCATE_BSS_DTCM alignas(32) static StaticTask_t s_ConsoleTaskTCB;
DefALLOCATE_BSS_DTCM alignas(32) static StaticTask_t s_StorageTaskTCB;
DefALLOCATE_BSS_DTCM alignas(32) static StaticTask_t s_LanTaskTCB;
//DefALLOCATE_BSS_DTCM alignas(32) static StaticTask_t s_SensorTaskTCB;
//DefALLOCATE_BSS_DTCM alignas(32) static StaticTask_t s_CameraTaskTCB;
DefALLOCATE_BSS_DTCM alignas(32) static StaticTask_t s_LcdTaskTCB;
DefALLOCATE_BSS_DTCM alignas(32) static StaticTask_t s_TouchScreenTaskTCB;

/** Task Stack (STATIC ALLOCATION)*/
DefALLOCATE_BSS_DTCM alignas(32) static uint32_t s_InitialTaskStack[8192/sizeof(uint32_t)];
DefALLOCATE_BSS_DTCM alignas(32) static uint32_t s_ConsoleTaskStack[8192/sizeof(uint32_t)];
DefALLOCATE_BSS_DTCM alignas(32) static uint32_t s_StorageTaskStack[8192/sizeof(uint32_t)];
DefALLOCATE_BSS_DTCM alignas(32) static uint32_t s_LanTaskStack[8192/sizeof(uint32_t)];
//DefALLOCATE_BSS_DTCM alignas(32) static uint32_t s_SensorTaskStack[8192/sizeof(uint32_t)];
//DefALLOCATE_BSS_DTCM alignas(32) static uint32_t s_CameraTaskStack[8192/sizeof(uint32_t)];
DefALLOCATE_BSS_DTCM alignas(32) static uint32_t s_LcdTaskStack[8192/sizeof(uint32_t)];
DefALLOCATE_BSS_DTCM alignas(32) static uint32_t s_TouchScreenTaskStack[8192/sizeof(uint32_t)];

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
		&g_LanTaskHandle,
		(osThreadFunc_t)LanTask,
		NULL,
		{"LanTask", osThreadDetached, &s_LanTaskTCB, sizeof(s_LanTaskTCB), s_LanTaskStack, sizeof(s_LanTaskStack), osPriorityBelowNormal, 0, 0},
	},
	{	/** LcdTask */
		&g_LcdTaskHandle,
		(osThreadFunc_t)LcdTask,
		NULL,
		{"LcdTask", osThreadDetached, &s_LcdTaskTCB, sizeof(s_LcdTaskTCB), s_LcdTaskStack, sizeof(s_LcdTaskStack), osPriorityAboveNormal, 0, 0},
	},
	{	/** LcdTask */
		&g_TouchScreenTaskHandle,
		(osThreadFunc_t)TouchScreenTask,
		NULL,
		{"TouchScreenTask", osThreadDetached, &s_TouchScreenTaskTCB, sizeof(s_TouchScreenTaskTCB), s_TouchScreenTaskStack, sizeof(s_TouchScreenTaskStack), osPriorityBelowNormal, 0, 0},
	},
#if 0	/** IMXRT1060-EVK doesn't have FXOS8700 */
	{	/** SensorTask */
		&g_SensorTaskHandle,
		(osThreadFunc_t)SensorTask,
		NULL,
		{"SensorTask", osThreadDetached, &s_SensorTaskTCB, sizeof(s_SensorTaskTCB), s_SensorTaskStack, sizeof(s_SensorTaskStack), osPriorityBelowNormal, 0, 0},
	},

	{	/** CameraTask */
		&g_CameraTaskHandle,
		(osThreadFunc_t)CameraTask,
		NULL,
		{"CameraTask", osThreadDetached, &s_CameraTaskTCB, sizeof(s_CameraTaskTCB), s_CameraTaskStack, sizeof(s_CameraTaskStack), osPriorityBelowNormal, 0, 0},
	},
#endif
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

/** CMSIS RTOS2 Event Flag Table */
typedef struct{
	osEventFlagsId_t *pefID;
	osEventFlagsAttr_t efAttr;
}stEventFlagTable_t;

/** osEventFlagsId_t */
DefALLOCATE_BSS_DTCM alignas(4) osEventFlagsId_t g_efLPUART[1+enLPUART_MAX];
DefALLOCATE_BSS_DTCM alignas(4) osEventFlagsId_t g_efFSReady;
DefALLOCATE_BSS_DTCM alignas(4) osEventFlagsId_t g_efCameraSensor;

/** StaticEventGroup_t */
DefALLOCATE_BSS_DTCM alignas(4) static StaticEventGroup_t s_xLPUARTEventGroupBuffer[1+enLPUART_MAX];
DefALLOCATE_BSS_DTCM alignas(4) static StaticEventGroup_t s_xFSReadyEventGroupBuffer;
DefALLOCATE_BSS_DTCM alignas(4) static StaticEventGroup_t s_xCameraSensorEventGroupBuffer;

static stEventFlagTable_t s_stEventFlagTable[] = {
	{&g_efLPUART[enLPUART1], {"EF_LPUART", 0, &s_xLPUARTEventGroupBuffer[enLPUART1], sizeof(StaticEventGroup_t)}},
	{&g_efLPUART[enLPUART2], {"EF_LPUART", 0, &s_xLPUARTEventGroupBuffer[enLPUART2], sizeof(StaticEventGroup_t)}},
	{&g_efLPUART[enLPUART3], {"EF_LPUART", 0, &s_xLPUARTEventGroupBuffer[enLPUART3], sizeof(StaticEventGroup_t)}},
	{&g_efLPUART[enLPUART4], {"EF_LPUART", 0, &s_xLPUARTEventGroupBuffer[enLPUART4], sizeof(StaticEventGroup_t)}},
	{&g_efLPUART[enLPUART5], {"EF_LPUART", 0, &s_xLPUARTEventGroupBuffer[enLPUART5], sizeof(StaticEventGroup_t)}},
	{&g_efLPUART[enLPUART6], {"EF_LPUART", 0, &s_xLPUARTEventGroupBuffer[enLPUART6], sizeof(StaticEventGroup_t)}},
	{&g_efLPUART[enLPUART7], {"EF_LPUART", 0, &s_xLPUARTEventGroupBuffer[enLPUART7], sizeof(StaticEventGroup_t)}},
	{&g_efLPUART[enLPUART8], {"EF_LPUART", 0, &s_xLPUARTEventGroupBuffer[enLPUART8], sizeof(StaticEventGroup_t)}},

	{&g_efFSReady, {"EF_FSREADY", 0, &s_xFSReadyEventGroupBuffer, sizeof(StaticEventGroup_t)}},
	{&g_efCameraSensor, {"EF_CAMERA_SENSOR", 0, &s_xCameraSensorEventGroupBuffer, sizeof(StaticEventGroup_t)}},
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

DefALLOCATE_BSS_DTCM alignas(4) osSemaphoreId_t g_bsIdLPUARTRxSemaphore[1+enLPUART_MAX] = {NULL};
DefALLOCATE_BSS_DTCM alignas(4) osSemaphoreId_t g_bsIdLPUARTTxSemaphore[1+enLPUART_MAX] = {NULL};
DefALLOCATE_BSS_DTCM alignas(4) osSemaphoreId_t g_bsIdStorageTaskMsg;
DefALLOCATE_BSS_DTCM alignas(4) osSemaphoreId_t g_bsIdComboSensor;
DefALLOCATE_BSS_DTCM alignas(4) osSemaphoreId_t g_bsIdCameraTask;

DefALLOCATE_BSS_DTCM alignas(32) static StaticSemaphore_t s_xLPUARTRxSemaphoreBuffer[1+enLPUART_MAX];
DefALLOCATE_BSS_DTCM alignas(32) static StaticSemaphore_t s_xLPUARTTxSemaphoreBuffer[1+enLPUART_MAX];
DefALLOCATE_BSS_DTCM alignas(32) static StaticSemaphore_t s_xStorageTaskMsgBuffer;
DefALLOCATE_BSS_DTCM alignas(32) static StaticSemaphore_t s_xComboSensor;
DefALLOCATE_BSS_DTCM alignas(32) static StaticSemaphore_t s_xCameraTask;

static stBinarySemaphoreTable_t s_stBinarySemaphoreTable[] = {
	{&g_bsIdLPUARTRxSemaphore[enLPUART1], {"BS_LPUART1RX", 0, &s_xLPUARTRxSemaphoreBuffer[enLPUART1], sizeof(StaticSemaphore_t)}, 1, 1},
	{&g_bsIdLPUARTRxSemaphore[enLPUART2], {"BS_LPUART2RX", 0, &s_xLPUARTRxSemaphoreBuffer[enLPUART2], sizeof(StaticSemaphore_t)}, 1, 1},
	{&g_bsIdLPUARTRxSemaphore[enLPUART3], {"BS_LPUART3RX", 0, &s_xLPUARTRxSemaphoreBuffer[enLPUART3], sizeof(StaticSemaphore_t)}, 1, 1},
	{&g_bsIdLPUARTRxSemaphore[enLPUART4], {"BS_LPUART4RX", 0, &s_xLPUARTRxSemaphoreBuffer[enLPUART4], sizeof(StaticSemaphore_t)}, 1, 1},
	{&g_bsIdLPUARTRxSemaphore[enLPUART5], {"BS_LPUART5RX", 0, &s_xLPUARTRxSemaphoreBuffer[enLPUART5], sizeof(StaticSemaphore_t)}, 1, 1},
	{&g_bsIdLPUARTRxSemaphore[enLPUART6], {"BS_LPUART6RX", 0, &s_xLPUARTRxSemaphoreBuffer[enLPUART6], sizeof(StaticSemaphore_t)}, 1, 1},
	{&g_bsIdLPUARTRxSemaphore[enLPUART7], {"BS_LPUART7RX", 0, &s_xLPUARTRxSemaphoreBuffer[enLPUART7], sizeof(StaticSemaphore_t)}, 1, 1},
	{&g_bsIdLPUARTRxSemaphore[enLPUART8], {"BS_LPUART8RX", 0, &s_xLPUARTRxSemaphoreBuffer[enLPUART8], sizeof(StaticSemaphore_t)}, 1, 1},

	{&g_bsIdLPUARTTxSemaphore[enLPUART1], {"BS_LPUART1TX", 0, &s_xLPUARTTxSemaphoreBuffer[enLPUART1], sizeof(StaticSemaphore_t)}, 1, 1},
	{&g_bsIdLPUARTTxSemaphore[enLPUART2], {"BS_LPUART2TX", 0, &s_xLPUARTTxSemaphoreBuffer[enLPUART2], sizeof(StaticSemaphore_t)}, 1, 1},
	{&g_bsIdLPUARTTxSemaphore[enLPUART3], {"BS_LPUART3TX", 0, &s_xLPUARTTxSemaphoreBuffer[enLPUART3], sizeof(StaticSemaphore_t)}, 1, 1},
	{&g_bsIdLPUARTTxSemaphore[enLPUART4], {"BS_LPUART4TX", 0, &s_xLPUARTTxSemaphoreBuffer[enLPUART4], sizeof(StaticSemaphore_t)}, 1, 1},
	{&g_bsIdLPUARTTxSemaphore[enLPUART5], {"BS_LPUART5TX", 0, &s_xLPUARTTxSemaphoreBuffer[enLPUART5], sizeof(StaticSemaphore_t)}, 1, 1},
	{&g_bsIdLPUARTTxSemaphore[enLPUART6], {"BS_LPUART6TX", 0, &s_xLPUARTTxSemaphoreBuffer[enLPUART6], sizeof(StaticSemaphore_t)}, 1, 1},
	{&g_bsIdLPUARTTxSemaphore[enLPUART7], {"BS_LPUART7TX", 0, &s_xLPUARTTxSemaphoreBuffer[enLPUART7], sizeof(StaticSemaphore_t)}, 1, 1},
	{&g_bsIdLPUARTTxSemaphore[enLPUART8], {"BS_LPUART8TX", 0, &s_xLPUARTTxSemaphoreBuffer[enLPUART8], sizeof(StaticSemaphore_t)}, 1, 1},

	{&g_bsIdStorageTaskMsg, {"BS_STORAGETASKMSG", 0, &s_xStorageTaskMsgBuffer, sizeof(StaticSemaphore_t)}, 1, 1},
	{&g_bsIdComboSensor, {"BS_COMBOSENSOR", 0, &s_xComboSensor, sizeof(StaticSemaphore_t)}, 1, 1},
	{&g_bsIdCameraTask, {"BS_CAMERATASKMSG", 0, &s_xCameraTask, sizeof(StaticSemaphore_t)}, 1, 1},

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
	uint32_t	u32BlockNum;
	uint32_t	u32SizeofBlock;
	osMessageQueueAttr_t stAttr;	/** 今のところ使い道がない */
}stQueueTable_t;

DefALLOCATE_BSS_DTCM alignas(32) osMessageQueueId_t g_StorageTaskQueueId = NULL;

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


typedef struct{
	QueueHandle_t    *pID;
	uint32_t         QueueLength;
	uint32_t         ItemSize;
	uint8_t          *pu8Buffer;
	StaticQueue_t    *pxMsgQueueStruct;
}stMsgQueue_t;

DefALLOCATE_BSS_DTCM alignas(32) QueueHandle_t g_mqLcdTask;
DefALLOCATE_BSS_DTCM alignas(32) static uint8_t s_u8LcdTskStorage[sizeof(stTaskMsgBlock_t) * 32];
DefALLOCATE_BSS_DTCM alignas(32) static StaticQueue_t g_sqLcdTask;

static stMsgQueue_t s_stMsgQueueTable[] = {
	{&g_mqLcdTask, 32, sizeof(stTaskMsgBlock_t), s_u8LcdTskStorage, &g_sqLcdTask},
	{NULL, 0, 0, NULL, NULL},
};

void CreateMsgQueue(void){
	uint32_t i=0;

	while(s_stMsgQueueTable[i].pID != NULL){
		*s_stMsgQueueTable[i].pID = xQueueCreateStatic(
			s_stMsgQueueTable[i].QueueLength, 
			s_stMsgQueueTable[i].ItemSize, 
			s_stMsgQueueTable[i].pu8Buffer, 
			s_stMsgQueueTable[i].pxMsgQueueStruct);
		i++;
	}
}
