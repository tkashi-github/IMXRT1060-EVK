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
DefALLOCATE_BSS_DTCM alignas(4) osEventFlagsId_t g_efLPUART[1+enLPUART_MAX];
DefALLOCATE_BSS_DTCM alignas(4) osEventFlagsId_t g_efFSReady;
DefALLOCATE_BSS_DTCM alignas(4) osEventFlagsId_t g_efCameraSensor;
DefALLOCATE_BSS_DTCM alignas(32) osEventFlagsId_t g_efSAITx[enNumOfSAI];
DefALLOCATE_BSS_DTCM alignas(32) osEventFlagsId_t g_efSAIRx[enNumOfSAI];
DefALLOCATE_BSS_DTCM alignas(32) osEventFlagsId_t g_efPlayCtrlEventGroup;
DefALLOCATE_BSS_DTCM alignas(32) osEventFlagsId_t g_efSoundTaskEventGroup[enNumOfSoundTask];

/** StaticEventGroup_t */
DefALLOCATE_BSS_DTCM alignas(4) static StaticEventGroup_t s_xLPUARTEventGroupBuffer[1+enLPUART_MAX];
DefALLOCATE_BSS_DTCM alignas(4) static StaticEventGroup_t s_xFSReadyEventGroupBuffer;
DefALLOCATE_BSS_DTCM alignas(4) static StaticEventGroup_t s_xCameraSensorEventGroupBuffer;
DefALLOCATE_BSS_DTCM alignas(32) static StaticEventGroup_t s_segSAITxEventGroupBuffer[enNumOfSAI];
DefALLOCATE_BSS_DTCM alignas(32) static StaticEventGroup_t s_segSAIRxEventGroupBuffer[enNumOfSAI];
DefALLOCATE_BSS_DTCM alignas(32) static StaticEventGroup_t s_segPlayCtrlEventGroupBuffer;
DefALLOCATE_BSS_DTCM alignas(32) static StaticEventGroup_t s_segSoundTaskEventGroupBuffer[enNumOfSoundTask];

static stEventFlagTable_t s_stEventFlagTable[] = {
	{&g_efLPUART[enLPUART1], {"EF_LPUART", 0, &s_xLPUARTEventGroupBuffer[enLPUART1], sizeof(StaticEventGroup_t)}},
	{&g_efLPUART[enLPUART2], {"EF_LPUART", 0, &s_xLPUARTEventGroupBuffer[enLPUART2], sizeof(StaticEventGroup_t)}},
	{&g_efLPUART[enLPUART3], {"EF_LPUART", 0, &s_xLPUARTEventGroupBuffer[enLPUART3], sizeof(StaticEventGroup_t)}},
	{&g_efLPUART[enLPUART4], {"EF_LPUART", 0, &s_xLPUARTEventGroupBuffer[enLPUART4], sizeof(StaticEventGroup_t)}},
	{&g_efLPUART[enLPUART5], {"EF_LPUART", 0, &s_xLPUARTEventGroupBuffer[enLPUART5], sizeof(StaticEventGroup_t)}},
	{&g_efLPUART[enLPUART6], {"EF_LPUART", 0, &s_xLPUARTEventGroupBuffer[enLPUART6], sizeof(StaticEventGroup_t)}},
	{&g_efLPUART[enLPUART7], {"EF_LPUART", 0, &s_xLPUARTEventGroupBuffer[enLPUART7], sizeof(StaticEventGroup_t)}},
	{&g_efLPUART[enLPUART8], {"EF_LPUART", 0, &s_xLPUARTEventGroupBuffer[enLPUART8], sizeof(StaticEventGroup_t)}},

	{&g_efSAITx[enSAI1], {"EF_SAITx1", 0, &s_segSAITxEventGroupBuffer[enSAI1], sizeof(StaticEventGroup_t)}},
	{&g_efSAITx[enSAI2], {"EF_SAITx2", 0, &s_segSAITxEventGroupBuffer[enSAI2], sizeof(StaticEventGroup_t)}},

	{&g_efSAIRx[enSAI1], {"EF_SAIRx1", 0, &s_segSAIRxEventGroupBuffer[enSAI1], sizeof(StaticEventGroup_t)}},
	{&g_efSAIRx[enSAI2], {"EF_SAIRx2", 0, &s_segSAIRxEventGroupBuffer[enSAI2], sizeof(StaticEventGroup_t)}},

	{&g_efPlayCtrlEventGroup, {"EF_PLAYCTRL", 0, &s_segPlayCtrlEventGroupBuffer, sizeof(StaticEventGroup_t)}},

	{&g_efSoundTaskEventGroup[enSAI1], {"EF_SoundTask1", 0, &s_segSoundTaskEventGroupBuffer[enSoundTask1], sizeof(StaticEventGroup_t)}},
	{&g_efSoundTaskEventGroup[enSAI2], {"EF_SoundTask2", 0, &s_segSoundTaskEventGroupBuffer[enSoundTask2], sizeof(StaticEventGroup_t)}},
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
DefALLOCATE_BSS_DTCM alignas(4) osSemaphoreId_t g_bsIdMousePosition;

DefALLOCATE_BSS_DTCM alignas(32) static StaticSemaphore_t s_xLPUARTRxSemaphoreBuffer[1+enLPUART_MAX];
DefALLOCATE_BSS_DTCM alignas(32) static StaticSemaphore_t s_xLPUARTTxSemaphoreBuffer[1+enLPUART_MAX];
DefALLOCATE_BSS_DTCM alignas(32) static StaticSemaphore_t s_xStorageTaskMsgBuffer;
DefALLOCATE_BSS_DTCM alignas(32) static StaticSemaphore_t s_xComboSensor;
DefALLOCATE_BSS_DTCM alignas(32) static StaticSemaphore_t s_xCameraTask;
DefALLOCATE_BSS_DTCM alignas(32) static StaticSemaphore_t s_xMousePosition;

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
	{&g_bsIdMousePosition, {"BS_MOUSEPOSITION", 0, &s_xMousePosition, sizeof(StaticSemaphore_t)}, 1, 1},

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
	osMessageQueueAttr_t stAttr;
}stQueueTable_t;

DefALLOCATE_BSS_DTCM alignas(32) osMessageQueueId_t g_mqLcdTask;
DefALLOCATE_BSS_DTCM alignas(32) static uint8_t s_u8LcdTskMsgQueue[sizeof(stTaskMsgBlock_t) * 32];
DefALLOCATE_BSS_DTCM alignas(32) static StaticQueue_t g_sqLcdTask;

DefALLOCATE_BSS_DTCM alignas(32) osMessageQueueId_t g_mqTouchScreenTask;
DefALLOCATE_BSS_DTCM alignas(32) static uint8_t s_u8TouchScreenTaskMsgQueue[sizeof(stTaskMsgBlock_t) * 32];
DefALLOCATE_BSS_DTCM alignas(32) static StaticQueue_t g_sqTouchScreenTask;

DefALLOCATE_BSS_DTCM alignas(32) osMessageQueueId_t g_mqPlayCtrlTask;
DefALLOCATE_BSS_DTCM alignas(32) static uint8_t s_u8PlayCtrlTaskMsgQueue[sizeof(stTaskMsgBlock_t) * 32];
DefALLOCATE_BSS_DTCM alignas(32) static StaticQueue_t g_sqPlayCtrlTask;

DefALLOCATE_BSS_DTCM alignas(32) osMessageQueueId_t g_mqSoundTask[enNumOfSoundTask];
DefALLOCATE_BSS_DTCM alignas(32) static uint8_t s_u8SoundTaskMsgQueue[enNumOfSoundTask][sizeof(stTaskMsgBlock_t) * 32];
DefALLOCATE_BSS_DTCM alignas(32) static StaticQueue_t g_sqSoundTask[enNumOfSoundTask];

static stQueueTable_t s_stQueueTable[] = {
	{&g_mqLcdTask, 32, sizeof(stTaskMsgBlock_t), {"MQLcdTask", 0, &g_sqLcdTask, sizeof(StaticQueue_t), s_u8LcdTskMsgQueue, sizeof(s_u8LcdTskMsgQueue)}},
	{&g_mqTouchScreenTask, 32, sizeof(stTaskMsgBlock_t), {"MQTouchScreenTask", 0, &g_sqTouchScreenTask, sizeof(StaticQueue_t), s_u8TouchScreenTaskMsgQueue, sizeof(s_u8TouchScreenTaskMsgQueue)}},
	{&g_mqPlayCtrlTask, 32, sizeof(stTaskMsgBlock_t), {"MQPlayCtrlTask", 0, &g_sqPlayCtrlTask, sizeof(StaticQueue_t), s_u8PlayCtrlTaskMsgQueue, sizeof(s_u8PlayCtrlTaskMsgQueue)}},
	{&g_mqSoundTask[enSoundTask1], 32, sizeof(stTaskMsgBlock_t), {"MQSoundTask1", 0, &g_sqSoundTask[enSoundTask1], sizeof(StaticQueue_t), s_u8SoundTaskMsgQueue[enSoundTask1], sizeof(stTaskMsgBlock_t) * 32}},
	{&g_mqSoundTask[enSoundTask2], 32, sizeof(stTaskMsgBlock_t), {"MQSoundTask1", 0, &g_sqSoundTask[enSoundTask2], sizeof(StaticQueue_t), s_u8SoundTaskMsgQueue[enSoundTask2], sizeof(stTaskMsgBlock_t) * 32}},
	{NULL, 0, 0, {0}},
};


void CreateMsgQueue(void){
	uint32_t i=0;

	while(s_stQueueTable[i].pID != NULL){
		*s_stQueueTable[i].pID = osMessageQueueNew(s_stQueueTable[i].u32BlockNum, s_stQueueTable[i].u32SizeofBlock, &s_stQueueTable[i].stAttr);
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
