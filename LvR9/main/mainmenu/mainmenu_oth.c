
#include <time.h>
#include <lvgl/lvgl.h>
#include "../utils/misc_utils.h"
#include "../utils/global_parm.h"
#include "../data_parse/img_res.h"
#include "../ex_widget/app_msg.h"
#include "../ex_widget/misc_draw.h"
#include "../setting/app_setting_base.h"
#include "../utils/style_tz.h"
#ifndef C_PC_SIM
#include "../include/adc_battery.h"

#endif

//#include "../setting/parental_control_pin.h"
#include "tlb_main_img.h"

#include "./mainmenu_base.h"

//--------------------------------------
extern int16_t  g_curTimeHM;

extern const lv_img_dsc_t  g_XXX_img_dsc_tbl[];


//--------------------------------------
#define MMN_BTN_CHECK_COLOR		0x00ff00    // 0xFFB3D7	// btn 颜色
#define MMN_BTN_TXT_COLOR		0x000000    // 0xFFFFFF	//字体色

//--------------------------------------

static int8_t   g_hasShowTipsScr;
static lv_obj_t * g_rootWin;
static lv_obj_t * g_mmnOthBtns[2];
static lv_obj_t * g_statBar;
static int8_t g_btnSelNo = 0;

bool setNetworkStat = false;
//--------------------------------------
static const char  * TAG = "mmnOth";

//--------------------------------------

//----------------------------------------------
static void _mmnOth_selBtn(int8_t selNo)
{
	for (int sn = 0; sn < 2; sn++) {
		if (sn == selNo) {
			lv_obj_add_state(g_mmnOthBtns[sn], LV_STATE_CHECKED);
		}
		else {
			lv_obj_clear_state(g_mmnOthBtns[sn], LV_STATE_CHECKED);
		}
		lv_obj_invalidate(g_mmnOthBtns[sn]);
	}
	g_btnSelNo = selNo;
}


static void _mmnOth_draw_statbar_cb(lv_event_t * e) {
#define MMN_RECHARGE_ICN_W  18
    lv_obj_t * obj = lv_event_get_target(e);
    lv_draw_ctx_t * draw_ctx = lv_event_get_draw_ctx(e);

    lv_area_t widget_coords;
    lv_obj_get_coords(obj, &widget_coords);

    uint8_t batteryLevelNo = mmn_get_batteryLevelNo();
    int batteryLevel = (batteryLevelNo & 0x0f);
    bool rechargeStat = ((batteryLevelNo & C_RECHARGE_FLAG) ? true : false);
    const lv_img_dsc_t  * pImgDscBat = g_XXX_img_dsc_tbl + MMN_battery_L_0 + batteryLevel - 1;
    int16_t iconW = pImgDscBat->header.w;
    lv_area_t icn_coords = widget_coords;
    icn_coords.x1 = SCREEN_WIDTH - 32;
    icn_coords.x2 = icn_coords.x1 + iconW - 1;
    icn_coords.y2 = icn_coords.y1 + pImgDscBat->header.h - 1;
    lv_draw_img_dsc_t dsc;
    lv_draw_img_dsc_init(&dsc);
    dsc.opa = LV_OPA_100;
    dsc.blend_mode = LV_BLEND_MODE_MULTIPLY;  // LV_BLEND_MODE_NORMAL;
    lv_draw_img(draw_ctx, &dsc, &icn_coords,  pImgDscBat);  // 

    if (rechargeStat) {
        const lv_img_dsc_t  * pImgDscRech = g_XXX_img_dsc_tbl + MMN_recharge_flag_a - 1;
        icn_coords = widget_coords;
        iconW = pImgDscRech->header.w;
        icn_coords.x1 = SCREEN_WIDTH - 32 - MMN_RECHARGE_ICN_W;
        icn_coords.x2 = icn_coords.x1 + iconW - 1;
        icn_coords.y2 = icn_coords.y1 + pImgDscRech->header.h - 1;
        lv_draw_img_dsc_init(&dsc);
        dsc.blend_mode = LV_BLEND_MODE_NORMAL; //  LV_BLEND_MODE_MULTIPLY
        lv_draw_img(draw_ctx, &dsc, &icn_coords,  pImgDscRech);	// 
    }

    //----------------------
    if (g_curTimeHM != -1) {
        lv_draw_label_dsc_t label_draw_dsc;
        lv_draw_label_dsc_init(&label_draw_dsc);
        lv_color_t colorTxt = LV_COLOR_MAKE(0, 0, 0);  // LV_COLOR_MAKE(0, 0xe0, 0);
        label_draw_dsc.color = colorTxt;
        label_draw_dsc.align = LV_TEXT_ALIGN_LEFT;

        label_draw_dsc.ofs_x = 12;
        label_draw_dsc.ofs_y = 1;
        label_draw_dsc.font = &lv_font_montserrat_16;
        char acTmText[20];
        snprintf(acTmText, sizeof(acTmText), "%02hd:%02hd", (int16_t)((g_curTimeHM>>8)&0x3f), (int16_t)(g_curTimeHM&0x3f));
        //lv_obj_set_style_text_font(obj, const lv_font_t * value, 0);
        lv_draw_label(draw_ctx, &label_draw_dsc, &widget_coords, acTmText, NULL);
    }
    //----------------------
    return;
}


static void _promptForNewUsr_draw_cb(lv_event_t * evt)
{
	return;
}

static void _promptForNewUsr_scr_cb(lv_event_t * evt) {
	lv_event_code_t evtCode = evt->code;

#ifdef C_PC_SIM
	int keyVal = 0;
	switch (evtCode) {
		case LV_EVENT_DRAW_POST:
			_promptForNewUsr_draw_cb(evt);
			break;
		case LV_EVENT_KEY_STAGE:
			keyVal = lv_event_get_key(evt);
			switch (keyVal) {
				case (LV_EX_FLAG_KEY_SHORT | LV_KEY_ENTER): {
					ESP_LOGI(TAG, "mmnu keyEnt");
					//main_menu_pg_init(MMN_SCR_NO_NET_CFG, 0);
					if (0 == g_btnSelNo) { // skip.
						main_menu_pg_init(0, 0);
					}
					else {
						int16_t skipVal = 1;
						//setPrjGlobalParm(ZB_PARM_SKIP_TIPS_FOR_NEWUSR, 1, &skipVal);
						main_menu_pg_init(0, 0);
					}
					break;
				}
				case (LV_EX_FLAG_KEY_SHORT | LV_KEY_ESC): {
					//ESP_LOGI(TAG, "mmnu keyEsc %d", (int)scrNo);
					main_menu_pg_init(0, 0);
					break;
				}
				case LV_EX_FLAG_KEY_SHORT | LV_KEY_LEFT:
				case LV_EX_FLAG_KEY_SHORT | LV_KEY_RIGHT: {
					_mmnOth_selBtn(0 == g_btnSelNo ? 1 : 0);
					break;
				}
				default:
					break;
			}
			break;
		case LV_EVENT_KEY: {
			if (!keyVal) {
				keyVal = lv_event_get_key(evt);
			}
			ESP_LOGI(TAG, "mmnu key %x", keyVal);
			break;
		}
		case LV_EVENT_DELETE: {
			g_hasShowTipsScr = 1;
			g_statBar = NULL;
			g_rootWin = NULL;
			break;
		}
		default:
			break;
	}
#endif
	return;
}


static void _mmn_createOneBtn(lv_obj_t * parent, int sn, const char * pcTitle)
{
	lv_obj_t * objBtn = lv_btn_create(parent);
	//lv_obj_remove_style(objBtn, NULL, 0);
	lv_obj_set_size(objBtn, (0 == sn ? 56 : 76), 35);
	lv_obj_set_style_border_width(objBtn , 0, 0);
	lv_obj_set_style_pad_all(objBtn, 0, 0);
	lv_obj_set_style_radius(objBtn, 20, 0);
	lv_obj_set_style_bg_opa(objBtn, LV_OPA_COVER, 0);	
	lv_obj_set_style_bg_color(objBtn, lv_color_white(), 0);  // lv_color_hex(MMN_BTN_CHECK_COLOR)
	lv_obj_set_style_bg_color(objBtn, lv_color_white(), LV_STATE_CHECKED);
	lv_obj_set_style_outline_width(objBtn, 1, 0);
	lv_obj_set_style_outline_color(objBtn, lv_color_black(), 0);
	lv_color_t  colorLine = lv_color_hex(0xa0a0a0);
	lv_obj_set_style_outline_color(objBtn, colorLine, 0);
	lv_obj_set_style_outline_color(objBtn, lv_color_black(), LV_STATE_CHECKED);
	lv_obj_set_style_outline_opa(objBtn, LV_OPA_COVER, 0);
	//lv_obj_set_style_outline_opa(objBtn, LV_OPA_COVER, LV_STATE_CHECKED);
	lv_obj_set_style_shadow_width(objBtn, 0, 0);
	lv_group_remove_obj(objBtn);

	lv_coord_t  x = (0 == sn ? 24 : 128);
	lv_obj_align(objBtn,  LV_ALIGN_BOTTOM_LEFT, x, -22);

	lv_obj_t * btn_label = lv_label_create(objBtn);
	//lv_obj_remove_style(btn_label, NULL, 0);
	lv_label_set_text(btn_label, pcTitle);
	lv_obj_set_style_text_color(btn_label, lv_color_hex(MMN_BTN_TXT_COLOR), 0);
	lv_obj_center(btn_label);
	//lv_group_focus_obj(objBtn);
	g_mmnOthBtns[sn] = objBtn;
	return;
}


//弹窗提示图片 请连接网络  cg  tag
int mmn_promptForNewUsr_scr_init(void) {
	ESP_LOGI(TAG, "prptNUsrScr A");

	memset(g_mmnOthBtns, 0, sizeof(g_mmnOthBtns));
	lv_obj_t * winBg = lv_obj_create(lv_scr_act());
	lv_obj_set_size(winBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(winBg, 0, 0);
	lv_obj_set_style_pad_all(winBg, 0, 0);
	lv_obj_set_style_radius(winBg, 0, 0);
	lv_obj_set_style_bg_color(winBg, lv_color_hex(0xfff4d2), 0);

	//lv_obj_t* label = lv_label_create(winBg);

	lv_obj_t * pObjStatBar = lv_obj_create(winBg);
	lv_obj_set_size(pObjStatBar, SCREEN_WIDTH, MMNU_STATUS_BAR_H);
	lv_obj_set_style_border_width(pObjStatBar, 0, 0);
	lv_obj_add_event_cb(pObjStatBar, _mmnOth_draw_statbar_cb, LV_EVENT_DRAW_MAIN, NULL);
	g_statBar = pObjStatBar;

	_mmn_createOneBtn(winBg, 0, "进入");
	_mmn_createOneBtn(winBg, 1, "不再提示");

	lv_obj_set_scrollbar_mode(winBg, LV_SCROLLBAR_MODE_OFF);
	lv_obj_add_event_cb(winBg, _promptForNewUsr_scr_cb, LV_EVENT_ALL, NULL);
	lv_group_add_obj(lv_group_get_default(), winBg);
	lv_group_focus_obj(winBg);
	g_rootWin = winBg;
	_mmnOth_selBtn(0);
	// mmn_set_screenNo(MMN_SCR_NO_PROMPT_NET_CFG, MMN_CHECK_CODE);
	return 0;
}



static void pop_skip_event_cb(lv_event_t* e)//用于设置中的跳转
{
#define  CMD_PAR_ENTER_SUB  0xc1
	NetStatusPopup* popup = lv_event_get_user_data(e);

	lv_obj_del(popup->mask_layer);
	free(popup);

	//setting_update_del();
	_stng_curScr_del();
	set_g_setngSel_num(0);
	lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_ENTER_SUB);
	//setting_update_del();
	setNetworkStat = true;
	//network_setting_entry();
}

static void close_popup_event(lv_event_t* e)
{
	NetStatusPopup* popup = lv_event_get_user_data(e);

	lv_obj_del(popup->mask_layer);
	free(popup);
}

//创建弹窗
lv_obj_t* create_network_popup(lv_obj_t* parent,const char* tip,const char*label,int y)//188
{

#define AFFIRM_BTN_W		200
#define AFFIRM_BTN_H		46
#define AFFIRM_BTN_X		0
#define AFFIRM_BTN_Y		30
#define ENTER_BTN_COLOR	0x008CFF
	/* 层级管理策略：
	   1. 如果传入parent，则基于当前页面创建
	   2. 如果parent为NULL，则自动附加到top屏幕
	*/
	lv_obj_t* root = parent ? parent : lv_layer_top();

	NetStatusPopup* popup = malloc(sizeof(NetStatusPopup));
	memset(popup, 0, sizeof(NetStatusPopup));

	popup->mask_layer = lv_obj_create(root);
	lv_obj_set_scroll_dir(popup->mask_layer, LV_DIR_NONE);
	lv_obj_set_size(popup->mask_layer, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_bg_color(popup->mask_layer, lv_color_hex(0x000000),0);
	lv_obj_clear_flag(popup->mask_layer, LV_OBJ_FLAG_SCROLLABLE);

	//lv_obj_add_flag(popup->mask_layer, LV_OBJ_FLAG_CLICKABLE); //暂不加点击黑色幕布部分删除处理
	lv_obj_set_style_pad_all(popup->mask_layer, 0, 0);
	lv_obj_set_style_radius(popup->mask_layer, 0, 0);
	lv_obj_set_style_border_width(popup->mask_layer, 0, 0);
	lv_obj_set_style_bg_color(popup->mask_layer, lv_color_hex(0x000000), 0);
	lv_obj_set_style_bg_opa(popup->mask_layer, LV_OPA_70, 0);

	popup->popup = lv_obj_create(popup->mask_layer);
	lv_obj_set_size(popup->popup, SCREEN_WIDTH, y);
	lv_obj_align(popup->popup, LV_ALIGN_BOTTOM_MID, 0, 20);
	lv_obj_set_style_bg_color(popup->popup, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
	lv_obj_set_style_radius(popup->popup, 19, LV_PART_MAIN);
	lv_obj_clear_flag(popup->popup, LV_OBJ_FLAG_SCROLLABLE);

	lv_obj_t* del_img = lv_btn_create(popup->popup);
	lv_obj_set_size(del_img, 50, 50);
	lv_obj_set_style_radius(del_img, 17, 0);
	lv_obj_set_style_bg_color(del_img, lv_color_hex(0xffffff), 0);
	lv_obj_set_style_bg_opa(del_img, LV_OPA_0, 0);
	lv_obj_set_style_shadow_width(del_img, 0, 0);
	lv_img_dsc_t* col_img = zb_img_res_load_one(NULL, SETTING_IMG_DEL, IMG_FREE_TYPE_AUTO, NULL);
	lv_obj_set_style_bg_img_src(del_img, col_img, 0);
	lv_obj_align(del_img, LV_ALIGN_TOP_RIGHT, -10, 5);
	lv_obj_add_event_cb(del_img, close_popup_event, LV_EVENT_CLICKED, popup);

	lv_obj_t* rect_tip = lv_label_create(popup->popup);
	lv_label_set_text(rect_tip, tip);
	lv_obj_set_style_text_color(rect_tip, lv_color_hex(0x18191B), 0);
	lv_obj_align(rect_tip, LV_ALIGN_TOP_LEFT, 20, 20);

	lv_obj_t* rect_labe = lv_label_create(popup->popup);
	lv_label_set_text(rect_labe, label);
	lv_obj_set_width(rect_labe, 270); 
	lv_label_set_long_mode(rect_labe, LV_LABEL_LONG_WRAP); 
	lv_obj_set_style_text_color(rect_labe, lv_color_hex(0x787F87), 0);
	lv_obj_align(rect_labe, LV_ALIGN_LEFT_MID, 20, -20);
	
	//if (is_sos_page_active()) { 
	//	lv_obj_move_foreground(lv_layer_sys()); 
	//	lv_obj_set_style_bg_opa(popup->mask_layer, LV_OPA_30); 
	//}

	return popup->mask_layer;
}

//后续不同的弹窗显示统一管理
lv_obj_t* create_custom_button(lv_obj_t* parent, const char* text, lv_event_cb_t event_cb, void* user_data) {
	lv_obj_t* btn = lv_btn_create(parent);
	lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -30);

	lv_obj_t* btn_label = lv_label_create(btn);
	lv_label_set_text(btn_label, text);
	lv_obj_align(btn_label, LV_ALIGN_CENTER, 0, 0);

	lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, user_data);

	return btn;
}

static lv_obj_t* dialog_ximaLY = NULL; // 存储对话框句柄
static lv_timer_t* dialog_timer = NULL; // 存储定时器标识符
static void dialog_delete_event_cb(lv_event_t* e) {
	ESP_LOGI("dialog_cb", "Dialog deleted");

	dialog_ximaLY = NULL;

	if (dialog_timer != NULL) {
		lv_timer_del(dialog_timer);
		dialog_timer = NULL;
	}
}


static void dialog_timer_cb(lv_timer_t* timer) {

	if (dialog_ximaLY != NULL) {
		lv_obj_del(dialog_ximaLY);
		dialog_ximaLY = NULL;
	}
	if (dialog_timer != NULL) {
		lv_timer_del(dialog_timer);
		dialog_timer = NULL;
	}

}

void del_dialog()
{
	if (dialog_ximaLY != NULL) {
		lv_obj_del(dialog_ximaLY);
		dialog_ximaLY = NULL;
	}
	if (dialog_timer != NULL) {
		lv_timer_del(dialog_timer);
		dialog_timer = NULL;
	}
}


//弹窗显示开发中
void create_dialog_with_timer(bool icon,const char* text, uint32_t timeout_ms,int x,int y) {
	if (dialog_ximaLY != NULL) {
		lv_obj_del(dialog_ximaLY);
		dialog_ximaLY = NULL;
	}
	if (dialog_timer != NULL) {
		lv_timer_del(dialog_timer);
		dialog_timer = NULL;
		ESP_LOGI("dialogTimer", "定时器已经删除");
	}


	dialog_ximaLY = lv_obj_create(lv_scr_act());
	lv_obj_add_style(dialog_ximaLY, get_global_style(STYLE_POPUP_BG), LV_PART_MAIN | LV_STATE_DEFAULT);


	if (x != 0 || y != 0) {
		lv_obj_set_size(dialog_ximaLY, x, y);
	}
	else {
		lv_obj_set_size(dialog_ximaLY, 140, 88);
	}


	lv_obj_align(dialog_ximaLY, LV_ALIGN_CENTER, 0, 0);


	lv_obj_t* label = lv_label_create(dialog_ximaLY);
#ifndef C_PC_SIM
	lv_label_set_cjk_font(label, 18, FT_FONT_STYLE_NORMAL);
#endif
	lv_label_set_text(label, text);
	lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

	
	lv_obj_set_width(label, lv_pct(92)); 
	lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0); 

	if (!icon) {
		
		lv_obj_center(label);
	}
	else {
		
		uint32_t img_res_id = SETTING_IMG_ARROW;
		if (strcmp(text, "连接成功") == 0) img_res_id = SETTING_IMG_CONNECT_SUCCESS;
		else if (strcmp(text, "连接失败") == 0) img_res_id = SETTING_IMG_CONNECT_ERROR;

		lv_obj_t* icon_obj = lv_img_create(dialog_ximaLY);
		lv_img_dsc_t* pImgDsc_img = zb_img_res_load_one(NULL, img_res_id, IMG_FREE_TYPE_AUTO, NULL);
		lv_img_set_src(icon_obj, pImgDsc_img);
		lv_obj_align(icon_obj, LV_ALIGN_TOP_MID, 0, 10); 

		
		lv_obj_align_to(label, icon_obj, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

		
		lv_coord_t content_height = lv_obj_get_height(icon_obj) +
			lv_obj_get_height(label) + 30; 
		if (content_height > y) {
			lv_obj_set_height(dialog_ximaLY, content_height);
		}
	}

	
	lv_obj_align(dialog_ximaLY, LV_ALIGN_CENTER, 0, 0);

	if (timeout_ms > 0) {
		dialog_timer = lv_timer_create(dialog_timer_cb, timeout_ms, NULL);
		if (dialog_timer == NULL) {
			ESP_LOGE("dialogTimer", "Failed to create timer");
			return;
		}
		ESP_LOGI("dialogTimer", "创建定时器");
	}

	lv_obj_add_event_cb(dialog_ximaLY, dialog_delete_event_cb, LV_EVENT_DELETE, NULL);
}









void mmnOth_draw_spaceItem_cb(lv_event_t * e) {
#if 0
#define MMN_RECHARGE_ICN_W  18
    lv_obj_t * obj = lv_event_get_target(e);
    lv_draw_ctx_t * draw_ctx = lv_event_get_draw_ctx(e);

    lv_area_t widget_coords;
    lv_obj_get_coords(obj, &widget_coords);
    //----------------------
    lv_draw_label_dsc_t label_draw_dsc;
    lv_draw_label_dsc_init(&label_draw_dsc);
    lv_color_t colorTxt =  lv_color_white();  // LV_COLOR_MAKE(0, 0xe0, 0)
    label_draw_dsc.color = colorTxt;
    label_draw_dsc.align = LV_TEXT_ALIGN_CENTER;

    label_draw_dsc.ofs_x = 0;
    label_draw_dsc.ofs_y = MMNU_GRID_ROW_H - 16/2 - 8;
    label_draw_dsc.font = &lv_font_main_cjk; 
    //lv_obj_set_style_text_font(obj, const lv_font_t * value, 0);
    lv_draw_label(draw_ctx, &label_draw_dsc, &widget_coords, "更新中", NULL);
#endif
	return;
}



bool mmn_promptForNewUsr_scr_wasShown(void)
{
	return g_hasShowTipsScr;
}

void mmn_promptForNewUsr_scr_deinit(void)
{
	if (g_rootWin) {
		lv_obj_del(g_rootWin);
	}
}

void mmn_promptForNewUsr_scr_invalid(void)
{
	if (NULL == g_statBar)
		return;
	lv_obj_invalidate(g_statBar);
}


