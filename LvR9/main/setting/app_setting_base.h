#ifndef __APP_SETTING_BASE_H__
#define __APP_SETTING_BASE_H__

#include "../mainmenu/tlb_main_img.h"
#include "../mainmenu/mainmenu_base.h"
#include "bluetooth.h"


// 暗色模式定义
#if DARK_MODE
#define DARK_BG_COLOR        0x000000  // 纯黑背景
#define DARK_TEXT_COLOR      lv_color_white() // 白色文字
#define DARK_BUTTON_BG_COLOR 0x212121  // 按钮背景色
#define DARK_LIST_BG_COLOR   0x1A1A1A  // 列表背景色
#define DARK_LIST_ITEM_COLOR 0x2D2D2D  // 列表项背景色
#define DARK_CONTAINER_COLOR 0x212121 // 容器背景色
#else
#define DARK_BG_COLOR        STNG_AP_BG_COLOR_HEX  // 原背景色
#define DARK_TEXT_COLOR      lv_color_black() // 黑色文字
#define DARK_BUTTON_BG_COLOR 0xFFFFFF  // 白色按钮背景
#define DARK_LIST_BG_COLOR   0xF4F4F4  // 原列表背景色
#define DARK_LIST_ITEM_COLOR 0xFFFFFF  // 原列表项背景色
#define DARK_CONTAINER_COLOR 0xFFFFFF // 容器背景色
#endif







extern int setng_menu_scr_init(int parm);

extern lv_obj_t* stng_createEscBtn(lv_obj_t* parent, lv_event_cb_t clickCb, int extra);
extern void stng_createDelBtn(lv_obj_t* parent, lv_event_cb_t clickCb, int extra, int x, int y);

extern void set_g_setngSel_num(int num);


extern void _stng_curScr_del(void);

extern const lv_img_dsc_t  g_XXX_img_dsc_tbl[];
extern const lv_img_dsc_t  g_setting_img_dsc[];

#define CMD_PAR_RET_STNG_MENU  0xb1

#define STNG_AP_BG_COLOR_HEX  0xf2f2f2

#ifndef ESP_OK
#define ESP_OK 0 
#endif

#endif
