/**
 * @file		PlayCtrl.c
 * @brief		TOOD
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
#include "PlayCtrl/PlayCtrl.h"
#include "PlayCtrl/PlayCtrlWav.h"
#include "board.h"
#include "mimiclib.h"
#include "ff.h"
#include "common/common.h"
#include "SoundTask/SoundTask.h"
#include "AudioFile/AudioFileList.h"
#include "PlayCtrl/PlayCtrlAudio.h"
#include "AudioFile/wav/wav.h"
#include "Task/MeterTask/MeterTask.h"
#include "GUI/MainWindow/MainWindow.h"

typedef enum
{
	enPlayCtrlStateStop = 0,
	enPlayCtrlStatePlay,
	enPlayCtrlStateRec,
	enPlayCtrlStateMAX,
} enPlayCtrlState_t;

typedef enum
{
	enPlayCtrlEventStop = 0,
	enPlayCtrlEventStart,
	enPlayCtrlEventPlaying,
	enPlayCtrlEventRec,
	enPlayCtrlEventRecording,
	enPlayCtrlEventNext,
	enPlayCtrlEventPrev,
	enPlayCtrlEventMAX,
} enPlayCtrlEvent_t;

DefALLOCATE_ITCM static void ClosePlayProcess(void);
DefALLOCATE_ITCM static void CloseRecProcess(void);

DefALLOCATE_ITCM static _Bool GetAudioFilePath(uint32_t u32TrackNo, TCHAR szFilePath[]);

DefALLOCATE_ITCM static _Bool PostMsgPlayCtrlPlaying(void);
DefALLOCATE_ITCM static _Bool PostMsgPlayCtrlRecording(void);

DefALLOCATE_ITCM static uint8_t *OpenNextPlayFile(uint32_t u32NowTrackNo, TCHAR szCurrentFilePath[], uint32_t SizeofStr, 
												  stCodecCondition_t *pst, uint32_t *pu32BufSize,
												  _Bool bForceInit);

DefALLOCATE_ITCM static _Bool getNextTrackNo(void);
DefALLOCATE_ITCM static _Bool getPrevTrackNo(void);
DefALLOCATE_BSS_DTCM uint32_t s_u32PlayStartTick = 0;

/** Matrix Functions */
typedef void (*pfnPlayCtrlMatrix_t)(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr);

DefALLOCATE_ITCM static void PlayCtrlDoMatrix(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S0_E0(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S0_E1(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S0_E3(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S1_E0(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S1_E2(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S1_E5(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S1_E6(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S2_E0(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void S2_E4(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr);
DefALLOCATE_ITCM static void XXXXX(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr);

DefALLOCATE_DATA_DTCM static pfnPlayCtrlMatrix_t s_pfnPlayCtrlMatrix[enPlayCtrlStateMAX][enPlayCtrlEventMAX] = {
	{S0_E0, S0_E1, XXXXX, S0_E3, XXXXX, XXXXX, XXXXX}, /** Stop */
	{S1_E0, XXXXX, S1_E2, XXXXX, XXXXX, S1_E5, S1_E6}, /** Play */
	{S2_E0, XXXXX, XXXXX, XXXXX, S2_E4, XXXXX, XXXXX}, /** Rec */
};

/** private member */
DefALLOCATE_DATA_DTCM static enPlayCtrlState_t s_enPlayCtrlState = enPlayCtrlStateStop;
DefALLOCATE_DATA_DTCM static enPlayCtrlPlayArea_t s_enPlayCtrlPlayArea = enPlayCtrlPlayAreaALL;
DefALLOCATE_DATA_DTCM static enPlayCtrlRepeat_t s_enPlayCtrlRepeat = enPlayCtrlRepeatOff;

DefALLOCATE_DATA_DTCM static uint32_t s_u32NowTrackNo = 0;
DefALLOCATE_BSS_DTCM static TCHAR s_szCurrentFilePath[768];

DefALLOCATE_DATA_DTCM static uint8_t *s_pu8PCMBufferPlayPCMBuffer = NULL;
DefALLOCATE_DATA_DTCM static uint32_t s_u32PCMBufferSize = 0;
DefALLOCATE_DATA_DTCM static stCodecCondition_t s_stPlayCondition = {
	kSAI_SampleRate44100Hz,
	kSAI_WordWidth16bits,
	enAudioFileWAV,
	2,
};

DefALLOCATE_DATA_DTCM static uint8_t *s_pu8PCMBufferRecPCMBuffer = NULL;
DefALLOCATE_DATA_DTCM static stCodecCondition_t s_stRecCondition = {
	kSAI_SampleRate44100Hz,
	kSAI_WordWidth16bits,
	enAudioFileWAV,
	2,
};

DefALLOCATE_ITCM static void StopPlayRecProcess(void)
{
	MeterTaskStop();
	PostMsgSoundTaskStop(enSAI1, __func__, __LINE__);
	s_enPlayCtrlState = enPlayCtrlStateStop;
}

/**
 * @brief S0_E0
 * @param [in]  enEvent Enevt Code
 * @param [in]  param parameter from T_MSG
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S0_E0(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr)
{
	if (enPlayCtrlEventStop != enEvent)
	{
		mimic_printf("[%s (%d)] Unexpected Event No!!(%d)\r\n", __func__, __LINE__, enEvent);
		return;
	}
	mimic_printf("[%s (%d)] PlayTask is already Stopped.\r\n", __func__, __LINE__);
}

/**
 * @brief S0_E1
 * @param [in]  enEvent Enevt Code
 * @param [in]  param parameter from T_MSG
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S0_E1(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr)
{
	if (enPlayCtrlEventStart != enEvent)
	{
		mimic_printf("[%s (%d)] Unexpected Event No!!(%d)\r\n", __func__, __LINE__, enEvent);
		return;
	}
	/** Init Track No */
	if (param[0] != UINT32_MAX)
	{
		s_u32NowTrackNo = param[0];
	}
	else
	{
		s_u32NowTrackNo = 0;
	}

	ClosePlayProcess();

	if (IsPlayAudioFileOpend() == false)
	{
		s_pu8PCMBufferPlayPCMBuffer = OpenNextPlayFile(s_u32NowTrackNo, s_szCurrentFilePath, sizeof(s_szCurrentFilePath), &s_stPlayCondition, &s_u32PCMBufferSize, true);

		if (s_pu8PCMBufferPlayPCMBuffer == NULL)
		{
			mimic_printf("*** Play Stop <%s (%d)>\r\n", __func__, __LINE__);
			StopPlayRecProcess();
			return;
		}
		s_u32PlayStartTick = osKernelGetTickCount();
	}
	/** Send Playing */
	if (PostMsgPlayCtrlPlaying() != false)
	{
		MeterTaskRun();
		s_enPlayCtrlState = enPlayCtrlStatePlay;
	}
	else
	{
		mimic_printf("[%s (%d)] PostMsgPlayCtrlPlaying NG\r\n", __func__, __LINE__);
	}
	
}

/**
 * @brief S0_E3
 * @param [in]  enEvent Enevt Code
 * @param [in]  param parameter from T_MSG
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S0_E3(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr)
{
	if (enPlayCtrlEventRec != enEvent)
	{
		mimic_printf("[%s (%d)] Unexpected Event No!!(%d)\r\n", __func__, __LINE__, enEvent);
		return;
	}

	CloseRecProcess();

	/** Send Recording */
	if (PostMsgPlayCtrlRecording() != false)
	{
		s_enPlayCtrlState = enPlayCtrlStateRec;
	}
}

/**
 * @brief S1_E0
 * @param [in]  enEvent Enevt Code
 * @param [in]  param parameter from T_MSG
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S1_E0(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr)
{
	if (enPlayCtrlEventStop != enEvent)
	{
		mimic_printf("[%s (%d)] Unexpected Event No!!(%d)\r\n", __func__, __LINE__, enEvent);
		return;
	}

	/** Stop */
	ClosePlayProcess();
	mimic_printf("*** Play Stop <%s (%d)>\r\n", __func__, __LINE__);

	StopPlayRecProcess();
}

/**
 * @brief S1_E2
 * @param [in]  enEvent Enevt Code
 * @param [in]  param parameter from T_MSG
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S1_E2(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr)
{
	_Bool *pBool = (_Bool*)outptr;

	*pBool = true;
	if (IsPlayAudioFileOpend() == false)
	{
		s_pu8PCMBufferPlayPCMBuffer = OpenNextPlayFile(s_u32NowTrackNo, s_szCurrentFilePath, sizeof(s_szCurrentFilePath), &s_stPlayCondition, &s_u32PCMBufferSize, false);

		/** Check PCM Buffer */
		if (s_pu8PCMBufferPlayPCMBuffer == NULL)
		{
			mimic_printf("*** Play Stop <%s (%d)>\r\n", __func__, __LINE__);
			/** 再生停止処理 */
			StopPlayRecProcess();
			return;
		}
		s_u32PlayStartTick = osKernelGetTickCount();
	}

	if (IsPlayAudioFileOpend() != false)
	{
		//mimic_printf("*** playing...[%03d] :  %lu msec\r\n", s_u32NowTrackNo, osKernelGetTickCount()-s_u32PlayStartTick);
		{
			char szTime[32];
			uint32_t u32Tick = osKernelGetTickCount()-s_u32PlayStartTick;
			uint32_t u32Hour = u32Tick / 3600000;
			uint32_t u32Min = u32Tick / 60000;
			u32Min %= 60;
			uint32_t u32Sec = u32Tick / 1000;
			u32Sec %= 60;

			uint32_t u32Mi = u32Tick % 1000;
			mimic_sprintf(szTime, sizeof(szTime),"%02lu:%02lu:%02lu.%03lu", u32Hour, u32Min, u32Sec, u32Mi);
			MainWindowSetTextAreaTrackTime(szTime);
		}
		/** オープンされたファイルからPCMデータを取得 */
		uint32_t u32ReadByte = ReadPlayAudioFile(&s_stPlayCondition, s_pu8PCMBufferPlayPCMBuffer, s_u32PCMBufferSize);
		if (u32ReadByte > 0)
		{
			/** SAIのEDMA転送 */
			if (PostMsgSoundTaskSendAudioData(enSAI1, s_pu8PCMBufferPlayPCMBuffer, u32ReadByte) == false)
			{
				/** 今のファイルクローズ */
				ClosePlayProcess();
				/** 停止した */
				mimic_printf("*** Play Stop <%s (%d)>\r\n", __func__, __LINE__);
				/** 再生停止処理 */
				StopPlayRecProcess();
				*pBool = false;
			}
			else
			{
				/** OKなのでPlayingイベント発行 */
				if(false == PostMsgPlayCtrlPlaying())
				{
					mimic_printf("[%s (%d)] PostMsgPlayCtrlPlaying NG\r\n", __func__, __LINE__);
				}
			}
		}
		else
		{
			/** 今のファイルクローズ */
			ClosePlayProcess();

			/** つぎのTrackNoは? */
			if (getNextTrackNo() == false)
			{
				/** ない */
				mimic_printf("*** Play Stop <%s (%d)>\r\n", __func__, __LINE__);
				/** 再生停止処理 */
				StopPlayRecProcess();
			}
			else
			{
				/** 次のファイルへ */
				mimic_printf("*** Play Next\r\n");
				PostMsgPlayCtrlPlaying();
			}
		}
	}
}

/**
 * @brief S1_E5
 * @param [in]  enEvent Enevt Code
 * @param [in]  param parameter from T_MSG
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */

DefALLOCATE_ITCM static void S1_E5(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr)
{
	/** つぎのTrackNoは? */
	if (IsPlayAudioFileOpend() != false)
	{
		if (getNextTrackNo() != false)
		{
			/** 今のファイルクローズ */
			ClosePlayProcess();
	
			/** 次のファイルへ */
			mimic_printf("*** Play Next <%03u>\r\n", s_u32NowTrackNo);
			PostMsgPlayCtrlPlaying();
		}
	}
}
/**
 * @brief S1_E6
 * @param [in]  enEvent Enevt Code
 * @param [in]  param parameter from T_MSG
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */

DefALLOCATE_ITCM static void S1_E6(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr)
{
	/** つぎのTrackNoは? */
	if (IsPlayAudioFileOpend() != false)
	{
		if (getPrevTrackNo() != false)
		{
			/** 今のファイルクローズ */
			ClosePlayProcess();
			/** 前のファイルへ */
			mimic_printf("*** Play Prev <%03u>\r\n", s_u32NowTrackNo);
			PostMsgPlayCtrlPlaying();
		}
	}
}

/**
 * @brief S2_E0
 * @param [in]  enEvent Enevt Code
 * @param [in]  param parameter from T_MSG
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S2_E0(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr)
{
	if (enPlayCtrlEventStop != enEvent)
	{
		mimic_printf("[%s (%d)] Unexpected Event No!!(%d)\r\n", __func__, __LINE__, enEvent);
		return;
	}

	/** Rec Stop */
	CloseRecProcess();
	StopPlayRecProcess();
}

/**
 * @brief S2_E4
 * @param [in]  enEvent Enevt Code
 * @param [in]  param parameter from T_MSG
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void S2_E4(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr)
{
	static uint32_t s_u32RecPCMBufferSize = 0;

	if (enPlayCtrlEventRecording != enEvent)
	{
		mimic_printf("[%s (%d)] Unexpected Event No!!(%d)\r\n", __func__, __LINE__, enEvent);
		return;
	}

	if (IsRecAudioFileOpend() == false)
	{

		/** Create File Name */
		switch (s_stRecCondition.enFileType)
		{
		case enAudioFileWAV:
			mimic_sprintf(s_szCurrentFilePath, sizeof(s_szCurrentFilePath), _T("T%03lu.wav"), GetAudioFileNumALL());
			mimic_printf("RecFileName : <%s>\r\n", s_szCurrentFilePath);
			break;
		default:
			/** Unkown */
			return;
		}

		/** Open Rec Audio File */
		s_pu8PCMBufferRecPCMBuffer = OpenRecAudioFile(s_szCurrentFilePath, &s_stRecCondition, &s_u32RecPCMBufferSize, true);
		if (s_pu8PCMBufferRecPCMBuffer == NULL)
		{
			mimic_printf("[%s (%d)] OpenRecAudioFile NG : <%s>\r\n", __func__, __LINE__, s_szCurrentFilePath);
			StopPlayRecProcess();
			return;
		}
		SoundTaskRxDMARestart(enSAI1);
		mimic_printf("s_u32RecPCMBufferSize = %lu\r\n", s_u32RecPCMBufferSize);
		mimic_printf("*** Now Rec[%03d] :  <%s>\r\n", GetAudioFileNumALL() + 1, s_szCurrentFilePath);
	}

	if (IsRecAudioFileOpend() != false)
	{
		uint32_t u32ReadByte;

		/** Get PCM Data From Sound Task */
		if (PostMsgSoundTaskRcvAudioData(enSAI1, s_pu8PCMBufferRecPCMBuffer, &u32ReadByte) != false)
		{
			/** SAIのEDMA転送 */
			if (WriteRecAudioFile(&s_stRecCondition, s_pu8PCMBufferRecPCMBuffer, u32ReadByte) != false)
			{
				/** OKなのでRecordingイベント発行 */
				PostMsgPlayCtrlRecording();
			}
			else
			{
				/** 録音停止 */
				CloseRecProcess();
				StopPlayRecProcess();
				mimic_printf("** Rec Stop\r\n");
			}
		}
		else
		{
			if (u32ReadByte > 0)
			{
				WriteRecAudioFile(&s_stRecCondition, s_pu8PCMBufferRecPCMBuffer, u32ReadByte);
			}
			/** 録音停止 */
			CloseRecProcess();
			StopPlayRecProcess();
			mimic_printf("** Rec Stop\r\n");
		}
	}
}

/**
 * @brief PlayCtrlDoMatrix
 * @param [in]  enEvent Enevt Code
 * @param [in]  param parameter from T_MSG
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void PlayCtrlDoMatrix(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr)
{
	enPlayCtrlState_t enState;

	if ((enEvent < enPlayCtrlEventStop) || (enEvent >= enPlayCtrlEventMAX))
	{
		mimic_printf("[%s (%d)] Unkown Event No!!(%d)\r\n", __func__, __LINE__, enEvent);
		return;
	}
	enState = s_enPlayCtrlState;
	if (s_pfnPlayCtrlMatrix[enState][enEvent] != NULL)
	{
		s_pfnPlayCtrlMatrix[enState][enEvent](enEvent, param, inptr, outptr);
	}
}

/**
 * @brief XXXXX
 * @param [in]  enEvent Enevt Code
 * @param [in]  param parameter from T_MSG
 * @param [in]  inptr Extend Data Pointer from T_MSG
 * @param [in/out]  outptr Extend Data Pointer from T_MSG
 * @return void
 */
DefALLOCATE_ITCM static void XXXXX(enPlayCtrlEvent_t enEvent, uint32_t param[], uintptr_t inptr, uintptr_t outptr)
{
	mimic_printf("[%s:%s (%d)] Nop!(S%d_E%d)\r\n", __FILE__, __func__, __LINE__, s_enPlayCtrlState, enEvent);
}

/** 
 * @brief Close Play Audio File
 */
DefALLOCATE_ITCM static void ClosePlayProcess(void)
{
	if (IsPlayAudioFileOpend() != false)
	{
		ClosePlayAudioFile(&s_stPlayCondition, s_pu8PCMBufferPlayPCMBuffer);
	}
}

/** 
 * @brief Close Rec Audio File
 */
DefALLOCATE_ITCM static void CloseRecProcess(void)
{
	if (IsRecAudioFileOpend() != false)
	{
		mimic_printf("[%s (%d)] File Close <%s>\r\n", __func__, __LINE__, s_szCurrentFilePath);
		CloseRecAudioFile(&s_stRecCondition, s_pu8PCMBufferRecPCMBuffer);
	}
}

/** 
 * @brief Close Rec Audio File
 * @param [in]  u32NowTrackNo Track Number
 * @param [in/out]  szCurrentFilePath FilePath
 * @param [in/out]  pst Codec Info
 * @param [in/out]  pu32BufSize size of PCM Buffer
 * @param [in]  szCurrentFilePath FilePath
 * @return pu8PCMBuffer Pointer of PCM Buffer
 */
DefALLOCATE_ITCM static uint8_t *OpenNextPlayFile(uint32_t u32NowTrackNo, TCHAR szCurrentFilePath[], uint32_t SizeofStr, stCodecCondition_t *pst, uint32_t *pu32BufSize, _Bool bForceInit)
{
	uint8_t *pu8PCMBuffer = NULL;

	/** トラック番号からファイル名を取得する */
	if (GetAudioFilePath(u32NowTrackNo, szCurrentFilePath) == false)
	{
		mimic_printf("[%s (%d)] GetAudioFilePath NG :  u32NowTrackNo = %lu\r\n", __func__, __LINE__, u32NowTrackNo);
		return NULL;
	}

	/** File Type? */
	pst->enFileType = GetAudioFileType(szCurrentFilePath, SizeofStr);

	/** Open AudioFile for Play */
	pu8PCMBuffer = OpenPlayAudioFile(szCurrentFilePath, pst, pu32BufSize);
	if (pu8PCMBuffer == NULL)
	{
		mimic_printf("[%s (%d)] OpenPlayAudioFile NG :  <%s>\r\n", __func__, __LINE__, szCurrentFilePath);
		return NULL;
	}

	/** SAI Init */
	if (((uint32_t)SoundTaskGetCurrentSampleRate(enSAI1) != pst->enSample) ||
		((uint16_t)SoundTaskGetCurrentWordWidth(enSAI1) != pst->enBitsWidth) ||
		(bForceInit != false))
	{
		mimic_printf("*** Init Sound Device ***\r\n");
		if (PostSyncMsgSoundTaskDeviceInit(enSAI1, pst->enSample, pst->enBitsWidth, false) == false)
		{
			mimic_printf("[%s (%d)] PostSyncMsgSoundTaskDeviceInit NG\r\n", __func__, __LINE__);
			ClosePlayAudioFile(pst, pu8PCMBuffer);
			return NULL;
		}
	}
	/** SoundTaskがPlayt注に送っても問題ない */
	PostMsgSoundTaskDeviceStart(enSAI1);

	mimic_printf("*** Now Play[%03d] :  <%s>\r\n\r\n", u32NowTrackNo, szCurrentFilePath);

	{
		char szTrackNo[8];
		mimic_sprintf(szTrackNo, sizeof(szTrackNo), "%03u", u32NowTrackNo);
		MainWindowSetTextAreaTrackNo(szTrackNo);
		MainWindowSetTextAreaTrackName(szCurrentFilePath);
	}
	return pu8PCMBuffer;
}

/** 
 * @brief Get Audio File Path by Track Number
 * @param [in]  u32NowTrackNo Track Number
 * @param [in/out]  szFilePath FilePath
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM static _Bool GetAudioFilePath(uint32_t u32TrackNo, TCHAR szFilePath[])
{
	if (s_enPlayCtrlPlayArea == enPlayCtrlPlayAreaDir)
	{
		return GetAudioFilePathCurrentDir(u32TrackNo, szFilePath);
	}
	else
	{
		return GetAudioFilePathALL(u32TrackNo, szFilePath);
	}
}


/** 
 * @brief Get Next Track No by Current Track Number
 * @param [in/out]  pu32TrackNo Pointer of Current Track Number and Next Track Number
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM static _Bool getNextTrackNo(void)
{
	if (s_enPlayCtrlRepeat == enPlayCtrlRepeatALL)
	{
		s_u32NowTrackNo += 1;
		if (s_enPlayCtrlPlayArea == enPlayCtrlPlayAreaALL)
		{
			s_u32NowTrackNo %= GetAudioFileNumALL();
		}
		else
		{
			s_u32NowTrackNo %= GetAudioFileNumCurrentDir();
		}
		return true;
	}
	else if (s_enPlayCtrlRepeat == enPlayCtrlRepeatOff)
	{
		s_u32NowTrackNo += 1;
		if (s_enPlayCtrlPlayArea == enPlayCtrlPlayAreaALL)
		{
			return (s_u32NowTrackNo >= GetAudioFileNumALL()) ? false : true;
		}
		else
		{
			return (s_u32NowTrackNo >= GetAudioFileNumCurrentDir()) ? false : true;
		}
	}
	else
	{
		/* enPlayCtrlRepeatOne */
		return true;
	}
}
DefALLOCATE_ITCM static _Bool getPrevTrackNo(void)
{
	if((s_u32PlayStartTick + 1000) > osKernelGetTickCount())
	{
		return true;
	}

	if(s_u32NowTrackNo > 0)
	{
		
		if (s_enPlayCtrlRepeat == enPlayCtrlRepeatALL)
		{
			s_u32NowTrackNo -= 1;
			if (s_enPlayCtrlPlayArea == enPlayCtrlPlayAreaALL)
			{
				s_u32NowTrackNo %= GetAudioFileNumALL();
			}
			else
			{
				s_u32NowTrackNo %= GetAudioFileNumCurrentDir();
			}
			return true;
		}
		else if (s_enPlayCtrlRepeat == enPlayCtrlRepeatOff)
		{
			s_u32NowTrackNo -= 1;
			if (s_enPlayCtrlPlayArea == enPlayCtrlPlayAreaALL)
			{
				return (s_u32NowTrackNo >= GetAudioFileNumALL()) ? false : true;
			}
			else
			{
				return (s_u32NowTrackNo >= GetAudioFileNumCurrentDir()) ? false : true;
			}
		}
		else
		{
			/* enPlayCtrlRepeatOne */
			return true;
		}
	}
	else
	{
		return true;
	}
}

/** 4sec + 1sec */
#define DefPostMsgTimeout_PrivateEvent (5000)

/** Private PostMsg */
DefALLOCATE_ITCM static _Bool PostMsgPlayCtrlPlaying(void)
{
	/*-- var --*/
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
	_Bool bret = false;

	stTaskMsg.enMsgId = enPlaying;
	stTaskMsg.ptrDataForSrc = (uintptr_t)&bret;
	if (osOK == osMessageQueuePut(g_mqidPlayCtrl, &stTaskMsg, 0, DefPostMsgTimeout_PrivateEvent))
	{
		bret = true;
	}
	else
	{
		mimic_printf("[%s (%d)] osMessageQueuePut NG\r\n", __func__, __LINE__);
	}

	return bret;
}
DefALLOCATE_ITCM static _Bool PostMsgPlayCtrlRecording(void)
{
	/*-- var --*/
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
	_Bool bret = false;

	stTaskMsg.enMsgId = enRecording;

	if (osOK == osMessageQueuePut(g_mqidPlayCtrl, &stTaskMsg, 0, DefPostMsgTimeout_PrivateEvent))
	{
		bret = true;
	}
	else
	{
		mimic_printf("[%s (%d)] osMessageQueuePut NG\r\n", __func__, __LINE__);
	}

	return bret;
}

/** Public PostMsg */

/**
 * @brief Post Message (enPlayStart)
 * @param [in]  u32TrackNo Current Track Number
 * @return true Post OK
 * @return false Post NG
 */
DefALLOCATE_ITCM _Bool PostMsgPlayCtrlStart(uint32_t u32TrackNo)
{
	/*-- var --*/
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
	_Bool bret = false;

	stTaskMsg.enMsgId = enPlayStart;
	stTaskMsg.param[0] = u32TrackNo;

	if (osOK == osMessageQueuePut(g_mqidPlayCtrl, &stTaskMsg, 0, 50))
	{
		bret = true;
	}
	else
	{
		mimic_printf("[%s (%d)] osMessageQueuePut NG\r\n", __func__, __LINE__);
	}

	return bret;
}
DefALLOCATE_ITCM _Bool PostMsgPlayCtrlStop(void)
{
	/*-- var --*/
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
	_Bool bret = false;

	stTaskMsg.enMsgId = enPlayStop;

	if (osOK == osMessageQueuePut(g_mqidPlayCtrl, &stTaskMsg, 0, 50))
	{
		bret = true;
	}
	else
	{
		mimic_printf("[%s (%d)] osMessageQueuePut NG\r\n", __func__, __LINE__);
	}

	return bret;
}
DefALLOCATE_ITCM _Bool PostSyncMsgPlayCtrlStop(void)
{
	/*-- var --*/
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
	_Bool bret = false;

	stTaskMsg.enMsgId = enPlayStop;
	stTaskMsg.SyncEGHandle = g_eidPlayCtrl;
	stTaskMsg.wakeupbits = 1;
	if (osOK == osMessageQueuePut(g_mqidPlayCtrl, &stTaskMsg, 0, 50))
	{
		uint32_t uxBits = osEventFlagsWait(g_eidPlayCtrl, 1, osFlagsWaitAny, 500);
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
	/* Sync */
	
	return bret;
}
DefALLOCATE_ITCM _Bool PostMsgPlayCtrlNext(void)
{
	/*-- var --*/
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
	_Bool bret = false;

	stTaskMsg.enMsgId = enPlayNext;

	if (osOK == osMessageQueuePut(g_mqidPlayCtrl, &stTaskMsg, 0, 50))
	{
		bret = true;
	}
	else
	{
		mimic_printf("[%s (%d)] osMessageQueuePut NG\r\n", __func__, __LINE__);
	}

	return bret;
}
DefALLOCATE_ITCM _Bool PostMsgPlayCtrlPrev(void)
{
	/*-- var --*/
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
	_Bool bret = false;

	stTaskMsg.enMsgId = enPlayPrev;

	if (osOK == osMessageQueuePut(g_mqidPlayCtrl, &stTaskMsg, 0, 50))
	{
		bret = true;
	}
	else
	{
		mimic_printf("[%s (%d)] osMessageQueuePut NG\r\n", __func__, __LINE__);
	}

	return bret;
}
/**
 * @brief Post Message (enRec)
 * @return true Post OK
 * @return false Post NG
 */
DefALLOCATE_ITCM _Bool PostMsgPlayCtrlRec(void)
{
	/*-- var --*/
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};
	_Bool bret = false;

	stTaskMsg.enMsgId = enRec;

	if (osOK == osMessageQueuePut(g_mqidPlayCtrl, &stTaskMsg, 0, 50))
	{
		bret = true;
	}
	else
	{
		mimic_printf("[%s (%d)] osMessageQueuePut NG\r\n", __func__, __LINE__);
	}

	return bret;
}

/**
 * @brief Task Main Loop
 */
DefALLOCATE_ITCM static void PlayCtrlActual(void)
{
	stTaskMsgBlock_t stTaskMsg = {0};
	uint8_t msg_prio; /* Message priority is ignored */

	if (osOK == osMessageQueueGet(g_mqidPlayCtrl, &stTaskMsg, &msg_prio, portMAX_DELAY))
	{
		enPlayCtrlEvent_t enEvent = enPlayCtrlEventMAX;
		switch (stTaskMsg.enMsgId)
		{
		case enPlayStop:
			enEvent = enPlayCtrlEventStop;
			break;
		case enPlayStart:
			enEvent = enPlayCtrlEventStart;
			break;
		case enPlaying:
			enEvent = enPlayCtrlEventPlaying;
			break;
		case enRec:
			enEvent = enPlayCtrlEventRec;
			break;
		case enRecording:
			enEvent = enPlayCtrlEventRecording;
			break;
		case enPlayNext:
			enEvent = enPlayCtrlEventNext;
			break;
		case enPlayPrev:
			enEvent = enPlayCtrlEventPrev;
			break;
		default:
			mimic_printf("[%s (%d)] Unkown Msg\r\n", __func__, __LINE__);
			break;
		}

		if (enEvent != enPlayCtrlEventMAX)
		{
			PlayCtrlDoMatrix(enEvent, stTaskMsg.param, stTaskMsg.ptrDataForDst, stTaskMsg.ptrDataForSrc);
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
DefALLOCATE_ITCM void PlayCtrl(void const *argument)
{

	mimic_printf("[%s (%d)] Start (%lu msec)\r\n", __func__, __LINE__, xTaskGetTickCount());

	for (;;)
	{
		PlayCtrlActual();
	}

	vTaskSuspend(NULL);
}

/**
 * @brief Stop Sound Task
 * @return true Post OK
 * @return false Post NG
 */
DefALLOCATE_ITCM _Bool PlayCtrlSoundDeviceStop(void)
{
	SoundTaskDeviceStop(enSAI1);
	return true;
}

/**
 * @brief Set Repeat Mode
 * @param [in]  enRepeat Repeat Mode
 * @return true Post OK
 * @return false Post NG
 */
DefALLOCATE_ITCM _Bool PlayCtrlSetRepeat(enPlayCtrlRepeat_t enRepeat)
{
	if ((enRepeat >= enPlayCtrlRepeatOff) && (enRepeat < enPlayCtrlRepeatMAX))
	{
		s_enPlayCtrlRepeat = enRepeat;
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * @brief Set Repeat Mode
 * @param [in]  enRepeat Repeat Mode
 * @return true Post OK
 * @return false Post NG
 */
DefALLOCATE_ITCM _Bool PlayCtrlSetPlayArea(enPlayCtrlPlayArea_t enPlayArea)
{
	if (s_enPlayCtrlState != enPlayCtrlStateStop)
	{
		return false;
	}

	if ((enPlayArea >= enPlayCtrlPlayAreaDir) && (enPlayArea < enPlayCtrlPlayAreaMAX))
	{
		s_enPlayCtrlPlayArea = enPlayArea;
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * @brief Set Rec Condition
 * @param [in]  pst Pointer Of Rec Condition
 * @return true Post OK
 * @return false Post NG
 */
DefALLOCATE_ITCM _Bool PlayCtrlSetRecCondition(const stCodecCondition_t *pst)
{
	uint32_t primask;

	if ((s_enPlayCtrlState != enPlayCtrlStateStop) || (pst == NULL))
	{
		return false;
	}
	primask = DisableGlobalIRQ();
	s_stRecCondition = *pst;
	EnableGlobalIRQ(primask);

	return true;
}

/** for Console */
void CmdPlay(uint32_t argc, const char *argv[])
{
	MakeAudioFileListALL();
	MakeAudioFileListCurrentDir();

	if (argc >= 2)
	{
		PostMsgPlayCtrlStart(strtoul(argv[1], 0, 10));
	}
	else
	{
		PostMsgPlayCtrlStart(0);
	}
}

void CmdStop(uint32_t argc, const char *argv[])
{
	PostMsgPlayCtrlStop();
}
void CmdRec(uint32_t argc, const char *argv[])
{
	uint32_t i;
	stCodecCondition_t stRec[] = {
		{enSampleRate44100Hz, enWordWidth16bits, enAudioFileWAV, 2},
		{enSampleRate48KHz, enWordWidth16bits, enAudioFileWAV, 2},
		{enSampleRate48KHz, enWordWidth24bits, enAudioFileWAV, 2},
		{enSampleRate96KHz, enWordWidth24bits, enAudioFileWAV, 2},
	};
	MakeAudioFileListALL();
	MakeAudioFileListCurrentDir();

	if (argc == 2)
	{
		i = strtoul(argv[1], 0, 10);
		if (i < 12)
		{
			if (PlayCtrlSetRecCondition(&stRec[i]) != false)
			{
				PostMsgPlayCtrlRec();
			}
			else
			{
				mimic_printf("[%s (%d)] PlayCtrlSetRecCondition NG\r\n", __func__, __LINE__);
			}
			return;
		}
	}

	mimic_printf("USAGE:\r\n");
	mimic_printf("     rec 0 (wav 44.1KHz 16bits)\r\n");
	mimic_printf("     rec 1 (wav 48KHz   16bits)\r\n");
	mimic_printf("     rec 2 (wav 48KHz   24bits)\r\n");
	mimic_printf("     rec 3 (wav 96KHz   24bits)\r\n");
}
void CmdRepeat(uint32_t argc, const char *argv[])
{
	if (argc == 2)
	{
		PlayCtrlSetRepeat((enPlayCtrlRepeat_t)strtoul(argv[1], 0, 10));
	}
	else
	{
		mimic_printf("USAGE:\r\n");
		mimic_printf("     repeat 0 (off)\r\n");
		mimic_printf("     repeat 1 (one)\r\n");
		mimic_printf("     repeat 2 (all)\r\n");
	}
}

void CmdMode(uint32_t argc, const char *argv[])
{
	if (argc == 2)
	{
		PlayCtrlSetPlayArea((enPlayCtrlPlayArea_t)strtoul(argv[1], 0, 10));
	}
	else
	{
		mimic_printf("USAGE:\r\n");
		mimic_printf("     mode 0 (Dir mode)\r\n");
		mimic_printf("     mode 1 (All mode)\r\n");
	}
}
