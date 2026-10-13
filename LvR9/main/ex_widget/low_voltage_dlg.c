
#include <lvgl/lvgl.h>
#include "../utils/misc_utils.h"
#include "../mainmenu/tlb_main_img.h"

extern void zb_request_power_off(int cause); // non-std 



#define TAG  "LvtDlg"


#define STR_BATT_LOW_WARN "电量过低,请尽快充电!"


extern const lv_img_dsc_t  g_XXX_img_dsc_tbl[];

//------------------------------------------------------
static lv_obj_t  * g_promptRootObj = NULL;

static lv_timer_t * g_timerA = NULL;
static int8_t  g_lvotDlgType;  // 2 means lowPower shutdownMode
static int32_t g_lowPwrDlgStartTm;

//------------------------------------------------------
static void lvtPromptTimer(struct _lv_timer_t * tmr)
{
	lv_timer_del(tmr);
	g_timerA = NULL;
	if (g_promptRootObj) {
		lv_obj_del(g_promptRootObj);  g_promptRootObj = NULL;
	}
}

static void _lowVolt_event_cb(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code) {
		case LV_EVENT_DELETE: {
			if (g_timerA) {
				lv_timer_del(g_timerA);  g_timerA = NULL;
			}
			g_promptRootObj = NULL;
			if (2 == g_lvotDlgType) {
				zb_request_power_off(2);  // pwr_off_cause_low_battery = 2
			}
			ESP_LOGI(TAG, "lowVotlg dlgDel Zz\n");
			break;
		}
		default:
			break;
	}
}


/** 显示低电压提示框 
 * @param dlgType - 2 means lowPower shutdown mode
 * @return >= 0 means create done.
 */
int sys_show_lowVoltageDlg(int16_t confirmCode, int16_t dlgType)
{
	int iFuncRet = -1;
	static const lv_coord_t col_dsc[] = // 
		{ LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
	static const lv_coord_t row_dsc[] = // 
		{ 64, LV_GRID_FR(1),  LV_GRID_TEMPLATE_LAST };

	ESP_LOGI(TAG, "showPromptDlg A");
	do {
		if (confirmCode != LOW_POWER_DLG_CONFIRM_CODE) {
			iFuncRet = -3;  break;
		}
		if (g_promptRootObj) { // if promptDlg is already shown, quit 
			iFuncRet = -5;  break;
		}
		/* if (zb_screen_layer_is_full()) {
			iFuncRet = -7;  break;
		} */

	#if 0
		sys_group = lv_group_create();
		ESP_LOGI(TAG, "bfr grpPush");
		zb_group_push(sys_group, 0);
	#endif
		lv_obj_t * bgObj = lv_obj_create(lv_layer_top());
		lv_obj_set_size(bgObj, SCREEN_WIDTH - 8 * 2, 130);
		lv_obj_set_style_pad_all(bgObj, 4, 0);
		lv_obj_align(bgObj, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_grid_dsc_array(bgObj, col_dsc, row_dsc);
		lv_obj_set_style_pad_row(bgObj, 0, 0);
		lv_obj_set_style_pad_column(bgObj, 0, 0);
		lv_obj_set_style_bg_color(bgObj, lv_color_hex(0xe8e8e8), 0);  // 0xff8c00

		lv_obj_t * imgA = lv_img_create(bgObj);
		lv_obj_set_grid_cell(imgA, LV_GRID_ALIGN_CENTER, 0, 1,  LV_GRID_ALIGN_CENTER, 0, 1);
		const lv_img_dsc_t * imgBatteryLow = g_XXX_img_dsc_tbl + MMN_battery_low - 1;
		lv_img_set_src(imgA, imgBatteryLow);
		lv_obj_set_size(imgA, imgBatteryLow->header.w, imgBatteryLow->header.h);

		lv_obj_t * labelA = lv_label_create(bgObj);
		lv_label_set_text(labelA, STR_BATT_LOW_WARN);
		lv_obj_set_grid_cell(labelA, LV_GRID_ALIGN_STRETCH, 0, 1,  LV_GRID_ALIGN_CENTER, 1, 1);
		lv_obj_align(labelA, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_style_text_align(labelA, LV_TEXT_ALIGN_CENTER, 0);

		g_lvotDlgType = dlgType;
		g_lowPwrDlgStartTm = zb_get_time_ms_since_boot();
		uint32_t period = (2 == dlgType ? 2200 : 3300);
		g_timerA = lv_timer_create(lvtPromptTimer, period, NULL);

	#if 0
		lv_group_add_obj(sys_group, bgObj);
	#endif
		lv_obj_add_event_cb(bgObj, _lowVolt_event_cb, LV_EVENT_ALL, NULL);
		g_promptRootObj = bgObj;
		iFuncRet = 0;
	}while (0);

	if (iFuncRet < 0) {
		ESP_LOGE(TAG, "showDlg fail %d", iFuncRet);
	}
	ESP_LOGI(TAG, "showDlg Zz");
	return iFuncRet;
}


int sys_lowVoltDlg_getDuration(void)
{
	if (NULL == g_promptRootObj)
		return 0;

	if (0 == g_lowPwrDlgStartTm)
		return 0;

	int32_t tmNow = zb_get_time_ms_since_boot();
	return tmNow - g_lowPwrDlgStartTm;
}



