
#include <lvgl/lvgl.h>
#include <time.h>
#include "../utils/misc_utils.h"
#include "../mainmenu/tlb_main_img.h"

#define TAG  "almPrmDlg"


extern void req_jump_to_alarm_player(void);

extern int getAlarmDatumPath(int step, char * pcPath, int bufLen);
extern void setParmForAlarmPlayer(void);
extern int zb_sys_update_next_alarm(void);
extern void turnOffLastAlarmForOnceMode(void);



extern const lv_img_dsc_t  g_XXX_img_dsc_tbl[];

//------------------------------------------------------

static lv_group_t * sys_group;
static lv_obj_t  * g_almPromptDlg = NULL;

static lv_timer_t * g_timerA = NULL;

static int32_t   g_promptTmSt;  // 开始提示的时刻(unit: ms, 在 PC_SIM 里为单位为秒)
static bool   g_doAlmPlayer;

//------------------------------------------------------

/* 响闹发生时刻的内部处理.
 * @param parmIn bit0 为 0 表示取消播放, 为 1 表示播放响闹音 
 */
void zb_alarmTimeProc(int16_t parmIn)
{
	ESP_LOGI(TAG, "almTmProc %hd", parmIn);
	turnOffLastAlarmForOnceMode();
	ESP_LOGI(TAG, "almTmProc B");

	if (parmIn & 1) {
		setParmForAlarmPlayer();
		req_jump_to_alarm_player();
	}
	else {
		ESP_LOGI(TAG, "cancelPly, bfr updNxAlm");
		zb_sys_update_next_alarm();
	}
}

static void _almPrompt_timer_cb(struct _lv_timer_t *tmr) {
	//ESP_LOGI(TAG, "tmr A");
	lv_obj_t * labelA = lv_obj_get_child(g_almPromptDlg, 1);  // non-std
	if (!labelA)
		return;

	int64_t tmCur = 0;
#ifndef C_PC_SIM
	tmCur = esp_timer_get_time();
	if (tmCur >= (g_promptTmSt + 8000) * 1000L) 
#else
	tmCur = time(NULL);
	if (tmCur >= (g_promptTmSt + 8)) 
#endif
	{
		//---- sugy 20240827 modify,  layer_stack 里的各层,必须是从外层起 逐层地退. 
		if (!(sys_group && zb_is_groupOnTopLayer(sys_group))) {
			ESP_LOGI(TAG, "tmr reach, not atTop, skip");
			return;
		}

		if (g_timerA) {
			lv_timer_del(g_timerA);  g_timerA = NULL;
		}
		g_doAlmPlayer = true;
		lv_obj_del(g_almPromptDlg);
	}
}


static void _almPrompt_event_cb(lv_event_t * e)
{
#if 0
	lv_event_code_t code = lv_event_get_code(e);
	switch (code) {
		case LV_EVENT_KEY_STAGE: {
			int key = *((int*)lv_event_get_param(e));
			if (code == LV_EVENT_KEY_STAGE) {
				if ((LV_EX_FLAG_KEY_MASK & key) != LV_EX_FLAG_KEY_SHORT) return;
				key &= ~LV_EX_FLAG_KEY_MASK;
			}
			ESP_LOGI(TAG, "key %d", key);
			switch (key) {
				case LV_KEY_ENTER: {
					lv_obj_t * objThis = g_almPromptDlg;
					lv_indev_t * indev = lv_indev_get_by_type(LV_INDEV_TYPE_KEYPAD);
					lv_indev_reset(indev, objThis);
					lv_event_stop_processing(e);

					g_doAlmPlayer = true;
					lv_obj_del(objThis);
					break;
				}
				case LV_KEY_ESC: {
					lv_obj_del(g_almPromptDlg);
					lv_event_stop_processing(e);
					break;
				}
				default:
					break;
			}
			break;
		}
		/* case LV_EVENT_DEFOCUSED:
			LV_LOG("%s : %d DEFOCUSED ....\n",__FUNCTION__,__LINE__);
			break; */
		case LV_EVENT_DELETE: {
			if (g_timerA) {
				lv_timer_del(g_timerA);  g_timerA = NULL;
			}
			if (sys_group != NULL) {
				lv_group_del(sys_group);  sys_group = NULL;
			}
			ESP_LOGI(TAG, "dlgDel Y\n");
			zb_group_pop(false);

			g_almPromptDlg = NULL;
			ESP_LOGI(TAG, "dlgDel Y c\n");

			zb_alarmTimeProc(g_doAlmPlayer ? 1 : 0);
			ESP_LOGI(TAG, "dlgDel Zz\n");
			break;
		}
		default:
			break;
	}
#endif
}


/** 显示背景播放操作框(暂停、继续、显示音乐名称) 
 * @return >= 0 means create done.
 */
int zb_show_alarmPromptDlg(void)
{
	int iFuncRet = -1;
	static const lv_coord_t col_dsc[] = // 
		{ LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
	static const lv_coord_t row_dsc[] = // 
		{ 10, 80, LV_GRID_FR(1), 10, LV_GRID_TEMPLATE_LAST };

	ESP_LOGI(TAG, "showBgPlyDlg A");
	do {
		if (g_almPromptDlg) { // if alarmDlg is already shown, quit 
			iFuncRet = -1;  break;
		}
		if (zb_screen_layer_is_full()) {
			iFuncRet = -5;  break;
		}

		//ESP_LOGI(TAG, "showBgPlyDlg B");
		
		lv_obj_t * bgObj = lv_obj_create(lv_layer_top());
		lv_obj_set_size(bgObj, SCREEN_WIDTH - 10 * 2, 220);
		lv_obj_set_style_pad_all(bgObj, 4, 0);
		lv_obj_align(bgObj, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_grid_dsc_array(bgObj, col_dsc, row_dsc);
		lv_obj_set_style_pad_row(bgObj, 0, 0);
		lv_obj_set_style_pad_column(bgObj, 0, 0);
		lv_obj_set_style_bg_color(bgObj, lv_color_white(), 0);
		lv_obj_set_style_border_width(bgObj, 1, 0);
		lv_obj_set_style_border_color(bgObj, lv_color_hex(0x808080), 0);

		lv_obj_t * imgA = lv_img_create(bgObj);
		const lv_img_dsc_t * imgResume = g_XXX_img_dsc_tbl + MMN_alarm_prompt_icn - 1;
		lv_obj_set_size(imgA, imgResume->header.w, imgResume->header.h);
		lv_obj_set_grid_cell(imgA, LV_GRID_ALIGN_CENTER, 0, 1,  LV_GRID_ALIGN_CENTER, 1, 1);
		lv_obj_set_style_bg_img_src(imgA, imgResume, 0);

		char acMscTitle[200];
		acMscTitle[0] = '\0';
		getAlarmDatumPath(0, acMscTitle, sizeof(acMscTitle));
		if (acMscTitle[0]) {
			ESP_LOGW(TAG, "alm audPath is space!");
		#ifdef C_PC_SIM
			strncat(acMscTitle, "英语七年级下", sizeof(acMscTitle) - 1);
		#endif
		}
		lv_obj_t * labelA = lv_label_create(bgObj);
		uint8_t apNo = zb_getApNo();
		const char * pcInfA = (C_ZB_AP_MAINMENU == apNo ? "" : "将离开当前应用\n");
		lv_label_set_text_fmt(labelA, "磨耳朵计划时间到了\n%s%s", pcInfA, acMscTitle);
		ESP_LOGI(TAG, "will alm with fl:%s", acMscTitle);
		lv_obj_set_grid_cell(labelA, LV_GRID_ALIGN_STRETCH, 0, 1,  LV_GRID_ALIGN_CENTER, 2, 1);
		lv_obj_align(labelA, LV_ALIGN_CENTER, 0, 0);
		//lv_label_set_long_mode(labelA, LV_LABEL_LONG_SCROLL_CIRCULAR);
		lv_obj_set_style_text_align(labelA, LV_TEXT_ALIGN_CENTER, 0);
		// lv_obj_set_style_max_height(labelA, 26, 0);


		lv_obj_add_event_cb(bgObj, _almPrompt_event_cb, LV_EVENT_ALL, NULL);

		g_timerA = lv_timer_create(_almPrompt_timer_cb, 250, NULL);
		//--------------------------------------
		sys_group = lv_group_create();
		ESP_LOGI(TAG, "bfr grpPush");
		zb_group_push(sys_group, 0);
		lv_group_add_obj(sys_group, bgObj);
		lv_group_focus_obj(bgObj);
		g_almPromptDlg = bgObj;
		g_doAlmPlayer = false;
	#ifndef C_PC_SIM
		g_promptTmSt = esp_timer_get_time() / 1000;
	#else
		g_promptTmSt = time(NULL);
	#endif
		iFuncRet = 0;
	}while (0);

	if (iFuncRet < 0) {
		ESP_LOGE(TAG, "showPrptDlg fail %d", iFuncRet);
	}
	ESP_LOGI(TAG, "showPrptDlg Zz");
	return iFuncRet;
}



