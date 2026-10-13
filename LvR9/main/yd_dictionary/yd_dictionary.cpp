extern "C" {
    #include "../data_parse/img_res.h"
	#include "../data_parse/img_std_res.h"
    #include "../../trd/openssl/include/openssl/sha.h"
    #include "../../trd/tz_audio_player/include/tz_pcm_audio.h"
    #include "../../trd/tz_audio_player/include/tz_audio_player.h"
}

#include "../online_tool/http_get_online_res.h" 
#include "../online_tool/global_http_tool.h"
#include <vector>
#include <iostream>
#include "yd_dictionary.h"

static YdDictionary *g_yd_dictionary = nullptr;
static std::list<std::pair<std::string,std::string>> m_record_list;
static void AP_MainWin_scr_cb(lv_event_t* evt);
static void AP_esc_click_cb(lv_event_t * evt);


static void _search_area_click_cb(lv_event_t * evt){
    g_yd_dictionary->Ap_MainWin_del();
    g_yd_dictionary->AP_SearchWin_init();
}
int YdDictionary::AP_MainWin_init(){
    mwin_CurWin = YdDictionary_MAIN;
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
	lv_label_set_text(lblTitle, "有道词典");
	lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
	lv_obj_align(lblTitle, LV_ALIGN_CENTER, 0,0);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), 0);

    lv_obj_t * button_win = lv_obj_create(mainBg);
    lv_obj_align(button_win, LV_ALIGN_BOTTOM_MID, 0, -120);
    lv_obj_set_size(button_win, 300, 100);
    lv_obj_set_style_border_width(button_win, 0, 0);
    lv_obj_set_scroll_dir(button_win, LV_DIR_NONE);

    lv_obj_t* label1 = lv_label_create(button_win);
    lv_label_set_text(label1, "有道简明");
    lv_obj_align(label1, LV_ALIGN_TOP_LEFT, 47, 25);
    lv_label_set_cjk_font(label1, 19, FT_FONT_STYLE_BOLD);
	lv_obj_set_style_text_color(label1, lv_color_hex(0xB6BFC9), 0); //  #B6BFC9

    lv_obj_t* label2 = lv_label_create(button_win);
    lv_label_set_text(label2, "译界金标");
    lv_obj_align(label2, LV_ALIGN_TOP_RIGHT, -47, 25);
    lv_label_set_cjk_font(label2, 19, FT_FONT_STYLE_BOLD);
	lv_obj_set_style_text_color(label2, lv_color_hex(0xB6BFC9), 0);

    lv_obj_t* label3 = lv_label_create(button_win);
    lv_label_set_text(label3, "应试助手");
    lv_obj_align(label3, LV_ALIGN_BOTTOM_LEFT, 49, -27);
    lv_label_set_cjk_font(label3, 14, FT_FONT_STYLE_NORMAL);
	lv_obj_set_style_text_color(label3, lv_color_hex(0xB6BFC9), 0);
    lv_obj_set_style_text_letter_space(label3, 5, 0);

    lv_obj_t* label4 = lv_label_create(button_win);
    lv_label_set_text(label4, "高效工具");
    lv_obj_align(label4, LV_ALIGN_BOTTOM_RIGHT, -47, -27);
    lv_label_set_cjk_font(label4, 14, FT_FONT_STYLE_NORMAL);
	lv_obj_set_style_text_color(label4, lv_color_hex(0xB6BFC9), 0);
    lv_obj_set_style_text_letter_space(label4, 5, 0);

    // 创建垂直线对象
    lv_obj_t *line = lv_line_create(button_win);
    static lv_point_t points[] = {{150, 33}, {150, 67}}; // X坐标相同，Y坐标不同
    lv_line_set_points(line, points, 2);

    lv_obj_set_style_line_width(line, 1, 0);
    lv_obj_set_style_line_color(line, lv_color_hex(0xB6BFC9), 0);//#B6BFC9


    lv_obj_t* search_area = lv_obj_create(mainBg);
    lv_obj_align(search_area, LV_ALIGN_TOP_MID, 0, 180);
    lv_obj_set_size(search_area, 270, 58);
    lv_obj_set_style_border_width(search_area, 1, 0);
    lv_obj_set_style_border_color(search_area, lv_color_black(), 0);
    lv_obj_set_style_radius(search_area, 12, 0);
    lv_obj_add_flag(search_area,LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t* search_img = lv_img_create(search_area);
    lv_img_set_src(search_img, &serch_icon);
    lv_obj_align(search_img, LV_ALIGN_LEFT_MID, 10, 0);

    lv_obj_t* search_label = lv_label_create(search_area);
    lv_label_set_text(search_label, "请输入单词");
    lv_label_set_cjk_font(search_label, 18, FT_FONT_STYLE_NORMAL);
    lv_obj_align_to(search_label,search_img, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_set_style_text_color(search_label, lv_color_hex(0xB6BFC9), 0);

    lv_obj_t* logo_img = lv_img_create(mainBg);
    lv_img_set_src(logo_img, &youdao_logo);
    lv_obj_align_to(logo_img,search_area, LV_ALIGN_OUT_TOP_MID, 0, -15);

    lv_obj_add_event_cb(search_area, _search_area_click_cb, LV_EVENT_CLICKED, NULL);


    sys_setCurMainWinAndCb(mainBg,AP_MainWin_scr_cb);
	mwin_BgWin = mainBg;
	return 0;
}



static lv_obj_t* last_list_win = nullptr;
static lv_obj_t* keyboard = nullptr;
void YdDictionary::Ap_set_search_word(std::string& word){
    m_search_word = word;
    if(m_search_word.empty()) {
        ESP_LOGE("SEARCH", "Empty search word detected");
        return;
    }
}
static void yd_dictionary_search_word(std::string& word){
    if(word.empty()){
        return;
    }
    g_yd_dictionary->is_en = true;
    g_yd_dictionary->Ap_set_search_word(word);
    
    g_yd_dictionary->Ap_send_dictionnary_request(word,g_yd_dictionary->is_en,[](){
        if(g_yd_dictionary->g_DictionaryRes->code != "0"){
            std::cout<<"error code:"<<g_yd_dictionary->g_DictionaryRes->code<<std::endl;
            return;
        }
        g_yd_dictionary->Ap_MainWin_del();
        g_yd_dictionary->AP_ContentWin_init();
    });
}
static void _clear_all_click_cb(lv_event_t * evt){
    lv_obj_clean(last_list_win);
    m_record_list.clear();
}
static void _record_item_click_cb(lv_event_t * evt){
	std::string* word = static_cast<std::string*>(lv_event_get_user_data(evt));
    yd_dictionary_search_word(*word);
}
static void keyboard_num_event_cb(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* ta = (lv_obj_t*)lv_event_get_user_data(e);
    if (code == LV_EVENT_READY) {
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        const char* text = lv_textarea_get_text(ta);
        if(text) {
            std::string search_word(text);
            yd_dictionary_search_word(search_word);
        } else {
            LV_LOG_WARN("Empty textarea content");
        }
    }
}
static void search_focus_event_cb(lv_event_t* e) {
	lv_obj_t* kb = keyboard;
	lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
	lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
}
static void list_insert_record(lv_obj_t* list_win,std::string& record_word,std::string& record_res){
	lv_obj_t*  record_item = lv_obj_create(list_win);
	lv_obj_set_size(record_item, 280, 50);
	lv_obj_set_style_border_width(record_item, 0, 0);
	lv_obj_set_style_border_color(record_item, lv_color_white(), 0);
	lv_obj_set_style_radius(record_item, 12, 0);
    lv_obj_set_scroll_dir(record_item, LV_DIR_NONE);
    lv_obj_t* label1 = lv_label_create(record_item);


    lv_label_set_text(label1, record_word.c_str());
    lv_label_set_cjk_font(label1, 18, FT_FONT_STYLE_NORMAL);
    lv_obj_align(label1, LV_ALIGN_LEFT_MID, 15, 0);
    lv_obj_set_style_text_color(label1, lv_color_black(), 0);

    lv_obj_t* label2 = lv_label_create(record_item);
    lv_label_set_text(label2, record_res.c_str());
    lv_label_set_cjk_font(label2, 18, FT_FONT_STYLE_NORMAL);
    lv_obj_align_to(label2,label1, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_set_style_text_color(label2, lv_color_hex(0xB6BFC9), 0);

    lv_obj_add_flag(record_item,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(record_item, _record_item_click_cb, LV_EVENT_CLICKED, static_cast<void*>(&record_word));
}
int YdDictionary::AP_SearchWin_init(){
    mwin_CurWin = YdDictionary_SEARCH;
    mwin_IsInSubItem = true;

    lv_obj_t* mainBg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mainBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainBg, 0, 0);
	lv_obj_set_style_pad_all(mainBg, 0, 0);
	lv_color_t colorBg = lv_color_hex(0xF0F0F0);
	lv_obj_set_style_bg_color(mainBg, colorBg, 0);
	lv_obj_set_scroll_dir(mainBg, LV_DIR_NONE);

    lv_obj_t * top_title_win = lv_obj_create(mainBg);
    lv_obj_set_size(top_title_win, LV_PCT(100), 60);
    lv_obj_align(top_title_win, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(top_title_win, lv_color_white(), 0);
    lv_obj_set_style_border_width(top_title_win, 0, 0);
	lv_obj_set_style_pad_all(top_title_win, 0, 0);
	lv_obj_set_style_pad_row(top_title_win, 0, 0);
	lv_obj_set_style_pad_column(top_title_win, 0, 0);
    lv_obj_set_style_radius(top_title_win, 0, 0);
	lv_obj_set_scroll_dir(top_title_win, LV_DIR_NONE);

	Ap_createEscBtn(top_title_win, AP_esc_click_cb, 0);
    lv_obj_t * search_textarea = lv_textarea_create(top_title_win);
    lv_textarea_set_one_line(search_textarea, true);
    lv_obj_set_size(search_textarea, 250, 44);
    lv_obj_set_style_border_width(search_textarea, 0, 0);
    lv_obj_set_style_radius(search_textarea, 12, 0);
    lv_obj_set_style_bg_color(search_textarea, lv_color_hex(0xF0F0F0), 0);
    lv_obj_align(search_textarea, LV_ALIGN_LEFT_MID, 55, 0);
    lv_obj_add_flag(search_textarea,LV_OBJ_FLAG_CLICKABLE);
    //lv_obj_add_flag(search_textarea,LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_add_event_cb(search_textarea, search_focus_event_cb, LV_EVENT_CLICKED, search_textarea);
	lv_obj_set_style_text_align(search_textarea, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);  // 文本左对齐
	lv_obj_set_style_pad_left(search_textarea, 10, LV_PART_MAIN);   // 左边距
	lv_obj_set_style_pad_right(search_textarea, 10, LV_PART_MAIN);  // 右边距
	lv_obj_set_style_pad_top(search_textarea, 12, LV_PART_MAIN);     // 上边距（调整垂直居中）

	//lv_obj_add_event_cb(search_textarea, search_focus_event_cb, LV_EVENT_FOCUSED, NULL);
	//lv_obj_add_event_cb(search_textarea, search_focus_event_cb, LV_EVENT_DEFOCUSED, NULL);

    lv_obj_t* label1 = lv_label_create(mainBg);
    lv_label_set_text(label1, "最近搜索");
    lv_label_set_cjk_font(label1, 18, FT_FONT_STYLE_NORMAL);
    lv_obj_align_to(label1,top_title_win, LV_ALIGN_OUT_BOTTOM_LEFT, 15, 15);

    lv_obj_t* clear_area = lv_obj_create(mainBg);
    lv_obj_align_to(clear_area,top_title_win, LV_ALIGN_OUT_BOTTOM_RIGHT, 25, 8);
    lv_obj_set_size(clear_area, 100, 40);
    lv_obj_set_style_bg_color(clear_area, lv_color_hex(0xF0F0F0), 0);
    lv_obj_set_style_border_width(clear_area, 0, 0);
    lv_obj_add_flag(clear_area,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(clear_area, _clear_all_click_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* label2 = lv_label_create(clear_area);
    lv_label_set_text(label2, "全部清空");
    lv_label_set_cjk_font(label2, 18, FT_FONT_STYLE_NORMAL);
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label2, lv_color_hex(0xB6BFC9), 0);

    last_list_win = lv_obj_create(mainBg);
	lv_obj_set_size(last_list_win, LV_PCT(100), SCREEN_HEIGHT - 120);	//todo 260 为顶部标题栏高度 + 底部播放按钮高度
	lv_obj_align_to(last_list_win,top_title_win, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
	lv_obj_set_style_bg_color(last_list_win, colorBg, 0);
	lv_obj_set_style_border_width(last_list_win, 0,0);
	lv_obj_set_scrollbar_mode(last_list_win, LV_SCROLLBAR_MODE_OFF);  // 关闭滚动条

	lv_obj_set_flex_flow(last_list_win, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(last_list_win, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_top(last_list_win, 0, 0);
	lv_obj_set_style_pad_left(last_list_win, 15, 0);
	lv_obj_set_style_pad_right(last_list_win, 15, 0);
	lv_obj_set_style_pad_row(last_list_win, 15, 0);
	lv_obj_set_style_pad_column(last_list_win, 10, 0);

    //插入历史记录
    for(auto& record : m_record_list){
        list_insert_record(last_list_win,record.first,record.second);
    }

    keyboard = lv_keyboard_create(mainBg);
	lv_obj_set_style_text_font(keyboard, &lv_font_montserrat_16, 0);
	lv_keyboard_set_textarea(keyboard, search_textarea);
	lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);

	lv_obj_set_size(keyboard, SCREEN_WIDTH, 160);
	lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_set_style_radius(keyboard, 0, LV_PART_MAIN);
	lv_obj_set_style_bg_color(keyboard, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
	
	lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
	lv_obj_add_event_cb(keyboard, keyboard_num_event_cb, LV_EVENT_ALL, search_textarea);

    sys_setCurMainWinAndCb(mainBg,AP_MainWin_scr_cb);
	mwin_BgWin = mainBg;
	return 0;
}

static lv_timer_t* icon_blink_timer = nullptr;
static lv_obj_t* last_icon = nullptr;
static bool is_blink = false;
static void add_trans_item_to_list(lv_obj_t* list_win,std::string& record_word,std::string& record_res){
    lv_obj_t*  record_item = lv_obj_create(list_win);
    lv_obj_add_flag(record_item,LV_OBJ_FLAG_HIDDEN);
	lv_obj_set_size(record_item, SCREEN_WIDTH, 30);
	lv_obj_set_style_border_width(record_item, 0, 0);
	lv_obj_set_style_bg_color(record_item, lv_color_hex(0xf0f0f0), 0);
    lv_obj_set_scroll_dir(record_item, LV_DIR_NONE);

    lv_obj_t* label1 = lv_label_create(record_item);
    lv_label_set_text(label1, record_word.c_str());
    lv_label_set_cjk_font(label1, 18, FT_FONT_STYLE_NORMAL);
    lv_obj_set_style_text_color(label1, lv_color_hex(0xB6BFC9), 0);
    lv_refr_now(nullptr);

    int lw = lv_obj_get_width(label1);
    lv_obj_t* label2 = lv_label_create(record_item);
    lv_label_set_text(label2, record_res.c_str());
    lv_refr_now(nullptr);
    if(lv_obj_get_width(label2) + lw > SCREEN_WIDTH - 30){
        lv_obj_set_width(label2, SCREEN_WIDTH - 30 - lw);
        lv_label_set_long_mode(label2, LV_LABEL_LONG_WRAP);
        lv_label_set_text(label2, record_res.c_str());
    }
    lv_label_set_cjk_font(label2, 18, FT_FONT_STYLE_NORMAL);
    lv_obj_set_style_text_color(label2, lv_color_black(), 0);
    lv_refr_now(nullptr);

    lv_obj_set_height(record_item,lv_obj_get_height(label2) + 10);
    lv_obj_align(label1, LV_ALIGN_TOP_LEFT, 15, 0);
    lv_obj_align_to(label2,label1, LV_ALIGN_OUT_RIGHT_TOP, 5, 0);
    lv_obj_clear_flag(record_item,LV_OBJ_FLAG_HIDDEN);
}
static void _record_win_click_cb(lv_event_t * evt){
    g_yd_dictionary->Ap_MainWin_del();
    g_yd_dictionary->AP_FollowWin_init();
}
static void play_audio_by_url(const std::string& url){
    tz_play_scheduling_stop_audio();
    tz_play_audio(url.c_str(),0,-1, false);
}
static void speak_icon_blink_stop(){
    if(icon_blink_timer!=nullptr){
        lv_timer_del(icon_blink_timer);
        icon_blink_timer = nullptr;
    }
    if(last_icon){
        lv_img_set_src(last_icon, &speak_out_icon);
        last_icon = nullptr;
    }
    is_blink = false;
}
static void speak_icon_timer_cb(lv_timer_t* t){
    static TZ_PLAYSTATE_T last_st = TZ_PLAYIDLE;
    TZ_PLAYSTATE_T p_st = get_play_audio_state();
    if(last_st == TZ_PLAYING && p_st != TZ_PLAYING){
        speak_icon_blink_stop();
    }
    last_st = p_st;
    if(last_icon){
        if(is_blink){
            lv_img_set_src(last_icon, &speak_out_icon);
            std::cout<<"speak out"<<std::endl;
        }else{
            lv_img_set_src(last_icon, &speak_out_icon_half);
            std::cout<<"speak in"<<std::endl;
        }
        is_blink = !is_blink;
    }
}
static void speak_icon_blink(lv_obj_t* speak_icon){
    speak_icon_blink_stop();
    last_icon = speak_icon;
    icon_blink_timer = lv_timer_create(speak_icon_timer_cb,400,nullptr);
}
static void speak_context_clicked(lv_event_t* e){
    lv_obj_t* ta = lv_event_get_target(e);
    lv_obj_t* icon = lv_obj_get_child(ta,0);
    speak_icon_blink(icon);
	std::string* word = static_cast<std::string*>(lv_event_get_user_data(e));
    play_audio_by_url(*word);
}
static void speak_context_tts_clicked(lv_event_t* e){
    lv_obj_t* ta = lv_event_get_target(e);
    lv_obj_t* icon = lv_obj_get_child(ta,0);
    speak_icon_blink(icon);
    tz_play_scheduling_stop_audio();
	std::string* word = static_cast<std::string*>(lv_event_get_user_data(e));
    std::string lang;
    if(g_yd_dictionary->is_en){
        lang = "en";
    }else{
        lang = "zh-CHS";
    }
    g_yd_dictionary->Ap_tts_gen(*word, lang,[](){
		if(g_yd_dictionary->get_cur_win() != YdDictionary_CONTENT){
				if(g_yd_dictionary->g_TtsRes){
					g_yd_dictionary->g_TtsRes.reset();
					g_yd_dictionary->g_TtsRes = nullptr;
				}
				return;
		}
		if(g_yd_dictionary->g_TtsRes.get()->code == "0"){
                const std::string filepath = "/userdata/tts_output.mp3";
                std::ofstream ofs(filepath, std::ios::binary);
                if(ofs) {
                    ofs.write(reinterpret_cast<const char*>(g_yd_dictionary->g_TtsRes.get()->result.data()), 
                    g_yd_dictionary->g_TtsRes.get()->result.size());
                    std::cout << "Audio saved to: " << filepath << std::endl;
                } else {
                    std::cerr << "Failed to open file for writing: " << filepath << std::endl;
                }
                ofs.close();
    
            tz_play_audio("/userdata/tts_output.mp3",0,-1, false);
		}else{
			printf("tts error:%s\n",g_yd_dictionary->g_TtsRes.get()->code.c_str());
		}
	});
}

int YdDictionary::AP_ContentWin_init(){
    mwin_CurWin = YdDictionary_CONTENT;
	mwin_IsInSubItem = true;

    std::string lang;
    if(is_en){
        lang = "ec";
    }else{
        lang = "ce";
    }
    DictionaryEntry& entry = g_DictionaryRes.get()->results[lang];
    lv_obj_t* mainBg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mainBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainBg, 0, 0);
	lv_obj_set_style_pad_all(mainBg, 0, 0);
	lv_color_t colorBg = lv_color_hex(0xF0F0F0);
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


	Ap_createEscBtn(top_title_win, AP_esc_click_cb, 1);
	lv_obj_t * lblTitle = lv_label_create(top_title_win);
	lv_label_set_text(lblTitle, "释义");
	lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
	lv_obj_align(lblTitle, LV_ALIGN_CENTER, 0,0);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), 0);


    lv_obj_t* trans_win = lv_obj_create(mainBg);
    lv_obj_add_flag(trans_win,LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(trans_win, LV_PCT(100),SCREEN_HEIGHT - 60);
    lv_obj_align(trans_win, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_set_style_bg_color(trans_win, colorBg, 0);
	lv_obj_set_style_border_width(trans_win, 0,0);
	lv_obj_set_scrollbar_mode(trans_win, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(trans_win,LV_DIR_VER);

	lv_obj_set_flex_flow(trans_win, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(trans_win, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_top(trans_win, 0, 0);
	lv_obj_set_style_pad_left(trans_win, 15, 0);
	lv_obj_set_style_pad_right(trans_win, 15, 0);
	lv_obj_set_style_pad_row(trans_win, 10, 0);
	lv_obj_set_style_pad_column(trans_win, 10, 0);

    DictionaryBasic& basic = entry.basic;
    lv_obj_t* content_win = lv_obj_create(trans_win);
    lv_obj_set_size(content_win, 290, 150);
    lv_obj_set_style_border_width(content_win, 0, 0);
    lv_obj_set_style_radius(content_win, 12, 0);
    lv_obj_set_style_bg_color(content_win, lv_color_white(), 0);
    lv_obj_set_scroll_dir(content_win, LV_DIR_NONE);

    lv_obj_t* label1 = lv_label_create(content_win);
    lv_obj_align(label1, LV_ALIGN_TOP_LEFT, 20, 17);
    lv_label_set_cjk_font(label1, 18, FT_FONT_STYLE_NORMAL);
    lv_label_set_text(label1, m_search_word.c_str());
    lv_obj_set_style_text_color(label1, lv_color_black(), 0);

    int speak_context_start_pos = 50;

    bool is_played = false;
    if(!basic.ukSpeech.empty()){
    lv_obj_t* speak_context = lv_obj_create(content_win);
    lv_obj_align(speak_context, LV_ALIGN_TOP_LEFT, 10, speak_context_start_pos);
    speak_context_start_pos += 40;
    lv_obj_set_size(speak_context, 40, 40);
    lv_obj_set_style_border_width(speak_context, 0, 0);
    lv_obj_add_flag(speak_context,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scroll_dir(speak_context, LV_DIR_NONE);
    lv_obj_t* speak_img = lv_img_create(speak_context);
    lv_img_set_src(speak_img, &speak_out_icon);
    lv_obj_align(speak_img, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* label2 = lv_label_create(content_win);
    lv_obj_align_to(label2,speak_img, LV_ALIGN_OUT_RIGHT_MID, 10, -5);
    lv_label_set_cjk_font(label2, 18, FT_FONT_STYLE_NORMAL);
    //lv_obj_set_style_text_font(label2, &lv_font_montserrat_16, 0);
    lv_label_set_text(label2, "英");
    lv_obj_t* label2en = lv_label_create(content_win);
    lv_obj_align_to(label2en,label2, LV_ALIGN_OUT_RIGHT_MID, 0, -2);
    lv_label_set_font(label2en, 18, FT_FONT_STYLE_NORMAL);
    //lv_obj_set_style_text_font(label2, &lv_font_montserrat_16, 0);
    lv_label_set_text(label2en, ("/"+basic.ukPhonetic+"/").c_str());
    lv_obj_add_event_cb(speak_context, speak_context_clicked, LV_EVENT_CLICKED, static_cast<void*>(&basic.ukSpeech));

    if(!is_played){
        play_audio_by_url(basic.ukSpeech);
        is_played = true;
    }
    }
    if(!basic.ukSpeech.empty()){
    lv_obj_t* speak_context2 = lv_obj_create(content_win);
    lv_obj_align(speak_context2, LV_ALIGN_TOP_LEFT, 10, speak_context_start_pos);
    speak_context_start_pos += 40;
    lv_obj_set_size(speak_context2, 40, 40);
    lv_obj_set_style_border_width(speak_context2, 0, 0);
    lv_obj_add_flag(speak_context2,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scroll_dir(speak_context2, LV_DIR_NONE);
    lv_obj_t* speak_imgs = lv_img_create(speak_context2);
    lv_img_set_src(speak_imgs, &speak_out_icon);
    lv_obj_align(speak_imgs, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* label2s = lv_label_create(content_win);
    lv_obj_align_to(label2s,speak_imgs, LV_ALIGN_OUT_RIGHT_MID, 10, -5);
    lv_label_set_cjk_font(label2s, 18, FT_FONT_STYLE_NORMAL);
    lv_label_set_text(label2s, "美");
    lv_obj_t* label2sen = lv_label_create(content_win);
    lv_obj_align_to(label2sen,label2s, LV_ALIGN_OUT_RIGHT_MID, 0, -2);
    lv_label_set_font(label2sen, 18, FT_FONT_STYLE_NORMAL);
    lv_label_set_text(label2sen, ("/"+basic.usPhonetic+"/").c_str());
    lv_obj_add_event_cb(speak_context2, speak_context_clicked, LV_EVENT_CLICKED, static_cast<void*>(&basic.usSpeech));
    if(!is_played){
        play_audio_by_url(basic.usSpeech);
        is_played = true;
    }
    }

    if(basic.phonetics.size() > 0){
        for(auto& phonetic : basic.phonetics){
        lv_obj_t* speak_contextzh = lv_obj_create(content_win);
        lv_obj_align(speak_contextzh, LV_ALIGN_TOP_LEFT, 10, speak_context_start_pos);
        speak_context_start_pos += 40;
        lv_obj_set_size(speak_contextzh, 40, 40);
        lv_obj_set_style_border_width(speak_contextzh, 0, 0);
        lv_obj_add_flag(speak_contextzh,LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_scroll_dir(speak_contextzh, LV_DIR_NONE);
        lv_obj_t* speak_imgzh = lv_img_create(speak_contextzh);
        lv_img_set_src(speak_imgzh, &speak_out_icon);
        lv_obj_align(speak_imgzh, LV_ALIGN_CENTER, 0, 0);

        lv_obj_t* labelzh = lv_label_create(content_win);
        lv_obj_align_to(labelzh,speak_imgzh, LV_ALIGN_OUT_RIGHT_MID, 10, -5);
        lv_label_set_cjk_font(labelzh, 18, FT_FONT_STYLE_NORMAL);
        lv_label_set_text(labelzh, (phonetic.phonetic+" "+phonetic.pronunciation).c_str());
        lv_obj_add_event_cb(speak_contextzh, speak_context_clicked, LV_EVENT_CLICKED, static_cast<void*>(&phonetic.speech));
        if(!is_played){
            play_audio_by_url(phonetic.speech);
            is_played = true;
        }
        } 	
    }

    lv_obj_set_size(content_win, 290, speak_context_start_pos+10);

    // lv_obj_t* record_context = lv_obj_create(content_win);
    // lv_obj_align(record_context, LV_ALIGN_BOTTOM_RIGHT, -10, -5);
    // lv_obj_set_size(record_context, 50, 50);
    // lv_obj_set_style_border_width(record_context, 0, 0);
    // lv_obj_add_flag(record_context,LV_OBJ_FLAG_CLICKABLE);
    // lv_obj_set_scroll_dir(record_context, LV_DIR_NONE);
    // lv_obj_t* record_img = lv_img_create(record_context);
    // lv_img_set_src(record_img, &record_icon);
    // lv_obj_align(record_img, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_add_event_cb(record_context, _record_win_click_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* title_win1 = lv_obj_create(trans_win);
    lv_obj_set_size(title_win1, LV_PCT(100), 30);
    lv_obj_set_style_border_width(title_win1, 0, 0);
    lv_obj_set_style_bg_color(title_win1, colorBg, 0);

    lv_obj_t* blue_cont = lv_obj_create(title_win1);
    lv_obj_set_size(blue_cont, 3, 20);
    lv_obj_align(blue_cont, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_bg_color(blue_cont, lv_color_hex(0x008CFF), 0);
    lv_obj_set_style_border_width(blue_cont, 0, 0);
    lv_obj_set_style_radius(blue_cont, 10, 0);

    lv_obj_t* title_label1 = lv_label_create(title_win1);
    lv_label_set_text(title_label1, "基本释义");
    lv_label_set_cjk_font(title_label1, 18, FT_FONT_STYLE_NORMAL);
    lv_obj_align_to(title_label1,blue_cont, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    std::vector<ExplainEntry>& explains = basic.explains;
    std::string explains_pos = explains[0].pos;
    std::string explains_trans = explains[0].trans;

    auto it = std::find_if(m_record_list.begin(), m_record_list.end(),
        [&](const auto& pair){ return pair.first == m_search_word; });

    if (it != m_record_list.end()) {
        m_record_list.erase(it);
    }
    
    m_record_list.push_front(std::make_pair(m_search_word, explains_pos + explains_trans));
    
    if(m_record_list.size() > 10) {
        m_record_list.pop_back();
    }


    for(auto& trans : basic.explains){
    	add_trans_item_to_list(trans_win,trans.pos,trans.trans);
    }


    lv_obj_t* title_win2 = lv_obj_create(trans_win);
    lv_obj_set_size(title_win2, LV_PCT(100), 30);
    lv_obj_set_style_border_width(title_win2, 0, 0);
    lv_obj_set_style_bg_color(title_win2, colorBg, 0);

    lv_obj_t* blue_cont2 = lv_obj_create(title_win2);
    lv_obj_set_size(blue_cont2, 3, 20);
    lv_obj_align(blue_cont2, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_bg_color(blue_cont2, lv_color_hex(0x008CFF), 0);
    lv_obj_set_style_border_width(blue_cont2, 0, 0);
    lv_obj_set_style_radius(blue_cont2, 10, 0);

    lv_obj_t* title_label2 = lv_label_create(title_win2);
    lv_label_set_text(title_label2, "双语例句");
    lv_label_set_cjk_font(title_label2, 18, FT_FONT_STYLE_NORMAL);
    lv_obj_align_to(title_label2,blue_cont2, LV_ALIGN_OUT_RIGHT_MID, 10, 0);


    lv_obj_t* sentence_win = lv_obj_create(trans_win);
    lv_obj_set_size(sentence_win, SCREEN_WIDTH, 100);
    lv_obj_set_style_border_width(sentence_win, 0, 0);
    lv_obj_set_scroll_dir(sentence_win, LV_DIR_NONE);
    lv_obj_set_style_bg_color(sentence_win, colorBg, 0);

    
    lv_obj_t* speak_context3 = lv_obj_create(sentence_win);
    lv_obj_align(speak_context3, LV_ALIGN_TOP_LEFT, 5, -5);
    lv_obj_set_size(speak_context3, 50, 50);
    lv_obj_set_style_border_width(speak_context3, 0, 0);
    lv_obj_set_style_bg_color(speak_context3, colorBg, 0);
    lv_obj_add_flag(speak_context3,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scroll_dir(speak_context3, LV_DIR_NONE);
    lv_obj_t* speak_img2 = lv_img_create(speak_context3);
    lv_img_set_src(speak_img2, &speak_out_icon);
    lv_obj_align(speak_img2, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(speak_context3, speak_context_tts_clicked, LV_EVENT_CLICKED, static_cast<void*>(&entry.sentenceSample[0].sentence));


    ExampleSentence& sentence = entry.sentenceSample[0];
    std::string orignal = sentence.sentence;
    std::string translation = sentence.translation;

    lv_obj_t * span_cont = lv_spangroup_create(sentence_win);
    lv_spangroup_set_align(span_cont, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(span_cont, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(span_cont, LV_SPAN_MODE_BREAK);
    lv_obj_set_width(span_cont, SCREEN_WIDTH - 80);
    lv_obj_set_height(span_cont, LV_SIZE_CONTENT);
    lv_obj_align_to(span_cont,speak_context3, LV_ALIGN_OUT_RIGHT_TOP, 10, 4);

    lv_style_t style_normal;
    lv_style_init(&style_normal);
    lv_style_set_text_color(&style_normal, lv_color_black());
    lv_ft_info_t info = generate_font_info(18, FT_FONT_STYLE_NORMAL);
    lv_style_set_text_font(&style_normal, info.font);
    
    lv_style_t style_highlight;
    lv_style_init(&style_highlight);
    lv_style_set_text_color(&style_highlight, lv_color_hex(0x008CFF));
    lv_style_set_text_font(&style_highlight, info.font);

    lv_style_t style_normal1;
    lv_style_init(&style_normal1);
    lv_style_set_text_color(&style_normal1, lv_color_black());
    lv_style_set_text_font(&style_normal1, info.font);

    lv_span_t * span;
    bool is_first = false;

    size_t pos = sentence.sentence.find(m_search_word);
    if(pos == std::string::npos){
        std::string str = m_search_word;
        str.front() = std::toupper(str.front());
        pos = sentence.sentence.find(str);
        is_first = true;
    }
    if(is_first){
    std::string str = m_search_word;
    str.front() = std::toupper(str.front());
    span = lv_spangroup_new_span(span_cont);
    lv_span_set_text(span, str.c_str());
    span->style = style_highlight;
    if(pos+m_search_word.size() < sentence.sentence.size()){
    span = lv_spangroup_new_span(span_cont);
    lv_span_set_text(span, sentence.sentence.substr(pos+m_search_word.size(),sentence.sentence.size()).c_str());
    span->style = style_normal;
    }
    }else{
    if(pos != 0){
    span = lv_spangroup_new_span(span_cont);
    lv_span_set_text(span, sentence.sentence.substr(0,pos).c_str());
    span->style = style_normal;
    }
    span = lv_spangroup_new_span(span_cont);
    lv_span_set_text(span, m_search_word.c_str());
    span->style = style_highlight;
    if(pos+m_search_word.size() < sentence.sentence.size()){
    span = lv_spangroup_new_span(span_cont);
    lv_span_set_text(span, sentence.sentence.substr(pos+m_search_word.size(),sentence.sentence.size()).c_str());
    span->style = style_normal1;
    }
    }
    
    

    // lv_obj_t* label3 = lv_label_create(sentence_win);
    // lv_obj_add_flag(label3,LV_OBJ_FLAG_HIDDEN);

    // lv_obj_align_to(label3,speak_context3, LV_ALIGN_OUT_RIGHT_MID, 10, -5);
    // lv_label_set_cjk_font(label3, 18, FT_FONT_STYLE_NORMAL);
    // lv_label_set_long_mode(label3, LV_LABEL_LONG_WRAP);
    // lv_label_set_text(label3, orignal.c_str());
    // lv_refr_now(NULL);
    // if(lv_obj_get_width(label3) > SCREEN_WIDTH - 80){
    //     lv_obj_set_width(label3, SCREEN_WIDTH - 80);
    //     lv_label_set_long_mode(label3, LV_LABEL_LONG_WRAP);
    //     lv_label_set_text(label3, orignal.c_str());
    // }
    // lv_obj_set_style_text_color(label3, lv_color_black(), 0);
    // lv_obj_clear_flag(label3,LV_OBJ_FLAG_HIDDEN);
    // lv_refr_now(NULL);
    
    
    lv_obj_t* label4 = lv_label_create(sentence_win);
    lv_obj_add_flag(label4,LV_OBJ_FLAG_HIDDEN);
    lv_obj_align_to(label4,span_cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_label_set_cjk_font(label4, 18, FT_FONT_STYLE_NORMAL);
    lv_label_set_text(label4, translation.c_str());
    lv_refr_now(NULL);
    if(lv_obj_get_width(label4) > SCREEN_WIDTH - 80){
        lv_obj_set_width(label4, SCREEN_WIDTH - 80);
        lv_label_set_long_mode(label4, LV_LABEL_LONG_WRAP);
        lv_label_set_text(label4, translation.c_str());
    }
    lv_obj_set_style_text_color(label4, lv_color_hex(0x787F87), 0);
    lv_obj_clear_flag(label4,LV_OBJ_FLAG_HIDDEN);
    lv_refr_now(NULL);
    
    int w1 = lv_obj_get_height(span_cont);
    int w2 = lv_obj_get_height(label4);

    lv_obj_set_size(sentence_win, SCREEN_WIDTH, 40 + w1 + w2);
    lv_obj_clear_flag(trans_win,LV_OBJ_FLAG_HIDDEN);
    

    sys_setCurMainWinAndCb(mainBg,AP_MainWin_scr_cb);
	mwin_BgWin = mainBg;
	return 0;
}


static void _speak_out_big_click_cb(lv_event_t * evt){
    
}
int YdDictionary::AP_FollowWin_init(){
	mwin_CurWin = YdDictionary_FOLLOW;
	mwin_IsInSubItem = true;
	lv_obj_t* mainBg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mainBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainBg, 0, 0);
	lv_obj_set_style_pad_all(mainBg, 0, 0);
	lv_color_t colorBg = lv_color_hex(0xffffff);
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
	lv_label_set_text(lblTitle, "跟读");
	lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
	lv_obj_align(lblTitle, LV_ALIGN_CENTER, 0,0);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), 0);


    lv_obj_t* speak_out_img = lv_img_create(mainBg);
    lv_img_set_src(speak_out_img, &speak_out_big);
    lv_obj_align(speak_out_img, LV_ALIGN_CENTER, 0, -100);
    lv_obj_add_flag(speak_out_img,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(speak_out_img, _speak_out_big_click_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* label = lv_label_create(mainBg);
    lv_label_set_text(label, m_search_word.c_str());
    lv_label_set_cjk_font(label, 20, FT_FONT_STYLE_NORMAL);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -10);



    sys_setCurMainWinAndCb(mainBg,AP_MainWin_scr_cb);
	mwin_BgWin = mainBg;
	return 0;
}


int YdDictionary::AP_DisWin_init(){
	mwin_CurWin = YdDictionary_DIS;
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

	sys_setCurMainWinAndCb(mainBg,AP_MainWin_scr_cb);
	mwin_BgWin = mainBg;
	return 0;
}



int YdDictionary::AP_MainWin_handler(lv_event_code_t msg, int param){
    LV_LOG("%s,[%d]\r\n ", __FUNCTION__,__LINE__);
	switch ((int)msg) 
	{
		case C_EVENT_FREE_AP_RES:
			LV_LOG("%s,[%d] , mwin_CurWin: %d \r\n ", __FUNCTION__,__LINE__, mwin_CurWin);
			if(g_yd_dictionary != nullptr){
				delete g_yd_dictionary;
				g_yd_dictionary = nullptr;
			}
			break;
		default:
			break;
	}
	return 0;
}

template<typename REQ>
std::string get_sign(const REQ& req,const std::string& q,const std::string& secret) {
    std::string input = q;
    size_t len = utf8::distance(q.begin(), q.end());

    if (len > 20) {
        auto front_end = q.begin();
        utf8::advance(front_end, 10, q.end());
        std::string first_part(q.begin(), front_end);

        auto back_start = q.begin();
        utf8::advance(back_start, len - 10, q.end());
        std::string last_part(back_start, q.end());
        input = first_part
              + std::to_string(len) 
              + last_part;
    }
    std::string signStr = req.appKey
                        + input
                        + req.salt
                        + req.curtime
                        + secret;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, signStr.c_str(), signStr.size());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::setw(2) << static_cast<unsigned int>(hash[i]);
    }

    return ss.str();
}
void YdDictionary::Ap_send_dictionnary_request(std::string& word,bool is_en,std::function<void()> callback){
    DictionnaryRequest req;
    req.q = word;
    req.appKey = DICTIONNARY_KEY;
    req.signType = "v3";
    req.curtime = get_current_time();
    req.salt = get_random_salt();
    req.sign = get_sign<DictionnaryRequest>(req,req.q,DICTIONNARY_SECRET);
    req.langType = "auto";
    if(is_en){
        req.dicts = "ec";
    }else{
        req.dicts = "ce";
    }
    req.docType = "json";
    send_request_async<DictionnaryRequest,DictionaryResponse>(req,send_Dictionary_request,g_DictionaryRes,callback);
}
void YdDictionary::Ap_tts_gen(const std::string& q,const std::string& language,
    std::function<void()> callback){
        TtsRequest req;
    req.q = q+"a";
    std::cout<<"input q = "<< req.q<<std::endl;
    req.appKey = TTS_APP_KEY;
    req.signType = u8"v3";
    req.curtime = get_current_time();
    req.salt = get_random_salt();
    req.sign = get_sign<TtsRequest>(req,req.q,TTS_APP_SECRET);

    req.q = url_encode(req.q);
    req.speed = u8"1.0";
    req.volume = u8"1.0"; 
    req.format = u8"mp3";
    std::string encoded_voice = (language == "en") ? u8"youxiaomei" : u8"youxiaoxun";
    req.voiceName = encoded_voice;
    send_request_async<TtsRequest,TtsResponse>
                    (req,send_Tts_request,g_TtsRes,callback);
}

void YdDictionary::Ap_MainWin_clean_res(){}
int YdDictionary::AP_MainWin_deinit(){
    Ap_MainWin_del();
    Ap_MainWin_clean_res();
    return 0;
}
void YdDictionary::Ap_MainWin_del(){
    tz_play_scheduling_stop_audio();
	if (mwin_BgWin) {
		lv_obj_del(mwin_BgWin);  mwin_BgWin = NULL;
	}
}
static void AP_MainWin_scr_cb(lv_event_t* evt){
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
static void AP_esc_click_cb(lv_event_t * evt){
    switch (g_yd_dictionary->get_cur_win())
	{
		case YdDictionary_DIS:
		case YdDictionary_MAIN:
		zb_requestSwitchAp(C_ZB_AP_MAINMENU);
			break;
		case YdDictionary_SEARCH:
        g_yd_dictionary->Ap_MainWin_del();
        g_yd_dictionary->AP_MainWin_init();
			break;
		case YdDictionary_CONTENT:
        speak_icon_blink_stop();
        g_yd_dictionary->Ap_MainWin_del();
        g_yd_dictionary->AP_SearchWin_init();
			break;
		case YdDictionary_FOLLOW:
        g_yd_dictionary->Ap_MainWin_del();
        g_yd_dictionary->AP_ContentWin_init();
			break;
		default: 
			break;
	}
    return;
}

int YdDictionary_ap_enter(){
    if(g_yd_dictionary){
       delete g_yd_dictionary;
       g_yd_dictionary = nullptr;
    } 
	g_yd_dictionary = new YdDictionary();
		if(get_is_net_connected())
			g_yd_dictionary->AP_MainWin_init();
        else 
        g_yd_dictionary->AP_DisWin_init();
	return 0;
}

extern "C" int YdDictionary_ap_init(void){
    YdDictionary_ap_enter();
    return 0;
}
extern "C" int YdDictionary_ap_handler(lv_event_code_t msg, int param){
    return g_yd_dictionary->AP_MainWin_handler(msg, param);
}