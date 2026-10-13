#include "scan.h"
#include "../utils/global_parm.h"
#include "../data_parse/img_res.h"
#include "../mainmenu/tlb_main_img.h"
#include "../utils/style_tz.h"
#include "./app_setting_base.h"
/*********************
 *      DEFINES
 *********************/
#define SCREEN_WIDTH     320
#define SCREEN_HEIGHT    480
#define QR_SIZE          240
#define TITLE_Y_OFFSET    96
#define BTN_WIDTH        180
#define BTN_HEIGHT       48
#define ANIM_SIZE        60

 /**********************
  *  STATIC PROTOTYPES
  **********************/
static void scan_bind_init(void);
static void scan_bind_build_ui(void);
static void update_ui_for_state(ScanState state);
static void handle_timeout(lv_timer_t* timer);
static void handle_network_check(lv_timer_t* timer);
static void action_btn_cb(lv_event_t* e);
static void user_interaction_handler(void);
void change_state(ScanState new_state);
static void scan_success_delay_timer(lv_timer_t* timer);
void scan_bind_screen_del(void);
/**********************
 *  STATIC VARIABLES
 **********************/
static ScanBindContext* g_scanBindCtx = NULL;

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void scan_bind_init(void) {
    if (g_scanBindCtx != NULL) return;

    g_scanBindCtx = lv_mem_alloc(sizeof(ScanBindContext));
    memset(g_scanBindCtx, 0, sizeof(ScanBindContext));

    // 初始状态
    g_scanBindCtx->current_state = SCAN_STATE_INITIAL;

    g_scanBindCtx->network_check_count = 0;

    // 检测初始网络状态
#ifndef C_PC_SIM
    //g_scanBindCtx->network_available = check_network_status();
    g_scanBindCtx->network_available = 1;
#else
    g_scanBindCtx->network_available = 1;
#endif
    // 创建网络检测定时器 (1秒间隔)
   // lv_timer_create(handle_network_check, 1000, NULL);
}

static void _stng_esc_btn_click_cb(lv_event_t* evt)
{
    lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
    return;
}

static void scan_page_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_KEY) {
        int32_t c = *((int32_t*)lv_event_get_param(e));

        if (c == LV_KEY_ESC) {
            lv_indev_keypad_wait_release();
            lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
        }
    }
}


static void scan_bind_build_ui(void) {
    // 创建主容器
    g_scanBindCtx->cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_scanBindCtx->cont, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align(g_scanBindCtx->cont, LV_ALIGN_TOP_LEFT, 0, 0);
#if DARK_MODE
    lv_obj_set_style_bg_color(g_scanBindCtx->cont, lv_color_hex(0x000000), 0); // 黑色背景
#else
    lv_obj_set_style_bg_color(g_scanBindCtx->cont, lv_color_hex(0xFFFFFF), 0);
#endif
    lv_obj_set_style_border_width(g_scanBindCtx->cont, 0, 0);
    lv_obj_set_style_radius(g_scanBindCtx->cont, 0, 0);
    lv_obj_clear_flag(g_scanBindCtx->cont, LV_OBJ_FLAG_SCROLLABLE);
    sys_setCurMainWinAndCb(g_scanBindCtx->cont, scan_page_event_cb);


    stng_createEscBtn(g_scanBindCtx->cont, _stng_esc_btn_click_cb, 0);

#define  scan_title "关注微信公众号【互动学生卡】登录后扫码绑定账号"
    // 标题
    g_scanBindCtx->title_label = lv_label_create(g_scanBindCtx->cont);
    lv_label_set_text(g_scanBindCtx->title_label, scan_title);
    lv_obj_set_width(g_scanBindCtx->title_label, 252); 
#if DARK_MODE
    lv_obj_set_style_text_color(g_scanBindCtx->title_label, lv_color_hex(0xFFFFFF), 0); // 白色文本
#endif
    lv_label_set_long_mode(g_scanBindCtx->title_label, LV_LABEL_LONG_WRAP); 
    lv_obj_align(g_scanBindCtx->title_label, LV_ALIGN_TOP_MID, 0, TITLE_Y_OFFSET);
    lv_obj_set_style_text_align(g_scanBindCtx->title_label, LV_TEXT_ALIGN_CENTER, 0);

    // 二维码图像 (居中)
    g_scanBindCtx->qr_img = lv_img_create(g_scanBindCtx->cont);
    lv_obj_set_size(g_scanBindCtx->qr_img, QR_SIZE, QR_SIZE);
    lv_obj_center(g_scanBindCtx->qr_img);

#if 1
    // 状态文本
    g_scanBindCtx->status_label = lv_label_create(g_scanBindCtx->cont);
    lv_obj_align(g_scanBindCtx->status_label, LV_ALIGN_BOTTOM_MID, 0, -20);
#if DARK_MODE
    lv_obj_set_style_text_color(g_scanBindCtx->status_label, lv_color_hex(0xFFFFFF), 0); // 白色文本
#endif
    lv_label_set_long_mode(g_scanBindCtx->status_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(g_scanBindCtx->status_label, SCREEN_WIDTH - 40);
    lv_obj_set_style_text_align(g_scanBindCtx->status_label, LV_TEXT_ALIGN_CENTER, 0);
#endif

    // 操作按钮
    g_scanBindCtx->action_btn = lv_btn_create(g_scanBindCtx->cont);
    lv_obj_set_size(g_scanBindCtx->action_btn, BTN_WIDTH, BTN_HEIGHT);
#if DARK_MODE
    lv_obj_set_style_bg_color(g_scanBindCtx->action_btn, lv_color_hex(0x212121), 0); // 深灰色按钮
#endif
    lv_obj_align(g_scanBindCtx->action_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(g_scanBindCtx->action_btn, action_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* btn_label = lv_label_create(g_scanBindCtx->action_btn);
    lv_label_set_text(btn_label, "");
#if DARK_MODE
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0xFFFFFF), 0); // 白色文本
#endif
    lv_obj_center(btn_label);

#ifndef C_PC_SIM
    char imei_ver[32];
    tz_get_imei(imei_ver);
#endif
    // IMEI文本
    g_scanBindCtx->IMEI = lv_label_create(g_scanBindCtx->cont);
    lv_obj_align(g_scanBindCtx->IMEI, LV_ALIGN_BOTTOM_MID, 0, -48);
#if DARK_MODE
    lv_obj_set_style_text_color(g_scanBindCtx->IMEI, lv_color_hex(0xFFFFFF), 0); // 白色文本
#endif
    lv_label_set_long_mode(g_scanBindCtx->IMEI, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(g_scanBindCtx->IMEI, SCREEN_WIDTH - 40);
    lv_obj_set_style_text_align(g_scanBindCtx->IMEI, LV_TEXT_ALIGN_CENTER, 0);
#ifndef C_PC_SIM
    lv_label_set_text_fmt(g_scanBindCtx->IMEI,"IMEI: %s", imei_ver);
#else
    lv_label_set_text(g_scanBindCtx->IMEI, "IMEI: 157785962541258");
#endif

    // 加载动画
    g_scanBindCtx->loading_anim = lv_spinner_create(g_scanBindCtx->cont, 1000, 60);
    lv_obj_set_size(g_scanBindCtx->loading_anim, ANIM_SIZE, ANIM_SIZE);
    lv_obj_center(g_scanBindCtx->loading_anim);
    lv_obj_add_flag(g_scanBindCtx->loading_anim, LV_OBJ_FLAG_HIDDEN);


    // 初始UI状态
    update_ui_for_state(SCAN_STATE_INITIAL);
}


static void scan_success_delay_timer(lv_timer_t* timer) {
    change_state(SCAN_STATE_GENERAL_LOADING);
    lv_timer_del(timer); // 删除定时器
}

static void update_ui_for_state(ScanState state) {
    
    // 清理之前可能创建的特殊元素
    if (g_scanBindCtx->reload_btn) {
        lv_obj_del(g_scanBindCtx->reload_btn);
        g_scanBindCtx->reload_btn = NULL;
    }
    if (g_scanBindCtx->network_switch_btn) {
        lv_obj_del(g_scanBindCtx->network_switch_btn);
        g_scanBindCtx->network_switch_btn = NULL;
    }
    if (g_scanBindCtx->success_panel) {
        lv_obj_del(g_scanBindCtx->success_panel);
        g_scanBindCtx->success_panel = NULL;
    }
    if (g_scanBindCtx->back_btn) {
        lv_obj_del(g_scanBindCtx->back_btn);
        g_scanBindCtx->back_btn = NULL;
    }

    // 恢复原有元素的可见性
    lv_obj_clear_flag(g_scanBindCtx->title_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_scanBindCtx->status_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_scanBindCtx->qr_img, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_scanBindCtx->action_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_scanBindCtx->loading_anim, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t* btn_label = lv_obj_get_child(g_scanBindCtx->action_btn, 0);

    switch (state) {
    case SCAN_STATE_INITIAL:
        lv_label_set_text(g_scanBindCtx->status_label, "请扫描二维码绑定账号");
        if (g_scanBindCtx->network_available) {
            change_state(SCAN_STATE_QR_LOADING);
        }
        else {
            change_state(SCAN_STATE_QR_ERROR);
        }
        break;

    case SCAN_STATE_QR_LOADING:
        //lv_label_set_text(g_scanBindCtx->status_label, "二维码加载中...");
        lv_obj_clear_flag(g_scanBindCtx->loading_anim, LV_OBJ_FLAG_HIDDEN);
       // start_qrcode_request();
        break;

    case SCAN_STATE_QR_DISPLAY:
        lv_label_set_text(g_scanBindCtx->status_label, "请使用微信扫描二维码");
        lv_img_set_src(g_scanBindCtx->qr_img, "A:qrcode.png");
        lv_obj_clear_flag(g_scanBindCtx->qr_img, LV_OBJ_FLAG_HIDDEN);

        // 启动30秒超时定时器
        if (g_scanBindCtx->timeout_timer == NULL) {
            g_scanBindCtx->timeout_timer = lv_timer_create(handle_timeout, 30000, NULL);
        }
        else {
            lv_timer_reset(g_scanBindCtx->timeout_timer);
        }
        break;

    case SCAN_STATE_QR_ERROR:
        lv_label_set_text(g_scanBindCtx->status_label, "网络异常，无法加载二维码");
        lv_obj_clear_flag(g_scanBindCtx->action_btn, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(btn_label, "重新加载");
        break;

    case SCAN_STATE_SCAN_SUCCESS:
        lv_label_set_text(g_scanBindCtx->status_label, "扫码成功");
        lv_img_set_src(g_scanBindCtx->qr_img, LV_SYMBOL_OK);
        lv_obj_set_style_text_color(g_scanBindCtx->qr_img, lv_color_hex(0x00FF00), 0);
        //lv_obj_set_style_text_font(g_scanBindCtx->qr_img, &lv_font_montserrat_48, 0);
        lv_obj_clear_flag(g_scanBindCtx->qr_img, LV_OBJ_FLAG_HIDDEN);

        //g_scanBindCtx->delay_timer = lv_timer_create(scan_success_delay_timer, 2000, NULL);
        //lv_timer_set_repeat_count(g_scanBindCtx->delay_timer, 1); 
        break;

    case SCAN_STATE_GENERAL_LOADING:
        lv_label_set_text(g_scanBindCtx->status_label, "处理中，请稍候...");
        lv_obj_clear_flag(g_scanBindCtx->loading_anim, LV_OBJ_FLAG_HIDDEN);
        //send_bind_request();
        break;

    case SCAN_STATE_TIMEOUT:

        g_scanBindCtx->reload_btn = lv_btn_create(g_scanBindCtx->cont);
        lv_obj_set_size(g_scanBindCtx->reload_btn, 170, 170);
        lv_obj_add_style(g_scanBindCtx->reload_btn, get_global_style(STYLE_BTN_BASE), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_center(g_scanBindCtx->reload_btn);
        lv_obj_set_style_bg_color(g_scanBindCtx->reload_btn, lv_color_hex(0xF2F2F2), 0);
        lv_obj_set_style_radius(g_scanBindCtx->reload_btn, 8, 0);
#if DARK_MODE
        lv_obj_set_style_bg_color(g_scanBindCtx->reload_btn, lv_color_hex(0x212121), 0); // 深灰色按钮
#endif
        lv_obj_add_event_cb(g_scanBindCtx->reload_btn, action_btn_cb, LV_EVENT_CLICKED, NULL);


        lv_obj_t* reload_img = lv_img_create(g_scanBindCtx->reload_btn);
        lv_img_dsc_t* img = zb_img_res_load_one(NULL, SCAN_IMG_LOADING, IMG_FREE_TYPE_AUTO, NULL);
        lv_img_set_src(reload_img, img);
        lv_obj_align(reload_img, LV_ALIGN_CENTER, 0, -15);

        // 添加文字
        lv_obj_t* reload_label = lv_label_create(g_scanBindCtx->reload_btn);
        lv_label_set_text(reload_label, "重新加载二维码");
#if DARK_MODE
        lv_obj_set_style_text_color(reload_label, lv_color_hex(0xFFFFFF), 0); // 白色文本
#endif
        lv_obj_align(reload_label, LV_ALIGN_CENTER, 0, 25);
        lv_obj_set_style_text_color(reload_label, lv_color_hex(0x999999), 0);

        // 创建网络切换组件
        g_scanBindCtx->network_switch_btn = lv_btn_create(g_scanBindCtx->cont);
        lv_obj_set_size(g_scanBindCtx->network_switch_btn, 130, 40);
        lv_obj_add_style(g_scanBindCtx->network_switch_btn, get_global_style(STYLE_BTN_OPA), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align_to(g_scanBindCtx->network_switch_btn, g_scanBindCtx->reload_btn,
            LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
        lv_obj_set_style_bg_opa(g_scanBindCtx->network_switch_btn, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(g_scanBindCtx->network_switch_btn, 0, 0);
#if DARK_MODE
        lv_obj_set_style_bg_color(g_scanBindCtx->network_switch_btn, lv_color_hex(0x212121), 0); // 深灰色按钮
#endif

        lv_obj_add_event_cb(g_scanBindCtx->network_switch_btn, action_btn_cb, LV_EVENT_CLICKED, NULL);


        lv_obj_t* switch_label = lv_label_create(g_scanBindCtx->network_switch_btn);
        lv_label_set_recolor(switch_label, true); 
#if DARK_MODE
        // 暗黑模式下全部使用白色
        lv_label_set_text(switch_label, "#FFFFFF 点击# #FFFFFF 切换网络#");
#else
        lv_label_set_text(switch_label, "#999999 点击# #008CFF 切换网络#");
#endif
        lv_label_set_text(switch_label,
            "#999999 点击# "  
            "#008CFF 切换网络#" 
        );

        lv_obj_align(switch_label, LV_ALIGN_TOP_MID, 0, 0);      
        lv_obj_set_style_text_align(switch_label, LV_TEXT_ALIGN_CENTER, 0);
 
        break;

    case SCAN_STATE_SCAN_FAIL:
        lv_label_set_text(g_scanBindCtx->status_label, "绑定失败，请重试");
        lv_obj_clear_flag(g_scanBindCtx->action_btn, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(btn_label, "重新扫码");
        break;

    case SCAN_STATE_BIND_SUCCESS:
        // 隐藏所有现有元素
        lv_obj_add_flag(g_scanBindCtx->title_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_scanBindCtx->qr_img, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_scanBindCtx->status_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_scanBindCtx->action_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_scanBindCtx->loading_anim, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_scanBindCtx->IMEI, LV_OBJ_FLAG_HIDDEN);
        

        //stng_createEscBtn(g_scanBindCtx->cont, action_btn_cb,0);

#if 0
        // 创建返回按钮（左上角）
        g_scanBindCtx->back_btn = lv_obj_create(g_scanBindCtx->cont);
        lv_obj_set_size(g_scanBindCtx->back_btn, 40, 40);
        lv_obj_align(g_scanBindCtx->back_btn, LV_ALIGN_TOP_LEFT, 10, 10);
        lv_obj_t* back_label = lv_label_create(g_scanBindCtx->back_btn);
        lv_label_set_text(back_label, LV_SYMBOL_LEFT);
        lv_obj_add_style(g_scanBindCtx->back_btn, get_global_style(STYLE_BTN_OPA), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_event_cb(g_scanBindCtx->back_btn, action_btn_cb, LV_EVENT_CLICKED, NULL);
#endif
        // 创建成功UI容器
        g_scanBindCtx->success_panel = lv_obj_create(g_scanBindCtx->cont);
        lv_obj_remove_style_all(g_scanBindCtx->success_panel);
        lv_obj_set_size(g_scanBindCtx->success_panel, SCREEN_WIDTH, SCREEN_HEIGHT);
        lv_obj_center(g_scanBindCtx->success_panel);
        lv_obj_set_style_bg_opa(g_scanBindCtx->success_panel, LV_OPA_TRANSP, 0);

        // 添加成功图标
        lv_obj_t* success_img = lv_img_create(g_scanBindCtx->success_panel);
        lv_img_dsc_t* suc_img = zb_img_res_load_one(NULL, SCAN_IMG_SUCCEED, IMG_FREE_TYPE_AUTO, NULL);
        lv_img_set_src(success_img, suc_img);
        lv_obj_align(success_img, LV_ALIGN_CENTER, 0, -30);

        // 添加成功文本（在图标下方28像素）
        lv_obj_t* success_label = lv_label_create(g_scanBindCtx->success_panel);
        lv_label_set_text(success_label, "扫码成功");
#if DARK_MODE
        lv_obj_set_style_text_color(success_label, lv_color_hex(0xFFFFFF), 0); // 白色文本
#endif
        lv_obj_align_to(success_label, success_img, LV_ALIGN_OUT_BOTTOM_MID, 0, 28);
        break;
    }
}

static void handle_timeout(lv_timer_t* timer) {
    if (g_scanBindCtx->current_state == SCAN_STATE_QR_DISPLAY) {
        change_state(SCAN_STATE_TIMEOUT);
    }
}

static void handle_network_check(lv_timer_t* timer) {
    g_scanBindCtx->network_check_count++;

    // 检查是否超时（30秒 = 30次检测）
    if (g_scanBindCtx->network_check_count > 30) {
        if (g_scanBindCtx->current_state == SCAN_STATE_QR_LOADING) {
            change_state(SCAN_STATE_QR_ERROR);
    }
        return; // 超时后不再检测
}
    bool prev_net_status = g_scanBindCtx->network_available;
#ifndef C_PC_SIM
   // g_scanBindCtx->network_available = check_network_status();
    g_scanBindCtx->network_available = 1;
#else
    g_scanBindCtx->network_available = 1;
#endif

    // 网络状态变化处理
    if (prev_net_status != g_scanBindCtx->network_available) {
        if (g_scanBindCtx->network_available) {
            // 网络恢复：如果是错误状态则重新加载
            if (g_scanBindCtx->current_state == SCAN_STATE_QR_ERROR ||
                g_scanBindCtx->current_state == SCAN_STATE_TIMEOUT) {
                change_state(SCAN_STATE_QR_LOADING);
            }
        }
        else {
            // 网络断开：如果是需要网络的状态则进入错误
            if (g_scanBindCtx->current_state == SCAN_STATE_QR_LOADING ||
                g_scanBindCtx->current_state == SCAN_STATE_QR_DISPLAY) {
                change_state(SCAN_STATE_QR_ERROR);
            }
        }
    }
}

static void action_btn_cb(lv_event_t* e) {
    lv_obj_t* target = lv_event_get_target(e);

    if (target == g_scanBindCtx->reload_btn) {
        // 重新加载二维码
        change_state(SCAN_STATE_QR_LOADING);
        return;
    }
    else if (target == g_scanBindCtx->network_switch_btn) {
        // 切换网络
       // open_network_settings();
        return;
    }
    else if (target == g_scanBindCtx->back_btn) {
        // 返回操作
        scan_bind_screen_del();
        return;
    }

    switch (g_scanBindCtx->current_state) {
    case SCAN_STATE_QR_ERROR:
        if (g_scanBindCtx->network_available) {
            change_state(SCAN_STATE_QR_LOADING);
        }
        break;

    case SCAN_STATE_TIMEOUT:
        // 切换网络操作
       // open_network_settings();
        break;

    case SCAN_STATE_SCAN_FAIL:
        change_state(SCAN_STATE_INITIAL);
        break;

    case SCAN_STATE_BIND_SUCCESS:
       // exit_scan_bind_screen();
        lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
        break;

    default:
        break;
    }
}

static void user_interaction_handler(void) {
    // 重置超时计时器
    if (g_scanBindCtx->current_state == SCAN_STATE_QR_DISPLAY &&
        g_scanBindCtx->timeout_timer) {
        lv_timer_reset(g_scanBindCtx->timeout_timer);
    }
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void change_state(ScanState new_state) {
    if (g_scanBindCtx == NULL || g_scanBindCtx->current_state == new_state)
        return;

    // 切换状态时重置网络检测计数
    g_scanBindCtx->network_check_count = 0;

    // 退出当前状态处理
    switch (g_scanBindCtx->current_state) {
    case SCAN_STATE_QR_DISPLAY:
        lv_label_set_text(g_scanBindCtx->status_label, "请使用微信扫描二维码\n(30秒后自动超时)");
        if (g_scanBindCtx->timeout_timer) {
            lv_timer_pause(g_scanBindCtx->timeout_timer);
        }
        break;

    case SCAN_STATE_GENERAL_LOADING:
        // 停止加载动画
        break;

    default:
        break;
    }

    g_scanBindCtx->current_state = new_state;
    update_ui_for_state(new_state);
}

void scan_bind_screen_del(void) {
    if (g_scanBindCtx != NULL) {
      
        if (g_scanBindCtx->timeout_timer) {
            lv_timer_del(g_scanBindCtx->timeout_timer);
            g_scanBindCtx->timeout_timer = NULL;
        }
        if (g_scanBindCtx->delay_timer) {
            lv_timer_del(g_scanBindCtx->delay_timer);
            g_scanBindCtx->delay_timer = NULL;
        }

        if (g_scanBindCtx->cont) {
            lv_obj_del(g_scanBindCtx->cont);
            g_scanBindCtx->cont = NULL;
        }

        lv_mem_free(g_scanBindCtx);
        g_scanBindCtx = NULL;
    }
}

void scan_bind_screen_entry(void) {
    scan_bind_screen_del();  
    scan_bind_init();
    scan_bind_build_ui();
}

// 扫码成功处理
void handle_scan_success(const char* data) {
    if (g_scanBindCtx && g_scanBindCtx->current_state == SCAN_STATE_QR_DISPLAY) {
#ifndef C_PC_SIM
       // if (validate_scan_data(data)) {
        if(0){
#else
        if(1){
#endif
            change_state(SCAN_STATE_SCAN_SUCCESS);
        }
        else {
            change_state(SCAN_STATE_SCAN_FAIL);
        }
    }
}

// 绑定结果处理
void handle_bind_result(bool success, const char* nickname) {
    if (g_scanBindCtx && g_scanBindCtx->current_state == SCAN_STATE_GENERAL_LOADING) {
        if (success) {
            strncpy(g_scanBindCtx->user_nickname, nickname,
                sizeof(g_scanBindCtx->user_nickname) - 1);
            change_state(SCAN_STATE_BIND_SUCCESS);
        }
        else {
            change_state(SCAN_STATE_SCAN_FAIL);
        }
    }
}