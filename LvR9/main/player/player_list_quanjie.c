
/*******************************************************
                    目录显示页面程序段
******************************************************/

#include "player.h"
#include "player_list.h"
#include "../data_parse/cJSON.h"

#define BROWSER_LINE_STEP 5
#define BROWSER_SCROLL_LINE_Y (17+BROWSER_LINE_STEP)
#define BROWSER_SCROLL_PAGE_Y (BROWSER_SCROLL_LINE_Y*12)
#define QUANJIE_QUES    6

static int32_t aud_offset = 0;
static int32_t aud_len = 0;
static uint16_t encParm = 0;
static uint16_t play_current_num = 0;
static uint8_t current_loop_times = 0;
static uint8_t total_loop_times = 1;
static bool list_in_play = false;

static player_book_info_t play_list_book_info = { 0 };

static lv_obj_t* title_label = NULL;
static lv_obj_t* btn_cont = NULL;

lv_obj_t* play_unit_list_win = NULL;       // 单元列表
lv_obj_t* play_chapter_list_win = NULL;    // 章节列表
lv_obj_t* play_quanjie_text_win = NULL;    // 全解内容
lv_obj_t* browser = NULL;
static int16_t active_unit;         // 记录活动单元
static int16_t active_chapter;      // 记录活动章节

struct QuanJieChapterData  // 全解的章节数据
{
    uint8_t item_sum;
    uint8_t item_style[15]; // 单词、课文等地址列表 
    uint32_t size;          // buf的大小
    uint16_t unit;          // 当前数据对应的 单元

    char* pData;            // 保存全解章节数据的 buf，后续释放
    char** pChapter;		// 章节字符串指针， 首个字符表示类型
} gQjcd;

struct QuanJieContentData
{
    uint8_t item_sum;
    int16_t active;
    uint32_t size;
    char* pData;            // 保存数据的 buf，后续释放
    cJSON* ques;
}gQjOtherData;

//extern char* LSNSPK_read_qj_data(const char* pPathUnit, uint8_t style, uint8_t* chapter_num, uint32_t* size, int* pErrNo);
static int fill_ques_content(int16_t activeItem);

int play_quanjie_unit_page_init(int parm); //显示单元：module X
int play_chapter_page_init(int16_t active_item); // 显示章节： 即7大模块
static int play_quanjie_text_page_init(int16_t active_item); // 显示正文

static int play_unit_list_page_free(void);
static int play_chapter_list_page_free(void);
static int play_quanjie_text_page_free(void);

#define TAG	"PLAY_QJ"

//char* LSNSPK_read_qj_data(const char* pPathUnit, uint8_t style, uint8_t* chapter_num, uint32_t* size, int* pErrNo)
//{
//    return "";
//}
/**
 * @brief 主窗口key事件回调
 * @param e
 */
static void play_unit_list_win_key_cb(lv_event_t* e)
{
    ESP_LOGI(TAG, "%s [%d]", __FUNCTION__, __LINE__);
    // printf("num == %d\n", (int)lv_group_get_obj_count(lv_group_get_default()));
    int keyVal = lv_event_get_key(e);
    if (LV_KEY_ESC == keyVal)
    {
#if play_main_online_page_init_has
        play_unit_list_page_free();
        if (player_class_num == ENGLISH || player_class_num == CHINESE_TEXTBOOK) {
            play_main_online_page_init(player_class_num, 1);
        }
        else
            play_main_page_init(player_class_num);
#endif
    }
    else if (LV_KEY_UP == keyVal)
    {
		lv_obj_t* f_obj = lv_group_get_focused(lv_group_get_default());
		active_unit --;
        if (active_unit < 0)
            active_unit = play_list_book_info.cnt - 1;
		f_obj = lv_obj_get_child(btn_cont, active_unit);
		lv_group_focus_obj(f_obj);			
		lv_obj_scroll_to_view(f_obj, LV_ANIM_OFF);
    }
    else if (LV_KEY_DOWN == keyVal)
    {
		lv_obj_t* f_obj = lv_group_get_focused(lv_group_get_default());
        active_unit++;
		if(active_unit >= play_list_book_info.cnt)
            active_unit = 0;
		f_obj = lv_obj_get_child(btn_cont, active_unit);
		lv_group_focus_obj(f_obj);
		lv_obj_scroll_to_view(f_obj, LV_ANIM_OFF);
    }
    zb_mem_print();
}

static int read_chapter_data(uint16_t unit)
{
    if (gQjcd.pData == NULL || gQjcd.unit != unit) {
        // 先释放旧资源量
        if (gQjcd.pData) {
            heap_caps_free(gQjcd.pData);
            heap_caps_free(gQjcd.pChapter);
            gQjcd.pData = NULL;
            gQjcd.pChapter = NULL;
        }
        char tmp_buf[ZB_MAX_PATH];
        sprintf(tmp_buf, "%s%s/%s", PLAYER_ONLINE_DATA_PATH, play_list_book_locate.path, play_list_book_info.pFileName[unit]);
        gQjcd.pData = LSNSPK_read_qj_data(tmp_buf, 0, &gQjcd.item_sum, &gQjcd.size, NULL);
        if (gQjcd.pChapter == NULL)
        {
            gQjcd.pChapter = (char*)heap_caps_malloc_dflt(sizeof(char*) * gQjcd.item_sum);
        }
        char* p = gQjcd.pData;
        for (int i = 0; i < gQjcd.item_sum; i++) {
            gQjcd.item_style[i] = (uint8_t)(*p);

            gQjcd.pChapter[i] = p + 1;
            p = strchr(p + 1, 0x0a);
            if (p)
                *p++ = 0;
        }
        gQjcd.unit = unit;
        return 0;
    }
    return -1;
}

/**
 * @brief 播放列表的按钮点击回调函数
 * @param e
 */
static void play_unit_list_btn_clicked_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED)
    {
        memset(&play_text_book_locate, 0, sizeof(play_text_book_locate));
        strcpy(play_text_book_locate.path, play_list_book_locate.path);
        strcpy(play_text_book_locate.title, lv_label_get_text(lv_obj_get_child(obj, 0)));
        play_text_book_locate.cover = play_list_book_locate.cover;
        play_text_book_locate.playNum = lv_obj_get_index(obj);
        //addRecord(play_text_book_locate.path, play_text_book_locate.title, play_text_book_locate.playNum);
        active_unit = lv_obj_get_index(obj);
        lv_indev_keypad_wait_release();
        read_chapter_data(active_unit);

        play_chapter_page_init(0);
        active_chapter = 0;
    }
}

/**
 * @brief 专辑列表页面释放
 */
static int play_unit_list_page_free(void)
{
    list_in_play = false;
    free_player_book_info_t(&play_list_book_info);
    if (gQjcd.pData) {
        heap_caps_free(gQjcd.pData);
        gQjcd.pData = NULL;
    }
    if (gQjcd.pChapter) {
        heap_caps_free(gQjcd.pChapter);
        gQjcd.pChapter = NULL;
    }

    if (play_unit_list_win)
    {
        lv_obj_del(play_unit_list_win);
        play_unit_list_win = NULL;
    }

    return 0;
}

/**
 * @brief 此函数实现了该本书的每课列表
 * @param parm: 列表的活动项
 * 
 */
int play_quanjie_unit_page_init(int parm)
{
    play_unit_list_page_free();
    //play_unit_list_win = lv_obj_create(lv_scr_act());
    play_unit_list_win = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(play_unit_list_win);
    lv_obj_set_size(play_unit_list_win, SCREEN_WIDTH, SCREEN_HEIGHT);

    lv_obj_add_event_cb(play_unit_list_win, play_unit_list_win_key_cb, LV_EVENT_KEY, NULL);
    sys_setCurMainWinObj(play_unit_list_win);
    // 背景
    lv_obj_t* img1 = lv_img_create(play_unit_list_win);
    lv_img_dsc_t* pImgDsc = zb_img_res_load_one(NULL, player_style.page_bg_img1, IMG_FREE_TYPE_AUTO, NULL);
    lv_img_set_src(img1, pImgDsc);

    // 标题
    title_label = lv_label_create(play_unit_list_win);
    lv_obj_set_width(title_label, 210);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    char* title_text = play_list_book_locate.title;
    lv_label_set_text(title_label, title_text);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_pos(title_label, 15, 15);

    // 列表obj
    lv_obj_t* uderbg_obj = lv_obj_create(play_unit_list_win);
    lv_obj_clear_flag(uderbg_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(uderbg_obj, SCREEN_WIDTH, 270);
    // lv_obj_set_style_radius(uderbg_obj, 12, 0);
    lv_obj_set_style_pad_top(uderbg_obj, 6, 0);
    lv_obj_align(uderbg_obj, LV_ALIGN_BOTTOM_MID, 0, 0);

    // 播放列表
    btn_cont = lv_obj_create(uderbg_obj);
    lv_obj_set_size(btn_cont, 230, 270);
    lv_obj_align(btn_cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_border_width(btn_cont, 0, 0); // 设置btn_cont的边框
    // lv_obj_set_scrollbar_mode(btn_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_top(btn_cont, 10, 0);
    lv_obj_set_style_pad_row(btn_cont, 10, 0);
    lv_obj_set_style_pad_left(btn_cont, 12, 0);
    lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_COLUMN);

    // 解析目录文件
    free_player_book_info_t(&play_list_book_info);
    char buf[200] = { 0 };
    sprintf(buf, "%s%s/album.mnu", PLAYER_ONLINE_DATA_PATH, play_list_book_locate.path);
    // printf("path == %s\n", buf);
    int ret = player_get_book_inf(buf, &play_list_book_info);
    if (ret)
    {
        ESP_LOGI("player_list", "%s\n", "get book menu fail");
        return -1;
    }

    // unit播放列表
    active_unit = parm;
    lv_obj_t* list_btn;
    lv_obj_t* list_btn_label;
    for (int i = 0; i < play_list_book_info.cnt; i++)
    {
        list_btn = lv_btn_create(btn_cont);
        lv_obj_set_size(list_btn, 200, 50);
        lv_obj_set_style_radius(list_btn, 15, 0);
        //-----------------------------------------> 240729 xzz
        lv_obj_set_style_pad_hor(list_btn, 10, 0);		//左右填充10个点
        lv_obj_set_style_pad_ver(list_btn, 10, 0);
        //------------------------------------------>
        lv_obj_set_style_bg_color(list_btn, lv_color_hex(player_style.btn_def_color), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(list_btn, lv_color_hex(player_style.btn_focus_color), LV_STATE_FOCUS_KEY);
        //ESP_LOGI("player_list","list_btn i: %d --obj:%p", i, list_btn);

        list_btn_label = lv_label_create(list_btn);

        lv_obj_set_width(list_btn_label, LV_PCT(100));

        lv_obj_align(list_btn_label, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_long_mode(list_btn_label, LV_LABEL_LONG_DOT);
        char* pTitle = play_list_book_info.pTile[i];
        if (strchr(pTitle, 0x05))
            pTitle = strchr(pTitle, 0x05) + 1;
        lv_label_set_text(list_btn_label, pTitle);

        lv_obj_set_style_text_color(list_btn, lv_color_hex(0x000000), LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(list_btn, lv_color_hex(0xffffff), LV_STATE_FOCUS_KEY);
        lv_obj_add_event_cb(list_btn, play_unit_list_btn_clicked_cb, LV_EVENT_CLICKED, NULL);
        if (i == parm)
        {
            lv_obj_scroll_to_view(list_btn, LV_ANIM_OFF);
            lv_group_focus_obj(list_btn);
        }
    }
    return 0;
}

/**
 * @brief 章节列表页面释放
 */
static int play_chapter_list_page_free(void)
{
    if (play_chapter_list_win)
    {
        lv_obj_del(play_chapter_list_win);
        play_chapter_list_win = NULL;
    }
   
    return 0;
}

static int play_quanjie_text_page_free(void) {
    if (gQjOtherData.pData) {
        heap_caps_free(gQjOtherData.pData);
        gQjOtherData.pData = NULL;
    }
    if (gQjOtherData.ques) {
        cJSON_Delete(gQjOtherData.ques);
        gQjOtherData.ques = NULL;
    }
    if (play_quanjie_text_win) {
        lv_obj_del(play_quanjie_text_win);
        play_quanjie_text_win = NULL;
    }
    return 0;
}
// 
// 
/**
 * @brief 章节列表的按钮点击回调函数
 * @param e
 */
static void play_chapter_list_btn_clicked_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED)
    {
        if (active_chapter == 0) {
            //memset(&play_text_book_locate, 0, sizeof(play_text_book_locate));
            //strcpy(play_text_book_locate.path, play_list_book_locate.path);
            //strcpy(play_text_book_locate.title, lv_label_get_text(lv_obj_get_child(obj, 0)));
            //play_text_book_locate.cover = play_list_book_locate.cover;
            //play_text_book_locate.playNum = lv_obj_get_index(obj);
            //addRecord(play_text_book_locate.path, play_text_book_locate.title, play_text_book_locate.playNum);
            lv_indev_keypad_wait_release();
            //play_chapter_list_page_free();

            //play_text_page_free();
            play_text_page_init(0, 1);
        }
        else {
            play_quanjie_text_page_init(active_chapter);
        }
    }
}
/**
 * @brief 主窗口key事件回调
 * @param e
 */
static void play_chapter_list_win_key_cb(lv_event_t* e)
{
    ESP_LOGI(TAG, "%s [%d]", __FUNCTION__, __LINE__);
    // printf("num == %d\n", (int)lv_group_get_obj_count(lv_group_get_default()));
    int keyVal = lv_event_get_key(e);
    if (LV_KEY_ESC == keyVal)
    {
        play_chapter_list_page_free();
        play_quanjie_unit_page_init(active_unit);
    }
    else if (LV_KEY_UP == keyVal)
    {
        lv_obj_t* f_obj = NULL; // lv_group_get_focused(lv_group_get_default());
        active_chapter--;
        if (active_chapter < 0)
            active_chapter = gQjcd.item_sum - 1;
        f_obj = lv_obj_get_child(btn_cont, active_chapter);
        lv_group_focus_obj(f_obj);
        lv_obj_scroll_to_view(f_obj, LV_ANIM_OFF);
    }
    else if (LV_KEY_DOWN == keyVal)
    {
        lv_obj_t* f_obj = NULL;// lv_group_get_focused(lv_group_get_default());
        active_chapter++;
        if (active_chapter >= gQjcd.item_sum)
            active_chapter = 0;
        f_obj = lv_obj_get_child(btn_cont, active_chapter);
        lv_group_focus_obj(f_obj);
        lv_obj_scroll_to_view(f_obj, LV_ANIM_OFF);
    }
    else if (keyVal == LV_KEY_ENTER)
    {
        if (gQjcd.item_style[active_chapter] == 0) {
            //addRecord(play_text_book_locate.path, play_text_book_locate.title, play_text_book_locate.playNum);
            lv_indev_keypad_wait_release();
            //play_chapter_list_page_free();
            //play_text_page_free();
            play_text_page_init(0, 1);
        }
        else {
            play_quanjie_text_page_init(active_chapter);
        }
    }

    zb_mem_print();
}

/**
* 
*/
static void play_quanjie_text_win_key_cb(lv_event_t* e)
{
    ESP_LOGI(TAG, "%s [%d]", __FUNCTION__, __LINE__);
    // printf("num == %d\n", (int)lv_group_get_obj_count(lv_group_get_default()));
    int keyVal = lv_event_get_key(e);
    lv_obj_t* obj = lv_event_get_target(e);
    int16_t nextItem = -1;

    if (LV_KEY_ESC == keyVal)
    {
        play_quanjie_text_page_free();
        play_chapter_page_init(active_chapter);
    }
    else if (LV_KEY_UP == keyVal)
    {
        lv_coord_t y = lv_obj_get_scroll_y(browser); //超长句子翻页
        if (y > BROWSER_SCROLL_LINE_Y)
            y -= BROWSER_SCROLL_LINE_Y;
        else
            y = 0;
        lv_obj_scroll_to_y(browser, y, LV_ANIM_OFF);
    }
    else if (LV_KEY_DOWN == keyVal)
    {
        lv_coord_t hh = lv_obj_get_content_height(browser);
        lv_coord_t y = lv_obj_get_scroll_y(browser);
        printf("y == %d\n", y);
        lv_obj_scroll_to_y(browser, y+ BROWSER_SCROLL_LINE_Y, LV_ANIM_OFF);
    }
    else if (LV_KEY_LEFT == keyVal)
    {
        if (gQjcd.item_style[active_chapter] == 1)
        {
            nextItem = gQjOtherData.active - 1;
            if (nextItem < 0)
                nextItem = gQjOtherData.item_sum - 1;
            ESP_LOGI(TAG, "previous ques is : %d", nextItem);
            lv_obj_clean(browser);
            fill_ques_content(nextItem);
        }
        else {
            
            lv_coord_t y = lv_obj_get_scroll_y(browser); //超长句子翻页
            if (y > BROWSER_SCROLL_PAGE_Y)
                y -= BROWSER_SCROLL_PAGE_Y;
            else
                y = 0;
            lv_obj_scroll_to_y(browser, y, LV_ANIM_OFF);
        }
    }
    else if (LV_KEY_RIGHT == keyVal)
    {
        if (gQjcd.item_style[active_chapter] == 1)
        {
            nextItem = gQjOtherData.active + 1;
            nextItem %= gQjOtherData.item_sum;
            ESP_LOGI(TAG, "next ques is : %d", nextItem);
            lv_obj_clean(browser);
            fill_ques_content(nextItem);
        }
        else {
            lv_coord_t hh = lv_obj_get_content_height(browser);
            lv_coord_t y = lv_obj_get_scroll_y(browser);
            printf("y == %d\n", y);
            lv_obj_scroll_to_y(browser, y + BROWSER_SCROLL_PAGE_Y, LV_ANIM_OFF);
        }
    }
    zb_mem_print();
    lv_event_stop_processing(e);
}

/*
*/
int play_chapter_page_init(int16_t active_item)
{
    play_chapter_list_page_free();

    play_chapter_list_win = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(play_chapter_list_win);
    lv_obj_set_size(play_chapter_list_win, SCREEN_WIDTH, SCREEN_HEIGHT);

    lv_obj_add_event_cb(play_chapter_list_win, play_chapter_list_win_key_cb, LV_EVENT_KEY, NULL);
    sys_setCurMainWinObj(play_chapter_list_win);
    // 背景
    lv_obj_t* img1 = lv_img_create(play_chapter_list_win);
    lv_img_dsc_t* pImgDsc = zb_img_res_load_one(NULL, player_style.page_bg_img1, IMG_FREE_TYPE_AUTO, NULL);
    lv_img_set_src(img1, pImgDsc);

    // 标题
    title_label = lv_label_create(play_chapter_list_win);
    lv_obj_set_width(title_label, 210);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    //char* title_text = play_list_book_locate.title;
    char* title_text = play_list_book_info.pTile[active_unit];
    lv_label_set_text(title_label, title_text);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_pos(title_label, 15, 15);

    // 列表obj
    lv_obj_t* uderbg_obj = lv_obj_create(play_chapter_list_win);
    lv_obj_clear_flag(uderbg_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(uderbg_obj, SCREEN_WIDTH, 270);
    // lv_obj_set_style_radius(uderbg_obj, 12, 0);
    lv_obj_set_style_pad_top(uderbg_obj, 6, 0);
    lv_obj_align(uderbg_obj, LV_ALIGN_BOTTOM_MID, 0, 0);

    // 播放列表
    btn_cont = lv_obj_create(uderbg_obj);
    lv_obj_set_size(btn_cont, 230, 270);
    lv_obj_align(btn_cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_border_width(btn_cont, 0, 0); // 设置btn_cont的边框
    // lv_obj_set_scrollbar_mode(btn_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_top(btn_cont, 10, 0);
    lv_obj_set_style_pad_row(btn_cont, 10, 0);
    lv_obj_set_style_pad_left(btn_cont, 12, 0);
    lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_COLUMN);


    if (active_item == -1)
        active_item = 0;

    // unit播放列表
    lv_obj_t* list_btn;
    lv_obj_t* list_btn_label;
    for (int i = 0; i < gQjcd.item_sum; i++)
    {
        list_btn = lv_btn_create(btn_cont);
        lv_obj_set_size(list_btn, 200, 50);
        lv_obj_set_style_radius(list_btn, 15, 0);
        //-----------------------------------------> 240729 xzz
        lv_obj_set_style_pad_hor(list_btn, 10, 0);		//左右填充10个点
        lv_obj_set_style_pad_ver(list_btn, 10, 0);
        //------------------------------------------>
        lv_obj_set_style_bg_color(list_btn, lv_color_hex(player_style.btn_def_color), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(list_btn, lv_color_hex(player_style.btn_focus_color), LV_STATE_FOCUS_KEY);
        //ESP_LOGI("player_list", "list_btn i: %d --obj:%p", i, list_btn);

        list_btn_label = lv_label_create(list_btn);

        lv_obj_set_width(list_btn_label, LV_PCT(100));

        lv_obj_align(list_btn_label, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_long_mode(list_btn_label, LV_LABEL_LONG_DOT);
        lv_label_set_text(list_btn_label, gQjcd.pChapter[i]);

        lv_obj_set_style_text_color(list_btn, lv_color_hex(0x000000), LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(list_btn, lv_color_hex(0xffffff), LV_STATE_FOCUS_KEY);
        //lv_obj_add_event_cb(list_btn, play_chapter_list_btn_clicked_cb, LV_EVENT_CLICKED, NULL);
        if (i == active_item)
        {
            lv_obj_scroll_to_view(list_btn, LV_ANIM_OFF);
            lv_group_focus_obj(list_btn);
        }
    }
    return 0;
}
//------------------------------------------全解显示--------------------------------------------
static void replace_str(char* str, char to_search, char to_replace) {
    char* p;
    for (p = str; *p != '\0'; p++) {
        if (*p == to_search)
            *p = to_replace;
    }
}

static void add_quanjie_text_ques(cJSON* json, const char *hint)
{
    int cols = cJSON_GetArraySize(json);
    if (cols == 0)
        ESP_LOGI(PLA, "大小不对，应该大于零!\n");
    else {
        for (int16_t sub = 0; sub < cols; sub++) {
            cJSON* jsonQuesItm = cJSON_GetArrayItem(json, sub);
            lv_obj_t* list_btn_label = lv_label_create(browser);
            lv_obj_set_style_text_line_space(list_btn_label, BROWSER_LINE_STEP, 0);
            lv_obj_set_width(list_btn_label, 230);
            replace_str(jsonQuesItm->valuestring, '$', '_');
            if (sub == 0)
                lv_label_set_text_fmt(list_btn_label, "%s%s", hint, jsonQuesItm->valuestring);
             else
                lv_label_set_text(list_btn_label, jsonQuesItm->valuestring);
            lv_obj_set_style_pad_left(list_btn_label, 5, 0);
            lv_obj_set_style_align(list_btn_label, LV_ALIGN_CENTER, 0);
            lv_label_set_long_mode(list_btn_label, LV_LABEL_LONG_WRAP);
        }
    }
}

static void add_quesjie_text_string(const char* buf)
{
    lv_obj_t* list_btn_label = lv_label_create(browser);
    lv_obj_set_style_text_line_space(list_btn_label, BROWSER_LINE_STEP, 0);
    lv_obj_set_width(list_btn_label, 230);
    lv_label_set_recolor(list_btn_label, true);//先得使能文本重绘色功能
    lv_label_set_text(list_btn_label, buf);
    //lv_obj_set_style_bg_opa(list_btn_label, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(list_btn_label, lv_color_hex(0x322D65), 0);
    lv_obj_set_style_border_opa(list_btn_label, LV_OPA_30, LV_STATE_DEFAULT);
}

static int fill_ques_content(int16_t activeItem) {
    char quesIdBuf[40];
    if (activeItem >= gQjOtherData.item_sum) {
        ESP_LOGI(TAG, "题目超出范围了，最大值为：%d", gQjOtherData.item_sum);
        return -1;
    }

    gQjOtherData.active = activeItem;
    cJSON *jsonQuesItm = cJSON_GetArrayItem(gQjOtherData.ques, gQjOtherData.active);
    if (!jsonQuesItm)
        return -1;
    sprintf(quesIdBuf, "[题目%02d] ", gQjOtherData.active + 1);
    add_quesjie_text_string(quesIdBuf);
    
    // 题干
    cJSON *jsonQues = cJSON_GetObjectItem(jsonQuesItm, "ques");
    add_quanjie_text_ques(jsonQues, "");
    // 选项
    jsonQues = cJSON_GetObjectItem(jsonQuesItm, "options");
    if (jsonQues)
        add_quanjie_text_ques(jsonQues, "");
    // 答案
    jsonQues = cJSON_GetObjectItem(jsonQuesItm, "answer");
    if (jsonQues) {
        add_quesjie_text_string("\n[答案]");
        add_quanjie_text_ques(jsonQues, "");
    }
    // 分析
    jsonQues = cJSON_GetObjectItem(jsonQuesItm, "analysis");
    if (jsonQues) {
        add_quesjie_text_string("[分析]");
        add_quanjie_text_ques(jsonQues, "");
    }
    return 0;
}

static int play_quanjie_text_page_init(int16_t active_item)
{
    play_quanjie_text_page_free();
    active_chapter = active_item;
    play_quanjie_text_win = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(play_quanjie_text_win);
    lv_obj_set_size(play_quanjie_text_win, SCREEN_WIDTH, SCREEN_HEIGHT);

    sys_setCurMainWinObj(play_quanjie_text_win);
    lv_obj_add_event_cb(play_quanjie_text_win, play_quanjie_text_win_key_cb, LV_EVENT_KEY, NULL);
    // 背景
    lv_obj_t* img1 = lv_img_create(play_quanjie_text_win);
    lv_img_dsc_t* pImgDsc = zb_img_res_load_one(NULL, player_style.page_bg_img1, IMG_FREE_TYPE_AUTO, NULL);
    lv_img_set_src(img1, pImgDsc);

    // 标题
    title_label = lv_label_create(play_quanjie_text_win);
    lv_obj_set_width(title_label, 210);
    //lv_label_set_long_mode(title_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    ESP_LOGI(TAG, "%d", active_chapter);
    if (gQjcd.pChapter == NULL)
        ESP_LOGI(TAG, "gQjcd.pChapter is NULL; %s,%d", __FUNCTION__, __LINE__);
    lv_label_set_text(title_label, gQjcd.pChapter[active_chapter]);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_pos(title_label, 15, 15);

    // flex content obj
    if (false) {
        browser = lv_obj_create(play_quanjie_text_win);
        lv_obj_set_size(browser, SCREEN_WIDTH, 269);
        //lv_obj_set_style_pad_top(browser, 6, 0);
        //lv_obj_add_event_cb(play_quanjie_text_win, play_quanjie_text_win_key_cb, LV_EVENT_KEY, NULL);
        lv_obj_align(browser, LV_ALIGN_BOTTOM_MID, 0, 0);
        //lv_obj_set_flex_flow(uderbg_obj, LV_FLEX_FLOW_COLUMN);
        //lv_obj_set_scroll_dir(uderbg_obj, LV_DIR_VER);

        lv_obj_set_flex_flow(browser, LV_FLEX_FLOW_COLUMN); /* 设置主轴为垂直 */
        lv_obj_set_style_flex_grow(browser, 1, 0);       /* 让browser占满全部空间 */
        //lv_obj_set_scroll_dir(browser, LV_DIR_VER);
        lv_obj_set_scrollbar_mode(browser, LV_SCROLLBAR_MODE_ACTIVE);
        //lv_obj_set_scroll_bar_mode(flexview, LV_SCROLL_MODE_ON, LV_SCROLL_MODE_OFF); // 开启垂直滚动条
    }
    else {
        browser = lv_obj_create(play_quanjie_text_win);
        lv_obj_set_size(browser, SCREEN_WIDTH, 270);
        //lv_obj_align_to(browser, play_text_win, LV_ALIGN_TOP_MID, 0, 40);
        lv_obj_align(browser, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_style_pad_left(browser, 6, 0);
        lv_obj_set_style_pad_top(browser, 3, 0);
        //lv_obj_set_style_bg_opa(browser, LV_OPA_0, LV_STATE_DEFAULT);
        //lv_obj_set_style_border_opa(browser, LV_OPA_0, LV_STATE_DEFAULT);
        lv_obj_set_flex_flow(browser, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(browser, BROWSER_LINE_STEP, 0);
    }
    //// 显示等同于章节部分：外研教材的7个部分：
    char tmp_buf[ZB_MAX_PATH];
    sprintf(tmp_buf, "%s%s/%s", PLAYER_ONLINE_DATA_PATH, play_list_book_locate.path, play_list_book_info.pFileName[active_unit]);
    gQjOtherData.pData = LSNSPK_read_qj_data(tmp_buf, active_chapter+1, &gQjOtherData.item_sum, &gQjOtherData.size, NULL);
    //printf("path == %s\n", gQjOtherData.pData);

    if (gQjcd.item_style[active_chapter] != 1) {
        if (false) {
            lv_obj_t* text_orginal_label = lv_label_create(browser);
            lv_obj_set_style_text_line_space(text_orginal_label, BROWSER_LINE_STEP, 0);
            lv_obj_set_width(text_orginal_label, 208);
            lv_label_set_long_mode(text_orginal_label, LV_LABEL_LONG_WRAP);
            lv_obj_set_style_text_align(text_orginal_label, LV_TEXT_ALIGN_LEFT, 0);
            //lv_obj_set_style_text_color(text_orginal_label, lv_color_hex(player_style.text_def_color), LV_STATE_DEFAULT);
            lv_label_set_text_static(text_orginal_label, gQjOtherData.pData);
        }
        else {
            char* pSplite = NULL;
            char* p = gQjOtherData.pData;
            lv_obj_t* list_btn_label;
            int line = 0;
            do
            {
                pSplite = strchr(p, 0x0a);
                if (pSplite) {
                    *pSplite++ = 0;
                }
                line++;
                list_btn_label = lv_label_create(browser);
                lv_obj_set_style_text_line_space(list_btn_label, BROWSER_LINE_STEP, 0);
                lv_obj_set_width(list_btn_label, 230);
                lv_label_set_text(list_btn_label, p);
                lv_obj_set_style_pad_left(list_btn_label, 5, 0);
                lv_obj_set_style_align(list_btn_label, LV_ALIGN_CENTER, 0);
                lv_label_set_long_mode(list_btn_label, LV_LABEL_LONG_WRAP);
                if (pSplite)
                    p = pSplite;
                else
                    break;
            } while (true);
            lv_obj_set_style_pad_bottom(list_btn_label, BROWSER_LINE_STEP, 0);
            printf("总行数： %d\n", line);
        }
    }
    else { // Ques
        gQjOtherData.ques = cJSON_Parse(gQjOtherData.pData);

        gQjOtherData.item_sum = cJSON_GetArraySize(gQjOtherData.ques);
        if (gQjOtherData.item_sum == 0)
            ESP_LOGW(PLA, "全解数据 的 大小不对，只应至少有一个!\n");
        else {
            fill_ques_content(0);
        }
    }
    return 0;
}

int textbook_quanjie_all_free()
{
    ESP_LOGI(TAG, "release all resource, line = %d", __LINE__);
    play_quanjie_text_page_free();
    play_chapter_list_page_free();
    play_unit_list_page_free();
    return 0;
}
