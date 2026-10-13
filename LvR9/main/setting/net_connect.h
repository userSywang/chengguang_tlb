#ifndef _NET_CONNECT_H
#define _NET_CONNECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>

#include "../src/wifi/wifi_connect.h"

/*********************
 *      DEFINES
 *********************/

typedef enum {
	NET_STATE_NOSTART,      //未开始
	NET_STATE_CONNECTING,    // 连接中
	NET_STATE_INSERACH,     // 搜索中
	NET_STATE_NOTFOUND,      //未发现
	NET_STATE_CONNECTED,     // 已连接
	NET_STATE_FAILED,         // 连接失败
	NET_STATE_ILE,              //不需要的时候
	NET_STATE_SCANNING,     // 扫描中（新增）
	NET_STATE_SHOWING_LIST, // 显示列表（新增）
} network_state_t;

//-------------GUI层-----------------------

typedef struct {
	lv_obj_t * cont;
	lv_obj_t* little_cont;
	lv_obj_t * title_label;
	//---------
	lv_obj_t * waitScrBg;
	lv_timer_t * tmrWaitting;   // timer for waitingScr 
	int64_t   waitScrTimeSt;    // [请稍侯]版面的起始时刻 
	int8_t    doesScanStart;

	lv_obj_t* status_label; //状态提示词
	lv_obj_t * wifi_sw_btn;		//wifi 开关背景框
	lv_obj_t* cellular_sw_btn;		//蜂窝数据 开关背景框
	lv_obj_t * wifi_sw_obj;		//wifi 开关 obj
	lv_obj_t* cellular_sw_obj;		//蜂窝数据 开关 obj
	lv_timer_t * timer;  		//搜索timer
	int 		times;
	lv_obj_t * ms_box;			//消息框
	uint16_t  wifi_sw_state;  //wifi 开关状态
	
	lv_obj_t * list;  			//无线路由器列表
	uint8_t  list_item_num;  	//数量
	uint8_t  list_sel;			//当前选中的条目
	

	lv_obj_t * psw_cont;		//密码窗口
	lv_obj_t * psw_lab_bg;		//密码标签框
	lv_obj_t * connect_btn;		//连接按钮
	lv_obj_t * keyboard;		//键盘
	
	lv_obj_t* title_cont; //标题容器

	lv_obj_t* my_network_cont;    // 我的网络容器
	lv_obj_t* other_network_label; // "其他网络"标签
	lv_obj_t* my_network_label; //我的网络标签
	lv_obj_t* my_network_obj;

	char* cur_wf_ssid; //当前选择的wifi
	char cur_wf_password[64];			//password
	char cur_wf_security[64];

	lv_obj_t* refresh_btn; 

	network_state_t current_state; //当前网络状态
} network_connect_set_t;

/** 网络配置版面初始化 
 */
extern void network_setting_entry(void);

/** 网络配置版面销毁 
 */
extern void network_setting_exit(void);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
