#include "../utils/misc_utils.h"
#include "../data_parse/img_res.h"
#include "../include/img_a_tlb.h"
#include "../data_support/db_manager.h"
#include "../data_support/yd_book_select.h"
#include "ui_collect.h"
#include "../data_parse/cJSON.h"
#include "../data_parse/cJSON_Utils.h"
#include "../player/player_list.h"
#include "../ex_widget/app_msg.h"
#include <errno.h>
#include "../utils/style_tz.h"
#include "../mainmenu/mainmenu_base.h"
#ifndef C_PC_SIM
#include "../include/sqlite3.h"
#endif
#define BTN_TYPE_AUDIO "audio"
#define BTN_TYPE_BOOK "book"

#define COLLECT_PANEL_MUSIC_HEIGHT 320
#define COLLECT_PANEL_ALBUM_HEIGHT 350
#define BOOK_LIST_RADIUS_SIZE 12

#define TAG  "collect_D"
static lv_PageResource_Collection* collect = NULL;
//static CollectionService* s_collection_service = NULL;
//static CollectionItem* g_items = NULL;
static CollectionType current_type= COLLECTION_TYPE_MUSIC;
static int16_t  g_scrNo = -1;  // 当前屏号 
static bool bTest = true;

static char** g_selected_paths = NULL;
static uint32_t g_selected_count = 0;
static lv_ll_t *g_p_down_ll=NULL, g_down_ll;  //当前的下载列表
Playlist g_play_list;

static void export_db(int no);

void collect_screen_init(void);


extern const char* getDatumBasePath(const char* pcPath2ndPart);
extern int play_list_collect_page_init(char* path, char* title, int16_t ap_num);
extern int play_single_page_init(char* path, uint16_t play_num, int16_t ap_num, uint8_t creat_group);
static int play_recently_or_collect_page_init(char* path, uint16_t play_num, int16_t ap_num, uint8_t list_num, uint8_t creat_group);
static void create_collection_list(CollectionType type);
static void collect_switchScr(int8_t scrNoNew, int parm);
static void collect_list_free(void);
static void remove_collect_free(void);

// 定义屏幕初始化和释放函数指针类型
typedef int (*FN_TBCD_SCREEN_INIT)(int parm);
typedef int (*FN_TBCD_SCREEN_FREE)(void);

struct DIC_SCR_INF {
    FN_TBCD_SCREEN_INIT  m_fn_init;
    FN_TBCD_SCREEN_FREE  m_fn_free;
};


static void draw_boader(lv_obj_t* obj) {
    lv_obj_set_style_border_width(obj, 2, 0);
    lv_obj_set_style_border_color(obj, lv_color_black(), 0);
}

static bool judge_need_tip(bool bSave)
{
    char file_path[ZB_MAX_PATH];
    sprintf(file_path, "%s/%s", ZB_WORK_DIR, "download_fist_tip1.bin");

    if (bSave) {
        FILE* fp;
        fp = zb_fopen(file_path, "w");
        zb_fwrite("not_tip\t1\n",1, 10, fp);
        zb_fclose(fp);
        return false;
    }
    else {
        if (zb_isFileExist(file_path))
            return false;
        else
            return  true;
    }
}

static void free_pop_wnd() {
    if (collect->created_component != NULL) {
        lv_obj_del(collect->created_component);
        collect->created_component = NULL;
    }
}

int PageResource_collect(lv_event_code_t msg, int param)
{
    switch (msg) {
    case EVENT_FREE_AP_RES:
    {
        if (collect != NULL)
        {
            if (collect->ui_collect) {
                lv_obj_del(collect->ui_collect);  collect->ui_collect = NULL;
            }
            if (collect->cached_items != NULL) {
                collection_service_free_items_array(collect->cached_items, collect->total_count);
                collect->cached_items = NULL;
            }
            free_pop_wnd();
            lv_mem_free(collect);
            collect = NULL;
        }
        play_text_page_free();
        play_list_page_free();
        collect_list_free();
        remove_collect_free();
        ESP_LOGI("collect", "EVENT_FREE_AP_RES");
        break;
    }
    case EVENT_EX_CMD:
    {
        ESP_LOGI(TAG, "EVENT_CMD %d", param);
        if (PLY_CMD_PLY_TXT_RET == param || PLY_CMD_PLY_LST_RET == param) {
            collect_screen_init();
        }
        ESP_LOGI("collect", "EVENT_EX_CMD");
        break;
    }
    default:
        break;
    }
    current_type = COLLECTION_TYPE_MUSIC;
    return 0;
}

static void collect_main_scr_del(void) {
    if (collect != NULL)
    {
        if (collect->ui_collect) {
            lv_obj_del(collect->ui_collect);  collect->ui_collect = NULL;
        }
        lv_mem_free(collect);
        collect = NULL;
    }
    return;
}

//显示文件名
static char* extractFileName(const char* path) {
    // 查找最后一个反斜杠的位置
    const char* last_slash = strrchr(path, '/');
    const char* last_backslash = strrchr(path, '\\');
    const char* last_sep = (last_slash > last_backslash) ? last_slash : last_backslash;

    if (last_sep != NULL) {
        // 跳过最后一个反斜杠
        last_sep++;
        // 返回文件名
        return last_sep;
    }
    else {
        // 如果没有找到反斜杠，返回整个路径
        return path;
    }
}

static int create_sqlite3_test()
{
#ifndef C_PC_SIM
    sqlite3* db;
    char* zErrMsg = 0;
    int ret;

    ret = sqlite3_open("test.db", &db);   //打开或创建数据库

    if (ret) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));  //打印错误信息
        return -1;
    }
    else {
        fprintf(stderr, "Opened database successfully\n");
    }
    sqlite3_close(db);   //关闭数据库
#endif
}



static void collectKey_cb(lv_event_t* evt) {
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
            if (collect->created_component){
                free_pop_wnd();
            }
            else {
                if (g_scrNo != COLLECT_LIST_CONTENT) {
                    collect_switchScr(COLLECT_LIST_CONTENT, 0);
                }
                else {
                    zb_requestSwitchAp(C_ZB_AP_MAINMENU);
                    LV_LOG_USER("esc mainmenu\r\n");
                    lv_event_stop_processing(evt);
                }
            }
            break;

        case LV_KEY_ENTER:

          lv_indev_keypad_wait_release();

          //play_list_collect_page_init(Collection[Collection->currentIndex - g_curLine].path, Collection[Collection->currentIndex - g_curLine].title, C_ZB_AP_COLLECT);
          //collect_main_scr_del();

          //play_recently_or_collect_page_init(pcAlbumPath, unitIdx, C_ZB_AP_COLLECT, g_curLine, 0);
          //collect_main_scr_del();           
            break;


        case LV_KEY_EX_FUNC1:
        {
            LV_LOG_USER("LV_KEY_EX_FUNC1\r\n");
#ifndef C_PC_SIM
#if 0
            // 创建服务
            CollectionService* service = collection_service_create("/userdata/collections.db");

            // 添加本地音乐
            collection_service_add_item(//service,
                COLLECTION_TYPE_MUSIC,
                "/music/builtin/song1.mp3",
                "Built-in Song",
                0,
                MUSIC_SOURCE_BUILT_IN,
                true,
                C_ZB_AP_COLLECT,0);

            // 添加在线音乐
            collection_service_add_item(//service,
                COLLECTION_TYPE_MUSIC,
                "https://example.com/song2.mp3", "Online Song", 5, true, C_ZB_AP_COLLECT, 0);

            // 设置自定义排序
            collection_service_set_sort_order(//service,
                COLLECTION_TYPE_MUSIC,
                "playNum DESC, timestamp DESC");

            // 获取第一首歌曲
            const CollectionItem* item = collection_service_get_item(//service,
                COLLECTION_TYPE_MUSIC, 0, C_ZB_AP_COLLECT);
            //if (item) {
            //    printf("Playing: %s (%s)\n", item->title,
            //        item->is_online ? "Online" : "Local");
            //}
#endif
#else

            Playlist temp_plist = { 0 };
            temp_plist.count = 5;
            temp_plist.items = malloc(temp_plist.count * sizeof(PlaylistItem));


            // 网络资源项
            temp_plist.items[0] = create_playlist_item(
                RES_YD_ONLINE,
                "https://dl.espressif.com/dl/audio/ff-16b-2c-44100hz.mp3", 0,
                "Online Stream",
                -1
            );


            temp_plist.items[1] = create_playlist_item(
                RES_YD_ONLINE,
                "https://dl.espressif.com/dl/audio/ff-16b-2c-44100hz.mp3",0,
                "Online Stream 22",
                -1
            );

            temp_plist.items[2] = create_playlist_item(
                RES_YD_ONLINE,
                "https://dl.espressif.com/dl/audio/ff-16b-2c-44100hz.mp3", 0,
                "Online Stream 33",
                -1
            );
            temp_plist.items[3] = create_playlist_item(
                RES_YD_ONLINE,
                "https://dl.espressif.com/dl/audio/ff-16b-2c-44100hz.mp3", 0,
                "Online Stream 44",
                -1
            );
            temp_plist.items[4] = create_playlist_item(
                RES_YD_ONLINE,
                "https://dl.espressif.com/dl/audio/ff-16b-2c-44100hz.mp3", 0,
                "Online Stream 66",
                -1
            );

            if (play_album_online_page_init(&temp_plist, 4, C_ZB_AP_COLLECT, 1) != 0) {
                
            }

#endif
           
            break;
        }



        default:
            break;
        }
        break;
    case LV_EVENT_DELETE:
        if (collect) {
            collect->ui_collect = NULL;
        }
        break;
    default:
        break;
    }
}


// 主页面点击事件处理函数
static void page_click_event_cb(lv_event_t* e) {
    lv_obj_t* target = lv_event_get_target(e);

    free_pop_wnd();
}

static void btn_download_event_cb(lv_event_t* e) {

    collect_switchScr(COLLECT_DOWNLOAD_CONTENT,0);
}

static void btn_removecollect_event_cb(lv_event_t* e) {

    collect_switchScr(REMOEVE_COLLECT_CONTENT,0);
}



static lv_obj_t* create_setting_obj(lv_obj_t* parent) {
    if (collect->created_component) {
        lv_obj_del(collect->created_component);
        collect->created_component = NULL;
        return NULL;
    }

    //if (current_type == COLLECTION_TYPE_MUSIC)
    {
        const int COMPONENT_WIDTH = 104;
        const int COMPONENT_HEIGHT = 45;//91;
        collect->created_component = lv_obj_create(lv_scr_act());
        lv_obj_set_size(collect->created_component, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_opa(collect->created_component, LV_OPA_0, 0); // 50% 透明度
        lv_obj_set_style_bg_color(collect->created_component, lv_color_black(), 0);
        lv_obj_set_style_radius(collect->created_component, 0, 0);
        lv_obj_add_flag(collect->created_component, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(collect->created_component, page_click_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t *popup = lv_obj_create(collect->created_component);
        lv_obj_set_size(popup, COMPONENT_WIDTH, COMPONENT_HEIGHT);

        // 设置边框宽度
        lv_obj_set_style_border_width(popup, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(popup, lv_color_hex(0xffe7e7e7), LV_PART_MAIN | LV_STATE_DEFAULT);

        // 设置阴影
        lv_obj_set_style_shadow_width(popup, 4, LV_PART_MAIN | LV_STATE_DEFAULT); 
        lv_obj_set_style_shadow_color(popup, lv_color_hex(0xE7E7E7), LV_PART_MAIN | LV_STATE_DEFAULT); // 阴影颜色
        lv_obj_set_style_shadow_ofs_x(popup, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_ofs_y(popup, 4, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_align(popup, LV_ALIGN_TOP_LEFT, 196, 46);

        //lv_obj_add_event_cb(parent, page_click_event_cb, LV_EVENT_CLICKED, NULL);

        /*lv_obj_t* btn1 = lv_btn_create(popup);
        lv_obj_set_size(btn1, 100, 45);
        lv_obj_align(btn1, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_bg_color(btn1, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btn1, 0, LV_STATE_DEFAULT);
        lv_obj_set_style_radius(btn1, 0, LV_STATE_DEFAULT);
        lv_obj_add_flag(btn1, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(btn1, btn_download_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t* label = lv_label_create(btn1);
        lv_label_set_text(label, "下载");
        lv_obj_set_style_text_color(label, lv_color_hex(0x18191B), LV_PART_MAIN);
        lv_obj_center(label);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
        */
        lv_obj_t* btn2 = lv_btn_create(popup);
        lv_obj_set_size(btn2, 100, 45);
        lv_obj_align(btn2, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_style_bg_color(btn2, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btn2, 0, LV_STATE_DEFAULT);
        lv_obj_set_style_radius(btn2, 0, LV_STATE_DEFAULT);
        lv_obj_add_flag(btn2, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(btn2, btn_removecollect_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t* label2 = lv_label_create(btn2);
        lv_label_set_text(label2, "取消收藏");
        lv_obj_set_style_text_color(label2, lv_color_hex(0x18191B), LV_PART_MAIN);
        lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_center(label2);
    }
    /*else
    {
        const int COMPONENT_WIDTH = 104;
        const int COMPONENT_HEIGHT = 45;


        collect->created_component = lv_obj_create(parent);
        lv_obj_set_size(collect->created_component, COMPONENT_WIDTH, COMPONENT_HEIGHT);
        lv_obj_align(collect->created_component, LV_ALIGN_TOP_LEFT, 196, 46);


        lv_obj_add_event_cb(parent, page_click_event_cb, LV_EVENT_CLICKED, NULL);


        lv_obj_t* btn2 = lv_btn_create(collect->created_component);
        lv_obj_set_size(btn2, 100, 45);
        lv_obj_align(btn2, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_style_bg_color(btn2, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btn2, 0, LV_STATE_DEFAULT);
        lv_obj_set_style_radius(btn2, 0, LV_STATE_DEFAULT);
        lv_obj_add_flag(btn2, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(btn2, btn_removecollect_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t* label2 = lv_label_create(btn2);
        lv_label_set_text(label2, "取消收藏");
        lv_obj_set_style_text_color(label2, lv_color_hex(0x18191B), LV_PART_MAIN);
        lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_center(label2);
    }*/
    return collect->created_component;
}


static void esc_key_cb(lv_event_t* e) {
    
    LV_LOG_USER("click esc key \r\n");
    zb_requestSwitchAp(C_ZB_AP_MAINMENU);
}

static lv_obj_t* res_createEscBtn(lv_obj_t* parent, lv_event_cb_t clickCb, int extra)
{
#define RES_ESC_BTN_LOC_Y  0
#define TOP_BTN_BORDER 12
    lv_obj_t* img = lv_img_create(parent);
    const lv_img_dsc_t* pImgDsc = g_XXX_img_dsc_tbl + MMN_icn_esc - 1;
    lv_img_set_src(img, pImgDsc);
    lv_obj_set_size(img, pImgDsc->header.w + TOP_BTN_BORDER * 2, pImgDsc->header.h + TOP_BTN_BORDER * 2);
    lv_obj_set_pos(img, 20, RES_ESC_BTN_LOC_Y);
    lv_obj_set_style_border_width(img, TOP_BTN_BORDER, 0);
    lv_obj_set_style_radius(img, 4, 0);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_style_opa(img, LV_OPA_60, 0);
    lv_obj_set_style_opa(img, LV_OPA_100, LV_STATE_FOCUS_KEY);
    lv_obj_set_style_opa(img, LV_OPA_80, LV_STATE_PRESSED);
    // lv_color_t colorBorderFcs = lv_color_white();
    lv_obj_set_style_border_opa(img, LV_OPA_0, 0);
    lv_obj_add_event_cb(img, clickCb, LV_EVENT_CLICKED, NULL);

    return img;
}

static lv_obj_t* res_createTitle(lv_obj_t* parent,const char* text)
{
    lv_obj_t* title = lv_label_create(collect->ui_collect);
    lv_obj_set_width(title, LV_SIZE_CONTENT);
    lv_obj_set_height(title, 18);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 16);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(title, text);
    lv_obj_set_style_text_color(title, lv_color_hex(0x18191B), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(title, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(ui_title, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    return title;
}


static lv_obj_t* res_createMain(lv_obj_t* parent)
{
    lv_obj_t*main = lv_obj_create(parent);
    lv_obj_clear_flag(main, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(main, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_border_width(main, 0, 0);
    lv_obj_set_style_radius(main, 0, 0);
    lv_obj_set_style_bg_color(main, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(main, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    return main;
}


static void  create_no_content_show(lv_obj_t* parent,const char * text) {
#define FILE_ICON_BTN_LOC_X  73
#define FILE_ICON_BTN_LOC_Y  178 //(178-56)
#define TOP_BTN_BORDER 12

    //draw_boader(parent);
    lv_obj_t* img = lv_img_create(parent);
    //lv_obj_set_style_border_width(parent, 2, 0);
    //lv_obj_set_style_border_color(parent, lv_color_black(), 0);
    const lv_img_dsc_t* pImgDsc = zb_img_res_load_one(NULL, COLLECT_IMG_FILEICON, IMG_FREE_TYPE_AUTO, NULL);
    lv_img_set_src(img, pImgDsc);
    if (g_scrNo == COLLECT_DOWNLOAD_CONTENT || g_scrNo == REMOEVE_COLLECT_CONTENT) {
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, FILE_ICON_BTN_LOC_X,178-56-18 );
    }
    else {
        if (current_type == COLLECTION_TYPE_MUSIC)
            lv_obj_align(img, LV_ALIGN_TOP_LEFT, FILE_ICON_BTN_LOC_X, 0);
        else
            lv_obj_align(img, LV_ALIGN_TOP_LEFT, FILE_ICON_BTN_LOC_X, (COLLECT_PANEL_ALBUM_HEIGHT- COLLECT_PANEL_MUSIC_HEIGHT));
    }
    //lv_obj_align(img,LV_ALIGN_TOP_LEFT, FILE_ICON_BTN_LOC_X, FILE_ICON_BTN_LOC_Y);

    lv_obj_t* label = lv_label_create(img);
    lv_label_set_text(label, "暂无内容");
    lv_obj_align_to(label, img, LV_ALIGN_TOP_LEFT, 51, 145);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t* label2 = lv_label_create(parent);
    lv_label_set_text(label2, text);
    lv_obj_align_to(label2, img, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);  
    lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label2, lv_color_hex(0x787F87),0);

}




static void btn_event_cb(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    LV_LOG_USER("Button clicked!\n");

    create_setting_obj(collect->ui_collect);
}

static lv_obj_t* create_func(lv_obj_t* parent) {
#define DIAN_ESC_BTN_LOC_X  272
#define DIAN_ESC_BTN_LOC_Y  0
#define TOP_BTN_BORDER 12
    lv_obj_t* img = lv_img_create(parent);
    const lv_img_dsc_t* pImgDsc = zb_img_res_load_one(NULL, COLLECT_IMG_DIANDIANDIAN, IMG_FREE_TYPE_AUTO, NULL);
    lv_img_set_src(img, pImgDsc);
    lv_obj_set_size(img, pImgDsc->header.w + TOP_BTN_BORDER * 2, pImgDsc->header.h + TOP_BTN_BORDER * 2);
    lv_obj_align(img,LV_ALIGN_TOP_LEFT, DIAN_ESC_BTN_LOC_X, DIAN_ESC_BTN_LOC_Y);
    lv_obj_set_style_border_width(img, TOP_BTN_BORDER, 0);
    lv_obj_set_style_radius(img, 4, 0);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_style_opa(img, LV_OPA_60, 0);
    lv_obj_set_style_opa(img, LV_OPA_100, LV_STATE_FOCUS_KEY);
    lv_obj_set_style_opa(img, LV_OPA_80, LV_STATE_PRESSED);
    // lv_color_t colorBorderFcs = lv_color_white();
    lv_obj_set_style_border_opa(img, LV_OPA_0, 0);
    lv_obj_add_event_cb(img, btn_event_cb, LV_EVENT_CLICKED, NULL);

    return img;
}


static void toggle_button_event(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* btn = lv_event_get_target(e);
    const char* btn_type = (const char*)lv_event_get_user_data(e); 

    if (code == LV_EVENT_CLICKED) {

        lv_obj_t* other_btn = (strcmp(btn_type, BTN_TYPE_AUDIO) == 0) ? collect->btn_book : collect->btn_audio;

        if (!lv_obj_has_state(btn, LV_STATE_CHECKED)) {
            lv_obj_add_state(btn, LV_STATE_CHECKED);
            lv_obj_clear_state(other_btn, LV_STATE_CHECKED);
        }

        current_type = (strcmp(btn_type, BTN_TYPE_AUDIO) == 0) ? COLLECTION_TYPE_MUSIC : COLLECTION_TYPE_ALBUM;
        if (current_type == COLLECTION_TYPE_MUSIC)
            lv_obj_set_height(collect->collect_Panel, COLLECT_PANEL_MUSIC_HEIGHT);
        else
            lv_obj_set_height(collect->collect_Panel, COLLECT_PANEL_ALBUM_HEIGHT);

        lv_obj_update_layout(collect->collect_Panel);

        create_collection_list(current_type);
    }
}

static lv_obj_t* create_toggle_button(lv_obj_t* parent,const char* text, int x, int y,const char* btn_type) {
    lv_obj_t* btn = lv_obj_create(parent);
    lv_obj_set_height(btn, 40);
    lv_obj_set_width(btn, 100);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, x, y);

    lv_obj_set_style_bg_color(btn, lv_color_hex(0xEEEFF1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(btn, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(btn, 90, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(btn, lv_color_hex(0x008CFF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_CHECKED);

    lv_obj_add_style(btn, get_global_style(STYLE_BTN_BASE), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_center(label);
 
    lv_obj_add_event_cb(btn, toggle_button_event, LV_EVENT_CLICKED, (void*)btn_type);
    return btn;
}

static void play_all_btn_click_cb(lv_event_t* e)
{
    ESP_LOGI(TAG, "etner....play_all_btn_click_cb\n");
    create_play_list(collect);
    play_album_online_page_init(&g_play_list, 0, C_ZB_AP_COLLECT, true);
}

// TODO 后续需要确认 g_play_list 是否在播放模块释放
int create_play_list(lv_PageResource_Collection* clt) {
    g_play_list.count = clt->total_count;
    g_play_list.current_index = 0;
    g_play_list.items = malloc(g_play_list.count * sizeof(PlaylistItem));
    
    CollectionItem* item;
    for (int i = 0; i < clt->total_count; i++) {
        item = &clt->cached_items[i];
        g_play_list.items[i].source_type = item->source_type;
        g_play_list.items[i].cover = -1;
        g_play_list.items[i].playNum = item->playNum;

        if (item->path) {
            g_play_list.items[i].location = strdup(item->path);
        }

        if (item->title) {
            g_play_list.items[i].title = strdup(item->title);
        }
        ESP_LOGE(TAG, "Insert【%d】:path=%s,title=%s,type=%d\n", i, item->path, item->title, item->source_type);
    }
    return g_play_list.count;
}

static void play_item_btn_click_cb(lv_event_t* e)
{
    //lv_event_code_t code = lv_event_get_code(e);
    //lv_obj_t* btn = lv_event_get_target(e);
    //lv_obj_t* label = lv_obj_get_child(btn, 0); // 获取第一个子对象（标签）

    int posi = (int)(intptr_t)lv_event_get_user_data(e);


    ESP_LOGI(TAG, "etner....play_item_btn_click_cb\n");
    //ESP_LOGI(TAG, "path=%s, title=%s, playNum=%d\n", item->path, item->title, item->source_type);

    //if (code == LV_EVENT_FOCUSED) {
    //    // 手动设置标签颜色（如果样式不生效）
    //    lv_obj_set_style_text_color(label, lv_color_hex(REMOVE_LABLE_COLOR_SELECT), 0);
    //}
    //else if (code == LV_EVENT_DEFOCUSED) {
    //    lv_obj_set_style_text_color(label, lv_color_hex(REMOVE_LABLE_COLOR_DEFAULT), 0);
    //}
    //else if (code == LV_EVENT_CLICKED) {
        //return;
        create_play_list(collect);
        g_play_list.current_index = posi;
        play_album_online_page_init(&g_play_list, posi, C_ZB_AP_COLLECT, true);
		 //lv_indev_wait_release(lv_indev_get_act());
    //}
}



static lv_obj_t* create_player_btn(lv_obj_t* parent)
{
#define btn_size 50
    /* 全部播放按钮 */
    lv_obj_t* all_play = lv_obj_create(parent);
    lv_obj_set_size(all_play, LV_PCT(60), 50);
    lv_obj_set_style_bg_color(all_play, lv_color_hex(0x1A143B), 0);
    lv_obj_set_style_bg_opa(all_play, LV_OPA_0, 0);
    lv_obj_set_style_border_width(all_play, 0, 0);
    lv_obj_set_style_shadow_width(all_play, 0, 0);
    lv_obj_clear_flag(all_play, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* all_play_btn = lv_obj_create(all_play);
    lv_obj_set_size(all_play_btn, btn_size, btn_size);
    lv_obj_set_style_bg_color(all_play_btn, lv_color_hex(0x1A143B), 0);
    lv_obj_set_style_bg_opa(all_play_btn, LV_OPA_0, 0);
    lv_obj_set_style_border_width(all_play_btn, 0, 0);
    lv_obj_set_style_shadow_width(all_play_btn, 0, 0);
    lv_img_dsc_t* all_play_btn_pImgDsc = zb_img_res_load_one(NULL, COLLECT_IMG_PLAY_BTN, IMG_FREE_TYPE_AUTO, NULL);
    lv_obj_set_style_bg_img_src(all_play_btn, all_play_btn_pImgDsc, 0);
    lv_obj_align(all_play_btn, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_event_cb(all_play_btn, play_all_btn_click_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* btn_label = lv_label_create(all_play);
    //lv_label_set_text(btn_label, "全部播放(8)");
    lv_label_set_text_fmt(btn_label, "继续播放（%d）", collect->total_count);
    //lv_obj_add_style(btn_label, &title_style, 0);
    //lv_style_set_text_color(&title_style, lv_color_hex(0xffffff));
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0x18191B),0);
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0x008CFF), LV_STATE_FOCUS_KEY);
    lv_obj_align_to(btn_label, all_play_btn, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

    return all_play;
}
#ifndef C_PC_SIM
// rsh_tbl test
static void display_tbl() {
    ESP_LOGE(TAG, "enter :display_tbl\n");
    CollectionItem* items = NULL;

    CollectionFilter filter = {
        .source_type = RES_YD_ONLINE,
    };
    collect->cached_items = collection_service_get_all_items_with_filter(COLLECTION_TYPE_MUSIC,
        C_ZB_AP_COLLECT, &filter, &collect->total_count);
    //int count = collection_service_get_items( COLLECTION_TYPE_MUSIC, &items, 1, C_ZB_AP_COLLECT);
    ESP_LOGE(TAG, "query data:%d\n", collect->total_count);
    if (collect->total_count > 0) {
        for (int i = 0; i < collect->total_count; i++) {
            ESP_LOGE(TAG, "::::::%d\n", i);
            printf("Item %d: %s (%s)\n", i, collect->cached_items[i].title, collect->cached_items[i].path);
        }
    }
    collection_service_free_items_array(collect->cached_items, collect->total_count);
}

static void test_insert_record()
{
#ifndef C_PC_SIM
    collection_service_add_item(COLLECTION_TYPE_MUSIC, "abc/adfa/01", "rsh_title", 1, RES_YD_ONLINE, 1, C_ZB_AP_COLLECT, 0);
    collection_service_add_item(COLLECTION_TYPE_MUSIC, "abc/adfa/02", "rsh_title", 1, RES_CG_ONLINE, 1, C_ZB_AP_COLLECT, 0);
    collection_service_add_item(COLLECTION_TYPE_MUSIC, "abc/adfa/03", "特色_title", 1, RES_YD_ONLINE, 1, C_ZB_AP_COLLECT, 0);
    collection_service_add_item(COLLECTION_TYPE_MUSIC, "abc/adfa/04", "特色_title", 1, RES_CG_ONLINE, 1, C_ZB_AP_COLLECT, 0);
#else
    //printf("");
#endif
}

#endif

//extern int test_CollectionItem_items(CollectionItem** out_items);
static void add_test_download_screen_data()
{
    if (true)
        return;
    //download_start("2024西南科技大学各专业录取分数线一览表yd_title 1", "abc/yd1.mp3", RES_YD_ONLINE);
    //download_start("yd_title 2", "abc/yd2.mp3", RES_YD_ONLINE);
    //download_start("yd_title 3", "abc/yd3.mp3", RES_YD_ONLINE);
    //download_start("cg_online 1", "box/hjb_4_1", RES_CG_ONLINE);
    //download_start("cg_online 2", "box/hjb_4_2", RES_CG_ONLINE);
    
    //download_finished("2024西南科技大学各专业录取分数线一览表yd_title 1", "abc/yd1.mp3", RES_YD_ONLINE, DOWNLOAD_STATUS_COMPLETED);
    //download_finished("yd_title 2", "abc/yd2.mp3", RES_YD_ONLINE, DOWNLOAD_STATUS_COMPLETED);
    
    //download_finished("cg_online 1", "box/hjb_4_1", RES_CG_ONLINE, DOWNLOAD_STATUS_COMPLETED);
    //download_finished("cg_online 2", "box/hjb_4_2", RES_CG_ONLINE, DOWNLOAD_STATUS_COMPLETED);
    //char extport_path[300] = { "/userdata/_data/rrr_db.json" };
    //collection_service_export_to_json(C_ZB_AP_COLLECT,(const char*) extport_path);
    //print_list();
}

void collect_screen_init(void)
{
    //download_start("rsh", "box/人教统编版/tape3a_002001_202412251537", RES_CG_ONLINE);
    //download_finished("rsh", "box/人教统编版/tape3a_002001_202412251537", RES_CG_ONLINE, 2);
    collect = lv_mem_alloc(sizeof(lv_PageResource_Collection));
    memset(collect, 0, sizeof(lv_PageResource_Collection));

    collect->current_screen = COLLECT_LIST_CONTENT; // 设置页面标识

    collect->ui_collect = res_createMain(lv_scr_act());

    res_createEscBtn(collect->ui_collect, esc_key_cb, 0);

    collect->ui_title = res_createTitle(collect->ui_collect,"我的收藏");

    collect->btn_audio = create_toggle_button(collect->ui_collect, "音频", 12, 68, BTN_TYPE_AUDIO);

    collect->btn_book = create_toggle_button(collect->ui_collect, "图书", 132, 68, BTN_TYPE_BOOK);

    create_func(collect->ui_collect);

    collect->collect_Panel = lv_obj_create(collect->ui_collect);
    lv_obj_set_width(collect->collect_Panel, SCREEN_WIDTH);
    //lv_obj_set_height(collect->collect_Panel, COLLECT_PANEL_MUSIC_HEIGHT);  // 125+播放列表
    if (current_type == COLLECTION_TYPE_MUSIC)
        lv_obj_set_height(collect->collect_Panel, COLLECT_PANEL_MUSIC_HEIGHT);
    else
        lv_obj_set_height(collect->collect_Panel, COLLECT_PANEL_ALBUM_HEIGHT);

    lv_obj_align(collect->collect_Panel, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(collect->collect_Panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(collect->collect_Panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_row(collect->collect_Panel, 12, LV_PART_MAIN);
    lv_obj_set_flex_align(collect->collect_Panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_border_width(collect->collect_Panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(collect->collect_Panel, 5, LV_PART_MAIN);

    add_test_download_screen_data(); // TEST code

    create_collection_list(current_type);

    g_scrNo = COLLECT_LIST_CONTENT;

#ifndef C_PC_SIM
    if (0) // TEST 数据保存
    {
        if (false) {
            char extport_path[400] = { 0 };
            sprintf(extport_path, "/userdata/collection_json");
            ESP_LOGE(TAG, "Save data: %s\n", extport_path);
            collection_service_add_item(COLLECTION_TYPE_MUSIC, "abc/adfa/56", "rsh_title", 1, RES_YD_ONLINE, 1, C_ZB_AP_COLLECT, 0);
            collection_service_add_item(COLLECTION_TYPE_MUSIC, "abc/adfa/78", "rsh_title", 1, RES_CG_ONLINE, 1, C_ZB_AP_COLLECT, 0);
            /*bool collection_service_export_to_json(
                CollectionService * service,
                CollectionType type,
                const char* json_path);*/
        }
        else
        {
        }
                // 使用后释放内存
                //collection_service_free_items_array(items, collect->total_count);
    }
#endif

    if(current_type == COLLECTION_TYPE_MUSIC)
        lv_obj_add_state(collect->btn_audio, LV_STATE_CHECKED);
    else
        lv_obj_add_state(collect->btn_book, LV_STATE_CHECKED);
    //sys_setCurMainWinObj(collect->collect_Panel);
    lv_obj_add_event_cb(collect->ui_collect, collectKey_cb, LV_EVENT_ALL, NULL);
    //lv_group_add_obj(lv_group_get_default(), collect->ui_collect);
  

    sys_setCurMainWinAndCb(collect->ui_collect, collectKey_cb);
    // lv_obj_add_event_cb(collect->ui_choosebtn, choosebtnKey_cb, LV_EVENT_ALL, NULL);

}



// 图片资源释放函数
static void release_image(lv_img_dsc_t* img) {
    if (img) {
        if (img->data) {
            heap_caps_free((void*)img->data);
        }
        heap_caps_free(img);
    }
}

// 图片缩放回调事件
static void img_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* img = lv_event_get_target(e);

    if (code == LV_EVENT_DELETE) {
        lv_img_dsc_t* img_dsc = (lv_img_dsc_t*)lv_event_get_user_data(e);
        if (img_dsc) {
            release_image(img_dsc);
        }
    }
}



// 自定义图片加载函数
static lv_img_dsc_t* zb_load_image_from_file_test(const char* pcPath) {
    lv_img_dsc_t* stImgDsc = NULL;
    FILE* fp = NULL;

    fp = zb_fopen(pcPath, "rb");
    if (!fp) {
        ESP_LOGE("IMG", "Fail to open file %s", pcPath);
        return NULL;
    }

    // 分配图片描述符
    stImgDsc = heap_caps_malloc_dflt(sizeof(lv_img_dsc_t));
    if (!stImgDsc) {
        ESP_LOGE("IMG", "Failed to allocate image descriptor");
        zb_fclose(fp);
        return NULL;
    }

    // 读取图片头部信息
    size_t read_bytes = zb_fread(stImgDsc, 1, 8, fp);

    // 验证数据大小
    if (stImgDsc->data_size < 1 || stImgDsc->data_size > 1024 * 400) {
        ESP_LOGE("IMG", "Invalid image size: %u", stImgDsc->data_size);
        heap_caps_free(stImgDsc);
        zb_fclose(fp);
        return NULL;
    }

    // 分配图片数据内存
    void* imgBuf = heap_caps_malloc_dflt(stImgDsc->data_size);
    if (!imgBuf) {
        ESP_LOGE("IMG", "Failed to allocate image buffer");
        heap_caps_free(stImgDsc);
        zb_fclose(fp);
        return NULL;
    }

    // 读取图片数据
    read_bytes = zb_fread(imgBuf, 1, stImgDsc->data_size, fp);
    if (read_bytes != stImgDsc->data_size) {
        ESP_LOGE("IMG", "Failed to read image data (expected %u, got %u)",
            stImgDsc->data_size, read_bytes);
        heap_caps_free(imgBuf);
        heap_caps_free(stImgDsc);
        zb_fclose(fp);
        return NULL;
    }

    // 绑定数据到描述符
    stImgDsc->data = imgBuf;
    zb_fclose(fp);

    return stImgDsc;
}

static void lv_img_zoom(lv_obj_t* parent, const char* imgPath, lv_area_t area) {
    // 加载图片资源
    lv_img_dsc_t* img_Src = zb_load_image_from_file_test(imgPath);
    if (!img_Src) {
        ESP_LOGE("IMG", "Failed to load image: %s", imgPath);
        return;
    }

    // 计算目标区域尺寸
    int32_t zreaW = lv_area_get_width(&area);
    int32_t zreaH = lv_area_get_height(&area);

    // 计算缩放比例
    float scale_x = (float)zreaW / img_Src->header.w;
    float scale_y = (float)zreaH / img_Src->header.h;
    float scale = fminf(scale_x, scale_y);

    // 创建图片对象
    lv_obj_t* img = lv_img_create(parent);
    lv_img_set_src(img, img_Src);

    // 设置缩放
    lv_img_set_zoom(img, (uint16_t)(256 * scale));

    // 设置位置
    lv_obj_set_pos(img, area.x1, area.y1);

    // 设置透明效果
    lv_obj_set_style_img_opa(img, LV_OPA_70, LV_STATE_PRESSED);

    // 添加删除事件回调以释放资源
    lv_obj_add_event_cb(img, img_event_cb, LV_EVENT_DELETE, img_Src);

    // 计算并设置新尺寸（可选，用于布局）
    uint16_t scaled_w = (uint16_t)(img_Src->header.w * scale);
    uint16_t scaled_h = (uint16_t)(img_Src->header.h * scale);
    lv_obj_set_size(img, scaled_w, scaled_h);

    // 居中显示（如果需要）
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
}


static void dl_query_db(CollectionType  type, uint16_t appNo)
{
    if (collect->cached_items != NULL)
    {
        // 使用后释放内存
        collection_service_free_items_array(collect->cached_items, collect->total_count);
        collect->cached_items = NULL;
    }

#ifndef C_PC_SIM
        collect->cached_items = collection_service_get_all_items_with_filter(type,
            appNo, NULL, &collect->total_count);
        ESP_LOGI(TAG, "%d:查询结果数量：%d\n", type, collect->total_count);
#else
    // 添加PC的测试数据
    collect->total_count = 5;
    collect->cached_items = malloc(collect->total_count * sizeof(CollectionItem));
    CollectionItem* items = collect->cached_items;

    memset(items, 0, collect->total_count * sizeof(CollectionItem));

    // 读取数据
    uint32_t index = 0;
    for (int i = 0; i<collect->total_count;i ++) {
        // 复制字符串字段
        const char path[300];
        if (type == COLLECTION_TYPE_MUSIC)
            sprintf(path, "music path  %d", i + 1);
        else
            sprintf(path, "album path  %d", i + 1);

        items[index].path = strdup(path);

        if (type == COLLECTION_TYPE_MUSIC) {
            if (i % 2 == 0)
                sprintf(path, "music title  %d", i + 1);
            else
                sprintf(path, "music title music titlemusic titlemusic title %d", i + 1);
        }
        else
            sprintf(path, "album title  album title  album title  album title  %d", i + 1);
        items[index].title =strdup(path);

        // 复制其他字段
        items[index].playNum = 1;
        items[index].timestamp = 1713413143;
        items[index].source_type = 3;
        items[index].is_downloaded = 1;
        items[index].appNo = C_ZB_AP_COLLECT;

        index++;
    }
#endif
    return;
}

// 焦点可视化
static void focus_debug_event(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (e->code == LV_EVENT_FOCUSED) {
        lv_obj_set_style_border_color(e->target, lv_color_hex(0xFF0000), 0);
    }
    else if (e->code == LV_EVENT_DEFOCUSED) {
        lv_obj_set_style_border_color(e->target, lv_color_hex(0x000000), 0);
    }
}


// 创建列表的函数
static void create_collection_list(CollectionType type) {
    
    if(collect->collect_Panel) lv_obj_clean(collect->collect_Panel);
    if (collect->player_obj)
    {
        lv_obj_del(collect->player_obj);
        collect->player_obj = NULL;
    }
    
    dl_query_db(type, C_ZB_AP_COLLECT);  // 查询数据库表
    //collect->total_count = 0;
    if (collect->total_count == 0)
    {
        ESP_LOGI(TAG, "没有找到一条\n");
        lv_obj_t* new_container = lv_obj_create(collect->collect_Panel);
        lv_obj_set_size(new_container, LV_PCT(100), LV_PCT(100));
        lv_obj_align(new_container, LV_ALIGN_TOP_LEFT, 10, 10);
        lv_obj_set_style_border_width(new_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_no_content_show(new_container, "您还没有收藏内容，去看看别的吧~");
        return;
    }

    if (type == COLLECTION_TYPE_MUSIC)
    {
        ESP_LOGE(TAG, "export data:%d\n", collect->total_count);
        if (collect->total_count > 0) {
            collect->player_obj = create_player_btn(collect->ui_collect);
            lv_obj_align_to(collect->player_obj, collect->collect_Panel, LV_ALIGN_OUT_TOP_LEFT, 0, 0);
            for (int i = 0; i < collect->total_count; i++) {
                ESP_LOGE(TAG, "::::::%d", i);
                printf("Item %d: %s (%s)\n", i, collect->cached_items[i].title, collect->cached_items[i].path);
                lv_obj_t* btn = lv_obj_create(collect->collect_Panel);
                lv_group_add_obj(lv_group_get_default(), btn);
                lv_obj_set_size(btn, 296, 76);
                //lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
                lv_obj_add_style(btn, get_global_style(STYLE_BTN_BASE), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(btn, lv_color_hex(REMOVE_LABLE_COLOR_BG), 0);
                //lv_obj_set_style_text_color(btn, lv_color_hex(REMOVE_LABLE_COLOR_DEFAULT), 0);
                //lv_obj_set_style_text_color(btn, lv_color_hex(REMOVE_LABLE_COLOR_SELECT), LV_STATE_FOCUS_KEY);
                //lv_obj_add_event_cb(btn, play_item_btn_click_cb, LV_EVENT_CLICKED, &collect->cached_items[i]);
                //lv_obj_add_event_cb(btn, play_item_btn_click_cb, LV_EVENT_ALL, &collect->cached_items[i]);
                lv_obj_add_event_cb(btn, play_item_btn_click_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);
                //lv_obj_add_event_cb(btn, play_item_btn_click_cb, LV_EVENT_FOCUSED| LV_EVENT_DEFOCUSED| LV_EVENT_CLICKED, &collect->cached_items[i]);
                //lv_obj_add_event_cb(btn, focus_debug_event, LV_EVENT_ALL, NULL);

                lv_obj_t* label = lv_label_create(btn);
                lv_label_set_text(label, collect->cached_items[i].title);
                lv_obj_set_width(label, SCREEN_WIDTH - 76);
                lv_obj_set_height(label, 25);
                lv_obj_set_style_text_color(label, lv_color_hex(REMOVE_LABLE_COLOR_DEFAULT), 0);
                lv_obj_set_style_text_color(label, lv_color_hex(REMOVE_LABLE_COLOR_SELECT), LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);
                //lv_obj_set_style_text_color(label, lv_color_hex(0xFF0000), LV_STATE_HOVERED);
                //lv_obj_set_style_text_color(label, lv_color_hex(0x0000FF), LV_STATE_PRESSED);    // 按下蓝色

                lv_obj_set_style_text_align(label, LV_ALIGN_LEFT_MID, 0);
                lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
                lv_obj_align(label, LV_ALIGN_OUT_RIGHT_MID, 52, 12);

                lv_obj_t* label_time = lv_label_create(btn);
                uint32_t sndTime = collect->cached_items[i].sndTime;
                lv_label_set_text_fmt(label_time, "%02d:%02d", (uint32_t)(sndTime/60), sndTime % 60);
                lv_obj_set_style_text_color(label_time, lv_color_hex(REMOVE_LABLE_COLOR_TIME), 0);
                lv_obj_set_style_text_color(label_time, lv_color_hex(REMOVE_LABLE_COLOR_TIME), LV_STATE_FOCUS_KEY);
                lv_obj_align(label_time, LV_ALIGN_OUT_RIGHT_MID, 52, 41);

                //draw_boader(label);
                //draw_boader(label_time);

                lv_obj_t* img = lv_img_create(btn);
                const lv_img_dsc_t* pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_BOOK_RDYPLAY, IMG_FREE_TYPE_AUTO, NULL);
                lv_img_set_src(img, pImgDsc);
                lv_obj_align(img, LV_ALIGN_LEFT_MID, 16, 0);
            }
        }

    }
    else if (type == COLLECTION_TYPE_ALBUM)
    {
        for (int i = 0; i < collect->total_count; i++)
        {
            if (false) {
                lv_obj_t* item = lv_obj_create(collect->collect_Panel);
                lv_obj_set_size(item, LV_PCT(100), 80);
                lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);  // 水平布局
                lv_obj_set_style_pad_all(item, 10, 0);         // 内边距
                lv_obj_set_style_pad_column(item, 15, 0);      // 图标与文本间距
                lv_obj_remove_style(item, NULL, LV_PART_SCROLLBAR);

                // 创建图标
                lv_obj_t* img = lv_img_create(item);


                ESP_LOGI(TAG, "当前的封面是：%s\n", collect->cached_items[i].path);
                char buf[400] = { 0 };
                //box/人教PEP版/tape2b_002015_202503182007
                //"bk": "boll/bjb/bjb_1_2"
                char* pName = extractFileName(collect->cached_items[i].path);
                if (memcmp(collect->cached_items[i].path, "boll/", 5) == 0) {
                    sprintf(buf, "/%s/_data/textbook/cover/%s.bin", YD_BOOK_PATH, pName);
                }
                else {
                    char* last_dash = strrchr(pName, '_');
                    if (last_dash) {
                        *last_dash = 0;
                        sprintf(buf, "/%s/_data/textbook/cover/%s.bin", YD_BOOK_PATH, pName);
                        *last_dash = '_';
                    }
                    else
                        sprintf(buf, "/%s/_data/textbook/cover/wyjqbyy_3_1.bin", YD_BOOK_PATH);
                }
                ESP_LOGI(TAG, "处理后的的封面是：%s\n", buf);
                if (!zb_isFileExist(buf))
                    sprintf(buf, "/%s/_data/textbook/cover/wyjqbyy_3_1.bin", YD_BOOK_PATH);


                lv_img_dsc_t* img_Src = zb_load_image_from_file(buf);
                lv_img_set_src(img, img_Src);

                static lv_style_t style;
                lv_style_init(&style);
                float scale = 0.7; // 缩小30%
                lv_style_set_transform_zoom(&style, 256 * scale);
                lv_style_set_radius(&style, 8);

                // 设置变换中心点（百分比）
                lv_style_set_transform_pivot_x(&style, 50);
                lv_style_set_transform_pivot_y(&style, 50);

                lv_obj_add_style(img, &style, 0);
                //lv_obj_center(img);

                // 仍然需要调整大小
                lv_coord_t w = img_Src->header.w * scale;
                lv_coord_t h = img_Src->header.h * scale;
                lv_obj_set_size(img, w, h);

                // 关键：使图标垂直居中
                lv_obj_set_align(img, LV_ALIGN_CENTER);

                // 创建文本容器（用于多行文本居中）
                lv_obj_t* text_cont = lv_obj_create(item);
                lv_obj_set_flex_grow(text_cont, 1);            // 占据剩余宽度
                lv_obj_set_height(text_cont, LV_PCT(100));     // 100%高度
                lv_obj_set_style_bg_opa(text_cont, LV_OPA_TRANSP, 0);
                lv_obj_set_style_border_opa(text_cont, LV_OPA_TRANSP, 0);
                lv_obj_set_style_pad_all(text_cont, 0, 0);

                // 关键：垂直居中布局
                lv_obj_set_flex_flow(text_cont, LV_FLEX_FLOW_COLUMN);  // 垂直布局
                lv_obj_set_flex_align(text_cont, LV_FLEX_ALIGN_CENTER, // 交叉轴居中
                    LV_FLEX_ALIGN_START,
                    LV_FLEX_ALIGN_CENTER);

                // 创建多行文本标签
                lv_obj_t* label = lv_label_create(text_cont);
                lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);     // 自动换行
                lv_label_set_text(label, collect->cached_items[i].title);
                lv_obj_set_width(label, LV_PCT(100));                  // 100%宽度
            }
            else
            {
                lv_obj_t* btn = lv_obj_create(collect->collect_Panel);
                lv_obj_set_size(btn, lv_pct(100), 80);
                lv_obj_add_style(btn, get_global_style(STYLE_BTN_BASE), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(btn, lv_color_hex(0xF5F7FA), 0);
                lv_obj_set_style_bg_opa(btn, 0, 0);
                lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
                lv_obj_add_event_cb(btn, play_item_btn_click_cb, LV_EVENT_CLICKED, NULL);

                //draw_boader(btn);

                lv_obj_t* img = lv_img_create(btn);
                if (0) {
                    ESP_LOGI(TAG, "当前的封面是：%s\n", collect->cached_items[i].path);
                    char buf[400] = { 0 };
                    //box/人教PEP版/tape2b_002015_202503182007
                    //    //"bk": "boll/bjb/bjb_1_2"
                    char* pName = extractFileName(collect->cached_items[i].path);
                    if (memcmp(collect->cached_items[i].path, "boll/", 5) == 0) {
                        sprintf(buf, "/%s/_data/textbook/cover/%s.bin", YD_BOOK_PATH, pName);
                    }
                    else {
                        char* last_dash = strrchr(pName, '_');
                        if (last_dash) {
                            *last_dash = 0;
                            sprintf(buf, "/%s/_data/textbook/cover/%s.bin", YD_BOOK_PATH, pName);
                            *last_dash = '_';
                        }
                        else
                            sprintf(buf, "/%s/_data/textbook/cover/wyjqbyy_3_1.bin", YD_BOOK_PATH);
                    }
                    ESP_LOGI(TAG, "处理后的的封面是：%s\n", buf);
                    if (!zb_isFileExist(buf))
                        sprintf(buf, "/%s/_data/textbook/cover/wyjqbyy_3_1.bin", YD_BOOK_PATH);

                    lv_img_dsc_t* img_Src = zb_load_image_from_file(buf);
                    lv_img_set_src(img, img_Src);
                    lv_obj_set_style_radius(img, 8, 0);
                    //lv_obj_set_style_border_width(img, 3, 0);
                    //lv_obj_set_style_border_color(img, lv_color_black(), 0);

                    static lv_style_t style;
                    lv_style_init(&style);
                    float scale = 0.5; // 缩小30%
                    lv_style_set_transform_zoom(&style, 256 * scale);
                    lv_style_set_radius(&style, 8);

                    // 设置变换中心点（百分比）
                    lv_style_set_transform_pivot_x(&style, 50);
                    lv_style_set_transform_pivot_y(&style, 50);

                    lv_obj_add_style(img, &style, 0);

                    // 仍然需要调整大小
                    lv_coord_t w = img_Src->header.w * scale;
                    lv_coord_t h = img_Src->header.h * scale;
                    lv_obj_set_size(img, w, h);

                    lv_obj_align(img, LV_ALIGN_LEFT_MID, 0, 0);
                    lv_obj_set_style_radius(img, BOOK_LIST_RADIUS_SIZE, LV_PART_MAIN);
                    lv_obj_set_style_clip_corner(img, BOOK_LIST_RADIUS_SIZE, LV_PART_MAIN);

                    //draw_boader(img);
                }
                else {
                    char buf[400] = { 0 }, * p = buf;
                    int pErrNo;

                    
                    if (!get_cg_album_path(collect->cached_items[i].path, &p)) {
#ifdef C_PC_SIM
                        get_cg_album_path("box/人教PEP版/tape1a_002015_202503241812", &p);
#endif
                    }
                    ESP_LOGI(TAG, "cover _path: %s\n", buf);
                    //lv_img_dsc_t* img_Src = LSNSPK_get_cover_data("/userdata/_data/textbook/box/人教PEP版/tape1a_002015_202503241812/book000/album.mnu", &pErrNo);
                    lv_img_dsc_t* img_Src = LSNSPK_get_cover_data(buf, &pErrNo);
                    lv_img_set_src(img, img_Src);
                    lv_obj_set_size(img, img_Src->header.w, img_Src->header.h);
                    lv_obj_set_style_radius(img, 5, 0);

                    lv_obj_align(img, LV_ALIGN_LEFT_MID, 5, 0);
                    lv_obj_set_style_clip_corner(img, BOOK_LIST_RADIUS_SIZE, LV_PART_MAIN);

                    //draw_boader(img);
                }
                
                lv_obj_t* label = lv_label_create(btn);
                lv_label_set_text(label, collect->cached_items[i].title);
                // 关键设置：
                lv_obj_set_style_flex_cross_place(label, LV_FLEX_ALIGN_CENTER, 0);
                lv_obj_set_flex_grow(label, 1);
                lv_obj_set_height(label, LV_PCT(80));
                lv_obj_set_width(label, (SCREEN_WIDTH - 97 - 28));
                lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP | LV_LABEL_LONG_DOT);
                lv_obj_align_to(label, img, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

                //draw_boader(label);
            }
        }
    }
    //lv_group_add_obj(lv_group_get_default(), collect->collect_Panel);

}

/*
取消收藏页面
*/
static void Del_key_cb(lv_event_t* e) {

    LV_LOG_USER("click Del_key \r\n");
    collect_switchScr(COLLECT_LIST_CONTENT, 0);
}

static lv_obj_t* rem_createDelBtn(lv_obj_t* parent, lv_event_cb_t clickCb, int extra)
{
#define RES_ESC_BTN_LOC_Y  0
#define TOP_BTN_BORDER 12
    lv_obj_t* img = lv_img_create(parent);
    const lv_img_dsc_t* pImgDsc = g_XXX_img_dsc_tbl + MMN_icn_esc - 1;
    //const lv_img_dsc_t* pImgDsc = zb_img_res_load_one(NULL,MY_DOWNLOAD_IMG_IC_CLOSE,IMG_FREE_TYPE_AUTO,NULL); //g_XXX_img_dsc_tbl + MMN_icn_esc - 1;
    lv_img_set_src(img, pImgDsc);
    lv_obj_set_size(img, pImgDsc->header.w + TOP_BTN_BORDER * 2, pImgDsc->header.h + TOP_BTN_BORDER * 2);
    lv_obj_set_pos(img, 20, RES_ESC_BTN_LOC_Y);
    lv_obj_set_style_border_width(img, TOP_BTN_BORDER, 0);
    lv_obj_set_style_radius(img, 4, 0);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_style_opa(img, LV_OPA_60, 0);
    lv_obj_set_style_opa(img, LV_OPA_100, LV_STATE_FOCUS_KEY);
    lv_obj_set_style_opa(img, LV_OPA_80, LV_STATE_PRESSED);
    // lv_color_t colorBorderFcs = lv_color_white();
    lv_obj_set_style_border_opa(img, LV_OPA_0, 0);
    lv_obj_add_event_cb(img, clickCb, LV_EVENT_CLICKED, NULL);

    return img;
}

static void update_selected_count_display()
{
    if (collect->selected_count == 0)
        /// lv_obj_add_flag(cb->checkmark, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(collect->remCol_btn, LV_OBJ_FLAG_HIDDEN);
    else {
        lv_obj_clear_flag(collect->remCol_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_t* label = lv_obj_get_child(collect->remCol_btn, 0);
        // 更新标签文本
    lv_label_set_text_fmt(label, "取消收藏(%d)", collect->selected_count);
        if (collect->total_count == collect->selected_count)
        {
            //collect->check_all_btn
        }
    }
}

// 更新所有项目的选中状态
static void update_all_items_selection(bool selected)
{
    
    // 遍历所有按钮
    for (int i = 0; i < collect->total_count; i++) {
        lv_obj_t* btn = lv_obj_get_child(collect->collect_Panel, i);
        btn_check_state_t* state = (btn_check_state_t*)lv_obj_get_user_data(btn);

        if (state) {
            // 更新状态
            state->checked = selected;

            // 更新图片
            lv_img_set_src(state->img_obj,
                selected ? collect->g_img_checked : collect->g_img_unchecked);
        }
    }
}

static void checkAll_btn_cb(lv_event_t* e)
{
    LV_LOG_USER("click checkAll_btn \r\n");
    bool selected_all;
    //collect->all_selected = !collect->all_selected;
    //lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* label = lv_obj_get_child(collect->check_all_btn, 0);
    if (collect->total_count != collect->selected_count) {
        lv_obj_add_state(collect->check_all_btn, LV_STATE_CHECKED);
        lv_label_set_text(label, "取消全选");
        selected_all = true;
    }
    else {
        lv_obj_clear_state(collect->check_all_btn, LV_STATE_CHECKED);
        lv_label_set_text(label, "全选");
        selected_all = false;
    }

    update_all_items_selection(selected_all);

    // 更新选中计数
    collect->selected_count = selected_all ? collect->total_count : 0;
    update_selected_count_display();
}

static lv_obj_t* create_btn_checkAll(lv_obj_t* parent) {
#define CHECKALL_BTN_LOC_X  90
#define CHECKALL_BTN_LOC_Y  40

    lv_obj_t* btn = lv_obj_create(parent);
    lv_obj_set_size(btn, CHECKALL_BTN_LOC_X, CHECKALL_BTN_LOC_Y);
    lv_obj_add_style(btn, get_global_style(STYLE_BTN_OPA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(btn, get_global_style(STYLE_BTN_BASE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(btn,LV_ALIGN_TOP_RIGHT,-10,5);

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, "全选");
    lv_obj_set_style_text_color(label, lv_color_hex(0x008CFF), LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_RIGHT_MID, 0, 0);  
    return btn;
}


static void remove_selected_items() {
    // 添加子组件到队列
    lv_obj_t* child;// = lv_obj_get_child(root, 0);
    int cnt = lv_obj_get_child_cnt(collect->collect_Panel);
    g_selected_paths = malloc(collect->selected_count * sizeof(char*));
    int  ichk = 0;
    printf("remove_selected_items：开始遍历，条数=%d\n", collect->selected_count);
    for (int i = 0; i < cnt; i++) {
        child = lv_obj_get_child(collect->collect_Panel, i);
        btn_check_state_t* state = (btn_check_state_t*)lv_obj_get_user_data(child);

        if (state->checked) {
            printf("当前点击的项目是：%d，删除bin数据...\n", i);
#ifndef C_PC_SIM
            //为删除数据表做准备
            g_selected_paths[ichk++] = zb_strdup(collect->cached_items[i].path);
#else
            char* label_text = lv_label_get_text(lv_obj_get_child(child, 0));
            g_selected_paths[ichk++] = zb_strdup(label_text);
#endif
        }
        else
            ESP_LOGI(TAG, "current: %d", i);

    }

#ifndef C_PC_SIM
    //2）删除数据表
    ESP_LOGI(TAG, "删除表中的数据:%d\n", current_type);
    export_db(10);
    bool success = collection_service_remove_items(current_type, (const char**)g_selected_paths, collect->selected_count, C_ZB_AP_COLLECT);
    if (success)
        ESP_LOGI(TAG, "............成功\n");
    else
        ESP_LOGI(TAG, "............失败\n");
    export_db(11);
#endif

    // 释放资源
    ESP_LOGI(TAG, "释放资源\n");
    if (g_selected_paths) {
        for (int i = 0; i < collect->selected_count; i++) {
            free(g_selected_paths[i]);
            free(g_selected_paths);
            g_selected_paths = NULL;
        }
}

    collect_switchScr(COLLECT_LIST_CONTENT, 0); // 有道听力宝，删除过后回到列表界面
}



static void check_event_cb(lv_event_t* e)
{
    lv_obj_t* btn = lv_event_get_target(e);

    bool  bChange = false;
    // 获取关联的状态数据
    btn_check_state_t* state = (btn_check_state_t*)lv_obj_get_user_data(btn);
    if (state) {
        // 切换选中状态
        bool was_checked = state->checked;

        state->checked = !state->checked;

        lv_img_set_src(state->img_obj,
            state->checked ? collect->g_img_checked : collect->g_img_unchecked);

        lv_obj_t* label = lv_obj_get_child(btn, 0);
        if (state->checked)
            lv_obj_set_style_text_color(label, lv_color_hex(REMOVE_LABLE_COLOR_SELECT), 0);
        else
            lv_obj_set_style_text_color(label, lv_color_hex(REMOVE_LABLE_COLOR_DEFAULT), 0);

        // 3. 更新计数器
        if (state->checked && !was_checked) {
            collect->selected_count++;
            if (collect->selected_count == collect->total_count)
                bChange = true;
        }else if (!state->checked && was_checked){
            if (collect->selected_count == collect->total_count)
                bChange = true;
            collect->selected_count--;
        }
        
        if (bChange) {
            lv_obj_t* label = lv_obj_get_child(collect->check_all_btn, 0);
            if (collect->total_count == collect->selected_count) {
                lv_obj_add_state(collect->check_all_btn, LV_STATE_CHECKED);
                lv_label_set_text(label, "取消全选");
            }
            else {
                lv_obj_clear_state(collect->check_all_btn, LV_STATE_CHECKED);
                lv_label_set_text(label, "全选");
            }
        }
        
        update_selected_count_display();
    }
}

//按钮删除事件回调（内存管理）
static void btn_del_event_cb(lv_event_t* e)
{
    lv_obj_t* btn = lv_event_get_target(e);
    btn_check_state_t* state = (btn_check_state_t*)lv_obj_get_user_data(btn);

    if (state) {
        free(state);
        lv_obj_set_user_data(btn, NULL);  // 清除指针
    }
}

static void remove_selected_items_async(void* data) {
    remove_selected_items();
}

//点击全部删除按钮回调
static void delAll_event_cb(lv_event_t* e)
{
    lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* popup = lv_obj_get_parent(btn);
    lv_obj_t* mask = lv_obj_get_parent(popup);

    if (collect->not_tip)
        judge_need_tip(true);

    // 先删除弹出框
    lv_obj_del(mask);
    // 延迟执行删除操作，确保 UI 更新完成
    lv_async_call(remove_selected_items_async, NULL);
    //free(popup);
    LV_LOG_USER("click delAll btn\r\n");
}


//添加取消收藏按钮
static lv_obj_t* create_remove_collect_button(lv_obj_t* parent) {
    lv_obj_t* remCol_btn = lv_obj_create(parent);
    lv_obj_set_size(remCol_btn, 280, 48);
    lv_obj_add_style(remCol_btn, get_global_style(STYLE_BTN_BASE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(remCol_btn, lv_color_hex(0xFF3333), 0);
    lv_obj_set_style_radius(remCol_btn, 90, 0);

    lv_obj_clear_flag(remCol_btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(remCol_btn, LV_ALIGN_BOTTOM_MID, 0, -20);

    lv_obj_t* remCol_label = lv_label_create(remCol_btn);
    lv_obj_set_style_text_color(remCol_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(remCol_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(remCol_label, LV_ALIGN_CENTER, 0, 0);

    return remCol_btn;
}


// 定义颜色
#define CHECKBOX_BORDER_COLOR lv_color_hex(0xDEDEDE)
#define CHECKBOX_BG_COLOR lv_color_hex(0xFFFFFF)
#define CHECKBOX_CHECKED_COLOR lv_color_hex(0x008CFF)
#define TEXT_COLOR lv_color_hex(0x787F87)

// 复选框事件回调
static void checkbox_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    custom_checkbox_t* cb = (custom_checkbox_t*)lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED || (code == LV_EVENT_KEY)) {
        // 切换状态
        cb->checked = !cb->checked;

        // 更新视觉状态
        if (cb->checked) {
            lv_obj_clear_flag(cb->checkmark, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_border_color(cb->box, CHECKBOX_CHECKED_COLOR, 0);
        }
        else {
            lv_obj_add_flag(cb->checkmark, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_border_color(cb->box, CHECKBOX_BORDER_COLOR, 0);
        }

        // 发送值改变事件
        lv_event_send(cb->obj, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

// 创建自定义复选框
static custom_checkbox_t* create_custom_checkbox(lv_obj_t* parent)
{
    // 创建容器
    custom_checkbox_t* cb = (custom_checkbox_t*)lv_mem_alloc(sizeof(custom_checkbox_t));
    cb->obj = lv_obj_create(parent);
    lv_obj_set_size(cb->obj, 100, 30);
    lv_obj_set_flex_flow(cb->obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cb->obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cb->obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_style_all(cb->obj);

    // 创建复选框框体
    cb->box = lv_obj_create(cb->obj);
    lv_obj_set_size(cb->box, 18, 18);
    lv_obj_set_style_border_color(cb->box, CHECKBOX_BORDER_COLOR, 0);
    lv_obj_set_style_border_width(cb->box, 1, 0);
    lv_obj_set_style_bg_color(cb->box, CHECKBOX_BG_COLOR, 0);
    lv_obj_set_style_radius(cb->box, 3, 0);
    lv_obj_set_style_pad_all(cb->box, 2, 0);

    // 创建选中标记（初始隐藏）
    cb->checkmark = lv_obj_create(cb->box);
    lv_obj_set_size(cb->checkmark, 12, 12);
    lv_obj_set_style_bg_color(cb->checkmark, CHECKBOX_CHECKED_COLOR, 0);
    lv_obj_set_style_radius(cb->checkmark, 2, 0);
    lv_obj_center(cb->checkmark);
    lv_obj_add_flag(cb->checkmark, LV_OBJ_FLAG_HIDDEN);

    // 创建标签
    cb->label = lv_label_create(cb->obj);
    lv_label_set_text(cb->label, "不再提示");
    lv_obj_set_style_text_color(cb->label, TEXT_COLOR, 0);
    //lv_obj_set_style_text_font(cb->label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_pad_left(cb->label, 25, 0); // 文本与复选框间距

    // 初始状态
    cb->checked = false;

    // 添加点击事件
    lv_obj_add_event_cb(cb->box, checkbox_event_cb, LV_EVENT_CLICKED, cb);
    lv_obj_add_event_cb(cb->obj, checkbox_event_cb, LV_EVENT_CLICKED, cb);
    lv_obj_add_event_cb(cb->obj, checkbox_event_cb, LV_EVENT_KEY, cb);

    return cb;
}


// 设置复选框状态
static void custom_checkbox_set_checked(custom_checkbox_t* cb, bool checked)
{
    if (cb->checked != checked) {
        cb->checked = checked;

        if (checked) {
            lv_obj_clear_flag(cb->checkmark, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_border_color(cb->box, CHECKBOX_CHECKED_COLOR, 0);
        }
        else {
            lv_obj_add_flag(cb->checkmark, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_border_color(cb->box, CHECKBOX_BORDER_COLOR, 0);
        }
    }
}

// 获取复选框状态
static bool custom_checkbox_is_checked(custom_checkbox_t* cb)
{
    return cb->checked;
}

static void dont_remind_event_cb(lv_event_t* e)
{
    custom_checkbox_t* cb = (custom_checkbox_t*)lv_event_get_user_data(e);
    bool checked = custom_checkbox_is_checked(cb);


    LV_LOG_USER("不再提示状态: %s", checked ? "已选中" : "未选中");

    // 根据状态执行操作
    if (checked) {
        // 用户选择不再提示，保存设置
        collect->not_tip = true;
    }
    else {
        collect->not_tip = false;
    }
}


static void remove_collect_btn_event_cb(lv_event_t* e)
{
    /*if (judge_need_tip(false))
    {
            lv_obj_t* mask = create_network_popup(collect->ui_collect, "删除全部?", "是否要删除收藏列表当中所有内容?", 231);

//#ifndef C_PC_SIM
//        lv_obj_t* mask = create_network_popup(collect->ui_collect, "删除全部?", "是否要删除下载列表当中所有内容?"); //251   231
//#else
//        lv_obj_t* mask = create_network_popup(collect->ui_collect, "删除全部?", "是否要删除下载列表当中所有内容?",249); //251   231
//#endif // !C_PC_SIM

        lv_obj_t* popup = lv_obj_get_child(mask, 0);
        lv_obj_t* main = lv_obj_get_parent(mask);

        lv_obj_t* btn = create_custom_button(popup, "确认删除", NULL, NULL);
        lv_obj_set_size(btn, 200, 46);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -54);
        lv_obj_add_style(btn, get_global_style(STYLE_BTN_BASE), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xFF3333), 0);
        lv_obj_add_event_cb(btn, delAll_event_cb,LV_EVENT_CLICKED, main);

        // 创建复选框
        custom_checkbox_t* dont_remind_cb = create_custom_checkbox(popup);
        lv_obj_add_event_cb(dont_remind_cb->obj, dont_remind_event_cb, LV_EVENT_VALUE_CHANGED, dont_remind_cb);
        lv_obj_align_to(dont_remind_cb->obj, btn, LV_ALIGN_BOTTOM_MID, 0, 140);
    }
    else*/
    {
        remove_selected_items();
    }
}


static void remove_collect_init()
{
    collect->current_screen = REMOEVE_COLLECT_CONTENT; // 设置页面标识
    collect->ui_collect = res_createMain(lv_scr_act());

    collect->ui_title = res_createTitle(collect->ui_collect, "选择");

    rem_createDelBtn(collect->ui_collect, Del_key_cb, 0);

    collect->check_all_btn = create_btn_checkAll(collect->ui_collect);
    
    lv_obj_add_event_cb(collect->check_all_btn, checkAll_btn_cb, LV_EVENT_CLICKED, NULL);

    collect->collect_Panel = lv_obj_create(collect->ui_collect);
    lv_obj_set_width(collect->collect_Panel, SCREEN_WIDTH);
    lv_obj_set_height(collect->collect_Panel, 356);
    lv_obj_align(collect->collect_Panel, LV_ALIGN_TOP_MID, 0, 56); //56  rsh  影响了批量删除
    lv_obj_set_flex_flow(collect->collect_Panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(collect->collect_Panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_row(collect->collect_Panel, 12, LV_PART_MAIN);
    lv_obj_set_flex_align(collect->collect_Panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_border_width(collect->collect_Panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(collect->collect_Panel, 5, LV_PART_MAIN);

    if (!collect->g_img_checked) {
        collect->g_img_checked = zb_img_res_load_one(NULL, COLLECT_IMG_CHECK, IMG_FREE_TYPE_AUTO, NULL);
        collect->g_img_unchecked = zb_img_res_load_one(NULL, COLLECT_IMG_UNCHECK, IMG_FREE_TYPE_AUTO, NULL);
    }

    //test_insert_record();
    //collect->total_count = 0;
    if (collect->total_count == 0)
    {
        lv_obj_t* new_container = lv_obj_create(collect->collect_Panel);
        lv_obj_set_size(new_container, LV_PCT(100), LV_PCT(100));
        lv_obj_align(new_container, LV_ALIGN_TOP_LEFT, 10, 10);
        lv_obj_set_style_border_width(new_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        create_no_content_show(new_container, "当前收藏列表为空");
        //lv_obj_del(collect->check_all_btn);
        lv_obj_add_flag(collect->check_all_btn, LV_OBJ_FLAG_HIDDEN);
    }
    else {
        collect->selected_count = 0;
        for (int i = 0; i < collect->total_count; i++)
        {
            lv_obj_t* btn = lv_obj_create(collect->collect_Panel);
            lv_obj_set_size(btn, 296, 56);
            lv_obj_add_style(btn, get_global_style(STYLE_BTN_BASE), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xF5F7FA), 0);
            lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);

            btn_check_state_t* state = (btn_check_state_t*)malloc(sizeof(btn_check_state_t));
            state->checked = false;  // 初始未选中


            lv_obj_set_user_data(btn, state);

            lv_obj_add_event_cb(btn, btn_del_event_cb, LV_EVENT_DELETE, NULL);


            lv_obj_t* label = lv_label_create(btn);
            lv_obj_set_width(label, 228);
            lv_obj_set_height(label, 20);
            lv_label_set_text(label, collect->cached_items[i].title);
            lv_obj_set_style_text_color(label, lv_color_hex(REMOVE_LABLE_COLOR_DEFAULT), 0);

            lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
            lv_obj_align(label, LV_ALIGN_LEFT_MID, 16, 0);
            lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);


            lv_obj_t* check = lv_img_create(btn);
            state->img_obj = check;
            const lv_img_dsc_t* pImgDsc = zb_img_res_load_one(NULL, COLLECT_IMG_UNCHECK, IMG_FREE_TYPE_AUTO, NULL);
            lv_img_set_src(check, pImgDsc);
            lv_obj_align(check, LV_ALIGN_TOP_RIGHT, -14, 14);

            lv_obj_add_event_cb(btn, check_event_cb, LV_EVENT_CLICKED, NULL);
        }

        collect->remCol_btn = create_remove_collect_button(collect->ui_collect);
        update_selected_count_display();
        lv_obj_add_event_cb(collect->remCol_btn, remove_collect_btn_event_cb, LV_EVENT_CLICKED, NULL);
    }

    lv_obj_add_event_cb(collect->ui_collect, collectKey_cb, LV_EVENT_ALL, NULL);
    lv_group_add_obj(lv_group_get_default(), collect->ui_collect);
    sys_setCurMainWinAndCb(collect->ui_collect, collectKey_cb);
}

static void collect_list_free(void) {
    if (!collect) return;
    if (collect->ui_collect) {
        lv_obj_del(collect->ui_collect);
        collect->ui_collect = NULL;
    }
    // 重置主页面相关指针
    collect->btn_audio = NULL;
    collect->btn_book = NULL;
    collect->player_obj = NULL;
    collect->created_component = NULL;
}

// 移除收藏页面释放函数
static void remove_collect_free(void) {
    if (!collect) return;

    // 先清除所有子对象
    if (collect->collect_Panel) {
        lv_obj_del(collect->collect_Panel);
        collect->collect_Panel = NULL;
    }

    if (collect->remCol_btn) {
        lv_obj_del(collect->remCol_btn);
        collect->remCol_btn = NULL;
    }

    if (collect->check_all_btn) {
        lv_obj_del(collect->check_all_btn);
        collect->check_all_btn = NULL;
    }

    // 最后删除主容器
    if (collect->ui_collect) {
        lv_obj_del(collect->ui_collect);
        collect->ui_collect = NULL;
    }

    // 重置计数器
    collect->selected_count = 0;
    collect->all_selected = false;
}
/*
页面切换


*/
static const struct DIC_SCR_INF tbcdScrInfTbl[] = {
    { collect_screen_init,  collect_list_free },   // COLLECT_LIST_CONTENT
    { remove_collect_init, remove_collect_free  }, // REMOEVE_COLLECT_CONTENT
    //{ downloading_init, downloading_free  }, // REMOEVE_COLLECT_CONTENT
    { NULL, NULL },
};

static void collect_switchScr(int8_t scrNoNew, int parm) {

    if ((scrNoNew < 0) || (scrNoNew >= M_ARRAY_LEN(tbcdScrInfTbl))) {
        ESP_LOGE(TAG, "error scrNo %d", scrNoNew);
        return;
    }

    FN_TBCD_SCREEN_INIT fnInit = tbcdScrInfTbl[scrNoNew].m_fn_init;
    if (!fnInit) {
        ESP_LOGW(TAG, "newScr cb isNul!");
        return;
    }

    if ((int)g_scrNo >= 0) {
        FN_TBCD_SCREEN_FREE fnFree = tbcdScrInfTbl[g_scrNo].m_fn_free;
        if (fnFree) {
            (*fnFree)();
        }
        else {

        }
    }

    if (collect) {
        memset(&collect->g_lines, 0, sizeof(collect->g_lines));
        memset(&collect->g_check, 0, sizeof(collect->g_check));
        collect->created_component = NULL;
        collect->player_obj = NULL;
        // 保留需要跨页面的状态
         collect->not_tip = collect->not_tip;
    }

    g_scrNo = scrNoNew;
    (*fnInit)(parm);
}

static void display_time() {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_buf[29];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H%M%S", tm_info);
    ESP_LOGI(TAG, "%s\n", time_buf);
}

static void export_db(int no)
{
    char extport_path[300];
    sprintf(extport_path, "/userdata/_data/rrr_db_%d.json", no);
#ifndef C_PC_SIM
    collection_service_export_to_json(C_ZB_AP_COLLECT,(const char*) extport_path);
#endif
    return;
}
static void dispear_item_timer_cb(lv_timer_t* t)
{
    lv_obj_del(t->user_data);
    lv_timer_del(t);
    // 当删除后都已经下载完成了，那么显示当前
    collect->total_count--;
    if (collect->total_count == 0) {
        collect_switchScr(COLLECT_DOWNLOAD_CONTENT, 0);
    }
}
