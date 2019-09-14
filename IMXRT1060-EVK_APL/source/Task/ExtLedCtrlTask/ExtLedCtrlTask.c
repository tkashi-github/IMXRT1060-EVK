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
#include "mimiclib.h"

#include "arm_math.h"

/**
 * @brief Task Entry
 * @param [in]  argument nouse
 * @return void
 */
DefALLOCATE_ITCM void ExtLedCtrlTask(void const *argument)
{
	uint8_t msg_prio;
	stTaskMsgBlock_t stTaskMsg = {0};
	uint32_t ExtLedVal[enPCA9685PortNum];
	_Bool bUp[enPCA9685PortNum];

	for(uint32_t i=enPCA9685PortBegin;i<=enPCA9685PortEnd;i++)
	{
		ExtLedVal[i] = 100 * ((double)i/enPCA9685PortNum);
		bUp[i] = true;
	}

	if(false == DrvPCA9685Init(LPI2C1))
	{
		mimic_printf("[%s (%d)] DrvPCA9685Init NG!\r\n", __func__, __LINE__);
	}

	for(;;)
	{
		if (osOK == osMessageQueueGet(g_mqidExtLedCtrlTask, &stTaskMsg, &msg_prio, 10))
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
		else
		{
			for(uint32_t i=enPCA9685PortBegin;i<=enPCA9685PortEnd;i++)
			{
				DrvPCA9685SetPWMVal(LPI2C1, (enPCA9685PortNo_t)i, 100*arm_sin_f32(PI*ExtLedVal[i]/100.0));
				if(bUp[i] != false)
				{
					if(ExtLedVal[i] < 100)
					{
						ExtLedVal[i]++;
					}
					else
					{
						bUp[i] = false;
					}
				}
				else
				{
					if(ExtLedVal[i] > 0)
					{
						ExtLedVal[i]--;
					}
					else
					{
						bUp[i] = true;
					}
				}
			}
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


void CmdExtLed(uint32_t argc, const char *argv[])
{
	uint32_t val=4095;

	if(argc == 1)
	{
		for(uint32_t k=0;k<3;k++)
		{
			for(uint32_t j=0;j<100;j++)
			{
				val = j;
				for(uint32_t i=enPCA9685PortBegin;i<=enPCA9685PortEnd;i++)
				{
					PostMsgExtLedCtrlTaskLedVal((enPCA9685PortNo_t)i, val);
				}
				osDelay(10);
			}
			for(uint32_t j=100;j>0;j--)
			{
				val = j;
				for(uint32_t i=enPCA9685PortBegin;i<=enPCA9685PortEnd;i++)
				{
					PostMsgExtLedCtrlTaskLedVal((enPCA9685PortNo_t)i, val);
				}
				osDelay(10);
			}
		}
	}
	else if(argc > 1)
	{
		val = mimic_strtoul(argv[1], 32, 10);
		mimic_printf("[%s (%d)] %s, %d\r\n", __func__, __LINE__, argv[1], val);
		for(uint32_t i=enPCA9685PortBegin;i<=enPCA9685PortEnd;i++)
		{
			PostMsgExtLedCtrlTaskLedVal((enPCA9685PortNo_t)i, val);
		}
	}

	
}
