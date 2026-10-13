extern "C" {
    #include "../data_parse/img_res.h"
	#include "../data_parse/img_std_res.h"
    #include "cpp_api_bridge.h"
    #include "../mainmenu/mainmenu_base.h"
	#include "../data_support/yd_book_select.h"
	#include "../player/player.h"
}
#include "online_res.h" 


#include "../online_tool/http_get_online_res.h" 
#include "../online_tool/global_http_tool.h"
#include <vector>
#include <iostream>

//global
static OnlinePlayAp* g_online_play_ap = nullptr;

void AP_MainWin_scr_cb(lv_event_t* evt);
static void _setting_btn_click_cb(lv_event_t * evt);
static void _down_btn_click_cb(lv_event_t * evt);
static void AP_esc_click_cb(lv_event_t * evt);

static bool PlayApIsOnline(uint8_t ap_sel){
	return ap_sel < IS_ONLINE;
}
static bool PlayApIsTeachMtl(uint8_t ap_sel){
	return ap_sel > IS_TEACH_MTL;
}

//main win
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
//用于指示哪些模块的哪些图书正处于下载中 value用于更新ui进度 在退出mainwin时要置空 重新进入是重新赋值
static uint8_t last_ap_sel = 0;
static uint8_t last_book_index = 0;
static uint8_t last_media_index = 0;
static std::vector<std::map<int,lv_obj_t*>> g_download_list(ONLINE_PLAY_AP_SUN);

static void _item_online_click_handler(lv_event_t * e) {
    size_t index = (size_t)lv_event_get_user_data(e);
	OnlineResNode& node = (*g_resNodeList.get())[index];
	std::string _name = node.name;
	std::string _path = node.path;
	 
	async_get_online_res_from_path(_path,[node,_name,index](){
		if( g_mediaNodeList == nullptr ) return;
        g_online_play_ap->Ap_MainWin_del();
		g_online_play_ap->set_content_win(get_online_img_by_url(node.coverImg),_name,node,index);
		g_online_play_ap->AP_ContentWin_init();
    });
}

static void _item_offline_click_handler(lv_event_t * e) {
    if (e->code == LV_EVENT_CLICKED) {
        size_t index = (size_t)lv_event_get_user_data(e);
		if(!g_resNodeList || index >= g_resNodeList->size()) {
            std::cerr << "Invalid resNodeList or index!" << std::endl;
            return;
        }
		std::string _name = (*g_resNodeList.get())[index].name;
		std::string part_path = "";
		if(PlayApIsTeachMtl(g_online_play_ap->mwin_ItemSel)){
			part_path = "/userdata/_data/textbook/"+(*g_resNodeList.get())[index].path+"/book000";
		}else{
			part_path = "/oem/_data/textbook/"+(*g_resNodeList.get())[index].path;
		}
		std::string _path = part_path + "/album.mnu";
		std::string _cover = (*g_resNodeList.get())[index].coverImg;
		//todo 获取img
		lv_img_dsc_t* img_Src = nullptr;
		if(PlayApIsTeachMtl(g_online_play_ap->mwin_ItemSel)){
			img_Src = get_tch_img_by_path(_cover);
		}else{
			img_Src = get_offline_img_by_ap_index(g_online_play_ap->mwin_ItemSel,index);
		}
		 
		
		player_book_info_t pmit; 
		player_get_book_inf(const_cast<char*>(_path.c_str()),&pmit);
		if(g_mediaNodeList == nullptr){
			g_mediaNodeList = std::make_unique<std::vector<OnlineMediaNode>>();
		}
		for(int i = 0 ; i < pmit.cnt ; i++){
			g_mediaNodeList.get()->emplace_back(OnlineMediaNode{
				pmit.pTile[i],
				part_path,
				"",
				"",
				true,
				""
			});
		}
		g_online_play_ap->Ap_MainWin_del();
		g_online_play_ap->set_content_win(img_Src,_name,(*g_resNodeList.get())[index],index);
		g_online_play_ap->AP_ContentWin_init();
    }
}
static void _item_download_click_handler(lv_event_t * e) {
    if (e->code == LV_EVENT_CLICKED) {
		size_t index = (size_t)lv_event_get_user_data(e);
		int ap_sel = g_online_play_ap->mwin_ItemSel; 
		lv_obj_t* item = lv_event_get_target(e);

		g_download_list[ap_sel][index] = item;
        lv_obj_t* img = lv_obj_get_child(item, 0);

		lv_obj_t* download_btn = lv_obj_get_child(img, 0);
		lv_obj_del(download_btn);

		lv_obj_clear_flag(item, LV_OBJ_FLAG_CLICKABLE);
		lv_obj_set_style_img_recolor_opa(img, LV_OPA_60, 0);
        lv_obj_t* bar = lv_bar_create(img);
        lv_obj_set_size(bar, 100, 10);
        lv_obj_align(bar, LV_ALIGN_CENTER, 0, -10);
		lv_obj_set_style_opa(bar, LV_OPA_100, 0);
        lv_bar_set_value(bar, 0, LV_ANIM_OFF);

		lv_obj_t* label = lv_label_create(img);
        lv_label_set_text(label, "正在下载");
		lv_label_set_cjk_font(label, 16, FT_FONT_STYLE_NORMAL);
        lv_obj_align_to(label ,bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
		lv_obj_set_style_opa(label, LV_OPA_100, 0);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);

		extern int get_cg_net_url(char* part_path, char** net_url);
        
		std::string _name = (*g_resNodeList.get())[index].name;
		std::string _path = (*g_resNodeList.get())[index].path;
		std::string _localPath = (*g_resNodeList.get())[index].local_path;
		char* part_path = (char*)_path.c_str();
		char* net_url = (char*)malloc(256);
		get_cg_net_url(part_path, &net_url);
		std::string _mediaUrl = net_url;
		std::cout<<"_path:"<<_path<<std::endl;
		std::cout<<"_mediaUrl:"<<_mediaUrl<<std::endl;
		std::cout<<"_localPath:"<<_localPath<<std::endl;
		free(net_url);


		size_t lastSlash = _localPath.rfind('/');
		my_mkdir(_localPath.substr(0, lastSlash).c_str());

		async_download_http_request(_mediaUrl,_localPath,
			[=](int progress, curl_off_t speed){
				lv_obj_t* item = g_download_list[ap_sel][index];
				if(g_online_play_ap == nullptr ||
					g_online_play_ap->get_cur_win() != ONLINE_PALY_MAIN ||
					ap_sel != g_online_play_ap->mwin_ItemSel ||
					item == nullptr) return;
				lv_obj_t* img = lv_obj_get_child(item, 0);
				lv_obj_t* bar = lv_obj_get_child(img, 0);
				lv_obj_t* label = lv_obj_get_child(img, 1);
				lv_bar_set_value(bar, progress, LV_ANIM_ON);
			},
			[=](bool success){
				lv_obj_t* item = g_download_list[ap_sel][index];
				g_download_list[ap_sel].erase(index);
				if(success){
					yd_split_cg_book(_localPath.c_str(),nullptr);
				}
				if(g_online_play_ap == nullptr ||
					g_online_play_ap->get_cur_win() != ONLINE_PALY_MAIN ||
					ap_sel != g_online_play_ap->mwin_ItemSel ||
					item == nullptr) return;
				lv_obj_t* img = lv_obj_get_child(item, 0);
				lv_obj_t* bar = lv_obj_get_child(img, 0);
				lv_obj_t* label = lv_obj_get_child(img, 1);
				lv_obj_del(bar);
				lv_obj_del(label);
				lv_obj_set_style_img_recolor_opa(img, LV_OPA_0, 0);
				if(success){
					(*g_resNodeList.get())[index].isDownlaoded = true;
					lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
					lv_obj_remove_event_cb(item, _item_download_click_handler); // 新增：移除旧回
					lv_obj_add_event_cb(item, _item_offline_click_handler, LV_EVENT_CLICKED,(void*)index);
				}else{
					(*g_resNodeList.get())[index].isDownlaoded = false;
					lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
					lv_obj_t* down_img = lv_img_create(img);
					lv_img_set_src(down_img, &download_img);
					lv_obj_set_size(down_img, download_img.header.w, download_img.header.h);
					lv_obj_align(down_img, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
				}
			}
		);
    }
}
int OnlinePlayAp::AP_MainWin_init(){
    mwin_CurWin = ONLINE_PALY_MAIN;
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
	lv_label_set_text(lblTitle, g_ApNodeList[mwin_ItemSel].name.c_str());
	lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
	lv_obj_align(lblTitle, LV_ALIGN_CENTER, 0,0);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), 0);

	if(PlayApIsTeachMtl(mwin_ItemSel)){
		lv_obj_t* switch_btn_con = lv_obj_create(top_title_win);
		lv_img_dsc_t* switch_btn_img = zb_img_res_load_one(NULL, TEXT_BOOK_ICON_IMG_SWITCH, IMG_FREE_TYPE_AUTO, NULL);
		lv_obj_set_size(switch_btn_con, switch_btn_img->header.w + TOP_BTN_BORDER * 2 + 14, switch_btn_img->header.h + TOP_BTN_BORDER * 2 + 10);
		lv_obj_align(switch_btn_con, LV_ALIGN_RIGHT_MID, -12, 2);
		lv_obj_set_style_border_width(switch_btn_con, 0, 0);
    	lv_obj_t * switch_btn = lv_img_create(switch_btn_con);
		lv_img_set_src(switch_btn, switch_btn_img);
		lv_obj_set_size(switch_btn, switch_btn_img->header.w + TOP_BTN_BORDER * 2, switch_btn_img->header.h + TOP_BTN_BORDER * 2);
		lv_obj_align(switch_btn, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_style_pad_all(switch_btn, TOP_BTN_BORDER, 0);
		lv_obj_set_style_radius(switch_btn, 4, 0);
		lv_obj_add_flag(switch_btn, LV_OBJ_FLAG_CLICKABLE);

		lv_color_t colorBg = lv_color_hex(0xc0c0c0);
		lv_obj_set_style_bg_color(switch_btn, colorBg, 0);
		lv_obj_set_style_bg_opa(switch_btn, LV_OPA_0, 0);
		lv_obj_set_style_bg_opa(switch_btn, LV_OPA_100, LV_STATE_FOCUS_KEY);
		lv_obj_set_style_bg_opa(switch_btn, LV_OPA_10, LV_STATE_PRESSED);
	
		lv_color_t colorBorderFcs = lv_color_white();
		lv_obj_set_style_border_color(switch_btn, colorBorderFcs, 0);
		lv_obj_add_event_cb(switch_btn, _setting_btn_click_cb, LV_EVENT_CLICKED, NULL);
	}
    
    lv_obj_t * lst_win = lv_obj_create(mainBg);
    lv_obj_set_size(lst_win, LV_PCT(100), SCREEN_HEIGHT - ONLINEAP_HEADER_HEIGHT);
    lv_obj_align_to(lst_win, top_title_win, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
	lv_obj_set_style_border_width(lst_win, 0,0);
	lv_obj_set_scrollbar_mode(lst_win, LV_SCROLLBAR_MODE_OFF);  // 关闭滚动条

	lv_obj_set_flex_flow(lst_win, LV_FLEX_FLOW_ROW_WRAP);			// 允许自动换行
	lv_obj_set_style_pad_top(lst_win, 0, 0);
	lv_obj_set_style_pad_left(lst_win, 15, 0);
	lv_obj_set_style_pad_right(lst_win, 15, 0);
	lv_obj_set_style_pad_row(lst_win, 5, 0);						// 行间距固定 20px
	lv_obj_set_style_pad_column(lst_win, 10, 0);
    

    if ( g_resNodeList == nullptr) AP_esc_click_cb(nullptr);
    for (size_t i = 0; i < g_resNodeList.get()->size(); i++){
		OnlineResNode& node = (*g_resNodeList.get())[i];
		lv_obj_t *item = lv_obj_create(lst_win);
		lv_obj_set_size(item, (SCREEN_WIDTH - 20) / 2 - 10, 215);			// 宽度自适应容器，高度固定160px
		lv_obj_set_style_radius(item, 18, 0);		// 圆角
		lv_obj_set_style_border_width(item, 0, 0);
		lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
		lv_obj_set_scroll_dir(item, LV_DIR_NONE);
		//lv_obj_set_style_bg_color(item, lv_color_hex(0xd3d3d3), LV_STATE_PRESSED);

		lv_obj_t *img = lv_img_create(item);
		lv_obj_set_size(img, 140, 162);
		lv_obj_set_style_radius(img, 18, LV_PART_MAIN);
		lv_obj_set_style_clip_corner(img, 18, LV_PART_MAIN);
		lv_img_dsc_t* img_Src = nullptr;
		if(node.isChenGuang){
			if(PlayApIsTeachMtl(mwin_ItemSel)){
				img_Src = get_tch_img_by_path(node.coverImg);
			}else{
				img_Src = get_offline_img_by_ap_index(mwin_ItemSel,i);
			}
		}else{
			img_Src = get_online_img_by_url(node.coverImg);
		}
		lv_img_set_src(img,img_Src);
		lv_obj_set_style_img_opa(img, LV_OPA_70, LV_STATE_PRESSED);

		lv_coord_t max_width = 140;  // 按钮宽度
		lv_coord_t max_height = 80;   // 两行高度（45按钮高度 + 35间距）
		lv_point_t text_size;
		const char* bookName = node.name.c_str();
		
		lv_obj_t* label_con = lv_obj_create(item);
		lv_obj_set_size(label_con, 140, 53);
		lv_obj_align(label_con, LV_ALIGN_BOTTOM_MID, 0, 0);
		lv_obj_set_style_border_width(label_con, 0, 0);
		lv_obj_set_scroll_dir(label_con, LV_DIR_NONE);
		lv_obj_set_style_opa(label_con, LV_OPA_100, 0);
		//lv_obj_set_style_bg_color(label_con, lv_color_hex(0xd3d3d3), LV_STATE_PRESSED);


		lv_obj_t *label = lv_label_create(label_con);
		lv_label_set_cjk_font(label, 18, FT_FONT_STYLE_NORMAL);
		lv_obj_set_width(label,140);
		lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);  // 文本水平居中
		lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
		size_t max_chars = 15;
       	size_t char_count = 0;
		char name_buf[500] = { 0 };
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
		lv_label_set_text(label, bookName);
		lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
		lv_obj_align_to(label,img, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);
		if(node.isChenGuang){
			if(node.isDownlaoded){
				lv_obj_add_event_cb(item, _item_offline_click_handler, LV_EVENT_CLICKED,(void*)i);
			}else{
				// if(g_download_list[mwin_ItemSel].find(i) != g_download_list[mwin_ItemSel].end()){
				// 	lv_obj_clear_flag(item, LV_OBJ_FLAG_CLICKABLE);
				// 	lv_obj_set_style_img_recolor_opa(img, LV_OPA_60, 0);
				// 	lv_obj_t* bar = lv_bar_create(img);
				// 	lv_obj_set_size(bar, 100, 10);
				// 	lv_obj_align(bar, LV_ALIGN_CENTER, 0, -10);
				// 	lv_obj_set_style_opa(bar, LV_OPA_100, 0);
				// 	lv_bar_set_value(bar, 0, LV_ANIM_OFF);
			
				// 	lv_obj_t* label = lv_label_create(img);
				// 	lv_label_set_text(label, "正在下载");
				// 	lv_label_set_cjk_font(label, 16, FT_FONT_STYLE_NORMAL);
				// 	lv_obj_align_to(label ,bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
				// 	lv_obj_set_style_opa(label, LV_OPA_100, 0);
				// 	lv_obj_set_style_text_color(label, lv_color_white(), 0);
				// 	g_download_list[mwin_ItemSel][i] = item;
				// }else{
					lv_obj_t* down_img = lv_img_create(img);
					lv_img_set_src(down_img, &download_img);
					lv_obj_set_size(down_img, download_img.header.w, download_img.header.h);
					lv_obj_align(down_img, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
					lv_obj_add_event_cb(item, _item_download_click_handler, LV_EVENT_CLICKED, (void*)i);
				//}
			}
		}else{
			lv_obj_add_event_cb(item, _item_online_click_handler, LV_EVENT_CLICKED,(void*)i);
		}
	}

    sys_setCurMainWinAndCb(mainBg,AP_MainWin_scr_cb);
	mwin_BgWin = mainBg;
	printf("online_play_ap_init end\n");
	return 0;
}

//content win
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

static lv_style_t style_btn_content;
static lv_obj_t* last_fouce_btn = nullptr;
lv_obj_t* res_list_win = nullptr;
extern int play_album_page_init(char* path, uint16_t play_num, int16_t ap_num);
static void play_media_by_index(int index){
	last_ap_sel = g_online_play_ap->mwin_ItemSel;
	last_book_index = g_online_play_ap->m_sel_res_index;
	last_media_index = index;
	auto& mediaNode = g_mediaNodeList.get()->at(index);
	if(mediaNode.mediaUrl == ""){
		mediaNode.path = mediaNode.path;
		char* mediaPath = (char*)mediaNode.path.c_str();
		play_album_page_init(mediaPath,index,0);
	}else{
		// std::vector<uint8_t> sub_data;
   			// if(!download_http_request_buf(_sub, sub_data) || sub_data.empty()) {
        	// 	ESP_LOGE("IMG", "下载失败: %s", _sub.c_str());
			// 	return;
    		// }
			// SimpleLrcParser parser;
			// if (!parser.loadFromBuffer(sub_data)) {
			// 	ESP_LOGE("IMG", "解析失败: %s", _sub.c_str());
			// 	return;
			// }

			// for(auto& entry : parser.entries){
			// 	std::cout<<"time:"<<entry.time<<std::endl;
			// 	std::cout<<"content_fir:"<<entry.content_fir<<std::endl;
			// 	std::cout<<"content_sec:"<<entry.content_sec<<std::endl;
			// }

		Playlist playlist = { 0 };
        playlist.count = g_mediaNodeList.get()->size();
        playlist.items = (PlaylistItem*)malloc(g_mediaNodeList.get()->size() * sizeof(PlaylistItem));
		if (!playlist.items) {
			LV_LOG_ERROR("Failed to allocate playlist items");
			return;
		}
		
		for(int i =0;i<g_mediaNodeList.get()->size();i++){
			playlist.items[i].source_type = MUSIC_SOURCE_YOUDAO;
			std::string play_local = "";
			if(g_mediaNodeList.get()->at(i).isDownlaoded){
				play_local=g_mediaNodeList.get()->at(i).local_path+"/"+g_mediaNodeList.get()->at(i).name;
			}else{
				play_local=g_mediaNodeList.get()->at(i).mediaUrl;
			}
			playlist.items[i].location =  strdup(const_cast<char*>(play_local.c_str()));
			playlist.items[i].title =  strdup(const_cast<char*>(g_mediaNodeList.get()->at(i).name.c_str()));
			playlist.items[i].cover = 0; 
			playlist.items[i].playNum = i;
		}            
	 
		int result = play_album_online_page_init(&playlist, index, C_ZB_AP_COLLECT, false);
	 
		for (uint32_t i = 0; i < playlist.count; i++) {
			free(playlist.items[i].location);
			free(playlist.items[i].title);
		}
		free(playlist.items);
	 
		if (result != 0) {
			LV_LOG_ERROR("Play initialization failed: %d", result);
		}
	}
	return;
}
static void _Online_res_item_btn_click_cb(lv_event_t* evt);
static void updata_content_list(){
	if(res_list_win == nullptr) return;
	lv_obj_clean(res_list_win);
	lv_img_dsc_t* selected_img = zb_img_res_load_one(NULL, TEXT_BOOK_ICON_IMG_VIDEOPLAYING, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_dsc_t* new_img = zb_img_res_load_one(NULL, TEXT_BOOK_ICON_IMG_VIDEONOPLAYING, IMG_FREE_TYPE_AUTO, NULL);


	for (int i = 0; i < g_mediaNodeList.get()->size(); i++) {
		auto& node = (*g_mediaNodeList.get())[i];
		std::string Namestr = node.name;
		if (Namestr.length() >= 4) {
		    const std::string suffix = Namestr.substr(Namestr.length() - 4);
		    if (suffix == ".mp3" || suffix == ".MP3") {
		        Namestr.resize(Namestr.length() - 4);
		    }
		}

		const char* text = Namestr.c_str();
		lv_obj_t* btn = lv_btn_create(res_list_win);
		lv_obj_set_size(btn, 300, 50);
		lv_obj_add_style(btn, &style_btn_content, 0);

		lv_obj_t* img = lv_img_create(btn);
		lv_img_dsc_t* img_img = new_img;
		if(last_ap_sel == g_online_play_ap->mwin_ItemSel&&
			last_book_index == g_online_play_ap->m_sel_res_index&&
			i == last_media_index)
			img_img = selected_img;
		lv_img_set_src(img, img_img);
		lv_obj_set_size(img, img_img->header.w, img_img->header.h);
		lv_obj_align(img, LV_ALIGN_LEFT_MID, 0, 0);

		lv_obj_t * label = lv_label_create(btn);
		lv_label_set_cjk_font(label, 18, FT_FONT_STYLE_NORMAL);
    	lv_label_set_text(label, text);
    	lv_obj_align_to(label, img, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
		// lv_obj_set_style_bg_opa(btn, LV_OPA_50, LV_EVENT_PRESSING); 
		
		lv_obj_add_event_cb(btn, _Online_res_item_btn_click_cb, LV_EVENT_CLICKED, (void*)i);
	}
}
static void play_all_btn_clickCb(lv_event_t* evt) {
	if(last_ap_sel == g_online_play_ap->mwin_ItemSel&&
		last_book_index == g_online_play_ap->m_sel_res_index){
		play_media_by_index(last_media_index);
	} else{
		play_media_by_index(0);
	}
	updata_content_list();
	last_fouce_btn = lv_obj_get_child(res_list_win,last_media_index);
	return;
}

static void _Online_res_item_btn_click_cb(lv_event_t* evt) {
	int index = (int)lv_event_get_user_data(evt);
	play_media_by_index(index);
	updata_content_list();
	last_fouce_btn = lv_obj_get_child(res_list_win,last_media_index);
	return;
}

int OnlinePlayAp::AP_ContentWin_init(){
	mwin_CurWin = ONLINE_PALY_CONTENT;
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

	char name_buf[500] = { 0 };
	char* bookName = (char*)m_name.c_str();
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

	Ap_createEscBtn(top_title_win, AP_esc_click_cb, 0);
	lv_obj_t * lblTitle = lv_label_create(top_title_win);
	lv_label_set_text(lblTitle, bookName);
	lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
	lv_obj_align(lblTitle, LV_ALIGN_CENTER, 0,0);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), 0);

	if(!m_sel_res_node.isChenGuang){
		lv_obj_t* download_content = lv_obj_create(top_title_win);
		lv_obj_set_size(download_content, 60, 60);
		lv_obj_align(download_content, LV_ALIGN_TOP_RIGHT, -6, 0);
		lv_obj_set_style_border_width(download_content, 0, 0);
		lv_obj_add_flag(download_content, LV_OBJ_FLAG_CLICKABLE);

		lv_obj_t* download_btn = lv_img_create(download_content);
		lv_img_dsc_t* download_btn_img = zb_img_res_load_one(NULL, TEXT_BOOK_ICON_IMG_ICONDOWNLOAD, IMG_FREE_TYPE_AUTO, NULL);
		lv_img_set_src(download_btn, download_btn_img);
		lv_obj_set_size(download_btn, download_btn_img->header.w + TOP_BTN_BORDER * 2, download_btn_img->header.h + TOP_BTN_BORDER * 2);
		lv_obj_align(download_btn, LV_ALIGN_CENTER, 5, 0);
		lv_obj_set_style_pad_all(download_btn, TOP_BTN_BORDER, 0);
		lv_obj_set_style_radius(download_btn, 4, 0);

		lv_obj_set_style_bg_color(download_btn, lv_color_hex(0xc0c0c0), 0);
		lv_obj_set_style_bg_opa(download_btn, LV_OPA_0, 0);
		lv_obj_set_style_bg_opa(download_btn, LV_OPA_100, LV_STATE_FOCUS_KEY);
		lv_obj_set_style_bg_opa(download_btn, LV_OPA_10, LV_STATE_PRESSED);
		lv_obj_set_style_border_color(download_btn, lv_color_white(), 0);
		lv_obj_add_event_cb(download_content, _down_btn_click_cb, LV_EVENT_CLICKED, NULL);
	}

	lv_obj_t* img_show_win = NULL;

		img_show_win = lv_obj_create(mainBg);
		lv_obj_set_size(img_show_win, LV_PCT(100), 150);
		lv_obj_align_to(img_show_win, top_title_win, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
		lv_obj_set_style_bg_color(img_show_win, main_colorBg, 0);
		lv_obj_set_style_border_width(img_show_win, 0, 0);
		lv_obj_set_style_pad_all(img_show_win, 0, 0);
		lv_obj_set_style_pad_row(img_show_win, 0, 0);
		lv_obj_set_scroll_dir(img_show_win, LV_DIR_NONE);

		lv_obj_t* text_img = lv_img_create(img_show_win);
		lv_img_set_src(text_img, m_img_Src);
		lv_obj_align(text_img, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_style_pad_all(text_img,0, 0);
		lv_obj_set_style_radius(text_img, 18, 0);
		lv_obj_set_style_clip_corner(text_img, 18, LV_PART_MAIN);

	lv_obj_t* play_all_win = lv_obj_create(mainBg);
	lv_obj_set_size(play_all_win, LV_PCT(100), 50);
	lv_obj_align_to(play_all_win, img_show_win, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
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
	lv_obj_set_style_border_color(play_all_btn, lv_color_white(), 0);
	lv_obj_add_event_cb(play_all_btn, play_all_btn_clickCb, LV_EVENT_CLICKED, NULL);

	lv_obj_t* play_all_title = lv_label_create(play_all_win);
	lv_label_set_cjk_font(play_all_title, 18, FT_FONT_STYLE_NORMAL);
	lv_obj_align_to(play_all_title, play_all_btn, LV_ALIGN_OUT_RIGHT_MID, 3, 0);
	lv_label_set_text(play_all_title, "继续播放");
	lv_obj_set_style_text_color(play_all_title, lv_color_black(), 0);

	lv_obj_t* count_title = lv_label_create(play_all_win);
	lv_label_set_cjk_font(count_title, 18, FT_FONT_STYLE_NORMAL);
	lv_obj_align_to(count_title, play_all_title, LV_ALIGN_OUT_RIGHT_MID, 1, 0);
    lv_label_set_text(count_title, ("("+std::to_string(g_mediaNodeList->size())+")").c_str());
	lv_obj_set_style_text_color(count_title, lv_color_hex(0x787F87), 0);

	res_list_win = lv_obj_create(mainBg);
	lv_obj_set_size(res_list_win, LV_PCT(100), SCREEN_HEIGHT - 260);	//todo 260 为顶部标题栏高度 + 底部播放按钮高度
	lv_obj_align_to(res_list_win,play_all_win, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
	lv_obj_set_style_bg_color(res_list_win, main_colorBg, 0);
	lv_obj_set_style_border_width(res_list_win, 0,0);
	lv_obj_set_scrollbar_mode(res_list_win, LV_SCROLLBAR_MODE_OFF);  // 关闭滚动条

	lv_obj_set_flex_flow(res_list_win, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(res_list_win, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_top(res_list_win, 4, 0);
	lv_obj_set_style_pad_left(res_list_win, 15, 0);
	lv_obj_set_style_pad_right(res_list_win, 15, 0);
	lv_obj_set_style_pad_row(res_list_win, 15, 0);
	lv_obj_set_style_pad_column(res_list_win, 10, 0);

	static bool isFirst = true;
	if (isFirst) {
	isFirst = false;
	lv_style_init(&style_btn_content);
    lv_style_set_bg_color(&style_btn_content, lv_color_hex(0xF5F5F5)); // 未选中时浅灰色背景
    lv_style_set_bg_opa(&style_btn_content, LV_OPA_100);
    lv_style_set_text_color(&style_btn_content, lv_color_hex(0x000000)); // 黑色字体 #000000
    lv_style_set_radius(&style_btn_content, 10);                         // 16px圆角
    lv_style_set_pad_hor(&style_btn_content, 20);                        // 水平内边距
    lv_style_set_pad_ver(&style_btn_content, 6);
    lv_style_set_border_width(&style_btn_content, 0); // 无边框
    lv_style_set_shadow_width(&style_btn_content, 0); // 无阴影
	}


	updata_content_list();

	sys_setCurMainWinAndCb(mainBg,AP_MainWin_scr_cb);
	mwin_BgWin = mainBg;
	return 0;
}


//disconnect win
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

int OnlinePlayAp::AP_DisWin_init(){
	std::cout<<"AP_DisWin_init start"<<std::endl;
	mwin_CurWin = ONLINE_PALY_DIS;
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

//download win
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static std::map<int,bool> g_downloadSel;
lv_obj_t* ItemSelectedTitle = nullptr;
lv_obj_t* ContentListWin = nullptr;

static void _download_item_selected(lv_obj_t* img){
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
static void _download_item_unselected(lv_obj_t* img){
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
void updata_count_title(){
	int count = 0;
	for(int i = 0; i < g_downloadSel.size(); i++){
		if(g_downloadSel[i]) count++;
	}
	if(ItemSelectedTitle!= nullptr){
		lv_label_set_text(ItemSelectedTitle, ("下载("+std::to_string(count)+")").c_str());
	}
}
static void all_sel_btn_clickCb(lv_event_t* e){
	bool all_sel_flag = false;
	int count = 0;
	for(int i = 0; i < g_downloadSel.size(); i++){
		if(g_downloadSel[i]) count++;
	}
	if(g_downloadSel.size() == count) all_sel_flag = false; //全不选
	else all_sel_flag = true; //全选
	
    for(uint32_t i = 0; i < g_downloadSel.size(); i++) {
        lv_obj_t* child = lv_obj_get_child(ContentListWin, i);
        lv_obj_t* img = lv_obj_get_child(child, 0);
        if(all_sel_flag) _download_item_selected(img);
		else _download_item_unselected(img);
		g_downloadSel[i] = all_sel_flag;
    }
	updata_count_title();
    return;
}
static void _Online_res_download_comform_click_cb(lv_event_t* e){
	for(int i = 0; i < g_downloadSel.size(); i++){
		if(g_downloadSel[i]){
			my_mkdir((*g_mediaNodeList.get())[i].local_path.c_str());
			async_download_http_request(
				(*g_mediaNodeList.get())[i].mediaUrl,
				(*g_mediaNodeList.get())[i].local_path +"/"+ (*g_mediaNodeList.get())[i].name,
				[](int progress,curl_off_t speed){
					std::cout<<"media progress:"<<progress<<std::endl;
				},
				[=](bool success){
					if(success) {
						std::cout<<"media download success"<<std::endl;
						(*g_mediaNodeList.get())[i].isDownlaoded = true;
					}else{
						std::cout<<"media download failed"<<std::endl;
					}
				}
			);
			async_download_http_request(
				(*g_mediaNodeList.get())[i].SubtitleUrl,
				(*g_mediaNodeList.get())[i].local_path +"/"+ (*g_mediaNodeList.get())[i].name + ".lrc",
				[](int progress,curl_off_t speed){
					std::cout<<"zh progress:"<<progress<<std::endl;
				},
				[](bool success){
					if(success) {
						std::cout<<"zh download success"<<std::endl;
					}else{
						std::cout<<"zh download failed"<<std::endl;
					}
				}
			);
		}
	}
	AP_esc_click_cb(nullptr);
}
static void _Online_res_download_item_btn_click_cb(lv_event_t* e){
    lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* img = lv_obj_get_child(btn, 0);
	int index = (int)lv_event_get_user_data(e);
    if(g_downloadSel[index]) {
        g_downloadSel[index]=false;
        _download_item_unselected(img);
    } else {
        g_downloadSel[index]=true;
        _download_item_selected(img);
    }
	updata_count_title();
}

int OnlinePlayAp::AP_DownWin_init(){
	std::cout<<"AP_DownWin_init start"<<std::endl;
	mwin_CurWin = ONLINE_PALY_DOWN;
	mwin_IsInSubItem = true;

	g_downloadSel.clear();
	ItemSelectedTitle = nullptr;
	ContentListWin = nullptr;

	lv_obj_t * mainBg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mainBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainBg, 0, 0);
	lv_obj_set_style_pad_all(mainBg, 0, 0);
	lv_color_t main_colorBg = lv_color_hex(AP_BG_COLOR_HEX);
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

    Ap_createEscBtn(top_title_win, AP_esc_click_cb, 0);
	lv_obj_t * lblTitle = lv_label_create(top_title_win);
    lv_label_set_text(lblTitle, "批量下载");
	lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
	lv_obj_align(lblTitle, LV_ALIGN_CENTER, 0,0);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), 0);

    lv_obj_t * all_sel_btn = lv_obj_create(top_title_win);
	lv_obj_set_size(all_sel_btn, 80, 60);
	lv_obj_align(all_sel_btn, LV_ALIGN_TOP_RIGHT, -6, 0);
	lv_obj_set_style_pad_all(all_sel_btn, TOP_BTN_BORDER, 0);
	lv_obj_set_style_radius(all_sel_btn, 4, 0);
	lv_obj_add_flag(all_sel_btn, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_set_scroll_dir(all_sel_btn, LV_DIR_NONE);

    lv_obj_t* all_sel_title = lv_label_create(all_sel_btn);
    lv_label_set_text(all_sel_title, "全选");
	lv_label_set_cjk_font(all_sel_title, 18, FT_FONT_STYLE_NORMAL);
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

	ContentListWin = lv_obj_create(mainBg);
	lv_obj_set_size(ContentListWin, LV_PCT(100), SCREEN_HEIGHT - 130);	//todo 260 为顶部标题栏高度 + 底部播放按钮高度
	lv_obj_align_to(ContentListWin,top_title_win, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
	lv_obj_set_style_bg_color(ContentListWin, main_colorBg, 0);
	lv_obj_set_style_border_width(ContentListWin, 0,0);
	lv_obj_set_scrollbar_mode(ContentListWin, LV_SCROLLBAR_MODE_OFF);  // 关闭滚动条

	lv_obj_set_flex_flow(ContentListWin, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(ContentListWin, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_top(ContentListWin, 0, 0);
	lv_obj_set_style_pad_left(ContentListWin, 15, 0);
	lv_obj_set_style_pad_right(ContentListWin, 15, 0);
	lv_obj_set_style_pad_row(ContentListWin, 15, 0);						// 行间距固定 20px
	lv_obj_set_style_pad_column(ContentListWin, 10, 0);

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
	
	for (int i = 0; i < g_mediaNodeList.get()->size() ; i++){
		auto& node = (*g_mediaNodeList.get())[i];
		if(node.isDownlaoded) continue;
		lv_obj_t* btn = lv_btn_create(ContentListWin);
		lv_obj_set_size(btn, 300, 75);
		lv_obj_add_style(btn, &style_btn, 0);

		lv_obj_t* img = lv_img_create(btn);
		_download_item_unselected(img);

		lv_obj_t * label = lv_label_create(btn);
		lv_obj_set_width(label, 220);
		lv_label_set_cjk_font(label, 18, FT_FONT_STYLE_NORMAL);
		lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    	lv_label_set_text(label, node.name.c_str());
    	lv_obj_align(label, LV_ALIGN_LEFT_MID, 2, 0);
		lv_obj_add_event_cb(btn, _Online_res_download_item_btn_click_cb, LV_EVENT_CLICKED, (void*)i);
		g_downloadSel.insert(std::make_pair(i,false));
	}

	lv_obj_t *conform_btn = lv_btn_create(mainBg);
	lv_obj_set_size(conform_btn, 300, 45);
	lv_obj_align_to(conform_btn, ContentListWin, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_set_style_radius(conform_btn, 25, 0);
	lv_obj_set_style_bg_color(conform_btn, lv_color_hex(0x008CFF), 0);
	ItemSelectedTitle = lv_label_create(conform_btn);
    lv_label_set_text(ItemSelectedTitle, "下载(0)");
	lv_label_set_cjk_font(ItemSelectedTitle, 18, FT_FONT_STYLE_NORMAL);
    lv_obj_center(ItemSelectedTitle);
	lv_obj_set_style_shadow_width(conform_btn, 0, 0);
    lv_obj_add_event_cb(conform_btn, _Online_res_download_comform_click_cb, LV_EVENT_CLICKED,NULL);
	sys_setCurMainWinAndCb(mainBg,AP_MainWin_scr_cb);
	mwin_BgWin = mainBg;
	std::cout<<"AP_DownWin_init end"<<std::endl;
	return 0;
}

//setting win
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static e_stage g_grade_sel[2] = { STAGE_PUPIL, STAGE_PUPIL};
static e_stage temp_grade_sel[2] = { STAGE_PUPIL, STAGE_PUPIL};
static uint8_t g_now_subject = 0;
static std::string version_name[2] = {"",""};
static uint8_t g_version_sel[2] = {0,0};
static uint8_t temp_version_sel[2][3] = {0};
lv_obj_t* publish_sel_win = nullptr;
static lv_style_t style_btn;
static lv_style_t age_btn_selected_style;
const char* c_grades[3] = {"小学","初中","高中"};

e_book_genre ap_sel_to_enum(uint8_t ap_sel);
e_stage grade_sel_to_enum(uint8_t grade_sel);
static void updata_publish_sel_win();
void read_offline_dir(uint8_t ap_sel);

static void _publisher_btn_click_cb(lv_event_t* e){
	lv_obj_t* btn = lv_event_get_target(e);
	int index = (int)lv_event_get_user_data(e);
	lv_obj_t* last_btn = lv_obj_get_child(publish_sel_win,temp_version_sel[g_now_subject][temp_grade_sel[g_now_subject]]);
	if(last_btn){
		lv_obj_remove_style(last_btn, &age_btn_selected_style, 0);
		lv_obj_add_style(last_btn, &style_btn, 0);
	}
	if(btn){
		lv_obj_remove_style(btn, &style_btn, 0);
		lv_obj_add_style(btn, &age_btn_selected_style, 0);
	}
	temp_version_sel[g_now_subject][temp_grade_sel[g_now_subject]] = index;
}

static void updata_publish_sel_win(){
	lv_obj_clean(publish_sel_win);
	e_book_genre ap_enum = ap_sel_to_enum(g_online_play_ap->mwin_ItemSel);
	uint16_t version_num = 0;
	const char** version_list = yd_read_versions(ap_enum,temp_grade_sel[g_now_subject], &version_num);
	if (version_num > 0) {
		for (int i = 0; i < version_num; i++) {
			lv_obj_t* publisher_tn = lv_btn_create(publish_sel_win);
			if(i==g_version_sel[g_now_subject]&&temp_grade_sel[g_now_subject]==g_grade_sel[g_now_subject]||
				temp_grade_sel[g_now_subject]!=g_grade_sel[g_now_subject]&&i==0){
				lv_obj_add_style(publisher_tn, &age_btn_selected_style, 0);
			}else{
				lv_obj_add_style(publisher_tn, &style_btn, 0);
			}
			lv_obj_set_size(publisher_tn, 300, 45);
			lv_obj_t* label = lv_label_create(publisher_tn);
			lv_label_set_cjk_font(label, 18, FT_FONT_STYLE_NORMAL);
			lv_label_set_text(label, version_list[i]);
			lv_obj_center(label);
			lv_obj_add_event_cb(publisher_tn, _publisher_btn_click_cb, LV_EVENT_CLICKED, (void*)i);
		}
	} 
	yd_book_versions_free();
}
static void _grade_btn_click_cb(lv_event_t* e){
	lv_obj_t* btn = lv_event_get_target(e);
	int index = (int)lv_event_get_user_data(e);
	temp_grade_sel[g_now_subject] = grade_sel_to_enum(index);
	lv_obj_t* btn_win = lv_obj_get_parent(btn);
	uint8_t grade_count = 0;
	if(g_online_play_ap->mwin_ItemSel == ONLINE_PLAY_AP_YINYU){
		grade_count = 3;
	}else{
		grade_count = 2;
	}

	for(int i = 0; i < grade_count; i++){
		lv_obj_t* child = lv_obj_get_child(btn_win, i);
		if(i == index){
			lv_obj_remove_style(child, &style_btn, 0);
			lv_obj_add_style(child, &age_btn_selected_style, 0);
		}else{
			lv_obj_remove_style(child, &age_btn_selected_style, 0);
			lv_obj_add_style(child, &style_btn, 0);
		} 
	}
	updata_publish_sel_win();
}

static void _cfg_comform_click_cb(lv_event_t* e){
	lv_obj_t* btn = lv_obj_get_child(publish_sel_win,temp_version_sel[g_now_subject][temp_grade_sel[g_now_subject]]);
	if(!btn) return;
	lv_obj_t* btn_label = lv_obj_get_child(btn,0);
	version_name[g_now_subject] = lv_label_get_text(btn_label);
	g_grade_sel[g_now_subject] = temp_grade_sel[g_now_subject];
	g_version_sel[g_now_subject] = temp_version_sel[g_now_subject][temp_grade_sel[g_now_subject]];
	if(g_resNodeList){
		g_resNodeList.reset();
		g_resNodeList = nullptr;
	}
	read_offline_dir(g_online_play_ap->mwin_ItemSel);
	read_offline_dir(g_online_play_ap->mwin_ItemSel);
	AP_esc_click_cb(nullptr);
}

int OnlinePlayAp::AP_SettingWin_init(){
	mwin_CurWin = ONLINE_PALY_SETTEING;
	mwin_IsInSubItem = true;

	publish_sel_win = nullptr;

	lv_obj_t * mainBg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mainBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainBg, 0, 0);
	lv_obj_set_style_pad_all(mainBg, 0, 0);
	lv_color_t main_colorBg = lv_color_hex(AP_BG_COLOR_HEX);
	lv_obj_set_style_bg_color(mainBg, main_colorBg, 0);
	lv_obj_set_scroll_dir(mainBg, LV_DIR_NONE);

    lv_obj_t * top_title_win = lv_obj_create(mainBg);
    lv_obj_set_size(top_title_win, LV_PCT(100), 60); // 60
    lv_obj_align(top_title_win, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(top_title_win, main_colorBg, 0);
    lv_obj_set_style_border_width(top_title_win, 0, 0);
	lv_obj_set_style_pad_all(top_title_win, 0, 0);
	lv_obj_set_style_pad_row(top_title_win, 0, 0);
	lv_obj_set_style_pad_column(top_title_win, 0, 0);
	lv_obj_set_scroll_dir(top_title_win, LV_DIR_NONE);

	Ap_createEscBtn(top_title_win, AP_esc_click_cb, 0);
	lv_obj_t * lblTitle = lv_label_create(top_title_win);
	lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
    lv_label_set_text(lblTitle, "选择学段");
	lv_obj_center(lblTitle);
	lv_obj_set_style_text_color(lblTitle, lv_color_black(), 0);

	lv_obj_t* title1 = lv_label_create(mainBg);
	lv_label_set_cjk_font(title1, 18, FT_FONT_STYLE_NORMAL);
	lv_label_set_text(title1, "选择学习阶段");
	lv_obj_align_to(title1,top_title_win, LV_ALIGN_BOTTOM_LEFT, 8, 20);
	lv_obj_set_style_text_color(title1, lv_color_hex(0x808080), 0);

	lv_obj_t* grade_sel_win = lv_obj_create(mainBg);
	lv_obj_set_size(grade_sel_win, LV_PCT(100), 120);
	lv_obj_align_to(grade_sel_win, title1, LV_ALIGN_TOP_LEFT, -8, 25);
	lv_obj_set_style_border_width(grade_sel_win, 0, 0);
	lv_obj_set_style_pad_all(grade_sel_win, 0, 0);
	lv_obj_set_style_pad_row(grade_sel_win, 0, 0);
	lv_obj_set_style_pad_column(grade_sel_win, 0, 0);
	lv_obj_set_scroll_dir(grade_sel_win, LV_DIR_NONE);
	static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(grade_sel_win, col_dsc, row_dsc);

	static bool isFirst = true;
	if (isFirst) {
	isFirst = false;
		lv_style_init(&style_btn);
		lv_style_set_bg_color(&style_btn, lv_color_hex(0xF5F5F5)); // 未选中时浅灰色背景
		lv_style_set_bg_opa(&style_btn, LV_OPA_100);
		lv_style_set_text_color(&style_btn, lv_color_hex(0x000000)); // 黑色字体 #333333
		lv_style_set_radius(&style_btn, 10);                         // 16px圆角
		lv_style_set_pad_hor(&style_btn, 20);                        // 水平内边距
		lv_style_set_pad_ver(&style_btn, 6);
		lv_style_set_border_width(&style_btn, 0); // 无边框
		lv_style_set_shadow_width(&style_btn, 0); // 无阴影

		lv_style_init(&age_btn_selected_style);
		lv_style_set_bg_color(&age_btn_selected_style, lv_color_hex(0xE6F0FF));     // 浅蓝色背景 #E6F0FF
		lv_style_set_text_color(&age_btn_selected_style, lv_color_hex(0x1A5FB4));   // 深蓝字体 #1A5FB4
		lv_style_set_border_width(&age_btn_selected_style, 0); 
		lv_style_set_radius(&age_btn_selected_style, 10);                         // 16px圆角
		lv_style_set_pad_hor(&age_btn_selected_style, 20);                        // 水平内边距
		lv_style_set_pad_ver(&age_btn_selected_style, 6);
		lv_style_set_shadow_width(&age_btn_selected_style, 0); // 无阴影
	}
	
	uint8_t grade_count = 0;
	if(g_online_play_ap->mwin_ItemSel == ONLINE_PLAY_AP_YINYU){
		grade_count = 3;
	}else{
		grade_count = 2;
	}

	for(int i = 0; i < grade_count; i++){
		lv_obj_t* grade_btn = lv_btn_create(grade_sel_win);
		if(i!=g_grade_sel[g_now_subject]) lv_obj_add_style(grade_btn, &style_btn, 0);
		else lv_obj_add_style(grade_btn, &age_btn_selected_style, 0);
		lv_obj_set_size(grade_btn, 140, 45);
		lv_obj_t * label = lv_label_create(grade_btn);
		lv_label_set_cjk_font(label, 18, FT_FONT_STYLE_NORMAL);
		lv_label_set_text(label, c_grades[i]);
    	lv_obj_center(label);
		lv_obj_set_grid_cell(grade_btn,
            LV_GRID_ALIGN_CENTER, i%2, 1,
            LV_GRID_ALIGN_CENTER, i/2, 1
        );
        lv_obj_add_event_cb(grade_btn, _grade_btn_click_cb, LV_EVENT_CLICKED, (void*)i);
	}

	lv_obj_t* title2 = lv_label_create(mainBg);
	lv_label_set_cjk_font(title2, 18, FT_FONT_STYLE_NORMAL);
	lv_label_set_text(title2, "选择版本");
	lv_obj_align_to(title2,grade_sel_win, LV_ALIGN_BOTTOM_LEFT, 8, 25);
	lv_obj_set_style_text_color(title2, lv_color_hex(0x808080), 0);

	publish_sel_win = lv_obj_create(mainBg);
	lv_obj_set_size(publish_sel_win, LV_PCT(100), 168);
	lv_obj_align_to(publish_sel_win, title2, LV_ALIGN_TOP_LEFT, -8, 25);
	lv_obj_set_style_border_width(publish_sel_win, 0, 0);
	lv_obj_set_style_pad_all(publish_sel_win, 0, 0);
	lv_obj_set_style_pad_row(publish_sel_win, 10, 0);
	lv_obj_set_style_pad_column(publish_sel_win, 0, 0);
	lv_obj_set_style_pad_top(publish_sel_win, 10, 0);
	lv_obj_set_style_pad_bottom(publish_sel_win, 10, 0);
	lv_obj_set_scroll_dir(publish_sel_win, LV_DIR_VER);

	lv_obj_set_flex_flow(publish_sel_win, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(publish_sel_win, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_border_opa(publish_sel_win, LV_OPA_0, 0);
	lv_obj_set_style_bg_opa(publish_sel_win, LV_OPA_0, 0);

	updata_publish_sel_win();

	lv_obj_t* conform_cont = lv_obj_create(mainBg);
	lv_obj_set_size(conform_cont, 320, 80);
	lv_obj_align(conform_cont,LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_set_style_border_width(conform_cont, 0, 0);
	lv_obj_add_flag(conform_cont, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_t *conform_btn = lv_btn_create(conform_cont);
	lv_obj_set_size(conform_btn, 300, 45);
	lv_obj_align_to(conform_btn, publish_sel_win, LV_ALIGN_BOTTOM_MID, 0, 80);
	lv_obj_set_style_bg_color(conform_btn, lv_color_hex(0x008CFF), 0);
	lv_obj_set_style_radius(conform_btn, 25, 0);
	lv_obj_t * label = lv_label_create(conform_btn);
	lv_label_set_cjk_font(label, 18, FT_FONT_STYLE_NORMAL);
	lv_label_set_text(label, "确定");
    lv_obj_center(label);
	lv_obj_set_style_shadow_width(conform_btn, 0, 0);
    lv_obj_add_event_cb(conform_cont, _cfg_comform_click_cb, LV_EVENT_CLICKED,NULL);
	lv_obj_add_event_cb(conform_btn, _cfg_comform_click_cb, LV_EVENT_CLICKED,NULL);

	sys_setCurMainWinAndCb(mainBg,AP_MainWin_scr_cb);
	mwin_BgWin = mainBg;
	return 0;
}


static void _setting_btn_click_cb(lv_event_t * evt){
    g_online_play_ap->Ap_MainWin_del();
    g_online_play_ap->AP_SettingWin_init();
}
static void _down_btn_click_cb(lv_event_t * evt){
    g_online_play_ap->Ap_MainWin_del();
    g_online_play_ap->AP_DownWin_init();
}

void OnlinePlayAp::Ap_back_content(void) {
	sys_setCurMainWinAndCb(mwin_BgWin, AP_MainWin_scr_cb);
	lv_indev_keypad_wait_release();
	lv_group_focus_obj(last_fouce_btn);
}

void OnlinePlayAp::Ap_MainWin_clean_res(){
}
static void AP_esc_click_cb(lv_event_t * evt){
    switch (g_online_play_ap->get_cur_win())
	{
		case ONLINE_PALY_DIS:
		case ONLINE_PALY_MAIN:
			if(g_resNodeList){
				g_resNodeList.reset();
				g_resNodeList = nullptr;
			}
			zb_requestSwitchAp(C_ZB_AP_MAINMENU);
			break;
		case ONLINE_PALY_SETTEING:
			temp_grade_sel[g_now_subject] = g_grade_sel[g_now_subject];
			for(int i=0;i<3;i++){
				if(i!=g_grade_sel[g_now_subject]){
					temp_version_sel[g_now_subject][i] = 0;
				}else{
					temp_version_sel[g_now_subject][i] = g_version_sel[g_now_subject];
				}
			}
			g_online_play_ap->Ap_MainWin_del();
			g_online_play_ap->AP_MainWin_init();
			break;
		case ONLINE_PALY_CONTENT:
			if(g_mediaNodeList){
				g_mediaNodeList.reset();
				g_mediaNodeList = nullptr;
			}
			g_online_play_ap->Ap_MainWin_del();
			g_online_play_ap->AP_MainWin_init();
			break;
		case ONLINE_PALY_DOWN:
			g_online_play_ap->Ap_MainWin_del();
			g_online_play_ap->AP_ContentWin_init();
			break;
		default: 
			break;
	}
    return;
}
int OnlinePlayAp::AP_MainWin_deinit(){
    Ap_MainWin_del();
    Ap_MainWin_clean_res();
    return 0;
}
void OnlinePlayAp::Ap_MainWin_del(){
	if(mwin_CurWin == ONLINE_PALY_MAIN){
		for(auto& node:g_download_list[mwin_ItemSel]){
			node.second = nullptr;
		}
	}
	if (mwin_BgWin) {
		lv_obj_del(mwin_BgWin);  mwin_BgWin = NULL;
	}
}
int OnlinePlayAp::AP_MainWin_handler(lv_event_code_t msg, int param) {
    LV_LOG("%s,[%d]\r\n ", __FUNCTION__,__LINE__);
	switch ((int)msg) 
	{
		case C_EVENT_FREE_AP_RES:
			LV_LOG("%s,[%d] , mwin_CurWin: %d \r\n ", __FUNCTION__,__LINE__, mwin_CurWin);
			if(g_online_play_ap != nullptr){
				delete g_online_play_ap;
				g_online_play_ap = nullptr;
			}
			break;
		case C_EVENT_EX_CMD: {
			if (param == PLY_CMD_PLY_LST_RET) {
				switch (mwin_CurWin)
				{
					case ONLINE_PALY_CONTENT:
						Ap_back_content();
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
void AP_MainWin_scr_cb(lv_event_t* evt) {
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

e_book_genre ap_sel_to_enum(uint8_t ap_sel){
	switch(ap_sel){
		case ONLINE_PLAY_AP_YINYU: return BOOK_ENG;
		case ONLINE_PLAY_AP_YUWEN: return BOOK_CHN;
		case ONLINE_PLAY_AP_EGE: return BOOK_SONG;
		case ONLINE_PLAY_AP_QINGJING: return BOOK_SITU_DIALOG;
		case ONLINE_PLAY_AP_GUOXUE: return BOOK_CHINA_CULT;
		case ONLINE_PLAY_AP_GUSHI: return BOOK_ANCI_POETRY;
		default: return BOOK_ERR;
	}
}

e_stage grade_sel_to_enum(uint8_t grade_sel){
	switch(grade_sel){
		case 0: return STAGE_PUPIL;
		case 1: return STAGE_JUNIOR;
		case 2: return STAGE_SENIOR;
	}
}

void read_offline_dir(uint8_t ap_sel){
	e_book_genre ap_enum = ap_sel_to_enum(ap_sel);
	uint16_t book_num = 0;
	yd_bk_node* bknode = nullptr;
	if(ap_enum == BOOK_ENG){
		g_now_subject = 0;
		if(version_name[g_now_subject]==""){
			bknode = yd_read_books(ap_enum,g_grade_sel[g_now_subject],"人教PEP版",&book_num);
		} else{
			bknode = yd_read_books(ap_enum,g_grade_sel[g_now_subject],version_name[g_now_subject].c_str(),&book_num);
		} 
	}else if(ap_enum == BOOK_CHN){
		g_now_subject = 1;
		if(version_name[g_now_subject]=="")bknode = yd_read_books(ap_enum,g_grade_sel[g_now_subject],"人教统编版",&book_num);
		else bknode = yd_read_books(ap_enum,g_grade_sel[g_now_subject],version_name[g_now_subject].c_str(),&book_num);
	}else{
		bknode = yd_read_books(ap_enum,STAGE_PUPIL,"",&book_num);
	}
	std::cout<<"book_num: "<<book_num<<std::endl;
	int count = 0;
	if(g_resNodeList == nullptr){
		g_resNodeList = std::make_unique<std::vector<OnlineResNode>>();
	}
	while(bknode){
		std::string name = bknode->data.name;
		std::string path = bknode->data.book_url;
		std::string cover_url = bknode->data.cover_url;
		cover_url = "/oem/_data/textbook/" + cover_url + ".bin";
		char* local_path = (char*)malloc(500);
		bool isdown;
		if(!PlayApIsTeachMtl(ap_sel)){
			isdown = check_res_down(RES_CG_BUILDIN,path.c_str(),&local_path,count++,false);
		}else{
			isdown = check_res_down(RES_CG_ONLINE,path.c_str(),&local_path,0,false);
		}
		std::cout<<"name: "<<name<<", path: "<<path<<", isdown: "<<isdown <<"cover"<<cover_url<<std::endl;
		std::string local_path_str = local_path;
		free(local_path);
		
		g_resNodeList.get()->emplace_back(OnlineResNode{name ,path,cover_url, true,"", isdown,local_path_str});
		bknode = bknode->next;
	}
	yd_book_node_free();
}


int OnlinePlayAp_ap_init(uint8_t ap_sel){
	if(g_resNodeList){
		g_resNodeList.reset();
		g_resNodeList = nullptr;
	}
	if(g_mediaNodeList){
		g_mediaNodeList.reset();
		g_mediaNodeList = nullptr;
	}

	g_online_play_ap = new OnlinePlayAp(ap_sel);
	if(PlayApIsOnline(ap_sel)){
		if(get_is_net_connected()){
			async_get_online_dir_list(ap_sel,[ap_sel](){
				if(g_resNodeList == nullptr){
					g_online_play_ap->AP_DisWin_init();
				}else{
					read_offline_dir(ap_sel);
					g_online_play_ap->AP_MainWin_init();
				}
			});
		}else g_online_play_ap->AP_DisWin_init();
	}else{
		read_offline_dir(ap_sel);
		g_online_play_ap->AP_MainWin_init();
	}
	return 0;
}


extern "C" int online_play_jianqiao_ap_init(void){
	OnlinePlayAp_ap_init(ONLINE_PLAY_AP_JIANQIAO);
    return 0;
}
extern "C" int online_play_jianqiao_ap_handler(lv_event_code_t msg, int param){
    return g_online_play_ap->AP_MainWin_handler(msg, param);
}
extern "C" int online_play_waijiao_ap_init(void){
    OnlinePlayAp_ap_init(ONLINE_PLAY_AP_WAIJIAO);
    return 0;
}
extern "C" int online_play_waijiao_ap_handler(lv_event_code_t msg, int param){
    return g_online_play_ap->AP_MainWin_handler(msg, param);
}
extern "C" int online_play_yinyuminzhu_ap_init(void){
    OnlinePlayAp_ap_init(ONLINE_PLAY_AP_YINGYUMINZHU);
    return 0;
}
extern "C" int online_play_yinyuminzhu_ap_handler(lv_event_code_t msg, int param){
    return g_online_play_ap->AP_MainWin_handler(msg, param);
}
extern "C" int online_play_dayuwen_ap_init(void){
    OnlinePlayAp_ap_init(ONLINE_PLAY_AP_DAYUWEN);
    return 0;
}
extern "C" int online_play_dayuwen_ap_handler(lv_event_code_t msg, int param){
    return g_online_play_ap->AP_MainWin_handler(msg, param);
}
extern "C" int online_play_kewaigushi_ap_init(void){
    OnlinePlayAp_ap_init(ONLINE_PLAY_AP_KEWAIGUSHI);
    return 0;
}
extern "C" int online_play_kewaigushi_ap_handler(lv_event_code_t msg, int param){
    return g_online_play_ap->AP_MainWin_handler(msg, param);
}
extern "C" int online_play_gushisuitin_ap_init(void){
	OnlinePlayAp_ap_init(ONLINE_PLAY_AP_GUSHI);
    return 0;
}
extern "C" int online_play_gushisuitin_ap_handler(lv_event_code_t msg, int param){
	return g_online_play_ap->AP_MainWin_handler(msg, param);
}
extern "C" int online_play_ege_ap_init(void){
	OnlinePlayAp_ap_init(ONLINE_PLAY_AP_EGE);
    return 0;
}
extern "C" int online_play_ege_ap_handler(lv_event_code_t msg, int param){
    return g_online_play_ap->AP_MainWin_handler(msg, param);
}
extern "C" int online_play_qingjing_ap_init(void){
    OnlinePlayAp_ap_init(ONLINE_PLAY_AP_QINGJING);
    return 0;
}
extern "C" int online_play_qingjing_ap_handler(lv_event_code_t msg, int param){
    return g_online_play_ap->AP_MainWin_handler(msg, param);
}
extern "C" int online_play_guoxuexuntin_ap_init(void){
	OnlinePlayAp_ap_init(ONLINE_PLAY_AP_GUOXUE);
	return 0;
}
extern "C" int online_play_guoxuexuntin_ap_handler(lv_event_code_t msg, int param){
	return g_online_play_ap->AP_MainWin_handler(msg, param);
}
extern "C" int online_play_yuwenjiaocai_ap_init(void){
	OnlinePlayAp_ap_init(ONLINE_PLAY_AP_YUWEN);
	return 0;
}
extern "C" int online_play_yuwenjiaocai_ap_handler(lv_event_code_t msg, int param){
	return g_online_play_ap->AP_MainWin_handler(msg, param);
}
extern "C" int online_play_yinyujiaocai_ap_init(void){
	OnlinePlayAp_ap_init(ONLINE_PLAY_AP_YINYU);
	return 0;
}
extern "C" int online_play_yinyujiaocai_ap_handler(lv_event_code_t msg, int param){
	return g_online_play_ap->AP_MainWin_handler(msg, param);
}
extern "C" int online_play_fenjiyuedu_ap_init(void){
	OnlinePlayAp_ap_init(ONLINE_PLAY_AP_FENJI);
    return 0;
}
extern "C" int online_play_fenjiyuedu_ap_handler(lv_event_code_t msg, int param){
    return g_online_play_ap->AP_MainWin_handler(msg,param);
}