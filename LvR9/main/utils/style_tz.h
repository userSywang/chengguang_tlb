#ifndef __STYLES_H__
#define __STYLES_H__

#include "lvgl/lvgl.h"

typedef enum {
    STYLE_FOCUS,
    STYLE_BTN_OPA,   //透明按键(用于加入背景图)
    STYLE_BTN_BASE,  //无边框按键
    STYLE_POPUP_BG,  //弹窗背景
    STYLE_MAIN_BG,  //主页面统一基本样式(边距/圆角/不可滑动)
    STYLE_LABEL_INFO, //居中文本
    STYLE_TYPE_COUNT

} StyleType;


void styles_init(void);


lv_style_t* get_global_style(StyleType type);

#endif