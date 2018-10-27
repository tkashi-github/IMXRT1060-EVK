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
/** Standard Header */
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>

/* CMSIS */
#include "cmsis_os.h"
#include "cmsis_os2.h"

/* Amazon FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "timers.h"
#include "semphr.h"
#include "stream_buffer.h"

/*  */
#include "MIMXRT1062.h"

/** version.h */
#include "version.h"

/** time_t */
typedef uint64_t bsptime_t;

/** Function Pointer */
typedef void (*pfnVector_t)(void);

/** LPUART enum */
typedef enum{
	enLPUART_MIN = 1,
	enLPUART1=1,
	enLPUART2,
	enLPUART3,
	enLPUART4,
	enLPUART5,
	enLPUART6,
	enLPUART7,
	enLPUART8,
	enLPUART_MAX = enLPUART8,
}enLPUART_t;

/** Task Message */
typedef enum{
	enNotify = 0,
	enSDInsterted,
	enSDRemoved,
	enStorageIsReady,
	enLanLinkChange,
	enLanRestart,
	enMsgID_MAX,
}enMsgID_t;

/** Task Message Block */
typedef struct{
	enMsgID_t          enMsgId;
	osEventFlagsId_t   SyncEGHandle;
	uint32_t           wakeupbits;
	uintptr_t          ptrDataForSrc;
	uintptr_t          ptrDataForDst;
	uint32_t           param[3];
}stTaskMsgBlock_t;

/** SD Card Slots */
typedef enum{
	enUSDHC1 = 0,
	enUSDHC2,
	enNumOfSD,
}enSD_t;

/** for TCHAR */
#define _INC_TCHAR
typedef char TCHAR;
#define _T(x) u8 ## x
#define _TEXT(x) u8 ## x

#define DefALLOCATE_ITCM  __attribute__((section(".ramfunc.$SRAM_ITC"))) 
#define DefALLOCATE_BSS_DTCM  __attribute__((section(".bss.$SRAM_DTC"))) 
#define DefALLOCATE_DATA_DTCM  __attribute__((section(".data.$SRAM_DTC"))) 

#ifdef __cplusplus
}
#endif

