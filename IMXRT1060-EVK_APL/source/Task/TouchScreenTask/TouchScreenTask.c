/**
 * @file TouchScreenTask.c
 * @brief TODO
 * @author Takashi Kashiwagi
 * @date 2019/03/10
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
 * - 2019/03/10: Takashi Kashiwagi: v0.1 for IMXRT1060-EVK
 */
#include "TouchScreenTask/TouchScreenTask.h"
#include "board.h"
#include "mimiclib/mimiclib.h"
#include "ff.h"
#include "common/common.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "fsl_ft5406_rt.h"


static ft5406_rt_handle_t s_hndFT5406;

/**
 * @brief Lcd Task Main Loop
 */
DefALLOCATE_ITCM static inline void TouchScreenTaskActual(void)
{
	stTaskMsgBlock_t stTaskMsg = {0};
	static touch_event_t s_LastTouchEvent = kTouch_Reserved;
	touch_event_t touch_event;
	
	static uint32_t s_u32LastX = 0;;
	static uint32_t s_u32LastY = 0;
	

	if (sizeof(stTaskMsg) == xStreamBufferReceive(g_sbhTouchScreenTask, &stTaskMsg, sizeof(stTaskMsg), portMAX_DELAY))
	{
		uint32_t u32PosX;
		uint32_t u32PosY;

		switch (stTaskMsg.enMsgId)
		{
		case enTouchEvent:
			touch_event = kTouch_Reserved;
			if(kStatus_Success == FT5406_RT_GetSingleTouch(&s_hndFT5406, &touch_event, (int)&u32PosX, (int)&u32PosY)){

				switch(touch_event){
				case kTouch_Down:
				case kTouch_Up:
					if(s_LastTouchEvent != touch_event){
						PostMsgLcdTaskMouseMove(u32PosX, u32PosY, touch_event);
					}
					s_u32LastX = u32PosX;
					s_u32LastY = u32PosY;
					break;
				case kTouch_Contact:
					if((s_u32LastX != u32PosX) && (s_u32LastY != u32PosY)){
						PostMsgLcdTaskMouseMove(u32PosX, u32PosY, touch_event);
					}
					s_u32LastX = u32PosX;
					s_u32LastY = u32PosY;
					break;
				default:
					break;
				}
				s_LastTouchEvent = touch_event;
			}
			break;
		default:
			mimic_printf("[%s (%d)] Unkown Msg\r\n", __FUNCTION__, __LINE__);
			break;
		}

		/** Sync */
		if (stTaskMsg.SyncEGHandle != NULL)
		{
			osEventFlagsSet(stTaskMsg.SyncEGHandle, stTaskMsg.wakeupbits);
		}

		/** Free */
		if (stTaskMsg.ptrDataForDst != (uintptr_t)NULL)
		{
			vPortFree((void *)stTaskMsg.ptrDataForDst);
		}
	}
}


/**
 * @brief TouchScreenTask Entry
 * @param [in]  argument nouse
 * @return void
 */
DefALLOCATE_ITCM void TouchScreenTask(void const *argument)
{
	gpio_pin_config_t gpio_config = {kGPIO_DigitalInput, 0, kGPIO_IntRisingEdge};

	GPIO_PinInit(DefCTP_INT_PORT, DefCTP_INT_PIN, &gpio_config);
	EnableIRQ(GPIO1_Combined_0_15_IRQn);
	GPIO_PortEnableInterrupts(DefCTP_INT_PORT, 1U << DefCTP_INT_PIN);

	s_hndFT5406.hndRtos = &g_hndI2CRTOS[1];

	if(kStatus_Success == FT5406_RT_Init(&s_hndFT5406, LPI2C1)){
		mimic_printf("[%s (%d)] FT5406_RT_Init OK\r\n", __FUNCTION__, __LINE__);
	}else{
		mimic_printf("[%s (%d)] FT5406_RT_Init NG\r\n", __FUNCTION__, __LINE__);
	}

	{
		/** Test Read */
		uint32_t u32PosX;
		uint32_t u32PosY;
		touch_event_t touch_event;
		if(kStatus_Success == FT5406_RT_GetSingleTouch(&s_hndFT5406, &touch_event, (int)&u32PosX, (int)&u32PosY)){
			mimic_printf("[%s (%d)] FT5406_RT_GetSingleTouch OK\r\n", __FUNCTION__, __LINE__);
		}else{
			mimic_printf("[%s (%d)] FT5406_RT_GetSingleTouch NG\r\n", __FUNCTION__, __LINE__);
		}
	}
	for (;;)
	{
		TouchScreenTaskActual();
	}

	vTaskSuspend(NULL);
}

DefALLOCATE_ITCM _Bool PostMsgTouchScreenTouchEvent(void)
{
	alignas(8) stTaskMsgBlock_t stTaskMsg = {0};

	stTaskMsg.enMsgId = enTouchEvent;
	if (pdFALSE != xPortIsInsideInterrupt())
	{
		BaseType_t xHigherPriorityTaskWoken;
		if (sizeof(stTaskMsg) != xStreamBufferSendFromISR(g_sbhTouchScreenTask, &stTaskMsg, sizeof(stTaskMsg), &xHigherPriorityTaskWoken))
		{
			return false;
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else
	{
		if (sizeof(stTaskMsg) != xStreamBufferSend(g_sbhTouchScreenTask, &stTaskMsg, sizeof(stTaskMsg), 50))
		{
			mimic_printf("[%s (%d)] xStreamBufferSend NG\r\n", __FUNCTION__, __LINE__);
			return false;
		}
	}
	return true;
}


void CmdCTPTest(void){
	TickType_t tick;
	mimic_printf("CTP Test\r\n");

	tick = xTaskGetTickCount();
	while (mimic_kbhit() == false)
	{
		uint32_t u32PosX;
		uint32_t u32PosY;
		touch_event_t touch_event;
		if(kStatus_Success == FT5406_RT_GetSingleTouch(&s_hndFT5406, &touch_event, (int)&u32PosX, (int)&u32PosY)){
			mimic_printf("\r[%s (%d)] X=%3u, Y=%3d\r\n", __FUNCTION__, __LINE__, u32PosX, u32PosY);
		}else{
			break;
		}
		vTaskDelayUntil((TickType_t *const) & tick, 20);
	}
}
