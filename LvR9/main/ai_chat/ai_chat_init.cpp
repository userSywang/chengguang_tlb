extern "C" {
#include "../data_parse/img_res.h"
#include "cpp_api_bridge.h"
}

#include "ai_chat.h"
#include <vector>


static int8_t g_ItemSel = 0;
int8_t g_gradeSel = 0;

lv_obj_t * g_chatMnuBgWin;
lv_obj_t * mainWin;
uint8_t g_chatCurWin = 0;
bool g_isInSubItem;

static const char * grades[] = {"1-3年级","4-6年级" ,"初中" , "高中"};

static lv_obj_t * mwin_Top_btn[4] = {0};

static lv_style_t style_btn;
static lv_style_t age_btn_selected_style;

static void _ai_chat_item_btn_click_cb(lv_event_t * e);

static const char * TAG = "apAiChat";

void ai_chat_clean_res(){
    g_chatMnuBgWin = nullptr;
    mainWin = nullptr;
    for(auto& btn : mwin_Top_btn){
        btn = nullptr;
    }
	if(g_defaultTopicRes){
		g_defaultTopicRes.reset();
		g_defaultTopicRes = nullptr;
	}
}

void ai_chat_switch_grade(int8_t grade){
	lv_obj_clean(mainWin);
	std::string grade_str = "";
	int sceneIndex = 0;
	std::vector<Topic>* Vtopics = nullptr;
	switch (grade) {
		case 0:
		case 1:
			grade_str = "初级";
			sceneIndex = 0;
			break;
		case 2:
			grade_str = "中级";
			sceneIndex = 1;
			break;
		case 3:
			grade_str = "高级";
			sceneIndex = 2;
			break;
	}
	if(g_defaultTopicRes == nullptr) return;
	for(auto& topics: g_defaultTopicRes.get()->data.topicList){
		if(topics.tag == grade_str){
			Vtopics = &topics.topics;
			break;
		}
	}
	for (int16_t itemNo = 0; itemNo < Vtopics->size(); itemNo++) {
		lv_obj_t* img = lv_img_create(mainWin);
	   	lv_img_dsc_t* returnBtn_img = zb_img_res_load_one(NULL, scene_icon[sceneIndex][itemNo], IMG_FREE_TYPE_AUTO, NULL);
	  	lv_img_set_src(img, returnBtn_img);
	   	lv_obj_set_size(img, 142,109);
	   	lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
	   	lv_obj_set_style_border_opa(img, LV_OPA_0, 0);
	   	lv_obj_set_style_img_recolor_opa(img, 15, LV_STATE_PRESSED);
	   	// lv_obj_set_grid_cell(img,
		//    	LV_GRID_ALIGN_CENTER, itemNo/3, 1,
		//    	LV_GRID_ALIGN_CENTER, itemNo%3, 1
	   	// );
		// lv_obj_set_style_margin_right(img, 20, 0); // 图片右边距
		// lv_obj_set_style_margin_bottom(img, 20, 0);// 图片下边距
	   	lv_obj_add_event_cb(img, _ai_chat_item_btn_click_cb, LV_EVENT_SHORT_CLICKED, reinterpret_cast<void*>(&Vtopics->at(itemNo))); 
   	}
}
void aiChat_createEscBtn(lv_obj_t * parent, lv_event_cb_t clickCb, int extra)
{
	printf("aichat_createEscBtn\n");
	lv_obj_t * img = lv_img_create(parent);
	const lv_img_dsc_t * pImgDsc = g_XXX_img_dsc_tbl + MMN_icn_esc - 1;
	lv_img_set_src(img, pImgDsc);
	lv_obj_set_size(img, pImgDsc->header.w + TOP_BTN_BORDER * 2, pImgDsc->header.h + TOP_BTN_BORDER * 2);
	lv_obj_set_pos(img, 16, 15);
	lv_obj_set_style_pad_all(img, TOP_BTN_BORDER, 0);
	lv_obj_set_style_radius(img, 4, 0);
	lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);

	lv_color_t colorBg = lv_color_hex(0xc0c0c0);
	lv_obj_set_style_bg_color(img, colorBg, 0);
	lv_obj_set_style_bg_opa(img, LV_OPA_0, 0);
	lv_obj_set_style_bg_opa(img, LV_OPA_100, LV_STATE_FOCUS_KEY);
	lv_obj_set_style_bg_opa(img, LV_OPA_10, LV_STATE_PRESSED);
	
	lv_color_t colorBorderFcs = lv_color_white();
	lv_obj_set_style_border_color(img, colorBorderFcs, 0);
	lv_obj_add_event_cb(img, clickCb, LV_EVENT_CLICKED, NULL);
}

static void _aiChat_lst_scr_esc_click_cb(lv_event_t * evt)
{
	zb_requestSwitchAp(C_ZB_AP_MAINMENU);
	return;
}

static void _ai_chat_top_btn_click_cb(lv_event_t * e){
    if (e->code!= LV_EVENT_CLICKED) return;
    lv_obj_t* obj = lv_event_get_target(e);
    int userData = (int)lv_obj_get_event_user_data(obj, _ai_chat_top_btn_click_cb);
    if(userData == g_gradeSel) return;
    printf("_ai_chat_top_btn_click_cb %d clcked\n",userData);
    g_gradeSel = userData;
    for(int i = 0; i < 4; i++){
        if(i == userData){
            lv_obj_remove_style(mwin_Top_btn[i], &style_btn, 0);
            lv_obj_add_style(mwin_Top_btn[i], &age_btn_selected_style, 0);
        }else{
            lv_obj_remove_style(mwin_Top_btn[i], &age_btn_selected_style, 0);
            lv_obj_add_style(mwin_Top_btn[i], &style_btn, 0);
        }
    }
	ai_chat_switch_grade(g_gradeSel);
	return;
}
static void ai_chat_switch_to_view(uint8_t grade, std::string classes ,std::string topic) {
	ai_chat_ap_win_deinit();
	ai_chat_view_init(grade, classes,topic);
}

static void _ai_chat_item_btn_click_cb(lv_event_t * e){
    if (e->code != LV_EVENT_SHORT_CLICKED)
		return;
    lv_obj_t* obj = lv_event_get_target(e);
	Topic* userData = static_cast<Topic*>(lv_obj_get_event_user_data(obj, _ai_chat_item_btn_click_cb)); 
	ai_chat_switch_to_view(g_gradeSel, userData->name , userData->enName);
}
void ai_chat_show_network_disconnect(){
	g_chatCurWin = MMENU_AI_CHAT_NETWORK_DISCON;
	g_isInSubItem = false;

	lv_obj_t * mainBg = lv_obj_create(lv_scr_act());
	lv_obj_set_size(mainBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainBg, 0, 0);
	lv_obj_set_style_pad_all(mainBg, 0, 0);
	lv_color_t colorBg = lv_color_hex(AICHAT_AP_BG_COLOR_HEX);
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

	aiChat_createEscBtn(top_title_win, _aiChat_lst_scr_esc_click_cb, 0);
	lv_obj_t * lblTitle = lv_label_create(top_title_win);
	lv_label_set_text(lblTitle, "无网络连接");
	lv_obj_align(lblTitle, LV_ALIGN_TOP_MID, 0, AICHAT_ESC_BTN_LOC_Y + TOP_BTN_BORDER + TOP_BTN_TOP_SPACE_H);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), 0);

	lv_obj_t* err_lb = lv_label_create(mainBg);
	lv_label_set_text(err_lb, "            该应用需要网络连接\n            当前处于无网络状态\n请前往设置->网络进行网络连接");
	lv_obj_center(err_lb);
	lv_obj_set_style_text_color(err_lb, lv_color_black(), 0);

	sys_setCurMainWinAndCb(mainBg,_ai_chat_scr_cb);
	g_chatMnuBgWin = mainBg;
}
int ai_chat_menu_scr_init(int parm) { // 入口菜单界面初始化 
	g_chatCurWin = MMENU_AI_CHAT;
	g_isInSubItem = false;

	lv_obj_t * mainBg = lv_obj_create(lv_scr_act());
	lv_obj_set_size(mainBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainBg, 0, 0);
	lv_obj_set_style_pad_all(mainBg, 0, 0);
	lv_color_t colorBg = lv_color_hex(AICHAT_AP_BG_COLOR_HEX);
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

	aiChat_createEscBtn(top_title_win, _aiChat_lst_scr_esc_click_cb, 0);
	lv_obj_t * lblTitle = lv_label_create(top_title_win);
	lv_label_set_text(lblTitle, "AI口语教练");
	lv_obj_align(lblTitle, LV_ALIGN_TOP_MID, 0, AICHAT_ESC_BTN_LOC_Y + TOP_BTN_BORDER + TOP_BTN_TOP_SPACE_H);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), 0);

    lv_obj_t * top_btn_win = lv_obj_create(mainBg);
    lv_obj_set_size(top_btn_win, LV_PCT(100), 60);
    lv_obj_align_to(top_btn_win, top_title_win, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_border_width(top_btn_win, 0, 0);
	lv_obj_set_style_pad_all(top_btn_win, 0, 0);
    //lv_obj_set_scroll_dir(top_btn_win, LV_DIR_NONE);
	lv_obj_set_scrollbar_mode(top_btn_win, LV_SCROLLBAR_MODE_OFF);  // 关闭滚动条

    lv_obj_set_flex_flow(top_btn_win, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(top_btn_win, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(top_btn_win, 20, 0);
	lv_obj_set_style_pad_right(top_btn_win, 20, 0);
	lv_obj_set_style_pad_column(top_btn_win, 18, 0);
	lv_obj_set_style_border_width(top_btn_win, 0, 0);
	lv_obj_set_style_border_opa(top_btn_win, LV_OPA_0, 0);
	lv_obj_set_style_bg_opa(top_btn_win, LV_OPA_0, 0);

	static bool isFirst = true;
    if(isFirst){
        isFirst = false;
	lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, lv_color_hex(0xF5F5F5)); // 未选中时浅灰色背景
    lv_style_set_bg_opa(&style_btn, LV_OPA_100);
    lv_style_set_text_color(&style_btn, lv_color_hex(0x000000)); // 黑色字体 #333333
    lv_style_set_radius(&style_btn, 19);                         // 16px圆角
    lv_style_set_pad_hor(&style_btn, 20);                        // 水平内边距
    lv_style_set_pad_ver(&style_btn, 6);
    lv_style_set_border_width(&style_btn, 0); // 无边框
    lv_style_set_shadow_width(&style_btn, 0); // 无阴影

    lv_style_init(&age_btn_selected_style);
    lv_style_set_bg_color(&age_btn_selected_style, lv_color_hex(0xE6F0FF));     // 浅蓝色背景 #E6F0FF
    lv_style_set_text_color(&age_btn_selected_style, lv_color_hex(0x1A5FB4));   // 深蓝字体 #1A5FB4
    lv_style_set_border_width(&age_btn_selected_style, 0); 
    lv_style_set_radius(&age_btn_selected_style, 19);                         // 16px圆角
    lv_style_set_pad_hor(&age_btn_selected_style, 20);                        // 水平内边距
    lv_style_set_pad_ver(&age_btn_selected_style, 6);
    lv_style_set_shadow_width(&age_btn_selected_style, 0); // 无阴影
    }
    

    mwin_Top_btn[0] = lv_btn_create(top_btn_win);
    lv_obj_set_size(mwin_Top_btn[0], 90, 35);
    lv_obj_add_event_cb(mwin_Top_btn[0], _ai_chat_top_btn_click_cb, LV_EVENT_CLICKED, (void*)0); // Add event callback to the button to handle click events and selecte
    lv_obj_t * label1 = lv_label_create(mwin_Top_btn[0]);
    lv_label_set_text(label1, grades[0]);
    lv_obj_center(label1);

    mwin_Top_btn[1] = lv_btn_create(top_btn_win);
    lv_obj_set_size(mwin_Top_btn[1], 90, 35);
    lv_obj_add_event_cb(mwin_Top_btn[1], _ai_chat_top_btn_click_cb, LV_EVENT_CLICKED, (void*)1);
    lv_obj_t * label2 = lv_label_create(mwin_Top_btn[1]);
    lv_label_set_text(label2, grades[1]);
    lv_obj_center(label2);

    mwin_Top_btn[2] = lv_btn_create(top_btn_win);
    lv_obj_set_size(mwin_Top_btn[2], 60, 35);
    lv_obj_add_event_cb(mwin_Top_btn[2], _ai_chat_top_btn_click_cb, LV_EVENT_CLICKED, (void*)2);
    lv_obj_t * label3 = lv_label_create(mwin_Top_btn[2]);
    lv_label_set_text(label3, grades[2]);
    lv_obj_center(label3);

	mwin_Top_btn[3] = lv_btn_create(top_btn_win);
    lv_obj_set_size(mwin_Top_btn[3], 60, 35);
    lv_obj_add_event_cb(mwin_Top_btn[3], _ai_chat_top_btn_click_cb, LV_EVENT_CLICKED, (void*)3);
    lv_obj_t * label4 = lv_label_create(mwin_Top_btn[3]);
    lv_label_set_text(label4, grades[3]);
    lv_obj_center(label4);

	for(int i = 0; i < 4; i++){
		if(i == g_gradeSel){
            lv_obj_add_style(mwin_Top_btn[i], &age_btn_selected_style, 0);
		}else{
            lv_obj_add_style(mwin_Top_btn[i], &style_btn, 0);
		}
	}

	//---------------
	mainWin = lv_obj_create(mainBg);
    lv_obj_remove_style_all(mainWin);  // Remove all styles to ensure no default style is applied
	lv_obj_set_size(mainWin, LV_PCT(100), SCREEN_HEIGHT - 120);
	lv_obj_align(mainWin, LV_ALIGN_BOTTOM_LEFT, 0, 0);
	
	/* 新增Flex布局设置 */
	lv_obj_set_flex_flow(mainWin, LV_FLEX_FLOW_ROW_WRAP); // 从左到右排列，自动换行
	lv_obj_set_flex_align(mainWin, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	lv_obj_set_style_pad_column(mainWin, 12, 0);  // 列间距
	lv_obj_set_style_pad_row(mainWin, 12, 0);     // 行间距
	lv_obj_set_style_border_width(mainWin, 0, 0);
	lv_obj_set_style_pad_all(mainWin, 12, 0);     // 容器内边距
	lv_obj_set_style_pad_top(mainWin, 0, 0);     // 容器内边距
	lv_obj_set_scrollbar_mode(mainWin, LV_SCROLLBAR_MODE_OFF);  // 关闭滚动条
	lv_obj_set_scroll_dir(mainWin, LV_DIR_VER);

	ai_chat_switch_grade(g_gradeSel);
	
	sys_setCurMainWinAndCb(mainBg,_ai_chat_scr_cb);
	g_chatMnuBgWin = mainBg;
	printf("ai_chat_menu_scr_init end\n");
	return 0;
}

int ai_chat_ap_win_deinit(void) {  // 入口菜单界面销毁 
	if (g_chatMnuBgWin) {
		lv_obj_del(g_chatMnuBgWin);  g_chatMnuBgWin = NULL;
	}
	return 0;
}

extern "C" int listenai_child_chat_init(void){
	if(get_is_net_connected() && ai_chat_get_default_topic()){
		if(g_defaultTopicRes.get()->code=="0" && g_defaultTopicRes.get()->msg=="SUCCESS"){
			printf("get topic success\n");
		}
		ai_chat_menu_scr_init(0);
	}else{
		ai_chat_show_network_disconnect();
	}
	
	return 0;
}


static int _ai_chat_choice_proc()
{
	LV_LOG("%s [%d] g_ItemSel:%d \n", __FUNCTION__, __LINE__, g_ItemSel);
	g_isInSubItem = true;
	switch (g_ItemSel + 1) 
	{
		default:
			ESP_LOGW(TAG, "invalid choice sub %d", g_ItemSel);
			g_isInSubItem = false;
			break;
		break;
	}

	return (1);
}

static void _ai_chat_curScr_del(void) {
	if (!g_isInSubItem) {
		ai_chat_ap_win_deinit();
		ai_chat_clean_res();
	}
	switch (g_ItemSel + 1) 
	{
		default:
	
		break;
	}
}

extern "C" int listenai_child_chat_handler(lv_event_code_t msg, int param){
	LV_LOG("%s,[%d]\r\n ", __FUNCTION__,__LINE__);
	switch ((int)msg) 
	{
	case C_EVENT_FREE_AP_RES:
		LV_LOG("%s,[%d] , g_ItemSel: %d \r\n ", __FUNCTION__,__LINE__, g_ItemSel);
		_ai_chat_curScr_del();
		break;

	case C_EVENT_EX_CMD: {
		break;
	}
	case C_EVENT_DOES_BLOCK_ALARM:

	case C_EVENT_DOES_BLOCK_SYS_POWER_KEY:
		return 0;

	default:
		break;
	}
	return 0;
}

void _ai_chat_scr_cb(lv_event_t* evt) {
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
			switch (g_chatCurWin)
			{
			case MMENU_AI_CHAT:
				ai_chat_ap_win_deinit();
				zb_requestSwitchAp(C_ZB_AP_MAINMENU);
				break;
			case MMENU_AI_CHAT_VIEW:
				ai_chat_view_del(nullptr);
				break;
			case MMENU_AI_CHAT_NETWORK_DISCON:
				ai_chat_ap_win_deinit();
				zb_requestSwitchAp(C_ZB_AP_MAINMENU);
				break;
			default:
				break;
			}
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