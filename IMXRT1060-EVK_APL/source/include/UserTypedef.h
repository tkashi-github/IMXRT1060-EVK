/**
 * @file UserTypedef.h
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
/** Standard Header */
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>

/* CMSIS */
#include "os_tick.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"

/* Amazon FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "timers.h"
#include "semphr.h"
#include "stream_buffer.h"
#include "queue.h"

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
	enCameraBtn,
	enTouchEvent,
	enCurEv,
	enSoundTaskInit,
	enSoundTaskStart,
	enSoundTaskSendAudioData,
	enSoundTaskRcvAudioData,
	enSoundTaskStop,
	enPlayStop,
	enPlayStart,
	enPlaying,
	enRec,
	enRecording,
	enExtLedUpdate,
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



/** SAI */
typedef enum{
	enSoundTask1 = 0,
	enSoundTask2,
	enNumOfSoundTask,
}enSoundTask_t;

/** SAI */
typedef enum{
	enSAI1 = 0,
	enSAI2,
	enNumOfSAI,
}enSAI_t;

/** SD Card Slots */
typedef enum{
	enUSDHC1 = 0,
	enUSDHC2,
	enNumOfSD,
}enSD_t;

typedef enum{
	enAudioFileWAV = 0,
	enAudioFileMAX,
}enAudioFileType_t;

/*! @brief Audio sample rate */
typedef enum{
	enSampleRateNone = 0,
    enSampleRate8KHz = 8000U,     /*!< Sample rate 8000 Hz */
    enSampleRate11025Hz = 11025U, /*!< Sample rate 11025 Hz */
    enSampleRate12KHz = 12000U,   /*!< Sample rate 12000 Hz */
    enSampleRate16KHz = 16000U,   /*!< Sample rate 16000 Hz */
    enSampleRate22050Hz = 22050U, /*!< Sample rate 22050 Hz */
    enSampleRate24KHz = 24000U,   /*!< Sample rate 24000 Hz */
    enSampleRate32KHz = 32000U,   /*!< Sample rate 32000 Hz */
    enSampleRate44100Hz = 44100U, /*!< Sample rate 44100 Hz */
    enSampleRate48KHz = 48000U,   /*!< Sample rate 48000 Hz */
    enSampleRate96KHz = 96000U    /*!< Sample rate 96000 Hz */
} enSampleRate_t;

/*! @brief Audio word width */
typedef enum{
	enWordWidthNone = 0,
    enWordWidth8bits = 8U,   /*!< Audio data width 8 bits */
    enWordWidth16bits = 16U, /*!< Audio data width 16 bits */
    enWordWidth24bits = 24U, /*!< Audio data width 24 bits */
    enWordWidth32bits = 32U  /*!< Audio data width 32 bits */
} enWorWidth_t;

typedef struct{
	enSampleRate_t   	enSample;
	enWorWidth_t    	enBitsWidth;
	enAudioFileType_t   enFileType;
	uint16_t            nChannels;
}stCodecCondition_t;


#define DEF_BUFFER_SAMPLE_SIZE		(128u)	/** 128 Sample */
#define DEF_BUFFER_QUEUE_SIZE		(64)
#define DEF_AUDIO_BUFFER_SIZE		(DEF_BUFFER_SAMPLE_SIZE * DEF_BUFFER_QUEUE_SIZE * sizeof(uint32_t))

#define SAI_XFER_QUEUE_SIZE 		DEF_BUFFER_QUEUE_SIZE		/* for FSL SDK */

/** for TCHAR */
#define _INC_TCHAR
typedef char TCHAR;
#define _T(x) u8 ## x
#define _TEXT(x) u8 ## x


#if 1
#define DefALLOCATE_ITCM  __attribute__((section(".ramfunc.$SRAM_ITC"))) 
#define DefALLOCATE_BSS_DTCM  __attribute__((section(".bss.$SRAM_DTC"))) 
#define DefALLOCATE_DATA_DTCM  __attribute__((section(".data.$SRAM_DTC"))) 
#else
#define DefALLOCATE_ITCM  
#define DefALLOCATE_BSS_DTCM 
#define DefALLOCATE_DATA_DTCM 
#endif

#ifdef __cplusplus
}
#endif

