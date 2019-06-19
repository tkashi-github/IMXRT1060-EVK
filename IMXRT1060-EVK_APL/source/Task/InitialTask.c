/**
 * @file InitialTask.c
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
#include "Task/InitialTask.h"

/** Standard Header */
#include <stdint.h>
#include <stdbool.h>

/** User Typedefine */
#include "UserTypedef.h"

#include "OSResource.h"
#include "common/update.h"
#include "common/common.h"
#include "mimiclib/mimiclib.h"
#include "Task/LanTask/LanTask.h"

#include "SPIFlash/SPIFlash.h"

/**  */
extern _Bool g_bInitEnd;

/**
 * @brief Starts Other Tasks
 * @param [in]  argument nouse
 * @return void
 */
void InitialTask(void const *argument)
{
	vTaskDelay(2000);
	g_bInitEnd = true;
	SPIFlashInit();
	PostMsgLanTaskRestart();
	mimic_printf("[%s (%d)] All Task Started! (%lu msec)\r\n", __func__, __LINE__, xTaskGetTickCount());
	osEventFlagsWait(g_efFSReady, 1, osFlagsWaitAny, portMAX_DELAY);   // Wait a maximum 
	if(CheckBinFile("IMXRT1060-EVK_APL_crc16.bin") != false){
		f_unlink("IMXRT1060-EVK_APL_crc16_end.bin");
		f_rename("IMXRT1060-EVK_APL_crc16.bin", "IMXRT1060-EVK_APL_crc16_end.bin");
		mimic_printf("Call SYSTEM RESET\r\n");
		reboot();
	}
	mimic_printf("[%s (%d)] Storage Init Complete (%lu msec)\r\n", __func__, __LINE__, xTaskGetTickCount());
	osThreadSuspend(osThreadGetId());
}


