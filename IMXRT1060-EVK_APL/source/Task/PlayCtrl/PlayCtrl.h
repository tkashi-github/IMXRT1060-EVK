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
#include "AudioFile/wav/wav.h"

	typedef enum
	{
		enPlayCtrlPlayAreaDir = 0,
		enPlayCtrlPlayAreaALL,
		enPlayCtrlPlayAreaMAX,
	} enPlayCtrlPlayArea_t;

	typedef enum
	{
		enPlayCtrlRepeatOff = 0,
		enPlayCtrlRepeatOne,
		enPlayCtrlRepeatALL,
		enPlayCtrlRepeatMAX,
	} enPlayCtrlRepeat_t;

	extern void PlayCtrl(void const *argument);
	extern _Bool PostMsgPlayCtrlStart(uint32_t u32TrackNo);
	extern _Bool PostSyncMsgPlayCtrlStop(void);
	extern _Bool PostMsgPlayCtrlRec(void);
	extern void PostSyncMsgPlayCtrlPlayNext(void);
	extern void PostSyncMsgPlayCtrlPlayPrev(void);

	extern _Bool PlayCtrlSoundDeviceStop(void);
	extern _Bool PlayCtrlSetRepeat(enPlayCtrlRepeat_t enRepeat);
	extern _Bool PlayCtrlSetPlayArea(enPlayCtrlPlayArea_t enPlayArea);

	extern void CmdRepeat(uint32_t argc, const char *argv[]);
	extern void CmdMode(uint32_t argc, const char *argv[]);
	extern void CmdPlay(uint32_t argc, const char *argv[]);
	extern void CmdStop(uint32_t argc, const char *argv[]);
	extern void CmdRec(uint32_t argc, const char *argv[]);
#ifdef __cplusplus
}
#endif
