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


/** 
 * @brief OSResource Macro : Task Declaration
 */
#define OS_RESOURCE_MACRO_TASK_DECLAR(TaskName)	\
extern osThreadId_t g_tid##TaskName

/** 
 * @brief OSResource Macro : Task Definition
 */
#define OS_RESOURCE_MACRO_TASK_DEFINE(TaskName, StackSize)	\
DefALLOCATE_BSS_DTCM alignas(4) osThreadId_t g_tid##TaskName;	\
DefALLOCATE_BSS_DTCM alignas(4) static StaticTask_t s_tcb##TaskName;	\
DefALLOCATE_BSS_DTCM alignas(4) static uint32_t s_stk##TaskName[(StackSize)/sizeof(uint32_t)]

/** 
 * @brief OSResource Macro : Task Table
 */
#define OS_RESOURCE_MACRO_TASK_TABLE(TaskName,  Argment, Priority)	\
	{&g_tid##TaskName, (osThreadFunc_t)TaskName,(Argment), {#TaskName, osThreadDetached, &s_tcb##TaskName, sizeof(s_tcb##TaskName), s_stk##TaskName, sizeof(s_stk##TaskName), (Priority), 0, 0}}


/** 
 * @brief OSResource Macro : EvenFlag Declaration
 */
#define OS_RESOURCE_MACRO_EVENT_DECLAR(EventName)	\
extern osEventFlagsId_t g_eid##EventName

/** 
 * @brief OSResource Macro : EvenFlag Definition
 */
#define OS_RESOURCE_MACRO_EVENT_DEFINE(EventName)	\
DefALLOCATE_BSS_DTCM alignas(4) osEventFlagsId_t g_eid##EventName;\
DefALLOCATE_BSS_DTCM alignas(4) static StaticEventGroup_t s_ef##EventName;

/** 
 * @brief OSResource Macro : EvenFlag Table
 */
#define OS_RESOURCE_MACRO_EVENT_TABLE(EventName) \
{&g_eid##EventName, {#EventName, 0, &s_ef##EventName, sizeof(StaticEventGroup_t)}}

/** 
 * @brief OSResource Macro : Semaphore Declaration
 */
#define OS_RESOURCE_MACRO_SEM_DECLAR(SemaphoreName)	\
extern osSemaphoreId_t g_semid##SemaphoreName

/** 
 * @brief OSResource Macro : Semaphore Definition
 */
#define OS_RESOURCE_MACRO_SEM_DEFINE(SemaphoreName)	\
DefALLOCATE_BSS_DTCM alignas(4) osSemaphoreId_t g_semid##SemaphoreName;\
DefALLOCATE_BSS_DTCM alignas(4) static StaticSemaphore_t s_sem##SemaphoreName

/** 
 * @brief OSResource Macro : Semaphore Table
 */
#define OS_RESOURCE_MACRO_SEM_TABLE(SemaphoreName, Max,Init)	\
{&g_semid##SemaphoreName, {#SemaphoreName, 0, &s_sem##SemaphoreName, sizeof(StaticSemaphore_t)}, (Max), (Init)}

/** 
 * @brief OSResource Macro : Message Queue Declaration
 */
#define OS_RESOURCE_MACRO_MSGQUEUE_DECLAR(MsgQueueName)	\
extern osMessageQueueId_t g_mqid##MsgQueueName;

/** 
 * @brief OSResource Macro : Message Queue Definition
 */
#define OS_RESOURCE_MACRO_MSGQUEUE_DEFINE(MsgQueueName, SizeOfMessage, NumOfMessages)	\
DefALLOCATE_BSS_DTCM alignas(4) osMessageQueueId_t g_mqid##MsgQueueName;\
DefALLOCATE_BSS_DTCM alignas(4) static uint8_t s_mqbuf##MsgQueueName[(SizeOfMessage) * (NumOfMessages)];\
DefALLOCATE_BSS_DTCM alignas(4) static StaticQueue_t s_mq##MsgQueueName

/** 
 * @brief OSResource Macro : Message Queue Table
 */
#define OS_RESOURCE_MACRO_MSGQUEUE_TABLE(MsgQueueName, SizeOfMessage, NumOfMessages)	\
{&g_mqid##MsgQueueName, (SizeOfMessage), (NumOfMessages), {#MsgQueueName, 0, &s_mq##MsgQueueName, sizeof(StaticQueue_t), s_mqbuf##MsgQueueName, sizeof(s_mqbuf##MsgQueueName)}}


/** 
 * @brief OSResource Macro : Stream Buffer Declaration
 */
#define OS_RESOURCE_MACRO_STREAM_DECLAR(StreamName)	\
extern StreamBufferHandle_t g_sbh##StreamName;

/** 
 * @brief OSResource Macro : Stream Buffer Definition
 */
#define OS_RESOURCE_MACRO_STREAM_DEFINE(StreamName, SizeOfStreamBuffer)	\
DefALLOCATE_BSS_DTCM alignas(4) StreamBufferHandle_t g_sbh##StreamName;\
DefALLOCATE_BSS_DTCM alignas(4) static uint8_t s_u8Storage##StreamName[(SizeOfStreamBuffer)+1];\
DefALLOCATE_BSS_DTCM alignas(4) static StaticStreamBuffer_t s_ssb##StreamName

/** 
 * @brief OSResource Macro : Stream Buffer Table
 */
#define OS_RESOURCE_MACRO_STREAM_TABLE(StreamName, SizeOfStreamBuffer, TriggerCount)	\
{&g_sbh##StreamName, (SizeOfStreamBuffer)+1, TriggerCount, s_u8Storage##StreamName, &s_ssb##StreamName}

/* Interrupt Priority */
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

/* for Protcol Stack */
#define TCPIP_MBOX_SIZE                 32
#define TCPIP_THREAD_STACKSIZE	        8192
#define TCPIP_THREAD_PRIO	            osPriorityBelowNormal



/* Task Handle */
OS_RESOURCE_MACRO_TASK_DECLAR(InitialTask);
OS_RESOURCE_MACRO_TASK_DECLAR(ConsoleTask);
OS_RESOURCE_MACRO_TASK_DECLAR(StorageTask);
OS_RESOURCE_MACRO_TASK_DECLAR(LanTask);
OS_RESOURCE_MACRO_TASK_DECLAR(LcdTask);
OS_RESOURCE_MACRO_TASK_DECLAR(TouchScreenTask);
OS_RESOURCE_MACRO_TASK_DECLAR(TempMoniTask);
OS_RESOURCE_MACRO_TASK_DECLAR(SoundTask);
OS_RESOURCE_MACRO_TASK_DECLAR(PlayCtrl);
OS_RESOURCE_MACRO_TASK_DECLAR(ExtLedCtrlTask);

/* Event Group */
OS_RESOURCE_MACRO_EVENT_DECLAR(LPUART[1+enLPUART_MAX]);
OS_RESOURCE_MACRO_EVENT_DECLAR(FSReady);
OS_RESOURCE_MACRO_EVENT_DECLAR(CameraSensor);
OS_RESOURCE_MACRO_EVENT_DECLAR(SAITx[enNumOfSAI]);
OS_RESOURCE_MACRO_EVENT_DECLAR(SAIRx[enNumOfSAI]);
OS_RESOURCE_MACRO_EVENT_DECLAR(PlayCtrl);
OS_RESOURCE_MACRO_EVENT_DECLAR(SoundTask[enNumOfSoundTask]);

/* Binary Semaphore */
OS_RESOURCE_MACRO_SEM_DECLAR(LPUARTRxSemaphore[1+enLPUART_MAX]);
OS_RESOURCE_MACRO_SEM_DECLAR(LPUARTTxSemaphore[1+enLPUART_MAX]);
OS_RESOURCE_MACRO_SEM_DECLAR(CameraTask);
OS_RESOURCE_MACRO_SEM_DECLAR(ComboSensor);
OS_RESOURCE_MACRO_SEM_DECLAR(WM8960);

/* Message Queue */
OS_RESOURCE_MACRO_MSGQUEUE_DECLAR(LcdTask);
OS_RESOURCE_MACRO_MSGQUEUE_DECLAR(TouchScreenTask);
OS_RESOURCE_MACRO_MSGQUEUE_DECLAR(PlayCtrl);
OS_RESOURCE_MACRO_MSGQUEUE_DECLAR(SoundTask[enNumOfSoundTask]);
OS_RESOURCE_MACRO_MSGQUEUE_DECLAR(StorageTask[enNumOfSD]);
OS_RESOURCE_MACRO_MSGQUEUE_DECLAR(LanTask);
OS_RESOURCE_MACRO_MSGQUEUE_DECLAR(CameraTask);
OS_RESOURCE_MACRO_MSGQUEUE_DECLAR(ExtLedCtrlTask);
OS_RESOURCE_MACRO_MSGQUEUE_DECLAR(MeterTask);

/* Stream Buffer */
OS_RESOURCE_MACRO_STREAM_DECLAR(LPUARTTx[1+enLPUART_MAX]);
OS_RESOURCE_MACRO_STREAM_DECLAR(LPUARTRx[1+enLPUART_MAX]);



extern void CreateTask(void);
extern void CreateMsgQueue(void);
extern void CreateBinarySemaphore(void);
extern void CreateEventGroup(void);
extern void CreateStreamBuffer(void);

#ifdef __cplusplus
}
#endif


