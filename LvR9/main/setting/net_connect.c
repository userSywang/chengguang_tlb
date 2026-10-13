//========================================================[]
//Name: net_connect.c
//Function: net connect.
//========================================================[]

/**********************
	 INCLUDES
 *********************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/wifi/wifi_connect.h"
#include "../utils/global_parm.h"
#include "net_connect.h"
#include "../ex_widget/app_msg.h"
#include "../utils/misc_utils.h"
#include "../data_parse/img_res.h"
#include "../mainmenu/tlb_main_img.h"
#include "../utils/global_parm.h"
#include "./app_setting_base.h"
#include "../utils/style_tz.h"
#ifndef C_PC_SIM
#include "../../trd/tz_sys_lib/include/cvi_tz_sys.h"
#endif


#ifndef C_PC_SIM
#include <time.h>
#include "../lvgl_main.h"
 //#include "http_client_example.h"
 //#include "audio_player.h"
 //#include "audio_stream.h"

#else
#include <Windows.h>

#endif

#include <lvgl/lvgl.h>

#define REDEFINE0701  1

 /*********************
  *      DEFINES
  *********************/
#define TAG "netCnnct"


  /**********************
   *      TYPEDEFS
   **********************/


   /**********************
	*  STATIC PROTOTYPES
   **********************/


   /**********************
	*      MACROS
	**********************/
	//----------- 主版面 ---------------//

#define	NETWORK_TITLE_STR			"配置网络"

#define NETWORK_TITLE_H		30
#define NETWORK_TITLE_Y		18

#define NETWORK_TITLE_TEXT_COLOR	0


#define NETWORK_MAIN_BG_X		0
#define NETWORK_MAIN_BG_Y		49
#define NETWORK_MAIN_BG_W		SCREEN_WIDTH
#define NETWORK_MAIN_BG_H		(SCREEN_HEIGHT - NETWORK_MAIN_BG_Y)

#define NETWORK_SWITCH_BG_W		(280) //第一个框 wifi 开关背景框
#define NETWORK_SWITCH_BG_H		50
#define NETWORK_SWITCH_BG_X		(11)
#define NETWORK_SWITCH_BG_Y		(60)

#define WIFI_SWITCH_STR			"Wi-Fi"

#define WIFI_SWITCH_STR_W		(50) //  wi - Fi  
#define WIFI_SWITCH_STR_H		(18)
#define WIFI_SWITCH_STR_X		(27)
#define WIFI_SWITCH_STR_Y		(0)
#define WIFI_SWITCH_STR_COLOR	0x18191B

#define CELLULAR_SWITCH_STR_W		(160) //  wi - Fi  
#define CELLULAR_SWITCH_STR_H		(18)
#define CELLULAR_SWITCH_STR_X		(27)
#define CELLULAR_SWITCH_STR_Y		(0)
#define CELLULAR_SWITCH_STR_COLOR	0

#define WIFI_SWITCH_W		(48) //  wi - Fi  
#define WIFI_SWITCH_H		(25)
#define WIFI_SWITCH_X		(- 20)
#define WIFI_SWITCH_Y		(0)


#define	NETWORK_LIST_TITLE_STR			"网络"

// 状态文本
#define NET_STATUS_TEXT_CLOSED   "    要查看可用网络，请打开wifi"
#define NET_STATUS_TEXT_SEARCHING "                   正在搜索网络..."
#define NET_STATUS_TEXT_NOT_FOUND "   没搜到网络，刷新试试"


#define NETWORK_LIST_TITLE_W		(SCREEN_WIDTH - 20) //网络列表标题
#define NETWORK_LIST_TITLE_H		30
#define NETWORK_LIST_TITLE_X		(10)
#define NETWORK_LIST_TITLE_Y		(122)


#define NETWORK_LIST_X		20
#define NETWORK_LIST_Y		147
#define NETWORK_LIST_W		SCREEN_WIDTH -24
#define NETWORK_LIST_H		(SCREEN_HEIGHT/2)

#define WIFI_LIST_BG_COLOR	0xFFFFFF
#define NETWORK_LIST_PAD_ALL	0
#define NETWORK_LIST_ROW_PAD	0

#define NETWORK_LIST_ITEM_W		(SCREEN_WIDTH-NETWORK_LIST_PAD_ALL*2)	//列表条目宽度
#define NETWORK_LIST_ITEM_H		50 				//列表条目高度
#define NETWORK_LIST_ITEM_X		10 				//列表条目高度
#define NETWORK_LIST_ITEM_Y		40 				//列表条目高度

#define NETWORK_LIST_ITEM_LABEL_W	210 //列表wifi名称最大长度
#define NETWORK_LIST_ITEM_LABEL_H	40 //列表wifi名称最大长度
#define NETWORK_LIST_ITEM_LABEL_X	27 //列表wifi名称起始坐标

#define NETWORK_LIST_ITEM_LOCK_X	(SCREEN_WIDTH - 100) //列表wifi 密码锁位置  相对list item的位置
#define NETWORK_LIST_ITEM_LOCK_Y	11 //列表wifi 密码锁位置
#define NETWORK_LIST_ITEM_LOCK_W	17 //
#define NETWORK_LIST_ITEM_LOCK_H	17 //

#define NETWORK_LIST_ITEM_RSSI_X	(SCREEN_WIDTH - 95) //列表wifi 信号强度 相对list item的位置
#define NETWORK_LIST_ITEM_RSSI_Y	5  // 
#define NETWORK_LIST_ITEM_LOCK_W	17 //
#define NETWORK_LIST_ITEM_LOCK_H	17 //



#define NETWORK_LIST_ITEM_FOCUSED_COLOR		(0x79B9FF)  	//选中时背景颜色
#define NETWORK_LIST_ITEM_PRESSED_COLOR	    (0xC0C0C0)  	//未选中时背景颜色
#define NETWORK_LIST_ITEM_UNFOCUSED_COLOR	(0xEFEFEF)  	//未选中时背景颜色

#define NETWORK_LIST_TEXT_FOCUSED_COLOR		(0xc0c0c0)  	//选中时字体颜色
#define NETWORK_LIST_TEXT_UNFOCUSED_COLOR	(0)  	//未选中时字体颜色

#define NETWORK_LIST_BG_COLOR		0x000000

#define NETWORK_SEL_COLOR		0x2196F3	//0xE7E7E7 //选择框颜色


#define CELLULAR_SWITCH_STR       "使用蜂窝数据链接"
#define CELLULAR_SWITCH_X         WIFI_SWITCH_X  
#define CELLULAR_SWITCH_Y         (WIFI_SWITCH_Y + 60) // 初始位置，动态调整

#define MIN_RSSI	0		//表示最弱程度的WIFI信号强度
#define MAX_RSSI	100			//表示最强程度的WIFI信号强度。
//-----------密码输入框--------------
#define WIFI_PSW_LABEL_W		(SCREEN_WIDTH - 40) //第一个框 wifi 
#define WIFI_PSW_LABEL_H		50
#define WIFI_PSW_LABEL_X		(30)
#define WIFI_PSW_LABEL_Y		(127)

//--------- 连接按钮 ------------
#define WIFI_CONNECT_BTN_H		 30
#define WIFI_CONNECT_BTN_Y		(10)

#define WIFI_CONNECT_BTN_STR	"连 接"

#define WIFI_CONNECT_BTN_BG_COLOR		0x79B9FF	//BTN 背景颜色
#define WIFI_CONNECT_BTN_TXT_COLOR		0xFFFFFF	//字体颜色

#define REFRESH_BTN_X 255
#define REFRESH_BTN_Y 10
#define REFRESH_BTN_SIZE 30
#define REFRESH_ICON_SIZE 20
//-----------------------------

/**********************
 *  STATIC VARIABLES
 **********************/

static network_connect_set_t* network;



static network_state_t net_state = 0;

static network_wifi_info_t  wf_test_list[] =
{
	{"XiaoMi_39678-high-speed", 40, 1 },
	{"ZJKJ-BOLL-4F", 30, 1 },
	{"CCMC-8086", 44, 1 },
	{"CCMC-com-4567", 0, 1 },
	{"ChinaNet-cjkey", 20, 1 },
	{"VIVO_S16_point", 40, 1 },
	{"Unknow-password-own", 46, 1 },
	{"CCMC-332256", 35, 1 },
	{"WKFLJ_B1030", 5, 1 },
	{"WKXCSJ-C3902", 50, 1 },

};
#if 0
const unsigned int rssi_levl_pic[5] =
{
	SETTING_WIFI_IMG_1,
	SETTING_WIFI_IMG_2,
	SETTING_WIFI_IMG_3,
	SETTING_WIFI_IMG_4,
	SETTING_WIFI_IMG_5
};
#endif

static tz_wifi_dev_t g_lastWifiDevTbl[NETWORK_DEFAULT_SCAN_LIST_SIZE];

static network_obj_list_t wf_olist[NETWORK_DEFAULT_SCAN_LIST_SIZE] = { 0 }; //当前搜索到的wifi 列表


static wifi_recent_info_t* recent_wf_list;  // only refer

static bool g_hasRequestScan;  // for turnOn wifi op.

/*************************************************************************************
 *   extra  FUNCTIONS
 ************************************************************************************/

bool get_esp32s3_wifi_connected_timeout(void);
int esp_wifi_recent_compare(char* in_ssid, char** out_pass);
void esp_wifi_save_to_recent(char* in_ssid, char* in_pass);
int esp_get_cur_conn_wifi(char** out_ssid, char** out_pass);
static void create_connected_network_info(char* ssid, int connection_status);
static void update_netword_device_ui(network_state_t stat);
wifi_recent_info_t* getPrjNetworkRecentList(void);

//wifi 开关状态
uint16_t  getPrjNetworkSWstate(void);
void  setPrjNetworkSWstate(void);
void  clearPrjNetworkSWstate(void);
static void refresh_button_event_cb(lv_event_t* e);
//--------------------------------------------------------------

/***********************************************************************************
 *  FUNCTIONS DECLARE
 ***********************************************************************************/
static void network_switch_list_event_handler(lv_event_t* e);
static void network_list_create(void);
static void network_list_del(void);
static void pssword_input_win_create(void);
static void pssword_input_win_del(void);
static void network_wifi_sw_create(void);
static void network_wifi_sw_del(void);

static void network_pc_sim_connect_wifi_init(void);


static void network_list_data_init(void);

static void network_list_move_connected_to_top(char* ssid);

static void network_wifi_list_data_get(void);

void esp_rescan_and_con_recent_wifi(void);

static void refresh_timer_handler(lv_timer_t* tmr);
static void network_title_create(void);
static void _netCfg_esc_btn_click_cb(lv_event_t* evt);
static void _netCfg_del_btn_click_cb(lv_event_t* evt);
static void refresh_layout();
/****************************************************************
 *   STATIC  FUNCTIONS
 ****************************************************************/
#ifndef C_PC_SIM

#else
bool sys_get_wifi_link_state(void)
{
	if (network->wifi_sw_state)
		return true;
	return false;
}
#endif


#if 1

//==============================================================[]
// wifi 记录存储
//==============================================================[]
// @return 1 表示成功，并将密码赋给指针, < 0 失败 
static int find_password_in_recent_lst(char* in_ssid, char* pOutPwd, int bufPwdLen, char* pOutSecurity, int bufScrtLen)
{
	if (!pOutPwd || (bufPwdLen < 2))
		return -3;

	int i = 0;
	for (i = 0; i < NETWORK_RECENT_LIST_MAX_NUM; i++)
	{
		if (strcmp(recent_wf_list[i].ssid, in_ssid) == 0)
		{
			ESP_LOGW(TAG, "^^find Pwd A");
			int pwdLen = strlen(recent_wf_list[i].password);
			if (bufPwdLen < pwdLen + 1)
				return -7;
			pOutPwd[0] = '\0';
			strncat(pOutPwd, recent_wf_list[i].password, bufPwdLen - 1);
			if (pOutSecurity) {
				pOutSecurity[0] = '\0';
				strncat(pOutSecurity, recent_wf_list[i].security, bufScrtLen - 1);
			}
			ESP_LOGW(TAG, "^^psw:%s, secrt:%s", pOutPwd, pOutSecurity);
			return(1);
		}
	}
	ESP_LOGW(TAG, "^^pwd not found! ssid:%s", in_ssid);
	return(0);
}
//-----------------保存已连接过的wifi-----------------------------------
//如果当前wifi没存储过，则存入并放在最前面，如果之前已存过，则把它移到最顶上。
//如果已存满，则最后一个丢掉；
static void network_wifi_save_to_recent(const char* in_ssid, const char* in_pass, const char* security)
{
	int i = 0;
	int ps = 0;
	for (i = 0; i < NETWORK_RECENT_LIST_MAX_NUM; i++)
	{
		if (strcmp(recent_wf_list[i].ssid, in_ssid) == 0)

			break;
	}
	ps = i;  //原来的位置	
	if (ps >= NETWORK_RECENT_LIST_MAX_NUM)
	{
		ps = NETWORK_RECENT_LIST_MAX_NUM - 1; //最后一个
	}

	i = ps;
	while (i > 0)
	{
		strlcpy(recent_wf_list[i].ssid, recent_wf_list[i - 1].ssid, sizeof(recent_wf_list[i].ssid));
		strlcpy(recent_wf_list[i].password, recent_wf_list[i - 1].password, sizeof(recent_wf_list[i].password));

		i--;
	}
	strlcpy(recent_wf_list[0].ssid, in_ssid, sizeof(recent_wf_list[0].ssid));
	strlcpy(recent_wf_list[0].password, in_pass, sizeof(recent_wf_list[0].password));
	strlcpy(recent_wf_list[0].security, security, sizeof(recent_wf_list[0].security));

}


/** 从最近的扫描结果列表里求 路由器的 Security
 */
static int _readWifiSecurityFromLastScanResult(const char* pcSsid, char* pcSecurity, int bufLen)
{
	if (!pcSsid || !pcSsid[0] || !pcSecurity || (bufLen < 2))
		return -1;

	pcSecurity[0] = '\0';
	int idx = 0;
	bool bFound = false;
	ESP_LOGI(TAG, "readScrty A 3, %s", pcSsid);
	for (idx = 0; idx < network->list_item_num; idx++)
	{
		if (strcmp(g_lastWifiDevTbl[idx].wifi_name, pcSsid) == 0) {
			bFound = true;  break;
		}
	}
	if (bFound) // found
	{
		strlcpy(pcSecurity, g_lastWifiDevTbl[idx].security, bufLen);
		ESP_LOGW(TAG, "readScrty ret %s", pcSecurity);
		return 0;
	}
	ESP_LOGW(TAG, "readScrty fail Z");
	return -1;
}


#endif

//==============================================================[]
// 启动wifi扫描部分
//==============================================================[]

#ifndef C_PC_SIM	

bool get_dev_wifi_connected_state(void)
{
	tz_wifi_status_t stStat = { 0 };
	if (tz_get_wifi_status(&stStat) == 0) {
		return stStat.link && stStat.ssid[0] != '\0';
	}
	return false;
}


/**
 * @return : 1- has no result, 2 - has result, but isExpired;
 */
int is_wifi_show_pls_wait_stat(int periodMs)
{
	tz_wifi_status_t stStat = { 0 };
	int ret = tz_get_wifi_status(&stStat);
	if (!stStat.on) {
		return 0;
	}
	if (!sys_wifi_has_scan_result())
		return 1;

	if (!sys_wifi_result_is_not_expired(periodMs))
		return 2;
	return 0;
}

int esp_get_cur_conn_wifi(char** out_ssid, char** out_pass)
{
	static tz_wifi_status_t stStat = { 0 };  // non-std
	static char g_acPwdSpc[1] = { 0 };

	int ret = tz_get_wifi_status(&stStat);

	if (out_ssid) {
		(*out_ssid) = stStat.ssid;
	}
	if (out_pass) {
		(*out_pass) = g_acPwdSpc;
	}
	return 0;
}


//---再次扫描并连接最近连接成功过的wifi  ----
static void wifi_rescan_and_connect_timer_handler(lv_timer_t* tmr)
{
	int* times = (int*)(tmr->user_data);

	tz_wifi_status_t  stWifiSta = { 0 };
	int getWifiRet = tz_get_wifi_status(&stWifiSta);
	ESP_LOGI(TAG, "gtWifiSta ret %d", getWifiRet);
	if (stWifiSta.on)
	{
		LV_LOG("%s [%d] finished \n", __FUNCTION__, __LINE__);
		lv_timer_del(network->timer);
		network->timer = NULL;
		if (network->ms_box != NULL)
		{
			LV_LOG("%s [%d] lv_obj_del(network->ms_box) \n", __FUNCTION__, __LINE__);
			lv_msgbox_close(network->ms_box);
			network->ms_box = NULL;
		}
		network_wifi_list_data_get();
		if (get_dev_wifi_connected_state())
		{
			char* ssid = NULL;
			esp_get_cur_conn_wifi(&ssid, NULL);
			network_list_move_connected_to_top(ssid);
		}
		network_list_create();
	}
	else
	{
		(*times)++;
		if (*times > 30)
		{
			//未成功 

			LV_LOG("%s [%d]  timeout ...... \n", __FUNCTION__, __LINE__);
			lv_timer_del(network->timer);
			network->timer = NULL;
			if (network->ms_box != NULL)
			{
				LV_LOG("%s [%d] lv_obj_del(network->ms_box) \n", __FUNCTION__, __LINE__);
				lv_msgbox_close(network->ms_box);
				network->ms_box = NULL;
			}
			app_msgbox_timeout("", "网络启动连接失败...", 1000, 0, 0);
			//		lv_obj_clear_state(network->wifi_sw_obj, LV_STATE_CHECKED);	
			return;
		}
		LV_LOG("%s [%d] times: %d\n", __FUNCTION__, __LINE__, (int)*times);
		lv_timer_reset(network->timer);
	}
}

//==============================================================[]
//连接网络...... 
//==============================================================[]
static void wait_wifi_turnon_timer_handler(lv_timer_t* tmr)
{
	int* times = (int*)(tmr->user_data);

	tz_wifi_status_t stWifiSta = { 0 };
	int getWifiRet = tz_get_wifi_status(&stWifiSta);
	ESP_LOGI(TAG, "gtWifiSta ret %d", getWifiRet);

	if (stWifiSta.on)
	{
		// 首次启动时请求扫描
		if (!g_hasRequestScan) {
			g_hasRequestScan = 1;
			int reqRet = sys_wifi_request_scan();
			if (reqRet > 0) {
				ESP_LOGI(TAG, "扫描请求成功");
			}
			else {
				ESP_LOGE(TAG, "扫描请求失败: %d", reqRet);
			}
			return; // 下次再检查
		}

		// 检查扫描结果是否就绪
		if (sys_wifi_has_scan_result())
		{
			// 关闭等待弹窗
			if (network->ms_box != NULL) {
				lv_msgbox_close(network->ms_box);
				network->ms_box = NULL;
			}

			// 获取扫描结果
			network_wifi_list_data_get();

			// 如果当前已连接WiFi，将其置顶显示
			if (get_dev_wifi_connected_state()) {
				char* ssid = NULL;
				esp_get_cur_conn_wifi(&ssid, NULL);
				network_list_move_connected_to_top(ssid);
			}

			// 创建列表UI
			network_list_create();

			// 删除定时器
			lv_timer_del(tmr);
			network->timer = NULL;
		}
		else
		{
			// 扫描未完成，检查超时
			(*times)++;
			if (*times > 30) // 15秒超时
			{
				lv_timer_del(tmr);
				network->timer = NULL;
				if (network->ms_box != NULL) {
					lv_msgbox_close(network->ms_box);
					network->ms_box = NULL;
				}
				app_msgbox_timeout("", "扫描网络超时", 1000, 0, 0);
			}
		}
	}
	else
	{
		// WiFi未开启，检查超时
		(*times)++;
		if (*times > 30) // 15秒超时
		{
			lv_timer_del(tmr);
			network->timer = NULL;
			if (network->ms_box != NULL) {
				lv_msgbox_close(network->ms_box);
				network->ms_box = NULL;
			}
			app_msgbox_timeout("", "启动WiFi超时", 1000, 0, 0);
			lv_obj_clear_state(network->wifi_sw_obj, LV_STATE_CHECKED);
		}
	}
}

#endif

//============================================================[]
//开启网络等待自动连接，以前连接记录的wifi
static void network_wait_wifi_turnon_init(void)
{
	setPrjNetworkSWstate();
#ifndef C_PC_SIM	
	LV_LOG("%s [%d] \n", __FUNCTION__, __LINE__);
	if (network->ms_box != NULL) lv_msgbox_close(network->ms_box);

#if REDEFINE0701
	update_netword_device_ui(NET_STATE_SCANNING);
#else
	network->ms_box = app_msgbox_obj(network->cont, "", "启动网络中...");
#endif
	tz_set_wifi_onoff(1);

	// 重置标志和定时器
	g_hasRequestScan = 0;
	network->times = 0;
	if (network->timer) lv_timer_del(network->timer);
	network->timer = lv_timer_create(wait_wifi_turnon_timer_handler, 500, &network->times);
#else

	network_pc_sim_connect_wifi_init();
	network_list_create();

#endif
}

//=================仿真连接=====================
static void network_pc_sim_connect_wifi_init(void)
{
	int i = 0;
	int ret = 0;
	LV_LOG("%s [%d] \n", __FUNCTION__, __LINE__);
	network_wifi_list_data_get();
	for (i = 0; i < network->list_item_num; i++)
	{
		char acPwd[64];
		char acSecurity[64];
		ret = find_password_in_recent_lst((char*)wf_olist[i].info->wifi_name, acPwd, sizeof(acPwd), acSecurity, sizeof(acSecurity));
		if (ret > 0) {
			network->cur_wf_ssid = wf_olist[i].info->wifi_name;
			strlcpy(network->cur_wf_password, acPwd, sizeof(network->cur_wf_password));
			strlcpy(network->cur_wf_security, acSecurity, sizeof(network->cur_wf_security));
			network_list_move_connected_to_top(network->cur_wf_ssid);

			network_wifi_save_to_recent(network->cur_wf_ssid, network->cur_wf_password, network->cur_wf_security);
			break;

		}

	}


}


//============================================================[]
static void network_wifi_turnoff(void)
{
	clearPrjNetworkSWstate();
#ifndef C_PC_SIM	
	LV_LOG("%s [%d] \n", __FUNCTION__, __LINE__);
	tz_set_wifi_onoff(0);
	ESP_LOGI(TAG, "aft setWifiOff");
#endif
}


//--------------------------------------------------------------------

static void wifi_connect_timer_handler(lv_timer_t* tmr)
{

	int* count_times = (int*)(tmr->user_data);
	if (get_dev_wifi_connected_state())
	{
		// 连接成功......
		ESP_LOGI(TAG, "%s [%d] wifi_connected  \n", __FUNCTION__, __LINE__);
#ifndef C_PC_SIM
		tz_wifi_status_t stStat = { 0 };
		if (tz_get_wifi_status(&stStat) != 0) {
			lv_timer_del(network->timer);
			app_msgbox_timeout("", "获取网络状态失败", 2000, 0, 0);
			return;
		}
		bool isConnectNewSuccess = (strcmp(stStat.ssid, network->cur_wf_ssid) == 0);
		ESP_LOGI(TAG, "%c  cur ssid:%s", (isConnectNewSuccess ? 'v' : 'x'), stStat.ssid);
#else
		bool isConnectNewSuccess = false;
#endif
		lv_timer_del(network->timer);
		network->timer = NULL;
		if (network->ms_box != NULL)
		{
			lv_msgbox_close(network->ms_box);
			network->ms_box = NULL;
		}

		if (isConnectNewSuccess) {
			create_dialog_with_timer(true, "连接成功", 2000, 184, 120);
			esp_wifi_save_to_recent(network->cur_wf_ssid, network->cur_wf_password);
			network_list_move_connected_to_top(network->cur_wf_ssid);
		}
		else {
#if 0
			app_msgbox_timeout("", "连接失败!", 1800 | MSGBOX_TOAST_FLAG, NULL, NULL);
#else
			create_dialog_with_timer(true, "连接失败", 2000, 184, 120);
#endif
		}
		network_list_create();
	}
	else
	{
		(*count_times)++;
		if (get_esp32s3_wifi_connected_timeout() && (*count_times) > 10)//连接超时或密码错误, todo?
		{
			*count_times = 0;
			lv_timer_del(network->timer);
			network->timer = NULL;
			ESP_LOGW(TAG, "连接超时或密码错误 ......");
			if (network->ms_box != NULL)
			{
				lv_msgbox_close(network->ms_box);
				network->ms_box = NULL;
			}
#if 0
			network->ms_box = app_msgbox_obj(network->cont, "", "连接失败！");
#else
			create_dialog_with_timer(true, "连接失败", 2000, 184, 120);
#endif		
#ifndef C_PC_SIM
			//再次搜索并连接
			usleep(100 * 1000);
			//todo, esp_rescan_and_con_recent_wifi();
			network->times = 0;
			network->timer = lv_timer_create(wifi_rescan_and_connect_timer_handler, 500, &network->times);
#else

			network_list_create();

#endif

		}
		else
		{ //可能驱动层无响应
			ESP_LOGI(TAG, "%s [%d]  count_times: %d  \n", __FUNCTION__, __LINE__, *count_times);
			if (*count_times > 30)
			{
				lv_timer_del(network->timer);  network->timer = NULL;
				network_list_create();
			}
			else
			{
				lv_timer_reset(network->timer);
			}
		}
	}

}


//--------------------------------------------------------


//--------------------------------------------------------


static void network_wifi_connect(char* ssid, char* password, char* security)//连接中的判断
{

#ifndef C_PC_SIM	
#if 1

	int ret = tz_wifi_connect((tz_wifi_dev_t[1]) { { .wifi_name = ssid, .security = security } }, password);
	if (ret != 0) {
		ESP_LOGE(TAG, "WiFi连接失败: %d", ret);
		create_dialog_with_timer(true, "连接失败", 2000, 184, 120);
		return;
	}



#else
	ESP_LOGI(TAG, "%s [%d] \n", __FUNCTION__, __LINE__);
	sys_wifi_request_connect(ssid, password, security);
	ESP_LOGI(TAG, "aft cnct req secrt %s.", security);

	if (network->ms_box != NULL)	lv_msgbox_close(network->ms_box);
	network->ms_box = app_msgbox_obj(network->cont, "", "网络连接中...");
	network->times = 0;
	network->timer = lv_timer_create(wifi_connect_timer_handler, 600, &network->times);
	


#endif
#else
	create_connected_network_info(ssid, 2);
	network_wifi_save_to_recent(network->cur_wf_ssid, network->cur_wf_password, network->cur_wf_security);
	network_list_move_connected_to_top(network->cur_wf_ssid);
	network_list_create();
	refresh_layout();
#endif
}
//--------------------------------------------------------
//重新输入密码....
static void wifi_reenter_password_timer_handler(lv_event_t* e)
{

	LV_LOG("%s [%d] wifi_connected \n", __FUNCTION__, __LINE__);
	lv_timer_del(network->timer);
	network->timer = NULL;
	if (network->ms_box != NULL)
	{
		lv_msgbox_close(network->ms_box);
		network->ms_box = NULL;
	}
	pssword_input_win_create();
}

//==============================================================[]
// 密码输入校验
//==============================================================[]
//----------------------------
//输入结束隐藏键盘，选中 “连接” 按钮，焦点设到父窗口接收事件
static void keyboard_input_off(void)
{
	lv_obj_add_flag(network->keyboard, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_state(network->psw_lab_bg, LV_STATE_CHECKED);

	lv_obj_add_state(network->connect_btn, LV_STATE_CHECKED);
	lv_group_focus_obj(network->psw_cont);

}
//------重新启用键盘输入---------
static void keyboard_input_on(void)
{
	lv_obj_clear_flag(network->keyboard, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_state(network->psw_lab_bg, LV_STATE_CHECKED);

	lv_obj_clear_state(network->connect_btn, LV_STATE_CHECKED);
	lv_group_focus_obj(network->keyboard);

}

static void psw_txtArea_event_cb(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* ta = lv_event_get_target(e);
	lv_obj_t* kb = lv_event_get_user_data(e);
	LV_LOG("%s :[ %d] code: %d\n", __FUNCTION__, __LINE__, code);
	if (code == LV_EVENT_FOCUSED)
	{
		lv_keyboard_set_textarea(kb, ta);
		lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}

	if (code == LV_EVENT_DEFOCUSED) {

	}
	else if (code == LV_EVENT_CANCEL) { //取消键，输入结束处理

		LV_LOG("%s :[ %d] LV_EVENT_CANCEL ta: %p\n", __FUNCTION__, __LINE__, ta);
		LV_LOG("chars: %s \n", lv_textarea_get_text(ta));
		keyboard_input_off();

	}
	else if (code == LV_EVENT_READY) {//确认键，输入结束处理
		LV_LOG("%s :[ %d] LV_EVENT_READY ta: %p \n", __FUNCTION__, __LINE__, ta);
		LV_LOG("chars: %s \n", lv_textarea_get_text(ta));
		lv_obj_t* ta = lv_keyboard_get_textarea(network->keyboard);
		char* pass = lv_textarea_get_text(ta);

		network->cur_wf_password[0] = '\0';
		strncat((char*)network->cur_wf_password, pass, 64 - 1);  //最长64 B
		char acSecurity[64];
		acSecurity[0] = '\0';
		int readRet = _readWifiSecurityFromLastScanResult(network->cur_wf_ssid, acSecurity, sizeof(acSecurity));
		if (readRet >= 0) {
			strlcpy(network->cur_wf_security, acSecurity, sizeof(network->cur_wf_security));
		}

		LV_LOG("ssid: %s \n", network->cur_wf_ssid);
		LV_LOG("password: %s \n", network->cur_wf_password);

		pssword_input_win_del();
		network_wifi_connect(network->cur_wf_ssid, network->cur_wf_password, acSecurity);
		ESP_LOGI(TAG, "cnctBtnClk Z");
		//keyboard_input_off();

	}
	else if (code == LV_EVENT_VALUE_CHANGED)
	{
		char* psw_text = lv_textarea_get_text(ta);
		LV_LOG("chars: %s \n", psw_text);
		//		LV_LOG("len: %s \n", strlen(psw_text));
	}
	lv_event_stop_processing(e);
}

static void _netCnct_connect_btn_click_cb(lv_event_t* evt)
{
	ESP_LOGI(TAG, "cnctBtnClk A");

	lv_obj_t* ta = lv_keyboard_get_textarea(network->keyboard);
	char* pass = lv_textarea_get_text(ta);

	network->cur_wf_password[0] = '\0';
	strncat((char*)network->cur_wf_password, pass, 64 - 1);  //最长64 B
	char acSecurity[64];
	acSecurity[0] = '\0';
	int readRet = _readWifiSecurityFromLastScanResult(network->cur_wf_ssid, acSecurity, sizeof(acSecurity));
	if (readRet >= 0) {
		strlcpy(network->cur_wf_security, acSecurity, sizeof(network->cur_wf_security));
	}

	LV_LOG("ssid: %s \n", network->cur_wf_ssid);
	LV_LOG("password: %s \n", network->cur_wf_password);

	pssword_input_win_del();
	network_wifi_connect(network->cur_wf_ssid, network->cur_wf_password, acSecurity);
	ESP_LOGI(TAG, "cnctBtnClk Z");
}

static void password_input_del_event_cb(lv_event_t* evt) {
	{
		lv_event_code_t code = lv_event_get_code(evt);

		ESP_LOGI(TAG, "%s :[ %d] code: %d\n", __FUNCTION__, __LINE__, code);
		if (code == LV_EVENT_DELETE)
			network->psw_cont = NULL;
	}
}

/** 从 wifi pwd 编辑版面退出
 */
static void _quitPwdEditProc(lv_event_t* evt) {
	ESP_LOGI(TAG, "escPwdEdt");
	pssword_input_win_del();
	lv_event_stop_processing(evt);

	network_wifi_list_data_get(); //不用再次搜索
#if 0	// ifndef C_PC_SIM
	if (get_dev_wifi_connected_state())
	{
		char* ssid = NULL;
		esp_get_cur_conn_wifi(&ssid, NULL);
		network_list_move_connected_to_top(ssid);
	}
#endif
	network_list_create();
}

static void password_input_event_handler(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* ta = lv_event_get_target(e);
	lv_obj_t* kb = lv_event_get_user_data(e);

	LV_LOG("%s :[ %d] code: %d\n", __FUNCTION__, __LINE__, code);
	if ((code == LV_EVENT_KEY_STAGE))
	{

		int keyVal = *(int*)lv_event_get_param(e);

		int keySub = (keyVal & LV_EX_FLAG_KEY_MASK);

		LV_LOG("%s [%d] keyVal: %x\n", __FUNCTION__, __LINE__, keyVal);

		if (keySub != LV_EX_FLAG_KEY_SHORT) 	return;

		int key = keyVal & 0xff;

		LV_LOG("%s [%d] Key: %d\n", __FUNCTION__, __LINE__, key);
		if ((key == LV_KEY_UP) || (key == LV_KEY_DOWN))
		{

			if ((network->psw_lab_bg->state & LV_STATE_CHECKED) == 0)
			{
				keyboard_input_on(); //重新输入
			}

		}
		else if (key == LV_KEY_ENTER)
		{
			if ((network->connect_btn->state & LV_STATE_CHECKED))
			{
				LV_LOG("%s [%d] 尝试连接wifi......\n", __FUNCTION__, __LINE__);

				lv_obj_t* ta = lv_keyboard_get_textarea(network->keyboard);
				char* pass = lv_textarea_get_text(ta);

				network->cur_wf_password[0] = '\0';
				strncat((char*)network->cur_wf_password, pass, 64 - 1);  //最长64 B

				LV_LOG("ssid: %s \n", network->cur_wf_ssid);
				LV_LOG("password: %s \n", network->cur_wf_password);

				pssword_input_win_del();

				network_wifi_connect(network->cur_wf_ssid, network->cur_wf_password, network->cur_wf_security);

				lv_event_stop_processing(e);

			}

		}
		else if (key == LV_KEY_ESC)
		{
			if (network->psw_lab_bg->state & LV_STATE_CHECKED)
			{
				_quitPwdEditProc(e);
				return;
			}
			else if (network->connect_btn->state & LV_STATE_CHECKED)
			{
				//不连接，退出
				LV_LOG("%s [%d] 不连接，退出......\n", __FUNCTION__, __LINE__);
				pssword_input_win_del();
				lv_event_stop_processing(e);

				network_wifi_list_data_get(); //不用再次搜索
#ifndef C_PC_SIM

				if (get_dev_wifi_connected_state())
				{
					char* ssid = NULL;
					esp_get_cur_conn_wifi(&ssid, NULL);
					network_list_move_connected_to_top(ssid);
				}
#endif
				network_list_create();
			}

		}
		else if (key == LV_KEY_LEFT) //test
		{
#if 0
			lv_obj_t* ta = lv_keyboard_get_textarea(network->keyboard);
			lv_textarea_set_text(ta, "");
			lv_textarea_set_placeholder_text(ta, " 请输入密码");
			keyboard_input_on();
#endif
		}
		else if (key == LV_KEY_EX_FUNC1) //AB 键
		{

			if (network->psw_lab_bg->state & LV_STATE_CHECKED) //如果是处在密码输入状态下。
			{
				lv_obj_t* ta = lv_keyboard_get_textarea(network->keyboard);
				lv_textarea_del_char(ta);
				lv_event_stop_processing(e);
				return;
			}

		}

	}

}

static void _netCfg_del_btn_click_cb(lv_event_t* evt)
{
	if (network->psw_cont) { //  when is pwdEditScr
		_quitPwdEditProc(evt);
	}
	else {
		ESP_LOGI(TAG, "escNetCfg");
		lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
	}
	return;
}


//-----------------------------------------
static void pssword_input_win_create(void)
{
	if (network->cont == NULL)  return;

	//lv_label_set_text(network->title_label, network->cur_wf_ssid);  // 用 router 名作为标题 

	//------------------------------	
	network->psw_cont = lv_obj_create(network->cont);

	lv_obj_set_style_pad_all(network->psw_cont, 0, 0);
	lv_obj_set_size(network->psw_cont, 320, 480);
	lv_obj_set_style_border_width(network->psw_cont, 0, 0);
	lv_obj_clear_flag(network->psw_cont, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_radius(network->psw_cont, 0, 0);


	lv_obj_set_style_bg_color(network->psw_cont, lv_color_hex(NETWORK_LIST_BG_COLOR), 0);
	lv_obj_set_style_bg_opa(network->psw_cont, LV_OPA_50, 0);

	lv_obj_t* curmain = lv_obj_create(network->psw_cont);
	lv_obj_set_size(curmain, 320, 411);
	lv_obj_align(curmain, LV_ALIGN_BOTTOM_MID, NETWORK_MAIN_BG_X, 0);
	lv_obj_clear_flag(curmain, LV_OBJ_FLAG_SCROLLABLE);

	lv_obj_set_style_pad_all(curmain, 0, 0);
	lv_obj_set_style_border_width(curmain, 0, 0);
	lv_obj_set_style_radius(curmain, 20, 0);
	//lv_obj_move_to_foreground(curmain); // 置顶显示
	lv_obj_move_foreground(curmain);

	lv_group_add_obj(lv_group_get_default(), curmain);
	lv_obj_add_event_cb(curmain, password_input_del_event_cb, LV_EVENT_DELETE, NULL);
	lv_group_add_obj(lv_group_get_default(), curmain);
	sys_setCurMainWinAndCb(curmain, password_input_event_handler);   //主要用来拦截键盘 ESC键，键盘内不处理。

	stng_createDelBtn(curmain, _netCfg_del_btn_click_cb, 0, 265, 5);


#if 1
	lv_obj_t* rect_tip = lv_label_create(curmain);
	//lv_obj_set_width(rect_tip, 251);
	//lv_label_set_long_mode(rect_tip, LV_LABEL_LONG_WRAP);
	char text_buf[256];
	snprintf(text_buf, sizeof(text_buf), "输入\"#008CFF %s#\"的密码", network->cur_wf_ssid);

	// 设置富文本并启用
	lv_label_set_text(rect_tip, text_buf);
	lv_obj_set_style_text_opa(rect_tip, LV_OPA_COVER, 0);
	lv_obj_set_style_text_color(rect_tip, lv_color_hex(0x18191B), 0);
	lv_label_set_recolor(rect_tip, true);

	lv_obj_align(rect_tip, LV_ALIGN_TOP_MID, 0, 67);


	// wi -fi  password

	network->psw_lab_bg = lv_obj_create(curmain);

	lv_obj_set_size(network->psw_lab_bg, WIFI_PSW_LABEL_W, WIFI_PSW_LABEL_H);

	lv_obj_align(network->psw_lab_bg, LV_ALIGN_TOP_LEFT, WIFI_PSW_LABEL_X, WIFI_PSW_LABEL_Y);

	lv_obj_set_style_bg_color(network->psw_lab_bg, lv_color_hex(0xF2F2F2), 0);
	lv_obj_set_style_bg_opa(network->psw_lab_bg, LV_OPA_COVER, 0);
	lv_obj_set_style_pad_all(network->psw_lab_bg, 0, 0);
	lv_obj_set_style_border_width(network->psw_lab_bg, 0, 0);
	lv_obj_set_style_radius(network->psw_lab_bg, 20, 0);
	lv_obj_set_style_border_width(network->psw_lab_bg, 0, 0);

	lv_obj_set_style_outline_width(network->psw_lab_bg, 0, 0);
	lv_obj_set_style_outline_width(network->psw_lab_bg, 0, LV_STATE_CHECKED);
	lv_obj_set_style_outline_opa(network->psw_lab_bg, LV_OPA_0, LV_STATE_CHECKED);
	//lv_obj_set_style_outline_color(network->psw_lab_bg, lv_color_hex(NETWORK_SEL_COLOR), LV_STATE_CHECKED);	
	//lv_obj_add_state(network->psw_lab_bg, LV_STATE_CHECKED);

	/*Create a keyboard to use it with an of the text areas*/
	network->keyboard = lv_keyboard_create(curmain);
	lv_obj_set_style_text_font(network->keyboard, &lv_font_montserrat_16, 0);

	/*Create a text area. The keyboard will write here*/
	lv_obj_t* text_a;
	text_a = lv_textarea_create(network->psw_lab_bg);

	lv_obj_set_style_pad_all(text_a, 0, 0);
	lv_obj_set_style_pad_hor(text_a, 12, 0);
	lv_obj_set_style_border_width(text_a, 0, 0);
	lv_obj_set_style_bg_color(text_a, lv_color_hex(0xF2F2F2), 0);

	lv_textarea_set_one_line(text_a, true);
	lv_obj_set_size(text_a, LV_PCT(100) - 20, 30);

	lv_textarea_set_placeholder_text(text_a, " 输入8-16位密码");
	// lv_textarea_set_one_line(text_a, true);
	lv_obj_align(text_a, LV_ALIGN_TOP_LEFT, 17, 16);

	//		lv_textarea_set_password_mode(text_a, true);  //有密码掩码显示
	lv_obj_add_event_cb(text_a, psw_txtArea_event_cb, LV_EVENT_ALL, network->keyboard);

	lv_keyboard_set_textarea(network->keyboard, text_a);

	lv_group_add_obj(lv_group_get_default(), network->keyboard);
	lv_group_focus_obj(network->keyboard);
#endif



}

static void pssword_input_win_del(void)
{
	if (network->psw_cont == NULL)  return;
	lv_obj_del(network->psw_cont);
	network->psw_cont = NULL;
	network->connect_btn = 0;
	network->keyboard = 0;


}

//==============================================================[]
// 根据信号强度排序
//==============================================================[]
#if 1
static void exch_list_item(network_obj_list_t list[], int16_t i, int16_t j)
{

	if (i == j)  return;

	network_obj_list_t temp_item;
#if 1
	temp_item = list[i];
	list[i] = list[j];
	list[j] = temp_item;
#else
	temp_item = list[i];
	list[i] = list[j];
	list[j] = temp_item;
#endif

}
// Quick sort 3 way
// 三切分快速排序
static void sort_list_items(network_obj_list_t list[], int16_t lo, int16_t hi)
{

	//	lv_file_explorer_t * explorer = (lv_file_explorer_t *)tb;

	if (lo >= hi) return;  //单个元素或者没有元素的情况

	int16_t lt = lo;
	int16_t i = lo + 1;  	//第一个元素是切分元素，所以指针i可以从lo+1开始
	int16_t gt = hi;

	int v = list[lo].info->signal_qua;

	while (i <= gt) //逆序大的在前，小的在后；
	{
		int i_data = list[i].info->signal_qua;

		if (i_data > v) //大于切分元素的放在lt左边，因此指针lt和指针i整体右移
		{
			exch_list_item(list, lt++, i++);
		}
		else if (i_data < v)  //小于切分元素的放在gt右边，因此指针gt需要左移
		{
			exch_list_item(list, i, gt--);
		}
		else
			i++;


	}

	//lt-gt的元素已经排定，只需对lt左边和gt右边的元素进行递归求解
	sort_list_items(list, lo, lt - 1);
	sort_list_items(list, gt + 1, hi);
}
#else
//C 库排序
static int compara_cell(void* list_i, void* list_j)
{

	network_obj_list_t* p_item_i = (network_obj_list_t*)list_i;
	network_obj_list_t* p_item_j = (network_obj_list_t*)list_j;

	//	return((p_item_j->rssi_level - p_item_i->rssi_level));
	return((p_item_j->info->signal_qua - p_item_i->info->signal_qua));

}

// void qsort(void*base,size_t num,size_t width,int(__cdecl*compare)(const void*,const void*)); 

static void sort_list_items(network_obj_list_t list[], int16_t lo, int16_t hi)
{

	qsort((void*)list, hi + 1, sizeof(network_obj_list_t), compara_cell);

}

#endif


static void sort_wifi_list(void)
{
	int  i = 0;
	LV_LOG_USER("sort_wifi_list begin . num:%d\r\n", network->list_item_num);

	sort_list_items(&wf_olist, 0, network->list_item_num - 1);
	LV_LOG_USER("sort_wifi_list end .");

	for (i = 0; i < network->list_item_num; i++)
	{
#if 0	
		lv_obj_t* lab = lv_obj_get_child(wf_olist[i].obj, 0);
		lv_label_set_text(lab, wf_olist[i].info->ssid);
#else		
		lv_obj_move_to_index(wf_olist[i].obj, i);
#endif
		LV_LOG("list_item: %p, ssid: %s, rssi:%d\r\n", wf_olist[i].obj, wf_olist[i].info->signal_qua, wf_olist[i].info->signal_qua);


	}

	//	lv_obj_invalidate(network->list);




}
//====================================================================
/*
计算信号强度
*/
// 总共4格信号，0 --4，5级
static uint8_t calculate_rssi_levl(int rssi)
{

	//	int min_rssi = MIN_RSSI; 	//表示最弱程度的WIFI信号强度
	//	int max_rssi = MAX_RSSI;	//表示最强程度的WIFI信号强度

	if (rssi < MIN_RSSI)
	{
		return(0);
	}
	if (rssi >= 90) {
		return 4;
	}
	if (rssi >= 70) {
		return 3;
	}
	if (rssi >= 50) {
		return 2;
	}
	if (rssi >= 30) {
		return 1;
	}
	return 0;

}
//----------------------------------------//
static void network_setting_init(void)
{
	if (network != NULL)	return;

#ifndef C_PC_SIM
	bool isWifiOn = sys_is_wifi_state_on();
	int16_t wifiSw = isWifiOn;
	setPrjGlobalParm(ZB_PARM_WIFI_SW, 2, &wifiSw);  // 同步drv里的 wifi 开关到全局变量 
#endif

	recent_wf_list = getPrjNetworkRecentList();
	network = heap_caps_malloc_dflt(sizeof(network_connect_set_t));
	memset(network, 0, sizeof(network_connect_set_t));

	recent_wf_list = getPrjNetworkRecentList();
#ifndef C_PC_SIM
	network->wifi_sw_state = getPrjNetworkSWstate();
#else
	network->wifi_sw_state = 0; //仿真默认是开的

#endif	
	int i = 0;
	for (i = 0; i < NETWORK_DEFAULT_SCAN_LIST_SIZE; i++)
	{
		if (wf_olist[i].info == NULL)
		{
			wf_olist[i].info = heap_caps_malloc_dflt(sizeof(network_wifi_info_t));
		}
		wf_olist[i].info->lock = 1;
		wf_olist[i].info->wifi_name[0] = '\0';
		wf_olist[i].info->signal_qua = 0;
		wf_olist[i].obj = NULL;
		//wf_olist[i].rssi_level = 0;
		wf_olist[i].conn_status = 0;

	}

	//	network_list_data_init();

	return;
}

static void network_setting_uninit(void)
{

	if (network == NULL)	return;

	heap_caps_free(network);  network = NULL;
	ESP_LOGI(TAG, "network_setting_uninit A");

	int i = 0;
	for (i = 0; i < NETWORK_DEFAULT_SCAN_LIST_SIZE; i++)
	{
		if (wf_olist[i].info != NULL)
		{
			heap_caps_free(wf_olist[i].info);  wf_olist[i].info = NULL;
		}

	}


	return;
}


//=================================================================[]
// list 相关函数
//=================================================================[]
static void network_list_data_init(void)
{
	int i = 0;
	for (i = 0; i < NETWORK_DEFAULT_SCAN_LIST_SIZE; i++)
	{
		//		LV_LOG("i: %d, lock: %d\r\n",i, wf_olist[i].info->lock );
		wf_olist[i].info->lock = 1;
		wf_olist[i].info->wifi_name[0] = '\0';
		wf_olist[i].info->signal_qua = 0;
		wf_olist[i].obj = NULL;
		//wf_olist[i].rssi_level = 0;
		wf_olist[i].conn_status = 0;
	}
}
//=================================================================[]
static void list_item_draw_handler(lv_event_t* e)
{
	lv_obj_t* obj = lv_event_get_target(e);
	lv_draw_ctx_t* draw_ctx = lv_event_get_draw_ctx(e);

	//    int index = (int)lv_event_get_user_data(e);
	int index = (int)lv_obj_get_index(obj);
#if 0
	lv_area_t p_area;
	lv_obj_get_coords(obj, &p_area);
#else
	lv_area_t p_area;
	lv_obj_get_content_coords(obj, &p_area);
#endif	
	//	LV_LOG("obj: %p: %d,%d, %d,%d, index:%d, lock:%d, level: %d \r\n", obj, p_area.x1, p_area.y1, p_area.x2, p_area.y2, index, wf_olist[index].info->lock, wf_olist[index].rssi_level);
		//LV_LOG("obj: %p: index:%d, rssi:%d, level: %d, conn_status:%d, ssid:%s \r\n", obj, index, wf_olist[index].info->signal_qua, wf_olist[index].rssi_level,  wf_olist[index].conn_status, wf_olist[index].info->wifi_name);
		//-----------------
	lv_img_dsc_t* pImgDst;
	lv_area_t pic_coords;
	lv_draw_img_dsc_t img_dsc;
#if 1	
	if (wf_olist[index].conn_status)  //如果已连接
	{
		//		pImgDst = zb_img_res_load_one(NULL, SETTING_WIFI_IMG_CONNECTED, IMG_FREE_TYPE_AUTO, NULL);	
		pImgDst = &g_XXX_img_dsc_tbl[MMN_connected - 1];

	}
	else
	{
		//		pImgDst = zb_img_res_load_one(NULL, SETTING_WIFI_IMG_LOCK, IMG_FREE_TYPE_AUTO, NULL);	
		pImgDst = &g_XXX_img_dsc_tbl[MMN_lock - 1];

	}
	if (pImgDst == NULL)
	{
		LV_LOG("%s [%d],	pImgDst ==  NULL,\n", __FUNCTION__, __LINE__);
		return;
	}
	pic_coords.x1 = p_area.x1 + NETWORK_LIST_ITEM_LOCK_X;
	pic_coords.y1 = p_area.y1 + NETWORK_LIST_ITEM_LOCK_Y;
	//pic_coords.x2 = pic_coords.x1 + pImgDst->header.w-1;
	//pic_coords.y2 = pic_coords.y1 + pImgDst->header.h-1;	

	lv_draw_img_dsc_init(&img_dsc);
	img_dsc.opa = LV_OPA_100;
	img_dsc.blend_mode = LV_BLEND_MODE_NORMAL; //LV_BLEND_MODE_MULTIPLY
	lv_draw_img(draw_ctx, &img_dsc, &pic_coords, pImgDst);


	lv_draw_img_dsc_t img2_dsc;

	//	uint32_t level_pic = rssi_levl_pic[wf_olist[index].rssi_level]; 
	//	pImgDst = zb_img_res_load_one(NULL, level_pic, IMG_FREE_TYPE_AUTO, NULL);	

	//	pImgDst = &g_XXX_img_dsc_tbl[MMN_1 - 1 + wf_olist[index].rssi_level];	

	//	LV_LOG(" index:%d, w:%d, h: %d\r\n", index, pImgDst->header.w, pImgDst->header.h);
	pic_coords.x1 = p_area.x1 + NETWORK_LIST_ITEM_RSSI_X;
	pic_coords.y1 = p_area.y1 + NETWORK_LIST_ITEM_RSSI_Y;
	pic_coords.x2 = pic_coords.x1 + pImgDst->header.w - 1;
	pic_coords.y2 = pic_coords.y1 + pImgDst->header.h - 1;

	lv_draw_img_dsc_init(&img2_dsc);

	img_dsc.opa = LV_OPA_100;
	img_dsc.blend_mode = LV_BLEND_MODE_NORMAL;
	lv_draw_img(draw_ctx, &img2_dsc, &pic_coords, pImgDst);

#endif

}


//-----------------------------------//
static void network_switch_list_event_handler(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* obj = lv_event_get_target(e);
	LV_LOG("%s [%d],code: %d , evt obj: %p,\n", __FUNCTION__, __LINE__, code, obj);

	if (code == LV_EVENT_CLICKED)
	{

	}
	else if (code == LV_EVENT_DELETE)
	{
		if (network->timer != NULL)
		{
			lv_timer_del(network->timer);
			network->timer = NULL;
		}
		network->cont = NULL;
	}
	else if ((code == LV_EVENT_KEY_STAGE))
	{

		int keyVal = *(int*)lv_event_get_param(e);

		int keySub = (keyVal & LV_EX_FLAG_KEY_MASK);

		if (keySub != LV_EX_FLAG_KEY_SHORT) 	return;

		int key = keyVal & 0xff;

		LV_LOG("%s [%d] Key: %d\n", __FUNCTION__, __LINE__, key);
        if (key == LV_KEY_ESC)
		{
			lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
		}
	}
}

static void _wifiLst_item_click_cb(lv_event_t* evt)
{
	lv_obj_t* obj = lv_event_get_target(evt);

	int32_t userData = (int32_t)lv_obj_get_event_user_data(obj, _wifiLst_item_click_cb);
	ESP_LOGI(TAG, "pgTbl %ld clcked", userData);

	network->list_sel = userData;
	network->cur_wf_ssid = wf_olist[network->list_sel].info->wifi_name; //选中的wifi
	network_list_del();
	LV_LOG("ssid: %s \n", network->cur_wf_ssid);

	char acPwd[64];
	char acSecurity[64];
	if (find_password_in_recent_lst(network->cur_wf_ssid, acPwd, sizeof(acPwd), acSecurity, sizeof(acSecurity)) > 0)
	{
		strlcpy(network->cur_wf_password, acPwd, sizeof(network->cur_wf_password));
		acSecurity[0] = '\0';
		int readRet = _readWifiSecurityFromLastScanResult(network->cur_wf_ssid, acSecurity, sizeof(acSecurity));
		if (readRet >= 0) {
			strlcpy(network->cur_wf_security, acSecurity, sizeof(network->cur_wf_security));
		}
		LV_LOG("password: %s \n", network->cur_wf_password);
		network_wifi_connect(network->cur_wf_ssid, network->cur_wf_password, acSecurity);
	}
	else
	{
		pssword_input_win_create();
		refresh_layout();
	}
	return;
}

//-----------------------------------//
//成功返回 1，失败： 0
// 每次添加一个wifi
//-----------------------------------//
static lv_obj_t* network_list_add_item(network_wifi_info_t* wf, int sn)
{
	if (network->list_item_num > NETWORK_DEFAULT_SCAN_LIST_SIZE)
	{
		LV_LOG("%s,[%d] Exceeded the maximum number! \r\n ", __FUNCTION__, __LINE__);

		return NULL;
	}

	if (network->list == NULL)
	{
		LV_LOG("%s,[%d] list is NULL! \r\n ", __FUNCTION__, __LINE__);

		return NULL;
	}

	lv_obj_t* item = lv_btn_create(network->list);
	lv_obj_remove_style_all(item);
	lv_obj_set_size(item, LV_PCT(100), NETWORK_LIST_ITEM_H);
	lv_obj_set_style_radius(item, 12, 0);
	lv_obj_set_style_pad_hor(item, 5, 0);
	lv_obj_set_style_pad_ver(item, 5, 0);

	lv_obj_t* lab = lv_label_create(item);

	lv_obj_set_size(lab, NETWORK_LIST_ITEM_LABEL_W, NETWORK_LIST_ITEM_LABEL_H); // LV_PCT(100)

	lv_obj_set_style_pad_all(lab, 0, 0);
	lv_obj_set_style_pad_ver(lab, 10, 0);

	lv_obj_align(lab, LV_ALIGN_LEFT_MID, 0, 0);  //相对btn居中  ，pad 属性优先级会降，可能不起作用

	//ESP_LOGI(TAG, "wifNm %s", wf->wifi_name);
	lv_label_set_text(lab, wf->wifi_name);

	lv_obj_set_style_text_align(lab, LV_TEXT_ALIGN_LEFT, 0); //文字相对于label

	//	lv_obj_set_style_text_align(lab, LV_TEXT_ALIGN_CENTER, 0); //文字相对于 label ,只是水平方向居中

	lv_label_set_long_mode(lab, LV_LABEL_LONG_DOT);  //要设置 label 高度才会起作用

	lv_obj_set_style_bg_opa(item, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(item, lv_color_hex(0xffffff), 0);
	//lv_obj_set_style_bg_color(item, lv_color_hex(NETWORK_LIST_ITEM_PRESSED_COLOR), LV_STATE_PRESSED); 
	//lv_obj_set_style_bg_color(item, lv_color_hex(NETWORK_LIST_ITEM_FOCUSED_COLOR), LV_STATE_CHECKED);
	//
	//lv_obj_set_style_text_color(item, lv_color_hex(NETWORK_LIST_TEXT_UNFOCUSED_COLOR), 0);		
	//lv_obj_set_style_text_color(item, lv_color_hex(NETWORK_LIST_TEXT_FOCUSED_COLOR), LV_STATE_CHECKED);
	lv_obj_add_event_cb(item, _wifiLst_item_click_cb, LV_EVENT_CLICKED, (void*)sn);

	// 判断是否为当前连接的WiFi
#ifndef C_PC_SIM
	if (get_dev_wifi_connected_state()) {
		char* cur_ssid = NULL;
		esp_get_cur_conn_wifi(&cur_ssid, NULL);
		if (strcmp(wf->wifi_name, cur_ssid) == 0) {
			// 特殊样式标记已连接
			lv_obj_set_style_bg_color(item, lv_color_hex(0xE6F7FF), 0);
			lv_obj_set_style_border_color(item, lv_color_hex(0x1890FF), 0);
			lv_obj_set_style_border_width(item, 2, 0);
		}
	}
#endif

	return item;

}
//---------------------------------------------------------[]
static void network_wifi_list_data_get(void)
{
	int i = 0;
	network_wifi_info_t* p_wf = NULL;

	network_list_data_init();
#ifndef C_PC_SIM
	int readNum = sys_wifi_read_scan_result(g_lastWifiDevTbl, NETWORK_DEFAULT_SCAN_LIST_SIZE);
	network->list_item_num = (readNum > 0 ? readNum : 0);
	///===============================
	LV_LOG("%s [%d] network->list_item_num:%d \n", __FUNCTION__, __LINE__, readNum);
	for (i = 0; i < network->list_item_num; i++)
	{
		memset(wf_olist[i].info, 0, sizeof(network_wifi_info_t));
		strlcpy(wf_olist[i].info->wifi_name, g_lastWifiDevTbl[i].wifi_name, sizeof(wf_olist[0].info->wifi_name));
		wf_olist[i].info->signal_qua = g_lastWifiDevTbl[i].signal_qua;
		//wf_olist[i].info->lock = g_lastWifiDevTbl[i].lock;
		ESP_LOGI(TAG, "~-No%d %s", i, wf_olist[i].info->wifi_name);
	}
#else
	network->list_item_num = M_ARRAY_LEN(wf_test_list);
	for (i = 0; i < network->list_item_num; i++)
	{
		p_wf = &wf_test_list[i];
		strlcpy(wf_olist[i].info->wifi_name, p_wf->wifi_name, sizeof(wf_olist[0].info->wifi_name));
		wf_olist[i].info->signal_qua = p_wf->signal_qua;
		wf_olist[i].info->lock = p_wf->lock;
	}
#endif
	ESP_LOGI(TAG, "wifiLstGet W");
	sort_list_items(&wf_olist, 0, network->list_item_num - 1);
	ESP_LOGI(TAG, "wifiLstGet Z");
	return;
}
//连接选中wifi连接后移动到最上面
static void network_list_move_connected_to_top(char* ssid)
{
#if 0
	int i = 0;

	network_wifi_info_t  sel_wf;
	//uint8_t rssi_level = wf_olist[network->list_sel].rssi_level;

	ESP_LOGI(TAG, "%s [%d] ssid:%s", __FUNCTION__, __LINE__, ssid);

	for (i = 0; i < network->list_item_num; i++)
	{
		if (strcmp(wf_olist[i].info->wifi_name, ssid) == 0)

			break;
	}
	sel_wf = *wf_olist[i].info;

	ESP_LOGI(TAG, "%s [%d] i: %d \n", __FUNCTION__, __LINE__, i);

	if ((i == network->list_item_num) && (i > 0)) i--;

	while (i > 0)
	{
		*wf_olist[i].info = *wf_olist[i - 1].info;
		//wf_olist[i].rssi_level =  wf_olist[i-1].rssi_level;
		wf_olist[i].conn_status = 0;
		i--;
	}

	*wf_olist[0].info = sel_wf;
	wf_olist[0].conn_status = 1;
	//wf_olist[0].rssi_level = rssi_level;
	network->list_sel = 0;

#else
	//应该不在列表中显示，在已连接中显示
	int found_index = -1;

	// 查找匹配的SSID在列表中的位置
	for (int i = 0; i < network->list_item_num; i++) {
		if (strcmp(wf_olist[i].info->wifi_name, ssid) == 0) {
			found_index = i;
			break;
		}
	}


	if (found_index >= 0) {

		for (int i = found_index; i < network->list_item_num - 1; i++) {
			wf_olist[i] = wf_olist[i + 1];
		}


		network->list_item_num--;

		if (network->list_sel > found_index) {
			network->list_sel--; 
		}
		else if (network->list_sel == found_index) {
			network->list_sel = 0; 
		}
	}
#endif
	net_state = NET_STATE_ILE;
	update_netword_device_ui(net_state);
	create_connected_network_info(ssid,2);


	refresh_layout();
}
//=========================================================


static void network_wifi_list_show(void)
{
	int i = 0;
	network_wifi_info_t* p_wf;
	lv_obj_t* item;

	if (network->list == NULL)
	{
		LV_LOG("%s,[%d] list is NULL! \r\n ", __FUNCTION__, __LINE__);

		return;
	}

	ESP_LOGI(TAG, "wifLst N:%d", network->list_item_num);
	for (i = 0; i < network->list_item_num;i++)
	{
		p_wf = wf_olist[i].info;

		item = network_list_add_item(p_wf, i);
		if (item == NULL)
		{
			LV_LOG("%s,[%d] network_list_add_item NULL!", __FUNCTION__, __LINE__);
			return;
		}

		if (i == 0)
		{

			network->list_sel = i;
			//lv_obj_add_state(item, LV_STATE_CHECKED);  //默认先选中第一个
			lv_obj_clear_state(network->wifi_sw_btn, LV_STATE_CHECKED);
		}
		if (item != NULL)//wifi信号
		{
			//wf_olist[i].obj = item;			
			//wf_olist[i].rssi_level = calculate_rssi_levl(p_wf->signal_qua);

			//LV_LOG("item:%p, ssid: %s, rssi: %d, level: %d, lock: %d\r\n", item,wf_olist[i].info->wifi_name, wf_olist[i].info->signal_qua, wf_olist[i].rssi_level,wf_olist[i].info->lock );
		}
		else
		{
			break;  //create add item failed.
		}
		lv_obj_add_event_cb(item, list_item_draw_handler, LV_EVENT_DRAW_POST, (void*)i);
	}
	network->list_item_num = i;
	LV_LOG("item num:%d\r\n", network->list_item_num);

	//lv_obj_update_layout(network->cont);
	//if (lv_obj_get_height(network->list) > 240) {
	//	lv_obj_set_style_pad_bottom(network->list, 20, 0); // 添加滚动空间
	//}
	return;
}

static void network_list_create(void)
{

	if (network->list != NULL)
	{
		LV_LOG("%s,[%d] list is created! \r\n ", __FUNCTION__, __LINE__);

		return;
	}
	ESP_LOGI(TAG, "%s,[%d]", __FUNCTION__, __LINE__);

	lv_label_set_text(network->title_label, NETWORK_TITLE_STR);



	network_wifi_list_show();


	lv_obj_invalidate(network->cont);
	lv_group_focus_obj(network->cont);

	sys_setCurMainWinAndCb(network->cont, network_switch_list_event_handler);
	return;
}

static void network_list_del(void)
{
	LV_LOG("%s,[%d] \r\n ", __FUNCTION__, __LINE__);

	// 隐藏但不删除标签
	lv_obj_add_flag(network->list, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(network->other_network_label, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(network->my_network_label, LV_OBJ_FLAG_HIDDEN);
}

static void netCfg_switch_chg_cb(lv_event_t* evt)
{
	ESP_LOGI(TAG, "swtChg A");
	if (network->wifi_sw_obj->state & LV_STATE_CHECKED) //打开 wifi
	{
		ESP_LOGI(TAG, "swtChg P");
		lv_obj_add_state(network->wifi_sw_obj, LV_STATE_CHECKED);
		ESP_LOGI(TAG, "swtChg R");
		network_wait_wifi_turnon_init();
		ESP_LOGI(TAG, "swtChg Z");

	}
	else
	{
		LV_LOG("%s,[%d] \r\n ", __FUNCTION__, __LINE__);
		network_wifi_turnoff();
		ESP_LOGI(TAG, "swtChg C");
		lv_obj_clear_state(network->wifi_sw_obj, LV_STATE_CHECKED);
		ESP_LOGI(TAG, "swtChg E");
		network_list_del();

		update_netword_device_ui(NET_STATE_NOSTART);
	}

	//refresh_layout();
}

static void network_wifi_sw_create(void)
{

	if (network->cont == NULL)  return;

	//  Wi Fi
	ESP_LOGI(TAG, "%s A", __FUNCTION__);
	if (network->wifi_sw_btn)
	{
		lv_obj_del(network->wifi_sw_btn);
		network->wifi_sw_btn = NULL;
	}
	network->wifi_sw_btn = lv_obj_create(network->cont);
	lv_obj_set_width(network->wifi_sw_btn, 280);
	lv_obj_set_height(network->wifi_sw_btn, NETWORK_SWITCH_BG_H);

	lv_obj_set_style_bg_color(network->wifi_sw_btn, lv_color_hex(0xFFFFFF), 0);
	lv_obj_set_style_bg_opa(network->wifi_sw_btn, LV_OPA_COVER, 0);
	lv_obj_set_style_pad_all(network->wifi_sw_btn, 0, 0);
	lv_obj_set_style_border_width(network->wifi_sw_btn, 0, 0);
	lv_obj_set_style_radius(network->wifi_sw_btn, 12, 0);

	lv_obj_set_style_outline_width(network->wifi_sw_btn, 0, 0);
	lv_obj_set_style_outline_width(network->wifi_sw_btn, 0, LV_STATE_CHECKED);
	//lv_obj_set_style_outline_color(network->wifi_sw_btn, lv_color_hex(NETWORK_SEL_COLOR), LV_STATE_CHECKED);	

	//lv_obj_add_state(network->wifi_sw_btn,  LV_STATE_CHECKED);

	// wi -fi 

	lv_obj_t* cellular_label = lv_label_create(network->wifi_sw_btn);
	lv_obj_set_size(cellular_label, WIFI_SWITCH_STR_W, WIFI_SWITCH_STR_H);
	lv_obj_set_style_pad_all(cellular_label, 0, 0);
	lv_obj_set_style_border_width(cellular_label, 0, 0);
	lv_obj_set_style_text_color(cellular_label, lv_color_hex(WIFI_SWITCH_STR_COLOR), 0);
	lv_label_set_text(cellular_label, WIFI_SWITCH_STR);
	lv_obj_align(cellular_label, LV_ALIGN_LEFT_MID, WIFI_SWITCH_STR_X, WIFI_SWITCH_STR_Y);

	network->wifi_sw_obj = lv_switch_create(network->wifi_sw_btn);
	lv_obj_set_size(network->wifi_sw_obj, WIFI_SWITCH_W, WIFI_SWITCH_H);
	lv_obj_align(network->wifi_sw_obj, LV_ALIGN_RIGHT_MID, WIFI_SWITCH_X, WIFI_SWITCH_Y);
	//lv_obj_style_set_outline_width(network->wifi_sw_obj, 0);
	//lv_obj_style_set_border_width(network->wifi_sw_obj, 0);
	lv_obj_set_style_border_width(network->wifi_sw_obj, 0, 0);
	lv_obj_set_style_outline_width(network->wifi_sw_obj, 0, 0);
	lv_obj_add_event_cb(network->wifi_sw_obj, netCfg_switch_chg_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_clear_flag(network->wifi_sw_obj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
	lv_obj_clear_state(network->wifi_sw_obj, LV_STATE_FOCUSED);
	lv_group_add_obj(lv_group_get_default(), network->cont);
	//lv_group_focus_obj(network->cont);
	lv_obj_add_event_cb(network->cont, network_switch_list_event_handler, LV_EVENT_DELETE, NULL);

	if (network->wifi_sw_state) {
		lv_obj_add_state(network->wifi_sw_obj, LV_STATE_CHECKED);

#ifndef C_PC_SIM
		// 如果已连接WiFi，显示连接信息
		if (get_dev_wifi_connected_state()) {
			char* ssid = NULL;
			esp_get_cur_conn_wifi(&ssid, NULL);
			create_connected_network_info(ssid, 2); // 已连接状态
		}
		else {
			// 启动WiFi连接流程
			network_wait_wifi_turnon_init();
		}
#endif
	}
	else {
		lv_obj_clear_state(network->wifi_sw_obj, LV_STATE_CHECKED);
	}



	sys_setCurMainWinAndCb(network->cont, network_switch_list_event_handler);
	LV_LOG_USER("%s Z", __FUNCTION__);
	return;
}
static void network_wifi_sw_del(void)
{
	if (network->wifi_sw_btn == NULL)
	{
		LV_LOG("%s,[%d] list is NULL! \r\n ", __FUNCTION__, __LINE__);

		return;
	}
	lv_obj_del(network->wifi_sw_btn);
	network->wifi_sw_btn = NULL;

	refresh_layout();
}
//------------------------------

// 蜂窝数据开关
static void cellular_switch_event_cb(lv_event_t* e) {
	lv_obj_t* sw = lv_event_get_target(e);
	if (lv_obj_has_state(sw, LV_STATE_CHECKED)) {
		// 开启蜂窝数据逻辑
	}
	else {
		// 关闭蜂窝数据逻辑
	}
}

static void network_cellular_sw_create(void) {


	network->cellular_sw_btn = lv_obj_create(network->cont);
	lv_obj_set_size(network->cellular_sw_btn, NETWORK_SWITCH_BG_W, NETWORK_SWITCH_BG_H);
	lv_obj_set_style_align(network->cellular_sw_btn, LV_ALIGN_BOTTOM_MID, 0);

	lv_obj_set_style_bg_color(network->cellular_sw_btn, lv_color_hex(0xFFFFFF), 0);
	lv_obj_set_style_bg_opa(network->cellular_sw_btn, LV_OPA_COVER, 0);
	lv_obj_set_style_pad_all(network->cellular_sw_btn, 0, 0);
	lv_obj_set_style_border_width(network->cellular_sw_btn, 0, 0);
	lv_obj_set_style_radius(network->cellular_sw_btn, 12, 0);

	lv_obj_set_style_outline_width(network->cellular_sw_btn, 0, 0);
	lv_obj_set_style_outline_width(network->cellular_sw_btn, 2, LV_STATE_CHECKED);
	lv_obj_set_style_outline_color(network->cellular_sw_btn, lv_color_hex(NETWORK_SEL_COLOR), LV_STATE_CHECKED);

	//lv_obj_add_state(network->cellular_sw_btn, LV_STATE_CHECKED);

	// wi -fi 

	lv_obj_t* cellular_label = lv_label_create(network->cellular_sw_btn);
	lv_obj_set_size(cellular_label, CELLULAR_SWITCH_STR_W, WIFI_SWITCH_STR_H);
	lv_obj_set_style_pad_all(cellular_label, 0, 0);
	lv_obj_set_style_border_width(cellular_label, 0, 0);
	lv_obj_set_style_text_color(cellular_label, lv_color_hex(WIFI_SWITCH_STR_COLOR), 0);
	lv_label_set_text(cellular_label, CELLULAR_SWITCH_STR);
	lv_obj_align(cellular_label, LV_ALIGN_LEFT_MID, WIFI_SWITCH_STR_X, WIFI_SWITCH_STR_Y);

	network->cellular_sw_obj = lv_switch_create(network->cellular_sw_btn);
	//	lv_obj_add_state(network->cellular_sw_obj,  LV_STATE_CHECKED);
	lv_obj_set_size(network->cellular_sw_obj, WIFI_SWITCH_W, WIFI_SWITCH_H);
	lv_obj_align(network->cellular_sw_obj, LV_ALIGN_RIGHT_MID, WIFI_SWITCH_X, WIFI_SWITCH_Y);
	lv_obj_add_event_cb(network->cellular_sw_obj, cellular_switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

}

static void network_title_create(void) {


	network->title_cont = lv_obj_create(network->cont);
	lv_obj_set_size(network->title_cont, LV_PCT(100), NETWORK_SWITCH_BG_H);
	//lv_obj_align(network->cellular_sw_btn, LV_ALIGN_TOP_LEFT, NETWORK_SWITCH_BG_X, NETWORK_SWITCH_BG_Y + 60);
	lv_obj_set_style_bg_color(network->title_cont, lv_color_hex(0xFFFFFF), 0);
	lv_obj_set_style_bg_opa(network->title_cont, LV_OPA_0, 0);
	lv_obj_set_style_pad_all(network->title_cont, 0, 0);
	lv_obj_set_style_border_width(network->title_cont, 0, 0);
	lv_obj_set_style_radius(network->title_cont, 12, 0);
	lv_obj_clear_flag(network->title_cont, LV_OBJ_FLAG_SCROLLABLE); //清除滚动


	lv_obj_set_style_outline_width(network->title_cont, 0, 0);
	lv_obj_set_style_outline_width(network->title_cont, 0, LV_STATE_CHECKED);
	//lv_obj_set_style_outline_color(network->title_cont, lv_color_hex(NETWORK_SEL_COLOR), LV_STATE_CHECKED);

	network->title_label = lv_label_create(network->title_cont);
	lv_obj_set_size(network->title_label, LV_SIZE_CONTENT, NETWORK_TITLE_H);
	lv_obj_set_style_pad_all(network->title_label, 0, 0);
	lv_obj_set_style_border_width(network->title_label, 0, 0);

	lv_obj_set_style_text_color(network->title_label, lv_color_hex(NETWORK_TITLE_TEXT_COLOR), 0);
	lv_label_set_text(network->title_label, NETWORK_TITLE_STR);
	lv_obj_align(network->title_label, LV_ALIGN_TOP_MID, 0, NETWORK_TITLE_Y);

	lv_obj_t* img = stng_createEscBtn(network->title_cont, _netCfg_esc_btn_click_cb, 0);
	lv_obj_set_x(img, -10);

#if 0
	lv_obj_t* cellular_label = lv_label_create(network->cellular_sw_btn);
	lv_obj_set_size(cellular_label, CELLULAR_SWITCH_STR_W, WIFI_SWITCH_STR_H);
	lv_obj_set_style_pad_all(cellular_label, 0, 0);
	lv_obj_set_style_border_width(cellular_label, 0, 0);
	lv_obj_set_style_text_color(cellular_label, lv_color_hex(WIFI_SWITCH_STR_COLOR), 0);
	lv_label_set_text(cellular_label, "配置网络");
	lv_obj_align(cellular_label, LV_ALIGN_LEFT_MID, WIFI_SWITCH_STR_X, WIFI_SWITCH_STR_Y);
#endif

	network->refresh_btn = lv_img_create(network->title_cont);
	lv_img_dsc_t* refresh_img = zb_img_res_load_one(NULL, SETTING_IMG_REFRESH, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(network->refresh_btn, refresh_img);
	lv_obj_set_style_pad_top(network->refresh_btn, 10, 0);
	lv_obj_align(network->refresh_btn, LV_ALIGN_TOP_LEFT, REFRESH_BTN_X, REFRESH_BTN_Y);
	lv_obj_add_flag(network->refresh_btn, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_set_ext_click_area(network->refresh_btn, 30);
	lv_obj_add_event_cb(network->refresh_btn, refresh_button_event_cb, LV_EVENT_CLICKED, NULL);
    
}

//------------------------------
static void wait_wifi_proc_timer_handler(lv_timer_t* tmr)
{
	ESP_LOGI(TAG, "waitTmrHdl A\n");

	// 确保处理超时情况
	if (network->tmrWaitting != tmr) {
		lv_timer_del(tmr); // 防止重复处理
		return;
	}

	// 处理超时
	int64_t currentTime = zb_get_time_ms_since_boot();
	if (currentTime > network->waitScrTimeSt + 15000) { // 15秒超时
		ESP_LOGW(TAG, "WiFi扫描超时");
		lv_timer_del(tmr);
		network->tmrWaitting = NULL;

		// 更新状态并显示错误
		net_state = NET_STATE_NOTFOUND;
		update_netword_device_ui(net_state);

		return;
	}

#ifndef C_PC_SIM
	if (!network->doesScanStart) {
		int reqRet = sys_wifi_request_scan();
		if (reqRet > 0) {
			network->doesScanStart = 1;
			ESP_LOGI(TAG, "扫描请求成功");
	}
		else {
			ESP_LOGE(TAG, "扫描请求失败: %d", reqRet);
		}
		return;
}

	// 检查扫描结果
	if (sys_wifi_has_scan_result()) {
		lv_timer_del(tmr);
		network->tmrWaitting = NULL;

		net_state = NET_STATE_ILE;
		update_netword_device_ui(net_state);

		// 获取并显示扫描结果
		network_wifi_list_data_get();
		network_list_create();
	}
#else
	// PC仿真模式处理
	//network_wifi_list_data_get();
	//network_list_create();
	//lv_timer_del(tmr);
	//network->tmrWaitting = NULL;
	
#endif
}
static void network_waiting_scr_event_cb(lv_event_t* e)
{
#define _FUNC_ABBR  "waitScrCb"
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* obj = lv_event_get_target(e);
	switch (code) {
	case LV_EVENT_DELETE:
	{
		ESP_LOGI(TAG, "%s del evt", _FUNC_ABBR);
		if (network->tmrWaitting != NULL) {
			lv_timer_del(network->tmrWaitting);  network->tmrWaitting = NULL;
		}
		network->waitScrBg = NULL;
		break;
	}
	case LV_EVENT_KEY_STAGE:
	{
		int keyVal = *(int*)lv_event_get_param(e);
		int keySub = (keyVal & LV_EX_FLAG_KEY_MASK);

		ESP_LOGI(TAG, "%s key evt %xd", _FUNC_ABBR, keyVal);
		if (keySub != LV_EX_FLAG_KEY_SHORT) 	return;
		int key = keyVal & 0xff;

		if (key == LV_KEY_ESC) {
			lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
		}
		break;
	}
	default:
		break;
	}
	return;
}

static void mynet_item_draw_handler(lv_event_t* e)
{
	lv_obj_t* obj = lv_event_get_target(e);
	lv_draw_ctx_t* draw_ctx = lv_event_get_draw_ctx(e);

	int index = (int)lv_obj_get_index(obj);

	// 获取用户数据（连接状态）
	int connection_status = (int)(intptr_t)lv_event_get_user_data(e); 

	lv_area_t p_area;
	lv_obj_get_content_coords(obj, &p_area);

	lv_img_dsc_t* pImgDst =NULL;
	lv_area_t pic_coords;
	lv_draw_img_dsc_t img_dsc;

	switch (connection_status) {
	case 0: pImgDst = &g_XXX_img_dsc_tbl[MMN_lock - 1];

		break;//在连接
	case 1: pImgDst = &g_XXX_img_dsc_tbl[MMN_connected - 1]; break;//已连接
	case 2: pImgDst = &g_XXX_img_dsc_tbl[MMN_connected - 1]; break;
	default: pImgDst = &g_XXX_img_dsc_tbl[MMN_connected - 1]; break;
	}

	if (pImgDst == NULL)
	{
		LV_LOG("%s [%d],	pImgDst ==  NULL,\n", __FUNCTION__, __LINE__);
		return;
	}
	pic_coords.x1 = p_area.x1 + NETWORK_LIST_ITEM_LOCK_X;
	pic_coords.y1 = p_area.y1 + NETWORK_LIST_ITEM_LOCK_Y;
	//pic_coords.x2 = pic_coords.x1 + pImgDst->header.w-1;
	//pic_coords.y2 = pic_coords.y1 + pImgDst->header.h-1;	

	lv_draw_img_dsc_init(&img_dsc);
	img_dsc.opa = LV_OPA_100;
	img_dsc.blend_mode = LV_BLEND_MODE_NORMAL; //LV_BLEND_MODE_MULTIPLY
	lv_draw_img(draw_ctx, &img_dsc, &pic_coords, pImgDst);


	lv_draw_img_dsc_t img2_dsc;
	pic_coords.x1 = p_area.x1 + NETWORK_LIST_ITEM_RSSI_X;
	pic_coords.y1 = p_area.y1 + NETWORK_LIST_ITEM_RSSI_Y;
	pic_coords.x2 = pic_coords.x1 + pImgDst->header.w - 1;
	pic_coords.y2 = pic_coords.y1 + pImgDst->header.h - 1;

	lv_draw_img_dsc_init(&img2_dsc);

	img_dsc.opa = LV_OPA_100;
	img_dsc.blend_mode = LV_BLEND_MODE_NORMAL;
	lv_draw_img(draw_ctx, &img2_dsc, &pic_coords, pImgDst);

}



static void create_connected_network_info(char* ssid, int connection_status) { //应该通过判断有无连接显示


	//item = network_list_add_item(p_wf, i);


	//lv_obj_add_event_cb(network->my_network_obj, mynet_item_draw_handler, LV_EVENT_DRAW_POST, (void*)network->my_network_obj);

	ESP_LOGI("create_connected_network_info","--------------A------------\r\n");
	// 添加标签和图标
	lv_obj_t* label = lv_label_create(network->my_network_obj);
	lv_label_set_text(label, ssid);
	lv_obj_align(label, LV_ALIGN_LEFT_MID, 10, 0);

	ESP_LOGI("create_connected_network_info", "--------------B------------\r\n");
	// 根据连接状态显示不同内容
	if (connection_status == 1) { // 正在连接状态
		// 创建并显示 spinner
		lv_obj_t* spinner = lv_spinner_create(network->my_network_obj, 1000, 60);
		lv_obj_set_size(spinner, 20, 20);
		lv_obj_center(spinner);
		lv_obj_set_style_arc_width(spinner, 5, LV_PART_MAIN);
		lv_obj_set_style_arc_width(spinner, 5, LV_PART_INDICATOR);

		// 将 spinner 对齐到右侧
		lv_obj_align_to(spinner, label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	}

	ESP_LOGI("create_connected_network_info", "--------------C------------\r\n");
	lv_obj_t* icon = lv_img_create(network->my_network_obj);

	// 根据状态选择图标
	switch (connection_status) {
	case 0: // 未连接
		lv_img_set_src(icon, &g_XXX_img_dsc_tbl[MMN_connected - 1]);
		break;
	case 1: //连接中 
		lv_img_set_src(icon, &g_XXX_img_dsc_tbl[MMN_lock - 1]);
		
		break;
	case 2: // 已连接
		lv_obj_set_style_text_color(label,lv_color_hex(0x008CFF),0);
		lv_img_set_src(icon, &g_XXX_img_dsc_tbl[MMN_connected - 1]);
		break;
	default: // 其他状态
		lv_img_set_src(icon, &g_XXX_img_dsc_tbl[MMN_lock - 1]);
		break;
	}
	ESP_LOGI("create_connected_network_info", "--------------D------------\r\n");
	// 将图标对齐到右侧
	lv_obj_align(icon, LV_ALIGN_RIGHT_MID, -20, 0);



	refresh_layout();
	ESP_LOGI("create_connected_network_info", "--------------E------------\r\n");
}


static void network_waiting_scr_create(void)
{
#if 0
	if (!network->cont)
		return;
	ESP_LOGI(TAG, "%s A", __FUNCTION__);

	network->waitScrTimeSt = zb_get_time_ms_since_boot();
	network->waitScrBg = lv_obj_create(network->cont);
	lv_obj_set_size(network->waitScrBg, SCREEN_WIDTH, SCREEN_HEIGHT);

	lv_obj_align(network->waitScrBg, LV_ALIGN_TOP_LEFT, NETWORK_MAIN_BG_X, NETWORK_MAIN_BG_Y);
	lv_obj_set_style_bg_color(network->waitScrBg, lv_color_hex(NETWORK_LIST_BG_COLOR), 0);
	lv_obj_set_style_bg_opa(network->waitScrBg, LV_OPA_COVER, 0);
	lv_obj_set_style_pad_all(network->waitScrBg, 0, 0);
	lv_obj_set_style_border_width(network->waitScrBg, 0, 0);
	//lv_obj_set_style_radius(network->waitScrBg, 20, 0);

	lv_obj_t* labelPlsWait = lv_label_create(network->waitScrBg);
	lv_obj_set_style_pad_all(labelPlsWait, 0, 0);
	lv_obj_set_style_border_width(labelPlsWait, 0, 0);
	lv_obj_set_style_text_color(labelPlsWait, lv_color_hex(WIFI_SWITCH_STR_COLOR), 0);
	lv_label_set_text(labelPlsWait, "请稍候...");
	lv_obj_align(labelPlsWait, LV_ALIGN_CENTER, 0, -30);
	network->waitScrBg = labelPlsWait;

	lv_group_add_obj(lv_group_get_default(), network->waitScrBg);
	lv_obj_add_event_cb(network->waitScrBg, network_waiting_scr_event_cb, LV_EVENT_ALL, NULL);
#else
#ifndef C_PC_SIM
	int reqRet = sys_wifi_request_scan();
	if (reqRet > 0) {
		network->doesScanStart = 1;
		ESP_LOGI(TAG, "reqScn ok\n");
	}
	else {
		ESP_LOGI(TAG, "reqScn fail %d\n", reqRet);
	}
#endif
#endif
	net_state = NET_STATE_INSERACH;
	update_netword_device_ui(net_state);

	network->tmrWaitting = lv_timer_create(wait_wifi_proc_timer_handler, 1000, NULL);

	lv_obj_invalidate(network->cont);
	return;
}

static void update_netword_device_ui(network_state_t stat) {

	// 根据状态更新UI
	switch (stat) {
	case NET_STATE_SCANNING:
		lv_label_set_text(network->status_label, "正在搜索网络");
		lv_obj_clear_flag(network->status_label, LV_OBJ_FLAG_HIDDEN);
		break;
	case NET_STATE_SHOWING_LIST:
		lv_obj_add_flag(network->status_label, LV_OBJ_FLAG_HIDDEN);

	case NET_STATE_NOSTART:
		lv_label_set_text(network->status_label, NET_STATUS_TEXT_CLOSED);
		lv_obj_add_flag(network->my_network_cont, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(network->other_network_label, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(network->list, LV_OBJ_FLAG_HIDDEN);

		break;

	case NET_STATE_CONNECTING:
		
		break;

	case NET_STATE_INSERACH:
		lv_label_set_text(network->status_label, NET_STATUS_TEXT_SEARCHING);
		break;

	case NET_STATE_NOTFOUND:
		lv_label_set_text(network->status_label, NET_STATUS_TEXT_NOT_FOUND);
		break;

	case NET_STATE_CONNECTED:

		break;
	case NET_STATE_FAILED:

		break;

	case  NET_STATE_ILE:
		
		if (network->status_label)
		{
			lv_obj_add_flag(network->status_label, LV_OBJ_FLAG_HIDDEN);
		}
		return;


	default:
		break;
	}
	// 显示状态标签
	//lv_obj_clear_flag(network->status_label, LV_OBJ_FLAG_HIDDEN);

	//if (network->status_label)
	//{
	//	int num = lv_obj_get_index(network->wifi_sw_btn);
	//	LV_LOG_USER("network->wifi_sw_btn index:%d", num);
	//	lv_obj_move_to_index(network->status_label, num);
	//}
	net_state = stat;
}


static void _netCfg_esc_btn_click_cb(lv_event_t* evt)
{
	if (network->psw_cont) { //  when is pwdEditScr
		_quitPwdEditProc(evt);
	}
	else {
		ESP_LOGI(TAG, "escNetCfg");
		lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
	}
	return;
}


// 刷新按钮事件处理函数
static void refresh_button_event_cb(lv_event_t* e)
{
	lv_obj_t* btn = lv_event_get_target(e);
	(void)btn; // 未使用参数

	if (lv_obj_get_state(network->wifi_sw_obj) & LV_STATE_CHECKED) {
		// 显示扫描提示
		if (network->ms_box) lv_msgbox_close(network->ms_box);
		network->ms_box = app_msgbox_obj(network->cont, "", "重新扫描中...");

		// 重置扫描状态
		g_hasRequestScan = 0;
		network->times = 0;

		// 启动新的扫描定时器
		if (network->timer) lv_timer_del(network->timer);
#ifndef C_PC_SIM
		network->timer = lv_timer_create(wait_wifi_turnon_timer_handler, 500, &network->times);
#endif
	}
}

// 刷新定时器处理函数
static void refresh_timer_handler(lv_timer_t* tmr)
{
	int* times = (int*)(tmr->user_data);
	(*times)++;

#ifndef C_PC_SIM
	if (sys_wifi_has_scan_result()) {
#else
	if (true) { // PC模拟直接通过
#endif
		// 关闭提示框
		if (network->ms_box) {
			lv_msgbox_close(network->ms_box);
			network->ms_box = NULL;
		}

		// 获取新数据并更新列表
		network_wifi_list_data_get();
		network_list_create();
#ifndef C_PC_SIM
		tz_wifi_status_t stStat = { 0 };
		int ret = tz_get_wifi_status(&stStat);
		if (stStat.link) {
			if (stStat.ssid[0]) {
				network_list_move_connected_to_top(stStat.ssid);
			}
		}
#endif
		lv_timer_del(network->timer);
		network->timer = NULL;
	}
	else if (*times > 20) { // 10秒超时
		if (network->ms_box) {
			lv_msgbox_close(network->ms_box);
			network->ms_box = NULL;
		}
		app_msgbox_timeout("", "扫描超时", 2000, 0, 0);
		lv_timer_del(network->timer);
		network->timer = NULL;
	}
	}

static void network_setting_constructor(void)
{
	//----------大背景 --------
	network->cont = lv_obj_create(lv_scr_act());

	lv_obj_set_size(network->cont, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_align(network->cont, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_set_style_pad_all(network->cont, 0, 0);
	lv_obj_set_style_bg_color(network->cont, lv_color_hex(STNG_AP_BG_COLOR_HEX), 0);
	lv_obj_set_style_bg_opa(network->cont, LV_OPA_COVER, 0);
	lv_obj_set_style_pad_hor(network->cont, 20, 0);
	lv_obj_set_style_border_width(network->cont, 0, 0);
	lv_obj_set_style_radius(network->cont, 0, 0);
	lv_obj_set_flex_flow(network->cont, LV_FLEX_FLOW_COLUMN); 

	lv_obj_set_style_pad_row(network->cont, 10, 0);

	lv_obj_set_scrollbar_mode(network->cont, LV_SCROLLBAR_MODE_OFF);


// ----- -- " 网 络 " 标题	----------
	network_title_create();

	network_wifi_sw_create();
	network_cellular_sw_create();

	network->status_label = lv_label_create(network->cont);

	lv_obj_set_style_text_color(network->status_label, lv_color_hex(0x787878), 0);

	network->my_network_cont = lv_obj_create(network->cont);
	lv_obj_set_size(network->my_network_cont,280, LV_SIZE_CONTENT);
	lv_obj_set_layout(network->my_network_cont, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(network->my_network_cont, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(network->my_network_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_all(network->my_network_cont, 0, 0);
	lv_obj_set_style_bg_opa(network->my_network_cont, LV_OPA_TRANSP, 0);
	lv_obj_set_style_border_width(network->my_network_cont, 0, 0);
	lv_obj_set_style_pad_row(network->my_network_cont, 12, 0);
	lv_obj_set_style_pad_bottom(network->my_network_cont, 15, 0);
	lv_obj_clear_flag(network->my_network_cont, LV_OBJ_FLAG_SCROLLABLE);



	network->my_network_label = lv_label_create(network->my_network_cont);
	lv_label_set_text(network->my_network_label, "我的网络");
	//lv_obj_move_after(my_network_label, network->wifi_sw_btn);


	// 创建容器
	network->my_network_obj = lv_obj_create(network->my_network_cont);
	lv_obj_set_size(network->my_network_obj, 280, 50);
	//lv_obj_move_after(network->my_network_cont, my_network_label);
	lv_obj_set_style_bg_color(network->my_network_obj, lv_color_hex(0xFFFFFF), 0);
	lv_obj_set_style_radius(network->my_network_obj, 12, 0);
	lv_obj_set_style_border_width(network->my_network_obj, 0, 0);



	network->other_network_label = lv_label_create(network->cont);
	lv_label_set_text(network->other_network_label, "其他网络");

	/*Create a list*/
	network->list = lv_list_create(network->cont);

	lv_obj_set_size(network->list, 280, LV_SIZE_CONTENT);
	lv_obj_set_flex_grow(network->list, 1);
	lv_obj_set_style_max_height(network->list, 240, 0);

	lv_obj_align(network->list, LV_ALIGN_TOP_LEFT, NETWORK_LIST_X, NETWORK_LIST_Y);
	lv_obj_set_style_pad_all(network->list, 0, 0);
	lv_obj_set_style_border_width(network->list, 0, 0);
	lv_obj_set_style_bg_opa(network->list, LV_OPA_100, 0);

	lv_obj_set_style_radius(network->list, 12, 0);
	lv_obj_set_style_bg_color(network->list, lv_color_hex(WIFI_LIST_BG_COLOR), 0); 
	lv_obj_set_style_pad_row(network->list, NETWORK_LIST_ROW_PAD, 0);
	lv_obj_set_style_pad_all(network->list, NETWORK_LIST_PAD_ALL, 0);
	lv_obj_set_style_pad_bottom(network->list, 10, 0);

	//	lv_obj_clear_flag(network->list, LV_OBJ_FLAG_SCROLLABLE); //清除滚动

	lv_obj_set_scrollbar_mode(network->list, LV_SCROLLBAR_MODE_OFF);





	return;
}


static void refresh_layout() {
	ESP_LOGI("refresh_layout","------------------A-------------------\r\n");
	if (network->wifi_sw_obj->state & LV_STATE_CHECKED)
	{
		ESP_LOGI("refresh_layout", "------------------B-------------------\r\n");
		if (network->list_item_num > 0) {
			ESP_LOGI("refresh_layout", "------------------C-------------------\r\n");
			if (network->other_network_label)lv_obj_clear_flag(network->other_network_label, LV_OBJ_FLAG_HIDDEN);
			if (network->list)lv_obj_clear_flag(network->list, LV_OBJ_FLAG_HIDDEN);
			if (network->my_network_label)lv_obj_clear_flag(network->my_network_label, LV_OBJ_FLAG_HIDDEN);
			if (network->my_network_cont)lv_obj_clear_flag(network->my_network_cont, LV_OBJ_FLAG_HIDDEN);
			
		}
		else {
			ESP_LOGI("refresh_layout", "------------------D-------------------\r\n");
			if (network->other_network_label)lv_obj_add_flag(network->other_network_label, LV_OBJ_FLAG_HIDDEN);
			if (network->list)lv_obj_add_flag(network->list, LV_OBJ_FLAG_HIDDEN);
			if (network->my_network_label)lv_obj_add_flag(network->my_network_label, LV_OBJ_FLAG_HIDDEN);
		}
	}
	else
	{
		ESP_LOGI("refresh_layout", "------------------E-------------------\r\n");
		lv_obj_add_flag(network->other_network_label, LV_OBJ_FLAG_HIDDEN);
		if (network->list)lv_obj_add_flag(network->list, LV_OBJ_FLAG_HIDDEN);
	}

	if (network->psw_cont) {
		ESP_LOGI("refresh_layout", "------------------F-------------------\r\n");
		lv_obj_set_parent(network->psw_cont, lv_scr_act());
		lv_obj_align(network->psw_cont, LV_ALIGN_BOTTOM_MID, 0, 0);
	}

	ESP_LOGI("refresh_layout", "------------------Z-------------------\r\n");
#if 0
	if (network->wifi_sw_btn)
	{
		lv_obj_move_to_index(network->wifi_sw_btn, 1);
	}
#endif

#if 0
	if (network->cellular_sw_btn) {
		lv_obj_move_to_index(network->cellular_sw_btn, lv_obj_get_child_cnt(network->cont) - 1);
	}
#endif

}


/**********************
*   GLOBAL FUNCTIONS
**********************/

void network_setting_exit(void)
{
	ESP_LOGI(TAG, "%s A", __FUNCTION__);
	savePrjGlobalParm();
	save_wifi_data();
	if (network != NULL) {
		if (network->timer) {
			lv_timer_del(network->timer);  network->timer = NULL;
		}
		if (network->ms_box != NULL)
		{
			lv_msgbox_close(network->ms_box);
			network->ms_box = NULL;
		}
		if (network->keyboard) lv_obj_del(network->keyboard);
		if (network->ms_box) lv_msgbox_close(network->ms_box);
		if (network != NULL)
		{
			lv_obj_del(network->cont);
			network_setting_uninit();
		}
	}
	else {
		ESP_LOGW(TAG, "ntwk has del!");
	}
	LV_LOG("%s,[%d] \r\n ", __FUNCTION__, __LINE__);

}



void network_setting_entry(void)
{

	network_setting_init();

	network_setting_constructor();

	int doesShowPlsWait = false;
#ifndef C_PC_SIM
	doesShowPlsWait = is_wifi_show_pls_wait_stat(60000);
#else
	doesShowPlsWait = network->wifi_sw_state && !sys_get_wifi_link_state();
	//doesShowPlsWait = true;
#endif
	ESP_LOGI(TAG, "showPlsWt %d", doesShowPlsWait);
	if (doesShowPlsWait) {
		network->wifi_sw_state = 1;
		network_waiting_scr_create();
	}
	else {
		update_netword_device_ui(NET_STATE_NOSTART);
	}


	//----------------------------------------------------------//
	return;
}


int isWifiStableState(void)
{
#ifndef C_PC_SIM
	if (getPrjNetworkSWstate() && !sys_get_wifi_link_state())
		return false;
	return true;
#else
	return true;
#endif
}

//================================//


