/*******************************************************
                    书籍显示页面程序段
******************************************************/

#include "player_main.h"
#include "../zb_base.h"
#include "../ex_widget/app_msg.h"

#define TAG "plyMain:"

extern int16_t ap_toggle_num;

static struct LSNSPK_MENU* class_menu = NULL;
player_class_locate_t player_class_locate = { 0 };
int player_class_num = -1;  // 学习的素材类型， 值为 CHINESE_TEXTBOOK 
uint8_t play_main_clicked_btn_num = 0;
uint8_t play_main_clicked_btn_num_pre = 0;

#ifndef C_PC_SIM
const char* class_info_tl[] = {
    "英语教材",
    "古诗古文",
    "必读名著",
    "儿歌",
    "故事王国",
    "国学熏听",
    "美文美句",
    "保林叔叔",
    "语文教材",
    "大猫分级",
    "历史故事",
    "成语乐园",
    "百科知识",
    "财商教育",
    "情景对话",
};
#else
const char* class_info_tl[] = {
   "英语教材",
   "古诗古文",
   "必读名著",
   "儿歌",
   "故事王国",
   "国学熏听",
   "美文美句",
   "保林叔叔",
   "语文教材",
   "大猫分级",
   "历史故事",
   "成语乐园",
   "百科知识",
   "财商教育",
   "情景对话",
};
#endif


/**  判断指定素材是允许跳查
 * @return 非 0 表示允许被跳查
 */
int beAllowedJumpDict(const char* pcDatumPath) {
    if (!pcDatumPath || !pcDatumPath[0])
        return 0;
    if (strstr(pcDatumPath, "大猫分级") || strstr(pcDatumPath, "英语教材") ||
        strstr(pcDatumPath, "情景对话"))
    {
        return 1;
    }
    return 0;
}

//===============================
void dictScreen_del(void); //销毁词典跳查
//===============================


/**
 * @brief 书籍按钮点击回调
 * @param e
 */
static void play_item_btn_click_cb(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target(e);
    memset(&play_list_book_locate, 0, sizeof(play_list_book_locate));
    play_list_book_locate.playNum = lv_obj_get_index(obj);
    play_main_clicked_btn_num_pre = play_main_clicked_btn_num;
    play_main_clicked_btn_num = play_list_book_locate.playNum;
    if (play_main_clicked_btn_num_pre != play_main_clicked_btn_num)
    {
        play_list_clicked_btn_num = 0;
    }
    memset(play_list_book_locate.title, 0, sizeof(play_list_book_locate.title));
    strncpy(play_list_book_locate.title, lv_label_get_text(lv_obj_get_child(obj, 0)), sizeof(play_list_book_locate.title) - 1);
    // sprintf(play_list_book_locate.path, "%s/%s", player_class_locate.path, lv_label_get_text(lv_obj_get_child(obj, 0)));
    sprintf(play_list_book_locate.path, "%s/book%03d", player_class_locate.path, play_list_book_locate.playNum);
    play_list_book_locate.cover = player_class_locate.cover;
     printf("play_list_book_locate.path ：%s\n", play_list_book_locate.path);
    lv_indev_keypad_wait_release();
    play_main_page_free();
#if 0
    int ret = play_list_page_init(0);
    if (ret == -1)
    {
#if play_main_online_page_init_has
        //如果数据不存在,则返回   。2024.10.10   --xzz
        if (player_class_num == ENGLISH || player_class_num == CHINESE_TEXTBOOK)
            play_main_online_page_init(player_class_num, 1);
        else
            play_main_page_init(player_class_num);
#endif
        app_msgbox_timeout("提示", "该文件不存在！", 2000 | MSGBOX_TOAST_FLAG, NULL, NULL);
    }
#else
    memset(&play_text_book_locate, 0, sizeof(play_text_book_locate));
    strncpy(play_text_book_locate.path, play_list_book_locate.path, sizeof(play_text_book_locate.path) - 1);


    //strncpy(play_text_book_locate.title, lv_label_get_text(lv_obj_get_child(obj, 0)), sizeof(play_text_book_locate.title) - 1);
    play_text_book_locate.cover = play_list_book_locate.cover;
    play_text_book_locate.playNum = 0;

    int ret = play_text_page_init(0, 0);
    if (ret != 0)
    {
        //if (current_textbook_is_quanjie())
        //{
        //    play_chapter_page_init(-1);
        //}
        //else
        //    play_list_page_init(0);

        app_msgbox_timeout("提示", "该文件不存在！", 2000 | MSGBOX_TOAST_FLAG, NULL, NULL);
    }
#endif
}

/**
 * @brief 返回回调函数
 */
static void return_btn_click_cb(lv_event_t* e)
{
    play_main_page_free();
    zb_requestSwitchAp(C_ZB_AP_MAINMENU);
}

/**
 * @brief 主界面key事件回调
 * @param e
 */
void play_main_win_key_cb(lv_event_t* e)
{
    int keyVal = lv_event_get_key(e);
    // printf("num == %d\n", (int)lv_group_get_obj_count(lv_group_get_default()));
    LV_LOG_USER("%s [%d]", __FUNCTION__, __LINE__);
    zb_mem_print();
    switch (keyVal)
    {
    case LV_KEY_UP:
    {
        lv_group_focus_prev(lv_group_get_default());
        break;
    }
    case LV_KEY_DOWN:
    {
        lv_group_focus_next(lv_group_get_default());
        break;
    }
    case LV_KEY_ESC:
    {
        play_main_page_free();
        zb_requestSwitchAp(C_ZB_AP_MAINMENU);
        break;
    }
#ifdef C_PC_SIM
    case LV_KEY_EX_FUNC1:
    {
        break;
    }
#endif
    }
}

int play_main_page_free(void)
{
    LV_LOG_USER("%s[%d]\n", __FUNCTION__, __LINE__);
    if (play_main_win)
    {
        lv_obj_del(play_main_win);
        play_main_win = NULL;
    }
    LSNSPK_menu_close(class_menu);
    class_menu = NULL;
    return 0;
}

int play_main_page_hander(lv_event_code_t msg, int param)
{
    switch (msg)
    {
    case EVENT_FREE_AP_RES:
    {
        LV_LOG_USER("EVENT_FREE_AP_RES");
        play_main_page_free();
#if 0
        //play_text_page_free();
#else
        play_text_win_free_keep();
#endif
        dictScreen_del(); //销毁词典跳查
        break;
    }
    case EVENT_EX_CMD:
    {
        LV_LOG_USER("EVENT_CMD");
        if (param == PLY_CMD_PLY_LST_RET) {
        
            play_main_page_init(player_class_num);
            LV_LOG_USER("enter play_main_page_init \r\n");
        }
        break;
    }
    default:
        break;
    }

    return 0;
}

/**
 * @brief 此函数为主界面的显示，作用是分类列表显示
 * @param mt
 * @return
 */
int play_main_page_init(int classNum)
{
    play_main_page_free();
    play_main_win = lv_obj_create(lv_scr_act());
    //lv_obj_remove_style_all(play_main_win);
    lv_obj_set_size(play_main_win, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(play_main_win, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_scroll_dir(play_main_win, LV_DIR_NONE);
    ap_toggle_num = -1;  //初始化模块入口
    if (player_class_num != classNum)
    {
        play_main_clicked_btn_num = 0;
        play_list_clicked_btn_num = 0;
    }
    player_class_num = classNum;

#if 0//原来使用的
    if (player_class_num == OLD_POEM)
    {
        player_style = player_poem_style;
    }
    else if (player_class_num == CHINESE_CLASSIC) //现在定义为国学熏听  04.24
    {
        player_style = player_style_chs_classic;

    }
    else if (player_class_num == DATUM_OLD_POEM) 
    {
        player_style = player_style_chs_classic;

    }
    else //现在定义为大猫04.24
    {
        player_style = player_style_mao;
    }
#else//目前修改成统一 05.15
    if (1) 
    {
        player_style = player_style_mao;
    }
#endif
    lv_obj_add_event_cb(play_main_win, play_main_win_key_cb, LV_EVENT_KEY, NULL);

     sys_setCurMainWinAndCb(play_main_win, play_main_win_key_cb);
    //// 背景
    //lv_obj_t *bgImg = lv_img_create(play_main_win);
    //lv_img_dsc_t *pImgDsc = zb_img_res_load_one(NULL, player_style.page_bg_img1, IMG_FREE_TYPE_AUTO, NULL);
    //lv_img_set_src(bgImg, pImgDsc);

    // 标题
    lv_obj_t* title_label = lv_label_create(play_main_win);

    lv_label_set_text(title_label, class_info_tl[classNum]);

    lv_obj_set_style_text_color(title_label, lv_color_hex(0x18191B), LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(title_label, &lv_font_simsun_16_cjk, 0);
   // lv_obj_set_pos(title_label, 15, 15);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, lv_pct(5));

    lv_obj_t* download_btn = lv_obj_create(play_main_win);
    lv_obj_set_size(download_btn, 34, 34);
    lv_obj_set_style_radius(download_btn, 17, 0);
    lv_obj_set_style_bg_color(download_btn, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(download_btn, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(download_btn, 0, 0);
    lv_obj_set_style_border_width(download_btn, 0, 0);
    lv_img_dsc_t* download_pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_DOWNLOADBLK_BOOK, IMG_FREE_TYPE_AUTO, NULL);
    lv_obj_set_style_bg_img_src(download_btn, download_pImgDsc, 0);
    lv_obj_align_to(download_btn, title_label, LV_ALIGN_OUT_RIGHT_MID, 70, 0);
    // lv_obj_add_event_cb(download_btn, return_btn_click_cb, LV_EVENT_CLICKED, NULL);  


    lv_obj_t* player_return = lv_obj_create(play_main_win);
    lv_obj_set_size(player_return, 34, 34);
    lv_obj_set_style_radius(player_return, 17, 0);
    lv_obj_set_style_bg_color(player_return, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(player_return, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(player_return, 0, 0);
    lv_obj_set_style_border_width(player_return, 0, 0);
    lv_img_dsc_t* return_pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_RETURNBLK_BOOK, IMG_FREE_TYPE_AUTO, NULL);
    lv_obj_set_style_bg_img_src(player_return, return_pImgDsc, 0);
    lv_obj_align_to(player_return, title_label, LV_ALIGN_OUT_LEFT_MID, -70, 0);
    lv_obj_add_event_cb(player_return, return_btn_click_cb, LV_EVENT_CLICKED, NULL);

    // 项目列表
    lv_obj_t* item_obj = lv_obj_create(play_main_win);
    lv_obj_set_size(item_obj, SCREEN_WIDTH, lv_pct(94));
    lv_obj_align_to(item_obj, play_main_win, LV_ALIGN_TOP_MID, 0, lv_pct(12));
    lv_obj_set_scrollbar_mode(item_obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(item_obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_text_color(item_obj, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(item_obj, 0, 0);
    lv_obj_set_style_pad_top(item_obj, 15, 0);
    lv_obj_set_style_pad_bottom(item_obj, 40, 0);
    lv_obj_set_style_pad_left(item_obj, 15, 0);
    lv_obj_set_style_pad_row(item_obj, 15, 0);

    char path[100] = { 0 };
#ifndef C_PC_SIM
    sprintf(path, "%s%s/%s.mnu", PLAYER_DATA_PATH, class_info_tl[classNum], class_info_tl[classNum]);
#else
    sprintf(path, "%s%s/%s.mnu", PLAYER_DATA_PATH, class_info_tl[classNum], class_info_tl[classNum]);
#endif
    printf("mypath == %s\n", path);
    class_menu = LSNSPK_menu_open(path);
    if (!class_menu)
    {
        LV_LOG_USER("error\r\n");
        return -1;
    }
    lv_obj_t* item_btn;
    lv_obj_t* item_btn_label;

    for (int i = 0; i < class_menu->m_itemTotal; i++)
    {
        item_btn = lv_btn_create(item_obj);
        //lv_obj_remove_style_all(play_main_win);
        lv_obj_set_size(item_btn, lv_pct(94), 50);
        lv_obj_set_style_radius(item_btn, 9, 0);

        lv_obj_set_style_pad_hor(item_btn, 10, 0); //左右填充10个点
        lv_obj_set_style_pad_ver(item_btn, 10, 0);
        lv_obj_set_style_shadow_width(item_btn, 0, 0);
        lv_obj_set_style_border_width(item_btn, 0, 0);

        lv_obj_set_style_bg_color(item_btn, lv_color_hex(player_style.btn_def_color), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(item_btn, lv_color_hex(player_style.btn_focus_color), LV_STATE_FOCUS_KEY);
        lv_obj_add_event_cb(item_btn, play_item_btn_click_cb, LV_EVENT_CLICKED, NULL);


        item_btn_label = lv_label_create(item_btn);
        lv_label_set_text_fmt(item_btn_label, "%s", class_menu->astAlbumTbl[i].m_ptitle);
        lv_obj_set_width(item_btn_label, LV_PCT(70));

        lv_obj_align(item_btn_label, LV_ALIGN_CENTER, 0, 0);
        //        lv_obj_align(item_btn_label, LV_ALIGN_LEFT_MID, 15, 0);

        lv_obj_set_style_text_color(item_btn, lv_color_hex(player_style.text_def_color), LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(item_btn, lv_color_hex(player_style.text_focus_color), LV_STATE_FOCUS_KEY);
        if (0)  //这个是当前焦点按钮位置，不是当前音频播放位置  但暂用  i == play_main_clicked_btn_num
        {
            lv_obj_scroll_to_view(item_btn, LV_ANIM_OFF);
            lv_group_focus_obj(item_btn);
            //if (player_class_num == CHINESE_CLASSIC)
          //  {
                lv_obj_t* plyicon = lv_obj_create(item_btn);
                lv_obj_set_size(plyicon, 35, 35);
                lv_obj_set_style_radius(plyicon, 17, 0);
                lv_obj_set_style_bg_color(plyicon, lv_color_hex(0xffffff), 0);
                lv_obj_set_style_bg_opa(plyicon, LV_OPA_0, 0);
                lv_obj_set_style_shadow_width(plyicon, 0, 0);
                lv_obj_set_style_border_width(plyicon, 0, 0);

                lv_img_dsc_t* plybtn_pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_BOOK_PLAYING, IMG_FREE_TYPE_AUTO, NULL);
                lv_obj_set_style_bg_img_src(plyicon, plybtn_pImgDsc, 0);
                lv_obj_align(plyicon, LV_ALIGN_LEFT_MID, 0, 0);
            //}
        }
        else
        {
           // if (player_class_num == CHINESE_CLASSIC)
           // {
                lv_obj_t* plyicon = lv_obj_create(item_btn);
                lv_obj_set_size(plyicon, 35, 35);
                lv_obj_set_style_radius(plyicon, 17, 0);
                lv_obj_set_style_bg_color(plyicon, lv_color_hex(0xffffff), 0);
                lv_obj_set_style_bg_opa(plyicon, LV_OPA_0, 0);
                lv_obj_set_style_shadow_width(plyicon, 0, 0);
                lv_obj_set_style_border_width(plyicon, 0, 0);

                lv_img_dsc_t* plybtn_pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_BOOK_RDYPLAY, IMG_FREE_TYPE_AUTO, NULL);
                lv_obj_set_style_bg_img_src(plyicon, plybtn_pImgDsc, 0);
                lv_obj_align(plyicon, LV_ALIGN_LEFT_MID, 0, 0);

           // }
        }
    }

    player_class_locate.path = class_info_tl[player_class_num];
    player_class_locate.cover = class_menu->astAlbumTbl->m_imgId;
    return 0;

}


int zb_ap_play_main_page_init(void)
{
    play_main_page_init(ENGLISH);
    return 0;
}



int zb_ap_aud_player_ap_init(void)
{
    ESP_LOGI(TAG, "audPlyAp init A");
    uint16_t apInParm = zb_getApInParm();
    LV_ASSERT(apInParm >= 0 && apInParm < 30);
    play_main_page_init(apInParm);
    return 0;
}

int zb_ap_aud_player_ap_handler(lv_event_code_t msg, int param)
{
    play_main_page_hander(msg, param);
    return 0;
}


//----------------------------------------------


