#include <lvgl/lvgl.h>

#include "../zb_base.h"
#include "../utils/misc_utils.h"
#include "../ex_widget/app_msg.h"
#include "./tch_mtrl.h"
#include "../include/img_a_tlb.h"
#include "../data_parse/img_res.h"

static const char* g_curSelectMtrl = NULL;
static lv_obj_t* g_tchMtrlContentListWin = NULL;
static lv_obj_t* g_tchMtrlItemSelectedTitle = NULL;

#define g_tchMtrlCount 10


static bool g_downloadSel[g_tchMtrlCount] = {0};
static char * text_name[g_tchMtrlCount] = {
	"Day1 单词列表一三十天背\n完小学单词",
	"Day2 单词列表二三十天背\n完小学单词",
	"Day3 单词列表三三十天背\n完小学单词", 
	"Day4 单词列表四三十天背\n完小学单词", 
	"Day5 单词列表五三十天背\n完小学单词", 
	"Day6 单词列表六三十天背\n完小学单词", 
	"Day7 单词列表七三十天背\n完小学单词", 
	"Day8 单词列表八三十天背\n完小学单词", 
	"Day9 单词列表九三十天背\n完小学单词", 
	"Day10 单词列表十三十天\n背完小学单词"
};

static void _tchMtrl_download_item_selected(lv_obj_t* img){
	static lv_img_dsc_t* img_selected = NULL;
	static bool isFirst = true;
	if (isFirst) {
		isFirst = false;
		img_selected = zb_img_res_load_one(NULL, TEXT_BOOK_ICON_IMG_SELECTED, IMG_FREE_TYPE_AUTO, NULL);
	}
	if(img&&img_selected != NULL) {
		lv_img_set_src(img, img_selected);
		lv_obj_set_size(img, img_selected->header.w, img_selected->header.h);
		lv_obj_align(img, LV_ALIGN_RIGHT_MID, 0, 0);
	}
}
static void _tchMtrl_download_item_unselected(lv_obj_t* img){
	static lv_img_dsc_t* img_unselected = NULL;
	static bool isFirst = true;
	if (isFirst) {
		isFirst = false;
		img_unselected = zb_img_res_load_one(NULL, TEXT_BOOK_ICON_IMG_UNSELECTED, IMG_FREE_TYPE_AUTO, NULL);
	}
	if (img && img_unselected != NULL){
		lv_img_set_src(img, img_unselected);
		lv_obj_set_size(img, img_unselected->header.w, img_unselected->header.h);
		lv_obj_align(img, LV_ALIGN_RIGHT_MID, 0, 0);
	}
}


void tch_mtrl_download_del(void){
    tch_mtrl_ap_win_deinit();
    tch_mtrl_content_init(g_curSelectMtrl);
}

void update_selected_title(void){
	int count = 0;
    for(uint32_t i = 0; i < g_tchMtrlCount; i++) {
        if(g_downloadSel[i]) {
            count++;
        }
    }
    char title[32] = {0};
    snprintf(title, sizeof(title), "下载(%d)", count);
    lv_label_set_text(g_tchMtrlItemSelectedTitle, title);
}

static void all_sel_btn_clickCb(void){
	bool all_sel_flag = false; //标识是全选还是全不选
	uint32_t i = 0;
	for(; i < g_tchMtrlCount; i++) 
		if(!g_downloadSel[i]) break;
	if(i == g_tchMtrlCount) all_sel_flag = false; //全不选
	else all_sel_flag = true; //全选

	uint32_t child_cnt = lv_obj_get_child_cnt(g_tchMtrlContentListWin);
    for(uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t* child = lv_obj_get_child(g_tchMtrlContentListWin, i);
        lv_obj_t* img = lv_obj_get_child(child, 0);
        if(all_sel_flag) _tchMtrl_download_item_selected(img);
		else _tchMtrl_download_item_unselected(img);
    }
    for(uint32_t i = 0; i < g_tchMtrlCount; i++) {
        g_downloadSel[i] = all_sel_flag;
    }
	update_selected_title();
    return;
}

static void _tchMtrl_download_item_btn_click_cb(lv_event_t* evt){
    int index = (int)lv_event_get_user_data(evt);
	lv_img_dsc_t* img_selected = zb_img_res_load_one(NULL, TEXT_BOOK_ICON_IMG_SELECTED, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_dsc_t* img_unselected = zb_img_res_load_one(NULL, TEXT_BOOK_ICON_IMG_UNSELECTED, IMG_FREE_TYPE_AUTO, NULL);
	if(g_downloadSel[index]) {
		_tchMtrl_download_item_unselected(lv_obj_get_child(lv_obj_get_child(g_tchMtrlContentListWin, index), 0));
		g_downloadSel[index] = false;
	}else{
		_tchMtrl_download_item_selected(lv_obj_get_child(lv_obj_get_child(g_tchMtrlContentListWin, index), 0));
		g_downloadSel[index] = true;
	}

	update_selected_title();
    return;
}


static void _tchMtrl_download_comform_click_cb(void){
    printf("download_btn_clickCb\n");
	//需要下载的文件下标已存在g_downloadSel数组中，遍历g_downloadSel数组，下载文件
}

void tch_mtrl_download_win_init(void) {
    g_tchCurWin = TCH_DOWNLOAD;
    lv_obj_t * mainBg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mainBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainBg, 0, 0);
	lv_obj_set_style_pad_all(mainBg, 0, 0);
	lv_color_t main_colorBg = lv_color_hex(TCH_MTRL_AP_BG_COLOR_HEX);
	lv_obj_set_style_bg_color(mainBg, main_colorBg, 0);
	lv_obj_set_scroll_dir(mainBg, LV_DIR_NONE);

    lv_obj_t * top_title_win = lv_obj_create(mainBg);
    lv_obj_set_size(top_title_win, LV_PCT(100), 60);
    lv_obj_align(top_title_win, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(top_title_win, main_colorBg, 0);
    lv_obj_set_style_border_width(top_title_win, 0, 0);
	lv_obj_set_style_pad_all(top_title_win, 0, 0);
	lv_obj_set_style_pad_row(top_title_win, 0, 0);
	lv_obj_set_style_pad_column(top_title_win, 0, 0);
	lv_obj_set_scroll_dir(top_title_win, LV_DIR_NONE);

    tchMtrl_createEscBtn(top_title_win, tch_mtrl_download_del, 0);
	lv_obj_t * lblTitle = lv_label_create(top_title_win);
    lv_label_set_text(lblTitle, "批量下载");
	lv_obj_align(lblTitle, LV_ALIGN_TOP_MID, 0, TCH_MTRL_ESC_BTN_LOC_Y + TOP_BTN_BORDER + TOP_BTN_TOP_SPACE_H);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), 0);

    lv_obj_t * all_sel_btn = lv_obj_create(top_title_win);
	lv_obj_set_size(all_sel_btn, 60, 40);
	lv_obj_align(all_sel_btn, LV_ALIGN_TOP_RIGHT, -16, 15);
	lv_obj_set_style_pad_all(all_sel_btn, TOP_BTN_BORDER, 0);
	lv_obj_set_style_radius(all_sel_btn, 4, 0);
	lv_obj_add_flag(all_sel_btn, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t* all_sel_title = lv_label_create(all_sel_btn);
    lv_label_set_text(all_sel_title, "全选");
	lv_obj_align(all_sel_title, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_text_color(all_sel_title, lv_color_hex(0x1A5FB4), 0);

	lv_color_t colorBg = lv_color_hex(0xc0c0c0);
	lv_obj_set_style_bg_color(all_sel_btn, colorBg, 0);
	lv_obj_set_style_bg_opa(all_sel_btn, LV_OPA_0, 0);
	lv_obj_set_style_bg_opa(all_sel_btn, LV_OPA_100, LV_STATE_FOCUS_KEY);
	lv_obj_set_style_bg_opa(all_sel_btn, LV_OPA_10, LV_STATE_PRESSED);
	
	lv_color_t colorBorderFcs = lv_color_white();
	lv_obj_set_style_border_color(all_sel_btn, colorBorderFcs, 0);
	lv_obj_add_event_cb(all_sel_btn, all_sel_btn_clickCb, LV_EVENT_CLICKED, NULL);

	g_tchMtrlContentListWin = lv_obj_create(mainBg);
	lv_obj_set_size(g_tchMtrlContentListWin, LV_PCT(100), SCREEN_HEIGHT - 130);	//todo 260 为顶部标题栏高度 + 底部播放按钮高度
	lv_obj_align_to(g_tchMtrlContentListWin,top_title_win, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
	lv_obj_set_style_bg_color(g_tchMtrlContentListWin, main_colorBg, 0);
	lv_obj_set_style_border_width(g_tchMtrlContentListWin, 0,0);
	lv_obj_set_scrollbar_mode(g_tchMtrlContentListWin, LV_SCROLLBAR_MODE_OFF);  // 关闭滚动条

	lv_obj_set_flex_flow(g_tchMtrlContentListWin, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(g_tchMtrlContentListWin, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_top(g_tchMtrlContentListWin, 0, 0);
	lv_obj_set_style_pad_left(g_tchMtrlContentListWin, 15, 0);
	lv_obj_set_style_pad_right(g_tchMtrlContentListWin, 15, 0);
	lv_obj_set_style_pad_row(g_tchMtrlContentListWin, 15, 0);						// 行间距固定 20px
	lv_obj_set_style_pad_column(g_tchMtrlContentListWin, 10, 0);

	static lv_style_t style_btn;
	static bool isFirst = true;
	if (isFirst) {
	isFirst = false;
	lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, lv_color_hex(0xF5F5F5)); // 未选中时浅灰色背景
    lv_style_set_bg_opa(&style_btn, LV_OPA_100);
    lv_style_set_text_color(&style_btn, lv_color_hex(0x000000)); // 黑色字体 #000000
    lv_style_set_radius(&style_btn, 10);                         // 16px圆角
    lv_style_set_pad_hor(&style_btn, 20);                        // 水平内边距
    lv_style_set_pad_ver(&style_btn, 6);
    lv_style_set_border_width(&style_btn, 0); // 无边框
    lv_style_set_shadow_width(&style_btn, 0); // 无阴影
	}

	memset(g_downloadSel, 0, sizeof(g_downloadSel));

	
	for (int i = 0; i < g_tchMtrlCount; i++) {
		char * text = text_name[i];
		lv_obj_t* btn = lv_btn_create(g_tchMtrlContentListWin);
		lv_obj_set_size(btn, 300, 75);
		lv_obj_add_style(btn, &style_btn, 0);

		lv_obj_t* img = lv_img_create(btn);
		_tchMtrl_download_item_unselected(img);

		lv_obj_t * label = lv_label_create(btn);
    	lv_label_set_text(label, text);
    	lv_obj_align_to(label, img, LV_ALIGN_OUT_LEFT_MID, -10, 0);
        lv_obj_add_event_cb(btn, _tchMtrl_download_item_btn_click_cb, LV_EVENT_CLICKED, i);
		lv_obj_set_style_bg_opa(btn, LV_OPA_50, LV_EVENT_PRESSING);
	}

	lv_obj_t *conform_btn = lv_btn_create(mainBg);
	lv_obj_set_size(conform_btn, 300, 45);
	lv_obj_align_to(conform_btn, g_tchMtrlContentListWin, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_set_style_radius(conform_btn, 25, 0);
	g_tchMtrlItemSelectedTitle = lv_label_create(conform_btn);
    lv_label_set_text(g_tchMtrlItemSelectedTitle, "下载(0)");
    lv_obj_center(g_tchMtrlItemSelectedTitle);
	lv_obj_set_style_bg_opa(conform_btn, LV_OPA_100, 0);
	lv_obj_set_style_bg_opa(conform_btn, LV_OPA_50, LV_EVENT_PRESSED);
	lv_obj_set_style_shadow_width(conform_btn, 0, 0);
    lv_obj_add_event_cb(conform_btn, _tchMtrl_download_comform_click_cb, LV_EVENT_CLICKED,NULL);

    sys_setCurMainWinAndCb(mainBg, _tch_mtrl_scr_cb);
    g_tchMnuBgWin = mainBg;
}

void tch_mtrl_download_init(const char* name) {
    g_curSelectMtrl = name;
    tch_mtrl_download_win_init();
}