#include "style_tz.h"

static lv_style_t g_styles[STYLE_TYPE_COUNT];
static bool g_initialized = false;

void styles_init(void)
{
    if(g_initialized) return;

    // Focus样式
    lv_style_init(&g_styles[STYLE_FOCUS]);
    lv_style_set_border_width(&g_styles[STYLE_FOCUS], 0);
    lv_style_set_outline_width(&g_styles[STYLE_FOCUS], 0);
    lv_style_set_shadow_width(&g_styles[STYLE_FOCUS], 0);

    // 透明底按键(背景加入图片)
    lv_style_init(&g_styles[STYLE_BTN_OPA]);
    lv_style_set_radius(&g_styles[STYLE_BTN_OPA], 19);
    lv_style_set_shadow_width(&g_styles[STYLE_BTN_OPA], 0);
    lv_style_set_bg_color(&g_styles[STYLE_BTN_OPA], lv_color_hex(0xffffff));
    lv_style_set_bg_opa(&g_styles[STYLE_BTN_OPA], LV_OPA_0);
    lv_style_set_shadow_opa(&g_styles[STYLE_BTN_OPA], LV_OPA_0);


    //无边框按键
    lv_style_init(&g_styles[STYLE_BTN_BASE]);
    lv_style_set_radius(&g_styles[STYLE_BTN_BASE], 12);
    lv_style_set_border_width(&g_styles[STYLE_BTN_BASE], 0);
    lv_style_set_outline_width(&g_styles[STYLE_BTN_BASE], 0);
    lv_style_set_shadow_width(&g_styles[STYLE_BTN_BASE], 0);

    // 弹窗背景样式
    lv_style_init(&g_styles[STYLE_POPUP_BG]);
    lv_style_set_bg_color(&g_styles[STYLE_POPUP_BG], lv_color_hex(0x000000));
    lv_style_set_bg_opa(&g_styles[STYLE_POPUP_BG], LV_OPA_80);
    lv_style_set_border_width(&g_styles[STYLE_POPUP_BG], 0);
    lv_style_set_clip_corner(&g_styles[STYLE_POPUP_BG], true);
    lv_style_set_radius(&g_styles[STYLE_POPUP_BG], 20);

    // 创建主页面ui样式
    lv_style_init(&g_styles[STYLE_MAIN_BG]);
    lv_style_set_bg_opa(&g_styles[STYLE_MAIN_BG], LV_OPA_100);
    lv_style_set_border_width(&g_styles[STYLE_MAIN_BG], 0);
    lv_style_set_radius(&g_styles[STYLE_MAIN_BG], 0);
    lv_style_set_pad_all(&g_styles[STYLE_MAIN_BG], 0);
    
    // 居中文本
    lv_style_init(&g_styles[STYLE_LABEL_INFO]);
    lv_style_set_text_align(&g_styles[STYLE_LABEL_INFO], LV_TEXT_ALIGN_CENTER);
    //lv_label_set_long_mode(g_pContactAp.labelInf, LV_LABEL_LONG_WRAP);
    //lv_obj_center(g_pContactAp.labelInf);











    g_initialized = true;
}

lv_style_t* get_global_style(StyleType type)
{
    if(type >= STYLE_TYPE_COUNT || !g_initialized) {
        return NULL;
    }
    return &g_styles[type];
}
//lv_obj_add_style(objItem, get_global_style(STYLE_BTN_BASE), LV_PART_MAIN | LV_STATE_DEFAULT);

#if 0 //禁止滑动
//lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
//lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
#endif 

#if 0  //图片设置扩大点击区域
//lv_obj_t* img = lv_img_create(parent);
//const lv_img_dsc_t* pImgDsc = g_XXX_img_dsc_tbl + MMN_icn_esc - 1;
//lv_img_set_src(img, pImgDsc);
//lv_obj_set_size(img, pImgDsc->header.w + TOP_BTN_BORDER * 2, pImgDsc->header.h + TOP_BTN_BORDER * 2);
//lv_obj_set_pos(img, 16, STNG_ESC_BTN_LOC_Y);
//lv_obj_set_style_border_width(img, TOP_BTN_BORDER, 0);
//lv_obj_set_style_radius(img, 4, 0);
//lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
//
//lv_obj_set_style_opa(img, LV_OPA_60, 0);
//lv_obj_set_style_opa(img, LV_OPA_100, LV_STATE_FOCUS_KEY);
//lv_obj_set_style_opa(img, LV_OPA_80, LV_STATE_PRESSED);
//// lv_color_t colorBorderFcs = lv_color_white();
//lv_obj_set_style_border_opa(img, LV_OPA_0, 0);
//lv_obj_add_event_cb(img, clickCb, LV_EVENT_CLICKED, NULL);
#endif

#if 0
        // 1. 设置文本对齐方式为居中
lv_obj_set_style_text_align(g_pContactAp.labelInf, LV_TEXT_ALIGN_CENTER, 0);
//LV_SIZE_CONTENT
#endif