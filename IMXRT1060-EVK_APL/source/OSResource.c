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
#include "Task/SensorTask/SensorTask.h"
#include "Task/CameraTask/CameraTask.h"
#include "Task/LcdTask/LcdTask.h"
#include "Task/TouchScreenTask/TouchScreenTask.h"
#include "Task/TempMoniTask/TempMoniTask.h"
#include "Task/SoundTask/SoundTask.h"
#include "Task/PlayCtrl/PlayCtrl.h"
#include "Task/ExtLedCtrlTask/ExtLedCtrlTask.h"

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
OS_RESOURCE_MACRO_TASK_DEFINE(ExtLedCtrlTask, 4096);

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
	OS_RESOURCE_MACRO_TASK_TABLE(ExtLedCtrlTask, NULL, osPriorityBelowNormal),

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
OS_RESOURCE_MACRO_MSGQUEUE_DEFINE(StorageTask[enNumOfSD], sizeof(stTaskMsgBlock_t), 32);
OS_RESOURCE_MACRO_MSGQUEUE_DEFINE(LanTask, sizeof(stTaskMsgBlock_t), 32);
OS_RESOURCE_MACRO_MSGQUEUE_DEFINE(CameraTask, sizeof(stTaskMsgBlock_t), 32);
OS_RESOURCE_MACRO_MSGQUEUE_DECLAR(ExtLedCtrlTask, sizeof(stTaskMsgBlock_t), 32);

static stMsgQueueTable_t s_stMsgQueueTable[] = {
	OS_RESOURCE_MACRO_MSGQUEUE_TABLE(LcdTask, sizeof(stTaskMsgBlock_t), 32),
	OS_RESOURCE_MACRO_MSGQUEUE_TABLE(TouchScreenTask, sizeof(stTaskMsgBlock_t), 32),
	OS_RESOURCE_MACRO_MSGQUEUE_TABLE(PlayCtrl, sizeof(stTaskMsgBlock_t), 32),
	OS_RESOURCE_MACRO_MSGQUEUE_TABLE(SoundTask[enSoundTask1], sizeof(stTaskMsgBlock_t), 32),
	OS_RESOURCE_MACRO_MSGQUEUE_TABLE(SoundTask[enSoundTask2], sizeof(stTaskMsgBlock_t), 32),
	OS_RESOURCE_MACRO_MSGQUEUE_TABLE(StorageTask[enUSDHC1], sizeof(stTaskMsgBlock_t), 32),
	OS_RESOURCE_MACRO_MSGQUEUE_TABLE(StorageTask[enUSDHC2], sizeof(stTaskMsgBlock_t), 32),
	OS_RESOURCE_MACRO_MSGQUEUE_TABLE(LanTask, sizeof(stTaskMsgBlock_t), 32),
	OS_RESOURCE_MACRO_MSGQUEUE_TABLE(CameraTask, sizeof(stTaskMsgBlock_t), 32),
	OS_RESOURCE_MACRO_MSGQUEUE_TABLE(ExtLedCtrlTask, sizeof(stTaskMsgBlock_t), 32),
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

OS_RESOURCE_MACRO_STREAM_DEFINE(LPUARTTx[1+enLPUART_MAX], 1024);
OS_RESOURCE_MACRO_STREAM_DEFINE(LPUARTRx[1+enLPUART_MAX], 1024);




static stStreamBuffer_t s_stStreamBufferTable[] = {
	OS_RESOURCE_MACRO_STREAM_TABLE(LPUARTTx[enLPUART1], 1024, sizeof(TCHAR)),
	OS_RESOURCE_MACRO_STREAM_TABLE(LPUARTTx[enLPUART2], 1024, sizeof(TCHAR)),
	OS_RESOURCE_MACRO_STREAM_TABLE(LPUARTTx[enLPUART3], 1024, sizeof(TCHAR)),
	OS_RESOURCE_MACRO_STREAM_TABLE(LPUARTTx[enLPUART4], 1024, sizeof(TCHAR)),
	OS_RESOURCE_MACRO_STREAM_TABLE(LPUARTTx[enLPUART5], 1024, sizeof(TCHAR)),
	OS_RESOURCE_MACRO_STREAM_TABLE(LPUARTTx[enLPUART6], 1024, sizeof(TCHAR)),
	OS_RESOURCE_MACRO_STREAM_TABLE(LPUARTTx[enLPUART7], 1024, sizeof(TCHAR)),
	OS_RESOURCE_MACRO_STREAM_TABLE(LPUARTTx[enLPUART8], 1024, sizeof(TCHAR)),
	OS_RESOURCE_MACRO_STREAM_TABLE(LPUARTRx[enLPUART1], 1024, sizeof(TCHAR)),
	OS_RESOURCE_MACRO_STREAM_TABLE(LPUARTRx[enLPUART2], 1024, sizeof(TCHAR)),
	OS_RESOURCE_MACRO_STREAM_TABLE(LPUARTRx[enLPUART3], 1024, sizeof(TCHAR)),
	OS_RESOURCE_MACRO_STREAM_TABLE(LPUARTRx[enLPUART4], 1024, sizeof(TCHAR)),
	OS_RESOURCE_MACRO_STREAM_TABLE(LPUARTRx[enLPUART5], 1024, sizeof(TCHAR)),
	OS_RESOURCE_MACRO_STREAM_TABLE(LPUARTRx[enLPUART6], 1024, sizeof(TCHAR)),
	OS_RESOURCE_MACRO_STREAM_TABLE(LPUARTRx[enLPUART7], 1024, sizeof(TCHAR)),
	OS_RESOURCE_MACRO_STREAM_TABLE(LPUARTRx[enLPUART8], 1024, sizeof(TCHAR)),

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
