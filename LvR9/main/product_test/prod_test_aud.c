
#include "../lvgl_main.h"

#include <lvgl/lvgl.h>
#include "../utils/misc_utils.h"
#include "../utils/global_parm.h"
#include <time.h>


#ifndef C_PC_SIM
#include <sys/types.h>
//#include <dirent.h>   // for opendir() 
#include <limits.h>

#include "../include/audio_stream.h"  



#include "../../trd/tz_audio_player/include/tz_audio_player.h"

#endif


#include "../setting/app_setting_base.h"

#include "product_test.h"


#define TAG "prTsAud"



lv_obj_t * g_prdTs_bgWin;
//--------------------

static uint8_t g_audTsHadAudFile;
static uint8_t g_audTsHadAudDone;
static lv_timer_t * g_audTsTmr;
static lv_obj_t * g_lblPrompt;
static lv_obj_t * g_lblPromptB;

static int32_t g_startTmMs;

static int32_t g_updTmMsPrev;


static void _audTs_timer_cb(struct _lv_timer_t *tmr) {
	//ESP_LOGI(TAG, "tmr A");
#ifndef C_PC_SIM
	if (!g_audTsHadAudDone) {
		TZ_PLAYSTATE_T audStat = get_play_audio_state();
		ESP_LOGI(TAG, "audStat %hd", (short)audStat);
		if (TZ_PLAYSTOPPED == audStat || TZ_PLAYIDLE == audStat) {
			g_audTsHadAudDone = 1;
			lv_label_set_text(g_lblPrompt, "发音结束");
		}
	}
#endif
}

static void _audTs_initPlaying(bool force) {
	if (!force && !g_audTsHadAudDone)
		return;
#ifndef C_PC_SIM
	TZ_PLAYSTATE_T audStat = get_play_audio_state();
	ESP_LOGI(TAG, "intPly audStat %hd", (short)audStat);
	if (force || (TZ_PLAYSTOPPED == audStat) || (TZ_PLAYIDLE == audStat)) {
		//audio_stream_mem_play(NULL, short_a_aud_start, short_a_aud_end);
		if (g_audTsHadAudFile) {
			tz_play_audio(PATH_TEST_A_MP3, 0, -1);
		}
	}
	else {
		return;
	}
#endif
	g_audTsHadAudDone = 0;
	lv_label_set_text(g_lblPrompt, "发音中...");
}



static void _prdTs_audWinEvtCb(lv_event_t * evt) {
	lv_event_code_t evtCode = evt->code;

	int keyVal = 0;
	switch (evtCode) {
		case LV_EVENT_KEY_STAGE: {
			keyVal = lv_event_get_key(evt);
			if (((LV_EX_FLAG_KEY_MASK & keyVal) != LV_EX_FLAG_KEY_REPEAT) && ((LV_EX_FLAG_KEY_MASK & keyVal) != LV_EX_FLAG_KEY_SHORT)) {
				return;
			}
			keyVal &= ~LV_EX_FLAG_KEY_MASK;
			ESP_LOGI(TAG, "audTs key %x", keyVal);
			switch (keyVal) {
				case LV_KEY_ENTER: {
					_audTs_initPlaying(false);
					break;
				}
				case LV_KEY_ESC:
					// todo, prdTs_switchScr(PRDTS_SCR_NO_MENU, 0);
					break;
				default:
					break;
			}
			break;
		}
		case LV_EVENT_DELETE:
			ESP_LOGI(TAG, "delBgObj A");
		#ifndef C_PC_SIM
			tz_play_stop_audio();
		#endif
			if (g_audTsTmr) {
				lv_timer_del(g_audTsTmr);  g_audTsTmr = NULL;
			}
			break;
		default:
			break;
	}
}


int prdTs_aud_scr_init(int parm) { // 语音测试界面初始化 
	lv_obj_t * mainWin = lv_obj_create(lv_scr_act());
	lv_obj_set_size(mainWin, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainWin, 6, 0);
	//lv_obj_set_style_bg_color(mainWin, lv_color_white(), 0);

	lv_obj_t * lblPrompt = lv_label_create(mainWin);
	lv_label_set_text(lblPrompt, "发音中...");
	lv_obj_set_align(lblPrompt, LV_ALIGN_CENTER);
	g_lblPrompt = lblPrompt;

	lv_obj_add_event_cb(mainWin, _prdTs_audWinEvtCb, LV_EVENT_ALL, NULL);
	sys_setCurMainWinAndCb(mainWin, _prdTs_audWinEvtCb);
	g_prdTs_bgWin = mainWin;
#ifndef C_PC_SIM
	int volNo = PARM_VOLUME_DFLT;
	setPrjGlobalParm(ZB_PARM_VOLUME, 2, &volNo);
	tz_play_set_volume((PARM_VOLUME_DFLT * 100)/(PARM_VOLUME_MAX - PARM_VOLUME_MIN));
#endif
	_audTs_initPlaying(true);

	g_audTsTmr = lv_timer_create(_audTs_timer_cb, 300, NULL);
	return 0;
}


int prdTs_aud_scr_deinit(void) {  // 语音测试界面销毁 
	if (g_prdTs_bgWin) {
		lv_obj_del(g_prdTs_bgWin);  g_prdTs_bgWin = NULL;
	}
	return 0;
}

//------------------------------------------------------
static void _tstAud_esc_btn_click_cb(lv_event_t * evt)
{
	//lv_obj_t* obj = lv_event_get_target(evt);
	lv_timer_create(zb_ex_cmd_timer_cb, 0, (void *)CMD_PAR_RET_STNG_MENU);
	return;
}


static void _audTs_initLpPlaying() {
#ifndef C_PC_SIM
	ESP_LOGI(TAG, "initPly A\n");
	g_audTsHadAudFile = zb_isFileExist(PATH_TEST_A_MP3);
	if (g_audTsHadAudFile) {
		tz_play_audio(PATH_TEST_A_MP3, 0, -1);  // 
	}
	else {
		ESP_LOGW(TAG, "no audFile %d", PATH_TEST_A_MP3);
	}
	ESP_LOGI(TAG, "initPly Z\n");
#endif
}

static void _audLoop_timer_cb(struct _lv_timer_t *tmr) {
	ESP_LOGI(TAG, "tmr A");
#ifndef C_PC_SIM
	TZ_PLAYSTATE_T audStat = get_play_audio_state();
	if ((TZ_PLAYSTOPPED == audStat) || (TZ_PLAYIDLE == audStat)) {
		ESP_LOGI(TAG, "audStat %hd", (short)audStat);
		_audTs_initLpPlaying();
	}
#endif

	int32_t timeCurMs = zb_get_time_ms_since_boot();
	if (timeCurMs >= (g_updTmMsPrev  + 1000)) {
		int timeElapseSec = (timeCurMs - g_startTmMs) / 1000;
		g_updTmMsPrev = timeCurMs;
		int secN    = timeElapseSec % 60;
		int minuteN = timeElapseSec / 60;
		lv_label_set_text_fmt(g_lblPromptB, "已进行 %d分%02d秒", minuteN, secN);
	}
}


static void _prdTs_audLoop_winEvtCb(lv_event_t * evt) {
	lv_event_code_t evtCode = evt->code;

	int keyVal = 0;
	switch (evtCode) {
		case LV_EVENT_KEY_STAGE: {
			keyVal = lv_event_get_key(evt);
			ESP_LOGI(TAG, "audTs key %x ", keyVal);
			int actType = (keyVal & LV_EX_FLAG_KEY_MASK);
			keyVal &= ~LV_EX_FLAG_KEY_MASK;
			if (LV_EX_FLAG_KEY_SHORT == actType) {
				switch(keyVal) {
					case LV_KEY_EX_FUNC1: {
					#ifndef C_PC_SIM
						TZ_PLAYSTATE_T audStat = get_play_audio_state();
						ESP_LOGI(TAG, "plStat: %d ", audStat);
						if (TZ_PLAYPAUSED == audStat) {
							tz_play_resume_audio();
						}
						else {
							tz_play_pause_audio();
						}
					#endif
						break;
					}
					default:
						break;
				}
			}
			break;
		}
		case LV_EVENT_DELETE: {
			ESP_LOGI(TAG, "lpWin delBgObj A");
		#ifndef C_PC_SIM
			tz_play_stop_audio();
		#endif
			g_lblPromptB = NULL;
			if (g_audTsTmr) {
				lv_timer_del(g_audTsTmr);  g_audTsTmr = NULL;
			}
			break;
		}
		default:
			break;
	}
}


int prdTs_audLoop_scr_init(int parm) { // 语音循环测试界面初始化 
	ESP_LOGI(TAG, "audLpScr A\n");
	lv_obj_t * mainWin = lv_obj_create(lv_scr_act());
	lv_obj_set_size(mainWin, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainWin, 0, 0);
	//lv_obj_set_style_bg_color(mainWin, lv_color_white(), 0);

	lv_obj_t * lblPrompt = lv_label_create(mainWin);
	lv_label_set_text(lblPrompt, "音乐播放中...(循环模式)");
	lv_obj_set_align(lblPrompt, LV_ALIGN_CENTER);
	g_lblPrompt = lblPrompt;

	lv_obj_t * lblPromptB = lv_label_create(mainWin);  // 当前时间 
	lv_label_set_text(lblPromptB, "");
	lv_obj_align(lblPromptB, LV_ALIGN_TOP_LEFT, 90, 20);
	lv_obj_set_style_text_color(lblPromptB, lv_color_black(), 0);
	g_lblPromptB = lblPromptB;

#if 0
	lv_obj_t * lblPromptC = lv_label_create(mainWin);  // 当前时间 
	lv_obj_remove_style_all(lblPromptC);
	lv_obj_set_style_border_width(lblPromptC, 0, 0);
	lv_obj_set_style_pad_all(lblPromptC, 0, 0);
	lv_label_set_text(lblPromptC, "量屏显器hely遥攒藏增懿\n繁翻循增帚瞳");
	//lv_label_set_text(lblPromptC, "09:54 SHENZHEN BA");
	//lv_obj_set_style_text_font(lblPromptC, &lv_font_montserrat_16, 0);
	lv_obj_set_size(lblPromptC, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_align(lblPromptC, LV_ALIGN_TOP_RIGHT, 0, 0);
	lv_obj_set_style_text_line_space(lblPromptC, 0, 0);
	lv_obj_set_style_text_color(lblPromptC, lv_color_black(), 0); 
#endif
	
	/* lv_obj_t * imgA = lv_img_create(mainWin);
	lv_obj_remove_style_all(imgA);
	lv_obj_align(imgA , LV_ALIGN_TOP_RIGHT, 0, 0);
	const lv_img_dsc_t * pImgDsc = g_XXX_img_dsc_tbl + MMN_youdao - 1; 
	lv_img_set_src(imgA, pImgDsc);
	lv_obj_set_size(imgA, pImgDsc->header.w, pImgDsc->header.h); */

	lv_obj_add_event_cb(mainWin, _prdTs_audLoop_winEvtCb, LV_EVENT_ALL, NULL);
	sys_setCurMainWinAndCb(mainWin, _prdTs_audLoop_winEvtCb);
	g_prdTs_bgWin = mainWin;
	stng_createEscBtn(mainWin, _tstAud_esc_btn_click_cb, 0);
	ESP_LOGI(TAG, "audLpScr H\n");

	g_updTmMsPrev = 0;
	int32_t tmMs = zb_get_time_ms_since_boot();
	g_startTmMs = g_updTmMsPrev = tmMs;
	_audTs_initLpPlaying(true);

	g_audTsTmr = lv_timer_create(_audLoop_timer_cb, 800, NULL);
	return 0;
}


int prdTs_audLoop_scr_deinit(void) {  // 语音循环测试界面销毁 
	if (g_prdTs_bgWin) {
		lv_obj_del(g_prdTs_bgWin);  g_prdTs_bgWin = NULL;
	}
	return 0;
}


//------------------------------------------------------


