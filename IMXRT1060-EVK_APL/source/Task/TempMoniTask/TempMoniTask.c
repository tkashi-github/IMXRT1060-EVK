/**
 * @brief		Temperature Monitor
 * @author		Takashi Kashiwagi
 * @date		2019/03/24
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
 * - 2019/03/24: Takashi Kashiwagi: v0.1 for IMXRT1060-EVK
 */
#include "TempMoniTask/TempMoniTask.h"
#include "board.h"
#include "mimiclib/mimiclib.h"
#include "ff.h"
#include "common/common.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "fsl_tempmon.h"

static float s_sfpInternalTemperature = 0;

/**
 * @brief TempMoniTask Entry
 * @param [in]  argument nouse
 * @return void
 */
DefALLOCATE_ITCM void TempMoniTask(void const *argument)
{
	TickType_t tick;
	tempmon_config_t config;
	//EnableIRQ(TEMP_LOW_HIGH_IRQn);
	//NVIC_SetPriority(GPIO1_Combined_0_15_IRQn, kIRQ_PRIORITY_GPIO);

	TEMPMON_GetDefaultConfig(&config);
	config.frequency = 10;

	TEMPMON_Init(TEMPMON, &config);
	TEMPMON_StartMeasure(TEMPMON);

	tick = xTaskGetTickCount();
	for (;;)
	{
		s_sfpInternalTemperature = TEMPMON_GetCurrentTemperature(TEMPMON);
		vTaskDelayUntil((TickType_t *const) & tick, 100);
	}
	vTaskSuspend(NULL);
}

extern float GetTempMoniTemperature(void)
{
	return s_sfpInternalTemperature;
}
extern float ReadFromDeviceTempMoniTemperature(void)
{
	return TEMPMON_GetCurrentTemperature(TEMPMON);
}

void CmdTemp(uint32_t argc, const char *argv[])
{
	TickType_t tick;
	mimic_printf("CTP Test\r\n");

	tick = xTaskGetTickCount();
	while (mimic_kbhit() == false)
	{
		mimic_printf("\rTemp = %f", s_sfpInternalTemperature);
		vTaskDelayUntil((TickType_t *const) & tick, 100);
	}
	mimic_printf("\r\n");
}
