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
DefALLOCATE_DATA_DTCM static enSoundTaskState_t s_enSoundTaskState[enNumOfSoundTask] = {
	enSoundTaskStateNoInit,
	enSoundTaskStateNoInit,
	//enSoundTaskStateNoInit,
};
DefALLOCATE_DATA_DTCM static sai_sample_rate_t s_enSampleRate[enNumOfSoundTask] = {
	kSAI_SampleRateNone,
	kSAI_SampleRateNone,
	//kSAI_SampleRateNone,
};
DefALLOCATE_DATA_DTCM static sai_word_width_t s_enWordWidth[enNumOfSoundTask] = {
	kSAI_WordWidthNone,
	kSAI_WordWidthNone,
	//kSAI_WordWidthNone,
};

DefALLOCATE_DATA_DTCM static uint32_t s_u32Volume[enNumOfSoundTask] = {50, 50};

/** State Machine Functions */
typedef void (*pfnSoundTaskMatrix_t)(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);

DefALLOCATE_ITCM static void SoundTaskDoMatrix(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S0_E0(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S1_E1(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S1_E3(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S2_E2(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S2_E3(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S2_E4(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void XXXXX(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr);

DefALLOCATE_DATA_DTCM static pfnSoundTaskMatrix_t s_pfnSoundTaskMatrix[enSoundTaskStateMAX][enSoundTaskEventMAX] = {
	/** Init  | Start| Play | Stop | Rec   */
	{S0_E0, XXXXX, XXXXX, XXXXX, XXXXX}, /** enSoundTaskStateNoInit */
	{S0_E0, S1_E1, XXXXX, S1_E3, XXXXX}, /** enSoundTaskStateStop */
	{S0_E0, XXXXX, S2_E2, S2_E3, S2_E4}, /** enSoundTaskStatePlay */
};

/**
 * @brief Do Matrix
 * @param [in]  enSoundTaskNo instanceNo
 * @param [in]  enEvent Enevt Code
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void SoundTaskDoMatrix(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr)
{
	enSoundTaskState_t enState;

	if ((enSoundTaskNo < enSoundTask1) || (enSoundTaskNo >= enNumOfSoundTask))
	{
		mimic_printf("[%s (%d)] Unkown Instance No!!(%d)\r\n", __func__, __LINE__, enSoundTaskNo);
		return;
	}
	if ((enEvent < enSoundTaskEventInit) || (enEvent >= enSoundTaskEventMAX))
	{
		mimic_printf("[%s (%d)] Unkown Event No!!(%d)\r\n", __func__, __LINE__, enEvent);
		return;
	}
	enState = s_enSoundTaskState[enSoundTaskNo];
	if (s_pfnSoundTaskMatrix[enState][enEvent] != NULL)
	{
		s_pfnSoundTaskMatrix[enState][enEvent](enSoundTaskNo, enEvent, u32param, inptr, outptr);
	}
}

/**
 * @brief Nop
 * @param [in]  enSoundTaskNo instanceNo
 * @param [in]  enEvent Enevt Code
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void XXXXX(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr)
{
	if ((enEvent == enSoundTaskEventStart) && (s_enSoundTaskState[enSoundTaskNo] == enSoundTaskStatePlay))
	{
		mimic_printf("Sound Task is Running\r\n");
	}
	else
	{
		mimic_printf("[%s (%d)] Nop!(S%d_E%d)\r\n", __func__, __LINE__, s_enSoundTaskState[enSoundTaskNo], enEvent);
	}
}

/**
 * @brief S0_E0
 * @param [in]  enSoundTaskNo instanceNo
 * @param [in]  enEvent Enevt Code
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S0_E0(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr)
{
	stSoundFormat_t *pstSoundFormat;
	pstSoundFormat = (stSoundFormat_t *)inptr;
	if (enSoundTaskEventInit != enEvent)
	{
		return;
	}

	while (DrvSAIIsTxBufferEmpty(enSoundTaskNo) != false)
	{
		/* NOP */
	}
	if (DrvSAIInit(enSoundTaskNo, pstSoundFormat->enSample, pstSoundFormat->enWidth, pstSoundFormat->bRec) == false)
	{
		mimic_printf("[%s (%d)] DrvSAIInit NG (SAI%d)!\r\n", __func__, __LINE__, enSoundTaskNo);
	}
	else
	{
		uint32_t u32CurVol = 0;
		while (u32CurVol < s_u32Volume[enSoundTaskNo])
		{
			u32CurVol++;
			SoundTaskWriteCurrentVolume(enSoundTask1, u32CurVol);
			vTaskDelay(2);
		}
		SoundTaskWriteCurrentVolume(enSoundTask1, s_u32Volume[enSoundTaskNo]);
		DrvSAITxReset(enSoundTaskNo);
		DrvSAIRxReset(enSoundTaskNo);

		s_enSampleRate[enSoundTaskNo] = pstSoundFormat->enSample;
		s_enWordWidth[enSoundTaskNo] = pstSoundFormat->enWidth;
		s_enSoundTaskState[enSoundTaskNo] = enSoundTaskStateStop;
		//mimic_printf("[%s (%d)] MOV !! s_enSoundTaskState[%d] = %d\r\n", __func__, __LINE__, enSoundTaskNo, s_enSoundTaskState[enSoundTaskNo]);
	}
}

/**
 * @brief S1_E1
 * @param [in]  enSoundTaskNo instanceNo
 * @param [in]  enEvent Enevt Code
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S1_E1(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr)
{
	if (enSoundTaskEventStart == enEvent)
	{
		uint32_t u32CurVol = 0;
		while (u32CurVol < s_u32Volume[enSoundTaskNo])
		{
			u32CurVol++;
			SoundTaskWriteCurrentVolume(enSoundTask1, u32CurVol);
			vTaskDelay(2);
		}
		SoundTaskWriteCurrentVolume(enSoundTaskNo, s_u32Volume[enSoundTaskNo]);
		//mimic_printf("[%s (%d)] Now Play Start (SAI%d)!\r\n", __func__, __LINE__, enSoundTaskNo);
		s_enSoundTaskState[enSoundTaskNo] = enSoundTaskStatePlay;
		//mimic_printf("[%s (%d)] MOV !! s_enSoundTaskState[%d] = %d\r\n", __func__, __LINE__, enSoundTaskNo, s_enSoundTaskState[enSoundTaskNo]);
	}
}

/**
 * @brief S1_E1
 * @param [in]  enSoundTaskNo instanceNo
 * @param [in]  enEvent Enevt Code
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S1_E3(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr)
{
	if (enSoundTaskEventStop != enEvent)
	{
		mimic_printf("[%s (%d)] Unexpected Event No!!(%d)\r\n", __func__, __LINE__, enEvent);
		return;
	}
	mimic_printf("[%s (%d)] SoundTask is already Stopped.\r\n", __func__, __LINE__);
}

/**
 * @brief S2_E2
 * @param [in]  enSoundTaskNo instanceNo
 * @param [in]  enEvent Enevt Code
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S2_E2(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr)
{
	if (enSoundTaskEventPlayData == enEvent)
	{
		stu8DataPtr_t *pst;
		_Bool *pBool = (_Bool *)outptr;
		pst = (stu8DataPtr_t *)inptr;

		if (DrvSAITx(enSoundTaskNo, pst->pu8, pst->u32ByteCnt) == false)
		{
			mimic_printf("[%s (%d)] DrvSAITx NG\r\n", __func__, __LINE__);

			mimic_printf("[%s (%d)] Sound Task Sate -> Stop\r\n", __func__, __LINE__);
			*pBool = false;

			/* STOPへ */
			s_enSoundTaskState[enSoundTaskNo] = enSoundTaskStateStop;
			DrvSAITxReset(enSoundTaskNo);
			DrvSAIRxReset(enSoundTaskNo);
		}
		else
		{
			*pBool = true;
		}
	}
}

/**
 * @brief S2_E3
 * @param [in]  enSoundTaskNo instanceNo
 * @param [in]  enEvent Enevt Code
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S2_E3(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr)
{
	if (enSoundTaskEventStop != enEvent)
	{
		mimic_printf("[%s (%d)] Unexpected Event No!!(%d)\r\n", __func__, __LINE__, enEvent);
		return;
	}

	mimic_printf("[%s (%d)] Now Stop (SAI%d)! by <%s (%d)>\r\n", __func__, __LINE__, enSoundTaskNo, (char *)u32param[0], u32param[1]);
	s_enSoundTaskState[enSoundTaskNo] = enSoundTaskStateStop;
	DrvSAITxReset(enSoundTaskNo);
	DrvSAIRxReset(enSoundTaskNo);
}

/**
 * @brief S2_E4
 * @param [in]  enSoundTaskNo instanceNo
 * @param [in]  enEvent Enevt Code
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S2_E4(enSoundTask_t enSoundTaskNo, enSoundTaskEvent_t enEvent, uint32_t u32param[], uintptr_t inptr, uintptr_t outptr)
{
	stu8DataPtr_t *pst;
	_Bool *pBool = (_Bool *)outptr;
	pst = (stu8DataPtr_t *)inptr;

	if (enSoundTaskEventRecData != enEvent)
	{
		mimic_printf("[%s (%d)] Unexpected Event No!!(%d)\r\n", __func__, __LINE__, enEvent);
		return;
	}

	if (DrvSAIRx(enSoundTaskNo, pst->pu8, &pst->u32RcvCnt) == false)
	{
		mimic_printf("[%s (%d)] DrvSAIRx Stop\r\n", __func__, __LINE__);
		*pBool = false;
		/** STOPへ */
		s_enSoundTaskState[enSoundTaskNo] = enSoundTaskStateStop;
		DrvSAITxReset(enSoundTaskNo);
		DrvSAIRxReset(enSoundTaskNo);
	}
	else
	{
		*pBool = true;
	}
}

/**
 * @brief Task Main Loop
 * @param [in]  enSoundTaskNo Instance Number
 * @return void
 */
DefALLOCATE_ITCM static void SoundTaskActual(enSoundTask_t enSoundTaskNo)
{
	stTaskMsgBlock_t stTaskMsg = {0};
	uint8_t msg_prio; /* Message priority is ignored */

	if (osOK == osMessageQueueGet(g_mqSoundTask[enSoundTaskNo], &stTaskMsg, &msg_prio, portMAX_DELAY))
	{
		switch (stTaskMsg.enMsgId)
		{
		case enSoundTaskInit:
			SoundTaskDoMatrix(enSoundTaskNo, enSoundTaskEventInit, stTaskMsg.param, stTaskMsg.ptrDataForDst, stTaskMsg.ptrDataForSrc);
			break;
		case enSoundTaskStart:
			SoundTaskDoMatrix(enSoundTaskNo, enSoundTaskEventStart, stTaskMsg.param, stTaskMsg.ptrDataForDst, stTaskMsg.ptrDataForSrc);
			break;
		case enSoundTaskSendAudioData:
			SoundTaskDoMatrix(enSoundTaskNo, enSoundTaskEventPlayData, stTaskMsg.param, stTaskMsg.ptrDataForDst, stTaskMsg.ptrDataForSrc);
			break;
		case enSoundTaskStop:
			SoundTaskDoMatrix(enSoundTaskNo, enSoundTaskEventStop, stTaskMsg.param, stTaskMsg.ptrDataForDst, stTaskMsg.ptrDataForSrc);
			break;
		case enSoundTaskRcvAudioData:
			SoundTaskDoMatrix(enSoundTaskNo, enSoundTaskEventRecData, stTaskMsg.param, stTaskMsg.ptrDataForDst, stTaskMsg.ptrDataForSrc);
			break;
		default:
			mimic_printf("[%s (%d)] Unkown Msg\r\n", __func__, __LINE__);
			break;
		}

		/* Sync */
		if (stTaskMsg.SyncEGHandle != NULL)
		{
			osEventFlagsSet(stTaskMsg.SyncEGHandle, stTaskMsg.wakeupbits);
		}

		/* Free */
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
	enSoundTask_t enSoundTaskNo = (enSoundTask_t)argument;

	mimic_printf("[%s (%d)] Start (%lu msec)\r\n", __func__, __LINE__, xTaskGetTickCount());

	if ((enSoundTaskNo < enSoundTask1) || (enSoundTaskNo >= enNumOfSoundTask))
	{
		vTaskSuspend(NULL);
	}


	if(false == DrvSAIInit(enSoundTaskNo, kSAI_SampleRate44100Hz, kSAI_WordWidth16bits, false))
	{
		mimic_printf("[%s (%d)] DrvSAIInit NG (%lu msec)\r\n", __func__, __LINE__, xTaskGetTickCount());

	}
	s_u32Volume[enSoundTaskNo] = 50;
	SoundTaskWriteCurrentVolume(enSoundTaskNo, 0);
	mimic_printf("Volume = %lu\r\n", SoundTaskGetCurrentVolume(enSoundTask1));
	for (;;)
	{
		SoundTaskActual(enSoundTaskNo);
	}

	vTaskSuspend(NULL);
}

/**
 * @brief Post Message (enSoundTaskNoInit)
 * @param [in]  enSoundTaskNo instance Number
 * @param [in]  enSample SampleRate
 * @param [in]  enWidth BitsWidth
 * @param [in]  bRec Recording Flag
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool PostSyncMsgSoundTaskDeviceInit(enSoundTask_t enSoundTaskNo, sai_sample_rate_t enSample, sai_word_width_t enWidth, _Bool bRec)
{
	/* var */
	_Bool bret = false;

	/* begin */
	if ((enSoundTaskNo < enSoundTask1) || (enSoundTaskNo >= enNumOfSoundTask))
	{
		goto _END;
	}

	{
		alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
		stSoundFormat_t *pstSoundFormat;
		stTaskMsg.enMsgId = enSoundTaskInit;
		stTaskMsg.SyncEGHandle = g_efSoundTaskEventGroup[enSoundTaskNo];
		stTaskMsg.wakeupbits = 1;
		pstSoundFormat = pvPortMalloc(sizeof(stSoundFormat_t));

		if (pstSoundFormat != NULL)
		{
			stTaskMsg.ptrDataForDst = (uintptr_t)pstSoundFormat;
			pstSoundFormat->enSample = enSample;
			pstSoundFormat->enWidth = enWidth;
			pstSoundFormat->bRec = bRec;

			if (osOK == osMessageQueuePut(g_mqSoundTask[enSoundTaskNo], &stTaskMsg, 0, 50))
			{
				/* Sync */
				uint32_t uxBits = osEventFlagsWait(g_efSoundTaskEventGroup[enSoundTaskNo], 1, osFlagsWaitAny, 500);
				if (uxBits == 1u)
				{
					bret = true;
				}
				else
				{
					mimic_printf("[%s (%d)] osEventFlagsWait NG\r\n", __func__, __LINE__);
				}
			}
			else
			{
				mimic_printf("[%s (%d)] osMessageQueuePut NG\r\n", __func__, __LINE__);
			}
			
		}
	}
_END:
	return bret;
}

/**
 * @brief Post Message (enSoundTaskStart)
 * @param [in]  enSoundTaskNo instance Number
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool PostMsgSoundTaskDeviceStart(enSoundTask_t enSoundTaskNo)
{
	/* var */
	_Bool bret = false;

	/* begin */
	if ((enSoundTaskNo < enSoundTask1) || (enSoundTaskNo >= enNumOfSoundTask))
	{
		goto _END;
	}

	{
		alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
		stTaskMsg.enMsgId = enSoundTaskStart;

		if (osOK == osMessageQueuePut(g_mqSoundTask[enSoundTaskNo], &stTaskMsg, 0, 50))
		{
			bret = true;
		}
		else
		{
			mimic_printf("[%s (%d)] osMessageQueuePut NG\r\n", __func__, __LINE__);
		}
	}
_END:
	return bret;
}

/**
 * @brief Post Sync Message (enSoundTaskNoSendAudioData)
 * @param [in]  enSoundTaskNo instance Number
 * @param [in]  pu8 Pointer of PCM data
 * @param [in]  u32ByteCnt size of PCM data
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool PostMsgSoundTaskSendAudioData(enSoundTask_t enSoundTaskNo, uint8_t pu8[], uint32_t u32ByteCnt)
{
	/* var */
	_Bool bret = false;

	/* begin */
	if ((enSoundTaskNo < enSoundTask1) || (enSoundTaskNo >= enNumOfSoundTask) || (pu8 == NULL))
	{
		goto _END;
	}

	{
		alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
		stu8DataPtr_t *pstBuf;
		stTaskMsg.enMsgId = enSoundTaskSendAudioData;
		pstBuf = pvPortMalloc(sizeof(stu8DataPtr_t));

		if (pstBuf != NULL)
		{
			uint32_t uxBits;

			stTaskMsg.ptrDataForDst = (uintptr_t)pstBuf;
			stTaskMsg.ptrDataForSrc = (uintptr_t)&bret;
			stTaskMsg.SyncEGHandle = g_efSoundTaskEventGroup[enSoundTaskNo];
			stTaskMsg.wakeupbits = 1;
			pstBuf->pu8 = pu8;
			pstBuf->u32ByteCnt = u32ByteCnt;

			if (osOK == osMessageQueuePut(g_mqSoundTask[enSoundTaskNo], &stTaskMsg, 0, 50))
			{
				/* Sync */
				uxBits = osEventFlagsWait(g_efSoundTaskEventGroup[enSoundTaskNo], 1, osFlagsWaitAny, 1000);
				if (uxBits == 1u)
				{
					bret = true;
				}
				else
				{
					mimic_printf("[%s (%d)] osEventFlagsWait NG\r\n", __func__, __LINE__);
				}
			}
			else
			{
				mimic_printf("[%s (%d)] osMessageQueuePut NG\r\n", __func__, __LINE__);
			}
		}
	}
_END:
	return bret;
}

/**
 * @brief Post Sync Message (enSoundTaskNoRcvAudioData)
 * @param [in]  enSoundTaskNo instance Number
 * @param [in]  pu8 Pointer of PCM Buffer
 * @param [in]  u32ByteCnt size of PCM Buffer
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool PostMsgSoundTaskRcvAudioData(enSoundTask_t enSoundTaskNo, uint8_t pu8[], uint32_t *pu32br)
{
	/* var */
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
	stu8DataPtr_t *pstBuf;
	_Bool bret = false;

	/* begin */
	if ((enSoundTaskNo < enSoundTask1) || (enSoundTaskNo >= enNumOfSoundTask) || (pu8 == NULL))
	{
		goto _END;
	}

	{
		stTaskMsg.enMsgId = enSoundTaskRcvAudioData;
		pstBuf = pvPortMalloc(sizeof(stu8DataPtr_t));

		if (pstBuf != NULL)
		{
			uint32_t uxBits;

			stTaskMsg.ptrDataForDst = (uintptr_t)pstBuf;
			stTaskMsg.ptrDataForSrc = (uintptr_t)&bret;
			stTaskMsg.SyncEGHandle = g_efSoundTaskEventGroup[enSoundTaskNo];
			stTaskMsg.wakeupbits = 1;
			pstBuf->pu8 = pu8;
			pstBuf->u32ByteCnt = 0;
			pstBuf->u32RcvCnt = 0;

			if (osOK == osMessageQueuePut(g_mqSoundTask[enSoundTaskNo], &stTaskMsg, 0, 50))
			
			{
				/* Sync */
				uxBits = osEventFlagsWait(g_efSoundTaskEventGroup[enSoundTaskNo], 1, osFlagsWaitAny, 10000);
				if (uxBits == 1u)
				{
					*pu32br = pstBuf->u32RcvCnt;
					bret = true;
				}
				else
				{
					mimic_printf("[%s (%d)] osEventFlagsWait NG (%d)\r\n", __func__, __LINE__, uxBits);
				}
			}
			else
			{
				mimic_printf("[%s (%d)] g_mqSoundTask NG\r\n", __func__, __LINE__);
			}
		}
	}
_END:
	return bret;
}

/**
 * @brief Post Message (enSoundTaskNoStop)
 * @param [in]  enSoundTaskNo instance Number
 * @param [in]  pu8 Pointer of PCM Buffer
 * @param [in]  u32ByteCnt size of PCM Buffer
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool PostMsgSoundTaskStop(enSoundTask_t enSoundTaskNo, const char *pszStr, uint32_t u32Line)
{
	/* var */
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
	_Bool bret = false;

	/* begin */
	if ((enSoundTaskNo < enSoundTask1) || (enSoundTaskNo >= enNumOfSoundTask))
	{
		goto _END;
	}

	{
		stTaskMsg.enMsgId = enSoundTaskStop;
		stTaskMsg.param[0] = (uint32_t)pszStr;
		stTaskMsg.param[1] = u32Line;
		if (osOK == osMessageQueuePut(g_mqSoundTask[enSoundTaskNo], &stTaskMsg, 0, 50))
		{
			bret = true;
		}
		else
		{
			mimic_printf("[%s (%d)] osMessageQueuePut NG\r\n", __func__, __LINE__);
		}
	}
_END:
	return bret;
}
DefALLOCATE_ITCM static _Bool PostSyncMsgSoundTaskStop(enSoundTask_t enSoundTaskNo, const char *pszStr, uint32_t u32Line)
{
	/* var */
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
	_Bool bret = false;
	/* begin */
	if ((enSoundTaskNo < enSoundTask1) || (enSoundTaskNo >= enNumOfSoundTask))
	{
		goto _END;
	}

	{
		stTaskMsg.enMsgId = enSoundTaskStop;
		stTaskMsg.param[0] = (uint32_t)pszStr;
		stTaskMsg.param[1] = u32Line;

		if (pdFALSE == xPortIsInsideInterrupt())
		{
			stTaskMsg.SyncEGHandle = g_efSoundTaskEventGroup[enSoundTaskNo];
			stTaskMsg.wakeupbits = 1;
		}

		if (osOK == osMessageQueuePut(g_mqSoundTask[enSoundTaskNo], &stTaskMsg, 0, 50))
		{
			if (pdFALSE == xPortIsInsideInterrupt())
			{
				bret = true;
			}
			else
			{
				/* Sync */
				uint32_t uxBits = osEventFlagsWait(g_efSoundTaskEventGroup[enSoundTaskNo], 1, osFlagsWaitAny, 500);
				if (uxBits == 1u)
				{
					bret = true;
				}
				else
				{
					mimic_printf("[%s (%d)] osEventFlagsWait NG\r\n", __func__, __LINE__);
				}
			}
		}
		else
		{
			mimic_printf("[%s (%d)] osMessageQueuePut NG\r\n", __func__, __LINE__);
		}
	}
_END:
	return bret;
}
/**
 * @brief DMA Stop
 * @param [in]  enSoundTaskNo instance Number
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool SoundTaskDeviceStop(enSoundTask_t enSoundTaskNo)
{
	if ((enSoundTaskNo < enSoundTask1) || (enSoundTaskNo >= enNumOfSoundTask))
	{
		return false;
	}

	/** ボリューム下げ */
	uint32_t u32CurVol = s_u32Volume[enSoundTaskNo];
	while (u32CurVol > 0)
	{
		u32CurVol--;
		SoundTaskWriteCurrentVolume(enSoundTask1, u32CurVol);
		vTaskDelay(2);
	}
	DrvSAIImmidiateRxStop(enSoundTaskNo);
	DrvSAIImmidiateTxStop(enSoundTaskNo);
	DrvSAITxReset(enSoundTaskNo);
	PostSyncMsgSoundTaskStop(enSoundTaskNo, __func__, __LINE__);
	return true;
}

/**
 * @brief RxDMA Restart
 * @param [in]  enSoundTaskNo instance Number
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool SoundTaskRxDMARestart(enSoundTask_t enSoundTaskNo)
{
	if ((enSoundTaskNo < enSoundTask1) || (enSoundTaskNo >= enNumOfSoundTask))
	{
		return false;
	}
	DrvSAIRxEDMABufferRestart(enSoundTaskNo);
	return true;
}

/**
 * @brief Get Current Sample Rate
 * @param [in]  enSoundTaskNo instance Number
 * @return sai_sample_rate_t 
 */
DefALLOCATE_ITCM sai_sample_rate_t SoundTaskGetCurrentSampleRate(enSoundTask_t enSoundTaskNo)
{
	if ((enSoundTaskNo < enSoundTask1) || (enSoundTaskNo >= enNumOfSoundTask))
	{
		return kSAI_SampleRateNone;
	}
	return s_enSampleRate[enSoundTaskNo];
}

/**
 * @brief Get Current PCM Bits Width
 * @param [in]  enSoundTaskNo instance Number
 * @return sai_word_width_t 
 */
DefALLOCATE_ITCM sai_word_width_t SoundTaskGetCurrentWordWidth(enSoundTask_t enSoundTaskNo)
{
	if ((enSoundTaskNo < enSoundTask1) || (enSoundTaskNo >= enNumOfSoundTask))
	{
		return kSAI_SampleRateNone;
	}
	return s_enWordWidth[enSoundTaskNo];
}

#define kVolumeMaxOfCodecIC (0x7Fu)

uint32_t SoundTaskGetCurrentVolume(enSoundTask_t enSoundTaskNo)
{
	if ((enSoundTaskNo < enSoundTask1) || (enSoundTaskNo >= enNumOfSoundTask))
	{
		return 0;
	}
	return s_u32Volume[enSoundTaskNo];
}
_Bool SoundTaskSetCurrentVolume(enSoundTask_t enSoundTaskNo, uint32_t u32Vol)
{
	if ((enSoundTaskNo < enSoundTask1) || (enSoundTaskNo >= enNumOfSoundTask))
	{
		return false;
	}
	s_u32Volume[enSoundTaskNo] = u32Vol;
	return true;
}
/**
 * @brief Get Current Volume
 * @param [in]  enSoundTaskNo instance Number
 * @return sai_word_width_t 
 */
DefALLOCATE_ITCM uint32_t SoundTaskReadCurrentVolume(enSoundTask_t enSoundTaskNo)
{
	uint32_t u32RawVol;
	uint32_t u32Vol;

	/* ボリュームの際はここで吸収する */
	u32RawVol = DrvSAIReadVolume(enSoundTaskNo);
	u32Vol = u32RawVol;
	u32Vol &= kVolumeMaxOfCodecIC;
	u32Vol *= 100;
	u32Vol /= kVolumeMaxOfCodecIC;

	/* この時点でu32Volは0 - 100 */

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
 * @param [in]  enSoundTaskNo instance Number
 * @return sai_word_width_t 
 */
DefALLOCATE_ITCM _Bool SoundTaskWriteCurrentVolume(enSoundTask_t enSoundTaskNo, uint32_t u32Vol)
{
	uint16_t u16RawVol;
	uint8_t u8Temp;

	if ((enSoundTaskNo < enSoundTask1) || (enSoundTaskNo >= enNumOfSoundTask))
	{
		return false;
	}
	/* この時点でu8Volは0 - 100 */
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

	/* ボリュームの際はここで吸収する */
	if (u8Temp > 100u)
	{
		u16RawVol = kVolumeMaxOfCodecIC;
	}
	else
	{
		u16RawVol = kVolumeMaxOfCodecIC * u8Temp;
		u16RawVol /= 100u;
	}

	return DrvSAIWriteVolume(enSoundTaskNo, u16RawVol);
}
