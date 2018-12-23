/**
 * @file common.c
 * @brief TODO
 * @author Takashi Kashiwagi
 * @date 2018/10/28
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
 * - 2018/10/28: Takashi Kashiwagi: v0.1 for IMXRT1060-EVK
 */
#include "common/common.h"
#include "board.h"
#include "mimiclib/mimiclib.h"
#include "ff.h"
#include "pin_mux.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"


void MemDump(uintptr_t addr, size_t ByteCnt)
{
	/** var */
	size_t i;
	size_t LoopTimes;
	uint8_t *pu8 = (uint8_t *)addr;
	LoopTimes = ByteCnt / 16u;

	if ((LoopTimes * 16u) != ByteCnt)
	{
		LoopTimes++;
	}
	for (i = 0; i < LoopTimes; i++)
	{
		size_t index = i * 16;
		size_t j;
		for (j = 0; j < 16; j++)
		{
			mimic_printf("%02X ", pu8[index + j]);
		}

		vTaskDelay(10);
		mimic_printf(" : ");
		for (j = 0; j < 16; j++)
		{
			mimic_printf("%c", mimic_isprint(pu8[index + j]) ? pu8[index + j] : '.');
		}
		mimic_printf("\r\n");
		vTaskDelay(10);
	}
	mimic_printf("-------------------------------------------------------------------\r\n");
}

void reboot(void){
	gpio_pin_config_t stGpioConfig;

	stGpioConfig.direction = kGPIO_DigitalOutput;
	stGpioConfig.outputLogic = 0;
	stGpioConfig.interruptMode = kGPIO_NoIntmode;
	IOMUXC_SetPinMux(IOMUXC_GPIO_B1_13_GPIO2_IO29, 0);
	GPIO_PinInit(GPIO2, 29, &stGpioConfig);
	uint8_t u8Out = 0;
	for(;;){
		mimic_printf("Wainting Reset (%lu msec)\r\n", osKernelGetTickCount());
		GPIO_PinWrite(GPIO2, 29, u8Out);
		u8Out ^= 1;
		vTaskDelay(2000);
	}
}

TCHAR *GetFileExt(TCHAR szExt[], const TCHAR szFilePath[], size_t SizeExt)
{
	size_t slen;
	size_t i;
	TCHAR *pret = szExt;
	szExt[0] = (TCHAR)'\0';
	slen = mimic_tcslen(szFilePath);
	if (slen > 0)
	{
		i = slen - 1;
		for (;;)
		{
			if (szFilePath[i] == (TCHAR)'.')
			{
				mimic_tcscpy(szExt, &szFilePath[i + 1], SizeExt);
				break;
			}
			if (i == 0)
			{
				break;
			}
			else
			{
				i--;
			}
		}
	}

	return pret;
}
