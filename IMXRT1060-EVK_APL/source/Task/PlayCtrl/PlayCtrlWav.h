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
#include "SoundTask/SoundTask.h"
#include "AudioFile/wav/wav.h"

	extern uint8_t *OpenWavFile(const TCHAR szFilePath[], stCodecCondition_t *pst, uint32_t *pu32PCMBufferSize);
	extern uint32_t ReadWavFile(uint8_t pu8PCMBuffer[], uint32_t u32PCMBufferSize);
	extern void CloseWavFile(void);

	extern _Bool OpenRecWavFile(const TCHAR szFilePath[], const stCodecCondition_t *pst);
	extern _Bool WriteRecWavFile(const stCodecCondition_t *pst, const uint8_t pu8PCMBuffer[], uint32_t u32PCMBufferSize);
	extern void CloseRecWavFile(void);

#ifdef __cplusplus
}
#endif