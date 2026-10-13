//========================================================[]
//Name: auto_screenoff.c
//Function: Setting auto_off screen time  
//Author: W sy
//Date: 2024-04-02
//========================================================[]
/**********************
     INCLUDES
 *********************/
#include <string.h>

#include "auto_screenoff.h"
#include "../utils/global_parm.h"
#include "../data_parse/img_res.h"
#include "../mainmenu/tlb_main_img.h"
#include "./app_setting_base.h"
#include "../include/img_a_tlb.h"
#ifndef C_PC_SIM
#include "../../trd/tz_sys_lib/include/cvi_tz_sys.h"
#endif

 /*********************
  *      DEFINES
  *********************/
#define OPTION_BTN_WIDTH    280
#define OPTION_BTN_HEIGHT   52
#define OPTION_SPACING      12
#define OPTION_MARGIN_TOP   80  // 选项区域顶部边距
#define OPTION_COUNT        6   // 1/3/5/10/15/20分钟

  /**********************
   *  STATIC PROTOTYPES
   **********************/
static void auto_shout_lcd_time_win_del(void);
static void _stng_autoOff_lst_scr_quit(lv_obj_t* obj);
static void auto_shout_lcd_time_cb(lv_event_t* e);
static void brightness_init(void);
static void _stng_esc_btn_click_cb(lv_event_t* evt);
static void _stng_scrOffArea_click_cb(lv_event_t* evt);
static void bright_setting_cb(lv_event_t* e);
static void bright_set_main_constructor(void);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_brightness_set_t* br_set = NULL;
#if DARK_MODE
static lv_style_t dark_style; // 新增暗色模式样式
#endif

// 修改为6个时间选项
const brightness_time_str_t set_offtime[OPTION_COUNT] = {
    {60,   "1分钟"},
    {180,  "3分钟"},
    {300,  "5分钟"},
    {600,  "10分钟"},
    {900,  "15分钟"},
    {1200, "20分钟"}
};

/**********************
 *   extra  FUNCTIONS
 **********************/
 // 返回秒数   second
static uint16_t get_sys_shout_scr_time(void)
{
#ifndef C_PC_SIM
    int offSec = 0;
    int ret = tz_get_screenoff_time(&offSec);
    LV_LOG_USER("tz_get_screenoff_time:%d", offSec);
    if (offSec > 0)
    {
        return offSec;
    }
#else
    uint16_t sec = getPrjGlobalParm(ZB_PARM_SCR_AUTO_OFF_TIME, 0, NULL);
    return sec;
#endif
    return 60;
}

static int set_sys_shout_scr_time(void)
{
    int iFuncRet = -1;
    LV_LOG_USER("set_sys_shout_scr_time:%d", br_set->time_sec);
#ifndef C_PC_SIM
    iFuncRet = tz_set_screenoff_time(br_set->time_sec);
#else
    iFuncRet = setPrjGlobalParm(ZB_PARM_SCR_AUTO_OFF_TIME, 2, &br_set->time_sec);
#endif
    return iFuncRet;
}

/**********************
 *   STATIC  FUNCTIONS
 **********************/
static void auto_shout_lcd_time_win_del(void)
{
    set_sys_shout_scr_time();
    br_set->stime_set_flag = 0;

}

#if 0
static void _stng_autoOff_lst_scr_quit(lv_obj_t* obj) {
    char* str = NULL;

    int16_t row = br_set->m_as_selNo;
    str = lv_table_get_cell_value(obj, row, 0);
    br_set->time_sec = set_offtime[row].sec;
    br_set->time_str = str;
    LV_LOG("%s,[%d]: %s, sec: %d \r\n ", __FUNCTION__, __LINE__, str, br_set->time_sec);
    lv_label_set_text(br_set->minu_sec_label, br_set->time_str);

    auto_shout_lcd_time_win_del();
}
#endif
static void auto_shout_lcd_time_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED || code == LV_EVENT_KEY) {
        int16_t row = (int16_t)(intptr_t)lv_obj_get_user_data(obj);

        for (int i = 0; i < OPTION_COUNT; i++) {
            lv_obj_add_flag(br_set->check_icons[i], LV_OBJ_FLAG_HIDDEN);
        }

        lv_obj_clear_flag(br_set->check_icons[row], LV_OBJ_FLAG_HIDDEN);
        br_set->current_selected_row = row;

        br_set->time_sec = set_offtime[row].sec;
        uint32_t timeout_ms = br_set->time_sec * 1000;
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "设备会在熄屏(且空闲)%d分钟后自动关机", br_set->time_sec / 60);
     
        auto_shout_lcd_time_win_del();
        create_dialog_with_timer(false, buffer,2000,234,120);
    }
    else if (code == LV_EVENT_KEY) {
        int32_t c = *((int32_t*)lv_event_get_param(e));
        if (c == LV_KEY_ESC) {
            auto_shout_lcd_time_win_del();
        }
    }
}

static void brightness_init(void)
{
    int i = 0;
    if (br_set != NULL)    return;
    LV_LOG("%s,[%d] \r\n ", __FUNCTION__, __LINE__);

    br_set = lv_mem_alloc(sizeof(lv_brightness_set_t));
    memset(br_set, 0, sizeof(lv_brightness_set_t));

    br_set->time_sec = get_sys_shout_scr_time();

    // 查找匹配的时间设置
    int found = 0;
    for (i = 0; i < OPTION_COUNT; i++) {
        if (set_offtime[i].sec == br_set->time_sec) {
            br_set->time_str = set_offtime[i].str;
            br_set->time_index = i;
            found = 1;
            break;
        }
    }

    // 如果没找到匹配项，使用默认值（5分钟）
    if (!found) {
        br_set->time_str = set_offtime[2].str; // 5分钟
        br_set->time_index = 2;
        br_set->time_sec = set_offtime[2].sec;
    }

    LV_LOG("%s,[%d] time: %s \r\n ", __FUNCTION__, __LINE__, br_set->time_str);
}

static void _stng_esc_btn_click_cb(lv_event_t* evt)
{
    lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
    return;
}

static void _stng_scrOffArea_click_cb(lv_event_t* evt)
{
    auto_shout_lcd_time_win();
    return;
}

static void bright_setting_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* f_obj = lv_event_get_target(e);

    LV_LOG("%s [%d] code : %d\n", __FUNCTION__, __LINE__, code);

    if (code == LV_EVENT_KEY) {
        int32_t c = *((int32_t*)lv_event_get_param(e));

        if (c == LV_KEY_ESC) {
            lv_indev_keypad_wait_release();
            lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
        }
    }
}

static void bright_set_main_constructor(void)
{
#if DARK_MODE
    // 初始化暗色模式样式
    lv_style_init(&dark_style);
    lv_style_set_bg_color(&dark_style, lv_color_hex(0x000000)); // 纯黑背景
    lv_style_set_text_color(&dark_style, lv_color_white());      // 白色文本
    lv_style_set_border_color(&dark_style, lv_color_hex(0x212121)); // 按钮边框色
#endif
    br_set->cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(br_set->cont, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align(br_set->cont, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_pad_all(br_set->cont, 0, 0);
    lv_obj_set_style_border_width(br_set->cont, 0, 0);
    lv_obj_set_style_radius(br_set->cont, 0, 0);
    lv_obj_clear_flag(br_set->cont, LV_OBJ_FLAG_SCROLLABLE);
#if DARK_MODE
    lv_obj_add_style(br_set->cont, &dark_style, 0); // 应用暗色样式
    lv_obj_set_style_bg_color(br_set->cont, lv_color_hex(0x000000), 0); // 纯黑背景
#else
    lv_obj_set_style_bg_color(br_set->cont, lv_color_hex(0xF2F2F2), 0); // 原浅灰
#endif

   
    br_set->title_label = lv_label_create(br_set->cont);
    lv_label_set_text(br_set->title_label, "自动息屏");
    //lv_obj_set_style_text_font(br_set->title_label, &lv_font_montserrat_20, 0);
#if DARK_MODE
    lv_obj_set_style_text_color(br_set->title_label, lv_color_white(), 0); // 白色文本
#endif
    lv_obj_align(br_set->title_label, LV_ALIGN_TOP_MID, 0, 20);

   
    stng_createEscBtn(br_set->cont, _stng_esc_btn_click_cb, 0);

   
    lv_obj_t* options_cont = lv_obj_create(br_set->cont);
    lv_obj_remove_style_all(options_cont);
    lv_obj_set_size(options_cont, OPTION_BTN_WIDTH,
        (OPTION_BTN_HEIGHT * OPTION_COUNT) + (OPTION_SPACING * (OPTION_COUNT - 1)));
    lv_obj_align(options_cont, LV_ALIGN_TOP_MID, 0, OPTION_MARGIN_TOP);
    lv_obj_set_flex_flow(options_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(options_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(options_cont, OPTION_SPACING, 0);

    
    for (int i = 0; i < OPTION_COUNT; i++) {
        lv_obj_t* btn = lv_btn_create(options_cont);
        lv_obj_set_size(btn, OPTION_BTN_WIDTH, OPTION_BTN_HEIGHT);
#if DARK_MODE
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x212121), 0); // 按钮暗灰色
        lv_obj_set_style_text_color(btn, lv_color_white(), 0);     // 按钮文本白色
#else
        lv_obj_set_style_bg_color(btn, lv_color_white(), 0);       // 原白色
#endif
        lv_obj_set_style_radius(btn, 12, 0);
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_set_style_outline_width(btn, 0, 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);

       
        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, set_offtime[i].str);
#if DARK_MODE
        lv_obj_set_style_text_color(label, lv_color_white(), 0);              // 白色文本
        lv_obj_set_style_text_color(label, lv_color_hex(0x008CFF), LV_STATE_FOCUS_KEY); // 焦点保持蓝色
#else
        lv_obj_set_style_text_color(label, lv_color_hex(0x18191B), 0);        // 原黑色
        lv_obj_set_style_text_color(label, lv_color_hex(0x008CFF), LV_STATE_FOCUS_KEY);
#endif
       // lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 20, 0);

       
        br_set->check_icons[i] = lv_img_create(btn);
        lv_img_dsc_t* imgCheck_sec = zb_img_res_load_one(NULL, SETTING_IMG_CHECK, IMG_FREE_TYPE_AUTO, NULL);
        lv_img_set_src(br_set->check_icons[i], imgCheck_sec);
        lv_obj_align(br_set->check_icons[i], LV_ALIGN_RIGHT_MID, -20, 0);
        lv_obj_add_flag(br_set->check_icons[i], LV_OBJ_FLAG_HIDDEN);

       
        lv_obj_set_user_data(btn, (void*)(intptr_t)i);
        lv_obj_add_event_cb(btn, auto_shout_lcd_time_cb, LV_EVENT_CLICKED, NULL);
    }

    
    lv_obj_clear_flag(br_set->check_icons[br_set->time_index], LV_OBJ_FLAG_HIDDEN);

  
    lv_group_add_obj(lv_group_get_default(), br_set->cont);
    lv_obj_add_flag(br_set->cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(br_set->cont, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    sys_setCurMainWinAndCb(br_set->cont, bright_setting_cb);
    lv_group_focus_obj(br_set->cont);
}

/**********************
*   GLOBAL FUNCTIONS
**********************/

void setting_autooff_screen_del(void)
{
    LV_LOG("%s [%d]\n", __FUNCTION__, __LINE__);

    if (br_set != NULL) {
        lv_obj_del(br_set->cont);
        lv_mem_free(br_set);
        br_set = NULL;
    }
}

void setting_autooff_screen_entry(void)
{
    brightness_init();
    bright_set_main_constructor();
}