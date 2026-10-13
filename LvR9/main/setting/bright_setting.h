//=========================================================
// bright_setting.c
//
// 
//Date: 2024-04-02
//========================================================

#ifndef BRIGHT_SETTIONG_H
#define BRIGHT_SETTIONG_H

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

#define SCR_OFF_LST_ITM_N 4
	/*Data of canvas*/
typedef struct {
	lv_obj_t obj;
	lv_obj_t * cont;
	lv_obj_t * title_label;
//	lv_obj_t * main_bg;
//	lv_obj_t * bright_area;
	lv_obj_t * slider;	
	
	uint16_t  bright;		//亮度值	
	lv_draw_rect_dsc_t br_rect;	//亮度设置矩形
	lv_draw_rect_dsc_t stime_rect;  //自动关屏设置矩形
	lv_area_t br_rect_area ; 
	lv_area_t stime_rect_area ; 

//	lv_obj_t * shut_scr_label; //息屏时间	
	lv_obj_t * minu_sec_label; //分钟数,秒数 标签
	uint16_t time_sec;		//息屏延时（秒数）
	uint16_t time_index;	//息屏延时（秒数 选择项）
	char* time_str;		//息屏延时 字符串
	uint8_t sel; 			//当前选中的调节项 ；0 --亮度条选中，1--息屏时间选中
	uint8_t stime_set_flag ; //分钟数设置面板状态，0--未进入，1--进入设置。

	
	lv_obj_t * as_win;  //自动息屏时间设置面板
	lv_obj_t * as_title_labe;  //自动息屏时间面板标题
	
	
	lv_obj_t * as_table;  //息屏时间列表
	int16_t  m_as_selNo;  // 息屏列表选项号 

	lv_obj_t* check_icons[SCR_OFF_LST_ITM_N];

	int current_selected_row;
} lv_brightness_set_t;


typedef struct {
	uint16_t sec;
	char* str; 
}brightness_time_str_t;

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*SETTIONG_H*/
