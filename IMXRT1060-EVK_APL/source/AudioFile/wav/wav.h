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
#include "ff.h"

/* Wave file header */
typedef struct __attribute__((__packed__)) {
	uint8_t	    riffId[4];
	uint32_t    riffSize;
	uint8_t     waveId[4];
}stRIFFChunkDescriptor_t;

typedef struct __attribute__((__packed__)) {
	uint8_t     ckId[4];
	uint32_t    ckSize;
}stChunkHeader_t;

typedef struct __attribute__((__packed__)) {
	uint8_t     ckId[4];
	uint32_t    ckSize;
	uint8_t     type[4];
}stChunkListHeader_t;

/** Format chunk */
typedef struct __attribute__((__packed__)) {
	uint16_t    wFormatTag;
	uint16_t	nChannels;
	uint32_t    nSamplesPerSec;
	uint32_t	nAvgBytesPerSec;
	uint16_t	nBlockAlign;
	uint16_t	wBitsPerSample;
}stFormatChunkData_t;

typedef struct __attribute__((__packed__))  {
	stChunkHeader_t     header;
	stFormatChunkData_t data;
}stFormatChunk_t;

/* Data chunk */
typedef struct {
	stChunkHeader_t header;
}stDataChunk_t;

typedef struct __attribute__((__packed__)) {
	stRIFFChunkDescriptor_t header;
	stFormatChunkData_t     format;
	stDataChunk_t           data;
}stWaveFileHeader_t;

/**
 * @brief WAVFileRead
 * @param [in]  szFilePath
 * @param [out]  aui16PCM[MaxArraySize][2] PCM DataBuf
 * @param [in]  MaxArraySize
 * @return NumberOfSamples
 */
extern uint32_t WAVFileRead(const TCHAR szFilePath[], uint8_t pu8Buffer[], size_t MaxArraySize, stFormatChunkData_t *pstFormat);
extern _Bool WAVFileReadFormatChunk(const TCHAR szFilePath[], stFormatChunkData_t *pstFormat);
extern uint32_t WAVFileReadPCMData(FIL *fp, uint8_t pu8[], uint32_t BufferSize, uint32_t *pu32RemainChunk);
extern _Bool WAVCreateHeader(FIL *pWav, uint32_t sampleRate, uint16_t numChannels, uint32_t bitsPerSample);
extern _Bool WAVWritePCMData(FIL *pWav, const uint8_t pu8[], uint32_t numberOfSamples, uint32_t bps, uint32_t *pu32OutByte);
extern _Bool WAVUpdateDataSize(FIL *pWav, uint32_t u32OutByte);
#ifdef __cplusplus
}
#endif

