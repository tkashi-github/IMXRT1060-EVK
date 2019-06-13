/**
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
#include "PlayCtrl/PlayCtrlAudio.h"
#include "SoundTask/SoundTask.h"
#include "mimiclib/mimiclibh"

static TCHAR s_szWAV[768];
static volatile _Bool s_bPlayFileOpen = false;
static volatile _Bool s_bRecFileOpen = false;

/**
 * @brief Is Play Audio File Opend?
 * @return true Play File is Open.
 * @return false Play File is Close.
 */
DefALLOCATE_ITCM _Bool IsPlayAudioFileOpend(void)
{
	return s_bPlayFileOpen;
}

/**
 * @brief Is Rec Audio File Opend?
 * @return true Rec File is Open.
 * @return false Rec File is Close.
 */
DefALLOCATE_ITCM _Bool IsRecAudioFileOpend(void)
{
	return s_bRecFileOpen;
}

/**
 * @brief Open Play Audio File
 * @param [in]  szFilePath FilePath
 * @param [in/out]  pst Codec Condition
 * @param [in]  pu32PCMBufferSize size of PCM Buffer
 * @return pu8PCMBuffer Pointer of PCM Buffer
 */
DefALLOCATE_ITCM uint8_t *OpenPlayAudioFile(const TCHAR szFilePath[], stCodecCondition_t *pst, uint32_t *pu32PCMBufferSize)
{
	uint8_t *pu8PCMBuffer = NULL;

	if ((szFilePath == NULL) ||
		(pst == NULL) ||
		(pu32PCMBufferSize == NULL))
	{
		mimic_printf("[%s (%d)] Param Error\r\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	if (s_bPlayFileOpen == false)
	{

		switch (pst->enFileType)
		{
		case enAudioFileWAV:
			pu8PCMBuffer = OpenWavFile(szFilePath, pst, pu32PCMBufferSize);
			break;
		default:
			break;
		}
		if (pu8PCMBuffer != NULL)
		{
			s_bPlayFileOpen = true;
		}
	}
	return pu8PCMBuffer;
}

/**
 * @brief Open Play Audio File
 * @param [in]  pst Codec Condition
 * @param [in]  pu8PCMBuffer Pointer of PCM Buffer
 * @param [in]  u32PCMBufferSize size of PCM Buffer
 * @return uint32_t Read Byte Count
 */
DefALLOCATE_ITCM uint32_t ReadPlayAudioFile(const stCodecCondition_t *pst, uint8_t pu8PCMBuffer[], uint32_t u32PCMBufferSize)
{
	if ((pst == NULL) || (pu8PCMBuffer == NULL))
	{
		mimic_printf("[%s (%d)] Param Error\r\n", __FUNCTION__, __LINE__);
		return 0;
	}
	switch (pst->enFileType)
	{
	case enAudioFileWAV:
		return ReadWavFile(pu8PCMBuffer, u32PCMBufferSize);
	default:
		mimic_printf("[%s (%d)] Param Error\r\n", __FUNCTION__, __LINE__);
		return 0;
	}
}

/**
 * @brief Close Play Audio File
 * @param [in]  pst Codec Condition
 * @param [in]  pu8PCMBuffer Pointer of PCM Buffer
 */
DefALLOCATE_ITCM void ClosePlayAudioFile(const stCodecCondition_t *pst, uint8_t *pu8PCMBuffer)
{
	//mimic_printf("[%s (%d)] ENTER (%lu)\r\n", __FUNCTION__, __LINE__);
	if (pst == NULL)
	{
		mimic_printf("[%s (%d)] Param Error\r\n", __FUNCTION__, __LINE__);
		return;
	}
	if (s_bPlayFileOpen != false)
	{
		switch (pst->enFileType)
		{
		case enAudioFileWAV:
			CloseWavFile();
			break;
		default:
			break;
		}

		if (pu8PCMBuffer != NULL)
		{
			vPortFree((void *)pu8PCMBuffer);
		}
		s_bPlayFileOpen = false;
	}
	//mimic_printf("[%s (%d)] EXIT (%lu)\r\n", __FUNCTION__, __LINE__);
}

/**
 * @brief Open Rec Audio File
 * @param [in]  szFilePath FilePath
 * @param [in]  pst Codec Condition
 * @param [in]  pu32PCMBufferSize size of PCM Buffer
 * @param [in]  bForceInit Init Flag
 * @return pu8PCMBuffer Pointer of PCM Buffer
 */
DefALLOCATE_ITCM uint8_t *OpenRecAudioFile(const TCHAR szFilePath[], const stCodecCondition_t *pst, uint32_t *pu32PCMBufferSize, _Bool bForceInit)
{
	uint8_t *pu8PCMBuffer = NULL;
	uint32_t sizeofpcm;

	sizeofpcm = pst->enBitsWidth / 8;
	*pu32PCMBufferSize = pst->enSample * pst->nChannels * sizeofpcm; /** とりあえず2sec確保 */
	*pu32PCMBufferSize /= DefAudioBufDiv;
	*pu32PCMBufferSize *= DefAudioBufBase;

	if (s_bRecFileOpen == false)
	{
		if (((uint32_t)SoundTaskGetCurrentSampleRate(enSAI1) != (uint32_t)pst->enSample) ||
			((uint16_t)SoundTaskGetCurrentWordWidth(enSAI1) != (uint32_t)pst->enBitsWidth) ||
			(bForceInit != false))
		{
			mimic_printf("[%s (%d)] Change SampleRate Or BitsWidth\r\n", __FUNCTION__, __LINE__);

			if (PostSyncMsgSoundTaskDeviceInit(enSAI1, (uint32_t)pst->enSample, (uint32_t)pst->enBitsWidth, true) == false)
			{
				mimic_printf("[%s (%d)] PostSyncMsgSoundTaskDeviceInit NG\r\n", __FUNCTION__, __LINE__);
				return NULL;
			}
		}

		pu8PCMBuffer = (uint8_t *)pvPortMalloc(*pu32PCMBufferSize);
		if (pu8PCMBuffer == NULL)
		{
			mimic_printf("[%s (%d)] pvPortMalloc NG\r\n", __FUNCTION__, __LINE__);
			return NULL;
		}

		/** SoundTaskがPlayt注に送っても問題ない */
		PostMsgSoundTaskDeviceStart(enSAI1);

		switch (pst->enFileType)
		{
		case enAudioFileWAV:
			if (OpenRecWavFile(szFilePath, pst) == false)
			{
				vPortFree(pu8PCMBuffer);
				pu8PCMBuffer = NULL;
			}
			break;
		default:
			break;
		}
		if (pu8PCMBuffer != NULL)
		{
			s_bRecFileOpen = true;
		}
	}
	return pu8PCMBuffer;
}

/**
 * @brief Write PCM Data to Rec Audio File
 * @param [in]  pst Codec Condition
 * @param [in]  pu8PCMBuffer Pointer of PCM Buffer
 * @param [in]  u32PCMBufferSize size of PCM Buffer
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool WriteRecAudioFile(const stCodecCondition_t *pst, const uint8_t pu8PCMBuffer[], uint32_t u32PCMBufferSize)
{
	if ((pst == NULL) || (pu8PCMBuffer == NULL) || (s_bRecFileOpen == false))
	{
		mimic_printf("[%s (%d)] Param Error\r\n", __FUNCTION__, __LINE__);
		return false;
	}

	switch (pst->enFileType)
	{
	case enAudioFileWAV:
		return WriteRecWavFile(pst, pu8PCMBuffer, u32PCMBufferSize);
	default:
		return false;
	}
}

/**
 * @brief Close Rec Audio File
 * @param [in]  pst Codec Condition
 * @param [in]  pu8PCMBuffer Pointer of PCM Buffer
 */
DefALLOCATE_ITCM void CloseRecAudioFile(const stCodecCondition_t *pst, uint8_t *pu8PCMBuffer)
{

	if (pu8PCMBuffer != NULL)
	{
		vPortFree((void *)pu8PCMBuffer);
	}
	if (s_bRecFileOpen != false)
	{
		switch (pst->enFileType)
		{
		case enAudioFileWAV:
			CloseRecWavFile();
			break;
		default:
			break;
		}
		s_bRecFileOpen = false;
	}
}
