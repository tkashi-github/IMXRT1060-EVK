/*
 * Copyright 2016-2018 NXP Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
/**
 * @file    IMXRT1060-EVK_APL.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MIMXRT1062.h"
#include "UserTypedef.h"
#include "OSResource.h"
#include "CPUFunc.h"
#include "UART/DrvLPUART.h"
#include "CameraTask/CameraTask.h"
#include "ELCDIF/DrvELCDIF.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern void CMSIS_OS2_SysTick_Handler (void);
extern void vPortSVCHandler( void );
extern void xPortPendSVHandler( void );
extern void prvForceHeapInit(void);
#ifdef __cplusplus
}
#endif
static void InitGpio(void){
	gpio_pin_config_t userCaedDetect;

	userCaedDetect.direction = kGPIO_DigitalInput;
	userCaedDetect.outputLogic = 0;
	userCaedDetect.interruptMode = kGPIO_NoIntmode;

	GPIO_PinInit(BOARD_USDHC_CD_GPIO_BASE, BOARD_USDHC_CD_GPIO_PIN, &userCaedDetect);
	GPIO_PinInit(BOARD_USER_BUTTON_GPIO, BOARD_USER_BUTTON_GPIO_PIN, &userCaedDetect);

}
/*
 * @brief   Application entry point.
 */
int main(void) {

  
  	/* Init board hardware. */
    BOARD_InitBootClocks();
//	BOARD_I2C_ReleaseBus();
    BOARD_InitBootPins();

	InitGpio();
	SysTick_Config(SystemCoreClock / 1000u);
	InstallIRQHandler(SysTick_IRQn, (uint32_t)CMSIS_OS2_SysTick_Handler);
	//EnableIRQ(SysTick_IRQn);
	//NVIC_SetPriority(SVCall_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	InstallIRQHandler(SVCall_IRQn, (uint32_t)vPortSVCHandler);
	//EnableIRQ(SVCall_IRQn);
	//NVIC_SetPriority(PendSV_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
	InstallIRQHandler(PendSV_IRQn, (uint32_t)xPortPendSVHandler);
	//EnableIRQ(PendSV_IRQn);
	NVIC_SetPriority(GPIO1_Combined_0_15_IRQn, kIRQ_PRIORITY_GPIO);
	BOARD_InitBootPeripherals();

	
	while(osOK != osKernelInitialize());

	CreateTask();
	CreateQueue();
	CreateBinarySemaphore();
	CreateEventGroup();
	CreateStreamBuffer();
	CreateMsgQueue();
	
	{
		lpuart_config_t lpuart_config;
		LPUART_GetDefaultConfig(&lpuart_config);
		lpuart_config.baudRate_Bps = 115200;
		DrvLPUARTInit(enLPUART1, &lpuart_config);
	}
	{
		/*Clock setting for LPI2C*/
		BOARD_LPI2C_Init(LPI2C1, BOARD_CODEC_I2C_CLOCK_FREQ);
	}
	osKernelStart();

	for(;;);
    return 0 ;
}

#define DefSDCardDetectTime	(100u)
#include "StorageTask/StorageTask.h"

/** GPIOのチャタリング対策 */
DefALLOCATE_ITCM static void GPIOMonitor(void){
	/** SD1 */
	{
		static uint32_t u32LastCDPin = 0;
		static uint32_t u32CDPinCnt = 0;
		uint32_t u32NowCDPin = GPIO_PinRead(BOARD_USDHC_CD_GPIO_BASE, BOARD_USDHC_CD_GPIO_PIN);

		if(u32NowCDPin != u32LastCDPin){
			u32CDPinCnt = 0;
		}else{
			if(u32CDPinCnt < DefSDCardDetectTime){
				u32CDPinCnt++;
			}else if(u32CDPinCnt == DefSDCardDetectTime){
				if(u32LastCDPin == 0){
					PostMsgStorageTaskInsertFromISR(true);
				}else{
					PostMsgStorageTaskInsertFromISR(false);
				}
				u32CDPinCnt++;
			}else{
				/* nop */
			}
		}
		u32LastCDPin = u32NowCDPin;
	}

	/** User SW */
	{
		static uint32_t u32LastSWPin = 0;
		static uint32_t u32SWPinCnt = 0;
		uint32_t u32NowSWPin = GPIO_PinRead(BOARD_USER_BUTTON_GPIO, BOARD_USER_BUTTON_GPIO_PIN);

		if(u32NowSWPin != u32LastSWPin){
			u32SWPinCnt = 0;
		}else{
			if(u32SWPinCnt < 100){
				u32SWPinCnt++;
			}else if(u32SWPinCnt == 100){
				if(u32LastSWPin == 0){
					PostMsgCameraTaskBtnPushed();
				}
				u32SWPinCnt++;
			}else{
				/* nop */
			}
		}
		u32LastSWPin = u32NowSWPin;
	}
	
}


/** CPU Idle用 */
#ifdef __cplusplus
extern "C"
{
#endif
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void GetRunCount(uint32_t *p32Last, uint32_t *pu32Max);
#ifdef __cplusplus
}
#endif

extern uint32_t g_u32CurrentRun;
static uint32_t s_u32LastRun = 0u;
static uint32_t s_u32RunMax = 0u;

#include "lvgl.h"
_Bool g_bInitEnd = false;
DefALLOCATE_ITCM void vApplicationTickHook(void)
{
	static uint32_t cnt = 0u;
	if(g_bInitEnd != false){
		if (cnt >= 1000u)
		{	/** 1 sec */
			cnt = 0u;
			uint32_t primask = CM7_DisableIRQ();

			s_u32LastRun = g_u32CurrentRun;
			g_u32CurrentRun = 0u;
			if (s_u32LastRun > s_u32RunMax)
			{
				s_u32RunMax = s_u32LastRun;
			}
			CM7_SetIRQ(primask);
		}else{
			cnt++;
		}
		GPIOMonitor();
	}else{
		g_u32CurrentRun = 0u;
	}

	lv_tick_inc(1);
}

void GetRunCount(uint32_t *p32Last, uint32_t *pu32Max)
{
	if ((p32Last != NULL) && (pu32Max != NULL))
	{
		uint32_t primask = CM7_DisableIRQ();
		*p32Last = s_u32LastRun;
		*pu32Max = s_u32RunMax;
		CM7_SetIRQ(primask);
	}
}

#include "TouchScreenTask/TouchScreenTask.h"

DefALLOCATE_ITCM void GPIO1_Combined_0_15_IRQHandler(void)
{
	uint32_t u32Pins = GPIO_PortGetInterruptFlags(GPIO1);

	if((u32Pins & (1<<11)) != 0){
		PostMsgTouchScreenTouchEvent();
	}
	
	GPIO_PortClearInterruptFlags(GPIO1, u32Pins);
}

