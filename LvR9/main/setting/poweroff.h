//=========================================================
// bright_setting.c
//
// 
//Date: 2024-04-02
//========================================================

#ifndef POWEROFF_H
#define POWEROFF_H

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

	/*Data of canvas*/
typedef struct {
   lv_obj_t* ui_Screen1;
   lv_obj_t* ui_Panel1;
   lv_obj_t* btn_tim;
   lv_obj_t* btn_label;
   lv_obj_t* ui_Label7;
   lv_obj_t* ui_Label1;
   lv_obj_t* pop;
   lv_obj_t* poptext;
   //lv_obj_t* popautooff;
   lv_obj_t* power;//aim
   lv_obj_t* ui_Checkbox;
   lv_obj_t* ui_Panelframe;

} lv_SETTING_poweroff;


typedef struct {
	uint16_t sec;
	char* str; 
}brightness_time_str_t;

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*SETTIONG_H*/
