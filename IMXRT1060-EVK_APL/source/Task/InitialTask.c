/**
 * @file TODO
 * @brief TODO
 * @author Takashi Kashiwagi
 * @date        2018/10/23
 * @version     0.1
 * @copyright   TODO
 * 
 * @par Update:
 * - 2018/10/23: Takashi Kashiwagi: for IMXRT1060-EVK
 */
/**  */
#include "Task/InitialTask.h"

/** Standard Header */
#include <stdint.h>
#include <stdbool.h>

/** User Typedefine */
#include "UserTypedef.h"

#include "OSResource.h"
#include "common/update.h"
#include "mimiclib/mimiclib.h"

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
		mimic_printf("[%s (%d)] All Task Started! (%lu msec)\r\n", __FUNCTION__, __LINE__, xTaskGetTickCount());
		xEventGroupWaitBits(
					g_xFSReadyEventGroup, // The event group being tested.
					0x00000001u,	  // The bits within the event group to wait for.
					pdTRUE,			  // BIT_0  should be cleared before returning.
					pdFALSE,		  // Don't wait for both bits, either bit will do.
					portMAX_DELAY);   // Wait a maximum 

		mimic_printf("[%s (%d)] Storage Init Complete (%lu msec)\r\n", __FUNCTION__, __LINE__, xTaskGetTickCount());
		osThreadSuspend(osThreadGetId());
}


