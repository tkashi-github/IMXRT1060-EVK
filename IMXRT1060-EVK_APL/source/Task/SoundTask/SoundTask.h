/**
 * @file		SoundTask.h
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
#error /* Only C11 */
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


extern sai_sample_rate_t SoundTaskGetCurrentSampleRate(enSoundTask_t enSoundTaskNo);
extern sai_word_width_t SoundTaskGetCurrentWordWidth(enSoundTask_t enSoundTaskNo);
extern uint32_t SoundTaskGetCurrentVolume(enSoundTask_t enSoundTaskNo);
extern _Bool SoundTaskSetCurrentVolume(enSoundTask_t enSoundTaskNo, uint32_t u32Vol);

extern uint32_t SoundTaskReadCurrentVolume(enSoundTask_t enSoundTaskNo);
extern _Bool SoundTaskWriteCurrentVolume(enSoundTask_t enSoundTaskNo, uint32_t u32Vol);


extern _Bool SoundTaskDeviceStop(enSoundTask_t enSoundTaskNo);
extern _Bool SoundTaskRxDMARestart(enSoundTask_t enSoundTaskNo);

extern _Bool PostSyncMsgSoundTaskDeviceInit(enSoundTask_t enSoundTaskNo, sai_sample_rate_t enSample, sai_word_width_t enWidth, _Bool bRec);
extern _Bool PostMsgSoundTaskDeviceStart(enSoundTask_t enSoundTaskNo);
extern _Bool PostMsgSoundTaskSendAudioData(enSoundTask_t enSoundTaskNo,  uint8_t pu8[], uint32_t u32ByteCnt);
extern _Bool PostMsgSoundTaskRcvAudioData(enSoundTask_t enSoundTaskNo, uint8_t pu8[], uint32_t *pu32br);

extern _Bool PostMsgSoundTaskStop(enSoundTask_t enSoundTaskNo, const char *pszStr, uint32_t u32Line);


#ifdef __cplusplus
}
#endif

