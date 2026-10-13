extern "C" {
#include "cpp_api_bridge.h"
}
#include "ai_chat.h"

static uint8_t g_selGrade = 0;
static std::string g_selClass = "";

// static const char * g_aiChatClass[6] = {
// 	"问候致意","兴趣爱好","多样天气","互相介绍","一年四季","缤纷多彩"
// };

// static const char* exp_text_eng = 
// 	"Hello there! It's nice to meet you. Can you repeat afther me? Sunny?";
// static const char* exp_text_cn =
// 	"你好！很高兴认识你。你能跟我重复一遍吗？Sunny？";
// static const char* exp_text_tip =
// 	"Yes,XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

//ai chat variables
static std::string g_text_eng = "";
static std::string g_text_cn = "";
static std::string g_text_tip = "";
static std::string g_text_mine = "";


//lvgl variables
static lv_obj_t* g_mainWin = NULL;
static lv_obj_t* g_tipItem = NULL;
static lv_obj_t* g_speakItem = NULL;
static lv_obj_t* g_headIcon = NULL;
static lv_obj_t* g_headIconmy = NULL;
static lv_obj_t* g_bubbles_blue = NULL;
static lv_obj_t* g_bubbles_yellow = NULL;
static lv_obj_t* g_bubbles_my = NULL;
static lv_obj_t* overlay = NULL;
static lv_obj_t* g_pressItem = NULL;
static lv_obj_t* g_closeItem = NULL;
static lv_area_t g_pressItemArea;
static lv_area_t g_closeItemArea;
static lv_anim_t* anim = NULL;

static uint32_t now_index = 0;

//other variables
static bool isTranEnable = false;
static bool isTipItemSelected = false;
static bool isChatOver = false;


static void ai_chat_show_chat(const char* text);
static void ai_chat_show_speak_view();
static void ai_chat_hidden_speak_view();
static void ai_chat_show_waiting_status();

static void ai_chat_view_scroll_to_bottom(){
    if (g_mainWin == NULL) return;
    lv_coord_t content_height = lv_obj_get_scroll_y(g_mainWin) 
                              + lv_obj_get_scroll_top(g_mainWin) 
                              + lv_obj_get_scroll_bottom(g_mainWin);
    lv_obj_scroll_to_y(g_mainWin, content_height, LV_ANIM_ON);
    lv_obj_update_layout(g_mainWin);
}

static void ai_chat_yellow_bubble_draw(const char* text){
	g_bubbles_yellow = lv_obj_create(g_mainWin);
	lv_obj_align_to(g_bubbles_yellow, g_bubbles_blue, LV_ALIGN_OUT_BOTTOM_LEFT, -30, 10);
	lv_obj_set_style_bg_color(g_bubbles_yellow, lv_color_hex(0xf8f1b3), 0);//rgb(248, 241, 179) #f8f1b3
	lv_obj_set_style_radius(g_bubbles_yellow, 12, 0);
	lv_obj_set_style_border_width(g_bubbles_yellow, 0, 0);
	int text_width = 0;
	int text_height = 0;
	lv_obj_t* text_lb = lv_label_create(g_bubbles_yellow);
	lv_label_set_text(text_lb, text);
	lv_obj_align(text_lb, LV_ALIGN_TOP_LEFT, 10,25);
	lv_obj_add_flag(g_bubbles_yellow, LV_OBJ_FLAG_HIDDEN);
	lv_refr_now(NULL);
	if(lv_obj_get_width(text_lb) > 260) {
		lv_obj_set_width(text_lb,260);
		lv_label_set_long_mode(text_lb, LV_LABEL_LONG_WRAP);
		lv_label_set_text(text_lb, text);
	}
	lv_refr_now(NULL);
	lv_obj_clear_flag(g_bubbles_yellow, LV_OBJ_FLAG_HIDDEN);
	text_width = lv_obj_get_width(text_lb);
	text_height = lv_obj_get_height(text_lb);
	lv_obj_set_size(g_bubbles_yellow, text_width + 20, text_height+50);
	lv_refr_now(NULL);
}
static void ai_chat_yellow_bubble_show(){
	lv_img_dsc_t* tip_yellow_img = zb_img_res_load_one(NULL, AI_CHAT_IMG_TIP_YELLOW, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(g_tipItem, tip_yellow_img);
	lv_obj_set_size(g_tipItem, tip_yellow_img->header.w, tip_yellow_img->header.h);
	ai_chat_yellow_bubble_draw(g_text_tip.c_str());
	ai_chat_view_scroll_to_bottom();
}
static void ai_chat_yellow_bubble_hidden(){
	lv_img_dsc_t *tip_blue_img = zb_img_res_load_one(NULL, AI_CHAT_IMG_TIP_BLUE, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(g_tipItem, tip_blue_img);
	lv_obj_set_size(g_tipItem, tip_blue_img->header.w, tip_blue_img->header.h);
	if(g_bubbles_yellow){
		lv_obj_del(g_bubbles_yellow);
		g_bubbles_yellow = NULL;
	}
	ai_chat_view_scroll_to_bottom();
}


static void ai_chat_show_my_bubble(const char* text){
	lv_obj_set_height(g_bubbles_blue,lv_obj_get_height(g_bubbles_blue)-50);
	lv_refr_now(NULL);
	g_headIconmy = lv_img_create(g_mainWin);
	lv_img_dsc_t *my_head_img = zb_img_res_load_one(NULL, AI_CHAT_IMG_MYSELF, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(g_headIconmy, my_head_img);
	lv_obj_set_size(g_headIconmy, my_head_img->header.w, my_head_img->header.h);
	lv_obj_align(g_headIconmy, LV_ALIGN_TOP_RIGHT, -10, now_index + lv_obj_get_height(g_bubbles_blue) + 30);

	int text_width = 0;
	int text_height = 0;

	g_bubbles_my = lv_obj_create(g_mainWin);
	lv_obj_set_size(g_bubbles_my, 200, 200);
	lv_obj_align_to(g_bubbles_my, g_headIconmy, LV_ALIGN_TOP_RIGHT, -20, -15);
	lv_obj_set_style_bg_color(g_bubbles_my, lv_color_hex(0xEEEEEE), 0);//rgb(248, 241, 179) #f8f1b3
	lv_obj_set_style_radius(g_bubbles_my, 12, 0);
	lv_obj_set_style_border_width(g_bubbles_my, 0, 0);

	lv_obj_t* text_lb = lv_label_create(g_bubbles_my);
	lv_label_set_text(text_lb, text);
	lv_obj_align(text_lb, LV_ALIGN_TOP_LEFT, 10,10);
	lv_obj_add_flag(g_bubbles_my, LV_OBJ_FLAG_HIDDEN);
	lv_refr_now(NULL);
	if(lv_obj_get_width(text_lb) > 250) {
		lv_obj_set_width(text_lb,250);
		lv_label_set_long_mode(text_lb, LV_LABEL_LONG_WRAP);
		lv_label_set_text(text_lb, text);
	}
	lv_refr_now(NULL);
	lv_obj_clear_flag(g_bubbles_my, LV_OBJ_FLAG_HIDDEN);
	text_width = lv_obj_get_width(text_lb);
	text_height = lv_obj_get_height(text_lb);
	lv_obj_set_size(g_bubbles_my, text_width+20, text_height+20);
	lv_obj_align_to(g_bubbles_my, g_headIconmy, LV_ALIGN_TOP_RIGHT, -20, -15);
	ai_chat_view_scroll_to_bottom();
}

static void ai_chat_tip_clickCb(lv_event_t* evt){
	if(isChatOver) return;
	if(g_text_eng == "") return;
	static std::string last_tip_text = "";
	if(last_tip_text != g_text_eng){
		ai_chat_generate_recommendation(
			g_generateTopicRes.get()->data.taskId,
			g_generateTopicRes.get()->data.scene,
			std::to_string(g_selGrade),
			g_chatHistory,
			"1",
			std::vector<std::string>{std::to_string(g_chatHistory.size())});
		if(g_generateRecommendationRes.get()->code == "0"){
			g_text_tip = g_generateRecommendationRes.get()->data.resultArr[0].result[0];
			last_tip_text = g_text_eng;
		}else{
			printf("recommendation error:%s\n",g_generateRecommendationRes.get()->code.c_str());
			return;
		}
	}
	if(isTipItemSelected) {
		ai_chat_yellow_bubble_hidden();
		isTipItemSelected = false;
	}else{
		ai_chat_yellow_bubble_show();
		isTipItemSelected = true;
	}
}

static void ai_chat_speak_out_clickCb(lv_event_t* evt){
	if(isChatOver) return;
	if(isTranEnable){
		ai_chat_tts(g_text_cn, "zh");
	}else{
		ai_chat_tts(g_text_eng, "en");
	}
	if(g_TtsRes.get()->code == "0"){
		play_tts_audio();
	}else{
		printf("tts error:%s\n",g_TtsRes.get()->code.c_str());
	}
}

static void ai_chat_tran_clickCb(lv_event_t* evt){
	if(isChatOver) return;
	if(g_text_eng == "") return;
	static std::string last_tran_text = "";
	if(last_tran_text != g_text_eng){
		ai_chat_enzh_translation(g_text_eng);
		if(g_enzhTranslationRes.get()->code == "0"){
			g_text_cn = g_enzhTranslationRes.get()->translation[0];
			last_tran_text = g_text_eng;
		}else{
			printf("translation error:%s\n",g_enzhTranslationRes.get()->code.c_str());
			return;
		}
		
	}
	if(isTranEnable) {
		isTranEnable = false;
		ai_chat_show_chat(g_text_eng.c_str());
	}else{
		isTranEnable = true;
		ai_chat_show_chat(g_text_cn.c_str());
	}
}

static void anim_img_angle_cb(void* var, int32_t value) {
	lv_img_set_angle((lv_obj_t*)var, (int16_t)value); // 显式类型转换
}
static void ai_chat_show_chat(const char* text) {
	if (anim) {
		lv_anim_del(anim, (lv_anim_exec_xcb_t)anim_img_angle_cb);
		lv_mem_free(anim);
		anim = NULL;
	}
	lv_obj_clean(g_bubbles_blue);

	int text_width = 0;
	int text_height = 0;
	lv_obj_t* text_lb = lv_label_create(g_bubbles_blue);
	lv_label_set_text(text_lb, text);
	lv_obj_align(text_lb, LV_ALIGN_TOP_LEFT, 10,10);
	lv_obj_add_flag(g_bubbles_blue, LV_OBJ_FLAG_HIDDEN);
	lv_refr_now(NULL);
	if(lv_obj_get_width(text_lb) > 230) {
		lv_obj_set_width(text_lb,230);
		lv_label_set_long_mode(text_lb, LV_LABEL_LONG_WRAP);
		lv_label_set_text(text_lb, text);
	}
	lv_refr_now(NULL);
	lv_obj_clear_flag(g_bubbles_blue, LV_OBJ_FLAG_HIDDEN);
	text_width = lv_obj_get_width(text_lb);
	text_height = lv_obj_get_height(text_lb);

	lv_obj_t* line = lv_obj_create(g_bubbles_blue);
	lv_obj_set_size(line, text_width, 2); 
	lv_obj_align_to(line,text_lb, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
	static lv_style_t style_line;
	static bool isFirst = true;
	if (isFirst) {
		isFirst = false;
		lv_style_init(&style_line);
		lv_style_set_bg_color(&style_line, lv_color_hex(0xA0A0A0));
		lv_style_set_bg_opa(&style_line, LV_OPA_50);
		lv_style_set_radius(&style_line, 1);
		lv_obj_add_style(line, &style_line, 0);
	}

	lv_obj_t* tran_btn = lv_img_create(g_bubbles_blue);
	lv_img_dsc_t* untran_img = zb_img_res_load_one(NULL, AI_CHAT_IMG_UNTRAN, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_dsc_t* tran_img = zb_img_res_load_one(NULL, AI_CHAT_IMG_TRAN, IMG_FREE_TYPE_AUTO, NULL);
	if(isTranEnable) lv_img_set_src(tran_btn, tran_img);
	else lv_img_set_src(tran_btn, untran_img);
	lv_obj_set_size(tran_btn, tran_img->header.w, tran_img->header.h);
	lv_obj_align_to(tran_btn, line ,LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
	lv_obj_t* speak_out_btn = lv_img_create(g_bubbles_blue);
	lv_img_dsc_t* speak_out_img = zb_img_res_load_one(NULL, AI_CHAT_IMG_SPEAK_OUT, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(speak_out_btn, speak_out_img);
	lv_obj_set_size(speak_out_btn, speak_out_img->header.w, speak_out_img->header.h);
	lv_obj_align_to(speak_out_btn, tran_btn, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
	lv_obj_set_size(g_bubbles_blue, text_width+20, text_height+65);

	lv_obj_add_flag(tran_btn, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_flag(speak_out_btn, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(tran_btn, ai_chat_tran_clickCb, LV_EVENT_CLICKED, NULL);
	lv_obj_add_event_cb(speak_out_btn, ai_chat_speak_out_clickCb, LV_EVENT_CLICKED, NULL);
	lv_refr_now(NULL);

	if(isChatOver){
		lv_obj_set_height(g_bubbles_blue,lv_obj_get_height(g_bubbles_blue)-50);
	}


	if(isTipItemSelected) {
		if(g_bubbles_yellow){
			lv_obj_del(g_bubbles_yellow);
			g_bubbles_yellow = NULL;
		}
		ai_chat_yellow_bubble_draw(g_text_tip.c_str());
	}
	ai_chat_view_scroll_to_bottom();
}
static void ai_chat_create_new_blue_bubble(){
	if(g_bubbles_blue){
		now_index += lv_obj_get_height(g_bubbles_blue) + 10;
		now_index += lv_obj_get_height(g_bubbles_my) + 10;
	}
	g_headIcon = lv_img_create(g_mainWin);
	lv_img_dsc_t* head_icon_img = zb_img_res_load_one(NULL, AI_CHAT_IMG_HEAD_ICON, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(g_headIcon, head_icon_img);
	lv_obj_set_size(g_headIcon, head_icon_img->header.w, head_icon_img->header.h);
	lv_obj_set_pos(g_headIcon ,10, now_index+10);

	g_bubbles_blue = lv_obj_create(g_mainWin);
	lv_obj_align_to(g_bubbles_blue, g_headIcon, LV_ALIGN_OUT_RIGHT_TOP, 8, -5);
	lv_obj_set_style_bg_color(g_bubbles_blue, lv_color_hex(0xE3F3FC), 0);//rgb(224, 239, 248)
	lv_obj_set_style_radius(g_bubbles_blue, 10, 0);
	lv_obj_set_style_border_width(g_bubbles_blue, 0, 0);
	lv_obj_set_size(g_bubbles_blue, 0, 0);
	lv_obj_set_scroll_dir(g_bubbles_blue, LV_DIR_NONE);
	ai_chat_show_waiting_status();
}

static void ai_chat_show_waiting_status(){
	lv_obj_set_size(g_bubbles_blue, 100, 50);
	lv_obj_t *img = lv_img_create(g_bubbles_blue);
	lv_img_dsc_t *waiting_img = zb_img_res_load_one(NULL, AI_CHAT_IMG_WAITING_IMG, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(img, waiting_img);
	lv_obj_set_size(img, waiting_img->header.w, waiting_img->header.h);
	lv_obj_align(img, LV_ALIGN_LEFT_MID, 10, 0);
	lv_img_set_pivot(img,
		lv_obj_get_width(img) / 2,
		lv_obj_get_height(img) / 2);
	lv_obj_set_style_opa(img, LV_OPA_COVER, 0);
	anim = (lv_anim_t*)lv_mem_alloc(sizeof(lv_anim_t));
	lv_anim_init(anim);
	lv_anim_set_var(anim, img);
	lv_anim_set_exec_cb(anim, (lv_anim_exec_xcb_t)anim_img_angle_cb); 
	lv_anim_set_values(anim, 0, 15);								  
	lv_anim_set_time(anim, 1500);									  
	lv_anim_set_repeat_count(anim, LV_ANIM_REPEAT_INFINITE);
	lv_anim_set_path_cb(anim, lv_anim_path_linear);
	lv_anim_start(anim);
	ai_chat_view_scroll_to_bottom();
}

static void ai_chat_speak_done(){
	isTranEnable = false;
	isTipItemSelected = false;
	if (anim) {
		lv_anim_del(anim, (lv_anim_exec_xcb_t)anim_img_angle_cb);
		lv_mem_free(anim);
		anim = NULL;
	}
	ai_chat_yellow_bubble_hidden();
	ai_chat_create_new_blue_bubble();
	ai_chat_show_waiting_status();
	lv_refr_now(NULL);
	ai_chat_generate_dialog(g_generateTopicRes.get()->data.taskId,
								g_generateTopicRes.get()->data.scene,
								std::to_string(g_selGrade),
								g_chatHistory);
		std::string& res = g_generateDialogRes.get()->data.resultArr[0].result[0];
		g_chatHistory.push_back(HistoryItem{"System",res});
		g_text_eng = res;
		ai_chat_show_chat(g_text_eng.c_str());
		lv_refr_now(NULL);
	if(g_generateDialogRes.get()->data.endCode == 1){
		printf("chat is over\n");
		isChatOver = true;
		return;
	}
}

static void ai_chat_speak_tip_clickCb(lv_event_t* evt){
	if(isChatOver) return;
	ai_chat_show_speak_view();
}

int ai_chat_view_init(uint8_t selGrade,std::string selClass,std::string selTopic){
    g_chatCurWin = MMENU_AI_CHAT_VIEW;
	g_isInSubItem = true;
    g_selGrade = selGrade;
    g_selClass = selClass;
	isTranEnable = false;
	isTipItemSelected = false;
	now_index = 0;
	isChatOver = false;

	g_mainWin = NULL;
	g_tipItem = NULL;
	g_speakItem = NULL;
	g_headIcon = NULL;
	g_headIconmy = NULL;
	g_bubbles_blue = NULL;
	g_bubbles_yellow = NULL;
	g_bubbles_my = NULL;
	overlay = NULL;
	g_pressItem = NULL;
	g_closeItem = NULL;
	anim = NULL;



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

	aiChat_createEscBtn(top_title_win, ai_chat_view_del, 0);
	lv_obj_t * lblTitle = lv_label_create(top_title_win);
	lv_label_set_text(lblTitle, g_selClass.c_str());
	lv_obj_align(lblTitle, LV_ALIGN_TOP_MID, 0, AICHAT_ESC_BTN_LOC_Y + TOP_BTN_BORDER + TOP_BTN_TOP_SPACE_H);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), 0);

	g_mainWin = lv_obj_create(mainBg);
	lv_obj_set_size(g_mainWin, SCREEN_WIDTH, SCREEN_HEIGHT - 120);
	lv_obj_align(g_mainWin, LV_ALIGN_TOP_LEFT, 0, 60);
	lv_obj_set_style_bg_color(g_mainWin, colorBg, 0);
	lv_obj_set_style_border_width(g_mainWin, 0, 0);
	lv_obj_set_scrollbar_mode(g_mainWin, LV_SCROLLBAR_MODE_OFF);  // 关闭滚动条

	g_tipItem = lv_img_create(mainBg);
	lv_img_dsc_t* tip_blue_img = zb_img_res_load_one(NULL, AI_CHAT_IMG_TIP_BLUE, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(g_tipItem, tip_blue_img);
	lv_obj_set_size(g_tipItem, tip_blue_img->header.w, tip_blue_img->header.h);
	lv_obj_align(g_tipItem, LV_ALIGN_BOTTOM_LEFT, 10, -8);
	lv_obj_add_flag(g_tipItem, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(g_tipItem, ai_chat_tip_clickCb, LV_EVENT_CLICKED, NULL);

	g_speakItem = lv_img_create(mainBg);
	lv_img_dsc_t* speak_tip_img = zb_img_res_load_one(NULL, AI_CHAT_IMG_SPEAK_TIP, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(g_speakItem, speak_tip_img);
	lv_obj_set_size(g_speakItem, speak_tip_img->header.w, speak_tip_img->header.h);
	lv_obj_align(g_speakItem, LV_ALIGN_BOTTOM_RIGHT, -10, -8);
	lv_obj_add_flag(g_speakItem, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(g_speakItem, ai_chat_speak_tip_clickCb, LV_EVENT_CLICKED, NULL);

	ai_chat_create_new_blue_bubble();

	g_bubbles_yellow = NULL;
	g_bubbles_my = NULL;

	sys_setCurMainWinAndCb(mainBg,_ai_chat_scr_cb);
	g_chatMnuBgWin = mainBg;

	lv_refr_now(NULL);

	g_chatHistory.clear();
	if(ai_chat_generate_topic(selTopic)){
		printf("taskId:%s\n",g_generateTopicRes.get()->data.taskId.c_str());
		printf("content:%s\n",g_generateTopicRes.get()->data.scene.content.c_str());
		ai_chat_generate_dialog(g_generateTopicRes.get()->data.taskId,
								g_generateTopicRes.get()->data.scene,
								std::to_string(g_selGrade),
								g_chatHistory);
		std::string& res = g_generateDialogRes.get()->data.resultArr[0].result[0];
		g_chatHistory.push_back(HistoryItem{"System",res});
		g_text_eng = res;
		ai_chat_show_chat(g_text_eng.c_str());
		lv_refr_now(NULL);
	}
	
	
    return 0;
}

void ai_chat_take_audio_done() {
	ai_chat_asr();
	if(g_AsrRes.get()->code != "0"){
		printf("asr error:%s\n",g_AsrRes.get()->code.c_str());
		return;
	}
	std::string& res = g_AsrRes.get()->result[0];
	g_chatHistory.push_back(HistoryItem{"User",res});
	g_text_mine = res;
	ai_chat_hidden_speak_view();
	ai_chat_show_my_bubble(g_text_mine.c_str());
	ai_chat_speak_done();
}

static void ai_chat_press_cb(lv_event_t* evt){
	if(isChatOver) return;
	static bool pressedItem = true;	//用于标记被按下按键是 true为press false为close
	lv_obj_t* img = lv_event_get_target(evt);
	lv_indev_t* indev = lv_indev_get_act(); // 获取当前输入设备（如触摸屏）
	if (!indev) return;
	lv_point_t point;
	lv_indev_get_point(indev, &point); // 获取触摸点全局坐标

	bool is_insidepress = _lv_area_is_point_on(&g_pressItemArea, &point, 0); // 检查是否在图片内
	bool is_insideclose = _lv_area_is_point_on(&g_closeItemArea, &point, 0);

	switch (evt->code) {
	case LV_EVENT_PRESSED:
		if (is_insideclose)pressedItem = false;
		if (is_insidepress)pressedItem = true;
		break;
	case LV_EVENT_PRESSING: // 持续按压（滑动时触发）
		if (is_insideclose) 
			lv_obj_set_style_img_opa(g_closeItem, LV_OPA_50, 0);  // 进入区域：半透明
		else 
			lv_obj_set_style_img_opa(g_closeItem, LV_OPA_100, 0); // 离开区域：恢复
		if (is_insidepress)
			lv_obj_set_style_img_opa(g_pressItem, LV_OPA_50, 0);  // 进入区域：半透明
		else
			lv_obj_set_style_img_opa(g_pressItem, LV_OPA_100, 0); // 离开区域：恢复
		break;
	case LV_EVENT_RELEASED:
		lv_obj_set_style_img_opa(g_closeItem, LV_OPA_100, 0); // 离开区域：恢复
		lv_obj_set_style_img_opa(g_pressItem, LV_OPA_100, 0); // 离开区域：恢复		
		if (is_insideclose) {
			if (!pressedItem)
				ai_chat_hidden_speak_view();
		} else {
			if(pressedItem){
				printf("开始录音5s\n");
				//wav_file_check_and_delete();
				ai_chat_take_audio_start_5s();
				ai_chat_take_audio_done();
				// ai_chat_hidden_speak_view();
				// ai_chat_show_my_bubble(g_text_eng.c_str());
				// ai_chat_speak_done();
			}
		}
		break;
	default:
		break;
	}
}

static void ai_chat_hidden_speak_view(){
	if(g_pressItem){
		lv_obj_del(g_pressItem);
		g_pressItem = NULL;
	}
	if(g_closeItem){
		lv_obj_del(g_closeItem);
		g_closeItem = NULL;
	}
	if(overlay){
		lv_obj_del(overlay);
		overlay = NULL;
	}
}

static void ai_chat_show_speak_view(){
	overlay = lv_obj_create(lv_layer_top());			   // 在系统顶层创建
	lv_obj_set_size(overlay, SCREEN_WIDTH, SCREEN_HEIGHT);			   // 全屏尺寸
	lv_obj_set_style_bg_color(overlay, lv_color_hex(0x404040), 0); // 黑灰色
	lv_obj_set_style_bg_opa(overlay, LV_OPA_70, 0);				   // 60% 不透明度
	lv_obj_add_flag(overlay, LV_OBJ_FLAG_CLICKABLE);			   // 拦截点击事件
	lv_obj_set_scroll_dir(overlay, LV_DIR_NONE);
	lv_obj_set_style_border_width(overlay, 0, 0);
	lv_obj_set_style_shadow_width(overlay, 0, 0);

	lv_obj_t *white_win = lv_obj_create(overlay);
	lv_obj_set_size(white_win, 1000, 1000); // 高度240px
	lv_obj_align(white_win, LV_ALIGN_BOTTOM_MID, 0, 850);
	lv_obj_set_style_radius(white_win, LV_RADIUS_CIRCLE, 0); // 关键：设为圆形
	lv_obj_set_style_bg_color(white_win, lv_color_white(), 0); // 白色背景
	lv_obj_set_style_bg_opa(white_win, LV_OPA_100, 0); // 不透明
	lv_obj_set_style_border_width(white_win, 0, 0); // 无边框

	lv_obj_t* speaking_Item = lv_img_create(overlay);
	lv_img_dsc_t* speaking_img = zb_img_res_load_one(NULL, AI_CHAT_IMG_SPEAKING, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(speaking_Item, speaking_img);
	lv_obj_set_size(speaking_Item, speaking_img->header.w, speaking_img->header.h);
	lv_obj_align_to(speaking_Item,white_win, LV_ALIGN_OUT_TOP_MID, 0, -45);

	g_pressItem = lv_img_create(white_win);
	lv_img_dsc_t* pressing_img = zb_img_res_load_one(NULL, AI_CHAT_IMG_PRESSING, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(g_pressItem, pressing_img);
	lv_obj_set_size(g_pressItem, pressing_img->header.w, pressing_img->header.h);
	lv_obj_align(g_pressItem, LV_ALIGN_TOP_MID, 0, 45);
	lv_obj_add_flag(g_pressItem, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(g_pressItem, ai_chat_press_cb, LV_EVENT_ALL, NULL);
	
	g_closeItem = lv_img_create(white_win);
	lv_img_dsc_t* close_img = zb_img_res_load_one(NULL, AI_CHAT_IMG_CLOSE, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(g_closeItem, close_img);
	lv_obj_set_size(g_closeItem, close_img->header.w, close_img->header.h);
	lv_obj_align_to(g_closeItem, g_pressItem ,LV_ALIGN_OUT_LEFT_MID, -40, 0);
	lv_obj_add_flag(g_closeItem, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(g_closeItem, ai_chat_press_cb, LV_EVENT_ALL, NULL);
	lv_refr_now(NULL);
	
	lv_obj_get_coords(g_pressItem, &g_pressItemArea);
	lv_obj_get_coords(g_closeItem, &g_closeItemArea);
}
static void ai_chat_view_clean(void){
	if (anim) {
		lv_anim_del(anim, (lv_anim_exec_xcb_t)anim_img_angle_cb);
		lv_mem_free(anim);
		anim = NULL;
	}
	g_mainWin = NULL;
	g_tipItem = NULL;
	g_speakItem = NULL;
	g_headIcon = NULL;
	g_headIconmy = NULL;
	g_bubbles_blue = NULL;
	g_bubbles_yellow = NULL;
	g_bubbles_my = NULL;
	overlay = NULL;
	g_pressItem = NULL;
	g_closeItem = NULL;
	anim = NULL;
	now_index = 0;
	isChatOver = false;

	g_text_cn = "";
	g_text_eng = "";
	g_text_tip = "";
	g_text_mine = "";
	if(g_generateDialogRes){
		g_generateDialogRes.reset();
		g_generateDialogRes = nullptr;
	}
	if(g_generateTopicRes){
		g_generateTopicRes.reset();
		g_generateTopicRes = nullptr;
	}
	if(g_generateRecommendationRes){
		g_generateRecommendationRes.reset();
		g_generateRecommendationRes = nullptr;
	}
	g_chatHistory.clear();

	if(g_enzhTranslationRes){
		g_enzhTranslationRes.reset();
		g_enzhTranslationRes = nullptr;
	}
	if(g_AsrRes){
		g_AsrRes.reset();
		g_AsrRes = nullptr;
	}
	if(g_TtsRes){
		g_TtsRes.reset();
		g_TtsRes = nullptr;
	}
}
void ai_chat_view_del(lv_event_t* e){
    ai_chat_ap_win_deinit();
    ai_chat_menu_scr_init(0);
	ai_chat_view_clean();
}