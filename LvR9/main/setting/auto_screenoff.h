//=========================================================
// auto_screenoff.h
//
// 
//Date: 2024-04-02
//========================================================

#ifndef AUTO_SCREENOFF_H
#define AUTO_SCREENOFF_H

#ifdef __cplusplus
extern "C" {
#endif

    /*********************
     *      INCLUDES
     *********************/
#include <lvgl/lvgl.h>

     /*********************
      *      DEFINES
      *********************/
#define OPTION_COUNT 6  // 1/3/5/10/15/20分钟

      /*Data of canvas*/
    typedef struct {
        lv_obj_t obj;
        lv_obj_t* cont;
        lv_obj_t* title_label;

        uint16_t time_sec;        //息屏延时（秒数）
        uint16_t time_index;    //息屏延时（秒数 选择项）
        char* time_str;        //息屏延时 字符串
        uint8_t stime_set_flag; //分钟数设置面板状态，0--未进入，1--进入设置。

        lv_obj_t* check_icons[OPTION_COUNT]; // 选中图标数组
        int current_selected_row;
    } lv_brightness_set_t;

    typedef struct {
        uint16_t sec;
        char* str;
    } brightness_time_str_t;

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*AUTO_SCREENOFF_H*/