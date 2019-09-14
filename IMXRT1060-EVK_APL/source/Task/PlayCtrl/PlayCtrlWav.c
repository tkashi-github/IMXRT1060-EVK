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
#include "PlayCtrl/PlayCtrlWav.h"
#include "mimiclib.h"

static uint32_t s_u32RemainSize = 0;
static FIL s_filPlayWav;
static FIL s_filRecWav;
static uint32_t s_u32WavOutByteCnt = 0;

/**
 * @brief Open Wav File for Play
 * @param [in]  szFilePath FilePath
 * @param [in/out]  pst Codec Condition
 * @param [in]  pu32PCMBufferSize size of PCM Buffer
 * @return pu8PCMBuffer Pointer of PCM Buffer
 */
DefALLOCATE_ITCM uint8_t *OpenWavFile(const TCHAR szFilePath[], stCodecCondition_t *pst, uint32_t *pu32PCMBufferSize)
{
	uint32_t sizeofpcm;
	uint8_t *pu8PCMBuffer = NULL;
	stFormatChunkData_t stFormat;

	if((szFilePath == NULL) || (pst == NULL))
	{
		mimic_printf("[%s (%d)] argment NG\r\n", __func__, __LINE__);
		goto _END;
	}
	s_u32RemainSize = 0;
	if (WAVFileReadFormatChunk(szFilePath, &stFormat) == false)
	{
		mimic_printf("[%s (%d)] WAVFileReadFormatChunk NG\r\n", __func__, __LINE__);
		goto _END;
	}

	if(stFormat.wBitsPerSample > 32)
	{
		mimic_printf("[%s (%d)] wBitsPerSample NG (%lu)\r\n", __func__, __LINE__, stFormat.wBitsPerSample);
		goto _END;
	}

	sizeofpcm = stFormat.wBitsPerSample / 8;
	{
		if( (DEF_BUFFER_QUEUE_SIZE % stFormat.nChannels) == 0)
		{
			*pu32PCMBufferSize = DEF_BUFFER_SAMPLE_SIZE * DEF_BUFFER_QUEUE_SIZE * sizeofpcm;
		}
		else
		{
			*pu32PCMBufferSize = DEF_BUFFER_SAMPLE_SIZE * stFormat.nChannels * sizeofpcm;
		}
		mimic_printf("[%s (%d)] stFormat.nChannels = %lu\r\n", __func__, __LINE__, stFormat.nChannels);
		mimic_printf("[%s (%d)] *pu32PCMBufferSize = %lu [bytes]\r\n", __func__, __LINE__, *pu32PCMBufferSize);
	}

	pu8PCMBuffer = (uint8_t *)pvPortMalloc(*pu32PCMBufferSize);
	if (pu8PCMBuffer == NULL)
	{
		goto _END;
	}

	if (FR_OK != f_open(&s_filPlayWav, szFilePath, FA_READ))
	{
		vPortFree(pu8PCMBuffer);
		pu8PCMBuffer = NULL;
		goto _END;
	}
	if (FR_OK != f_lseek(&s_filPlayWav, sizeof(stRIFFChunkDescriptor_t)))
	{
		f_close(&s_filPlayWav);
		vPortFree(pu8PCMBuffer);
		pu8PCMBuffer = NULL;
		goto _END;
	}

	pst->enSample = stFormat.nSamplesPerSec;
	pst->enBitsWidth = stFormat.wBitsPerSample;
	//mimic_printf("Sample   = %d : %d\r\n", SoundTaskGetCurrentSampleRate(enSAI1), pst->enSample);
	//mimic_printf("BitWidth = %d : %d\r\n", SoundTaskGetCurrentWordWidth(enSAI1), pst->enBitsWidth);

_END:
	return pu8PCMBuffer;
}

/**
 * @brief Read Wav File for Play
 * @param [in]  pu8PCMBuffer Pointer of PCM Buffer
 * @param [in]  u32PCMBufferSize size of PCM Buffer
 * @return uint32_t Read Byte Count
 */
DefALLOCATE_ITCM uint32_t ReadWavFile(uint8_t pu8PCMBuffer[], uint32_t u32PCMBufferSize)
{
	uint32_t u32ReadByte;
	//mimic_printf("[%s (%d)] ENTER\r\n", __func__, __LINE__);
	memset(pu8PCMBuffer, 0, u32PCMBufferSize);

	/** WAVファイルからPCMの読み出し */
	u32ReadByte = WAVFileReadPCMData(&s_filPlayWav, pu8PCMBuffer, u32PCMBufferSize, &s_u32RemainSize);
	//mimic_printf("[%s (%d)] AudioData = %lu bytes\r\n", __func__, __LINE__, u32ReadByte);
	return u32ReadByte;
}

/**
 * @brief Close Wav File For Play
 */
DefALLOCATE_ITCM void CloseWavFile(void)
{
	f_close(&s_filPlayWav);
}

/**
 * @brief Open Wav File for Rec
 * @param [in]  szFilePath FilePath
 * @param [in]  pst Codec Condition
 * @return true Post OK
 * @return false Post NG
 */
DefALLOCATE_ITCM _Bool OpenRecWavFile(const TCHAR szFilePath[], const stCodecCondition_t *pst)
{

	s_u32WavOutByteCnt = 0;

	if (FR_OK != f_open(&s_filRecWav, szFilePath, FA_CREATE_ALWAYS | FA_WRITE))
	{
		mimic_printf("[%s (%d)] pvPortMalloc NG\r\n", __func__, __LINE__);
		return false;
	}
	if (WAVCreateHeader(&s_filRecWav, (uint32_t)pst->enSample, (uint32_t)pst->nChannels, (uint32_t)pst->enBitsWidth) == false)
	{
		mimic_printf("[%s (%d)] WAVCreateHeader NG\r\n", __func__, __LINE__);
		f_close(&s_filRecWav);
		return false;
	}

	return true;
}

/**
 * @brief Write PCM Data to Wav File
 * @param [in]  pst Codec Condition
 * @param [in]  pu8PCMBuffer Pointer of PCM Buffer
 * @param [in]  u32PCMBufferSize size of PCM Buffer
 * @return true OK
 * @return false NG
 */
DefALLOCATE_ITCM _Bool WriteRecWavFile(const stCodecCondition_t *pst, const uint8_t pu8PCMBuffer[], uint32_t u32PCMBufferSize)
{
	uint32_t numberOfSamples = u32PCMBufferSize;

	numberOfSamples /= (pst->enBitsWidth / 8);

	return WAVWritePCMData(&s_filRecWav, pu8PCMBuffer, numberOfSamples, pst->enBitsWidth, &s_u32WavOutByteCnt);
}

/**
 * @brief Close Rec File for Rec
 */
DefALLOCATE_ITCM void CloseRecWavFile(void)
{
	WAVUpdateDataSize(&s_filRecWav, s_u32WavOutByteCnt);
	f_close(&s_filRecWav);
}
