/**
 * @file TODO
 * @brief TODO
 * @author Takashi Kashiwagi
 * @date        2018/10/23
 * @version     0.1
 * @copyright   TODO
 * 
 * @par Update:
 * - 2018/10/23: Takashi Kashiwagi: for IMXRT1060-EVK
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
#define kIRQ_PRIORITY_LPUART    (10u)
#define kIRQ_PRIORITY_USDHC     (9u)

/** Task Handle */
extern osThreadId_t g_InitialTaskHandle;
extern osThreadId_t g_ConsoleTaskHandle;
extern osThreadId_t g_LEDTaskHandle;

/** Event Group */
extern EventGroupHandle_t g_xLPUARTEventGroup[];
extern EventGroupHandle_t g_xFSReadyEventGroup;
extern SemaphoreHandle_t g_xLPUARTRxSemaphore[];
extern SemaphoreHandle_t g_xLPUARTTxSemaphore[];

/** Queue */
extern osMessageQueueId_t g_StorageTaskQueueId;


/** Stream Buffer */
extern StreamBufferHandle_t g_sbhStorageTask[enNumOfSD];
extern StreamBufferHandle_t g_sbhUpdateTask;

extern void CreateTask(void);
extern void CreateQueue(void);
extern void CreateBinarySemaphore(void);
extern void CreateEventGroup(void);
extern void CreateStreamBuffer(void);

#ifdef __cplusplus
}
#endif


