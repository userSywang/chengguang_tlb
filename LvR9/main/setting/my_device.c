//========================================================[]
//Name: my_device.c
//Function: device information browse,and rest all settings, erase all data.
//Author: Xie Zhenzong 
//Date: 2024-04-08
//========================================================[]
/********************** 
     INCLUDES
 *********************/
#include <string.h>
#include <time.h> 

#include "my_device.h"
#include "../ex_widget/app_msg.h"
#include "../utils/misc_utils.h"
#include "../utils/global_parm.h"
#include "./app_setting_base.h"
#include "../include/img_a_tlb.h"
//#include "./parental_control_pin.h"
#include "../data_parse/img_res.h"
#ifndef C_PC_SIM
#include "../../trd/tz_sys_lib/include/cvi_tz_sys.h"
#include "../../trd/ubus/include/tz_hal_quectel.h"

#endif


extern bool zb_is_lcdDontAutoOffMode(void);


/*********************
 *      DEFINES
 *********************/
#if LV_USE_FS_WIN32

#else

#endif

#define BAIDUWEBDISK_SD_MAIN_DIR_PATH   "/userdata/Baidu_Netdisk"   //百度网盘在SD卡中的主目录路径 

/**********************
 *      TYPEDEFS
 **********************/




/**********************
 *  STATIC PROTOTYPES
 **********************/


/**********************
 *  STATIC VARIABLES
 **********************/

static lv_my_device_set_t * my_dev;

/**********************
 *      MACROS
 **********************/

static const char * sys_data_path = "/userdata/_data/";

//----------- 主版面 ---------------//

#define MYDEV_TITLE_BG_W		SCREEN_WIDTH
#define MYDEV_TITLE_BG_H		60

#define	MYDEV_TITLE_STR			"我的设备"
#define	RESET_SETTING_STR		"恢复出厂设置"
#define	NEW_VERSION_STR			"最新版本"
#define	DEL_FELE_NOTICE_STR		"将会删除此设备上的本地缓存及设置文件"

#define MYDEV_TITLE_W		200
#define MYDEV_TITLE_H		30
#define MYDEV_TITLE_Y		20

#define MYDEV_MAIN_BG_W		SCREEN_WIDTH
#define MYDEV_MAIN_BG_H		(SCREEN_HEIGHT - MYDEV_TITLE_H + 40)
#define MYDEV_MAIN_BG_X		0
#define MYDEV_MAIN_BG_Y		42

#define MYDEV_INFO_AREA_W		(220) //第一个框 
#define MYDEV_INFO_AREA_H		60
#define MYDEV_INFO_AREA_X		(11)
#define MYDEV_INFO_AREA_Y		(65)

#define MYDEV_IP_W		(MYDEV_INFO_AREA_W)
#define MYDEV_IP_H		(26)
#define MYDEV_IP_X		(28)
#define MYDEV_IP_Y		(0)

#define MYDEV_VER_W		(MYDEV_INFO_AREA_W)
#define MYDEV_VER_H		(26)
#define MYDEV_VER_X		(28)
#define MYDEV_VER_Y		(MYDEV_IP_Y+206)

#define SN_AREA_W		(220)  //第二个框 SN号
#define SN_AREA_H		50
#define SN_AREA_X		(11)
#define SN_AREA_Y		(MYDEV_INFO_AREA_H + MYDEV_INFO_AREA_Y + 12)


#define SN_label_W		(SN_AREA_W)  // SN号 标签
#define SN_label_H		24
#define SN_label_X		(28)
#define SN_label_Y		(150)


#define RESET_BTN_W		(220)  //第三个框 恢复出厂设置 btn 矩形
#define RESET_BTN_H		50


#define MYDEV_MAIN_BG_COLOR		0xF0FCE0 //0xF7FCED  //主背景色
	
#define MYDEV_TITLE_BG_COLOR	0xAEE34B  //标题背景色

#define MYDEV_SEL_COLOR			0x008CFF //0xAEE34B  	//矩形 选中框颜色

#define MYDEV_RESET_BTN_COLOR	 0xFFFFFF   	//恢复出厂设置按钮颜色 

#define ESC_BTN_COLOR	0xFF5858  	// 取消按钮颜色 
#define ENTER_BTN_COLOR	0xFF3333  	// 确定按钮颜色 

//-----------------------------
// 子窗口
#define NOTICE_W		208 	// 我的设备说明版面弹出框大小
#define NOTICE_H		206
#define NOTICE_X		16
#define NOTICE_Y		69

#define NOTICE_TITLE_W		88 	// 我的设备
#define NOTICE_TITLE_H		30
#define NOTICE_TITLE_X		(NOTICE_W - NOTICE_TITLE_W)/2
#define NOTICE_TITLE_Y		15

#define NOTICE_CONT_W		(NOTICE_W - 20*2)	// 提示内容
#define NOTICE_CONT_H		60
#define NOTICE_CONT_X		24
#define NOTICE_CONT_Y		64


#define ESC_BTN_W		70
#define ESC_BTN_H		30
#define ESC_BTN_X		31
#define ESC_BTN_Y		146

#define ENTER_BTN_W		200
#define ENTER_BTN_H		46
#define ENTER_BTN_X		0
#define ENTER_BTN_Y		20

#define NOTICE_LINE_W	NOTICE_W
#define NOTICE_LINE_H	2
#define NOTICE_LINE_X	16
#define NOTICE_LINE_Y	115

#define NOTICE_LINE_COLOR	0xE7E7E7 //画线颜色 

#if DARK_MODE
#define DARK_BG_COLOR        0x000000  // 纯黑背景
#define DARK_TEXT_COLOR      lv_color_white() // 白色文字
#define DARK_BUTTON_BG_COLOR 0x212121  // 按钮背景色
#define DARK_BORDER_COLOR    0x424242  // 边框颜色
#else
#define DARK_BG_COLOR        STNG_AP_BG_COLOR_HEX  // 使用原来的背景色
#define DARK_TEXT_COLOR      lv_color_black() // 黑色文字
#define DARK_BUTTON_BG_COLOR 0xFFFFFF  // 白色按钮背景
#define DARK_BORDER_COLOR    0xCCCCCC  // 边框颜色
#endif


/**********************
 *   extra  FUNCTIONS
 **********************/
char * get_esp32s3_wifi_get_ip(void);
int esp32s3_get_sdcard_capacity( uint64_t *total_space , uint64_t *free_space);
char * get_tlb_cur_version(void);
int reset_tlb_data_set(void);
int remove_global_parm_data(void);

//==================================================================
void setting_my_device_entry(void);
void setting_my_device_del(void);


//==================================================================
static const char * TAG = "myDev";

 //获取系统版本
char * get_sys_version(void)
{
	static char g_acVersion[20];
	g_acVersion[0] = '\0';
#ifndef C_PC_SIM
	tz_system_info_t  stSysInf = { 0 };
	tz_get_system_info(&stSysInf);
	strlcpy(g_acVersion, stSysInf.version, sizeof(g_acVersion));
	return g_acVersion;
#else
	 return "1.01"; // todo
#endif
}

// 返回 I P
char* get_sys_ip_addr(void)
{
	return ("192.168.1.16");  // todo
}
 


//=====================================================
//删除存放临时数据文件夹。
int reset_tlb_data_set(void)
{
	LV_LOG("%s,[%d] \r\n ", __FUNCTION__,__LINE__);
	//remove_global_parm_data();	
	zb_delete_directory(sys_data_path);
	zb_delete_directory(BAIDUWEBDISK_SD_MAIN_DIR_PATH);
	zb_mkdir(sys_data_path, 0777);
	return(1);
}


/**********************
 *   STATIC  FUNCTIONS
 **********************/

static void my_device_init(void)
{

	int i = 0;
	
	LV_LOG("%s,[%d] \r\n ", __FUNCTION__,__LINE__);
	
	if( my_dev != NULL)	return;
	
	my_dev = lv_mem_alloc(sizeof(lv_my_device_set_t));
	memset(my_dev, 0, sizeof(lv_my_device_set_t));
	
	my_dev->ip_addr = get_sys_ip_addr();	
	my_dev->version = get_sys_version();

	strncpy(my_dev->imei_id, get_imei_id(), sizeof(my_dev->imei_id) - 1);
	my_dev->imei_id[sizeof(my_dev->imei_id) - 1] = '\0'; 
	LV_LOG_USER("Stored IMEI: %s", my_dev->imei_id);

	int stat = zb_readProductSerialNo(&my_dev->sn, ZB_PROD_SN_LEN+1);  

	LV_LOG_USER("zb_readProductSerialNo stat %d", zb_readProductSerialNo);
	return;
}

static void my_dev_del_cb(lv_event_t* evt)
{ 
	// lv_obj_t * obj = lv_event_get_target(evt);
	lv_event_code_t evtCode = evt->code;
	if (evtCode == LV_EVENT_DELETE) {
		my_dev->cont = NULL;
	}
}

static void notice_win_del(void)
{
	LV_LOG("%s [%d] \n", __FUNCTION__,__LINE__);
	
	if (my_dev->notice_cont != NULL)
	{
		lv_obj_del(my_dev->notice_cont);	   
	    my_dev->notice_cont = NULL;
	}
	lv_group_focus_obj(my_dev->reset_btn);
}
//=======================================================//
// 提示消息延时退出回调
static msgbox_exit_cb_t reset_data_exit_cb(msgbox_exit_param_t * pdata)
{
	LV_LOG("%s [%d],pdata: %p \n", __FUNCTION__,__LINE__, pdata);
	
	reset_tlb_data_set();  //清除设置数据;
	notice_win_del();
#ifndef	C_PC_SIM	
	ESP_LOGW(TAG, "should reset here.");
	//todo, esp_restart();   //重启机器
#endif
	return NULL;
}

//=======================================================[]
static void notice_win_btn_handler(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* f_obj = lv_event_get_target(e);
	char c = *((char*)lv_event_get_param(e));
	
	LV_LOG("%s [%d] code : %d, f_obj: %p\n", __FUNCTION__,__LINE__, code, f_obj);

	if (code == LV_EVENT_CLICKED) 
	{
		LV_LOG_USER("Clicked\n");

		if(f_obj == my_dev->enter_btn) 
		{
			if (my_dev->notice_cont)
			{
				lv_obj_del(my_dev->notice_cont);
				my_dev->notice_cont = NULL;
		    }
			//清除数据 
#ifndef C_PC_SIM
			tz_factory_reset(0);
#endif
			reset_tlb_data_set();  //清除设置数据;
			//app_msgbox_timeout("","清除数据中...\n稍后重启机器",1000, reset_data_exit_cb, (msgbox_exit_param_t*)f_obj);				
		
		}
		else if(f_obj == my_dev->esc_btn)
		{
			notice_win_del();			
		}
	 
	}
	else if (code == LV_EVENT_VALUE_CHANGED) 
	{
		LV_LOG_USER("Toggled\n");
	}
#if 1
	else if ((code == LV_EVENT_KEY) || (code == LV_EVENT_KEY_STAGE))
	{

			int keyVal = *(int*)lv_event_get_param(e);
			
			int keySub = (keyVal & LV_EX_FLAG_KEY_MASK);
			
			LV_LOG("%s [%d] keysub: %x, keyVal: %x\n",__FUNCTION__,__LINE__,  keySub,keyVal);
			
			if (keySub != LV_EX_FLAG_KEY_SHORT) return;
		
		//	char c = *((char*)lv_event_get_param(e));
		
			int c = keyVal & 0xff;

		LV_LOG("LV_EVENT_KEY: %d\n", c);
        if (c == LV_KEY_ENTER)
		{
			if(f_obj == my_dev->enter_btn) 
			{
				//清除数据 
				app_msgbox_timeout("","清除数据中...\n稍后重启机器\n",1000, reset_data_exit_cb, (msgbox_exit_param_t*)f_obj); 			
						
			}
			else if(f_obj == my_dev->esc_btn)
			{
				notice_win_del();	
			}
						
			
		}
		else if(c == LV_KEY_ESC)
		{
			notice_win_del();	
		}		
		lv_indev_keypad_wait_release();

	}
#endif


}

static void bg_click_cb(lv_event_t* e) {  
	lv_obj_t* bgObj = lv_event_get_target(e);
	lv_obj_t* rect = (lv_obj_t*)lv_obj_get_user_data(bgObj);

	lv_point_t click_pos;
	lv_indev_get_point(lv_indev_get_act(), &click_pos);

	lv_point_t pos_in_rect;
	lv_obj_t* parent = lv_obj_get_parent(rect);
	lv_area_t rect_coords;
	lv_obj_get_coords(rect, &rect_coords);

	pos_in_rect.x = click_pos.x - rect_coords.x1;
	pos_in_rect.y = click_pos.y - rect_coords.y1;


	if (!_lv_area_is_point_on(&rect_coords, &click_pos, 0)) {
		//incoming_call_scr_del();
		//ready_call_scr_del();
		if (my_dev->notice_cont)
		{
			lv_obj_del(my_dev->notice_cont);
			my_dev->notice_cont = NULL;
		}
	}
}

static void del_event_cb(lv_event_t* e)
{
	if (my_dev->notice_cont)
	{
		lv_obj_del(my_dev->notice_cont);
		my_dev->notice_cont = NULL;
	}
}



static lv_style_t style_clickBtn;
static void notice_win_open(void)
{
//----------大背景 --------
	lv_obj_t * title_labe;  //说明标题
	lv_obj_t * content_labe;  //说明内容
	LV_LOG("%s [%d]\n", __FUNCTION__,__LINE__);

	if(my_dev->notice_cont != NULL)	return; // key down 事件后，还会有个 LV_EVENT_CLICKED
	
	//my_dev->notice_cont = lv_obj_create(my_dev->cont);

	my_dev->notice_cont = lv_obj_create(my_dev->cont);
	//lv_obj_t* my_dev->notice_cont = lv_obj_create(lv_layer_top());  //不一定要顶层，因为还需要通话弹窗  这个可以直接在联系人页面生成
	//my_dev->notice_cont->flags |= LV_OBJ_FLAG_OUT_OF_DEF_GROUP;
	lv_obj_set_size(my_dev->notice_cont, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_scroll_dir(my_dev->notice_cont, LV_DIR_NONE);
	lv_obj_set_style_pad_all(my_dev->notice_cont, 0, 0);
	lv_obj_set_style_radius(my_dev->notice_cont, 0, 0);
	lv_obj_set_style_border_width(my_dev->notice_cont, 0, 0);
	lv_obj_set_style_bg_color(my_dev->notice_cont, lv_color_hex(DARK_BG_COLOR), 0);

	// Make the background clickable
	lv_obj_add_flag(my_dev->notice_cont, LV_OBJ_FLAG_CLICKABLE);


	static lv_style_t style;
	lv_style_init(&style);


	lv_style_set_bg_opa(&style, LV_OPA_70);
	lv_obj_add_style(my_dev->notice_cont, &style, 0);

	lv_style_init(&style_clickBtn);
	custom_style_set_img_recolor(&style_clickBtn);

	lv_obj_t* rect = lv_obj_create(my_dev->notice_cont);
	lv_obj_set_size(rect, SCREEN_WIDTH, 188);
	lv_obj_align(rect, LV_ALIGN_BOTTOM_MID,0,20);
#if DARK_MODE
	lv_obj_set_style_bg_color(rect, lv_color_hex(DARK_BUTTON_BG_COLOR), LV_PART_MAIN);
#else
	lv_obj_set_style_bg_color(rect, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
#endif
	lv_obj_set_style_radius(rect, 19, LV_PART_MAIN);

	lv_obj_set_user_data(my_dev->notice_cont, rect);

	lv_obj_add_event_cb(my_dev->notice_cont, bg_click_cb, LV_EVENT_CLICKED, NULL);

	lv_obj_t* rect_tip = lv_label_create(rect);
	lv_label_set_text(rect_tip, "清除所有数据");
	lv_obj_set_style_text_color(rect_tip, DARK_TEXT_COLOR, 0);
	lv_obj_align(rect_tip, LV_ALIGN_TOP_LEFT, 20, 20);

	lv_obj_t* rect_labe = lv_label_create(rect);
	lv_label_set_text(rect_labe, "清空所有数据并恢复出厂时的设置");
#if DARK_MODE
	lv_obj_set_style_text_color(rect_labe, lv_color_hex(0x9E9E9E), 0);
#else
	lv_obj_set_style_text_color(rect_labe, lv_color_hex(0x787F87), 0);
#endif
	lv_obj_align(rect_labe, LV_ALIGN_CENTER, 0, -20);

	lv_obj_t* del_img = lv_btn_create(rect);
	lv_obj_set_size(del_img, 50, 50);
	lv_obj_set_style_radius(del_img, 17, 0);


	lv_obj_set_style_bg_opa(del_img, LV_OPA_0, 0);
	lv_obj_set_style_shadow_width(del_img, 0, 0);
	lv_img_dsc_t* col_img = zb_img_res_load_one(NULL, SETTING_IMG_DEL, IMG_FREE_TYPE_AUTO, NULL);
	lv_obj_set_style_bg_img_src(del_img, col_img, 0);
	lv_obj_align(del_img, LV_ALIGN_TOP_RIGHT, -10, 5);
	lv_obj_add_event_cb(del_img, del_event_cb, LV_EVENT_CLICKED, NULL);


	
	// ----- 确定按钮 ---- 
		
	my_dev->enter_btn = lv_btn_create(rect);
	
	lv_obj_set_size(my_dev->enter_btn, ENTER_BTN_W, ENTER_BTN_H);
	lv_obj_align(my_dev->enter_btn, LV_ALIGN_CENTER, ENTER_BTN_X, ENTER_BTN_Y);


	lv_obj_set_style_bg_color(my_dev->enter_btn, lv_color_hex(ENTER_BTN_COLOR), 0);


	lv_obj_set_style_pad_all(my_dev->enter_btn,0, 0);
	lv_obj_set_style_border_width(my_dev->enter_btn, 0, 0);
	lv_obj_set_style_radius(my_dev->enter_btn, 67, 0);
	lv_obj_set_style_bg_opa(my_dev->enter_btn, LV_OPA_COVER, 0);

	lv_obj_set_style_bg_color(my_dev->enter_btn, lv_color_hex(ENTER_BTN_COLOR), LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(my_dev->enter_btn, lv_color_hex(ENTER_BTN_COLOR), LV_STATE_CHECKED);
	lv_obj_set_style_bg_color(my_dev->enter_btn, lv_color_hex(ENTER_BTN_COLOR), LV_STATE_FOCUSED);
	
	lv_obj_t * enter_btn_label = lv_label_create(my_dev->enter_btn);
	lv_label_set_text(enter_btn_label, "确认");
	lv_obj_set_style_text_color(enter_btn_label, lv_color_hex(0xffffff), 0);
	lv_obj_align(enter_btn_label, LV_ALIGN_CENTER, 0, 0);
	
	lv_obj_add_event_cb(my_dev->enter_btn, notice_win_btn_handler, LV_EVENT_KEY_STAGE, my_dev->enter_btn);
	lv_obj_add_event_cb(my_dev->enter_btn, notice_win_btn_handler, LV_EVENT_KEY, my_dev->enter_btn);
	lv_obj_add_event_cb(my_dev->enter_btn, notice_win_btn_handler, LV_EVENT_CLICKED, my_dev->enter_btn);
	

	lv_obj_add_flag(my_dev->enter_btn, LV_OBJ_FLAG_CHECKABLE);
	lv_group_add_obj(lv_group_get_default(), my_dev->enter_btn);  
	
	//lv_obj_add_flag(my_dev->esc_btn, LV_OBJ_FLAG_CHECKABLE);
	//lv_group_add_obj(lv_group_get_default(), my_dev->esc_btn);  
	//
	//lv_group_focus_obj(my_dev->esc_btn);




}


static void reset_btn_handler(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* f_obj = lv_event_get_target(e);
	
	LV_LOG("%s [%d] code : %d\n", __FUNCTION__,__LINE__, code);
	if (code == LV_EVENT_CLICKED) 
	{
		notice_win_open();
	}
	else if ((code == LV_EVENT_KEY) || (code ==LV_EVENT_KEY_STAGE))
	{
		int keyVal = *(int*)lv_event_get_param(e);
		int keySub = (keyVal & LV_EX_FLAG_KEY_MASK);
		if (keySub != LV_EX_FLAG_KEY_SHORT) return;

	//	char c = *((char*)lv_event_get_param(e));
	
		int c = keyVal & 0xff;
	
		LV_LOG("LV_EVENT_KEY: %d\n", c);
		if (c == LV_KEY_RIGHT)
		{
			

		}
		else if (c == LV_KEY_LEFT)
		{

		}
		if (c == LV_KEY_DOWN)
		{
				

		}
		else if (c == LV_KEY_UP)
		{
			
		}		
		else if (c == LV_KEY_ENTER)
		{
			/* if (prp_pwdHash_read() != 0) { // has password, then enter login screen.
				prp_loginScr_init(0);
				my_dev->m_isPwdEdtScr = 1;
			}
			else  */
			{
				notice_win_open();
			}
		}
		else if(c == LV_KEY_ESC)
		{
			lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
		}

		lv_indev_keypad_wait_release();

	}

}


static void _myDev_esc_btn_click_cb(lv_event_t * evt)
{
	lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
	return;
}


#define MYDEV_BDOOR_CLK_N  7

static int g_bdoorClkCnt = 0;
static uint32_t g_bdoorBtnClkTmTbl[MYDEV_BDOOR_CLK_N];

static void _myDev_bdoor_btn_click_cb(lv_event_t * evt)
{
	lv_obj_t* obj = lv_event_get_target(evt);

	if (g_bdoorClkCnt >= MYDEV_BDOOR_CLK_N) {
		memmove(g_bdoorBtnClkTmTbl, g_bdoorBtnClkTmTbl + 1, sizeof(g_bdoorBtnClkTmTbl[0]) * (MYDEV_BDOOR_CLK_N - 1));
		g_bdoorClkCnt = MYDEV_BDOOR_CLK_N - 1;
	}
	uint32_t tmCur = time(NULL);
	g_bdoorBtnClkTmTbl[g_bdoorClkCnt] = tmCur;
	g_bdoorClkCnt++;

	if ((g_bdoorClkCnt >= MYDEV_BDOOR_CLK_N) && (tmCur > g_bdoorBtnClkTmTbl[0]) && (tmCur <= g_bdoorBtnClkTmTbl[0] + 6) ) 
	{
		g_bdoorClkCnt = 0;
	#ifndef C_PC_SIM
		UsbMode mode = 0;
		int getRet = tz_get_otg_mode(&mode);
		if (0 == getRet) {
			app_msgbox_timeout("", "rd 操作失败!", 2500|MSGBOX_TOAST_FLAG, NULL, NULL);
		}
		else {
			UsbMode modNew = (USB_HOST == mode ? USB_PERIPHERAL : USB_HOST);
			ESP_LOGI(TAG, "bfr setOtgMod %d", (int)modNew);
			int setRet = tz_set_otg_mode(modNew);
			if (0 == setRet) {
				app_msgbox_timeout("", "set 操作失败!", 2500|MSGBOX_TOAST_FLAG, NULL, NULL);
			}
			else {
				char acMsg[100];
				snprintf(acMsg, sizeof(acMsg), "已切换为模式 %d", (int)modNew);
				app_msgbox_timeout("", acMsg, 2500|MSGBOX_TOAST_FLAG, NULL, NULL);
			}
		}
	#else
		ESP_LOGW(TAG, "bdoor A~~");
	#endif
	}

	return;
}

#define NETWORK_SWITCH_BG_W		(SCREEN_WIDTH - 24) //第一个框 wifi 开关背景框
#define NETWORK_SWITCH_BG_H		50
#define NETWORK_SWITCH_BG_X		(11)
#define NETWORK_SWITCH_BG_Y		(60)
lv_obj_t* btn_list_create()
{
	lv_obj_t* obj = lv_obj_create(my_dev->cont);
	lv_obj_set_size(obj, NETWORK_SWITCH_BG_W, NETWORK_SWITCH_BG_H);
	lv_obj_align(obj, LV_ALIGN_TOP_LEFT, NETWORK_SWITCH_BG_X, NETWORK_SWITCH_BG_Y + 60);

	lv_obj_set_style_bg_color(obj, lv_color_hex(DARK_BUTTON_BG_COLOR), 0);
	lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
	lv_obj_set_style_pad_all(obj, 0, 0);
	lv_obj_set_style_border_width(obj, 0, 0);
	lv_obj_set_style_radius(obj, 20, 0);

	lv_obj_set_style_outline_width(obj, 0, 0);
	lv_obj_set_style_outline_width(obj, 0, LV_STATE_CHECKED);

	return obj;
}

#define BTN_LIST_WIDTH  (LV_HOR_RES - 20)  
lv_obj_t* create_info_row(const char* title, const char* value, lv_obj_t* prev_row) {

	lv_obj_t* cont = btn_list_create();
	lv_obj_set_size(cont, BTN_LIST_WIDTH, 40); 
	lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(cont, lv_color_hex(DARK_BUTTON_BG_COLOR), 0);

	lv_obj_t* title_label = lv_label_create(cont);
	lv_label_set_text(title_label, title);
	lv_obj_set_style_text_color(title_label, DARK_TEXT_COLOR, 0);
	//lv_obj_set_style_text_font(title_label, &lv_font_montserrat_16, 0);
	lv_obj_align(title_label, LV_ALIGN_LEFT_MID, 20, 0); 


	lv_obj_t* value_label = lv_label_create(cont);
	lv_label_set_text(value_label, value);
#if DARK_MODE
	lv_obj_set_style_text_color(value_label, lv_color_hex(0x9E9E9E), 0);
#else
	lv_obj_set_style_text_color(value_label, lv_color_hex(0xB6BFC9), 0);
#endif
	lv_obj_align(value_label, LV_ALIGN_RIGHT_MID, -20, 0);


	if (prev_row) {
		lv_obj_align_to(cont, prev_row, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	}
	else {
		lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 10, MYDEV_INFO_AREA_Y + 10);
	}

	return value_label; 
}


static void setting_my_device_constructor(void)
{
//----------大背景 --------
	my_dev->cont = lv_obj_create(lv_scr_act());

	lv_obj_set_size(my_dev->cont, SCREEN_WIDTH, SCREEN_HEIGHT); 
	lv_obj_align(my_dev->cont, LV_ALIGN_TOP_LEFT, 0, 0);
	
	lv_obj_set_style_pad_all(my_dev->cont, 0, 0);
	lv_obj_set_style_border_width(my_dev->cont, 0, 0);
	lv_obj_set_style_radius(my_dev->cont, 0, 0);
	lv_obj_clear_flag(my_dev->cont, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(my_dev->cont, lv_color_hex(DARK_BG_COLOR), 0);

	lv_obj_add_event_cb(my_dev->cont, my_dev_del_cb, LV_EVENT_DELETE, NULL);

// ----- ------------ 我的设备标题	----------
	stng_createEscBtn(my_dev->cont, _myDev_esc_btn_click_cb, 0);

	my_dev->title_label = lv_label_create(my_dev->cont);
	//lv_obj_set_size(my_dev->title_label, MYDEV_TITLE_W, MYDEV_TITLE_H);	
	lv_obj_set_style_pad_all(my_dev->title_label,0, 0);
	lv_obj_set_style_border_width(my_dev->title_label, 0, 0);

	lv_label_set_text(my_dev->title_label, MYDEV_TITLE_STR);
	lv_obj_set_style_text_color(my_dev->title_label, DARK_TEXT_COLOR, 0);
	lv_obj_align(my_dev->title_label,  LV_ALIGN_TOP_MID, 0, MYDEV_TITLE_Y);

//------  ip信息 --------	
	my_dev->ip_label = create_info_row("IP 地址:", my_dev->ip_addr, NULL);	

//------  版本信息 --------	
	my_dev->ver_label = create_info_row("软件版本:", my_dev->version, lv_obj_get_parent(my_dev->ip_label));
#ifndef C_PC_SIM
	int stat = tz_get_hw_version(my_dev->hardware_ver);
	my_dev->ver_hware = create_info_row("硬件版本:", my_dev->hardware_ver, lv_obj_get_parent(my_dev->ver_label));
#else
	my_dev->ver_hware = create_info_row("硬件版本:", my_dev->version, lv_obj_get_parent(my_dev->ver_label));
#endif

//------  S N 号 --------	
#ifndef C_PC_SIM
	my_dev->sn_label = create_info_row("SN:",my_dev->sn, lv_obj_get_parent(my_dev->ver_hware));
#else
	my_dev->sn_label = create_info_row("SN:", "TLB07C4F20019190", lv_obj_get_parent(my_dev->ver_hware));
	
#endif

#ifndef C_PC_SIM
	tz_get_imei(my_dev->imei_ver);
	my_dev->imei_label = create_info_row("IMEI:", my_dev->imei_ver, lv_obj_get_parent(my_dev->sn_label));
#else
	my_dev->imei_label = create_info_row("IMEI:", "TLB07C4F20019190", lv_obj_get_parent(my_dev->sn_label));
#endif

	lv_obj_t* device_info = create_info_row("设备型号:", "YDZR9", lv_obj_get_parent(my_dev->imei_label));


	static lv_style_t style_info_row;
	lv_style_init(&style_info_row);


	lv_style_set_bg_color(&style_info_row, lv_color_hex(DARK_BUTTON_BG_COLOR));


	lv_style_set_text_color(&style_info_row, DARK_TEXT_COLOR);


	lv_obj_add_style(lv_obj_get_parent(my_dev->ip_label), &style_info_row, 0);
	lv_obj_add_style(lv_obj_get_parent(my_dev->ver_label), &style_info_row, 0);
	lv_obj_add_style(lv_obj_get_parent(my_dev->ver_hware), &style_info_row, 0);
	lv_obj_add_style(lv_obj_get_parent(my_dev->sn_label), &style_info_row, 0);
	lv_obj_add_style(lv_obj_get_parent(my_dev->imei_label), &style_info_row, 0);
	lv_obj_add_style(lv_obj_get_parent(device_info), &style_info_row, 0);
		
//-----------------------
	memset(g_bdoorBtnClkTmTbl, 0, sizeof(g_bdoorBtnClkTmTbl));
	g_bdoorClkCnt = 0;
	lv_obj_t * objBDoorBtn = lv_btn_create(my_dev->cont);
	lv_obj_remove_style_all(objBDoorBtn);
	lv_obj_set_size(objBDoorBtn, 200, 90);
	lv_obj_align(objBDoorBtn,	LV_ALIGN_TOP_LEFT, (SCREEN_WIDTH-200)/2, 180);
	lv_obj_add_event_cb(objBDoorBtn, _myDev_bdoor_btn_click_cb, LV_EVENT_CLICKED, NULL); // todo
	lv_color_t bgColor = lv_color_hex(0xf0f0f0);
	lv_obj_set_style_border_opa(objBDoorBtn, LV_OPA_0, 0);
	lv_obj_set_style_shadow_width(objBDoorBtn, 0, 0);
	lv_obj_set_style_bg_color(objBDoorBtn, bgColor, 0);
	lv_obj_set_style_bg_color(objBDoorBtn, bgColor, LV_STATE_PRESSED);
//------  恢复出厂设置 --------

	my_dev->reset_btn = lv_obj_create(my_dev->cont);
	
	lv_obj_set_size(my_dev->reset_btn, RESET_BTN_W,RESET_BTN_H);
	lv_obj_set_style_border_width(my_dev->reset_btn, 0, 0);

	lv_obj_set_style_pad_all(my_dev->reset_btn,0, 0);
	lv_obj_set_style_radius(my_dev->reset_btn, 20, 0);
	
	//lv_obj_set_style_bg_color(my_dev->reset_btn, lv_color_hex(DARK_BUTTON_BG_COLOR), 0);

	lv_obj_set_style_bg_opa(my_dev->reset_btn, LV_OPA_COVER, 0);	
	lv_obj_set_style_bg_color(my_dev->reset_btn, lv_color_hex(MYDEV_RESET_BTN_COLOR), 0);
	lv_obj_set_style_bg_color(my_dev->reset_btn, lv_color_hex(MYDEV_RESET_BTN_COLOR), LV_STATE_CHECKED);
	lv_obj_set_style_bg_color(my_dev->reset_btn, lv_color_hex(MYDEV_RESET_BTN_COLOR), LV_STATE_FOCUSED);
	lv_obj_set_style_bg_color(my_dev->reset_btn, lv_color_hex(MYDEV_RESET_BTN_COLOR), LV_STATE_FOCUS_KEY);
	lv_obj_set_style_bg_color(my_dev->reset_btn, lv_color_hex(MYDEV_RESET_BTN_COLOR), LV_STATE_PRESSED);



//	lv_obj_set_style_border_color(my_dev->reset_btn, lv_color_hex(MYDEV_SEL_COLOR), 0);
//	lv_obj_set_style_border_color(my_dev->reset_btn, lv_color_hex(MYDEV_SEL_COLOR), LV_STATE_CHECKED);
//	lv_obj_set_style_border_opa(my_dev->reset_btn, LV_OPA_COVER, 0);

	lv_obj_set_style_outline_color(my_dev->reset_btn, lv_color_hex(MYDEV_SEL_COLOR), 0);
	lv_obj_set_style_outline_opa(my_dev->reset_btn, LV_OPA_COVER, 0);

	lv_obj_set_style_outline_width(my_dev->reset_btn, 3, 0);
	lv_obj_align(my_dev->reset_btn, LV_ALIGN_BOTTOM_MID, 0, -46);
	lv_obj_add_flag(my_dev->reset_btn, LV_OBJ_FLAG_CHECKABLE);

	my_dev->reset_btn_label = lv_label_create(my_dev->reset_btn);
	
//	lv_obj_set_size(my_dev->reset_btn_label, LV_PCT(100),LV_PCT(100));
	
	lv_label_set_text(my_dev->reset_btn_label, RESET_SETTING_STR);
	lv_obj_set_style_text_color(my_dev->reset_btn_label, lv_color_hex(0x008CFF), 0);
	lv_obj_center(my_dev->reset_btn_label);

	//lv_obj_add_event_cb(my_dev->reset_btn, reset_btn_handler, LV_EVENT_KEY_STAGE, NULL);
//	lv_obj_add_event_cb(my_dev->reset_btn, reset_btn_handler, LV_EVENT_KEY, NULL);
	lv_obj_add_event_cb(my_dev->reset_btn, reset_btn_handler, LV_EVENT_CLICKED, NULL);  //按确认键后仍会有 clicked 事件过来
	
//----------------------------------------------------------//
    lv_obj_add_flag( my_dev->reset_btn, LV_OBJ_FLAG_CLICK_FOCUSABLE); //
    
//	lv_obj_add_state( my_dev->reset_btn, LV_STATE_FOCUS_KEY);  //加此状态后才会有选中框效果. 

	lv_group_add_obj(lv_group_get_default(), my_dev->reset_btn);  
	
	lv_group_focus_obj(my_dev->reset_btn);
	sys_setCurMainWinAndCb(my_dev->reset_btn, reset_btn_handler);
	return;
}

/**********************
*   GLOBAL FUNCTIONS
**********************/

void setting_my_device_del(void)
{
	if (my_dev != NULL)
	{
		/* if (my_dev->m_isPwdEdtScr) {
			prp_loginScr_deinit();  my_dev->m_isPwdEdtScr = 0;
		} */
		lv_obj_del(my_dev->cont);
		lv_mem_free(my_dev);  my_dev = NULL;
	}

}

void setting_my_device_entry(void)
{

	my_device_init();

	setting_my_device_constructor();
//----------------------------------------------------------//

	lv_group_add_obj(lv_group_get_default(), my_dev->cont);  //xzz
	
	lv_obj_add_flag(my_dev->cont, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_flag(my_dev->cont, LV_OBJ_FLAG_CLICK_FOCUSABLE);
	
//    lv_obj_add_event_cb(my_dev->cont, my_device_cb, LV_EVENT_KEY, NULL);
	
//	lv_group_focus_obj(my_dev->cont);

	return;
	
	
}


/** 
 * sugy add 20241118 
 */
int seting_hanlder_sub_my_device(int param)
{
#if 0
	if (PRP_CMD_CODE_LOGIN_OK== param) {
		prp_loginScr_deinit();
		notice_win_open();
	}
	else if (PRP_CMD_CODE_LOGIN_FAIL == param)
	{
		prp_loginScr_deinit();
		my_dev->m_isPwdEdtScr = 0;
		lv_group_focus_obj(my_dev->reset_btn);
	}
#endif
	return 0;
}



