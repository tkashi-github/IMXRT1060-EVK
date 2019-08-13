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
#include "AudioFile/wav/wav.h"
#include "mimiclib/source/mimiclib.h"

static _Bool WaveFilaHeaderRead(FIL *pFile, stWaveFileHeader_t *pstWaveHeader);
static _Bool WaveFilaHeaderCheck(stRIFFChunkDescriptor_t *pstHeader);
static _Bool ChunkHeaderRead(FIL *pFile, stChunkHeader_t *pstChunkHeader);
static _Bool FormatChunkRead(FIL *pFile, uint32_t u32ChunkSize, stFormatChunkData_t *pstChunk);
static _Bool ChunkDataRead(FIL *pFile, uint32_t u32ChunkSize, uint8_t pu8Buffer[]);

static uint32_t ActualWAVFileRead(const TCHAR szFilePath[], uint8_t pu8Buffer[], size_t MaxArraySize, stFormatChunkData_t *pstFormat);

static _Bool WaveFilaHeaderCheck(stRIFFChunkDescriptor_t *pstHeader)
{
	_Bool bret = false;

	if ((mimic_memcmp((uintptr_t)pstHeader->riffId, (uintptr_t)"RIFF", 4) == 0) && (mimic_memcmp((uintptr_t)pstHeader->waveId, (uintptr_t)"WAVE", 4) == 0))
	{
		//mimic_printf("[%s (%d)] riffSize = %lu\r\n", __func__, __LINE__, pstHeader->riffSize);
		bret = true;
	}
	return bret;
}

static _Bool WaveFilaHeaderRead(FIL *pFile, stWaveFileHeader_t *pstWaveHeader)
{
	_Bool bret = false;
	uint32_t u32ReadByte = 0u;

	if (FR_OK == f_read(pFile, &(pstWaveHeader->header), sizeof(stRIFFChunkDescriptor_t), (UINT *)&u32ReadByte))
	{
		if (u32ReadByte == sizeof(stRIFFChunkDescriptor_t))
		{
			bret = WaveFilaHeaderCheck(&(pstWaveHeader->header));
		}
	}

	return bret;
}
static _Bool ChunkHeaderRead(FIL *pFile, stChunkHeader_t *pstChunkHeader)
{
	_Bool bret = false;
	uint32_t u32ReadByte = 0u;

	if (FR_OK == f_read(pFile, pstChunkHeader, sizeof(stChunkHeader_t), (UINT *)&u32ReadByte))
	{
		if (u32ReadByte == sizeof(stChunkHeader_t))
		{
			bret = true;
		}
	}

	return bret;
}

static _Bool FormatChunkRead(FIL *pFile, uint32_t u32ChunkSize, stFormatChunkData_t *pstChunk)
{
	_Bool bret = false;
	uint32_t u32ReadByte = 0u;
	uint8_t *pu8buf = (uint8_t *)pvPortMalloc(u32ChunkSize);

	if (pu8buf == NULL)
	{
		return false;
	}
	if (FR_OK == f_read(pFile, pu8buf, u32ChunkSize, (UINT *)&u32ReadByte))
	{
		if (u32ReadByte == u32ChunkSize)
		{
			mimic_memcpy((uintptr_t)pstChunk, (uintptr_t)pu8buf, sizeof(stFormatChunkData_t));
			bret = true;
		}
	}

	vPortFree(pu8buf);
	return bret;
}

static _Bool ChunkDataRead(FIL *pFile, uint32_t u32ChunkSize, uint8_t pu8Buffer[])
{
	_Bool bret = false;
	uint32_t u32ReadByte = 0u;

	if (FR_OK == f_read(pFile, pu8Buffer, u32ChunkSize, (UINT *)&u32ReadByte))
	{
		if (u32ReadByte == u32ChunkSize)
		{
			bret = true;
		}
	}
	else
	{
		mimic_printf("[%s (%d)] f_read NG : <%s>\r\n", __func__, __LINE__);
	}

	return bret;
}

static uint32_t ActualWAVFileRead(const TCHAR szFilePath[], uint8_t pu8Buffer[], size_t MaxArraySize, stFormatChunkData_t *pstFormat)
{
	/** var */
	FIL stFile;
	uint32_t u32ReadByte = 0u;
	stWaveFileHeader_t stWaveHeader;

	/** begin */
	if (FR_OK != f_open(&stFile, szFilePath, FA_READ))
	{
		mimic_printf("[%s (%d)] f_open NG : <%s>\r\n", __func__, __LINE__, szFilePath);
		return 0;
	}

	if (false != WaveFilaHeaderRead(&stFile, &stWaveHeader))
	{
		stChunkHeader_t stChunkHeader;
		_Bool bOK = true;

		while ((ChunkHeaderRead(&stFile, &stChunkHeader) != false) && bOK)
		{
			bOK = true;
			if (mimic_memcmp((uintptr_t)stChunkHeader.ckId, (uintptr_t)"fmt ", 4) == 0)
			{
				/** Read fmt Chunk */
				mimic_printf("[%s (%d)] Read fmt Chunk\r\n", __func__, __LINE__);

				bOK = FormatChunkRead(&stFile, stChunkHeader.ckSize, pstFormat);
			}
			else if (mimic_memcmp((uintptr_t)stChunkHeader.ckId, (uintptr_t)"data", 4) == 0)
			{
				/** Read data Chunk */
				mimic_printf("[%s (%d)] Read data Chunk(stChunkHeader.ckSize = %lu)\r\n", __func__, __LINE__, stChunkHeader.ckSize);
				if ((u32ReadByte + stChunkHeader.ckSize) > MaxArraySize)
				{
					bOK = false;
				}
				else
				{
					bOK = ChunkDataRead(&stFile, stChunkHeader.ckSize, &pu8Buffer[u32ReadByte]);
					u32ReadByte += stChunkHeader.ckSize;
				}
			}
			else
			{
				/** Skip */
				mimic_printf("[%s (%d)] Unkown Chunk : <%c%c%c%c>\r\n", __func__, __LINE__, stChunkHeader.ckId[0], stChunkHeader.ckId[1], stChunkHeader.ckId[2], stChunkHeader.ckId[3]);

				/** Dummy read */
				ChunkDataRead(&stFile, stChunkHeader.ckSize, &pu8Buffer[u32ReadByte]);
			}
		}
	}

	f_close(&stFile);
	return u32ReadByte;
}

uint32_t WAVFileRead(const TCHAR szFilePath[], uint8_t pu8Buffer[], size_t MaxArraySize, stFormatChunkData_t *pstFormat)
{
	/** var */
	uint32_t u32Samples = 0u;

	/** begin */
	if ((szFilePath != NULL) && (pu8Buffer != NULL) && (pstFormat != NULL))
	{
		uint32_t u32ReadByte;
		u32ReadByte = ActualWAVFileRead(szFilePath, pu8Buffer, MaxArraySize, pstFormat);
		if (u32ReadByte != 0)
		{
			mimic_printf("** WAVE FORMAT CHUNK INFORMATION **\r\n");
			mimic_printf("wFormatTag      = %u\r\n", pstFormat->wFormatTag);
			mimic_printf("nChannels       = %u\r\n", pstFormat->nChannels);
			mimic_printf("nSamplesPerSec  = %u\r\n", pstFormat->nSamplesPerSec);
			mimic_printf("nAvgBytesPerSec = %u\r\n", pstFormat->nAvgBytesPerSec);
			mimic_printf("nBlockAlign     = %u\r\n", pstFormat->nBlockAlign);
			mimic_printf("wBitsPerSample  = %u\r\n", pstFormat->wBitsPerSample);
			mimic_printf("***********************************\r\n");

			u32Samples = u32ReadByte;
			u32Samples /= pstFormat->nChannels;

			switch (pstFormat->wBitsPerSample)
			{
			case 16:
				u32Samples /= 2;
				break;
			case 24:
				u32Samples /= 3;
				break;
			default:
				break;
			}
			mimic_printf("[%s (%d)] Total Samples = %lu * %u\r\n", __func__, __LINE__, u32Samples, pstFormat->nChannels);
			mimic_printf("[%s (%d)] Play Time     = %f\r\n", __func__, __LINE__, (double)u32Samples / (double)pstFormat->nSamplesPerSec);
		}
	}
	else
	{
		mimic_printf("[%s (%d)] Parameter Error\r\n", __func__, __LINE__);
	}

	return u32Samples;
}

_Bool WAVFileReadFormatChunk(const TCHAR szFilePath[], stFormatChunkData_t *pstFormat)
{
	/** var */
	FIL stFile;
	_Bool bret = false;
	stWaveFileHeader_t stWaveHeader;

	/** begin */
	if (FR_OK != f_open(&stFile, szFilePath, FA_READ))
	{
		mimic_printf("[%s (%d)] f_open NG : <%s>\r\n", __func__, __LINE__, szFilePath);
		return false;
	}

	if (false != WaveFilaHeaderRead(&stFile, &stWaveHeader))
	{
		stChunkHeader_t stChunkHeader;
		bret = true;

		while ((ChunkHeaderRead(&stFile, &stChunkHeader) != false) && bret)
		{
			if (mimic_memcmp((uintptr_t)stChunkHeader.ckId, (uintptr_t)"fmt ", 4) == 0)
			{
				/** Read fmt Chunk */
				bret = FormatChunkRead(&stFile, stChunkHeader.ckSize, pstFormat);
			}
			else
			{
				/** Skip */
				//mimic_printf("[%s (%d)] Unkown Chunk : <%c%c%c%c>\r\n", __func__, __LINE__, stChunkHeader.ckId[0], stChunkHeader.ckId[1], stChunkHeader.ckId[2], stChunkHeader.ckId[3]);

				/** Dummy read */
				if (FR_OK != f_lseek(&stFile, f_tell(&stFile) + stChunkHeader.ckSize))
				{
					bret = false;
				}
			}
		}
	}

	f_close(&stFile);
	return bret;
}

/**
 * @brief WAVFileReadPCMData
 * @param [in]  fp File pointer
 * @param [out]  pu8 pointer of pcm data
 * @param [in]  BufferSize size of pcm pu8
 * @param [inout]  pu32RemainChunk pointer of RemainChunk
 * @return u32ReadByte
 */
uint32_t WAVFileReadPCMData(FIL *fp, uint8_t pu8[], uint32_t BufferSize, uint32_t *pu32RemainChunk)
{
	/** var */
	stChunkHeader_t stChunkHeader;
	uint32_t u32ReadByte = 0;

	/** begin */
	//mimic_printf("[%s (%d)] *pu32RemainChunk = %lu\r\n", __func__, __LINE__, *pu32RemainChunk);
	while (*pu32RemainChunk > 0)
	{
		if ((BufferSize - u32ReadByte) > *pu32RemainChunk)
		{
			/** pu8にchunkごと読み込める */
			if (false != ChunkDataRead(fp, *pu32RemainChunk, &pu8[u32ReadByte]))
			{
				u32ReadByte += *pu32RemainChunk;
				*pu32RemainChunk = 0;
			}
			else
			{
				break;
			}
		}
		else
		{
			/** 丸ごと読めない */
			/** 残りを読み込む */
			uint32_t RemainSize;
			RemainSize = BufferSize - u32ReadByte;
			if (false != ChunkDataRead(fp, RemainSize, &pu8[u32ReadByte]))
			{
				u32ReadByte += RemainSize;

				/** chunkに残ってるデータ数を残す */
				*pu32RemainChunk = *pu32RemainChunk - RemainSize;
			}
			return u32ReadByte;
		}
	}

	while (ChunkHeaderRead(fp, &stChunkHeader) != false)
	{
		if (mimic_memcmp((uintptr_t)stChunkHeader.ckId, (uintptr_t)"data", 4) == 0)
		{

			if ((BufferSize - u32ReadByte) > stChunkHeader.ckSize)
			{
				/** pu8にchunkごと読み込める */
				//mimic_printf("[%s (%d)] stChunkHeader.ckSize = %lu\r\n", __func__, __LINE__, stChunkHeader.ckSize);
				if (false != ChunkDataRead(fp, stChunkHeader.ckSize, &pu8[u32ReadByte]))
				{
					u32ReadByte += stChunkHeader.ckSize;
					*pu32RemainChunk = 0;
				}
				else
				{
					break;
				}
			}
			else
			{
				/** 残りを読み込む */
				uint32_t u32;
				u32 = BufferSize - u32ReadByte;
				//mimic_printf("[%s (%d)] u32 = %lu\r\n", __func__, __LINE__, u32);
				if (false != ChunkDataRead(fp, u32, &pu8[u32ReadByte]))
				{
					u32ReadByte += u32;

					/** chunkに残ってるデータ数を残す */
					*pu32RemainChunk = stChunkHeader.ckSize - u32;
				}
				break;
			}
		}
		else
		{
			/** Skip */
			//mimic_printf("[%s (%d)] Unkown Chunk(size=%lu) : <%c%c%c%c>\r\n", __func__, __LINE__, stChunkHeader.ckSize, stChunkHeader.ckId[0], stChunkHeader.ckId[1], stChunkHeader.ckId[2], stChunkHeader.ckId[3]);

			/** Dummy read */
			if (FR_OK != f_lseek(fp, f_tell(fp) + stChunkHeader.ckSize))
			{
				break;
			}
		}
	}

	return u32ReadByte;
}

_Bool WAVCreateHeader(FIL *pWav, uint32_t sampleRate, uint16_t numChannels, uint32_t bitsPerSample)
{
	stRIFFChunkDescriptor_t stRIFFcd;
	stDataChunk_t stData;
	stFormatChunk_t stFmt;
	uint32_t bw = 0;

	if (bitsPerSample != 16 && bitsPerSample != 24 && bitsPerSample != 32)
	{
		return false;
	}

	mimic_memcpy((uintptr_t)stRIFFcd.riffId, (uintptr_t)"RIFF", 4);
	stRIFFcd.riffSize = 0x7FFFFFFF;
	mimic_memcpy((uintptr_t)stRIFFcd.waveId, (uintptr_t)"WAVE", 4);
	if (FR_OK != f_write(pWav, (uint8_t *)&stRIFFcd, sizeof(stRIFFcd), (UINT *)&bw))
	{
		return false;
	}

	mimic_memcpy((uintptr_t)stFmt.header.ckId, (uintptr_t)"fmt ", 4);
	stFmt.header.ckSize = 16;
	stFmt.data.wFormatTag = 0x01u;
	stFmt.data.nChannels = numChannels;
	stFmt.data.nSamplesPerSec = sampleRate;
	stFmt.data.nBlockAlign = (uint16_t)(numChannels * (bitsPerSample >> 3));
	stFmt.data.nAvgBytesPerSec = sampleRate * stFmt.data.nBlockAlign;
	stFmt.data.wBitsPerSample = bitsPerSample;
	if (FR_OK != f_write(pWav, (uint8_t *)&stFmt, sizeof(stFmt), (UINT *)&bw))
	{
		return false;
	}

	mimic_memcpy((uintptr_t)stData.header.ckId, (uintptr_t)"data", 4);
	stData.header.ckSize = 0x7fffffff - 36; /** 12 + 16 + 8 = 36*/
	if (FR_OK != f_write(pWav, (uint8_t *)&stData, sizeof(stData), (UINT *)&bw))
	{
		return false;
	}

	return true;
}

_Bool WAVWritePCMData(FIL *pWav, const uint8_t pu8[], uint32_t numberOfSamples, uint32_t BitPerSample, uint32_t *pu32OutByte)
{
	uint32_t bw;

	if (FR_OK != f_write(pWav, pu8, numberOfSamples * (BitPerSample / 8), (UINT *)&bw))
	{
		mimic_printf("[%s (%d)] f_write NG\r\n", __func__, __LINE__);
		return false;
	}

	if (bw != (numberOfSamples * BitPerSample / 8))
	{
		mimic_printf("[%s (%d)] f_write NG\r\n", __func__, __LINE__);
		return false;
	}

	*pu32OutByte += (numberOfSamples * (BitPerSample >> 3));
	//mimic_printf("[%s (%d)] *pu32OutByte = %lu\r\n", __func__, __LINE__, *pu32OutByte);

	return true;
}

_Bool WAVUpdateDataSize(FIL *pWav, uint32_t u32OutByte)
{
	uint32_t DataSize = u32OutByte;
	uint32_t RiffSize = u32OutByte + 36;
	uint32_t bw;

	if (pWav == NULL)
	{
		return false;
	}

	if (FR_OK != f_lseek(pWav, 4))
	{
		mimic_printf("[%s (%d)] f_lseek NG\r\n", __func__, __LINE__);
	}

	if (FR_OK != f_write(pWav, &RiffSize, 4, (UINT *)&bw))
	{
		mimic_printf("[%s (%d)] f_write NG\r\n", __func__, __LINE__);
	}
	if (FR_OK != f_lseek(pWav, 40))
	{
		mimic_printf("[%s (%d)] f_lseek NG\r\n", __func__, __LINE__);
	}

	if (FR_OK != f_write(pWav, &DataSize, 4, (UINT *)&bw))
	{
		mimic_printf("[%s (%d)] f_write NG\r\n", __func__, __LINE__);
	}

	return true;
}
