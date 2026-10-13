//========================================================[]
//Name: app_update.h
//Function: update program and data.
//Author: Xie Zhenzong 
//Date: 2024-05-13
//========================================================[]
#ifndef  _APP_UPDATE_
#define  _APP_UPDATE_

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl/lvgl.h>
#include "../utils/global_parm.h"


typedef enum {
	UPD_STATE_IDLE,         // 空闲状态
	UPD_STATE_AVAILABLE,    // 更新可用
	UPD_STATE_DOWNLOADING,  // 下载中
	UPD_STATE_PAUSED,       // 已暂停
	UPD_STATE_COMPLETED     // 下载完成
} update_state_t;

typedef struct _st_up_set {
	lv_obj_t * cont;
	lv_obj_t * main_bg;
	lv_obj_t * title_label;
	lv_obj_t * ver_label;	
	lv_obj_t* tip_label;

	uint64_t total_space;
	uint64_t used_space;
	uint32_t file_total_size;	//总大小
	uint32_t process_size;  	//已升级的大小
	

}app_update_set_t;

typedef struct {

	lv_obj_t* cont;        // 主容器
	lv_obj_t* btn_back;    // 返回按钮
	lv_obj_t* lbl_title;   // 标题
	lv_obj_t* header;
	lv_obj_t* hint_text;    //提示文本

	// 版本信息
	lv_obj_t* lbl_current; // 当前版本
	lv_obj_t* lbl_available; // 可用版本

	// 进度系统
	lv_obj_t* progress_bar; // 进度条
	lv_obj_t* lbl_progress; // 进度文本
	lv_obj_t* release_notes; // 更新说明文本框
	// 状态管理
	update_state_t curr_state;

	int m_update_progress;
	int m_update_tmp_pro;

	char new_version[128];
	char update_info[4096];

} app_update_ui_t;


// 触发状态转换的事件
typedef enum {
	EVENT_CHECK_NONE,     // 未开始下载
	EVENT_DOWNLOAD_ING,// 下载中
	EVENT_DOWNLOAD_COMPLETE,// 下载完成
	EVENT_DOWNLOAD_INSTALL, //正在安装
	EVENT_DOWNLOAD_ERROR,
} FirmwareUpdateEvent;


typedef struct {
	int retry_count;
	bool check_complete;
	int final_result;
	int final_check;
	char final_version[128];
	char final_info[4096];
} ota_check_context_t;



#ifdef __cplusplus
}
#endif

#endif //_APP_UPDATE_
