//=========================================================
// my_device.h
//
// 
//Date: 2024-04-02
//========================================================

#ifndef MY_DEVICE_H
#define MY_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>
#include "../utils/global_parm.h"

/*********************
 *      DEFINES
 *********************/

	/*Data of canvas*/
typedef struct {
	int8_t   m_isPwdEdtScr;
	lv_obj_t * cont;
	lv_obj_t * title_label;

	lv_obj_t * ip_label; //IP 地址
	lv_obj_t * sn_label; //SN 号
	lv_obj_t * ver_label;	//软件版本信息
	lv_obj_t* ver_hware;	//硬件版本信息
	lv_obj_t* imei_label;
	char imei_id[16];	//imei号
	char* ip_addr;	
	char* version;	
	char hardware_ver[18];
	char imei_ver[32];
	char sn[ZB_PROD_SN_LEN+1];	//16位
	lv_draw_rect_dsc_t info_rect;	//信息框
	lv_draw_rect_dsc_t reset_rect;  //恢复出厂
	lv_obj_t * reset_btn;
	lv_obj_t * reset_btn_label;

	lv_obj_t * notice_cont;  //恢复出厂背景面板	
	lv_obj_t * notice_win;  //恢复出厂设置面板
	lv_obj_t * notice_title_labe;  //说明标题
	lv_obj_t * notice_content_labe;  //说明内容
	
	lv_obj_t * enter_btn;  //取消
	lv_obj_t * esc_btn;  //取消
} lv_my_device_set_t;



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*MY_DEVICE_H*/
