/**
 * @file LcdTask.h
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

#include "LcdTask/LcdTask.h"

#include "board.h"
#include "mimiclib/mimiclib.h"

#include "pin_mux.h"
#include "board.h"
#include "clock_config.h"
#include "fsl_common.h"
#include "common.h"

/** 
 * Window System 
 * https://littlevgl.com/
 * https://mcuoneclipse.com/2018/08/12/tutorial-open-source-embedded-gui-library-littlevgl-with-i-mx-rt1050-evk/
*/
#include "lvgl.h"


/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_flush_ready()' has to be called when finished
 * This function is required only when LV_VDB_SIZE != 0 in lv_conf.h*/
DefALLOCATE_ITCM static void ex_disp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t *color_p)
{
	/*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
	int32_t x;
	int32_t y;
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			/* Put a pixel to the display. */
			DrvELCDIFWritePixel(x, y, color_p->full);
			color_p++;
		}
	}
	/* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing*/
	lv_flush_ready();
}

/* Write a pixel array (called 'map') to the a specific area on the display
 * This function is required only when LV_VDB_SIZE == 0 in lv_conf.h*/
DefALLOCATE_ITCM static void ex_disp_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t *color_p)
{
	/*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
	int32_t x;
	int32_t y;

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			/* Put a pixel to the display.*/
			DrvELCDIFWritePixel(x, y, color_p->full);
			color_p++;
		}
	}
}

/* Write a pixel array (called 'map') to the a specific area on the display
 * This function is required only when LV_VDB_SIZE == 0 in lv_conf.h*/
DefALLOCATE_ITCM static void ex_disp_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
{
	/*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
	int32_t x;
	int32_t y;

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			/* Put a pixel to the display.*/
			DrvELCDIFWritePixel(x, y, color.full);
		}
	}
}

DefALLOCATE_DATA_DTCM static uint32_t s_u32PosX = 0;
DefALLOCATE_DATA_DTCM static uint32_t s_u32PosY = 0;
DefALLOCATE_DATA_DTCM static touch_event_t s_enLastTouchEvent = kTouch_Up;

/* Read the touchpad and store it in 'data'
 * Return false if no more data read; true for ready again */
DefALLOCATE_ITCM static bool ex_tp_read(lv_indev_data_t *data)
{
	/* Read the touchpad */
	if (osSemaphoreAcquire(g_bsIdMousePosition, 10) == osOK)
	{
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
		osSemaphoreRelease(g_bsIdMousePosition);
	}
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

static lv_res_t slider_action(lv_obj_t *slider)
{
	//mimic_printf("New slider value: %d\r\n", lv_slider_get_value(slider));

	return LV_RES_OK;
}
void SampleSlider(void)
{
	/*Called when a new value id set on the slider*/

	/*Create a default slider*/
	lv_obj_t *slider1 = lv_slider_create(lv_scr_act(), NULL);
	lv_obj_set_size(slider1, 160, 30);
	lv_obj_align(slider1, NULL, LV_ALIGN_IN_TOP_RIGHT, -30, 30);
	lv_slider_set_action(slider1, slider_action);
	lv_bar_set_value(slider1, 70);

	/*Create a label right to the slider*/
	lv_obj_t *slider1_label = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_text(slider1_label, "Default");
	lv_obj_align(slider1_label, slider1, LV_ALIGN_OUT_LEFT_MID, -20, 0);

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
	TickType_t tick;

	DrvELCDIFInit();

	/** Init GUI Library */
	LV_Init();

	/** Create Slider */
	SampleSlider();

	tick = xTaskGetTickCount();
	for (;;)
	{
		lv_task_handler();
		vTaskDelayUntil((TickType_t *const) & tick, 4);
	}

	vTaskSuspend(NULL);
}

DefALLOCATE_ITCM _Bool SetLcdTaskMouseMove(uint32_t u32X, uint32_t u32Y, touch_event_t enTouchEvent)
{
	_Bool bret = false;
	if (osSemaphoreAcquire(g_bsIdMousePosition, 10) == osOK)
	{
		s_u32PosY = u32X;
		s_u32PosX = u32Y;
		s_enLastTouchEvent = enTouchEvent;
		bret = true;
		osSemaphoreRelease(g_bsIdMousePosition);
	}

	return bret;
}
