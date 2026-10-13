
#include <lvgl/lvgl.h>
#include "../utils/misc_utils.h"
#include "../mainmenu/tlb_main_img.h"

#define TAG  "bgPlyDlg"


extern int play_or_pause_backstage_unit(void);
extern char *get_backstage_play_unit_name(void);



extern const lv_img_dsc_t  g_XXX_img_dsc_tbl[];

//------------------------------------------------------

static lv_group_t * sys_group;
static lv_obj_t  * g_plyDlg = NULL;
static lv_obj_t  * g_imgA = NULL;

static const char * g_pcMscTitle = NULL;
static lv_timer_t * g_timerA = NULL;


//------------------------------------------------------
static const char * g_pcSpaceTitle = "--";

void zb_del_bgPlayerDlg(void) {
	ESP_LOGI(TAG, "del bgPlyDlg");
	if (g_plyDlg) {
		lv_obj_del(g_plyDlg);  g_plyDlg = NULL;
	}
}


int is_bgPlayerDlg_shown(void) {
	return (g_plyDlg != NULL);
}


static void _updPauseResumeBtn(void) {
	int bgPlayerStat = get_player_backstage_state();
	if (bgPlayerStat < 0)
		return;

	lv_obj_t * imgA = g_imgA;
	if (!imgA)
		return;

	ESP_LOGI(TAG, "updPauseResm stat %d", bgPlayerStat);
	if (bgPlayerStat & 2) {
		lv_obj_add_state(imgA, LV_STATE_CHECKED);
	}
	else {
		lv_obj_clear_state(imgA, LV_STATE_CHECKED);
	}
}


static void _bgPlayer_timer_cb(struct _lv_timer_t *tmr) {
	//ESP_LOGI(TAG, "tmr A");
	lv_obj_t * labelA = lv_obj_get_child(g_plyDlg, 0);  // non-std
	if (!labelA)
		return;

	int bgPlayerStat = get_player_backstage_state();
	if (bgPlayerStat < 0) {
		zb_del_bgPlayerDlg();
		return;
	}
	char * pcMscTitleN = get_backstage_play_unit_name();
	if (!pcMscTitleN || !pcMscTitleN[0]) {
		pcMscTitleN = g_pcSpaceTitle;
	}
	if (strcmp(pcMscTitleN, g_pcMscTitle) != 0) {
		g_pcMscTitle = pcMscTitleN;
		lv_label_set_text(labelA, pcMscTitleN);
	}
	_updPauseResumeBtn();
}


static void bgPlayer_event_cb(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code) {
		case C_EVENT_KEY_STAGE: {
			int key = *((int*)lv_event_get_param(e));
			if (code == LV_EVENT_KEY_STAGE) {
				if ((LV_EX_FLAG_KEY_MASK & key) != LV_EX_FLAG_KEY_SHORT) return;
				key &= ~LV_EX_FLAG_KEY_MASK;
			}
			ESP_LOGI(TAG, "key %d", key);
			switch (key) {
				case LV_KEY_ENTER: {
					ESP_LOGI(TAG, "resume or pause");
					bool isChecked = lv_obj_has_state(g_imgA, LV_STATE_CHECKED);
					int bgPlyStat = get_player_backstage_state();
					bool needOp = false;
					if (!isChecked) { // is playing
						if ((bgPlyStat & 2) == 0) {
							lv_obj_add_state(g_imgA, LV_STATE_CHECKED);  needOp = true;
						}
					}
					else {  // is pausing 
						if (bgPlyStat & 2) {
							lv_obj_clear_state(g_imgA, LV_STATE_CHECKED); needOp = true;
						}
					}
					if (needOp) {
						ESP_LOGI(TAG, "switch pauseResm");
						play_or_pause_backstage_unit();
					}
					break;
				}
				case LV_KEY_ESC: {
					lv_obj_del(g_plyDlg);
					break;
				}
				default:
					break;
			}
			break;
		}
		case LV_EVENT_DEFOCUSED:
			LV_LOG("%s : %d DEFOCUSED ....\n",__FUNCTION__,__LINE__);
			break;
		case LV_EVENT_DELETE: {
			if (g_timerA) {
				lv_timer_del(g_timerA);  g_timerA = NULL;
			}
			if (sys_group != NULL) {
				lv_group_del(sys_group);  sys_group = NULL;
			}
			ESP_LOGI(TAG, "dlgDel Z\n");
			zb_group_pop(false);

			g_plyDlg = NULL;
			g_imgA = NULL;
			ESP_LOGI(TAG, "dlgDel Zz\n");
			break;
		}
		default:
			break;
	}
}


/** 显示背景播放操作框(暂停、继续、显示音乐名称) 
 * @return >= 0 means create done.
 */
int zb_show_bgPlayerDlg(void)
{
	int iFuncRet = -1;
	static const lv_coord_t col_dsc[] = // 
		{ LV_GRID_FR(1), 52, LV_GRID_TEMPLATE_LAST };
	static const lv_coord_t row_dsc[] = // 
		{ LV_GRID_FR(1),  LV_GRID_TEMPLATE_LAST };

	ESP_LOGI(TAG, "showBgPlyDlg A");
	do {
		if (g_plyDlg) { // if alarmDlg is already shown, quit 
			iFuncRet = -1;  break;
		}
		int8_t bgPlayerStat = -1;
#ifndef C_PC_SIM
		bgPlayerStat = get_player_backstage_state();
		if (bgPlayerStat < 0) {
			iFuncRet = -3;  break;
		}
#endif
		if (zb_screen_layer_is_full()) {
			iFuncRet = -5;  break;
		}

		lv_obj_t * bgObj = lv_obj_create(lv_layer_top());
		lv_obj_set_size(bgObj, SCREEN_WIDTH - 8 * 2, 56);
		lv_obj_set_style_pad_all(bgObj, 4, 0);
		lv_obj_align(bgObj, LV_ALIGN_TOP_MID, 0, 20);
		lv_obj_set_grid_dsc_array(bgObj, col_dsc, row_dsc);
		lv_obj_set_style_pad_row(bgObj, 0, 0);
		lv_obj_set_style_pad_column(bgObj, 0, 0);
		//lv_obj_set_style_bg_color(bgObj, lv_color_hex(0xff8c00), 0);
		const lv_img_dsc_t * imgBg = g_XXX_img_dsc_tbl + MMN_backstage_dlg_bg - 1;
		lv_obj_set_style_bg_img_src(bgObj, imgBg, 0);

		char * pcMscTitle = get_backstage_play_unit_name();
		if (!pcMscTitle || !pcMscTitle[0]) {
			pcMscTitle = g_pcSpaceTitle;
		}
		g_pcMscTitle = pcMscTitle;
		lv_obj_t * labelA = lv_label_create(bgObj);
		lv_label_set_text(labelA, pcMscTitle);
		lv_obj_set_grid_cell(labelA, LV_GRID_ALIGN_STRETCH, 0, 1,  LV_GRID_ALIGN_CENTER, 0, 1);
		lv_obj_align(labelA, LV_ALIGN_CENTER, 0, 0);
		lv_label_set_long_mode(labelA, LV_LABEL_LONG_SCROLL_CIRCULAR);
		lv_obj_set_style_text_align(labelA, LV_TEXT_ALIGN_CENTER, 0);
		// lv_obj_set_style_max_height(labelA, 26, 0);


		lv_obj_t * imgA = lv_img_create(bgObj);
		const lv_img_dsc_t * imgResume = g_XXX_img_dsc_tbl + MMN_btn_play - 1;
		const lv_img_dsc_t * imgPause  = g_XXX_img_dsc_tbl + MMN_btn_pause - 1;
		lv_obj_set_size(imgA, imgResume->header.w, imgResume->header.h);
		lv_obj_set_grid_cell(imgA, LV_GRID_ALIGN_CENTER, 1, 1,  LV_GRID_ALIGN_CENTER, 0, 1);
		g_imgA = imgA;

		lv_obj_set_style_bg_img_src(imgA, imgPause, 0);
		lv_obj_set_style_bg_img_src(imgA, imgResume, LV_STATE_CHECKED);

		_updPauseResumeBtn();
		lv_obj_add_event_cb(bgObj, bgPlayer_event_cb, LV_EVENT_ALL, NULL);

		g_timerA = lv_timer_create(_bgPlayer_timer_cb, 1000, NULL);

		sys_group = lv_group_create();
		ESP_LOGI(TAG, "bfr grpPush");
		zb_group_push(sys_group, 0);
		lv_group_add_obj(sys_group, bgObj);
		g_plyDlg = bgObj;
		iFuncRet = 0;
	}while (0);

	if (iFuncRet < 0) {
		ESP_LOGE(TAG, "showDlg fail %d", iFuncRet);
	}
	ESP_LOGI(TAG, "showDlg Zz");
	return iFuncRet;
}



