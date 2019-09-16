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
DefALLOCATE_ITCM void LCD_WritePixel(int32_t x, int32_t y, uint16_t color)
{
	g_u16frameBuffer[y][x] = color;
}

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished. */
DefALLOCATE_ITCM static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

    int32_t x;
    int32_t y;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            /* Put a pixel to the display. For example: */
            /* put_px(x, y, *color_p)*/
			g_u16frameBuffer[y][x] = color_p->full;
            color_p++;
        }
    }

    /* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}

DefALLOCATE_DATA_DTCM static uint32_t s_u32PosX = 0;
DefALLOCATE_DATA_DTCM static uint32_t s_u32PosY = 0;
DefALLOCATE_DATA_DTCM static touch_event_t s_enLastTouchEvent = kTouch_Up;

/* Will be called by the library to read the touchpad */
DefALLOCATE_ITCM static bool touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    /*Save the pressed coordinates and the state*/
    if ((s_enLastTouchEvent == kTouch_Down) || (s_enLastTouchEvent == kTouch_Contact))
	{
        data->state = LV_INDEV_STATE_PR;
    }
	else
	{
        data->state = LV_INDEV_STATE_REL;
    }

    /*Set the last pressed coordinates*/
    data->point.x = s_u32PosX;
    data->point.y = s_u32PosY;

    /*Return `false` because we are not buffering and no more data to read*/
    return false;
}

DefALLOCATE_DATA_DTCM static lv_indev_drv_t indev_drv; /*Descriptor of an input device driver*/
DefALLOCATE_DATA_DTCM static lv_disp_drv_t disp_drv;
DefALLOCATE_BSS_DTCM static lv_disp_buf_t disp_buf_2;
DefALLOCATE_BSS_DTCM static lv_color_t buf2_1[LV_HOR_RES_MAX * 10];                        /*A buffer for 10 rows*/
DefALLOCATE_BSS_DTCM static lv_color_t buf2_2[LV_HOR_RES_MAX * 10];                        /*An other buffer for 10 rows*/

void LV_Init(void)
{
	lv_init();

    lv_disp_buf_init(&disp_buf_2, buf2_1, buf2_2, LV_HOR_RES_MAX * 10);   /*Initialize the display buffer*/

	lv_disp_drv_init(&disp_drv);
    /*Set the resolution of the display*/
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;

	/*Set up the functions to access to your display*/
	disp_drv.flush_cb = disp_flush; /*Used in buffered mode (LV_VDB_SIZE != 0  in lv_conf.h)*/

#if USE_LV_GPU
	/*Optionally add functions to access the GPU. (Only in buffered mode, LV_VDB_SIZE != 0)*/
	disp_drv.mem_blend = ex_mem_blend; /*Blend two color array using opacity*/
	disp_drv.mem_fill = ex_mem_fill;   /*Fill a memory array with a color*/
#endif
	disp_drv.buffer = &disp_buf_2;

	/*Finally register the driver*/
	lv_disp_drv_register(&disp_drv);

	/*************************
   * Input device interface
   *************************/
	/*Add a touchpad in the example*/
	/*touchpad_init();*/ /*Initialize your touchpad*/

	lv_indev_drv_init(&indev_drv);			/*Basic initialization*/
	indev_drv.type = LV_INDEV_TYPE_POINTER; /*The touchpad is pointer type device*/
	indev_drv.read_cb = touchpad_read;			/*Library ready your touchpad via this function*/
	lv_indev_drv_register(&indev_drv);		/*Finally register the driver*/
}

DefALLOCATE_DATA_DTCM static lv_obj_t *slider1;
DefALLOCATE_DATA_DTCM static lv_obj_t *vol_slider;
static lv_obj_t * s_Vollabel1;
    
#include "Task/SoundTask/SoundTask.h"
DefALLOCATE_ITCM static void vol_slider_action(lv_obj_t * slider, lv_event_t event)
{
	if(event == LV_EVENT_VALUE_CHANGED) {
		uint16_t temp = lv_slider_get_value(slider);
		SoundTaskWriteCurrentVolume(enSoundTask1, temp);
		char szstr[32];
		mimic_sprintf(szstr, sizeof(szstr), "%d", temp);
		lv_label_set_text(s_Vollabel1, szstr);
	}
}

void lv_ex_label_1(void)
{
	s_Vollabel1 = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_long_mode(s_Vollabel1, LV_LABEL_LONG_BREAK);     /*Break the long lines*/
    lv_label_set_recolor(s_Vollabel1, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_align(s_Vollabel1, LV_LABEL_ALIGN_CENTER);       /*Center aligned lines*/
    lv_label_set_text(s_Vollabel1, "70");
    lv_obj_set_width(s_Vollabel1, 150);
    lv_obj_align(s_Vollabel1, NULL, LV_ALIGN_CENTER, 0, -30);
}

DefALLOCATE_ITCM void VolumeSlider(void)
{
	/*Called when a new value id set on the slider*/

	/*Create a default slider*/
	vol_slider = lv_slider_create(lv_scr_act(), NULL);
	lv_obj_set_size(vol_slider, 200, 30);
	lv_obj_align(vol_slider, NULL, LV_ALIGN_IN_TOP_RIGHT, -30, 90);
	lv_obj_set_event_cb(vol_slider, vol_slider_action);
	lv_bar_set_value(vol_slider, 70, LV_ANIM_ON);
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
    style_indic.body.padding.left = 3;
    style_indic.body.padding.right = 3;
    style_indic.body.padding.top = 3;
    style_indic.body.padding.bottom = 3;

	lv_style_copy(&style_knob, &lv_style_pretty);
	style_knob.body.radius = LV_RADIUS_CIRCLE;
	style_knob.body.opa = LV_OPA_70;
	style_knob.body.padding.top = 10;
	style_knob.body.padding.bottom = 10;
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

DefALLOCATE_ITCM _Bool PostMsgLcdTaskMouseMove(uint32_t u32X, uint32_t u32Y, touch_event_t enTouchEvent)
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
