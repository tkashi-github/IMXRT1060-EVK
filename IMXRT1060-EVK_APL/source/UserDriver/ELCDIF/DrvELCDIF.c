/**
 * @file DrvELCDIF.c
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


#include "ELCDIF/DrvELCDIF.h"

#include "board.h"
#include "mimiclib/mimiclib.h"

#include "pin_mux.h"
#include "board.h"
#include "clock_config.h"
#include "fsl_common.h"
#include "common.h"

#include "PWM/DrvPWM.h"

/** FSL */
#include "fsl_elcdif.h"

#define DEF_HSW 41
#define DEF_HFP 4
#define DEF_HBP 8
#define DEF_VSW 10
#define DEF_VFP 4
#define DEF_VBP 2
#define DEF_POL_FLAGS (kELCDIF_DataEnableActiveHigh | kELCDIF_VsyncActiveLow | kELCDIF_HsyncActiveLow | kELCDIF_DriveDataOnRisingClkEdge)

/* Display. */
#define LCD_DISP_GPIO GPIO1
#define LCD_DISP_GPIO_PIN 2
/* Back light. */
#define LCD_BL_GPIO GPIO2
#define LCD_BL_GPIO_PIN 31


__attribute__((section(".bss.$VRAM"))) alignas(64) uint16_t g_u16frameBuffer[DEF_IMG_HEIGHT][DEF_IMG_WIDTH];


/* Initialize the LCD_DISP. */
static void DrvELCDIFInitLcd(void)
{
	/** TODO: Change to PWN */
    volatile uint32_t i = 0x100U;

    gpio_pin_config_t config = {
        kGPIO_DigitalOutput, 0,
    };

    /* Reset the LCD. */
    GPIO_PinInit(LCD_DISP_GPIO, LCD_DISP_GPIO_PIN, &config);

    GPIO_PinWrite(LCD_DISP_GPIO, LCD_DISP_GPIO_PIN, 0);

    while (i--)
    {
    }

	

    GPIO_PinWrite(LCD_DISP_GPIO, LCD_DISP_GPIO_PIN, 1);
#if 1
    /* Backlight. */
    config.outputLogic = 1;
    GPIO_PinInit(LCD_BL_GPIO, LCD_BL_GPIO_PIN, &config);
#else
	DrvPWMInit();
#endif
}

static void DrvELCDIFInitLcdifPixelClock(void)
{
    /*
     * The desired output frame rate is 60Hz. So the pixel clock frequency is:
     * (480 + 41 + 4 + 18) * (272 + 10 + 4 + 2) * 60 = 9.2M.
     * Here set the LCDIF pixel clock to 9.3M.
     */

    /*
     * Initialize the Video PLL.
     * Video PLL output clock is OSC24M * (loopDivider + (denominator / numerator)) / postDivider = 93MHz.
     */
    clock_video_pll_config_t config = {
        .loopDivider = 31, .postDivider = 8, .numerator = 0, .denominator = 0,
    };

    CLOCK_InitVideoPll(&config);

    /*
     * 000 derive clock from PLL2
     * 001 derive clock from PLL3 PFD3
     * 010 derive clock from PLL5
     * 011 derive clock from PLL2 PFD0
     * 100 derive clock from PLL2 PFD1
     * 101 derive clock from PLL3 PFD1
     */
    CLOCK_SetMux(kCLOCK_LcdifPreMux, 2);

    CLOCK_SetDiv(kCLOCK_LcdifPreDiv, 4);

    CLOCK_SetDiv(kCLOCK_LcdifDiv, 1);
}
void DrvELCDIFInit(void)
{
    const elcdif_rgb_mode_config_t config = {
        .panelWidth = DEF_IMG_WIDTH,
        .panelHeight = DEF_IMG_HEIGHT,
        .hsw = DEF_HSW,
        .hfp = DEF_HFP,
        .hbp = DEF_HBP,
        .vsw = DEF_VSW,
        .vfp = DEF_VFP,
        .vbp = DEF_VBP,
        .polarityFlags = DEF_POL_FLAGS,
        .bufferAddr = (uint32_t)g_u16frameBuffer,
        .pixelFormat = kELCDIF_PixelFormatRGB565,
        .dataBus = kELCDIF_DataBus16Bit,
    };
	DrvELCDIFInitLcd();
	DrvELCDIFInitLcdifPixelClock();

    ELCDIF_RgbModeInit(LCDIF, &config);
	memset(g_u16frameBuffer, 0, sizeof(g_u16frameBuffer));
	ELCDIF_RgbModeStart(LCDIF);

	ELCDIF_SetNextBufferAddr(LCDIF, (uint32_t)g_u16frameBuffer);
}

void DrvELCDIFFillFrameBuffer(uint16_t u16color){
	for(uint32_t i=0;i<DEF_IMG_HEIGHT;i++){
		for(uint32_t j=0;j<DEF_IMG_WIDTH;j++){
			g_u16frameBuffer[i][j] = u16color;
		}
	}
}
