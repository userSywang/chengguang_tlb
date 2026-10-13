#include "alipay_page_manager.h"
#include "../utils/misc_utils.h"
#include "../data_parse/img_res.h"
#include "../utils/style_tz.h"

static void back_button_cb(lv_event_t* e);
static lv_obj_t* alipay_create_qrcode_simple(lv_obj_t* parent, const char* qr_data, lv_coord_t x, lv_coord_t y);
static void render_settings_state();

/*********************
 * 页面全局变量定义
 ********************/
static lv_obj_t* g_menu_root = NULL;      // 菜单页面根容器
static lv_obj_t* g_payment_root = NULL;    // 付款码页面根容器
static lv_obj_t* g_bind_root = NULL;       // 绑定页面根容器
static lv_timer_t* g_bind_timer = NULL;    // 绑定页面计时器



/************************************************************
 * 通用UI组件创建函数 (复用组件)
 ***********************************************************/

 // 创建标题栏组件
static lv_obj_t* create_header(lv_obj_t* parent, const char* title, lv_coord_t height, uint8_t has_back) {
    // 头部容器
    lv_obj_t* header = lv_obj_create(parent);
    lv_obj_set_size(header, lv_pct(100), height);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_bg_opa(header, 0, 0);
    //lv_obj_set_style_opa(header, 0, 0);
    lv_obj_set_align(header, LV_ALIGN_TOP_MID);

    if (has_back) {
        lv_obj_t* back_img = lv_img_create(header);
        lv_img_set_src(back_img, zb_img_res_load_one(NULL, ALIPAY_IMG_RETURN_ICON, IMG_FREE_TYPE_AUTO, NULL));
        lv_obj_set_ext_click_area(back_img, 15);
        lv_obj_add_flag(back_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(back_img, back_button_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_align(back_img, LV_ALIGN_LEFT_MID,20,0);
    }

    // 主标题
    lv_obj_t* title_label = lv_label_create(header);
    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
#ifndef C_PC_SIM
    lv_label_set_cjk_font(title_label, 18, FT_FONT_STYLE_NORMAL);
#endif
    lv_obj_center(title_label);

    return header;
}

// 创建功能卡片组件
static lv_obj_t* create_function_card(lv_obj_t* parent, const char* text) {
    // 卡片容器
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, 280, 64);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x1F1F1F), 0);
    lv_obj_set_style_radius(card, 8, 0);
    lv_obj_set_style_border_width(card, 0, 0);
   // lv_obj_set_style_border_color(card, bg_color, 0);


    // 功能文本
    lv_obj_t* text_label = lv_label_create(card);
    lv_label_set_text(text_label, text);
    lv_obj_align(text_label, LV_ALIGN_LEFT_MID, 60, 0);
    lv_obj_set_style_text_color(text_label, lv_color_white(),0);
    //lv_obj_set_style_text_font(text_label, &lv_font_montserrat_18, 0);

    // 右侧箭头
    lv_obj_t* arrow_img = lv_img_create(card);
    lv_img_set_src(arrow_img, zb_img_res_load_one(NULL, SETTING_IMG_ARROW, IMG_FREE_TYPE_AUTO, NULL));
    lv_obj_align(arrow_img, LV_ALIGN_RIGHT_MID, -16, 0);

    return card;
}

/**
 * 创建渐变色按钮(多用于付款状态使用)
 */
lv_obj_t* create_gradient_button(lv_obj_t* parent, lv_coord_t x, lv_coord_t y,
    lv_coord_t width, lv_coord_t height,
    lv_coord_t radius, lv_color_t color_start,
    lv_color_t color_end, lv_grad_dir_t grad_dir) {
    // 创建按钮对象
    lv_obj_t* btn = lv_obj_create(parent);

    // 设置按钮的位置和大小
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, width, height);

    // 创建并初始化样式
    static lv_style_t style_btn;
    lv_style_init(&style_btn);

    // 设置背景为渐变
    lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
    lv_style_set_bg_color(&style_btn, color_start);
    lv_style_set_bg_grad_color(&style_btn, color_end);
    lv_style_set_bg_grad_dir(&style_btn, grad_dir);

    // 设置圆角
    lv_style_set_radius(&style_btn, radius);

    // 设置内边距为0
    lv_style_set_pad_all(&style_btn, 0);

    // 移除边框
    lv_style_set_border_width(&style_btn, 0);

    // 应用样式到按钮
    lv_obj_add_style(btn, &style_btn, LV_STATE_DEFAULT);

    return btn;
}

/**
 * 创建统一规格按钮
 *
 * @param parent 父容器
 * @param cfg 按钮配置
 * @param y_pos 在父容器中的Y坐标位置
 * @return 创建的按钮句柄
 */
static lv_obj_t* create_unified_button(lv_obj_t* parent, button_config_t cfg, lv_coord_t y_pos) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 280, 48);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, y_pos);
    lv_obj_set_style_bg_color(btn, cfg.bg_color, 0);
    lv_obj_set_style_radius(btn, 67, 0);

    lv_obj_t* btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, cfg.text);
    lv_obj_set_style_text_color(btn_label, cfg.text_color, 0);
    lv_obj_center(btn_label);

    if (cfg.callback) {
        lv_obj_add_event_cb(btn, cfg.callback, LV_EVENT_CLICKED, NULL);
    }

    return btn;
}

/************************************************************
 * 菜单页面实现
 ***********************************************************/
static void payment_btn_cb(lv_event_t* e);
static void settings_btn_cb(lv_event_t* e);
static void help_btn_cb(lv_event_t* e);




int menu_page_init(int param) {
#define change_version 1
    // 创建根容器
    g_menu_root = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_menu_root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(g_menu_root, lv_color_black(), 0);
    lv_obj_add_style(g_menu_root, get_global_style(STYLE_MAIN_BG), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 页面标题 (无返回按钮)
    create_header(g_menu_root, "支付宝", 60, 1);

#if !change_version   
    // 菜单容器
    lv_obj_t* menu_cont = lv_obj_create(g_menu_root);
    lv_obj_set_size(menu_cont, LV_PCT(90), LV_PCT(75));
    lv_obj_align(menu_cont, LV_ALIGN_TOP_MID, 0, 70);
    lv_obj_set_flex_flow(menu_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menu_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(menu_cont, 0, 0);
    lv_obj_set_style_pad_row(menu_cont, 12, 0);
    lv_obj_set_style_border_width(menu_cont, 0, 0);
    lv_obj_set_style_bg_opa(menu_cont, LV_OPA_TRANSP, 0);

    // 创建功能卡片 (对应图片中的三个按钮)
    // 付款码按钮 (蓝色)
    lv_obj_t* payment_card = create_function_card(menu_cont, "付款码", ALIPAY_IMG_PAYMENT);
    lv_obj_add_event_cb(payment_card, payment_btn_cb, LV_EVENT_CLICKED, NULL);

    // 设置按钮 (绿色)
    lv_obj_t* settings_card = create_function_card(menu_cont, "设置", ALIPAY_IMG_SETTING);
    lv_obj_add_event_cb(settings_card, settings_btn_cb, LV_EVENT_CLICKED, NULL);

    // 帮助按钮 (橙色)
    lv_obj_t* help_card = create_function_card(menu_cont, "帮助", ALIPAY_IMG_HELP);
    lv_obj_add_event_cb(help_card, help_btn_cb, LV_EVENT_CLICKED, NULL);
#else

    lv_obj_t* money = lv_label_create(g_menu_root);
    lv_obj_set_size(money, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_label_set_text(money, "零花钱");
    lv_obj_align(money, LV_ALIGN_TOP_LEFT, 20, 64);
    lv_obj_set_style_text_color(money, lv_color_hex(0xFFFFFF),0);

    lv_obj_t* money_img = lv_img_create(money);
    lv_img_set_src(money_img, zb_img_res_load_one(NULL, ALIPAY_IMG_MONEY, IMG_FREE_TYPE_AUTO, NULL));
    lv_obj_align_to(money_img, money, LV_ALIGN_BOTTOM_LEFT, 0, 30);


    lv_obj_t* card = lv_obj_create(g_menu_root);
    lv_obj_set_size(card, 102, 140);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x1F1F1F), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_TRANSP, 0); 
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_add_event_cb(card, payment_btn_cb, LV_EVENT_CLICKED, NULL);


     // 图标
    lv_obj_t* icon_img = lv_img_create(card);
    lv_img_set_src(icon_img, zb_img_res_load_one(NULL, ALIPAY_IMG_PAYQR, IMG_FREE_TYPE_AUTO, NULL));
    lv_obj_align(icon_img, LV_ALIGN_TOP_MID, 0, 0);

    // 功能文本
    lv_obj_t* text_label = lv_label_create(card);
    lv_label_set_text(text_label, "付款码");
    lv_obj_align_to(text_label, icon_img, LV_ALIGN_BOTTOM_MID, 0, 36);
    lv_obj_set_style_text_color(text_label, lv_color_white(), 0);

    lv_obj_align(card, LV_ALIGN_TOP_LEFT, 32, 176);


    lv_obj_t* settings_card = create_function_card(g_menu_root, "家长设置");
    lv_obj_align(settings_card,LV_ALIGN_TOP_LEFT,20,336);


    lv_obj_add_event_cb(settings_card, settings_btn_cb, LV_EVENT_CLICKED, NULL);
#endif


    return 0;
}

int menu_page_free(void) {
    if (g_menu_root) {
        lv_obj_del(g_menu_root);
        g_menu_root = NULL;
    }
    return 0;
}

// 功能按钮回调函数
static void payment_btn_cb(lv_event_t* e) {
    alipay_page_switch(ALIPAY_PAGE_PAYMENT, 0);
}

static void settings_btn_cb(lv_event_t* e) {
    alipay_page_switch(ALIPAY_PAGE_SETTINGS, 0);
}

static void help_btn_cb(lv_event_t* e) {
    alipay_page_switch(ALIPAY_PAGE_BIND, 0);
}

/************************************************************
 * 付款码页面实现
 ***********************************************************/
static void payment_back_cb(lv_event_t* e);
static void switch_code_cb(lv_event_t* e);

typedef enum {
    PAY_SHOWING_QRCODE,
    PAY_SHOWING_BARCODE
} pay_page_show_state_t;

static pay_page_show_state_t g_show_state = PAY_SHOWING_QRCODE;
static lv_obj_t* g_qrcode_container = NULL;
static lv_obj_t* g_barcode_container = NULL;

// 滑动切换回调函数
static void swipe_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_target(e);

    if (code == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir == LV_DIR_LEFT || dir == LV_DIR_RIGHT) {
            // 切换显示状态
            if (g_show_state == PAY_SHOWING_QRCODE) {
                // 滑动到条形码
                lv_obj_add_flag(g_qrcode_container, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(g_barcode_container, LV_OBJ_FLAG_HIDDEN);
                g_show_state = PAY_SHOWING_BARCODE;
            }
            else {
                // 滑动到二维码
                lv_obj_add_flag(g_barcode_container, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(g_qrcode_container, LV_OBJ_FLAG_HIDDEN);
                g_show_state = PAY_SHOWING_QRCODE;
            }
        }
    }
}

int payment_page_init(int param) {
    // 创建根容器
    g_payment_root = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_payment_root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(g_payment_root, lv_color_hex(0x1A5FD8), 0); // 蓝色背景
    lv_obj_set_style_border_width(g_payment_root, 0, 0);
    lv_obj_add_style(g_payment_root, get_global_style(STYLE_MAIN_BG), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 页面标题 (有返回按钮)
    lv_obj_t* header = create_header(g_payment_root, "付款码", 60, 1);

    // 副标题
    lv_obj_t* subtitle_label = lv_label_create(g_payment_root);
    lv_label_set_text(subtitle_label, "收银员扫一扫收钱");
    lv_obj_set_style_text_color(subtitle_label, lv_color_white(), 0);
    //lv_obj_set_style_text_font(subtitle_label, &lv_font_montserrat_16, 0);
    lv_obj_align(subtitle_label, LV_ALIGN_TOP_MID, 0, 82);

    // 创建滑动容器
    lv_obj_t* swipe_area = lv_obj_create(g_payment_root);
    lv_obj_set_size(swipe_area, LV_PCT(100), LV_PCT(80)); // 80%高度用于滑动区域
    lv_obj_set_style_bg_opa(swipe_area, LV_OPA_TRANSP, 0); // 透明背景
    lv_obj_set_style_border_width(swipe_area, 0, 0);
    lv_obj_align(swipe_area, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_add_event_cb(swipe_area, swipe_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_flag(swipe_area, LV_OBJ_FLAG_CLICKABLE);    // 可点击
    lv_obj_add_flag(swipe_area, LV_OBJ_FLAG_GESTURE_BUBBLE); // 允许手势事件冒泡




    // 创建二维码容器
    g_qrcode_container = lv_obj_create(swipe_area);
    lv_obj_set_size(g_qrcode_container, LV_PCT(90), 240);
    lv_obj_set_style_bg_opa(g_qrcode_container, LV_OPA_TRANSP, 0); // 透明背景
    lv_obj_set_style_border_width(g_qrcode_container, 0, 0);
    lv_obj_align_to(g_qrcode_container, subtitle_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    lv_obj_clear_flag(g_qrcode_container, LV_OBJ_FLAG_SCROLLABLE);

    // 创建二维码
    lv_obj_t* qrcode = alipay_create_qrcode_simple(g_qrcode_container, "http://weixin.qq.com/q/023s1ywzLKfcl1CmBrhCcz", 0, 0);
    lv_obj_align(qrcode, LV_ALIGN_CENTER, 0, 0);

    // 创建条形码容器 (初始隐藏)
    g_barcode_container = lv_obj_create(swipe_area);
    lv_obj_set_size(g_barcode_container, LV_PCT(90), 240);
    lv_obj_set_style_bg_opa(g_barcode_container, LV_OPA_TRANSP, 0); // 透明背景
    lv_obj_set_style_border_width(g_barcode_container, 0, 0);
    lv_obj_align_to(g_barcode_container, subtitle_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    lv_obj_add_flag(g_barcode_container, LV_OBJ_FLAG_HIDDEN); // 初始隐藏
    lv_obj_clear_flag(g_barcode_container, LV_OBJ_FLAG_SCROLLABLE);

    // 创建条形码 (这里用白色矩形模拟)
    lv_obj_t* barcode = lv_obj_create(g_barcode_container);
    lv_obj_set_size(barcode, LV_PCT(90), 100);
    lv_obj_set_style_bg_color(barcode, lv_color_hex(0xF3F3F3), 0);
    lv_obj_set_style_border_width(barcode, 0, 0);
    lv_obj_align(barcode, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t* icon_img = lv_img_create(barcode);
    lv_img_set_src(icon_img, zb_img_res_load_one(NULL, ALIPAY_IMG_YOUDAO_LABEL, IMG_FREE_TYPE_AUTO, NULL));
    lv_obj_align(icon_img, LV_ALIGN_CENTER, 0, 0);

    // 创建条形码文本
    lv_obj_t* barcode_text = lv_label_create(g_barcode_container);
    lv_label_set_text(barcode_text, "1234567890123");
    lv_obj_set_style_text_color(barcode_text, lv_color_white(), 0);
    //lv_obj_set_style_text_font(barcode_text, &lv_font_montserrat_20, 0);
    lv_obj_align_to(barcode_text, barcode, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // 创建切换标签
    lv_obj_t* switch_label = lv_label_create(g_payment_root);
    lv_label_set_text(switch_label, "切换条形码");
    lv_obj_add_flag(switch_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_text_color(switch_label, lv_color_white(), 0);
    lv_obj_set_style_text_decor(switch_label, LV_TEXT_DECOR_UNDERLINE, 0); // 下划线
    lv_obj_set_style_text_color(switch_label, lv_color_hex(0x0A3F8C), LV_STATE_PRESSED); // 按下时深蓝色

    // 底部对齐
    lv_obj_align(switch_label, LV_ALIGN_BOTTOM_MID, 0, -30);

    // 添加点击切换事件
    lv_obj_add_event_cb(switch_label, switch_code_cb, LV_EVENT_CLICKED, NULL);

    return 0;
}
int payment_page_free(void) {
    if (g_payment_root) {
        lv_obj_del(g_payment_root);
        g_payment_root = NULL;
    }
    return 0;
}


/************************************************************
 * 设置页面实现
 ***********************************************************/
static lv_obj_t* g_settings_root = NULL;    // 设置页面根容器


int settings_page_free(void) {
    if (g_settings_root) {
        lv_obj_del(g_settings_root);
        g_settings_root = NULL;
    }
    return 0;
}

int settings_page_init(int param) {
    // 创建根容器
    g_settings_root = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_settings_root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(g_settings_root, lv_color_black(), 0);
    lv_obj_add_style(g_settings_root, get_global_style(STYLE_MAIN_BG), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* header = create_header(g_settings_root, "家长设置", 60, 1);


    // ====== 家长设置管理区域 ======
    lv_obj_t* settings_container = lv_obj_create(g_settings_root);
    lv_obj_set_size(settings_container, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(settings_container, LV_OPA_TRANSP, 0); // 透明背景
    lv_obj_set_style_border_width(settings_container, 0, 0); // 无边框
    lv_obj_set_style_pad_top(settings_container, 0, 0);
    lv_obj_set_style_pad_bottom(settings_container, 20, 0);
    lv_obj_align_to(settings_container, header, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);

    // "家长设置管理"标题
    lv_obj_t* parent_title = lv_label_create(settings_container);
    lv_label_set_text(parent_title, "家长设置管理");
    lv_obj_set_style_text_color(parent_title, lv_color_hex(0xCCCCCC), 0); // 浅灰色
    //lv_obj_set_style_text_font(parent_title, &lv_font_montserrat_20, 0);
    lv_obj_align(parent_title, LV_ALIGN_TOP_LEFT, 20, 0);

    // 创建管理选项列表
    lv_obj_t* list = lv_obj_create(settings_container);
    lv_obj_set_size(list, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0); // 透明背景
    lv_obj_set_style_border_width(list, 0, 0); // 无边框
    lv_obj_align_to(list, parent_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    // 管理零花钱
    lv_obj_t* list_item1 = lv_label_create(list); 
    lv_label_set_text(list_item1,".管理零花钱\n.管理手表支付宝功能\n.解绑手表");
    lv_obj_set_style_text_color(list_item1, lv_color_white(), 0);



    // ====== 二维码区域 ======
    lv_obj_t* qr_container = lv_obj_create(g_settings_root);
    lv_obj_set_size(qr_container, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(qr_container, LV_OPA_TRANSP, 0); // 透明背景
    lv_obj_set_style_border_width(qr_container, 0, 0); // 无边框
    lv_obj_set_flex_flow(qr_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(qr_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_align(qr_container, LV_ALIGN_CENTER, 0, 80);

    // "家长手机扫码查看协议"标题
    lv_obj_t* qr_title = lv_label_create(qr_container);
    lv_label_set_text(qr_title, "家长手机扫码查看协议");
    lv_obj_set_style_text_color(qr_title, lv_color_hex(0xCCCCCC), 0); // 浅灰色
    //lv_obj_set_style_text_font(qr_title, &lv_font_montserrat_20, 0);
    //lv_obj_set_style_margin_bottom(qr_title, 20, 0);

    // 二维码容器
    lv_obj_t* qr_code_bg = lv_obj_create(qr_container);
    lv_obj_set_size(qr_code_bg, 200, 200); // 固定尺寸
    lv_obj_set_style_bg_color(qr_code_bg, lv_color_black(), 0);
    lv_obj_set_style_border_color(qr_code_bg, lv_color_white(), 0);
    lv_obj_set_style_border_width(qr_code_bg, 2, 0); // 白色边框
    lv_obj_set_style_radius(qr_code_bg, 8, 0); // 圆角

    // 添加二维码图片 (实际项目中应使用真实的二维码图片)
    lv_obj_t* qr_img = alipay_create_qrcode_simple(qr_code_bg, "http://weixin.qq.com/q/023s1ywzLKfcl1CmBrhCcz", 0, 0);
    lv_obj_align(qr_img, LV_ALIGN_CENTER, 0, 0);


    // "请用支付宝188******98扫码"提示文字
    lv_obj_t* scan_tip = lv_label_create(qr_container);
    lv_label_set_text(scan_tip, "请用支付宝188******98扫码");
    lv_obj_set_style_text_color(scan_tip, lv_color_hex(0x888888), 0); // 深灰色
    //lv_obj_set_style_text_font(scan_tip, &lv_font_montserrat_16, 0);
    //lv_obj_set_style_margin_top(scan_tip, 20, 0);





    lv_obj_t* settings_card = create_function_card(g_menu_root, "查看协议");
    lv_obj_align(settings_card, LV_ALIGN_TOP_LEFT, 20, 336);



    return 0;
}


// 返回按钮回调
static void payment_back_cb(lv_event_t* e) {
    alipay_page_switch(ALIPAY_PAGE_MENU, 0);
}

// 切换按钮回调
static void switch_code_cb(lv_event_t* e) {
    // 这里可以添加二维码/条形码切换逻辑
    static bool stat = false;

    stat ^= 1;
    if (stat)
    {
        lv_obj_clear_flag(g_barcode_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_qrcode_container, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(g_barcode_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_qrcode_container, LV_OBJ_FLAG_HIDDEN);
    }
}

/************************************************************
 * 扫码绑定页面实现
 ***********************************************************/
static void bind_back_cb(lv_event_t* e);
static void bind_timeout_cb(lv_timer_t* timer);

int bind_page_init(int param) {
    // 创建根容器
    g_bind_root = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_bind_root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(g_bind_root, lv_color_black(), 0);
    lv_obj_add_style(g_bind_root, get_global_style(STYLE_MAIN_BG), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 页面标题 (有返回按钮)
    lv_obj_t* header = create_header(g_bind_root, "帮助", 60, 1);

    lv_obj_t* qrcode = alipay_create_qrcode_simple(g_bind_root, "http://weixin.qq.com/q/023s1ywzLKfcl1CmBrhCcz", 0, 0);
    lv_obj_align(qrcode, LV_ALIGN_CENTER, 0, 0);

    // 提示文字
    lv_obj_t* hint_label = lv_label_create(g_bind_root);
    lv_label_set_text(hint_label, "请使用支付宝扫码求助");
    lv_obj_set_style_text_color(hint_label, lv_color_white(), 0);
#ifndef C_PC_SIM
    lv_label_set_cjk_font(hint_label, 18, FT_FONT_STYLE_NORMAL);
#endif
    lv_obj_align(hint_label, LV_ALIGN_TOP_MID, 0,78);

    // 设置绑定超时时间（5秒）
    //g_bind_timer = lv_timer_create(bind_timeout_cb, 5000, NULL);

    return 0;
}

int bind_page_free(void) {
    if (g_bind_timer) {
        lv_timer_del(g_bind_timer);
        g_bind_timer = NULL;
    }

    if (g_bind_root) {
        lv_obj_del(g_bind_root);
        g_bind_root = NULL;
    }
    return 0;
}

// 返回按钮回调
static void bind_back_cb(lv_event_t* e) {
    alipay_page_switch(ALIPAY_PAGE_MENU, 0);
}

// 绑定超时回调
static void bind_timeout_cb(lv_timer_t* timer) {
    // 5秒后自动返回主菜单
    alipay_page_switch(ALIPAY_PAGE_MENU, 0);
}

/************************************************************
 * 通用回调函数
 ***********************************************************/

 // 返回按钮通用回调
static void back_button_cb(lv_event_t* e) {
    alipay_page_t current = alipay_get_current_page();

    switch (current) {
    case ALIPAY_PAGE_PAYMENT:
    case ALIPAY_PAGE_BIND:
    case ALIPAY_PAGE_SETTINGS:
        alipay_page_switch(ALIPAY_PAGE_MENU, 0);
        break;
    default:

        LV_LOG_USER("switch ap \r\n");
        zb_requestSwitchAp(C_ZB_AP_MAINMENU);
        break;
    }
}

static lv_obj_t* alipay_create_qrcode_simple(lv_obj_t* parent, const char* qr_data, lv_coord_t x, lv_coord_t y) {
    
    const lv_coord_t size = 240;                  
    const lv_color_t dark_color = lv_color_hex(0x000000);  
    const lv_color_t light_color = lv_color_hex(0xFFFFFF);  
    const lv_coord_t border_width = 7;            
    const lv_coord_t radius = 12;               

   
    lv_obj_t* qr = lv_qrcode_create(parent, size, dark_color, light_color);

   
    if (qr_data && *qr_data) {
        lv_qrcode_update(qr, qr_data, strlen(qr_data));
    }

   
    lv_obj_set_pos(qr, x, y);

    
    lv_obj_set_style_border_width(qr, border_width, 0);
    lv_obj_set_style_border_color(qr, light_color, 0); 
    lv_obj_set_style_radius(qr, radius, 0);

    return qr;
}

/*********************
 * 页面函数表定义
 ********************/
const alipay_page_info_t page_table[ALIPAY_PAGE_MAX] = {
    [ALIPAY_PAGE_MENU] = {menu_page_init, menu_page_free},
    [ALIPAY_PAGE_PAYMENT] = {payment_page_init, payment_page_free},
    [ALIPAY_PAGE_BIND] = {bind_page_init, bind_page_free},
    [ALIPAY_PAGE_SETTINGS] = {settings_page_init, settings_page_free}
};