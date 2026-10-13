
/*******************************************************
                    目录显示页面程序段
******************************************************/

#include "player_list.h"
#include "../ex_widget/app_msg.h"

extern const char* getDatumBasePath(const char* pcPath2ndPart);



int play_list_clicked_btn_num = 0;

static int32_t aud_offset = 0;
static int32_t aud_len = 0;
static uint16_t encParm = 0;
static FILE* fpUnit = NULL;
static struct LSNSPK_unit* pUnitInf = NULL;
static uint16_t play_current_num = 0;
static uint8_t current_loop_times = 0;
static uint8_t total_loop_times = 1;
static bool list_in_play = false;

int16_t ap_toggle_num = -1;
uint8_t collect_album_flag = 0;   	//收藏专辑标记
static player_book_info_t play_list_book_info = { 0 };

lv_obj_t* title_label = NULL;
static lv_timer_t* play_inquire_timer = NULL;
static lv_obj_t* collect_btn = NULL;
static lv_obj_t* all_play_btn = NULL;
static lv_obj_t* btn_cont = NULL;

static bool dialogFlag = 1;

static lv_obj_t* dialog; //弹窗
static lv_timer_t* timer_power;

//----------------------------------------------
#define TAG	"PLAY_LS"

extern bool current_textbook_is_quanjie();
extern int play_chapter_page_init(int16_t active_item);


/* 定时器回调函数 */
static void timer_cb(lv_timer_t* timer) {
    if (dialog != NULL)
    {
        lv_obj_del(dialog);  dialog = NULL;

    }
    if (timer != NULL)
    {
        lv_timer_del(timer);
    }
    dialogFlag = 1;
}


static lv_style_t style_dialog;
static void create_dialog_style(void) {
    lv_style_init(&style_dialog);
    lv_style_set_bg_color(&style_dialog, lv_color_hex(0x000000));
    lv_style_set_bg_opa(&style_dialog, LV_OPA_80);
    lv_style_set_border_width(&style_dialog, 0);
    lv_style_set_clip_corner(&style_dialog, true);
    lv_style_set_radius(&style_dialog, 15);
}


static void create_dialog(const char* text) {

    create_dialog_style();

    dialog = lv_obj_create(lv_scr_act());
    lv_obj_add_style(dialog, &style_dialog, 0);
    lv_obj_set_size(dialog, 164, 66);
    lv_obj_set_pos(dialog, 38, 127);


    lv_obj_t* label = lv_label_create(dialog);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label);

    timer_power = lv_timer_create(timer_cb, 2000, NULL);
    lv_timer_enable(timer_power);
}


/**
 * @brief 获取歌曲文件信息
 */
static int get_unit_info(char* path, uint16_t play_num)
{
    char buf[200] = { 0 };

    const char* pcBasePath = getDatumBasePath(path);
    snprintf(buf, sizeof(buf), "%s%s/%s", pcBasePath, path, play_list_book_info.pFileName[play_num]);
    // printf("buf == %s\n",buf);
    LSNSPK_unit_close(pUnitInf);
    pUnitInf = NULL;
    int error = 0;
    pUnitInf = LSNSPK_unit_open(buf, &error);
    if (pUnitInf == NULL || error < 0)
    {
        return -1;
    }
    fpUnit = ((LSNSPK_unit_t*)pUnitInf)->fpUnit;
    encParm = ((LSNSPK_unit_t*)pUnitInf)->encParm;
    aud_len = LSNSPK_get_lesson_aud_location(pUnitInf, &aud_offset);
    // printf("len = %d\n", aud_len);
    return 0;
}

/**
 * @brief 播放
 */
static int play_unit(char* path, uint16_t play_num)
{
    int ret = get_unit_info(path, play_num);
    if (ret)
    {
        list_in_play = false;
        lv_label_set_text_fmt(title_label, "%s解析错误,请播放下一首", play_list_book_info.pTile[play_num]);
        return -1;
    }
#ifndef C_PC_SIM
    aud_file_part_play(NULL, fpUnit, aud_offset, aud_len, encParm, MUSIC_PLAY_SPEED_1_00);
#endif
    return 0;
}

/**
 * @brief 全部播放回调函数
 */
static void all_play_btn_click_cb(lv_event_t* e)
{
#ifndef C_PC_SIM
    int ret = 0;
    if (!list_in_play)
    {
        list_in_play = true;
        play_text_page_free();
        ret = play_unit(play_list_book_locate.path, 0);
        if (ret)
        {
            return;
        }
        lv_timer_resume(play_inquire_timer);
    }
    else
    {
        music_play_status_t state = audio_stream_get_play_status();
        if (state == MUSIC_STATUS_PLAYING)
        {
            audio_stream_set_pause();
            lv_timer_pause(play_inquire_timer);
            return;
        }
        else
        {
            audio_stream_set_resume();
            lv_timer_resume(play_inquire_timer);
            return;
        }
    }
#endif
}

/**
 * @brief 连续播放
 */
static void play_unit_loop(void)
{
#ifndef C_PC_SIM
    int ret = 0;
    music_play_status_t state = audio_stream_get_play_status();
    if (state == MUSIC_STATUS_FINISH)
    {
        play_current_num = (play_current_num + 1) % play_list_book_info.cnt;
        if (play_current_num == 0)
        {
            current_loop_times = (current_loop_times + 1) % total_loop_times;
            if (current_loop_times == 0)
            {
                list_in_play = false;
                audio_stream_stop_play();
                lv_timer_pause(play_inquire_timer);
            }
            else
            {
                ret = play_unit(play_list_book_locate.path, play_current_num);
                if (ret)
                {
                    audio_stream_stop_play();
                    lv_timer_pause(play_inquire_timer);
                }
            }
        }
        else
        {
            ret = play_unit(play_list_book_locate.path, play_current_num);
            if (ret)
            {
                audio_stream_stop_play();
                lv_timer_pause(play_inquire_timer);
            }
        }
    }
#endif
}

/**
 * @brief 播放检测定时器回调
 */
static void play_inquire_timer_cb(lv_timer_t* timer)
{
    play_unit_loop();
}

/**
 * @brief 主窗口key事件回调
 * @param e
 */
static void play_list_win_key_cb(lv_event_t* e)
{
    ESP_LOGI(TAG, "%s [%d]", __FUNCTION__, __LINE__);
    // printf("num == %d\n", (int)lv_group_get_obj_count(lv_group_get_default()));
    int keyVal = lv_event_get_key(e);
    if (LV_KEY_ESC == keyVal)
    {
        if (ap_toggle_num == -1)
        {
#if play_main_online_page_init_has
            play_list_page_free();
            if (player_class_num == ENGLISH || player_class_num == CHINESE_TEXTBOOK)
                play_main_online_page_init(player_class_num, 1);
            else
                play_main_page_init(player_class_num);
#else
            play_main_page_init(player_class_num);
#endif
        }
        else
        {
            play_list_page_free();
            play_text_page_free();
            lv_timer_create(zb_ex_cmd_timer_cb, 0, (void*)PLY_CMD_PLY_LST_RET);
            ap_toggle_num = -1;
        }
    }
    else if (LV_KEY_UP == keyVal)
    {
#if 1
        lv_obj_t* f_obj = lv_group_get_focused(lv_group_get_default());
        if (f_obj == collect_btn)
        {
            play_list_clicked_btn_num = play_list_book_info.cnt - 1;
            f_obj = lv_obj_get_child(btn_cont, play_list_clicked_btn_num);
            lv_group_focus_obj(f_obj);
            lv_obj_scroll_to_view(f_obj, LV_ANIM_ON);

        }
        else if (f_obj == all_play_btn)
        {
            lv_group_focus_obj(collect_btn);
        }
        else
        {

            if (play_list_clicked_btn_num == 0)
            {
                play_list_clicked_btn_num = play_list_book_info.cnt - 1;
                lv_group_focus_obj(all_play_btn);
            }
            else
            {
                play_list_clicked_btn_num--;
                f_obj = lv_obj_get_child(btn_cont, play_list_clicked_btn_num);
                lv_group_focus_obj(f_obj);
                lv_obj_scroll_to_view(f_obj, LV_ANIM_ON);
            }
        }


#else
        lv_group_focus_prev(lv_group_get_default());
        if (play_text_win == lv_group_get_focused(lv_group_get_default()))
        {
            lv_group_focus_obj(play_list_win);
        }

#endif
    }
    else if (LV_KEY_DOWN == keyVal)
    {
#if 1
        lv_obj_t* f_obj = lv_group_get_focused(lv_group_get_default());
        if (f_obj == collect_btn)
        {
            lv_group_focus_obj(all_play_btn);
        }
        else if (f_obj == all_play_btn)
        {
            play_list_clicked_btn_num = 0;
            f_obj = lv_obj_get_child(btn_cont, play_list_clicked_btn_num);
            lv_group_focus_obj(f_obj);
            lv_obj_scroll_to_view(f_obj, LV_ANIM_ON);

        }
        else
        {
            play_list_clicked_btn_num++;
            if (play_list_clicked_btn_num >= play_list_book_info.cnt)
            {
                play_list_clicked_btn_num = 0;
                lv_group_focus_obj(collect_btn);  //到最后一个，选中收藏
            }
            else
            {
                f_obj = lv_obj_get_child(btn_cont, play_list_clicked_btn_num);
                lv_group_focus_obj(f_obj);
                lv_obj_scroll_to_view(f_obj, LV_ANIM_ON);
            }
        }

#else
        lv_group_focus_next(lv_group_get_default());
        ESP_LOGI(TAG, "%s [%d], focus: %p ", __FUNCTION__, __LINE__, lv_group_get_focused(lv_group_get_default()));
        if (play_text_win == lv_group_get_focused(lv_group_get_default()))
        {
            lv_group_focus_obj(play_list_win);
        }
#endif
    }
    zb_mem_print();
}

/**
 * @brief 播放列表的按钮点击回调函数
 * @param e
 */
static void play_list_btn_clicked_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_target(e);
    ESP_LOGI("", "%s [%d] list_path: %s\n", __FUNCTION__, __LINE__, play_list_book_locate.path);
    ESP_LOGI("", "%s [%d] title: %s\n", __FUNCTION__, __LINE__, play_list_book_locate.title);
    play_list_clicked_btn_num = lv_obj_get_child_id(obj);

    if (code == LV_EVENT_CLICKED)
    {
        memset(&play_text_book_locate, 0, sizeof(play_text_book_locate));
        strncpy(play_text_book_locate.path, play_list_book_locate.path, sizeof(play_text_book_locate.path) - 1);

        strncpy(play_text_book_locate.title, lv_label_get_text(lv_obj_get_child(obj, 0)), sizeof(play_text_book_locate.title) - 1);
        play_text_book_locate.cover = play_list_book_locate.cover;
        play_text_book_locate.playNum = lv_obj_get_index(obj);
        //addRecord(play_text_book_locate.path, play_text_book_locate.title, play_text_book_locate.playNum);
        lv_indev_keypad_wait_release();
        play_list_page_free();
        play_text_page_free();
        int ret = play_text_page_init(0, 1);
        if (ret != 0)
        {
            if (current_textbook_is_quanjie())
            {
                play_chapter_page_init(-1);
            }
            else
                play_list_page_init(0);

            app_msgbox_timeout("提示", "该文件不存在！", 2000 | MSGBOX_TOAST_FLAG, NULL, NULL);
        }

        if (ap_toggle_num == -1)
        {
            player_backstage_flag = 1;//后台播放
        }
        else
        {
            player_backstage_flag = 0;
        }
    }
}

/**
 * @brief 收藏回调函数
 */
static void collect_btn_click_cb(lv_event_t* e)
{
    ESP_LOGI(TAG, "进入收藏回调");
    int ret = 0; //addMusicCol_collect(play_list_book_locate.path, play_list_book_locate.title, 0, 1);
    lv_img_dsc_t* collect_pImgDsc = NULL;
    if (ret == 1)
    {
        collect_pImgDsc = zb_img_res_load_one(NULL, COMMON_IMG_START_SEL, IMG_FREE_TYPE_AUTO, NULL);
    }
    else if (ret == 2)
    {
        collect_pImgDsc = zb_img_res_load_one(NULL, COMMON_IMG_STAR, IMG_FREE_TYPE_AUTO, NULL);
        if (dialogFlag)
        {
            ESP_LOGI(TAG, "收藏已满！");
            create_dialog("收藏已满！");
            dialogFlag = 0;
        }
    }
    else
    {
        collect_pImgDsc = zb_img_res_load_one(NULL, COMMON_IMG_STAR, IMG_FREE_TYPE_AUTO, NULL);
    }
    lv_obj_set_style_bg_img_src(collect_btn, collect_pImgDsc, 0);

    // printf("path = %s,name = %s\n", play_list_book_locate.path, play_list_book_locate.title);
}

/**
 * @brief 返回回调函数
 */
static void return_btn_click_cb(lv_event_t* e)
{
    if (ap_toggle_num == -1)
    {

        play_list_page_free();
#if play_main_online_page_init_has
        if (player_class_num == ENGLISH || player_class_num == CHINESE_TEXTBOOK)
            play_main_online_page_init(player_class_num, 1);
        else
            play_main_page_init(player_class_num);
#else
        
        play_main_page_init(player_class_num);
#endif
    }
    else
    {
        play_list_page_free();
        play_text_page_free();
        lv_timer_create(zb_ex_cmd_timer_cb, 0, (void*)PLY_CMD_PLY_LST_RET);
        ap_toggle_num = -1;
    }
}

///**
// * @brief 专辑列表页面释放
// */
//int play_list_page_free(void)
//{
//    ESP_LOGI("", "%s[%d]\n", __FUNCTION__, __LINE__);
//    list_in_play = false;
//    free_player_book_info_t(&play_list_book_info);
//    if (play_list_win)
//    {
//        lv_obj_del(play_list_win);
//        play_list_win = NULL;
//    }
//    if (play_inquire_timer)
//    {
//        lv_timer_del(play_inquire_timer);
//        play_inquire_timer = NULL;
//    }
//    if (pUnitInf)
//    {
//#ifndef C_PC_SIM
//        audio_stream_stop_play();
//#endif
//        LSNSPK_unit_close(pUnitInf);
//        pUnitInf = NULL;
//    }
//    if (dialog != NULL)
//    {
//        lv_obj_del(dialog);  dialog = NULL;
//
//    }
//    ESP_LOGI("", "%s[%d]\n", __FUNCTION__, __LINE__);
//    return 0;
//}

/**
 * @brief 此函数实现了播放列表显示，专辑的显示，全部播放的功能
 * @param lt 结构体包含了背景图片，专辑名称，课文目录等信息内容，需要自行定义结构体信息
 */
int play_list_page_init(int parm)
{
    ESP_LOGI("", "%s [%d]\n", __FUNCTION__, __LINE__);
    play_list_page_free();

    // 解析目录文件
    free_player_book_info_t(&play_list_book_info);
    char buf[300] = { 0 };

    const char* pcBasePath = getDatumBasePath(play_list_book_locate.path);
    snprintf(buf, sizeof(buf), "%s%s/album.mnu", pcBasePath, play_list_book_locate.path);
    ESP_LOGI("", "%s[%d], path:%s ,size: %d\n", __FUNCTION__, __LINE__, buf, strlen(buf));
    int ret = player_get_book_inf(buf, &play_list_book_info);
    if (ret)
    {
        ESP_LOGI("player_list", "%s\n", "get book menu fail");
        play_list_page_free();
        return -1;
    }

    play_list_win = lv_obj_create(lv_scr_act());
    //lv_obj_remove_style_all(play_list_win);
    lv_obj_set_scroll_dir(play_list_win, LV_DIR_NONE);
    lv_obj_set_size(play_list_win, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(play_list_win, lv_color_hex(0xFFFFFF), 0);

    lv_obj_add_event_cb(play_list_win, play_list_win_key_cb, LV_EVENT_KEY, NULL);
    sys_setCurMainWinObj(play_list_win);
    //// 背景
    //lv_obj_t* img1 = lv_img_create(play_list_win);
    //lv_img_dsc_t* pImgDsc = zb_img_res_load_one(NULL, player_style.page_bg_img1, IMG_FREE_TYPE_AUTO, NULL);
    //lv_img_set_src(img1, pImgDsc);

    // 标题
    title_label = lv_label_create(play_list_win);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    char* title_text = play_list_book_locate.title;
    lv_label_set_text(title_label, title_text);
    lv_obj_set_style_text_color(title_label, lv_color_hex(player_style.text_def_color), LV_STATE_DEFAULT);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, lv_pct(5));


    lv_obj_t* player_return = lv_btn_create(play_list_win);
    lv_obj_set_size(player_return, 34, 34);
    lv_obj_set_style_radius(player_return, 17, 0);
    lv_obj_set_style_bg_color(player_return, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(player_return, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(player_return, 0, 0);
    lv_img_dsc_t* return_pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_RETURNBLK_BOOK, IMG_FREE_TYPE_AUTO, NULL);
    lv_obj_set_style_bg_img_src(player_return, return_pImgDsc, 0);
    lv_obj_align_to(player_return, title_label, LV_ALIGN_OUT_LEFT_MID,-90 , 0);
    lv_obj_add_event_cb(player_return, return_btn_click_cb, LV_EVENT_CLICKED, NULL);

    // 收藏
    collect_btn = lv_btn_create(play_list_win);
    lv_obj_set_style_bg_opa(collect_btn, LV_OPA_0, 0);
    lv_obj_set_style_shadow_opa(collect_btn, LV_OPA_0, 0);
    lv_img_dsc_t* collect_pImgDsc;

    int indexToDelete = 0;//hasSameMusicName(play_list_book_locate.title, 1);
    ESP_LOGI(TAG, "indexToDelete :%d", indexToDelete);
    if (indexToDelete > 0)
        // if(1)//test
    {
        //  collect_pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYERCOL, IMG_FREE_TYPE_AUTO, NULL);
    }
    else
    {
        collect_pImgDsc = zb_img_res_load_one(NULL, COMMON_IMG_STAR, IMG_FREE_TYPE_AUTO, NULL);
    }
    lv_obj_set_style_bg_img_src(collect_btn, collect_pImgDsc, 0);
    lv_obj_set_size(collect_btn, 24, 24);
    lv_obj_align_to(collect_btn, title_label, LV_ALIGN_OUT_RIGHT_MID, 80, 0);
    lv_obj_add_event_cb(collect_btn, collect_btn_click_cb, LV_EVENT_CLICKED, 0);

    // 列表obj
    lv_obj_t* uderbg_obj = lv_obj_create(play_list_win);
    lv_obj_clear_flag(uderbg_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(uderbg_obj, SCREEN_WIDTH, lv_pct(94));
    lv_obj_set_style_radius(uderbg_obj, 0, 0);
    lv_obj_set_style_pad_top(uderbg_obj, 36, 0);
    lv_obj_align(uderbg_obj, LV_ALIGN_TOP_MID, 0, 46);
    lv_obj_set_style_border_width(uderbg_obj, 0, 0);
    lv_obj_set_style_bg_color(uderbg_obj, lv_color_hex(0xFFFFFF), 0);


    // 播放列表
    btn_cont = lv_obj_create(uderbg_obj);
    lv_obj_set_size(btn_cont, SCREEN_WIDTH, lv_pct(85));
    lv_obj_align(btn_cont, LV_ALIGN_TOP_MID, 0, lv_pct(10));
    lv_obj_set_scrollbar_mode(btn_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(btn_cont, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(btn_cont, 10, 0);
    lv_obj_set_style_pad_row(btn_cont, 10, 0);
    lv_obj_set_style_pad_left(btn_cont, 10, 0);
    lv_obj_set_style_pad_bottom(btn_cont, 30, 0);
    lv_obj_set_style_border_width(btn_cont, 0, 0);
    lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_COLUMN);

    // 播放检测定时器
    play_inquire_timer = lv_timer_create(play_inquire_timer_cb, 500, NULL);
    lv_timer_pause(play_inquire_timer);



    ESP_LOGI("", "%s [%d]\n", __FUNCTION__, __LINE__);
    // 全部播放按钮
    all_play_btn = lv_btn_create(uderbg_obj);
    lv_obj_set_size(all_play_btn, 24, 24);
    lv_obj_set_style_bg_color(all_play_btn, lv_color_hex(player_style.btn_focus_color), LV_STATE_DEFAULT);
    lv_obj_set_style_radius(all_play_btn, 90, 0);
    lv_obj_set_style_border_width(all_play_btn, 0, 0);
    lv_obj_align(all_play_btn, LV_ALIGN_TOP_LEFT, 10, 0);
    lv_obj_add_event_cb(all_play_btn, all_play_btn_click_cb, LV_EVENT_CLICKED, NULL);

    lv_img_dsc_t* allplay_pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_ALLPLAY, IMG_FREE_TYPE_AUTO, NULL);
    lv_obj_set_style_bg_img_src(all_play_btn, allplay_pImgDsc, 0);

    lv_obj_t* all_play_label = lv_label_create(uderbg_obj);
    lv_label_set_text_fmt(all_play_label, "全部播放(%d)", play_list_book_info.cnt);
    lv_obj_set_style_text_color(all_play_label, lv_color_hex(player_style.text_def_color), LV_STATE_DEFAULT);
    lv_obj_align_to(all_play_label, all_play_btn, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // unit播放列表
    lv_obj_t* list_btn;
    lv_obj_t* list_btn_label;
    for (int i = 0; i < play_list_book_info.cnt; i++)
    {
        list_btn = lv_btn_create(btn_cont);
        lv_obj_set_size(list_btn, 300, 50);
        lv_obj_set_style_radius(list_btn, 15, 0);
        lv_obj_set_style_border_width(list_btn,0, 0);
        lv_obj_set_style_shadow_width(list_btn, 0,0);
        //-----------------------------------------> 240729 xzz
        lv_obj_set_style_pad_hor(list_btn, 10, 0);		//左右填充10个点
        lv_obj_set_style_pad_ver(list_btn, 10, 0);
        //------------------------------------------>
        lv_obj_set_style_bg_color(list_btn, lv_color_hex(player_style.btn_def_color), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(list_btn, lv_color_hex(player_style.btn_focus_color), LV_STATE_FOCUS_KEY);
        //        ESP_LOGI("player_list","list_btn i: %d --obj:%p", i, list_btn);

        list_btn_label = lv_label_create(list_btn);

        lv_obj_set_width(list_btn_label, LV_PCT(100));

        lv_obj_align(list_btn_label, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_long_mode(list_btn_label, LV_LABEL_LONG_DOT);
        char* pTitle = play_list_book_info.pTile[i];
        if (strchr(pTitle, 0x05))
            pTitle = strchr(pTitle, 0x05) + 1;
        lv_label_set_text(list_btn_label, pTitle);

        lv_obj_set_style_text_color(list_btn, lv_color_hex(player_style.text_def_color), LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(list_btn, lv_color_hex(player_style.text_focus_color), LV_STATE_FOCUS_KEY);
        lv_obj_add_event_cb(list_btn, play_list_btn_clicked_cb, LV_EVENT_CLICKED, NULL);
    }

    int selNo = play_list_clicked_btn_num;
    lv_obj_t* objSel = lv_obj_get_child(btn_cont, selNo);
    if (objSel != NULL) {
        if (selNo != 0)
        {
            lv_obj_scroll_to_view(objSel, LV_ANIM_OFF);
        }
        lv_group_focus_obj(objSel);
    }
    // free_player_book_info_t(&play_list_book_info);
    return 0;
}

/**
 * @brief 专辑列表页面函数初始化
 * @param
 *          path 路径，例如（必读名著/book0002）
 *          ap_num 调用本接口的函数的AP号（在main\utils\misc_utils.h的宏定义）
 */
int play_list_collect_page_init(char* path, char* title, int16_t ap_num)
{
    ESP_LOGI("", "%s [%d]\n", __FUNCTION__, __LINE__);
    if (!path)
    {
        ESP_LOGI(PLA, "error: path is null\n");
        return -1;
    }
    collect_album_flag = 1; //收藏专辑
    if (strstr(path, "古诗古文"))
    {
        player_style = player_poem_style;
    }
    else
    {
        player_style = player_normal_style;
    }
    ap_toggle_num = ap_num;
    play_list_book_locate.cover = -1;
    strncpy(play_list_book_locate.path, path, sizeof(play_list_book_locate.path) - 1);

    strncpy(play_list_book_locate.title, title, sizeof(play_list_book_locate.title) - 1);

    ESP_LOGI("", "%s [%d] path: %s\n", __FUNCTION__, __LINE__, play_list_book_locate.path);
    ESP_LOGI("", "%s [%d] title: %s\n", __FUNCTION__, __LINE__, play_list_book_locate.title);
    play_list_book_locate.playNum = 0;
    int ret = play_list_page_init(0);
    if (ret == -1)
    {
        play_list_page_free();
        play_text_page_free();
        lv_timer_create(zb_ex_cmd_timer_cb, 0, (void*)PLY_CMD_PLY_LST_RET);
        ap_toggle_num = -1;

        app_msgbox_timeout("提示", "该文件不存在！", 2000 | MSGBOX_TOAST_FLAG, NULL, NULL);
    }
    return 0;
}
