//========================================================
//Name: qrcode_page.c
//Function: QR Code Display Page
//Author: Your Name
//Date: 2024-04-03
//========================================================
/**********************
     INCLUDES
 *********************/
#include <string.h>
#include <lvgl/lvgl.h>
#include "./app_setting_base.h"
#include "../utils/global_parm.h"
 /*********************
  *      DEFINES
  *********************/
#define QR_CODE_SIZE    120  // 二维码大小
#define PAGE_WIDTH      320  // 页面宽度
#define PAGE_HEIGHT     480  // 页面高度

  /**********************
   *  STATIC PROTOTYPES
   **********************/
static void qrcode_page_constructor(void);
static void qrcode_page_destructor(void);
static void qrcode_page_event_cb(lv_event_t* e);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t* qrcode_page = NULL;
static lv_obj_t* qrcode_label = NULL;
static lv_obj_t* qrcode_img = NULL;

extern const lv_img_dsc_t QRservice;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void qrcode_page_init(void) {
    qrcode_page_constructor();
}

void qrcode_page_deinit(void) {
    qrcode_page_destructor();
}

static void _stng_esc_btn_click_cb(lv_event_t* evt)
{
    lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
    return;
}


/**********************
 *   STATIC FUNCTIONS
 **********************/
static void qrcode_page_constructor(void) {
    // 创建页面容器
    qrcode_page = lv_obj_create(lv_scr_act());
    lv_obj_set_style_pad_all(qrcode_page, 0, 0);
    lv_obj_set_size(qrcode_page, PAGE_WIDTH, PAGE_HEIGHT);
    lv_obj_set_style_border_width(qrcode_page, 0, 0);
    lv_obj_set_style_radius(qrcode_page, 0, 0);
    lv_obj_clear_flag(qrcode_page, LV_OBJ_FLAG_SCROLLABLE);
#if DARK_MODE
    lv_obj_set_style_bg_color(qrcode_page, lv_color_hex(0x000000), 0); // 黑色背景
#else
    lv_obj_set_style_bg_color(qrcode_page, lv_color_hex(0xF2F2F2), 0);
#endif
    sys_setCurMainWinAndCb(qrcode_page, qrcode_page_event_cb);

    stng_createEscBtn(qrcode_page, _stng_esc_btn_click_cb, 0);

    // 创建标题标签
    lv_obj_t* title_label = lv_label_create(qrcode_page);
    lv_label_set_text(title_label, "微信客服");
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 20);
    //lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, 0);
#if DARK_MODE
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0); // 白色文本
#else
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x18191B), 0);
#endif

#if 0
    LV_LOG_USER("进入显示二维码");
    lv_coord_t  qr_size;
    lv_obj_t* qr_login = lv_qrcode_create((qrcode_page),
        qr_size = 210,
        lv_color_hex3(0x000000),
        lv_color_hex3(0xFFFFFF));
    /* 设置数据 */

    lv_qrcode_update(qr_login, "http://weixin.qq.com/q/023s1ywzLKfcl1CmBrhCcz", strlen("http://weixin.qq.com/q/023s1ywzLKfcl1CmBrhCcz"));
    /* 二维码中间对齐 */
    lv_obj_align(qr_login, LV_ALIGN_TOP_LEFT,35,127);
    lv_obj_set_style_border_width(qr_login, 15, 0);
    lv_obj_set_style_border_color(qr_login, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_radius(qr_login, 12, 0);
#else
    lv_obj_t* player_img = lv_img_create(qrcode_page);
    lv_obj_align(player_img, LV_ALIGN_CENTER, 0, 0);

    lv_img_dsc_t* pImgDsc_img = &QRservice;
    lv_img_set_src(player_img, pImgDsc_img);
    //lv_obj_align(player_img, LV_ALIGN_TOP_LEFT, 35, 127);

    //lv_obj_set_style_radius(player_img, 12, 0);           
    //lv_obj_set_style_pad_all(player_img, 14, 0);         
    //lv_obj_set_style_clip_corner(player_img, true, 0);    

    lv_img_set_zoom(player_img, 128);
    lv_obj_align(player_img, LV_ALIGN_CENTER, 0, 0);
#if 0
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_transform_zoom(&style, 256 * 0.6); // 缩小40%

    // 设置变换中心点（百分比）
    lv_style_set_transform_pivot_x(&style, 50);
    lv_style_set_transform_pivot_y(&style, 50);

    lv_obj_add_style(player_img, &style, 0);
    //lv_obj_center(img);

    // 仍然需要调整大小
    lv_obj_set_size(player_img,
        (lv_coord_t)(pImgDsc_img->header.w * 0.6),
        (lv_coord_t)(pImgDsc_img->header.h * 0.6));

#endif
#endif
    // 创建二维码文本标签
    qrcode_label = lv_label_create(qrcode_page);
    lv_label_set_text(qrcode_label, "请使用微信扫描二维码");
    lv_obj_align(qrcode_label, LV_ALIGN_TOP_MID, 0, 380);
#if DARK_MODE
    lv_obj_set_style_text_color(qrcode_label, lv_color_hex(0xFFFFFF), 0); // 白色文本
#else
    lv_obj_set_style_text_color(qrcode_label, lv_color_hex(0x18191B), 0);
#endif

}

static void qrcode_page_destructor(void) {
    if (qrcode_page != NULL) {
        lv_obj_del(qrcode_page);
        qrcode_page = NULL;
    }
}

static void qrcode_page_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_KEY) {
        int32_t c = *((int32_t*)lv_event_get_param(e));

        if (c == LV_KEY_ESC) {
            lv_indev_keypad_wait_release();
            lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
        }
    }
}
