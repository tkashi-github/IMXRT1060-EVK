/**
 * @file		MeterTask.c
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

#include "Task/MeterTask/MeterTask.h"
#include "lvgl.h"
#include "arm_math.h"
#include "mimiclib.h"

#define kPeekdbMin (-40.0)

DefALLOCATE_DATA_DTCM static float64_t s_dfpLastPeekdB[2] = {kPeekdbMin, kPeekdbMin};
DefALLOCATE_DATA_DTCM static uint32_t s_u32LastPCMMax = INT16_MAX;

static _Bool s_bRun = false;

DefALLOCATE_BSS_DTCM static lv_style_t s_style_Lmeter;
DefALLOCATE_BSS_DTCM static lv_style_t s_style_Rmeter;
DefALLOCATE_DATA_DTCM static lv_obj_t *s_pLmeter=NULL;
DefALLOCATE_DATA_DTCM static lv_obj_t *s_pRmeter=NULL;

void CreatePeekMeter(void)
{
	/*Create a style for the line meter*/

	lv_style_copy(&s_style_Lmeter, &lv_style_pretty_color);
	s_style_Lmeter.line.width = 2;
	s_style_Lmeter.line.color = LV_COLOR_SILVER;
	s_style_Lmeter.body.main_color = lv_color_hex(0x91bfed); /*Light blue*/
	s_style_Lmeter.body.grad_color = lv_color_hex(0x04386c); /*Dark blue*/
	s_style_Lmeter.body.padding.left = 16;					 /*Line length*/

	/*Create a line meter */
	s_pLmeter = lv_lmeter_create(lv_scr_act(), NULL);
	lv_lmeter_set_range(s_pLmeter, 0, 100);								   /*Set the range*/
	lv_lmeter_set_value(s_pLmeter, 0);									   /*Set the current value*/
	lv_lmeter_set_scale(s_pLmeter, 120, 25);							   /*Set the angle and number of lines*/
	lv_lmeter_set_style(s_pLmeter, LV_LMETER_STYLE_MAIN, &s_style_Lmeter); /*Apply the new style*/
	lv_obj_set_size(s_pLmeter, 150, 150);
	lv_obj_align(s_pLmeter, NULL, LV_ALIGN_IN_TOP_LEFT, 75, 200);

	lv_style_copy(&s_style_Rmeter, &lv_style_pretty_color);
	s_style_Rmeter.line.width = 2;
	s_style_Rmeter.line.color = LV_COLOR_SILVER;
	s_style_Rmeter.body.main_color = lv_color_hex(0x91bfed); /*Light blue*/
	s_style_Rmeter.body.grad_color = lv_color_hex(0x04386c); /*Dark blue*/
	s_style_Rmeter.body.padding.left = 16;					 /*Line length*/

	/*Create a line meter */
	s_pRmeter = lv_lmeter_create(lv_scr_act(), NULL);
	lv_lmeter_set_range(s_pRmeter, 0, 100);								   /*Set the range*/
	lv_lmeter_set_value(s_pRmeter, 0);									   /*Set the current value*/
	lv_lmeter_set_scale(s_pRmeter, 120, 25);							   /*Set the angle and number of lines*/
	lv_lmeter_set_style(s_pRmeter, LV_LMETER_STYLE_MAIN, &s_style_Rmeter); /*Apply the new style*/
	lv_obj_set_size(s_pRmeter, 150, 150);
	lv_obj_align(s_pRmeter, NULL, LV_ALIGN_IN_TOP_LEFT, 275, 200);
}

static inline float64_t GetPeekdBFS(float64_t dfpPrevPeekdBFS)
{
	float64_t dfpTemp = dfpPrevPeekdBFS;
	dfpTemp -= 0.012 * DEF_PEEK_METER_REFRESH_RATE;

	return dfpTemp;
}

DefALLOCATE_ITCM static void UpdatePeekMeter(uint32_t u32Peek[])
{

	float64_t dfpTmp[2];
	float64_t dfpPeekdB[2] = {kPeekdbMin, kPeekdbMin};

	dfpTmp[0] = GetPeekdBFS(s_dfpLastPeekdB[0]);
	dfpTmp[1] = GetPeekdBFS(s_dfpLastPeekdB[1]);
	s_u32LastPCMMax = u32Peek[2];
	if (u32Peek[0] > 0)
	{
		dfpPeekdB[0] = 20 * log10((float64_t)u32Peek[0] / (float64_t)s_u32LastPCMMax);
	}
	if (u32Peek[1] > 0)
	{
		dfpPeekdB[1] = 20 * log10((float64_t)u32Peek[1] / (float64_t)s_u32LastPCMMax);
	}

	if (dfpPeekdB[0] < dfpTmp[0])
	{
		dfpPeekdB[0] = dfpTmp[0];
	}
	if (dfpPeekdB[1] < dfpTmp[1])
	{
		dfpPeekdB[1] = dfpTmp[1];
	}

	if (dfpPeekdB[0] < kPeekdbMin)
	{
		dfpPeekdB[0] = kPeekdbMin;
	}
	if (dfpPeekdB[1] < kPeekdbMin)
	{
		dfpPeekdB[1] = kPeekdbMin;
	}
	s_dfpLastPeekdB[0] = dfpPeekdB[0];
	s_dfpLastPeekdB[1] = dfpPeekdB[1];

	dfpTmp[0] /= -kPeekdbMin;
	dfpTmp[1] /= -kPeekdbMin;
	dfpTmp[0] *= 100;
	dfpTmp[1] *= 100;
	dfpTmp[0] += 100.5;
	dfpTmp[1] += 100.5;
	if (dfpTmp[0] <= 1.0)
	{
		dfpTmp[0] = 0.0;
	}
	if (dfpTmp[1] <= 1.0)
	{
		dfpTmp[1] = 0.0;
	}
	if (dfpTmp[0] > 100.0)
	{
		dfpTmp[0] = 100.0;
	}
	if (dfpTmp[1] > 100.0)
	{
		dfpTmp[1] = 100.0;
	}
	if(s_pLmeter != NULL)
	{
		lv_lmeter_set_value(s_pLmeter,dfpTmp[0]);
	}
	if(s_pRmeter != NULL)
	{
		lv_lmeter_set_value(s_pRmeter,dfpTmp[1]);
	}
}
DefALLOCATE_ITCM static void UpdatePeekMeter_NoInput(void)
{
	uint32_t u32Peek[3] = {1, 1, s_u32LastPCMMax};

	UpdatePeekMeter(u32Peek);
}
/**
 * @brief LEDTask
 * @param [in]  argument nouse
 * @return void
 */
DefALLOCATE_ITCM void MeterTask(void const *argument)
{
	TickType_t tick;

	tick = xTaskGetTickCount();
	for (;;)
	{
		if (s_bRun)
		{
			uint8_t msg_prio;
			uint32_t u32Peek[3];
			if (osOK == osMessageQueueGet(g_mqidMeterTask, u32Peek, &msg_prio, 0))
			{
				UpdatePeekMeter(u32Peek);
			}
		}
		else
		{
			if ((s_dfpLastPeekdB[0] >= kPeekdbMin) || (s_dfpLastPeekdB[1] >= kPeekdbMin))
			{
				UpdatePeekMeter_NoInput();
			}
			else
			{
				/* NOP */
			}
		}
		vTaskDelayUntil((TickType_t *const) & tick, 1000 / DEF_PEEK_METER_REFRESH_RATE); // 50FPS
	}
}

DefALLOCATE_ITCM static _Bool PostMsgMeterTaskPeekStereo(int32_t i32RMax, int32_t i32LMax, uint32_t u32Max)
{
	_Bool bret = false;

	uint32_t u32Peek[3];
	u32Peek[0] = (uint32_t)i32RMax;
	u32Peek[1] = (uint32_t)i32LMax;
	u32Peek[2] = u32Max;
	if (osOK == osMessageQueuePut(g_mqidMeterTask, &u32Peek, 0, 50))
	{
		bret = true;
	}
	return bret;
}

DefALLOCATE_ITCM _Bool PostMsgMeterTaskPeek16BitStereo(int16_t i16RMax, int16_t i16LMax)
{
	return PostMsgMeterTaskPeekStereo(i16RMax, i16LMax, INT16_MAX);
}
DefALLOCATE_ITCM _Bool PostMsgMeterTaskPeek24BitStereo(int32_t i32RMax, int32_t i32LMax)
{
	return PostMsgMeterTaskPeekStereo(i32RMax, i32LMax, INT32_MAX);
}
DefALLOCATE_ITCM _Bool PostMsgMeterTaskPeek32BitStereo(int32_t i32RMax, int32_t i32LMax)
{
	return PostMsgMeterTaskPeekStereo(i32RMax, i32LMax, INT32_MAX);
}
DefALLOCATE_ITCM void MeterTaskRestart(void)
{
	s_bRun = false;
	osMessageQueueReset(g_mqidMeterTask);
	s_bRun = true;
}

DefALLOCATE_ITCM void MeterTaskRun(void)
{
	s_bRun = true;
}

DefALLOCATE_ITCM void MeterTaskStop(void)
{
	s_bRun = false;
	osMessageQueueReset(g_mqidMeterTask);
}
