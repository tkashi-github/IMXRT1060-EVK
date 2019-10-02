/**
 * @file		MainWindow.c
 * @brief		MainWindow by lvgl
 * @author		Takashi Kashiwagi
 * @date		2019/09/29
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
 * - 2019/09/29: Takashi Kashiwagi:
 */
#include "GUI/MainWindow/MainWindow.h"
#include "lvgl.h"
#include "arm_math.h"
#include "mimiclib.h"
#include "Task/PlayCtrl/PlayCtrl.h"

#include "Task/SoundTask/SoundTask.h"
DefALLOCATE_DATA_DTCM static lv_obj_t *vol_slider;
DefALLOCATE_ITCM static void vol_slider_action(lv_obj_t *slider, lv_event_t event)
{
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		uint16_t temp = lv_slider_get_value(slider);
		SoundTaskWriteCurrentVolume(enSoundTask1, temp);
		char szstr[32];
		mimic_sprintf(szstr, sizeof(szstr), "%d", temp);
	}
}
void MainWindowCreateVolumeSlider(void)
{
	/*Called when a new value id set on the slider*/

	/*Create a default slider*/
	vol_slider = lv_slider_create(lv_scr_act(), NULL);
	lv_obj_set_size(vol_slider, 300, 30);
	lv_obj_set_pos(vol_slider, 150, 5);
	lv_obj_set_event_cb(vol_slider, vol_slider_action);
	lv_bar_set_value(vol_slider, 70, LV_ANIM_ON);
	lv_bar_set_range(vol_slider, 0, 100);

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

DefALLOCATE_ITCM static void StartBtnOnClecked(lv_obj_t *obj, lv_event_t event)
{
	if (LV_EVENT_PRESSED == event)
	{
		mimic_printf("[%s (%d)] TP\r\n", __func__, __LINE__);
		PostMsgPlayCtrlStart(UINT32_MAX);
	}
}
DefALLOCATE_ITCM static void StopBtnOnClecked(lv_obj_t *obj, lv_event_t event)
{
	if (LV_EVENT_PRESSED == event)
	{
		mimic_printf("[%s (%d)] TP\r\n", __func__, __LINE__);
		PostMsgPlayCtrlStop();
	}
}
DefALLOCATE_ITCM static void NextBtnOnClecked(lv_obj_t *obj, lv_event_t event)
{
	if (LV_EVENT_PRESSED == event)
	{
		mimic_printf("[%s (%d)] TP\r\n", __func__, __LINE__);
		PostMsgPlayCtrlNext();
	}
}
DefALLOCATE_ITCM static void PrevBtnOnClecked(lv_obj_t *obj, lv_event_t event)
{
	if (LV_EVENT_PRESSED == event)
	{
		mimic_printf("[%s (%d)] TP\r\n", __func__, __LINE__);
		PostMsgPlayCtrlPrev();
	}
}

void MainWindowCreatePlayStopBtn(void)
{
	static lv_style_t style_btn_rel;				 /*Styles can't be local variables*/
	lv_style_copy(&style_btn_rel, &lv_style_pretty); /*Copy a built-in style as a starting point*/
	style_btn_rel.body.radius = 0;					 /*Fully round corners*/
	style_btn_rel.body.main_color = LV_COLOR_BLACK;  /*White main color*/
	style_btn_rel.body.grad_color = LV_COLOR_GRAY;   /*Blue gradient color*/
	style_btn_rel.body.border.width = 0;
	style_btn_rel.text.color = LV_COLOR_WHITE; /*Red text color */
	style_btn_rel.text.letter_space = 10;	  /*10 px letter space*/

	static lv_style_t style_btn_pr;									/*Styles can't be local variables*/
	lv_style_copy(&style_btn_pr, &lv_style_pretty);					/*Copy a built-in style as a starting point*/
	style_btn_pr.body.radius = 0;									/*Fully round corners*/
	style_btn_pr.body.main_color = LV_COLOR_MAKE(0x40, 0x40, 0x40); /*White main color*/
	style_btn_pr.body.grad_color = LV_COLOR_GRAY;					/*Blue gradient color*/
	style_btn_pr.body.border.width = 0;
	style_btn_pr.text.color = LV_COLOR_WHITE; /*Red text color */
	style_btn_pr.text.letter_space = 10;	  /*10 px letter space*/

	/*Add a button*/
	static lv_obj_t *StartBtn;
	StartBtn = lv_btn_create(lv_scr_act(), NULL); /*Add to the active screen*/
	lv_btn_set_style(StartBtn, LV_BTN_STYLE_PR, &style_btn_pr);
	lv_btn_set_style(StartBtn, LV_BTN_STYLE_REL, &style_btn_rel);
	lv_obj_set_pos(StartBtn, 5, 5);	/*Adjust the position*/
	lv_obj_set_size(StartBtn, 90, 45); /* set size of button */
	lv_obj_set_event_cb(StartBtn, StartBtnOnClecked);
	/*Add text*/
	static lv_obj_t *pBtnLabelStart;
	pBtnLabelStart = lv_label_create(StartBtn, NULL); /*Put on 'btn1'*/
	lv_label_set_text(pBtnLabelStart, LV_SYMBOL_PLAY);

	/*Add a button*/
	static lv_obj_t *StopBtn;
	StopBtn = lv_btn_create(lv_scr_act(), NULL); /*Add to the active screen*/
	lv_btn_set_style(StopBtn, LV_BTN_STYLE_PR, &style_btn_pr);
	lv_btn_set_style(StopBtn, LV_BTN_STYLE_REL, &style_btn_rel);
	lv_obj_set_pos(StopBtn, 5, 55);   /*Adjust the position*/
	lv_obj_set_size(StopBtn, 90, 45); /* set size of button */
	lv_obj_set_event_cb(StopBtn, StopBtnOnClecked);

	/*Add text*/
	static lv_obj_t *pBtnLabelStop;
	pBtnLabelStop = lv_label_create(StopBtn, NULL); /*Put on 'btn1'*/
	lv_label_set_text(pBtnLabelStop, LV_SYMBOL_STOP);

	/*Add a button*/
	static lv_obj_t *NextBtn;
	NextBtn = lv_btn_create(lv_scr_act(), NULL); /*Add to the active screen*/
	lv_btn_set_style(NextBtn, LV_BTN_STYLE_PR, &style_btn_pr);
	lv_btn_set_style(NextBtn, LV_BTN_STYLE_REL, &style_btn_rel);
	lv_obj_set_pos(NextBtn, 5, 105);  /*Adjust the position*/
	lv_obj_set_size(NextBtn, 90, 45); /* set size of button */
	lv_obj_set_event_cb(NextBtn, NextBtnOnClecked);
	/*Add text*/
	static lv_obj_t *pBtnLabelNext;
	pBtnLabelNext = lv_label_create(NextBtn, NULL); /*Put on 'btn1'*/
	lv_label_set_text(pBtnLabelNext, LV_SYMBOL_NEXT);

	/*Add a button*/
	static lv_obj_t *PrevBtn;
	PrevBtn = lv_btn_create(lv_scr_act(), NULL); /*Add to the active screen*/
	lv_btn_set_style(PrevBtn, LV_BTN_STYLE_PR, &style_btn_pr);
	lv_btn_set_style(PrevBtn, LV_BTN_STYLE_REL, &style_btn_rel);
	lv_obj_set_pos(PrevBtn, 5, 155);  /*Adjust the position*/
	lv_obj_set_size(PrevBtn, 90, 45); /* set size of button */
	lv_obj_set_event_cb(PrevBtn, PrevBtnOnClecked);
	/*Add text*/
	static lv_obj_t *pBtnLabelPrev;
	pBtnLabelPrev = lv_label_create(PrevBtn, NULL); /*Put on 'btn1'*/
	lv_label_set_text(pBtnLabelPrev, LV_SYMBOL_PREV);
}

DefALLOCATE_DATA_DTCM static lv_obj_t *s_pTextAreaTrackNo;
DefALLOCATE_DATA_DTCM static lv_obj_t *s_pTextAreaTrackName;
DefALLOCATE_DATA_DTCM static lv_obj_t *s_pTextAreaTrackTime;

void MainWindowCreateTextAreaTrack(void)
{
	s_pTextAreaTrackNo = lv_label_create(lv_scr_act(), NULL);
	lv_obj_set_size(s_pTextAreaTrackNo, 50, 20);
	lv_obj_set_pos(s_pTextAreaTrackNo, 150, 50);
	lv_label_set_align(s_pTextAreaTrackNo, LV_LABEL_ALIGN_CENTER);

	s_pTextAreaTrackTime = lv_label_create(lv_scr_act(), NULL);
	lv_obj_set_size(s_pTextAreaTrackTime, 90, 20);
	lv_obj_set_pos(s_pTextAreaTrackTime, 200, 50);
	lv_label_set_align(s_pTextAreaTrackTime, LV_LABEL_ALIGN_CENTER);

	s_pTextAreaTrackName = lv_label_create(lv_scr_act(), NULL);
	lv_obj_set_size(s_pTextAreaTrackName, 150, 20);
	lv_obj_set_pos(s_pTextAreaTrackName, 300, 50);
	lv_label_set_align(s_pTextAreaTrackName, LV_LABEL_ALIGN_CENTER);

	lv_label_set_text(s_pTextAreaTrackNo, "XXX");
	lv_label_set_text(s_pTextAreaTrackTime, "00:00:00.000");
	lv_label_set_text(s_pTextAreaTrackName, "--------------------");
}

DefALLOCATE_ITCM void MainWindowSetTextAreaTrackNo(const char szStr[])
{
	lv_label_set_text(s_pTextAreaTrackNo, szStr);
}

DefALLOCATE_ITCM void MainWindowSetTextAreaTrackName(const char szStr[])
{
	lv_label_set_text(s_pTextAreaTrackName, szStr);
}

DefALLOCATE_ITCM void MainWindowSetTextAreaTrackTime(const char szStr[])
{
	lv_label_set_text(s_pTextAreaTrackTime, szStr);
}
