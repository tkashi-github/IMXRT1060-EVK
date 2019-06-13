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
#include "SAI/DrvSAI.h"

void SoundTask(void const *argument);


extern sai_sample_rate_t SoundTaskGetCurrentSampleRate(enSAI_t enSAI);
extern sai_word_width_t SoundTaskGetCurrentWordWidth(enSAI_t enSAI);
extern uint32_t SoundTaskGetCurrentVolume(enSAI_t enSAI);
extern _Bool SoundTaskSetCurrentVolume(enSAI_t enSAI, uint32_t u32Vol);

extern uint32_t SoundTaskReadCurrentVolume(enSAI_t enSAI);
extern _Bool SoundTaskWriteCurrentVolume(enSAI_t enSAI, uint32_t u32Vol);


extern _Bool SoundTaskDeviceStop(enSAI_t enSAI);
extern _Bool SoundTaskRxDMARestart(enSAI_t enSAI);

extern _Bool PostSyncMsgSoundTaskDeviceInit(enSAI_t enSAI, sai_sample_rate_t enSample, sai_word_width_t enWidth, _Bool bRec);
extern _Bool PostMsgSoundTaskDeviceStart(enSAI_t enSAI);
extern _Bool PostMsgSoundTaskSendAudioData(enSAI_t enSAI,  uint8_t pu8[], uint32_t u32ByteCnt);
extern _Bool PostMsgSoundTaskRcvAudioData(enSAI_t enSAI, uint8_t pu8[], uint32_t *pu32br);

extern _Bool PostMsgSoundTaskStop(enSAI_t enSAI, const char *pszStr, uint32_t u32Line);


#ifdef __cplusplus
}
#endif

