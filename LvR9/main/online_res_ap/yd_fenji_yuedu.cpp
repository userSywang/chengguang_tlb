extern "C" {
    #include "../data_parse/img_res.h"
	#include "../data_parse/img_std_res.h"
    #include "cpp_api_bridge.h"
    #include "../mainmenu/mainmenu_base.h"
	#include "../data_support/yd_book_select.h"
	#include "../player/player.h"
}
#include "yd_fenji_yuedu.h"


#include "../online_tool/http_get_online_res.h" 
#include "../online_tool/global_http_tool.h"
#include <vector>
#include <iostream>

//global
static YD_FenjiYuedu* g_fenji_ap = nullptr;

static void AP_MainWin_scr_cb(lv_event_t* evt);
static void AP_esc_click_cb(lv_event_t * evt);

//main win
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
int YD_FenjiYuedu::AP_MainWin_init(){
    mwin_CurWin = YD_FENJI_MAINWIN;
    mwin_IsInSubItem = false;
    
    lv_obj_t* mainBg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mainBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainBg, 0, 0);
	lv_obj_set_style_pad_all(mainBg, 0, 0);
	lv_color_t colorBg = lv_color_hex(AP_BG_COLOR_HEX);
	lv_obj_set_style_bg_color(mainBg, colorBg, 0);
	lv_obj_set_scroll_dir(mainBg, LV_DIR_NONE);

    lv_obj_t * top_title_win = lv_obj_create(mainBg);
    lv_obj_set_size(top_title_win, LV_PCT(100), 60);
    lv_obj_align(top_title_win, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(top_title_win, colorBg, 0);
    lv_obj_set_style_border_width(top_title_win, 0, 0);
	lv_obj_set_style_pad_all(top_title_win, 0, 0);
	lv_obj_set_style_pad_row(top_title_win, 0, 0);
	lv_obj_set_style_pad_column(top_title_win, 0, 0);
	lv_obj_set_scroll_dir(top_title_win, LV_DIR_NONE);

	Ap_createEscBtn(top_title_win, AP_esc_click_cb, 0);
	lv_obj_t * lblTitle = lv_label_create(top_title_win);
	lv_label_set_text(lblTitle, "大猫分级");
	lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
	lv_obj_align(lblTitle, LV_ALIGN_CENTER, 0,0);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), 0);

    

    // for (size_t i = 0; i < g_resNodeList.get()->size(); i++){
	// 	OnlineResNode& node = (*g_resNodeList.get())[i];
	// 	lv_obj_t *item = lv_obj_create(lst_win);
	// 	lv_obj_set_size(item, (SCREEN_WIDTH - 20) / 2 - 10, 225);			// 宽度自适应容器，高度固定160px
	// 	lv_obj_set_style_radius(item, 18, 0);		// 圆角
	// 	lv_obj_set_style_border_width(item, 0, 0);
	// 	lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
	// 	lv_obj_set_scroll_dir(item, LV_DIR_NONE);
	// 	lv_obj_set_style_bg_color(item, lv_color_hex(0xd3d3d3), LV_STATE_PRESSED);

	// 	lv_obj_t *img = lv_img_create(item);
	// 	lv_obj_set_size(img, 140, 162);
	// 	lv_obj_set_style_radius(img, 18, LV_PART_MAIN);
	// 	lv_obj_set_style_clip_corner(img, 18, LV_PART_MAIN);
	// 	lv_img_dsc_t* img_Src = nullptr;
	// 	if(node.isChenGuang){
	// 		if(PlayApIsTeachMtl(mwin_ItemSel)){
	// 			img_Src = get_tch_img_by_path(node.coverImg);
	// 		}else{
	// 			img_Src = get_offline_img_by_ap_index(mwin_ItemSel,i);
	// 		}
	// 	}else{
	// 		img_Src = get_online_img_by_url(node.coverImg);
	// 	}
	// 	lv_img_set_src(img,img_Src);
	// 	lv_obj_set_style_img_opa(img, LV_OPA_70, LV_STATE_PRESSED);

	// 	lv_coord_t max_width = 140;  // 按钮宽度
	// 	lv_coord_t max_height = 80;   // 两行高度（45按钮高度 + 35间距）
	// 	lv_point_t text_size;
	// 	const char* bookName = node.name.c_str();
		
	// 	lv_obj_t *label = lv_label_create(item);
	// 	lv_label_set_cjk_font(label, 18, FT_FONT_STYLE_NORMAL);
	// 	lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);  // 文本水平居中
	// 	lv_obj_set_size(label, 140, 45);
	// 	size_t max_chars = 15;
    //    	size_t char_count = 0;
	// 	char name_buf[500] = { 0 };
    //    	const char *p = bookName;
    //     while(*p && char_count < max_chars) {
    //         if((*p & 0xC0) != 0x80) char_count++;
    //         p++;
    //     }

    //     if(char_count >= max_chars) {
    //         size_t pos = 0;
    //         size_t count = 0;
    //         const char *ptr = bookName;
            
    //         while(*ptr && count < max_chars-3) {
    //             if((*ptr & 0xC0) != 0x80) count++;
    //         	pos++;
    //         	ptr = bookName + pos;
    //         }
            
    //         strncpy(name_buf, bookName, pos);
    //         strcat(name_buf, "...");
	// 		bookName = name_buf;
    //     }
	// 	lv_label_set_text(label, bookName);
	// 	lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
	// 	lv_obj_align_to(label,img, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);
	// 	if(node.isChenGuang){
	// 		if(node.isDownlaoded){
	// 			lv_obj_add_event_cb(item, _item_offline_click_handler, LV_EVENT_CLICKED,(void*)i);
	// 		}else{
	// 			lv_obj_t* down_img = lv_img_create(img);
	// 			lv_img_set_src(down_img, &download_img);
	// 			lv_obj_set_size(down_img, download_img.header.w, download_img.header.h);
	// 			lv_obj_align(down_img, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
	// 			lv_obj_add_event_cb(item, _item_download_click_handler, LV_EVENT_CLICKED, (void*)i);
	// 		}
	// 	}else{
	// 		lv_obj_add_event_cb(item, _item_online_click_handler, LV_EVENT_CLICKED,(void*)i);
	// 	}
	// }

    sys_setCurMainWinAndCb(mainBg,AP_MainWin_scr_cb);
	mwin_BgWin = mainBg;
	printf("online_play_ap_init end\n");
	return 0;
}

//content win
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/


int YD_FenjiYuedu::AP_ContentWin_init(){
	mwin_CurWin = YD_FENJI_CONTENTWIN;
    mwin_IsInSubItem = true;

	lv_color_t main_colorBg = lv_color_hex(0xffffff);
	lv_obj_t* mainBg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mainBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainBg, 0, 0);
	lv_obj_set_style_pad_all(mainBg, 0, 0);
	lv_color_t colorBg = lv_color_hex(AP_BG_COLOR_HEX);
	lv_obj_set_style_bg_color(mainBg, colorBg, 0);
	lv_obj_set_scroll_dir(mainBg, LV_DIR_NONE);

    lv_obj_t * top_title_win = lv_obj_create(mainBg);
    lv_obj_set_size(top_title_win, LV_PCT(100), 60);
    lv_obj_align(top_title_win, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(top_title_win, colorBg, 0);
    lv_obj_set_style_border_width(top_title_win, 0, 0);
	lv_obj_set_style_pad_all(top_title_win, 0, 0);
	lv_obj_set_style_pad_row(top_title_win, 0, 0);
	lv_obj_set_style_pad_column(top_title_win, 0, 0);
	lv_obj_set_scroll_dir(top_title_win, LV_DIR_NONE);

	// char name_buf[500] = { 0 };
	// char* bookName = (char*)m_name.c_str();
	// size_t max_chars = 15;
    // size_t char_count = 0;
    // const char *p = bookName;
    // while(*p && char_count < max_chars) {
    //     if((*p & 0xC0) != 0x80) char_count++;
    //     p++;
    // }

    // if(char_count >= max_chars) {
    //     size_t pos = 0;
    //     size_t count = 0;
    //     const char *ptr = bookName;
            
    //     while(*ptr && count < max_chars-3) {
    //         if((*ptr & 0xC0) != 0x80) count++;
    //         pos++;
    //         ptr = bookName + pos;
    //     }
            
    //     strncpy(name_buf, bookName, pos);
    //     strcat(name_buf, "...");
	// 	bookName = name_buf;
    // }

	Ap_createEscBtn(top_title_win, AP_esc_click_cb, 0);
	lv_obj_t * lblTitle = lv_label_create(top_title_win);
	lv_label_set_text(lblTitle, "");
	lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
	lv_obj_align(lblTitle, LV_ALIGN_CENTER, 0,0);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), 0);

	

	sys_setCurMainWinAndCb(mainBg,AP_MainWin_scr_cb);
	mwin_BgWin = mainBg;
	return 0;
}


//disconnect win
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

int YD_FenjiYuedu::AP_DisWin_init(){
	std::cout<<"AP_DisWin_init start"<<std::endl;
	mwin_CurWin = YD_FENJI_DISWIN;
	mwin_IsInSubItem = false;

	lv_obj_t* mainBg = lv_obj_create(lv_scr_act());
	lv_obj_set_size(mainBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainBg, 0, 0);
	lv_obj_set_style_pad_all(mainBg, 0, 0);
	lv_color_t colorBg = lv_color_hex(AP_BG_COLOR_HEX);
	lv_obj_set_style_bg_color(mainBg, colorBg, 0);
	lv_obj_set_scroll_dir(mainBg, LV_DIR_NONE);

    lv_obj_t * top_title_win = lv_obj_create(mainBg);
    lv_obj_set_size(top_title_win, LV_PCT(100), 60);
    lv_obj_align(top_title_win, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(top_title_win, colorBg, 0);
    lv_obj_set_style_border_width(top_title_win, 0, 0);
	lv_obj_set_style_pad_all(top_title_win, 0, 0);
	lv_obj_set_style_pad_row(top_title_win, 0, 0);
	lv_obj_set_style_pad_column(top_title_win, 0, 0);
	lv_obj_set_scroll_dir(top_title_win, LV_DIR_NONE);

	Ap_createEscBtn(top_title_win, AP_esc_click_cb, 0);
	lv_obj_t * lblTitle = lv_label_create(top_title_win);
	lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
	lv_label_set_text(lblTitle, "无网络连接");
	lv_obj_align(lblTitle, LV_ALIGN_TOP_MID, 0, AICHAT_ESC_BTN_LOC_Y + TOP_BTN_BORDER + TOP_BTN_TOP_SPACE_H);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), 0);

	lv_obj_t* err_lb = lv_label_create(mainBg);
	lv_label_set_text(err_lb, "            该应用需要网络连接\n请前往设置->网络进行网络连接");
	lv_obj_center(err_lb);
	lv_label_set_cjk_font(err_lb, 18, FT_FONT_STYLE_NORMAL);
	lv_obj_set_style_text_color(err_lb, lv_color_black(), 0);
	std::cout<<"AP_DisWin_init end"<<std::endl;

	sys_setCurMainWinAndCb(mainBg,AP_MainWin_scr_cb);
	mwin_BgWin = mainBg;
	return 0;
}


void YD_FenjiYuedu::Ap_MainWin_clean_res(){
}
static void AP_esc_click_cb(lv_event_t * evt){
    switch (g_fenji_ap->get_cur_win())
	{
		case YD_FENJI_DISWIN:
		case YD_FENJI_MAINWIN:
			zb_requestSwitchAp(C_ZB_AP_MAINMENU);
			break;
		case YD_FENJI_CONTENTWIN:
			g_fenji_ap->Ap_MainWin_del();
			g_fenji_ap->AP_MainWin_init();
			break;
		default: 
			break;
	}
    return;
}
int YD_FenjiYuedu::AP_MainWin_deinit(){
    Ap_MainWin_del();
    Ap_MainWin_clean_res();
    return 0;
}
void YD_FenjiYuedu::Ap_MainWin_del(){
	if (mwin_BgWin) {
		lv_obj_del(mwin_BgWin);  mwin_BgWin = NULL;
	}
}
int YD_FenjiYuedu::AP_MainWin_handler(lv_event_code_t msg, int param) {
    LV_LOG("%s,[%d]\r\n ", __FUNCTION__,__LINE__);
	switch ((int)msg) 
	{
		case C_EVENT_FREE_AP_RES:
			LV_LOG("%s,[%d] , mwin_CurWin: %d \r\n ", __FUNCTION__,__LINE__, mwin_CurWin);
			if(g_fenji_ap != nullptr){
				delete g_fenji_ap;
				g_fenji_ap = nullptr;
			}
			break;
		case C_EVENT_EX_CMD: {
			if (param == PLY_CMD_PLY_LST_RET) {
				switch (mwin_CurWin)
				{
					case YD_FENJI_CONTENTWIN:
						
						break;
					default:
						break;
				}
			}
			break;
		}
		default:
			break;
	}
	return 0;
}
static void AP_MainWin_scr_cb(lv_event_t* evt) {
	lv_obj_t* obj = lv_event_get_target(evt);
	int keyVal = 0;
	lv_event_code_t evtCode = evt->code;

	if (LV_EVENT_KEY_STAGE == evt->code) {
		keyVal = lv_event_get_key(evt);
		if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_REPEAT) {
			keyVal &= ~LV_EX_FLAG_KEY_MASK;
			evtCode = LV_EVENT_KEY;
		}
	}

	switch (evtCode) {
	case LV_EVENT_KEY_STAGE:
		if ((LV_EX_FLAG_KEY_MASK & keyVal) != LV_EX_FLAG_KEY_SHORT)
			break;
		keyVal &= 0xff;
		switch (keyVal) {
		case LV_KEY_ESC:
			AP_esc_click_cb(nullptr);
			break;
		case LV_KEY_ENTER:

			break;
		default:
			break;
		}
		break;
	case LV_EVENT_DELETE:
		break;
	default:
		break;
	}
}

int YD_FenjiYuedu_ap_init(){
    if(g_fenji_ap){
        delete g_fenji_ap;
        g_fenji_ap = nullptr;
    }
	g_fenji_ap = new YD_FenjiYuedu();
	if(get_is_net_connected())
        async_get_online_dir_list(99,[](){
            if(g_resNodeList == nullptr){
                g_fenji_ap->AP_DisWin_init();
            }else{
                g_fenji_ap->AP_MainWin_init();
            }
        });
	else 
        g_fenji_ap->AP_DisWin_init();
	return 0;
}

// extern "C" int online_play_fenjiyuedu_ap_init(void){
//     return YD_FenjiYuedu_ap_init();
// }
// extern "C" int online_play_fenjiyuedu_ap_handler(lv_event_code_t msg, int param){
//     return g_fenji_ap->AP_MainWin_handler(msg,param);
// }