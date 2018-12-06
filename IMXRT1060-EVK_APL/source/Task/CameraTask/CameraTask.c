/**
 * @file SensorTask.h
 * @brief TODO
 * @author Takashi Kashiwagi
 * @date 2018/11/11
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
 * - 2018/11/11: Takashi Kashiwagi: v0.1 for IMXRT1060-EVK
 */



#include "CameraTask/CameraTask.h"
#include "fsl_camera.h"
#include "fsl_camera_receiver.h"
#include "fsl_camera_device.h"

#include "board.h"

#include "fsl_gpio.h"
#include "fsl_csi.h"
#include "fsl_csi_camera_adapter.h"
#include "fsl_ov7725.h"
#include "fsl_mt9m114.h"
#include "fsl_iomuxc.h"
#include "OSResource.h"
#include "mimiclib/mimiclib.h"

/*--*/
/*
 * Copyright (c) 2017, NXP Semiconductors, Inc.
 * All rights reserved.
 *
 * 
 * SPDX-License-Identifier: BSD-3-Clause
 */
#define APP_CAMERA_HEIGHT 272
#define APP_CAMERA_WIDTH 480
#define APP_CAMERA_CONTROL_FLAGS (kCAMERA_HrefActiveHigh | kCAMERA_DataLatchOnRisingEdge)
#define APP_CAMERA_OV7725 0
#define APP_CAMERA_MT9M114 1
#define APP_CAMERA_TYPE APP_CAMERA_OV7725
static void BOARD_PullCameraResetPin(bool pullUp)
{
    /* Reset pin is connected to DCDC_3V3. */
    return;
}
static csi_resource_t csiResource = {
    .csiBase = CSI,
};

static csi_private_data_t csiPrivateData;

camera_receiver_handle_t cameraReceiver = {
    .resource = &csiResource, .ops = &csi_ops, .privateData = &csiPrivateData,
};

#if (APP_CAMERA_TYPE == APP_CAMERA_OV7725)
static void BOARD_PullCameraPowerDownPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(GPIO1, 4, 1);
    }
    else
    {
        GPIO_PinWrite(GPIO1, 4, 0);
    }
}

static ov7725_resource_t ov7725Resource = {
    .i2cSendFunc = BOARD_Camera_I2C_SendSCCB,
    .i2cReceiveFunc = BOARD_Camera_I2C_ReceiveSCCB,
    .pullResetPin = BOARD_PullCameraResetPin,
    .pullPowerDownPin = BOARD_PullCameraPowerDownPin,
    .inputClockFreq_Hz = 24000000,
};

camera_device_handle_t cameraDevice = {
    .resource = &ov7725Resource, .ops = &ov7725_ops,
};
#else
/*
 * MT9M114 camera module has PWDN pin, but the pin is not
 * connected internally, MT9M114 does not have power down pin.
 * The reset pin is connected to high, so the module could
 * not be reseted, so at the begining, use GPIO to let camera
 * release the I2C bus.
 */
static void i2c_release_bus_delay(void)
{
    uint32_t i = 0;
    for (i = 0; i < 0x200; i++)
    {
        __NOP();
    }
}

#define CAMERA_I2C_SCL_GPIO GPIO1
#define CAMERA_I2C_SCL_PIN 16
#define CAMERA_I2C_SDA_GPIO GPIO1
#define CAMERA_I2C_SDA_PIN 17

void BOARD_I2C_ReleaseBus(void)
{
    uint8_t i = 0;
    const gpio_pin_config_t pin_config = {.direction = kGPIO_DigitalOutput, .outputLogic = 1};

    CLOCK_EnableClock(kCLOCK_Iomuxc);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_01_GPIO1_IO17, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_00_GPIO1_IO16, 0U);

    GPIO_PinInit(CAMERA_I2C_SCL_GPIO, CAMERA_I2C_SCL_PIN, &pin_config);
    GPIO_PinInit(CAMERA_I2C_SDA_GPIO, CAMERA_I2C_SDA_PIN, &pin_config);

    /* Drive SDA low first to simulate a start */
    GPIO_PinWrite(CAMERA_I2C_SDA_GPIO, CAMERA_I2C_SDA_PIN, 0U);
    i2c_release_bus_delay();

    /* Send 9 pulses on SCL and keep SDA high */
    for (i = 0; i < 9; i++)
    {
        GPIO_PinWrite(CAMERA_I2C_SCL_GPIO, CAMERA_I2C_SCL_PIN, 0U);
        i2c_release_bus_delay();

        GPIO_PinWrite(CAMERA_I2C_SDA_GPIO, CAMERA_I2C_SDA_PIN, 1U);
        i2c_release_bus_delay();

        GPIO_PinWrite(CAMERA_I2C_SCL_GPIO, CAMERA_I2C_SCL_PIN, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    /* Send stop */
    GPIO_PinWrite(CAMERA_I2C_SCL_GPIO, CAMERA_I2C_SCL_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(CAMERA_I2C_SDA_GPIO, CAMERA_I2C_SDA_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(CAMERA_I2C_SCL_GPIO, CAMERA_I2C_SCL_PIN, 1U);
    i2c_release_bus_delay();

    GPIO_PinWrite(CAMERA_I2C_SDA_GPIO, CAMERA_I2C_SDA_PIN, 1U);
    i2c_release_bus_delay();
}

static mt9m114_resource_t mt9m114Resource = {
    .i2cSendFunc = BOARD_Camera_I2C_Send,
    .i2cReceiveFunc = BOARD_Camera_I2C_Receive,
    .pullResetPin = BOARD_PullCameraResetPin,
    .inputClockFreq_Hz = 24000000,
};

camera_device_handle_t cameraDevice = {
    .resource = &mt9m114Resource, .ops = &mt9m114_ops,
};
#endif


extern void CSI_DriverIRQHandler(void);

void CSI_IRQHandler(void)
{
    CSI_DriverIRQHandler();
}

void BOARD_InitCameraResource(void)
{
    BOARD_Camera_I2C_Init();

    /* CSI MCLK select 24M. */
    /*
     * CSI clock source:
     *
     * 00 derive clock from osc_clk (24M)
     * 01 derive clock from PLL2 PFD2
     * 10 derive clock from pll3_120M
     * 11 derive clock from PLL3 PFD1
     */
    CLOCK_SetMux(kCLOCK_CsiMux, 0);
    /*
     * CSI clock divider:
     *
     * 000 divide by 1
     * 001 divide by 2
     * 010 divide by 3
     * 011 divide by 4
     * 100 divide by 5
     * 101 divide by 6
     * 110 divide by 7
     * 111 divide by 8
     */
    CLOCK_SetDiv(kCLOCK_CsiDiv, 0);

    /*
     * For RT1060, there is not dedicate clock gate for CSI MCLK, it use CSI
     * clock gate.
     */

    /* Set the pins for CSI reset and power down. */
    gpio_pin_config_t pinConfig = {
        kGPIO_DigitalOutput, 1,
    };

    GPIO_PinInit(GPIO1, 4, &pinConfig);
}
/*--*/

__attribute__((section(".NonCacheable"))) alignas(64) static uint16_t s_frameBuffer[APP_CAMERA_HEIGHT][APP_CAMERA_WIDTH];

static void CameraTaskActual(void){


	stTaskMsgBlock_t stTaskMsg = {0};
	if (sizeof(stTaskMsg) == xStreamBufferReceive(g_sbhCameraTask, &stTaskMsg, sizeof(stTaskMsg), portMAX_DELAY))
	{
		switch (stTaskMsg.enMsgId)
		{
		case enCameraBtn:
			CAMERA_DEVICE_Start(&cameraDevice);

			/* Submit the empty frame buffers to buffer queue. */
			CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, (uint32_t)s_frameBuffer);

			CAMERA_RECEIVER_Start(&cameraReceiver);
			/*
			* The LCDIF has active buffer and inactive buffer, so get two buffers here.
			*/
			/* Wait to get the full frame buffer to show. */
			uint32_t u32Bits = osEventFlagsWait(g_efCameraSensor, 1, osFlagsWaitAny, 100);
			if(u32Bits == 1){
				mimic_printf("[%s (%d)] osEventFlagsWait OK\r\n", __FUNCTION__, __LINE__);
			}else{
				mimic_printf("[%s (%d)] osEventFlagsWait NG\r\n", __FUNCTION__, __LINE__);
			}
			CAMERA_RECEIVER_Stop(&cameraReceiver);
			CAMERA_DEVICE_Stop(&cameraDevice);
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
	}
}

static void CallBackCameraDriverReceived(camera_receiver_handle_t *handle, status_t status, void *userData){
	osEventFlagsSet(g_efCameraSensor, 1);
}

void CameraTask(void const *argument){

	mimic_printf("\r\n[%s (%d)] Start\r\n", __FUNCTION__, __LINE__);
    BOARD_InitCameraResource();
	status_t sts;

    const camera_config_t cameraConfig = {
        .pixelFormat = kVIDEO_PixelFormatRGB565,
        .bytesPerPixel = 2,
        .resolution = FSL_VIDEO_RESOLUTION(APP_CAMERA_WIDTH, APP_CAMERA_HEIGHT),
        .frameBufferLinePitch_Bytes = APP_CAMERA_WIDTH * 2,
        .interface = kCAMERA_InterfaceGatedClock,
        .controlFlags = APP_CAMERA_CONTROL_FLAGS,
        .framePerSec = 30,
    };

    memset(s_frameBuffer, 0, sizeof(s_frameBuffer));

	for(uint32_t i=0;i<0x7F;i++){
		uint8_t u8ReadBuffer[8] = {0};
		status_t sts = BOARD_LPI2C_Receive(LPI2C1, i, OV7725_PID_REG, 1, u8ReadBuffer, 2);
		if (kStatus_Success == sts)
		{
			mimic_printf("[%s (%d)] BOARD_LPI2C_Receive OK (i=%lu, 0x%02X%02X)\r\n", __FUNCTION__, __LINE__, i, u8ReadBuffer[0], u8ReadBuffer[1]);
		}
	}

	sts = CAMERA_RECEIVER_Init(&cameraReceiver, &cameraConfig, CallBackCameraDriverReceived, NULL);
    if(kStatus_Success != sts){
		mimic_printf("\r\n[%s (%d)] CAMERA_RECEIVER_Init NG (%d)\r\n", __FUNCTION__, __LINE__, sts);
	}

	sts = CAMERA_DEVICE_Init(&cameraDevice, &cameraConfig);
    if(kStatus_Success != sts){
		mimic_printf("\r\n[%s (%d)] CAMERA_DEVICE_Init NG (%d)\r\n", __FUNCTION__, __LINE__, sts);
	}

	for (;;)
	{
		CameraTaskActual();
	}
    

}

DefALLOCATE_ITCM void PostMsgCameraTaskBtnPushed(void)
{

	TickType_t tTimeout = portMAX_DELAY;
	if(pdFALSE != xPortIsInsideInterrupt()){
		tTimeout = 0;
	}

	if (osSemaphoreAcquire(g_bsIdCameraTask, tTimeout) == osOK)
	{
		stTaskMsgBlock_t stTaskMsg;
		memset(&stTaskMsg, 0, sizeof(stTaskMsgBlock_t));
		stTaskMsg.enMsgId = enCameraBtn;

		if(pdFALSE != xPortIsInsideInterrupt()){
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			xStreamBufferSendFromISR(g_sbhCameraTask, &stTaskMsg, sizeof(stTaskMsg), &xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}else{
			xStreamBufferSend(g_sbhCameraTask, &stTaskMsg, sizeof(stTaskMsg), 10);
		}
		osSemaphoreRelease(g_bsIdCameraTask);
	}
}

void CmdCamera(uint32_t argc, const char *argv[]){
}
