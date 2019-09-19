/**
 * @file		MeterTask.h
 * @brief		Meter Task 
 * @author		Takashi Kashiwagi
 * @date		2019/09/18
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
 * - 2019/09/18: Takashi Kashiwagi:
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
#include "OSResource.h"

#define DEF_PEEK_METER_REFRESH_RATE	(50u)

extern void MeterTask(void const *argument);
extern _Bool PostMsgLcdMeterTask(uint32_t u32X, uint32_t u32Y);
extern void CreatePeekMeter(void);

extern _Bool PostMsgMeterTaskPeek16BitStereo(int16_t i16RMax, int16_t i16LMax);
extern _Bool PostMsgMeterTaskPeek24BitStereo(int32_t i32RMax, int32_t i32LMax);
extern _Bool PostMsgMeterTaskPeek32BitStereo(int32_t i32RMax, int32_t i32LMax);
extern  void MeterTaskRestart(void);
extern  void MeterTaskRun(void);
extern  void MeterTaskStop(void);
#ifdef __cplusplus
}
#endif
