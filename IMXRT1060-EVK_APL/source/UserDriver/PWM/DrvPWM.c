/**
 * @file DrvPWM.c
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
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "PWM/DrvPWM.h"
#include "fsl_pwm.h"

#include "board.h"
#include "mimiclib/mimiclib.h"

#include "pin_mux.h"
#include "board.h"
#include "clock_config.h"
#include "fsl_common.h"
#include "common.h"
#include "fsl_pwm.h"

#include "pin_mux.h"
#include "fsl_xbara.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* The PWM base address */

_Bool DrvPWMInit(void)
{
	uint16_t deadTimeVal;
	pwm_signal_param_t pwmSignal[2];
	uint32_t pwmSourceClockInHz;
	uint32_t pwmFrequencyInHz = 1000;
	pwm_config_t pwmConfig;



XBARA_Init(XBARA1);
    XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm1Fault0);
    XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm1Fault1);
    XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm1234Fault2);
    XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm1234Fault3);

	PWM_GetDefaultConfig(&pwmConfig);

	/* Use full cycle reload */
	pwmConfig.reloadLogic = kPWM_ReloadPwmFullCycle;
	pwmConfig.pairOperation = kPWM_Independent;
	pwmConfig.enableDebugMode = true;



	/* Initialize submodule 3 the same way as submodule 1 */
	if (PWM_Init(BOARD_PWM_BASEADDR, kPWM_Module_3, &pwmConfig) == kStatus_Fail)
	{
		mimic_printf("PWM initialization failed\n");
		return false;
	}


	pwmSourceClockInHz = PWM_SRC_CLK_FREQ;

	memset(pwmSignal, 0, sizeof(pwmSignal));

	/* Set deadtime count, we set this to about 650ns */
	deadTimeVal = ((uint64_t)pwmSourceClockInHz * 650) / 150000000;
;
	pwmSignal[0].pwmChannel = kPWM_PwmA;
	pwmSignal[0].level = kPWM_HighTrue;
	pwmSignal[0].dutyCyclePercent = 50; /* 1 percent dutycycle */
	pwmSignal[0].deadtimeValue = deadTimeVal;

	PWM_SetupPwm(BOARD_PWM_BASEADDR, kPWM_Module_3, pwmSignal, 1, kPWM_SignedCenterAligned, pwmFrequencyInHz,
				 pwmSourceClockInHz);

	PWM_SetPwmLdok(BOARD_PWM_BASEADDR, kPWM_Control_Module_3, true);

	PWM_StartTimer(BOARD_PWM_BASEADDR, kPWM_Control_Module_3);

	/* Update duty cycles for all 3 PWM signals */
	PWM_UpdatePwmDutycycle(BOARD_PWM_BASEADDR, kPWM_Module_3, kPWM_PwmA, kPWM_SignedCenterAligned, pwmSignal[0].dutyCyclePercent);

	/* Set the load okay bit for all submodules to load registers from their buffer */
	PWM_SetPwmLdok(BOARD_PWM_BASEADDR, kPWM_Control_Module_3, true);
	return true;
}
