/**
 * @file OSResource.h
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
#ifndef __cplusplus
#if __STDC_VERSION__ < 201112L
#error /** Only C11 */
#endif
#endif
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

/** User Typedefine */
#include "UserTypedef.h"

/** Interrupt Priority */
#define kIRQ_PRIORITY_LPUART    (14u)
#define kIRQ_PRIORITY_LPI2C     (13u)
#define kIRQ_PRIORITY_GPIO      (12u)
#define kIRQ_PRIORITY_LCDIF    	(11u)
#define kIRQ_PRIORITY_ENET    	(10u)
#define kIRQ_PRIORITY_USDHC     (9u)
#define kIRQ_PRIORITY_SAI     	(8u)
#define kIRQ_PRIORITY_DMA0     	(6u)	/** Highest */

#define ENET_PRIORITY       kIRQ_PRIORITY_ENET
#define ENET_1588_PRIORITY  kIRQ_PRIORITY_ENET

/** for Protcol Stack */
#define TCPIP_MBOX_SIZE                 32
#define TCPIP_THREAD_STACKSIZE	        8192
#define TCPIP_THREAD_PRIO	            osPriorityBelowNormal

/** Task Handle */
extern osThreadId_t g_InitialTaskHandle;
extern osThreadId_t g_ConsoleTaskHandle;
extern osThreadId_t g_LEDTaskHandle;
extern osThreadId_t g_LanTaskHandle;
extern osThreadId_t g_CameraTaskHandle;

/** Event Group */
extern osEventFlagsId_t g_efLPUART[];
extern osEventFlagsId_t g_efFSReady;
extern osEventFlagsId_t g_efCameraSensor;

/** Binary Semaphore */
extern osSemaphoreId_t g_bsIdLPUARTRxSemaphore[];
extern osSemaphoreId_t g_bsIdLPUARTTxSemaphore[];
extern osSemaphoreId_t g_bsIdStorageTaskMsg;
extern osSemaphoreId_t g_bsIdComboSensor;
extern osSemaphoreId_t g_bsIdCameraTask;

/** Queue */
extern osMessageQueueId_t g_StorageTaskQueueId;
extern QueueHandle_t g_mqLcdTask;

/** Stream Buffer */
extern StreamBufferHandle_t g_sbhLPUARTTx[1+enLPUART_MAX];
extern StreamBufferHandle_t g_sbhLPUARTRx[1+enLPUART_MAX];
extern StreamBufferHandle_t g_sbhStorageTask[enNumOfSD];
extern StreamBufferHandle_t g_sbhUpdateTask;
extern StreamBufferHandle_t g_sbhLanTask;
extern StreamBufferHandle_t g_sbhCameraTask;
extern StreamBufferHandle_t g_sbhTouchScreenTask;

extern void CreateTask(void);
extern void CreateQueue(void);
extern void CreateBinarySemaphore(void);
extern void CreateEventGroup(void);
extern void CreateStreamBuffer(void);
extern void CreateMsgQueue(void);

#ifdef __cplusplus
}
#endif


