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

/**  */
extern _Bool g_bInitEnd;

/**
 * @brief Starts Other Tasks
 * @param [in]  argument nouse
 * @return void
 */
void InitialTask(void const *argument)
{
		g_bInitEnd = true;
		CheckROM();
		xEventGroupWaitBits(
					g_xFSReadyEventGroup, // The event group being tested.
					0x00000001u,	  // The bits within the event group to wait for.
					pdTRUE,			  // BIT_0  should be cleared before returning.
					pdFALSE,		  // Don't wait for both bits, either bit will do.
					portMAX_DELAY);   // Wait a maximum 

		//CheckBinFile("IMXRT1050-EVKB_APL_crc16.bin");
		osThreadSuspend(osThreadGetId());
}


