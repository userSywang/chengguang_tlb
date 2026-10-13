
extern int main_menu_ap_init(void);
extern int main_menu_ap_cb(lv_event_code_t msg, int param);

extern int book_sel_ap_init(void);
extern int book_sel_ap_handler(lv_event_code_t msg, int param);

extern int zb_ap_my_download_select_init(void);
extern int zb_ap_my_download_hanlder(lv_event_code_t msg, int param);

extern int listenai_child_chat_init(void);  //儿童聊天 
extern int listenai_child_chat_handler(lv_event_code_t msg, int param);  //

extern int tch_mtrl_eng_ap_init(void);
extern int tch_mtrl_eng_ap_handler(lv_event_code_t msg, int param);
extern int tch_mtrl_cn_ap_init(void);
extern int tch_mtrl_cn_ap_handler(lv_event_code_t msg, int param);

extern int zb_example_ap_a_init(void);
extern int zb_example_ap_a_cb(lv_event_code_t msg, int param);

extern int zb_example_ap_b_init(void);
extern int zb_example_ap_b_cb(lv_event_code_t msg, int param);

extern int zb_example_ap_img_init(void);
extern int zb_example_ap_img_cb(lv_event_code_t msg, int param);

extern int zb_example_ap_img_dec_init(void);
extern int zb_example_ap_img_dec_cb(lv_event_code_t msg, int param);

extern int zb_example_ap_aud_init(void);
extern int zb_example_ap_aud_hndlr(lv_event_code_t msg, int param);

extern int zb_example_ap_mh_init(void);
extern int zb_example_ap_mh_handler(lv_event_code_t msg, int param);

extern int zb_ap_web_disk_resources_init(void);
extern int zb_ap_web_disk_resources_hanlder(lv_event_code_t msg, int param);

extern int zb_ap_ear_plan_init(void);
extern int zb_example_ap_ear_plan_hanlder(lv_event_code_t msg, int param);

extern int zb_example_ap_phonics_type_init(void);
extern int zb_example_ap_phonics_hanlder(lv_event_code_t msg, int param);

extern int zb_example_ap_ucc_init(void);
extern int zb_example_ap_ucc_hanlder(lv_event_code_t msg, int param);
//------------------------------

extern int zb_prodTst_ap_init(void);
extern int zb_prodTest_ap_handler(lv_event_code_t msg, int param);
	
extern int zb_setting_ap_init(void);
extern int zb_setting_ap_handler(lv_event_code_t msg, int param);

extern int zb_codeUpdateByFile_ap_init(void);
extern int zb_codeUpdateByFile_ap_handler(lv_event_code_t msg, int param);

extern int prpin_ap_init(void);
extern int prpin_ap_handler(lv_event_code_t msg, int param);

void listenai_oral_entry(void);
void listenai_oral_exit(void);

void listenai_child_chat_entry(void);  //儿童聊天 
void listenai_child_chat_exit(void);  //

extern void zb_pinyin_ap_init(void);
extern int zb_ap_pinyin_study_handler(lv_event_code_t msg, int param);

void zb_search_ap_init(void);
int zb_search_ap_handler(lv_event_code_t msg, int param);

//------------- 
extern void collect_screen_init(void);
extern int PageResource_collect(lv_event_code_t msg, int param);

extern void recentlyPlayed_Screen_init(void);
extern int PageResource_recently(lv_event_code_t msg, int param);
extern void one_group(void);

extern int wrd_ap_init(void);
extern int wrd_ap_handler(lv_event_code_t msg, int param);

extern int contact_ap_init(void);
extern int contact_ap_handler(lv_event_code_t msg, int param);


extern int PageResource_Stuword(lv_event_code_t msg, int param);

extern int ximaLY_menu_ap_init(void);
extern int ximaLY_menu_ap_cb(void);

extern int zb_ap_aud_player_ap_init(void);
extern int zb_ap_aud_player_ap_handler(lv_event_code_t msg, int param);

extern int zb_ap_textbook_ap_init(void);
extern int zb_ap_textbook_ap_handler(lv_event_code_t msg, int param);

extern int dic_ap_init(void);
extern int dic_ap_handler(lv_event_code_t msg, int param);

extern void MMN_player_enter(void);
extern int MMN_player_page_init();
extern int play_text_win_free_keep();

extern void alipay_page_init_manager(void);
extern void alipay_page_free_all(void);

/**
 * AP 定义表，在 misc_utils.h 宏定义，在 mainmenu.c 绑定宏
 * Note：该表各项必须按 C_ZB_AP_MAINMENU 等一组常量的值来排列 
 */
static const struct zb_app_info g_zb_appInfoTbl[] = {
    { main_menu_ap_init, main_menu_ap_cb }, 
    { MMN_player_enter, play_text_win_free_keep},//播放器
    { zb_setting_ap_init, zb_setting_ap_handler },//设置
    { NULL, NULL },
    { contact_ap_init, contact_ap_handler },//通讯录
    { NULL, NULL },
    { NULL, NULL },
    { zb_ap_my_download_select_init, zb_ap_my_download_hanlder },//我的下载    
    { NULL, NULL },//{ listenai_child_chat_init, listenai_child_chat_handler },//儿童聊天
    { NULL, NULL },
	{ collect_screen_init,PageResource_collect },
    { NULL, NULL },
	{ NULL, NULL },
    { NULL, NULL },
	{ alipay_page_init_manager,alipay_page_free_all},//15
    { NULL, NULL },
	{ NULL, NULL },
    { NULL, NULL },
	{ NULL, NULL },
    { NULL, NULL },
	{ zb_ap_aud_player_ap_init, zb_ap_aud_player_ap_handler },//21
    { tch_mtrl_eng_ap_init, tch_mtrl_eng_ap_handler },
    { tch_mtrl_cn_ap_init, tch_mtrl_cn_ap_handler },
    { NULL, NULL },
	{ NULL, NULL },
    { NULL, NULL },
#if 0
    { zb_prodTst_ap_init, zb_prodTest_ap_handler },
    { apDemo_Widgets_init, apSimple_handler }, // debug 
    { zb_example_ap_a_init, zb_example_ap_a_cb },
    { zb_example_ap_aud_init, zb_example_ap_aud_hndlr },
    { book_sel_ap_init, book_sel_ap_handler },
    { listenai_oral_entry, listenai_oral_exit },
    { listenai_child_chat_entry, listenai_child_chat_exit },
    { zb_pinyin_ap_init, zb_ap_pinyin_study_handler },
    { collect_screen_init,PageResource_collect  },
    { recentlyPlayed_Screen_init,PageResource_recently  },
    { one_group, PageResource_Stuword },
    { zb_search_ap_init, zb_search_ap_handler },  // 搜索 
    { ximaLY_menu_ap_init, ximaLY_menu_ap_cb }, //喜马拉雅 ximaLY_menu_ap_init
    { zb_ap_web_disk_resources_init, zb_ap_web_disk_resources_hanlder},//百度网盘 31

    { zb_example_ap_ucc_init, zb_example_ap_ucc_hanlder },        //认识汉字——》识汉字的年级   17  zb_example_ap_ucc_grade_init
    { zb_example_ap_phonics_type_init, zb_example_ap_phonics_hanlder},            //自然拼读 18 zb_example_ap_phonics_tone_type_init
    { zb_ap_ear_plan_init, zb_example_ap_ear_plan_hanlder },//磨耳朵计划      19
    { zb_ap_aud_player_ap_init, zb_ap_aud_player_ap_handler },  // 21
    { zb_ap_textbook_ap_init, zb_ap_textbook_ap_handler },
    { zb_codeUpdateByFile_ap_init, zb_codeUpdateByFile_ap_handler },
    { prpin_ap_init, prpin_ap_handler },
    { dic_ap_init, dic_ap_handler },
    { wrd_ap_init, wrd_ap_handler },
    //{ NULL, NULL },
#endif    
};


