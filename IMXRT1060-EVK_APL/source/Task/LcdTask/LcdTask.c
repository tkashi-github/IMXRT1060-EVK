/**
 * @file LcdTask.h
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


/*
 * Copyright (c) 2017, NXP Semiconductors, Inc.
 * All rights reserved.
 *
 * 
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "LcdTask/LcdTask.h"

#include "board.h"
#include "mimiclib/mimiclib.h"

#include "pin_mux.h"
#include "board.h"
#include "clock_config.h"
#include "fsl_common.h"
#include "common.h"

/** LCD Driver */
#include "ELCDIF/DrvELCDIF.h"




/**
 * @brief Task Entry
 * @param [in]  argument nouse
 * @return void
 */
DefALLOCATE_ITCM void LcdTask(void const *argument)
{
	TickType_t tick;

	tick = xTaskGetTickCount();
	DrvELCDIFInit();

	for (;;)
	{
		DrvELCDIFFillFrameBuffer(tick % 65535);
		vTaskDelayUntil((TickType_t *const) & tick, 100);
	}

	vTaskSuspend(NULL);
}


_Bool PostMsgLcdTaskMouseMove(uint32_t u32X, uint32_t u32Y, touch_event_t enTouchEvent){
	_Bool bret = false;
	stTaskMsgBlock_t stTaskMsg;
	memset(&stTaskMsg, 0, sizeof(stTaskMsgBlock_t));
	stTaskMsg.enMsgId = enTouchEvent;
	stTaskMsg.param[0] = u32X;
	stTaskMsg.param[1] = u32Y;
	stTaskMsg.param[2] = (uint32_t)enTouchEvent;

	if(pdFALSE != xPortIsInsideInterrupt()){
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if(pdFALSE != xQueueSendFromISR(g_mqLcdTask, &stTaskMsg, &xHigherPriorityTaskWoken)){
			bret = true;
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}else{
		if(pdFALSE != xQueueSend(g_mqLcdTask, &stTaskMsg, 10)){
			bret = true;
		}
	}

	return bret;
}
