/**
 * @brief		GUI Library wrapper 
 * @author		Takashi Kashiwagi
 * @date		2019/03/10
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
#include "mimiclib.h"

#include "pin_mux.h"
#include "board.h"
#include "clock_config.h"
#include "fsl_common.h"
#include "common.h"

/** LCD Driver */
#include "ELCDIF/DrvELCDIF.h"

/** 
 * Window System 
 * https://littlevgl.com/
 * https://mcuoneclipse.com/2018/08/12/tutorial-open-source-embedded-gui-library-littlevgl-with-i-mx-rt1050-evk/
*/
#include "lvgl.h"
void LCD_WritePixel(int32_t x, int32_t y, uint16_t color)
{
	g_u16frameBuffer[y][x] = color;
}

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_flush_ready()' has to be called when finished
 * This function is required only when LV_VDB_SIZE != 0 in lv_conf.h*/
static void ex_disp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t *color_p)
{
	/*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
	int32_t x;
	int32_t y;
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			/* Put a pixel to the display. */
			LCD_WritePixel(x, y, color_p->full);
			color_p++;
		}
	}
	/* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing*/
	lv_flush_ready();
}

/* Write a pixel array (called 'map') to the a specific area on the display
 * This function is required only when LV_VDB_SIZE == 0 in lv_conf.h*/
static void ex_disp_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t *color_p)
{
	/*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
	int32_t x;
	int32_t y;

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			/* Put a pixel to the display.*/
			LCD_WritePixel(x, y, color_p->full);
			color_p++;
		}
	}
}

/* Write a pixel array (called 'map') to the a specific area on the display
 * This function is required only when LV_VDB_SIZE == 0 in lv_conf.h*/
static void ex_disp_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
{
	/*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
	int32_t x;
	int32_t y;

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			/* Put a pixel to the display.*/
			LCD_WritePixel(x, y, color.full);
		}
	}
}

static uint32_t s_u32PosX = 0;
static uint32_t s_u32PosY = 0;
static touch_event_t s_enLastTouchEvent = kTouch_Up;

/* Read the touchpad and store it in 'data'
 * Return false if no more data read; true for ready again */
static bool ex_tp_read(lv_indev_data_t *data)
{
	/* Read the touchpad */
	if ((s_enLastTouchEvent == kTouch_Down) || (s_enLastTouchEvent == kTouch_Contact))
	{
		data->state = LV_INDEV_STATE_PR;
	}
	else
	{
		data->state = LV_INDEV_STATE_REL;
	}
	data->point.x = s_u32PosX;
	data->point.y = s_u32PosY;
	return false; /*false: no more data to read because we are no buffering*/
}

DefALLOCATE_DATA_DTCM static lv_indev_drv_t indev_drv; /*Descriptor of an input device driver*/
DefALLOCATE_DATA_DTCM static lv_disp_drv_t disp_drv;

void LV_Init(void)
{
	lv_init();
	/*Set up the functions to access to your display*/
	disp_drv.disp_flush = ex_disp_flush; /*Used in buffered mode (LV_VDB_SIZE != 0  in lv_conf.h)*/
	disp_drv.disp_fill = ex_disp_fill;   /*Used in unbuffered mode (LV_VDB_SIZE == 0  in lv_conf.h)*/
	disp_drv.disp_map = ex_disp_map;	 /*Used in unbuffered mode (LV_VDB_SIZE == 0  in lv_conf.h)*/

#if USE_LV_GPU
	/*Optionally add functions to access the GPU. (Only in buffered mode, LV_VDB_SIZE != 0)*/
	disp_drv.mem_blend = ex_mem_blend; /*Blend two color array using opacity*/
	disp_drv.mem_fill = ex_mem_fill;   /*Fill a memory array with a color*/
#endif

	/*Finally register the driver*/
	lv_disp_drv_register(&disp_drv);

	/*************************
   * Input device interface
   *************************/
	/*Add a touchpad in the example*/
	/*touchpad_init();*/ /*Initialize your touchpad*/

	lv_indev_drv_init(&indev_drv);			/*Basic initialization*/
	indev_drv.type = LV_INDEV_TYPE_POINTER; /*The touchpad is pointer type device*/
	indev_drv.read = ex_tp_read;			/*Library ready your touchpad via this function*/
	lv_indev_drv_register(&indev_drv);		/*Finally register the driver*/
}

static lv_obj_t *slider1;

static lv_obj_t *vol_slider;

#include "Task/ExtLedCtrlTask/ExtLedCtrlTask.h"
static lv_res_t slider_action(lv_obj_t *slider)
{
	uint16_t temp = lv_slider_get_value(slider);
	for(uint32_t i=enPCA9685PortBegin;i<=enPCA9685PortEnd;i++)
	{
		PostMsgExtLedCtrlTaskLedVal((enPCA9685PortNo_t)i, temp);
	}
	return LV_RES_OK;
}
#include "Task/SoundTask/SoundTask.h"
static lv_res_t vol_slider_action(lv_obj_t *slider)
{
	uint16_t temp = lv_slider_get_value(slider);
	SoundTaskWriteCurrentVolume(enSoundTask1, temp);
	return LV_RES_OK;
}

void SampleSlider(void)
{
	/*Called when a new value id set on the slider*/

	/*Create a default slider*/
	slider1 = lv_slider_create(lv_scr_act(), NULL);
	lv_obj_set_size(slider1, 400, 30);
	lv_obj_align(slider1, NULL, LV_ALIGN_IN_TOP_RIGHT, -30, 30);
	lv_slider_set_action(slider1, slider_action);
	lv_bar_set_value(slider1, 70);
	lv_bar_set_range(slider1,0,100);

	{
		for(uint32_t i=enPCA9685PortBegin;i<=enPCA9685PortEnd;i++)
		{
			PostMsgExtLedCtrlTaskLedVal((enPCA9685PortNo_t)i, 70);
		}
	}
	/*Create a bar, an indicator and a knob style*/
	static lv_style_t style_bg;
	static lv_style_t style_indic;
	static lv_style_t style_knob;

	lv_style_copy(&style_bg, &lv_style_pretty);
	style_bg.body.main_color = LV_COLOR_BLACK;
	style_bg.body.grad_color = LV_COLOR_GRAY;
	style_bg.body.radius = LV_RADIUS_CIRCLE;
	style_bg.body.border.color = LV_COLOR_WHITE;

	lv_style_copy(&style_indic, &lv_style_pretty);
	style_indic.body.grad_color = LV_COLOR_GREEN;
	style_indic.body.main_color = LV_COLOR_LIME;
	style_indic.body.radius = LV_RADIUS_CIRCLE;
	style_indic.body.shadow.width = 10;
	style_indic.body.shadow.color = LV_COLOR_LIME;
	style_indic.body.padding.hor = 3;
	style_indic.body.padding.ver = 3;

	lv_style_copy(&style_knob, &lv_style_pretty);
	style_knob.body.radius = LV_RADIUS_CIRCLE;
	style_knob.body.opa = LV_OPA_70;
	style_knob.body.padding.ver = 10;
}


void VolumeSlider(void)
{
	/*Called when a new value id set on the slider*/

	/*Create a default slider*/
	vol_slider = lv_slider_create(lv_scr_act(), NULL);
	lv_obj_set_size(vol_slider, 400, 30);
	lv_obj_align(vol_slider, NULL, LV_ALIGN_IN_TOP_RIGHT, -30, 90);
	lv_slider_set_action(vol_slider, vol_slider_action);
	lv_bar_set_value(vol_slider, 70);
	lv_bar_set_range(vol_slider,0,100);

	SoundTaskWriteCurrentVolume(enSoundTask1, 70);

	/*Create a bar, an indicator and a knob style*/
	static lv_style_t style_bg;
	static lv_style_t style_indic;
	static lv_style_t style_knob;

	lv_style_copy(&style_bg, &lv_style_pretty);
	style_bg.body.main_color = LV_COLOR_BLACK;
	style_bg.body.grad_color = LV_COLOR_GRAY;
	style_bg.body.radius = LV_RADIUS_CIRCLE;
	style_bg.body.border.color = LV_COLOR_WHITE;

	lv_style_copy(&style_indic, &lv_style_pretty);
	style_indic.body.grad_color = LV_COLOR_GREEN;
	style_indic.body.main_color = LV_COLOR_LIME;
	style_indic.body.radius = LV_RADIUS_CIRCLE;
	style_indic.body.shadow.width = 10;
	style_indic.body.shadow.color = LV_COLOR_LIME;
	style_indic.body.padding.hor = 3;
	style_indic.body.padding.ver = 3;

	lv_style_copy(&style_knob, &lv_style_pretty);
	style_knob.body.radius = LV_RADIUS_CIRCLE;
	style_knob.body.opa = LV_OPA_70;
	style_knob.body.padding.ver = 10;
}
/**
 * @brief Task Entry
 * @param [in]  argument nouse
 * @return void
 */
DefALLOCATE_ITCM void LcdTask(void const *argument)
{
	TickType_t LastTick;
	stTaskMsgBlock_t stTaskMsg;

	LastTick = xTaskGetTickCount();
	DrvELCDIFInit();

	LV_Init();

#if 0
	{
		static lv_style_t style;
		lv_style_copy(&style, &lv_style_plain);
		style.line.width = 10;                         /*10 px thick arc*/
		style.line.color = LV_COLOR_HEX3(0x258);       /*Blueish arc color*/

		style.body.border.color = LV_COLOR_HEX3(0xBBB); /*Gray background color*/
		style.body.border.width = 10;
		style.body.padding.hor = 0;

		/*Create a Preloader object*/
		lv_obj_t *preload = lv_preload_create(lv_scr_act(), NULL);
		if(preload != NULL){
			lv_obj_set_size(preload, 100, 100);
			lv_obj_align(preload, NULL, LV_ALIGN_CENTER, 0, 0);
			//lv_preload_set_spin_time(preload, 1000);
			lv_preload_set_style(preload, LV_PRELOAD_STYLE_MAIN, &style);
		}
	}
#endif

	SampleSlider();
	VolumeSlider();
	for (;;)
	{
		uint8_t msg_prio; /* Message priority is ignored */
		if (osOK == osMessageQueueGet(g_mqidLcdTask, &stTaskMsg, &msg_prio, 4))
		{
			switch (stTaskMsg.enMsgId)
			{
			case enCurEv:
				s_u32PosY = stTaskMsg.param[0];
				s_u32PosX = stTaskMsg.param[1];
				s_enLastTouchEvent = (touch_event_t)stTaskMsg.param[2];
				break;
			default:
				break;
			}
		}

		if (LastTick != xTaskGetTickCount())
		{
			LastTick = xTaskGetTickCount();
			lv_task_handler();
		}
	}

	vTaskSuspend(NULL);
}

_Bool PostMsgLcdTaskMouseMove(uint32_t u32X, uint32_t u32Y, touch_event_t enTouchEvent)
{
	_Bool bret = false;
	stTaskMsgBlock_t stTaskMsg;
	memset(&stTaskMsg, 0, sizeof(stTaskMsgBlock_t));
	stTaskMsg.enMsgId = enCurEv;
	stTaskMsg.param[0] = u32X;
	stTaskMsg.param[1] = u32Y;
	stTaskMsg.param[2] = (uint32_t)enTouchEvent;
	if (osOK == osMessageQueuePut(g_mqidLcdTask, &stTaskMsg, 0, 10))
	{
		bret = true;
	}
	return bret;
}
