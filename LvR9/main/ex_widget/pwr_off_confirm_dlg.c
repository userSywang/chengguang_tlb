
#include <lvgl/lvgl.h>
#include <time.h>
#include "../utils/misc_utils.h"
#include "../utils/global_parm.h"

#include "../lvgl_main.h"

#include "../mainmenu/tlb_main_img.h"

#define TAG  "pwrOffCfm"



extern const lv_img_dsc_t  g_XXX_img_dsc_tbl[];

enum  e_pwr_off_cfm_act {
	pwr_off_cfm_act_cancel,
	pwr_off_cfm_act_shutdown,
	pwr_off_cfm_act_restart,
};

#define PWR_OFF_CFM_WHOLE_SCR 1  // 0 - partScreen, 1 - wholeScreen


#define DLG_HAS_RESET_BTN  0
//------------------------------------------------------

static lv_group_t * sys_group;
static lv_obj_t * g_pwfOffCfmDlg = NULL;
static lv_obj_t * g_labelShutdown;
#if DLG_HAS_RESET_BTN
static lv_obj_t * g_labelReset;
#endif

static int8_t   g_itemSel;  // 0 or 1
static enum  e_pwr_off_cfm_act   g_pwrOffActNo;  // 0-cancel, 1-pwrOff, 2-reset 

static lv_timer_t * g_off_timer = NULL;

//------------------------------------------------------


static void _pwrOffCfmselBtn(int8_t sel) {
	if (0 == sel) {
		g_itemSel = sel;
		lv_obj_add_state(g_labelShutdown, LV_STATE_CHECKED);
	#if DLG_HAS_RESET_BTN
		lv_obj_clear_state(g_labelReset, LV_STATE_CHECKED);
	#endif
	}
	else if (1 == sel) {
		g_itemSel = sel;
		lv_obj_clear_state(g_labelShutdown, LV_STATE_CHECKED);
	#if DLG_HAS_RESET_BTN
		lv_obj_add_state(g_labelReset, LV_STATE_CHECKED);
	#endif
	}
}

static void _pwrOffCfm_event_cb(lv_event_t * e)
{
#if 1
	lv_event_code_t code = lv_event_get_code(e);
	switch (code) {
		case LV_EVENT_KEY:  {
			int key = *((int*)lv_event_get_param(e));
			ESP_LOGI(TAG, "keyPrs %d", key);
			break;
		}
		case LV_EVENT_KEY_STAGE: {
			int key = *((int*)lv_event_get_param(e));
			ESP_LOGI(TAG, "keyStg %d", key);
			if (code == LV_EVENT_KEY_STAGE) {
				if ((LV_EX_FLAG_KEY_MASK & key) != LV_EX_FLAG_KEY_SHORT) return;
				key &= ~LV_EX_FLAG_KEY_MASK;
			}
			ESP_LOGI(TAG, "key %d", key);
			switch (key) {
				// case LV_KEY_LEFT: {
				// 	if (g_itemSel < 0) break;
				// 	_pwrOffCfmselBtn(0);
				// 	break;
				// }
				// case LV_KEY_RIGHT: {
				// 	if (g_itemSel >= 1) break;
				// 	_pwrOffCfmselBtn(1);
				// 	break;
				// }
				// case LV_KEY_ENTER: {
				// 	lv_obj_t * objThis = g_pwfOffCfmDlg;
				// 	lv_indev_t * indev = lv_indev_get_by_type(LV_INDEV_TYPE_KEYPAD);
				// 	lv_indev_reset(indev, objThis);
				// 	lv_event_stop_processing(e);

				// 	g_pwrOffActNo = (0 == g_itemSel ? pwr_off_cfm_act_shutdown : pwr_off_cfm_act_restart);
				// 	lv_obj_del(objThis);
				// 	break;
				// }
				// case LV_KEY_ESC: {
				// 	g_pwrOffActNo = pwr_off_cfm_act_cancel;
				// 	lv_obj_del(g_pwfOffCfmDlg);
				// 	lv_event_stop_processing(e);
				// 	break;
				// }
				default:
					break;
			}
			break;
		}
		case LV_EVENT_CLICKED:
		// case LV_EVENT_PRESSED:
		// 	ESP_LOGI(TAG, "click, willCloseDlg");
		// 	if (g_pwfOffCfmDlg) {
		// 		lv_indev_wait_release(lv_indev_get_by_type(LV_INDEV_TYPE_POINTER));
		// 		lv_event_stop_processing(e);
		// 		lv_obj_del(g_pwfOffCfmDlg);
		// 	}
		// 	break;
		case LV_EVENT_DELETE: {
			/* if (g_timerA) {
				lv_timer_del(g_timerA);  g_timerA = NULL;
			} */
			 if(g_off_timer) {
				lv_timer_del(g_off_timer);  g_off_timer = NULL;
			}
			if (sys_group != NULL) {
				lv_group_del(sys_group);  sys_group = NULL;
			}
			ESP_LOGI(TAG, "dlgDel Y\n");
			zb_group_pop(false);

			ESP_LOGI(TAG, "dlgDel Y c\n");
			if (pwr_off_cfm_act_shutdown == g_pwrOffActNo) {
				ESP_LOGI(TAG, "shutdown now---");
				zb_request_power_off(pwr_off_cause_key_power);
			}
			else if (pwr_off_cfm_act_restart == g_pwrOffActNo) {
				ESP_LOGI(TAG, "restart now---");
				zb_request_power_off(pwr_off_cause_restart);
			}
			g_pwfOffCfmDlg = NULL;
			g_labelShutdown = NULL;
		#if DLG_HAS_RESET_BTN
			g_labelReset = NULL;
		#endif
			ESP_LOGI(TAG, "dlgDel Zz\n");
			break;
		}
		default:
			break;
	}
#endif
}


static void _pwrCf_initBtnTitleLabel(lv_obj_t * label) {
	lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
//#if PWR_OFF_CFM_WHOLE_SCR
#if 1
	lv_obj_set_style_text_color(label, lv_color_hex(0x808080), 0);
	lv_obj_set_style_text_color(label, lv_color_white(), LV_STATE_CHECKED);
#endif
	lv_obj_set_style_bg_opa(label, LV_OPA_80, LV_STATE_CHECKED);
	lv_obj_set_style_bg_color(label, lv_color_hex(0x404040), LV_STATE_CHECKED);
	lv_obj_set_style_pad_all(label, 6, 0);
}


static void _pwrCf_shutdown_click_cb(lv_event_t * evt)
{
	ESP_LOGI(TAG, "willPwrOff A");
	//savePrjGlobalParm();

	zb_request_power_off(pwr_off_cause_key_power);
#if 0
	usleep(1000 * 30);
	system("poweroff"); //关机 
#endif
}


/** 显示背景播放操作框(暂停、继续、显示音乐名称) 
 * @return >= 0 means create done.
 */
int zb_show_pwrOffConfirmDlg(void)
{
	int iFuncRet = -1;

// #if 0 == PWR_OFF_CFM_WHOLE_SCR	
// 	static const lv_coord_t col_dsc[] = // 
// 		{ LV_GRID_FR(1), 80, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
// 		//{ 40, 60, LV_GRID_FR(1), 60, 40, LV_GRID_TEMPLATE_LAST };
// 	static const lv_coord_t row_dsc[] = // 
// 		{ 12, LV_GRID_FR(1), 36, 4, LV_GRID_TEMPLATE_LAST };
// #else 
// 	static const lv_coord_t col_dsc[] = // 
// 		{ LV_GRID_FR(1), 80, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
// 		//{ 40, 60, LV_GRID_FR(1), 60, 40, LV_GRID_TEMPLATE_LAST };
// 	static const lv_coord_t row_dsc[] = // 
// 		{ LV_GRID_FR(1), 110, 26, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
// #endif	

// 	ESP_LOGI(TAG, "showBgPlyDlg A");
// 	do {
// 		if (g_pwfOffCfmDlg) { // if alarmDlg is already shown, quit 
// 			iFuncRet = -1;  break;
// 		}
// 		// todo, if is_com_vol_shown, turn off it.

// 		sys_group = lv_group_create();
// 		ESP_LOGI(TAG, "bfr grpPush");
// 		zb_group_push(sys_group, 0);

// 		lv_obj_t * bgObj = lv_obj_create(lv_layer_top());
// 	#if 0 == PWR_OFF_CFM_WHOLE_SCR
// 		lv_obj_set_size(bgObj, SCREEN_WIDTH - 4*2, 120);
// 		lv_obj_align(bgObj, LV_ALIGN_CENTER, 0, 0);
// 	#else
// 		lv_obj_set_size(bgObj, SCREEN_WIDTH, SCREEN_HEIGHT);
// 	#endif
// 		lv_obj_add_flag(bgObj, LV_OBJ_FLAG_CLICKABLE);

// 		lv_obj_set_grid_dsc_array(bgObj, col_dsc, row_dsc);
// 		lv_obj_set_style_pad_row(bgObj, 0, 0);
// 		lv_obj_set_style_pad_column(bgObj, 0, 0);
// 	#if 0 == PWR_OFF_CFM_WHOLE_SCR
// 		lv_obj_set_style_bg_color(bgObj, lv_color_hex(0x101010), 0);
// 		lv_obj_set_style_bg_opa(bgObj, LV_OPA_50, 0);
// 	#else
// 		lv_obj_set_style_bg_color(bgObj, lv_color_hex(0x606060), 0);
// 		lv_obj_set_style_border_width(bgObj, 0, 0);
// 		lv_obj_set_style_radius(bgObj, 0, 0);
// 		lv_obj_set_style_bg_opa(bgObj, LV_OPA_50, 0);
// 	#endif
// 		lv_obj_t * imgShutdown = lv_img_create(bgObj);
// 		const lv_img_dsc_t * imgDscShutdown = g_XXX_img_dsc_tbl + MMN_icn_shutdown - 1;
// 		lv_obj_set_size(imgShutdown, imgDscShutdown->header.w, imgDscShutdown->header.h);
// 		lv_obj_set_grid_cell(imgShutdown, LV_GRID_ALIGN_CENTER, 1, 1,  LV_GRID_ALIGN_CENTER, 1, 1);
// 		lv_obj_set_style_bg_img_src(imgShutdown, imgDscShutdown, 0);
// 		lv_obj_add_flag(imgShutdown, LV_OBJ_FLAG_CLICKABLE);
// 		lv_obj_add_event_cb(imgShutdown, _pwrCf_shutdown_click_cb, LV_EVENT_CLICKED, NULL);  // 

// 	#if DLG_HAS_RESET_BTN
// 		lv_obj_t * imgReset = lv_img_create(bgObj);
// 		const lv_img_dsc_t * imgDscReset = g_XXX_img_dsc_tbl + MMN_icn_restart - 1;
// 		lv_obj_set_size(imgReset, imgDscReset->header.w, imgDscReset->header.h);
// 		lv_obj_set_grid_cell(imgReset, LV_GRID_ALIGN_CENTER, 3, 1,  LV_GRID_ALIGN_CENTER, 1, 1);
// 		lv_obj_set_style_bg_img_src(imgReset, imgDscReset, 0);
// 	#endif

// 		lv_obj_t * labelShutdown = lv_label_create(bgObj);
// 		lv_obj_set_grid_cell(labelShutdown, LV_GRID_ALIGN_STRETCH, 1, 1,  LV_GRID_ALIGN_CENTER, 2, 1);
// 		lv_label_set_text_static(labelShutdown, "关机");
// 		_pwrCf_initBtnTitleLabel(labelShutdown);

// 	#if DLG_HAS_RESET_BTN
// 		lv_obj_t * labelReset = lv_label_create(bgObj);
// 		lv_obj_set_grid_cell(labelReset, LV_GRID_ALIGN_STRETCH, 3, 1,  LV_GRID_ALIGN_CENTER, 2, 1);
// 		lv_label_set_text_static(labelReset, "重启");
// 		_pwrCf_initBtnTitleLabel(labelReset);
// 	#endif

// 		g_labelShutdown = labelShutdown;
// 	#if DLG_HAS_RESET_BTN
// 		g_labelReset = labelReset;
// 	#endif
// 		lv_obj_add_flag(bgObj, LV_OBJ_FLAG_CLICKABLE);
// 		lv_obj_add_event_cb(bgObj, _pwrOffCfm_event_cb, LV_EVENT_ALL, NULL);
// 		ESP_LOGI(TAG, "bgObj evtPtr %p, n:%d, cb %p", bgObj->spec_attr->event_dsc , (int)bgObj->spec_attr->event_dsc_cnt, _pwrOffCfm_event_cb);

		//g_timerA = lv_timer_create(_pwrOffCfm_timer_cb, 250, NULL);
		//--------------------------------------
		//lv_group_add_obj(sys_group, bgObj);
		//lv_group_focus_obj(bgObj);/
	lv_obj_t* bgobj = lv_obj_create(lv_layer_top());
	lv_obj_set_size(bgobj, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_bg_color(bgobj, lv_color_black(), 0);
	lv_obj_set_style_radius(bgobj, 0, 0);
	lv_obj_set_style_border_width(bgobj, 0, 0);
	lv_obj_t *powerOff_lb = lv_label_create(bgobj);
	lv_label_set_text(powerOff_lb, "正在关机");
	lv_obj_set_style_text_color(powerOff_lb, lv_color_white(), 0);
	lv_obj_align(powerOff_lb, LV_ALIGN_CENTER, 0, 0);
	g_off_timer = lv_timer_create(_pwrCf_shutdown_click_cb, 3000, NULL);

	sys_setCurMainWinAndCb(bgobj, _pwrOffCfm_event_cb);

	// lv_obj_add_state(g_labelShutdown, LV_STATE_CHECKED);
	g_itemSel = 0;
	g_pwrOffActNo = pwr_off_cfm_act_cancel;
	g_pwfOffCfmDlg = bgobj;
	iFuncRet = 0;
	//}while (0);

	if (iFuncRet < 0) {
		ESP_LOGE(TAG, "showPrptDlg fail %d", iFuncRet);
	}
	ESP_LOGI(TAG, "showPrptDlg Zz");
	return iFuncRet;
}

void zb_del_pwrOffConfirmDlg(void) {
	if (g_pwfOffCfmDlg) {
		lv_obj_del(g_pwfOffCfmDlg);  g_pwfOffCfmDlg = NULL;
	}
}


int is_pwrOffConfirmDlg_shown(void) {
	return (g_pwfOffCfmDlg != NULL);
}



