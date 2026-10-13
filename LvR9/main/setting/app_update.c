
/********************** 
     INCLUDES
 *********************/
#include <string.h>
#include <math.h>

#include <lvgl/lvgl.h>
 
#include "../utils/global_parm.h"
#include "../utils/misc_utils.h"
#include "app_update.h"
#include "../ex_widget/app_msg.h"
#include "./app_setting_base.h"
#include "./app_update_by_file.h"
#include "../include/img_a_tlb.h"
#include "../data_parse/img_res.h"

#include "../LvR9/trd/ubus/include/cJSON.h"
#include "../mainmenu/mainmenu_base.h"
#ifndef C_PC_SIM
#include <pthread.h>

#include "LvR9//trd/tz_sys_lib/include/cvi_tz_sys.h"
#endif


/*********************
 *      DEFINES
 *********************/

static const char * TAG = "apUpd";

/**********************
 *  STATIC VARIABLES
 **********************/

static app_update_set_t * up_set = NULL;

static FirmwareUpdateEvent UPDATE_STAT;
#ifndef C_PC_SIM
static pthread_t thread_device_update;
static volatile bool is_thread_running = false; // 线程运行状态标志
static pthread_t check_thread;
#endif
static ota_check_context_t g_ota_ctx;
static volatile bool thread_running = false;
static app_update_ui_t ui;
//----------- 主版面 ---------------//

#define HEADER_H 60
#define PROGRESS_H 25
#define CONTENT_PAD 20

#define UP_TITLE_BG_W		SCREEN_WIDTH
#define UP_TITLE_BG_H		60

#define	UP_TITLE_STR	"系统升级"

#define UP_TITLE_W		80
#define UP_TITLE_H		30
#define UP_TITLE_X		11
#define UP_TITLE_Y		15

#define MAIN_BG_W		SCREEN_WIDTH
#define MAIN_BG_H		(SCREEN_HEIGHT - UP_TITLE_BG_H + 40)
#define MAIN_BG_X		0
#define MAIN_BG_Y		42

#define LATEST_VER_TXT_STR			"当前已是最新版本!"
#define NET_DISCONNECTED_TXT_STR	"请连接网络后再试!"


#define NOTICE_TXT_COLOR	0x0010F0	//字体色

#define UPDATE_MAIN_BG_COLOR	0xFFFFFF  //主背景色
	
#define TITLE_BG_COLOR	0xFFB3D7  //主标题背景色
#define TITLE_COLOR		0xFFFFFF  		//标题字体色


 #define DATE_TIME __DATE__ " " __TIME__


//====================================================[]
//   fuctions declare
//====================================================[]
static void setting_update_init(void);
static void update_menu_create(void);
static void update_menu_del(void);
static void init_styles(void);
static void create_new_version_scr(const char* version, const char* info);
static void async_update_ui_cb(void* arg);
static void sysUpd_scr_del_cb(lv_event_t* evt);

//=========================================================

/**
 * @brief 设置固件更新状态并打印状态日志
 * @param event 新的固件更新状态
 */
void setFirmwareUpdateState(FirmwareUpdateEvent event)
{
	if (event == UPDATE_STAT) {
		return; 
	}

	// 将枚举值转换为可读字符串
	const char* state_desc = "未知状态";
	switch (event) {
	case EVENT_CHECK_NONE:
		state_desc = "未开始下载";
		break;
	case EVENT_DOWNLOAD_ING:
		state_desc = "下载中";
		break;
	case EVENT_DOWNLOAD_COMPLETE:
		state_desc = "下载完成";
		break;
	case EVENT_DOWNLOAD_ERROR:
		state_desc = "下载错误";
		break;
	default:
		return;
	}

    UPDATE_STAT = event;

    LV_LOG_USER("固件更新状态变更 → %s", state_desc);
	
}




#ifndef C_PC_SIM
typedef struct thread_ota_updata_data
{
	// ui_wifi_callback_t callback;
	void (*cb)(int ntype, int progress, int version_updatable, const char* new_version,
		const char* new_version_description);
	int ntype;
} thread_ota_updata_data_t;

typedef struct ota_updata_status
{
	int ntype;
	int progress;
	bool version_updatable;
	char* new_version;
	char* new_version_description;
};

// 这个函数将会在 GUI 线程中异步执行
static void async_update_ui_to_call(void* arg)
{
	struct ota_updata_status* data = (struct ota_updata_status*)arg;
	/* 使用data中的数据 */
	LOGD("Received ntype=%d, progress=%d version_updatable=%d \n", data->ntype, data->progress,
		data->version_updatable);

	if (data->ntype == 0) //更新
	{

	}
	else {
		if (data->progress == -1) {
			lv_label_set_text_fmt(ui.lbl_progress, "校验失败，请重新下载");
		}
		else {
		//	struct firmware_update_status status;
			if (data->progress < 100) {
				lv_bar_set_value(ui.progress_bar, data->progress, 0);
				lv_label_set_text_fmt(ui.lbl_progress, "正在安装%d%%", data->progress);
			}
			else {
				lv_bar_set_value(ui.progress_bar,100, 0);
				lv_label_set_text_fmt(ui.lbl_progress, "安装完成,点击重启");
			}
		}
	}

	free(data);
}

// 异步回调的实现
void device_update_sync_callback(int ntype, int progress, int version_updatable, const char* new_version,
	const char* new_version_description)
{
	LOGD("#### ntype:%d,progress;%d new_version:%s new_version_description:%s\n", ntype, progress, new_version,
		new_version_description);

	struct ota_updata_status* data = malloc(sizeof(struct ota_updata_status));
	data->ntype = ntype;
	data->progress = progress;
	if (new_version == NULL) {
		data->new_version = new_version;
	}
	else {
		LOGD("enter\n");
		data->new_version = strdup(new_version);
	}
	if (new_version_description == NULL) {
		data->new_version_description = new_version_description;
	}
	else {
		LOGD("enter\n");
		data->new_version_description = strdup(new_version_description);
	}

	data->version_updatable = version_updatable;

	LOGD("#### ntype:%d,progress;%d new_version:%s new_version_description:%s\n", ntype, progress, new_version,
		data->new_version_description);
	lv_async_call(async_update_ui_to_call, data);
}


//ota升级主要调用
void* recv_device_update_hander(void* arg)
{
	struct thread_ota_updata_data* event_data = (struct thread_ota_updata_data*)arg;
	int ntype = event_data->ntype;
	int dlstatus = -1;
	int ret;

	while (true) {
		//主动获取进度信息
		ret = tz_parse_ota_progress(&ui.m_update_progress);
		ret = tz_parse_ota_dlstatus(&dlstatus);
		LOGD("m_progress=%d,dlstatus=%d\n", ui.m_update_progress, dlstatus);
		if (dlstatus == 1 && ui.m_update_progress == 100) {
			event_data->cb(1, 100, 0, NULL, NULL);
			setFirmwareUpdateState(EVENT_DOWNLOAD_COMPLETE);
			return NULL;
		}
		if (ui.m_update_progress >= 0 && ui.m_update_progress >= ui.m_update_tmp_pro && ui.m_update_progress <= 100) {
			ui.m_update_tmp_pro = ui.m_update_progress;
			event_data->cb(1, ui.m_update_progress, 0, NULL, NULL);

			setFirmwareUpdateState(EVENT_DOWNLOAD_ING);
			if (ui.m_update_progress >= 100)
			{
				setFirmwareUpdateState(EVENT_DOWNLOAD_COMPLETE);
				
				return NULL;
			}
		}
		usleep(100*1000);
	}
	return NULL;
}

#endif



char* parse_and_process_json(const char* json_str) {
	char* processed_text = NULL;

	cJSON* root = cJSON_Parse(json_str);
	if (!root) {
		LV_LOG_ERROR("JSON parse error");
		goto cleanup;
	}

	cJSON* item = cJSON_GetArrayItem(root, 0);
	if (!item) {
		LV_LOG_ERROR("Failed to get first item");
		goto cleanup;
	}

	cJSON* content = cJSON_GetObjectItem(item, "content");
	if (!cJSON_IsString(content)) {
		LV_LOG_ERROR("Content is not a string");
		goto cleanup;
	}

	processed_text = (char*)malloc(strlen(content->valuestring) + 1);
	if (!processed_text) {
		LV_LOG_ERROR("Memory allocation failed");
		goto cleanup;
	}

	char* src = content->valuestring;
	char* dst = processed_text;

	while (*src) {
		if (*src == '\\' && *(src + 1) == 'r' && *(src + 2) == '\\' && *(src + 3) == 'n') {
			*dst++ = '\n';
			src += 4;
		}
		else {
			*dst++ = *src++;
		}
	}
	*dst = '\0';

cleanup:
	cJSON_Delete(root);
	return processed_text;
}



static void sdcard_space_get(void)
{

#ifndef C_PC_SIM
		int ret = sys_get_sdcard_capacity(&up_set->total_space, &up_set->used_space);
		// 返回的单位是KB.
#else
		up_set->total_space = 8*1024*1024;
		up_set->used_space =  5.21*1024*1024;

#endif
		LV_LOG_USER("total_space: %lld, used_space: %lld\r\n ",up_set->total_space, up_set->used_space);

}







static void* ota_check_thread(void* arg) {
	ota_check_context_t* ctx = (ota_check_context_t*)arg;
	int retry = 0;
	int result = -1;
	int check = -1;

	

	LV_LOG_USER("enter ota_check_thread A\n");
	char new_version[128] = { 0 };
	char update_info[4096] = { 0 };

	LV_LOG_USER("enter ota_check_thread B\n");
	sleep(3);
	while (retry < 3 && !ctx->check_complete) {
		result = tz_parse_ota_check(&check, new_version, update_info);
		if (result == 0) {
			ctx->final_result = result;
			ctx->final_check = check;
			strncpy(ctx->final_version, new_version, sizeof(ctx->final_version) - 1);
			strncpy(ctx->final_info, update_info, sizeof(ctx->final_info) - 1);
			ctx->check_complete = true;
			LV_LOG_USER("VER:%s,INFO:%s", new_version, update_info);
			LV_LOG_USER("VER:%s,INFO:%s", ctx->final_version, ctx->final_info);
			break;
		}
		retry++;
		sleep(3);
	}
	LV_LOG_USER("enter ota_check_thread C\n");

	if (!ctx->check_complete) {
		ctx->final_result = -1;
	}


	lv_async_call(async_update_ui_cb, ctx);
	LV_LOG_USER("enter ota_check_thread D\n");
	thread_running = false;
	return NULL;
}

static void async_update_ui_cb(void* arg) {
	ota_check_context_t* ctx = (ota_check_context_t*)arg;
	LV_LOG_USER("enter async_update_ui_cb\n");

	if (ctx->final_result == 0) {
		if (ctx->final_check == 1) {
			LV_LOG_USER("enter async_update_ui_cb new version\n");
			create_new_version_scr(ctx->final_version,ctx->final_info);
			return;
		}
		else {
			LV_LOG_USER("enter async_update_ui_cb is new version\n");
			lv_label_set_text(ui.hint_text, LATEST_VER_TXT_STR);

			lv_obj_t* label_ver = lv_label_create(ui.cont);
			lv_obj_set_size(label_ver, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
			lv_label_set_text_fmt(label_ver, "R9_SW_%s", get_sys_version());
			lv_obj_align_to(label_ver, ui.hint_text, LV_ALIGN_OUT_TOP_MID, 0, -5);
			lv_obj_set_style_text_align(label_ver, LV_TEXT_ALIGN_CENTER, 0);
			return;
			
		}
	}
	else {
		LV_LOG_USER("enter async_update_ui_cb error\n");
		// 显示查询失败
		lv_label_set_text(ui.hint_text, LATEST_VER_TXT_STR);

		lv_obj_t* label_ver = lv_label_create(ui.cont);
		lv_obj_set_size(label_ver, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
		lv_label_set_text_fmt(label_ver, "%s", get_sys_version());
		lv_obj_align_to(label_ver, ui.hint_text, LV_ALIGN_OUT_TOP_MID, 0, -5);
		lv_obj_set_style_text_align(label_ver, LV_TEXT_ALIGN_CENTER, 0);
	}
}




static void updata_menu_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
	LV_LOG("%s [%d],code: %d , evt obj: %p,\n", __FUNCTION__,__LINE__, code, obj);
    if(code == LV_EVENT_CLICKED) 
	{

    }
	else if ((code == LV_EVENT_KEY) || (code == LV_EVENT_KEY_STAGE))
	{

		int keyVal = *(int*)lv_event_get_param(e);
		
		int keySub = (keyVal & LV_EX_FLAG_KEY_MASK);
		
//		LV_LOG("%s [%d] keysub: %x, keyVal: %x\n",__FUNCTION__,__LINE__,  keySub, keyVal);
		
		if (keySub != LV_EX_FLAG_KEY_SHORT) 	return;		

		int c = keyVal & 0xff;

		LV_LOG("%s [%d] Key: %d\n", __FUNCTION__,__LINE__,c);

		if ((c == LV_KEY_UP))
		{
				

			
		}
		else if(c == LV_KEY_DOWN)
		{
		


		}			
		else if (c == LV_KEY_ENTER)
		{

		}
		else if(c == LV_KEY_ESC)
		{
			if (UPDATE_STAT == EVENT_CHECK_NONE)
			{
				
				LV_LOG_USER("返回主菜单 \n");
				zb_requestSwitchAp(C_ZB_AP_MAINMENU);
			}

		}		
		else if(c == LV_KEY_LEFT)
		{
			LV_LOG("%s [%d] test \n", __FUNCTION__,__LINE__);					

		}
#if 0
		else if (c == LV_KEY_EX_FUNC1)
		{

		}
#endif		

	}

}

static lv_style_t style_header;
static lv_style_t style_btn;
static lv_style_t style_progress;
static lv_style_t style_progress_bg;   // 背景样式（未完成部分）
static lv_style_t style_progress_indic; // 前景样式（完成部分）

static void progress_bar_event_cb(lv_event_t* e) {
	lv_event_code_t code = lv_event_get_code(e);
	static bool stat = 0;
	if (code == LV_EVENT_CLICKED) {


			LV_LOG_USER("click\r\n");
			LV_LOG_USER("UPDATE_STAT %d", UPDATE_STAT);
#ifndef C_PC_SIM
			if (UPDATE_STAT == EVENT_CHECK_NONE||UPDATE_STAT == EVENT_DOWNLOAD_COMPLETE)
			{
				tz_set_backlight_onoff(1);
				//tz_set_screenoff_time(3000);
				//开启下载进程
				if (!is_ota_process_running()) {

					system("/usr/bin/ota_download_app &");
					LOGD("ota ota_download_app started\n");
				}
				LOGD("ota ota_download_app checkUpdate\n");

				if (!is_thread_running) {
					is_thread_running = true;
					thread_ota_updata_data_t* event_data = (thread_ota_updata_data_t*)malloc(sizeof(thread_ota_updata_data_t));
					event_data->cb = device_update_sync_callback;
					event_data->ntype = 0;
					pthread_create(&thread_device_update, NULL, recv_device_update_hander, event_data);
					pthread_detach(thread_device_update);

				}
				tz_ota_start_download();

			}
			else if (UPDATE_STAT == EVENT_DOWNLOAD_COMPLETE)
			{
				setFirmwareUpdateState(EVENT_DOWNLOAD_INSTALL);
				system("reboot recovery");
			}

			//pthread_cancel 删除线程
#endif
		
	}
}

static void app_update_ui_cleanup(app_update_ui_t* ui) {
	if (ui == NULL) return;

	if (ui->cont) {
		lv_obj_del(ui->cont);    
		ui->cont = NULL;
	}
	ui->progress_bar = NULL;
	ui->lbl_progress = NULL;
	ui->lbl_title = NULL;
}

static void update_return_cb(lv_event_t* e) {

	if (UPDATE_STAT == EVENT_CHECK_NONE||UPDATE_STAT == EVENT_DOWNLOAD_COMPLETE)
	{
		LV_LOG("%s [%d] 返回主菜单 \n", __FUNCTION__, __LINE__);
		//if (up_set->cont != NULL) {
		//	lv_obj_del(up_set->cont);  up_set->cont = NULL;
		//}
	   // app_update_ui_cleanup(&ui);

		zb_requestSwitchAp(C_ZB_AP_MAINMENU);
		lv_indev_keypad_wait_release();
	}
}

static void update_menu_create(void)
{

	init_styles();

	UPDATE_STAT = EVENT_CHECK_NONE;
	// 主容器
	ui.cont = lv_obj_create(lv_scr_act());
#if DARK_MODE
	lv_obj_set_style_bg_color(ui.cont, lv_color_hex(0x000000), 0); // 黑色背景
#else
	lv_obj_set_style_bg_color(ui.cont, lv_color_hex(0xF2F2F2), 0);
#endif
	lv_obj_set_style_pad_all(ui.cont, 0, 0);
	lv_obj_set_style_radius(ui.cont, 0, 0);
	lv_obj_set_style_border_width(ui.cont, 0, 0);
	lv_obj_set_size(ui.cont, LV_PCT(100), LV_PCT(100));
	lv_obj_clear_flag(ui.cont, LV_OBJ_FLAG_SCROLLABLE);

	// 标题栏
	ui.header = lv_obj_create(ui.cont);
	lv_obj_add_style(ui.header, &style_header, 0);
	lv_obj_clear_flag(ui.header, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_size(ui.header, LV_PCT(100), 80);
	lv_obj_set_style_bg_color(ui.header, lv_color_hex(0x000000), 0);

	// 标题
	ui.lbl_title = lv_label_create(ui.header);
	lv_label_set_text(ui.lbl_title, "系统升级");
	lv_obj_align(ui.lbl_title, LV_ALIGN_TOP_MID, 0, 10);

	lv_obj_t* return_btn = lv_btn_create(ui.header);
	lv_obj_set_size(return_btn, 50, 50);
	lv_obj_set_style_radius(return_btn, 17, 0);
	lv_obj_set_style_bg_color(return_btn, lv_color_hex(0xffffff), 0);
	lv_obj_set_style_bg_opa(return_btn, LV_OPA_0, 0);
	lv_obj_set_style_shadow_width(return_btn, 0, 0);
	lv_obj_set_style_border_width(return_btn, 0, 0);
	lv_obj_set_style_outline_width(return_btn, 0, 0);
	lv_img_dsc_t* return_pImgDsc = zb_img_res_load_one(NULL, SETTING_IMG_RETURN, IMG_FREE_TYPE_AUTO, NULL);
	lv_obj_set_style_bg_img_src(return_btn, return_pImgDsc, 0);
	lv_obj_align(return_btn, LV_ALIGN_TOP_LEFT, -5, -5);
	
	lv_obj_add_event_cb(return_btn, update_return_cb, LV_EVENT_CLICKED, NULL);

	sdcard_space_get();//目前是不准的，需要重新对接

#ifndef C_PC_SIM
	if (!tz_check_net_connect())
#else
	if (1)
#endif
	{
#if 0
		ui.hint_text = lv_label_create(ui.cont);
		lv_obj_set_size(ui.hint_text, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
		lv_label_set_text(ui.hint_text, NET_DISCONNECTED_TXT_STR);
		lv_obj_align(ui.hint_text, LV_ALIGN_CENTER, 0, 10);
		lv_obj_set_style_text_align(ui.hint_text, LV_TEXT_ALIGN_CENTER, 0);
		LV_LOG_USER("no connected\n");
#else
		lv_obj_t* mask = create_network_popup(ui.cont,"无网络","检测失败，请连接网络",168);

#if 0
		lv_obj_t* popup = lv_obj_get_child(mask, 0);

		lv_obj_t* btn = create_custom_button(popup, "设置网络", NULL, NULL);
		lv_obj_set_size(btn, 200, 46);
		lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -54);
		lv_obj_add_style(btn, get_global_style(STYLE_BTN_BASE), LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(btn, lv_color_hex(0xFF3333), 0);
		lv_obj_add_event_cb(btn, delAll_event_cb, LV_EVENT_CLICKED, main);
#endif
#endif
	}
	else
	{
		memset(&g_ota_ctx, 0, sizeof(g_ota_ctx));
		g_ota_ctx.retry_count = 3;
		g_ota_ctx.check_complete = false;

		ui.hint_text = lv_label_create(ui.cont);
		lv_obj_set_size(ui.hint_text, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
#if DARK_MODE
		lv_obj_set_style_text_color(ui.hint_text, lv_color_hex(0xFFFFFF), 0); // 白色文本
#endif
		lv_label_set_text(ui.hint_text, "加载中");
		lv_obj_align(ui.hint_text, LV_ALIGN_CENTER, 0, 10);
		lv_obj_set_style_text_align(ui.hint_text, LV_TEXT_ALIGN_CENTER, 0);

#ifndef C_PC_SIM
		LV_LOG_USER("is_ota_process_running\n");
		if (!is_ota_process_running()) {

			system("/usr/bin/ota_download_app &");
			LOGD("ota ota_download_app started\n");
		}


		LV_LOG_USER("create thread\n");
		// 创建检查线程
	
		if (pthread_create(&check_thread, NULL, ota_check_thread, &g_ota_ctx) == 0) {
			thread_running = true;
			pthread_detach(check_thread);
		}
		else {
			lv_obj_t* label = lv_label_create(ui.cont);
			lv_label_set_text(label, "初始化失败");
			lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
		}
#endif
	}


	LV_LOG_USER("update_menu_create D\r\n");

	lv_group_add_obj(lv_group_get_default(), ui.cont);
	lv_group_focus_obj(ui.cont);

	sys_setCurMainWinAndCb(ui.cont, updata_menu_event_handler);
	return;
}



static void update_set_main_constructor(void)
{
	//----------大背景 --------
	LV_LOG("%s [%d]\n", __FUNCTION__,__LINE__);
	up_set->cont = lv_obj_create(lv_scr_act());

	lv_obj_set_size(up_set->cont, SCREEN_WIDTH, SCREEN_HEIGHT); 
	lv_obj_align(up_set->cont, LV_ALIGN_TOP_LEFT, 0, 0);
	
	lv_obj_set_style_pad_all(up_set->cont,0, 0);
	lv_obj_set_style_border_width(up_set->cont, 0, 0);

	lv_obj_set_style_radius(up_set->cont, 0, 0);
	lv_obj_set_style_bg_opa(up_set->cont, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(up_set->cont, lv_color_hex(TITLE_BG_COLOR), 0);

	lv_obj_clear_flag(up_set->cont, LV_OBJ_FLAG_SCROLLABLE);
	
//	lv_obj_add_event_cb(up_set->cont, draw_main, LV_EVENT_DRAW_MAIN, up_set->cont);

// ----- ------- 标题	----------
	up_set->title_label = lv_label_create(up_set->cont);
	lv_obj_set_size(up_set->title_label, UP_TITLE_W, UP_TITLE_H);	
	lv_obj_set_style_pad_all(up_set->title_label,0, 0);
	lv_obj_set_style_border_width(up_set->title_label, 0, 0);

	lv_label_set_text(up_set->title_label, UP_TITLE_STR);
	lv_obj_set_style_text_color(up_set->title_label, lv_color_hex(TITLE_COLOR), 0);
	lv_obj_align(up_set->title_label,  LV_ALIGN_TOP_LEFT, UP_TITLE_X, UP_TITLE_Y);

	LV_LOG("%s [%d]\n", __FUNCTION__,__LINE__);

	// ----- ----- 圆弧背景框	----------
	up_set->main_bg = lv_obj_create(up_set->cont);

	lv_obj_set_size(up_set->main_bg, MAIN_BG_W, MAIN_BG_H); 
	lv_obj_align(up_set->main_bg, LV_ALIGN_TOP_LEFT, MAIN_BG_X, MAIN_BG_Y);
	
	lv_obj_set_style_pad_all(up_set->main_bg,0, 0);
	lv_obj_set_style_border_width(up_set->main_bg, 0, 0);

	lv_obj_set_style_radius(up_set->main_bg, 20, 0);
	lv_obj_set_style_bg_opa(up_set->main_bg, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(up_set->main_bg, lv_color_hex(UPDATE_MAIN_BG_COLOR), 0);

	lv_obj_add_event_cb(up_set->cont, sysUpd_scr_del_cb, LV_EVENT_DELETE, NULL);
	return;
}
//--------------------------------------------------------

static void init_styles(void) {
	// 标题样式
	lv_style_init(&style_header);
#if DARK_MODE
	lv_style_set_bg_color(&style_header, lv_color_hex(0x212121)); // 深灰色背景
	lv_style_set_text_color(&style_header, lv_color_hex(0xFFFFFF)); // 白色文本
#else
	lv_style_set_bg_color(&style_header, lv_color_hex(0xF2F2F2));
	lv_style_set_text_color(&style_header, lv_color_hex(0x18191B));
#endif
	lv_style_set_border_width(&style_header, 0);
	lv_style_set_pad_all(&style_header, 10);

	// 按钮样式
	lv_style_init(&style_btn);
#if DARK_MODE
	lv_style_set_bg_color(&style_btn, lv_color_hex(0x212121)); // 深灰色按钮
	lv_style_set_text_color(&style_btn, lv_color_hex(0xFFFFFF)); // 白色文本
#else
	lv_style_set_bg_color(&style_btn, lv_color_hex(0x4CAF50));
#endif
	lv_style_set_radius(&style_btn, 8);

	// 进度条样式
	lv_style_init(&style_progress);
	lv_style_set_bg_color(&style_progress, lv_color_hex(0xE0E0E0));
	lv_style_set_radius(&style_progress, 12);

	// 背景样式（未完成部分）
	lv_style_init(&style_progress_bg);
	lv_style_set_bg_color(&style_progress_bg, lv_color_hex(0x008CFF));
	lv_style_set_bg_opa(&style_progress_bg, LV_OPA_20); // 20%透明度
	lv_style_set_radius(&style_progress_bg, 10);       // 圆角半径

	// 前景样式（完成部分）
	lv_style_init(&style_progress_indic);
	lv_style_set_bg_color(&style_progress_indic, lv_color_hex(0x008CFF));
	lv_style_set_bg_opa(&style_progress_indic, LV_OPA_COVER); // 100%不透明
	lv_style_set_radius(&style_progress_indic, 10);          // 圆角半径
}

static void create_new_version_scr(const char* version, const char* info)
{

#ifndef C_PC_SIM
	char* result = parse_and_process_json(info);
	LV_LOG_USER("%s\r\n", result);
#else
	const char* json_str =
		"[{\"country\":\"zh_CN\",\"content\":\"1.优化系统.修复错误\\r\\n2.测试OTA下载\\r\\n3.test ota Download\"}]";
	char* result = parse_and_process_json(json_str);
#endif

	LV_LOG_USER("update_menu_create B\r\n");
	lv_obj_t* list = lv_obj_create(ui.cont);
	lv_obj_set_size(list, (SCREEN_WIDTH - 24), 50);
	lv_obj_align(list, LV_ALIGN_TOP_LEFT, 11, 120);

	lv_obj_set_style_bg_color(list, lv_color_hex(0xFFFFFF), 0);
	lv_obj_set_style_bg_opa(list, LV_OPA_COVER, 0);
	lv_obj_set_style_pad_all(list, 0, 0);
	lv_obj_set_style_border_width(list, 0, 0);
	lv_obj_set_style_radius(list, 20, 0);

	lv_obj_set_style_outline_width(list, 0, 0);
	lv_obj_set_style_outline_width(list, 0, LV_STATE_CHECKED);
	lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_align_to(list, ui.header, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

	ui.lbl_current = lv_label_create(list);
	lv_label_set_text(ui.lbl_current, "新版本可用");
	lv_obj_align(ui.lbl_current, LV_ALIGN_LEFT_MID, 20, 0);

	lv_obj_t* ver_cur = lv_label_create(list);
#ifndef C_PC_SIM
	lv_label_set_text_fmt(ver_cur, "R9_SW_%s", version);
#else
	lv_label_set_text(ver_cur, "R9_SW_1.3.5");
#endif
	lv_obj_align(ver_cur, LV_ALIGN_RIGHT_MID, -20, 0);
	lv_obj_set_style_text_color(ver_cur, lv_color_hex(0x008CFF), 0);
	LV_LOG_USER("update_menu_create C\r\n");

	// 进度系统
	ui.progress_bar = lv_bar_create(ui.cont);
	lv_obj_set_size(ui.progress_bar, LV_PCT(80), 55);
	lv_obj_align(ui.progress_bar, LV_ALIGN_BOTTOM_MID, 0, -20);
	lv_bar_set_range(ui.progress_bar, 0, 100);
	lv_obj_add_style(ui.progress_bar, &style_progress_bg, LV_PART_MAIN);
	lv_obj_add_style(ui.progress_bar, &style_progress_indic, LV_PART_INDICATOR);
	lv_obj_add_flag(ui.progress_bar, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(ui.progress_bar, progress_bar_event_cb, LV_EVENT_CLICKED, NULL);
	lv_bar_set_value(ui.progress_bar, 100, 0);

	ui.lbl_progress = lv_label_create(ui.progress_bar);
	lv_label_set_text(ui.lbl_progress, "点击下载");
#if DARK_MODE
	lv_obj_set_style_text_color(ui.lbl_progress, lv_color_hex(0xFFFFFF), 0); // 白色文本
#else
	lv_obj_set_style_text_color(ui.lbl_progress, lv_color_hex(0xFFFFFF), 0);
#endif
	lv_obj_align(ui.lbl_progress, LV_ALIGN_CENTER, 0, 0);

	// 更新说明
	ui.release_notes = lv_obj_create(ui.cont);
	lv_obj_set_size(ui.release_notes, LV_PCT(85), 160);
#if DARK_MODE
	lv_obj_set_style_bg_color(ui.release_notes, lv_color_hex(0x212121), 0); // 深灰色背景
#else
	lv_obj_set_style_bg_color(ui.release_notes, lv_color_hex(0xF2F2F2), 0);
#endif
	lv_obj_align(ui.release_notes, LV_ALIGN_CENTER, 0, 30);
	lv_obj_set_style_border_width(ui.release_notes, 0, 0);
	lv_obj_add_flag(ui.release_notes, LV_OBJ_FLAG_SCROLLABLE);

	lv_obj_t* title = lv_label_create(ui.release_notes);
#if DARK_MODE
	lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0); // 白色文本
#else
	lv_obj_set_style_text_color(title, lv_color_hex(0x666666), 0);
#endif
	lv_obj_set_size(title, LV_PCT(90), LV_PCT(90));
	lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_LEFT, 0);
	lv_obj_set_height(title, LV_SIZE_CONTENT);
	LV_LOG_USER("update_menu_create D\r\n");

	if (result != NULL)
	{
		lv_label_set_text_fmt(title, "更新内容说明：\r\n\n%s", result);
    }

	lv_obj_set_style_text_color(title, lv_color_hex(0x666666), 0);
	LV_LOG_USER("update_menu_create E\r\n");
}



static void sysUpd_scr_del_cb(lv_event_t* evt)  // 
{
	lv_event_code_t code = lv_event_get_code(evt);
	up_set->title_label = NULL;
	up_set->main_bg = NULL;
	return;
}



static void setting_update_init(void)
{
	if (up_set != NULL)	return;
	LV_LOG("%s,[%d] \r\n ", __FUNCTION__, __LINE__);

	up_set = lv_mem_alloc(sizeof(app_update_set_t));
	memset(up_set, 0, sizeof(app_update_set_t));
	return;
}


void setting_update_del(void)
{
	LV_LOG("%s [%d]\n", __FUNCTION__,__LINE__);
	
	if (up_set->cont != NULL) {
		lv_obj_del(up_set->cont);  up_set->cont = NULL;
	}
	app_update_ui_cleanup(&ui);
}

void setting_update_entry(vrelease_notesoid)
{
	LV_LOG("%s [%d] \n", __FUNCTION__,__LINE__);

	setting_update_init();

	update_set_main_constructor();
	update_menu_create();	
	return;
}
