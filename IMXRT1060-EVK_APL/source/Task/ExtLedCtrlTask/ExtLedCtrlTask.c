/**
 * @file	ExtLedCtrlTask.h
 * @brief	
 * @author Takashi Kashiwagi
 * @date 2019/08/14
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
 * - 2019/08/14: Takashi Kashiwagi: v0.1 for IMXRT1060-EVK
 */
#include "ExtLedCtrlTask/ExtLedCtrlTask.h"
#include "OSResource.h"
#include "mimiclib/source/mimiclib.h"


/**
 * @brief Task Entry
 * @param [in]  argument nouse
 * @return void
 */
DefALLOCATE_ITCM void ExtLedCtrlTask(void const *argument)
{
	uint8_t msg_prio;
	stTaskMsgBlock_t stTaskMsg = {0};

	if(false == DrvPCA9685Init(LPI2C1))
	{
		mimic_printf("[%s (%d)] DrvPCA9685Init NG!\r\n", __func__, __LINE__);
	}

	if (osOK == osMessageQueueGet(g_mqidExtLedCtrlTask, &stTaskMsg, &msg_prio, portMAX_DELAY))
	{
		switch (stTaskMsg.enMsgId)
		{
		case enExtLedUpdate:
			DrvPCA9685SetPWMVal(LPI2C1, (enPCA9685PortNo_t)stTaskMsg.param[0], stTaskMsg.param[1]);
			break;
		default:
			mimic_printf("[%s (%d)] Unkown Msg!\r\n", __func__, __LINE__);
			break;
		}

		/** Sync */
		if (stTaskMsg.SyncEGHandle != NULL)
		{
			osEventFlagsSet(stTaskMsg.SyncEGHandle, stTaskMsg.wakeupbits);
		}
	}

	vTaskSuspend(NULL);
}


DefALLOCATE_ITCM _Bool PostMsgExtLedCtrlTaskLedVal(enPCA9685PortNo_t enExtLedNo, uint16_t val)
{
	_Bool bret = false;

	if((enExtLedNo >= enPCA9685Port0) && (enExtLedNo <= enPCA9685Port15))
	{
		stTaskMsgBlock_t stTaskMsg = {0};
		stTaskMsg.enMsgId = enExtLedUpdate;
		stTaskMsg.param[0] = enExtLedNo;
		stTaskMsg.param[1] = val;

		if(osOK == osMessageQueuePut(g_mqidExtLedCtrlTask, &stTaskMsg, 0, 50))
		{
			bret = true;
		}
	}

	return bret;
}

