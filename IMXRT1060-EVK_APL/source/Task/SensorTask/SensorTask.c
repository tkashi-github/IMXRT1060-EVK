/**
 * @file SensorTask.h
 * @brief TODO
 * @author Takashi Kashiwagi
 * @date 2018/11/11
 * @version     0.1
 * @details 
 * --
 * License Type <MIT License>
 * --
 * Copyright 2018 Takashi Kashiwagi
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
 * - 2018/11/11: Takashi Kashiwagi: v0.1 for IMXRT1060-EVK
 */

#include "SensorTask/SensorTask.h"
#include "OSResource.h"
#include "FXOS8700/DrvFXOS8700.h"
#include "mimiclib/mimiclib.h"

static uint16_t s_au16Accel[3];
static uint16_t s_au16Mag[3];

static void SensorTaskReadDataFromDevice(uint16_t pu16Accel[], uint16_t pu16Mag[]){
	uint16_t au16Accel[3];
	uint16_t au16Mag[3];
	
	if(FXOS8700ReadData(au16Accel, au16Mag) == kStatus_Success){
		if (osSemaphoreAcquire(g_bsIdComboSensor, 5) == osOK)
		{
			
			mimic_memcpy(pu16Accel, au16Accel, sizeof(au16Accel));
			mimic_memcpy(pu16Mag, au16Mag, sizeof(au16Mag));
			osSemaphoreRelease(g_bsIdComboSensor);
		}
	}else{
		mimic_printf("[%s (%d)] FXOS8700ReadData NG\r\n", __FUNCTION__, __LINE__);
	}
}
/**
 * @brief Task Entry
 * @param [in]  argument nouse
 * @return void
 */
DefALLOCATE_ITCM void SensorTask(void const *argument)
{
	TickType_t tick;
	uint8_t u8sts;
	if(kStatus_Success != FXOS8700Init()){
		mimic_printf("[%s (%d)] FXOS8700Init NG\r\n", __FUNCTION__, __LINE__);
	}
	tick = xTaskGetTickCount();
	for (;;)
	{
		if(FXOS8700ReadStatus(&u8sts) == kStatus_Success){
			if(u8sts != 0u){
				SensorTaskReadDataFromDevice(s_au16Accel, s_au16Mag);
			}
		}else{
			mimic_printf("[%s (%d)] FXOS8700ReadStatus NG\r\n", __FUNCTION__, __LINE__);
		}
		vTaskDelayUntil((TickType_t *const) & tick, 100);
	}

	vTaskSuspend(NULL);
}

_Bool SensorTaskReadData(uint16_t pu16Accel[], uint16_t pu16Mag[]){
	_Bool bret = false;
	if (osSemaphoreAcquire(g_bsIdComboSensor, 5) == osOK)
	{
		bret = true;
		mimic_memcpy(pu16Accel, s_au16Accel, sizeof(s_au16Accel));
		mimic_memcpy(pu16Mag, s_au16Mag, sizeof(s_au16Mag));
		osSemaphoreRelease(g_bsIdComboSensor);
	}

	return bret;
}

void CmdSensor(uint32_t argc, const char *argv[])
{
	TickType_t tick;
	mimic_printf("\r\nCOMBO SENSOR\r\n");

	tick = xTaskGetTickCount();
	while (mimic_kbhit() == false)
	{
		uint16_t au16Accel[3];
		uint16_t au16Mag[3];

		SensorTaskReadData(au16Accel, au16Mag);
		mimic_printf("\rAccel = %5d,%5d,%5d, Mag = %5d,%5d,%5d", au16Accel[0], au16Accel[1], au16Accel[2], au16Mag[0], au16Mag[1], au16Mag[2]);
		vTaskDelayUntil((TickType_t *const) & tick, 100);
	}
	mimic_printf("\r\n");
}
