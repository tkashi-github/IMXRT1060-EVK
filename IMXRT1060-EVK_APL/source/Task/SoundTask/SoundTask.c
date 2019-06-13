/**
 * @brief		Sound Device Control Task
 * @author		Takashi Kashiwagi
 * @date		2019/06/13
 * @version     0.1
 * @details 
 * --
 * License Type <MIT License>
 * --
 * Copyright 2019 Takashi Kashiwagi
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
 * - 2019/06/13: Takashi Kashiwagi: v0.1 for IMXRT1060-EVK
 */
#include "SoundTask/SoundTask.h"
#include "board.h"
#include "mimiclib/mimiclib.h"
#include "ff.h"
#include "common/common.h"
#include "AudioFile/wav/wav.h"


typedef struct
{
	uint8_t *pu8;
	uint32_t u32ByteCnt; /** Array Size or Vallid Data Size */
	uint32_t u32RcvCnt;
} stu8DataPtr_t;

typedef struct
{
	sai_sample_rate_t enSample;
	sai_word_width_t enWidth;
	_Bool bRec;
} stSoundFormat_t;

typedef enum
{
	enSoundTaskStateNoInit = 0,
	enSoundTaskStateStop,
	enSoundTaskStatePlay,
	enSoundTaskStateMAX,
} enSoundTaskState_t;

typedef enum
{
	enSoundTaskEventInit = 0,
	enSoundTaskEventStart,
	enSoundTaskEventPlayData,
	enSoundTaskEventStop,
	enSoundTaskEventRecData,
	enSoundTaskEventMAX,
} enSoundTaskEvent_t;

/** Protected Member */
DefALLOCATE_DATA_DTCM static enSoundTaskState_t s_enSoundTaskState[enNumOfSAI] = {
	enSoundTaskStateNoInit,
	enSoundTaskStateNoInit,
	//enSoundTaskStateNoInit,
};
DefALLOCATE_DATA_DTCM static sai_sample_rate_t s_enSampleRate[enNumOfSAI] = {
	kSAI_SampleRateNone,
	kSAI_SampleRateNone,
	//kSAI_SampleRateNone,
};
DefALLOCATE_DATA_DTCM static sai_word_width_t s_enWordWidth[enNumOfSAI] = {
	kSAI_WordWidthNone,
	kSAI_WordWidthNone,
	//kSAI_WordWidthNone,
};

DefALLOCATE_DATA_DTCM static uint32_t s_u32Volume[enNumOfSAI] = {50, 50};

/** State Machine Functions */
typedef void (*pfnSoundTaskMatrix_t)(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);

DefALLOCATE_ITCM static void SoundTaskDoMatrix(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S0_E0(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S1_E1(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S1_E3(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S2_E2(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S2_E3(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S2_E4(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void XXXXX(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);

DefALLOCATE_DATA_DTCM static pfnSoundTaskMatrix_t s_pfnSoundTaskMatrix[enSoundTaskStateMAX][enSoundTaskEventMAX] = {
	/** Init  | Start| Play | Stop | Rec   */
	{S0_E0, XXXXX, XXXXX, XXXXX, XXXXX}, /** enSoundTaskStateNoInit */
	{S0_E0, S1_E1, XXXXX, S1_E3, XXXXX}, /** enSoundTaskStateStop */
	{S0_E0, XXXXX, S2_E2, S2_E3, S2_E4}, /** enSoundTaskStatePlay */
};

/**
 * @brief Do Matrix
 * @param [in]  enSAI instanceNo
 * @param [in]  enEvent Enevt Code
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void SoundTaskDoMatrix(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr)
{
	enSoundTaskState_t enState;

	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		mimic_printf("[%s (%d)] Unkown Instance No!!(%d)\r\n", __FUNCTION__, __LINE__, enSAI);
		return;
	}
	if ((enEvent < enSoundTaskEventInit) || (enEvent >= enSoundTaskEventMAX))
	{
		mimic_printf("[%s (%d)] Unkown Event No!!(%d)\r\n", __FUNCTION__, __LINE__, enEvent);
		return;
	}
	enState = s_enSoundTaskState[enSAI];
	if (s_pfnSoundTaskMatrix[enState][enEvent] != NULL)
	{
		s_pfnSoundTaskMatrix[enState][enEvent](enSAI, enEvent, u32param, inptr, outptr);
	}
}

/**
 * @brief Nop
 * @param [in]  enSAI instanceNo
 * @param [in]  enEvent Enevt Code
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void XXXXX(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr)
{
	if ((enEvent == enSoundTaskEventStart) && (s_enSoundTaskState[enSAI] == enSoundTaskStatePlay))
	{
		mimic_printf("Sound Task is Running\r\n");
	}
	else
	{
		mimic_printf("[%s (%d)] Nop!(S%d_E%d)\r\n", __FUNCTION__, __LINE__, s_enSoundTaskState[enSAI], enEvent);
	}
}

/**
 * @brief S0_E0
 * @param [in]  enSAI instanceNo
 * @param [in]  enEvent Enevt Code
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S0_E0(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr)
{
	stSoundFormat_t *pstSoundFormat;
	pstSoundFormat = (stSoundFormat_t *)inptr;
	if (enSoundTaskEventInit != enEvent)
	{
		return;
	}

	while (DrvSAIIsTxBufferEmpty(enSAI) != false)
	{
		/* NOP */
	}
	if (DrvSAIInit(enSAI, pstSoundFormat->enSample, pstSoundFormat->enWidth, pstSoundFormat->bRec) == false)
	{
		mimic_printf("[%s (%d)] DrvSAIInit NG (SAI%d)!\r\n", __FUNCTION__, __LINE__, enSAI);
	}
	else
	{
		uint32_t u32CurVol = 0;
		while (u32CurVol < s_u32Volume[enSAI])
		{
			u32CurVol++;
			SoundTaskWriteCurrentVolume(enSAI1, u32CurVol);
			vTaskDelay(2);
		}
		SoundTaskWriteCurrentVolume(enSAI1, s_u32Volume[enSAI]);
		DrvSAITxReset(enSAI);
		DrvSAIRxReset(enSAI);

		s_enSampleRate[enSAI] = pstSoundFormat->enSample;
		s_enWordWidth[enSAI] = pstSoundFormat->enWidth;
		s_enSoundTaskState[enSAI] = enSoundTaskStateStop;
		//mimic_printf("[%s (%d)] MOV !! s_enSoundTaskState[%d] = %d\r\n", __FUNCTION__, __LINE__, enSAI, s_enSoundTaskState[enSAI]);
	}
}

/**
 * @brief S1_E1
 * @param [in]  enSAI instanceNo
 * @param [in]  enEvent Enevt Code
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S1_E1(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr)
{
	if (enSoundTaskEventStart == enEvent)
	{
		uint32_t u32CurVol = 0;
		while (u32CurVol < s_u32Volume[enSAI])
		{
			u32CurVol++;
			SoundTaskWriteCurrentVolume(enSAI1, u32CurVol);
			vTaskDelay(2);
		}
		SoundTaskWriteCurrentVolume(enSAI, s_u32Volume[enSAI]);
		//mimic_printf("[%s (%d)] Now Play Start (SAI%d)!\r\n", __FUNCTION__, __LINE__, enSAI);
		s_enSoundTaskState[enSAI] = enSoundTaskStatePlay;
		//mimic_printf("[%s (%d)] MOV !! s_enSoundTaskState[%d] = %d\r\n", __FUNCTION__, __LINE__, enSAI, s_enSoundTaskState[enSAI]);
	}
}

/**
 * @brief S1_E1
 * @param [in]  enSAI instanceNo
 * @param [in]  enEvent Enevt Code
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S1_E3(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr)
{
	if (enSoundTaskEventStop != enEvent)
	{
		mimic_printf("[%s (%d)] Unexpected Event No!!(%d)\r\n", __FUNCTION__, __LINE__, enEvent);
		return;
	}
	mimic_printf("[%s (%d)] SoundTask is already Stopped.\r\n", __FUNCTION__, __LINE__);
}

/**
 * @brief S2_E2
 * @param [in]  enSAI instanceNo
 * @param [in]  enEvent Enevt Code
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S2_E2(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr)
{
	if (enSoundTaskEventPlayData == enEvent)
	{
		stu8DataPtr_t *pst;
		_Bool *pBool = (_Bool *)outptr;
		pst = (stu8DataPtr_t *)inptr;

		if (DrvSAITx(enSAI, pst->pu8, pst->u32ByteCnt) == false)
		{
			mimic_printf("[%s (%d)] DrvSAITx Stop\r\n", __FUNCTION__, __LINE__);
			*pBool = false;

			/** STOPへ */
			s_enSoundTaskState[enSAI] = enSoundTaskStateStop;
			DrvSAITxReset(enSAI);
			DrvSAIRxReset(enSAI);
		}
		else
		{
			*pBool = true;
		}
	}
}

/**
 * @brief S2_E3
 * @param [in]  enSAI instanceNo
 * @param [in]  enEvent Enevt Code
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S2_E3(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr)
{
	if (enSoundTaskEventStop != enEvent)
	{
		mimic_printf("[%s (%d)] Unexpected Event No!!(%d)\r\n", __FUNCTION__, __LINE__, enEvent);
		return;
	}

	mimic_printf("[%s (%d)] Now Stop (SAI%d)! by <%s (%d)>\r\n", __FUNCTION__, __LINE__, enSAI, (char *)u32param[0], u32param[1]);
	s_enSoundTaskState[enSAI] = enSoundTaskStateStop;
	DrvSAITxReset(enSAI);
	DrvSAIRxReset(enSAI);
}

/**
 * @brief S2_E4
 * @param [in]  enSAI instanceNo
 * @param [in]  enEvent Enevt Code
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S2_E4(enSAI_t enSAI, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr)
{
	stu8DataPtr_t *pst;
	_Bool *pBool = (_Bool *)outptr;
	pst = (stu8DataPtr_t *)inptr;

	if (enSoundTaskEventRecData != enEvent)
	{
		mimic_printf("[%s (%d)] Unexpected Event No!!(%d)\r\n", __FUNCTION__, __LINE__, enEvent);
		return;
	}

	if (DrvSAIRx(enSAI, pst->pu8, &pst->u32RcvCnt) == false)
	{
		mimic_printf("[%s (%d)] DrvSAIRx Stop\r\n", __FUNCTION__, __LINE__);
		*pBool = false;
		/** STOPへ */
		s_enSoundTaskState[enSAI] = enSoundTaskStateStop;
		DrvSAITxReset(enSAI);
		DrvSAIRxReset(enSAI);
	}
	else
	{
		*pBool = true;
	}
}

/**
 * @brief Task Main Loop
 * @param [in]  enSAI Instance Number
 * @return void
 */
DefALLOCATE_ITCM static void SoundTaskActual(enSAI_t enSAI)
{
	stTaskMsgBlock_t stTaskMsg = {0};
	if (sizeof(stTaskMsg) <= xStreamBufferReceive(g_sbhSoundTask[enSAI], &stTaskMsg, sizeof(stTaskMsg), portMAX_DELAY))
	{
		switch (stTaskMsg.enMsgId)
		{
		case enSAIInit:
			SoundTaskDoMatrix(enSAI, enSoundTaskEventInit, stTaskMsg.param, stTaskMsg.ptrDataForDst, stTaskMsg.ptrDataForSrc);
			break;
		case enSAIStart:
			SoundTaskDoMatrix(enSAI, enSoundTaskEventStart, stTaskMsg.param, stTaskMsg.ptrDataForDst, stTaskMsg.ptrDataForSrc);
			break;
		case enSAISendAudioData:
			SoundTaskDoMatrix(enSAI, enSoundTaskEventPlayData, stTaskMsg.param, stTaskMsg.ptrDataForDst, stTaskMsg.ptrDataForSrc);
			break;
		case enSAIStop:
			SoundTaskDoMatrix(enSAI, enSoundTaskEventStop, stTaskMsg.param, stTaskMsg.ptrDataForDst, stTaskMsg.ptrDataForSrc);
			break;
		case enSAIRcvAudioData:
			SoundTaskDoMatrix(enSAI, enSoundTaskEventRecData, stTaskMsg.param, stTaskMsg.ptrDataForDst, stTaskMsg.ptrDataForSrc);
			break;
		default:
			mimic_printf("[%s (%d)] Unkown Msg\r\n", __FUNCTION__, __LINE__);
			break;
		}

		/** Sync */
		if (stTaskMsg.SyncEGHandle != NULL)
		{
			osEventFlagsSet(stTaskMsg.SyncEGHandle, stTaskMsg.wakeupbits);
		}

		/** Free */
		if (stTaskMsg.ptrDataForDst != (uintptr_t)NULL)
		{
			vPortFree((void *)stTaskMsg.ptrDataForDst);
		}
	}
}

/**
 * @brief Task Entry
 * @param [in]  argument nouse
 * @return void
 */
DefALLOCATE_ITCM void SoundTask(void const *argument)
{
	enSAI_t enSAI = (enSAI_t)argument;

	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		vTaskSuspend(NULL);
	}

	DebugTraceX("DEBUG", dbgMINOR, "Start");

	DrvSAIInit(enSAI, kSAI_SampleRate44100Hz, kSAI_WordWidth16bits, false);
	s_u32Volume[enSAI] = 50;
	SoundTaskWriteCurrentVolume(enSAI, 0);
	mimic_printf("Volume = %lu\r\n", SoundTaskGetCurrentVolume(enSAI1));
	for (;;)
	{
		SoundTaskActual(enSAI);
	}

	vTaskSuspend(NULL);
}

/**
 * @brief Post Message (enSAIInit)
 * @param [in]  enSAI instance Number
 * @param [in]  enSample SampleRate
 * @param [in]  enWidth BitsWidth
 * @param [in]  bRec Recording Flag
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool PostSyncMsgSoundTaskDeviceInit(enSAI_t enSAI, sai_sample_rate_t enSample, sai_word_width_t enWidth, _Bool bRec)
{
	/** var */
	_Bool bret = true;

	/** begin */
	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return false;
	}

	if (osSemaphoreAcquire(g_bsidPostMsgSoundTask, 10) == osOK)
	{
		alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
		stSoundFormat_t *pstSoundFormat;
		stTaskMsg.enMsgId = enSAIInit;
		stTaskMsg.SyncEGHandle = g_efMsgSoundTask[enSAI];
		stTaskMsg.wakeupbits = 1;
		pstSoundFormat = pvPortMalloc(sizeof(stSoundFormat_t));

		if (pstSoundFormat != NULL)
		{
			stTaskMsg.ptrDataForDst = (uintptr_t)pstSoundFormat;
			pstSoundFormat->enSample = enSample;
			pstSoundFormat->enWidth = enWidth;
			pstSoundFormat->bRec = bRec;

			if (sizeof(stTaskMsg) != xStreamBufferSend(g_sbhSoundTask[enSAI], &stTaskMsg, sizeof(stTaskMsg), 50))
			{
				mimic_printf("[%s (%d)] xStreamBufferSend NG\r\n", __FUNCTION__, __LINE__);
				bret = false;
			}
			/** Sync */
			uint32_t uxBits = osEventFlagsWait(g_efMsgSoundTask[enSAI], 1, osFlagsWaitAny, 500);
			if (uxBits != 1u)
			{
				mimic_printf("[%s (%d)] osEventFlagsWait NG\r\n", __FUNCTION__, __LINE__);
				bret = false;
			}
		}
		osSemaphoreRelease(g_bsidPostMsgSoundTask);
	}
	else
	{
		bret = false;
	}
	return bret;
}

/**
 * @brief Post Message (enSAIStart)
 * @param [in]  enSAI instance Number
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool PostMsgSoundTaskDeviceStart(enSAI_t enSAI)
{
	/** var */
	_Bool bret = true;

	/** begin */
	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return false;
	}

	if (osSemaphoreAcquire(g_bsidPostMsgSoundTask, 10) == osOK)
	{
		alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
		stTaskMsg.enMsgId = enSAIStart;

		if (sizeof(stTaskMsg) != xStreamBufferSend(g_sbhSoundTask[enSAI], &stTaskMsg, sizeof(stTaskMsg), 50))
		{
			mimic_printf("[%s (%d)] xStreamBufferSend NG\r\n", __FUNCTION__, __LINE__);
			bret = false;
		}
		osSemaphoreRelease(g_bsidPostMsgSoundTask);
	}
	else
	{
		bret = false;
	}
	return bret;
}

/**
 * @brief Post Sync Message (enSAISendAudioData)
 * @param [in]  enSAI instance Number
 * @param [in]  pu8 Pointer of PCM data
 * @param [in]  u32ByteCnt size of PCM data
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool PostMsgSoundTaskSendAudioData(enSAI_t enSAI, uint8_t pu8[], uint32_t u32ByteCnt)
{
	/** var */
	_Bool bret = true;

	/** begin */
	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return false;
	}
	if (pu8 == NULL)
	{
		return false;
	}

	if (osSemaphoreAcquire(g_bsidPostMsgSoundTask, 10) == osOK)
	{
		alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
		stu8DataPtr_t *pstBuf;
		stTaskMsg.enMsgId = enSAISendAudioData;
		pstBuf = pvPortMalloc(sizeof(stu8DataPtr_t));

		if (pstBuf != NULL)
		{
			uint32_t uxBits;

			stTaskMsg.ptrDataForDst = (uintptr_t)pstBuf;
			stTaskMsg.ptrDataForSrc = (uintptr_t)&bret;
			stTaskMsg.SyncEGHandle = g_efSoundTaskEventGroup[enSAI];
			stTaskMsg.wakeupbits = 1;
			pstBuf->pu8 = pu8;
			pstBuf->u32ByteCnt = u32ByteCnt;

			if (sizeof(stTaskMsg) != xStreamBufferSend(g_sbhSoundTask[enSAI], &stTaskMsg, sizeof(stTaskMsg), 50))
			{
				mimic_printf("[%s (%d)] xStreamBufferSend NG\r\n", __FUNCTION__, __LINE__);
				bret = false;
			}
			else
			{
				/** Sync */
				uxBits = osEventFlagsWait(g_efSoundTaskEventGroup[enSAI], 1, osFlagsWaitAny, 10000);
				if (uxBits != 1u)
				{
					mimic_printf("[%s (%d)] osEventFlagsWait NG\r\n", __FUNCTION__, __LINE__);
					bret = false;
				}
			}
		}
		osSemaphoreRelease(g_bsidPostMsgSoundTask);
	}
	else
	{
		bret = false;
	}
	return bret;
}

/**
 * @brief Post Sync Message (enSAIRcvAudioData)
 * @param [in]  enSAI instance Number
 * @param [in]  pu8 Pointer of PCM Buffer
 * @param [in]  u32ByteCnt size of PCM Buffer
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool PostMsgSoundTaskRcvAudioData(enSAI_t enSAI, uint8_t pu8[], uint32_t *pu32br)
{
	/** var */
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
	stu8DataPtr_t *pstBuf;
	_Bool bret = true;
	/** begin */
	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return false;
	}
	if (pu8 == NULL)
	{
		return false;
	}

	if (osSemaphoreAcquire(g_bsidPostMsgSoundTask, 10) == osOK)
	{
		stTaskMsg.enMsgId = enSAIRcvAudioData;
		pstBuf = pvPortMalloc(sizeof(stu8DataPtr_t));

		if (pstBuf != NULL)
		{
			uint32_t uxBits;

			stTaskMsg.ptrDataForDst = (uintptr_t)pstBuf;
			stTaskMsg.ptrDataForSrc = (uintptr_t)&bret;
			stTaskMsg.SyncEGHandle = g_efSoundTaskEventGroup[enSAI];
			stTaskMsg.wakeupbits = 1;
			pstBuf->pu8 = pu8;
			pstBuf->u32ByteCnt = 0;
			pstBuf->u32RcvCnt = 0;

			if (sizeof(stTaskMsg) != xStreamBufferSend(g_sbhSoundTask[enSAI], &stTaskMsg, sizeof(stTaskMsg), 50))
			{
				mimic_printf("[%s (%d)] xStreamBufferSend NG\r\n", __FUNCTION__, __LINE__);
				bret = false;
			}
			else
			{
				/** Sync */
				uxBits = osEventFlagsWait(g_efSoundTaskEventGroup[enSAI], 1, osFlagsWaitAny, 10000);
				if (uxBits != 1u)
				{
					mimic_printf("[%s (%d)] osEventFlagsWait NG (%d)\r\n", __FUNCTION__, __LINE__, uxBits);
					bret = false;
				}
				else
				{
					*pu32br = pstBuf->u32RcvCnt;
				}
			}
		}
		osSemaphoreRelease(g_bsidPostMsgSoundTask);
	}
	else
	{
		bret = false;
	}
	return bret;
}

/**
 * @brief Post Message (enSAIStop)
 * @param [in]  enSAI instance Number
 * @param [in]  pu8 Pointer of PCM Buffer
 * @param [in]  u32ByteCnt size of PCM Buffer
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool PostMsgSoundTaskStop(enSAI_t enSAI, const char *pszStr, uint32_t u32Line)
{
	/** var */
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
	_Bool bret = true;
	/** begin */
	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return false;
	}

	stTaskMsg.enMsgId = enSAIStop;
	stTaskMsg.param[0] = (uint32_t)pszStr;
	stTaskMsg.param[1] = u32Line;

	if (pdFALSE == xPortIsInsideInterrupt())
	{
		if (osSemaphoreAcquire(g_bsidPostMsgSoundTask, 10) == osOK)
		{
			if (sizeof(stTaskMsg) != xStreamBufferSend(g_sbhSoundTask[enSAI], &stTaskMsg, sizeof(stTaskMsg), 50))
			{
				mimic_printf("[%s (%d)] xStreamBufferSend NG\r\n", __FUNCTION__, __LINE__);
				bret = false;
			}
			osSemaphoreRelease(g_bsidPostMsgSoundTask);
		}
		else
		{
			bret = false;
		}
	}
	else
	{
		BaseType_t xHigherPriorityTaskWoken;
		if (osSemaphoreAcquire(g_bsidPostMsgSoundTask, 0) == osOK)
		{
			if (sizeof(stTaskMsg) != xStreamBufferSendFromISR(g_sbhSoundTask[enSAI], &stTaskMsg, sizeof(stTaskMsg), &xHigherPriorityTaskWoken))
			{
				bret = false;
			}
			osSemaphoreRelease(g_bsidPostMsgSoundTask);
		}
		else
		{
			bret = false;
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}

	return bret;
}
DefALLOCATE_ITCM static _Bool PostSyncMsgSoundTaskStop(enSAI_t enSAI, const char *pszStr, uint32_t u32Line)
{
	/** var */
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
	_Bool bret = true;
	/** begin */
	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return false;
	}

	stTaskMsg.enMsgId = enSAIStop;
	stTaskMsg.param[0] = (uint32_t)pszStr;
	stTaskMsg.param[1] = u32Line;

	if (pdFALSE == xPortIsInsideInterrupt())
	{
		stTaskMsg.SyncEGHandle = g_efMsgSoundTask[enSAI];
		stTaskMsg.wakeupbits = 1;
		if (osSemaphoreAcquire(g_bsidPostMsgSoundTask, 10) == osOK)
		{
			if (sizeof(stTaskMsg) != xStreamBufferSend(g_sbhSoundTask[enSAI], &stTaskMsg, sizeof(stTaskMsg), 50))
			{
				mimic_printf("[%s (%d)] xStreamBufferSend NG\r\n", __FUNCTION__, __LINE__);
				bret = false;
			}
			/** Sync */
			uint32_t uxBits = osEventFlagsWait(g_efMsgSoundTask[enSAI], 1, osFlagsWaitAny, 500);
			if (uxBits != 1u)
			{
				mimic_printf("[%s (%d)] osEventFlagsWait NG\r\n", __FUNCTION__, __LINE__);
				bret = false;
			}
			osSemaphoreRelease(g_bsidPostMsgSoundTask);
		}
		else
		{
			bret = false;
		}
	}
	else
	{
		BaseType_t xHigherPriorityTaskWoken;
		if (osSemaphoreAcquire(g_bsidPostMsgSoundTask, 0) == osOK)
		{
			if (sizeof(stTaskMsg) != xStreamBufferSendFromISR(g_sbhSoundTask[enSAI], &stTaskMsg, sizeof(stTaskMsg), &xHigherPriorityTaskWoken))
			{
				bret = false;
			}
			osSemaphoreRelease(g_bsidPostMsgSoundTask);
		}
		else
		{
			bret = false;
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}

	return bret;
}
/**
 * @brief DMA Stop
 * @param [in]  enSAI instance Number
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool SoundTaskDeviceStop(enSAI_t enSAI)
{
	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return false;
	}

	/** ボリューム下げ */
	uint32_t u32CurVol = s_u32Volume[enSAI];
	while (u32CurVol > 0)
	{
		u32CurVol--;
		SoundTaskWriteCurrentVolume(enSAI1, u32CurVol);
		vTaskDelay(2);
	}
	DrvSAIImmidiateRxStop(enSAI);
	DrvSAIImmidiateTxStop(enSAI);
	DrvSAITxReset(enSAI);
	PostSyncMsgSoundTaskStop(enSAI, __FUNCTION__, __LINE__);
	return true;
}

/**
 * @brief RxDMA Restart
 * @param [in]  enSAI instance Number
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool SoundTaskRxDMARestart(enSAI_t enSAI)
{
	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return false;
	}
	DrvSAIRxEDMABufferRestart(enSAI);
	return true;
}

/**
 * @brief Get Current Sample Rate
 * @param [in]  enSAI instance Number
 * @return sai_sample_rate_t 
 */
DefALLOCATE_ITCM sai_sample_rate_t SoundTaskGetCurrentSampleRate(enSAI_t enSAI)
{
	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return kSAI_SampleRateNone;
	}
	return s_enSampleRate[enSAI];
}

/**
 * @brief Get Current PCM Bits Width
 * @param [in]  enSAI instance Number
 * @return sai_word_width_t 
 */
DefALLOCATE_ITCM sai_word_width_t SoundTaskGetCurrentWordWidth(enSAI_t enSAI)
{
	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return kSAI_SampleRateNone;
	}
	return s_enWordWidth[enSAI];
}

#define kVolumeMaxOfCodecIC (0x7Fu)

uint32_t SoundTaskGetCurrentVolume(enSAI_t enSAI)
{
	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return 0;
	}
	return s_u32Volume[enSAI];
}
_Bool SoundTaskSetCurrentVolume(enSAI_t enSAI, uint32_t u32Vol)
{
	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return false;
	}
	s_u32Volume[enSAI] = u32Vol;
	return true;
}
/**
 * @brief Get Current Volume
 * @param [in]  enSAI instance Number
 * @return sai_word_width_t 
 */
DefALLOCATE_ITCM uint32_t SoundTaskReadCurrentVolume(enSAI_t enSAI)
{
	uint32_t u32RawVol;
	uint32_t u32Vol;

	/** ボリュームの際はここで吸収する */
	u32RawVol = DrvSAIReadVolume(enSAI);
	u32Vol = u32RawVol;
	u32Vol &= kVolumeMaxOfCodecIC;
	u32Vol *= 100;
	u32Vol /= kVolumeMaxOfCodecIC;

	/** この時点でu32Volは0 - 100 */

	if (u32Vol != 0)
	{
		float64_t dfpTmp = u32Vol;
		dfpTmp /= 100.0;
		dfpTmp = dfpTmp * dfpTmp * dfpTmp * dfpTmp;
		u32Vol = (dfpTmp * 100.0 + 0.5);
		if (u32Vol > 100)
		{
			u32Vol = 100;
		}
	}
	mimic_printf("u32Vol = %lu\r\n", u32Vol);
	return u32Vol;
}
/**
 * @brief Set Current Volume
 * @param [in]  enSAI instance Number
 * @return sai_word_width_t 
 */
DefALLOCATE_ITCM _Bool SoundTaskWriteCurrentVolume(enSAI_t enSAI, uint32_t u32Vol)
{
	uint16_t u16RawVol;
	uint8_t u8Temp;

	if ((enSAI < enSAI1) || (enSAI >= enNumOfSAI))
	{
		return false;
	}
	/** この時点でu8Volは0 - 100 */
	if (u32Vol != 0)
	{
		float64_t dfpTmp = (float64_t)u32Vol;
		dfpTmp /= 100.0;
		dfpTmp = sqrt(sqrt(dfpTmp));
		u8Temp = (dfpTmp * 100.0 + 0.5);
		if (u8Temp > 100)
		{
			u8Temp = 100;
		}
	}
	else
	{
		u8Temp = 0;
	}
	//mimic_printf("%lu -> %lu\r\n", u8Vol, u8Temp);

	/** ボリュームの際はここで吸収する */
	if (u8Temp > 100u)
	{
		u16RawVol = kVolumeMaxOfCodecIC;
	}
	else
	{
		u16RawVol = kVolumeMaxOfCodecIC * u8Temp;
		u16RawVol /= 100u;
	}

	return DrvSAIWriteVolume(enSAI, u16RawVol);
}
