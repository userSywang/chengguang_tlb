#include <lvgl/lvgl.h>

#include "../zb_base.h"
#include "../utils/misc_utils.h"
#include "../ex_widget/app_msg.h"
#include "./tch_mtrl.h"
#include "../include/img_a_tlb.h"
#include "../data_parse/img_res.h"
#include "../player/player_data_parse.h"
#include "book_select.h"
#include "../player/player.h"

static const char* g_curSelectMtrl = NULL;
static const char* g_curSelectMtrlIndex = ""; 	//当前正在播放的音频name

static lv_obj_t* g_tchMtrlContentListWin = NULL;

extern int16_t ap_toggle_num;

static lv_style_t style_btn;
static player_book_info_t play_list_book_info = { 0 };
void tch_mtrl_content_win_init(void);

#define g_tchMtrlCount 10

static char * text_name[g_tchMtrlCount] = {
	"Day1 单词列表1",
	"Day2 单词列表2",
	"Day3 单词列表3", 
	"Day4 单词列表4", 
	"Day5 单词列表5", 
	"Day6 单词列表6", 
	"Day7 单词列表7", 
	"Day8 单词列表8", 
	"Day9 单词列表9", 
	"Day10 单词列表10"
};

void tch_mtrl_content_del(void){
    tch_mtrl_ap_win_deinit();
    tch_mtrl_win_init(FUN_BACKWARD);
}

static void _tchMtrl_play_video_by_name(const char* name) {
    printf("play video by name : %s\n", name);
	uint32_t child_cnt = lv_obj_get_child_cnt(g_tchMtrlContentListWin);
	lv_img_dsc_t* selected_img = zb_img_res_load_one(NULL, TEXT_BOOK_ICON_IMG_VIDEOPLAYING, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_dsc_t* new_img = zb_img_res_load_one(NULL, TEXT_BOOK_ICON_IMG_VIDEONOPLAYING, IMG_FREE_TYPE_AUTO, NULL);
    for(uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t* child = lv_obj_get_child(g_tchMtrlContentListWin, i);
        lv_obj_t* img = lv_obj_get_child(child, 0);
		lv_obj_t* label = lv_obj_get_child(child, 1); // 获取按钮中的label对象
		if(label && strcmp(lv_label_get_text(label), name) == 0)
            lv_img_set_src(img, selected_img);
        else {
            lv_img_set_src(img, new_img);
        }
    }
    
    
    // 更新全局索引
    g_curSelectMtrlIndex = name;  
    printf("content : %s\n", g_curSelectMtrlIndex);

	//todo 播放音频 需要播放的音频名称为g_curSelectMtrlIndex
    return;
}

//todo 根据传入的名字进行格式化返回显示用的名字
static const char* content_win_show_name(const char* name) {
	return gp_cur_book->name;
	//return get_book_name_by_bookbin(name);
	//return "一年级上（小学人教...";
}

static void download_btn_clickCb(lv_event_t* evt) { 
    tch_mtrl_ap_win_deinit();
    tch_mtrl_download_init(g_curSelectMtrl);
    return; 
}

void tch_mtrl_back_content(void) {
	//lv_obj_t* child = lv_obj_get_child(g_tchMtrlContentListWin, play_text_book_locate.playNum);
	//sys_setCurMainWinAndCb(g_tchMtrlContentListWin, child);
	sys_setCurMainWinAndCb(g_tchMnuBgWin, _tch_mtrl_scr_cb);
	lv_indev_keypad_wait_release();
	//lv_obj_scroll_to_view(child, LV_ANIM_OFF);
}

static void play_all_btn_clickCb(lv_event_t* evt) {
	if(strcmp(g_curSelectMtrlIndex, "") == 0) 
		_tchMtrl_play_video_by_name(text_name[0]);
	else
		_tchMtrl_play_video_by_name(g_curSelectMtrlIndex);
	return;
}

static void _tchMtrl_content_item_btn_click_cb(lv_event_t* evt) {
	const char* text = (const char*)lv_event_get_user_data(evt);
	lv_obj_t* obj = lv_event_get_target(evt);
	_tchMtrl_play_video_by_name(text);


	char str[160];
	sprintf(str, "/books/%s/book000", g_curSelectMtrl);
	printf("str:%s",str);

	uint16_t playNum = lv_obj_get_index(obj);

	uint16_t g_subjectSel = return_g_subjectSel();
	int ap_num = 0;
	if (g_subjectSel == TCH_MTRL_TEXT_CN)
		ap_num = C_ZB_AP_TEXTBOOK_CN;
	else if (g_subjectSel == TCH_MTRL_TEXT_ENG)
		ap_num = C_ZB_AP_TEXTBOOK_ENG;

	play_album_page_init(str, playNum, ap_num);//DATUM_CHINESE_TEXTBOOK

}


void tch_mtrl_content_win_init(void) {
	char buf[300] = { 0 };
	//if (strncmp(g_curSelectMtrl, "一年级上", 12) == 0) //失敗
	//	snprintf(buf, sizeof(buf), "/userdata/_data/textbook/english/pep/tape1a_002015_202503241812/album.mnu");
	//else
	//	snprintf(buf, sizeof(buf), "/userdata/_data/textbook/english/pep/tape3a_002003_202409180537/album.mnu");
	snprintf(buf, sizeof(buf), "/%s/_data/textbook/books/%s/book000/album.mnu",BOOK_PATH, g_curSelectMtrl);
	ESP_LOGI("", "%s[%d], path:%s ,size: %d\n", __FUNCTION__, __LINE__, buf, strlen(buf));
	printf("準備打開文件: %s\n", buf);

	int ret = player_get_book_inf(buf, &play_list_book_info);
	if (ret)
	{
		ESP_LOGI("player_list", "%s\n", "get book menu fail");
		printf("準備打開文件失敗: %s\n", buf);
		return ;
	}

	char* _display_cover = true; // false;
	//char* _display_cover = false;

    g_tchCurWin = TCH_CONTENT;
    lv_obj_t * mainBg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mainBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainBg, 0, 0);
	lv_obj_set_style_pad_all(mainBg, 0, 0);
	lv_color_t main_colorBg = lv_color_hex(TCH_MTRL_AP_BG_COLOR_HEX);
	lv_obj_set_style_bg_color(mainBg, main_colorBg, 0);
	lv_obj_set_scroll_dir(mainBg, LV_DIR_NONE);

    lv_obj_t * top_title_win = lv_obj_create(mainBg);
    lv_obj_set_size(top_title_win, SCREEN_WIDTH, TCH_HEADER_HEIGHT);
    lv_obj_align(top_title_win, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(top_title_win, main_colorBg, 0);
    lv_obj_set_style_border_width(top_title_win, 0, 0);
	lv_obj_set_style_pad_all(top_title_win, 0, 0);
	lv_obj_set_style_pad_row(top_title_win, 0, 0);
	lv_obj_set_style_pad_column(top_title_win, 0, 0);
	lv_obj_set_scroll_dir(top_title_win, LV_DIR_NONE);

	//lv_obj_set_style_border_color(top_title_win, lv_color_black(), 0);
	//lv_obj_set_style_border_width(top_title_win, 1, 0);


	//lv_obj_t *esc_img = tchMtrl_createEscBtn(top_title_win, tch_mtrl_content_del, 0,16,15);
	lv_obj_t *esc_img = tchMtrl_createEscBtn(top_title_win, tch_mtrl_content_del, 0,20 - TOP_BTN_BORDER, 14 - TOP_BTN_BORDER);
	lv_obj_t * lblTitle = lv_label_create(top_title_win);
#ifndef C_PC_SIM
	lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
#endif

	char* bookName = get_cur_book_name();
	char name_buf[500] = { 0 };
	size_t max_chars = 15;
        size_t char_count = 0;
        const char *p = bookName;
        while(*p && char_count < max_chars) {
            if((*p & 0xC0) != 0x80) char_count++;
            p++;
        }

        if(char_count >= max_chars) {
            size_t pos = 0;
            size_t count = 0;
            const char *ptr = bookName;
            
            while(*ptr && count < max_chars-3) {
                if((*ptr & 0xC0) != 0x80) count++;
                pos++;
                ptr = bookName + pos;
            }
            
            strncpy(name_buf, bookName, pos);
            strcat(name_buf, "...");
			bookName = name_buf;
        }
    lv_label_set_text(lblTitle,bookName );
	//lv_obj_set_pos(lblTitle, )
	
	//lv_label_set_long_mode(lblTitle, LV_LABEL_LONG_SCROLL_CIRCULAR);

	lv_obj_align(lblTitle,LV_ALIGN_CENTER, 5, 0);
	//lv_obj_align(lblTitle, LV_ALIGN_TOP_MID, 0, TCH_MTRL_ESC_BTN_LOC_Y + TOP_BTN_BORDER + TOP_BTN_TOP_SPACE_H);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), LV_STATE_DEFAULT);
	
	//lv_obj_set_style_border_color(lblTitle, lv_color_make(0x180, 0x0, 0x00), 0);
	//lv_obj_set_style_border_width(lblTitle, 1, 0);
	
	lv_color_t colorBg = lv_color_hex(0xc0c0c0);
	lv_color_t colorBorderFcs = lv_color_white();
	if (false) {
		lv_obj_t* download_btn = lv_img_create(top_title_win);
		lv_img_dsc_t* download_btn_img = zb_img_res_load_one(NULL, TEXT_BOOK_ICON_IMG_ICONDOWNLOAD, IMG_FREE_TYPE_AUTO, NULL);
		lv_img_set_src(download_btn, download_btn_img);
		lv_obj_set_size(download_btn, download_btn_img->header.w + TOP_BTN_BORDER * 2, download_btn_img->header.h + TOP_BTN_BORDER * 2);
		lv_obj_align(download_btn, LV_ALIGN_TOP_RIGHT, -16, 10);
		lv_obj_set_style_pad_all(download_btn, TOP_BTN_BORDER, 0);
		lv_obj_set_style_radius(download_btn, 4, 0);
		lv_obj_add_flag(download_btn, LV_OBJ_FLAG_CLICKABLE);

		lv_obj_set_style_bg_color(download_btn, colorBg, 0);
		lv_obj_set_style_bg_opa(download_btn, LV_OPA_0, 0);
		lv_obj_set_style_bg_opa(download_btn, LV_OPA_100, LV_STATE_FOCUS_KEY);
		lv_obj_set_style_bg_opa(download_btn, LV_OPA_10, LV_STATE_PRESSED);

		lv_obj_set_style_border_color(download_btn, colorBorderFcs, 0);
		lv_obj_add_event_cb(download_btn, download_btn_clickCb, LV_EVENT_CLICKED, NULL);
	}

	lv_obj_t* img_show_win = NULL;
	if (_display_cover) {
		img_show_win = lv_obj_create(mainBg);
		lv_obj_set_size(img_show_win, LV_PCT(100), 162);
		lv_obj_align_to(img_show_win, top_title_win, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
		lv_obj_set_style_bg_color(img_show_win, main_colorBg, 0);
		lv_obj_set_style_border_width(img_show_win, 0, 0);
		lv_obj_set_style_pad_all(img_show_win, 0, 0);
		lv_obj_set_style_pad_row(img_show_win, 0, 0);
		lv_obj_set_scroll_dir(img_show_win, LV_DIR_NONE);

		lv_obj_t* text_img = lv_img_create(img_show_win);
		lv_img_dsc_t* text_img_img = get_cur_book_img_dsc();// zb_img_res_load_one(NULL, TEXT_BOOK_ICON_IMG_TEXTBOOK5, IMG_FREE_TYPE_AUTO, NULL);
		lv_img_set_src(text_img, text_img_img);
		//lv_obj_set_size(text_img, text_img_img->header.w + TOP_BTN_BORDER * 2, text_img_img->header.h + TOP_BTN_BORDER * 2);
		lv_obj_set_size(text_img, text_img_img->header.w , text_img_img->header.h );
		lv_obj_align(text_img, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_style_pad_all(text_img, /*TOP_BTN_BORDER*/0, 0);
		lv_obj_set_style_radius(text_img, 8, 0);
		lv_obj_set_style_clip_corner(text_img, 8, LV_PART_MAIN);

		//lv_obj_set_style_border_color(text_img, lv_color_black(), 0);
		//lv_obj_set_style_border_width(text_img, 2, 0);
	}
	lv_obj_t* play_all_win = lv_obj_create(mainBg);
	lv_obj_set_size(play_all_win, LV_PCT(100), 50);
	if (_display_cover)
		lv_obj_align_to(play_all_win, img_show_win, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
	else
		lv_obj_align_to(play_all_win, top_title_win, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
	lv_obj_set_style_bg_color(play_all_win, main_colorBg, 0);
	lv_obj_set_style_border_width(play_all_win, 0, 0);
	lv_obj_set_style_pad_all(play_all_win, 0, 0);
	lv_obj_set_style_pad_left(play_all_win, 10, 0);
	lv_obj_set_style_pad_row(play_all_win, 0, 0);
	lv_obj_set_scroll_dir(play_all_win, LV_DIR_NONE);
	lv_obj_set_style_border_opa(play_all_win, LV_OPA_0, 0);
	lv_obj_set_style_bg_opa(play_all_win, LV_OPA_0, 0);

	lv_obj_t* play_all_btn = lv_img_create(play_all_win);
	lv_obj_align(play_all_btn, LV_ALIGN_LEFT_MID, 0, 0);
	lv_img_dsc_t* play_all_btn_img = zb_img_res_load_one(NULL, TEXT_BOOK_ICON_IMG_VIDEOPLAY, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(play_all_btn, play_all_btn_img);
	lv_obj_set_size(play_all_btn, play_all_btn_img->header.w + TOP_BTN_BORDER * 2, play_all_btn_img->header.h + TOP_BTN_BORDER * 2);
	lv_obj_set_style_pad_all(play_all_btn, TOP_BTN_BORDER, 0);
	lv_obj_set_style_radius(play_all_btn, 4, 0);
	lv_obj_add_flag(play_all_btn, LV_OBJ_FLAG_CLICKABLE);

	lv_obj_set_style_bg_color(play_all_btn, colorBg, 0);
	lv_obj_set_style_bg_opa(play_all_btn, LV_OPA_0, 0);
	lv_obj_set_style_bg_opa(play_all_btn, LV_OPA_100, LV_STATE_FOCUS_KEY);
	lv_obj_set_style_bg_opa(play_all_btn, LV_OPA_10, LV_STATE_PRESSED);
	lv_obj_set_style_border_color(play_all_btn, colorBorderFcs, 0);
	lv_obj_add_event_cb(play_all_btn, play_all_btn_clickCb, LV_EVENT_CLICKED, NULL);

	lv_obj_t* play_all_title = lv_label_create(play_all_win);
#ifndef C_PC_SIM
	lv_label_set_cjk_font(play_all_title, 18, FT_FONT_STYLE_NORMAL);
#endif
	lv_obj_align_to(play_all_title, play_all_btn, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	lv_label_set_text(play_all_title, "继续播放");
	lv_obj_set_style_text_color(play_all_title, lv_color_black(), 0);

	lv_obj_t* count_title = lv_label_create(play_all_win);
#ifndef C_PC_SIM
	lv_label_set_cjk_font(count_title, 18, FT_FONT_STYLE_NORMAL);
#endif
	lv_obj_align_to(count_title, play_all_title, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
	char count_str[16];
    snprintf(count_str, sizeof(count_str), "(%d)", play_list_book_info.cnt);
    lv_label_set_text(count_title, count_str);
	lv_obj_set_style_text_color(count_title, lv_color_hex(0x787F87), 0);

	g_tchMtrlContentListWin = lv_obj_create(mainBg);
	lv_obj_set_size(g_tchMtrlContentListWin, LV_PCT(100), _display_cover? SCREEN_HEIGHT - 260: SCREEN_HEIGHT-110);	//todo 260 为顶部标题栏高度 + 底部播放按钮高度
	lv_obj_align_to(g_tchMtrlContentListWin,play_all_win, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
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


	lv_img_dsc_t* selected_img = zb_img_res_load_one(NULL, TEXT_BOOK_ICON_IMG_VIDEOPLAYING, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_dsc_t* new_img = zb_img_res_load_one(NULL, TEXT_BOOK_ICON_IMG_VIDEONOPLAYING, IMG_FREE_TYPE_AUTO, NULL);


	//todo 这里的text_name需要在初始化时获取教材中的所有内容name，并动态分配数组，名字为text_name，这里是写死的
	//这里的10也要改为动态获取的count
	//for (int i = 0; i < g_tchMtrlCount; i++) {
	for (int i = 0; i < play_list_book_info.cnt; i++) {
		char * text = play_list_book_info.pTile[i];
		if (strchr(text, 0x05))
			text = strchr(text, 0x05) + 1;

		lv_obj_t* btn = lv_btn_create(g_tchMtrlContentListWin);
		lv_obj_set_size(btn, 300, 50);
		lv_obj_add_style(btn, &style_btn, 0);

		lv_obj_t* img = lv_img_create(btn);
		lv_img_dsc_t* img_img;
		if(strcmp(g_curSelectMtrlIndex,text)) img_img = new_img;
		else img_img = selected_img;
		lv_img_set_src(img, img_img);
		lv_obj_set_size(img, img_img->header.w, img_img->header.h);
		lv_obj_align(img, LV_ALIGN_LEFT_MID, 0, 0);

		lv_obj_t * label = lv_label_create(btn);
#ifndef C_PC_SIM
		lv_label_set_cjk_font(label, 18, FT_FONT_STYLE_NORMAL);
#endif
    	lv_label_set_text(label, text);
    	lv_obj_align_to(label, img, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
        lv_obj_add_event_cb(btn, _tchMtrl_content_item_btn_click_cb, LV_EVENT_CLICKED, text);
		lv_obj_set_style_bg_opa(btn, LV_OPA_50, LV_EVENT_PRESSING);
	}

	sys_setCurMainWinAndCb(mainBg, _tch_mtrl_scr_cb);
    g_tchMnuBgWin = mainBg;
}

void tch_mtrl_content_init(const char* name) {
    g_curSelectMtrl = name;
    tch_mtrl_content_win_init();
}