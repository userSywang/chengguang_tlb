/*************************************************************************************************************
 *
 * function        : wifi test demo
 *
*************************************************************************************************************/
#ifndef C_PC_SIM
#include <string.h>
#include <sys/shm.h>
#include <pthread.h>


#include "../../utils/misc_utils.h"

#include "../../setting/net_connect.h"

#include "../../zb_base.h"


#include "./wifi_connect.h"
#ifndef C_PC_SIM
#include "../../../trd/tz_sys_lib/include/cvi_tz_sys.h"
#endif


//----------------------------------------------
/**
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */

//static int32_t g_wifi_lastConnectTm = 0; // 上次连接路由器的时刻, unit is ms.

static volatile bool g_wifi_task_state = false;  //线程生存状态。
static volatile bool g_wifi_task_run = false;
static volatile bool g_wifi_task_init_finished = false;
static volatile bool g_wifiConnected = false;
static volatile bool g_wifi_request_connect_flag = false;  //1-请求开始新的连接。 0 -- 停止连接

static volatile bool g_wifi_recent_task = false;  //连接最近wifi的线程状态
static volatile bool g_wifi_scan_flag = false;
static volatile bool g_wifi_scan_finished = false;

//static volatile SemaphoreHandle_t wifi_task_sem  = NULL;


//===================================================================
static wifi_recent_info_t * recent_wf_list;
static char ip_addr[18] = {0};		// ip 地址;

//===================================================================
wifi_recent_info_t * getPrjNetworkRecentList(void);

uint16_t  getPrjNetworkSWstate(void);
void esp_wifi_task_deinit(void);
//===================================================================


//===================================================================

static const char *TAG = "AP-WF";



bool sys_get_wifi_link_state(void) 
{
	tz_wifi_status_t stStat = { 0 };
	int ret = tz_get_wifi_status(&stStat);
	ESP_LOGI(TAG, "getWfStat ret:%d", ret);
	return stStat.link;  // non-std
}

bool sys_is_wifi_state_on(void) 
{
	tz_wifi_status_t stStat = { 0 };
	int ret = tz_get_wifi_status(&stStat);
	ESP_LOGI(TAG, "getWfStat ret:%d", ret);
	return stStat.on;
}


bool sys_get_wifi_scan_finished_state(void)
{	
    ESP_LOGI(TAG, "%s [%d]", __FUNCTION__,__LINE__);
	return(g_wifi_scan_finished);
}

bool get_esp32s3_wifi_connected_state(void)
{	
    ESP_LOGI(TAG, "%s [%d], g_wifiConnected: %d \n", __FUNCTION__,__LINE__, g_wifiConnected);

	return(g_wifiConnected);
}
//------连接是否超时------------//
bool get_esp32s3_wifi_connected_timeout(void)
{	
	return(false);
}


//---------------------------------------------------------------

char * get_esp32s3_wifi_get_ip(void)
{
	if(g_wifiConnected)
		return((char * )ip_addr);
	else
		return("0.0.0.0");
}


//-----------------保存已连接过的wifi-----------------------------------
//如果当前wifi没存储过，则存入并放在最前面，如果之前已存过，则把它移到最顶上。
//如果已存满，则最后一个丢掉；
void esp_wifi_save_to_recent(char* in_ssid, char* in_pass)
{
	int i = 0;
	int ps = 0;
	ESP_LOGI(TAG, "%s [%d]", __FUNCTION__,__LINE__);

	recent_wf_list = getPrjNetworkRecentList(); 

	for(i = 0; i < NETWORK_RECENT_LIST_MAX_NUM; i++)
	{
		if(strcmp(recent_wf_list[i].ssid, in_ssid) == 0)
		
		break;
	}
	
	if(i >= NETWORK_RECENT_LIST_MAX_NUM) // 如果以前没存过，放到第一个位置
	{
		i = NETWORK_RECENT_LIST_MAX_NUM-1; //最后一个
	}
	
	
	while(i > 0)
	{
		strlcpy(recent_wf_list[i].ssid, recent_wf_list[i-1].ssid, sizeof(recent_wf_list[i].ssid));	
		strlcpy(recent_wf_list[i].password, recent_wf_list[i-1].password, sizeof(recent_wf_list[i].password));	
			
		i--;
	}
	strlcpy(recent_wf_list[0].ssid, in_ssid, sizeof(recent_wf_list[i].ssid));	//放到第一个位置
	strlcpy(recent_wf_list[0].password, in_pass, sizeof(recent_wf_list[i].password));	
		
}

//===================================================================
//尝试连接最近连过的wifi
static void esp_connect_recent_wifi_task(void * arg)
{
	ESP_LOGI(TAG, "%s [%d]", __FUNCTION__,__LINE__);
}


//=========================================================
//	启动 wifi 
// @param  parm bit0 == 1 表示不启动 connect_recent_wifi_task 
//=========================================================
//scan_connect_
void esp_wifi_task_init_ex(int16_t parm)
{
	ESP_LOGI(TAG, "%s [%d]", __FUNCTION__,__LINE__);
	return;
}


//
void esp_wifi_task_init(void)
{
	ESP_LOGI(TAG, "%s [%d]", __FUNCTION__,__LINE__);
	return;
}

//-------------------------------------------------------[]

bool zb_is_wifi_connect() {
	return g_wifiConnected;
}

//------------------------------------------------------
#define BOLL_WIFI_OP_SCAN     0
#define BOLL_WIFI_OP_CONNECT  1

#define MY_WIFI_SCAN_STAT_STARTING  1
#define MY_WIFI_SCAN_STAT_RUNNING   2

static volatile int8_t g_wifiOpType;
static volatile int g_scanThreadFlg = 0;  // 1 means creating, 2 - running. 

static volatile int16_t g_scanResultNum = 0;
static volatile int64_t g_scanResultTmMSec;

static volatile wifi_recent_info_t g_wifiInfForCnct;

static tz_wifi_dev_t g_wifiResultList[NETWORK_DEFAULT_SCAN_LIST_SIZE];


static void * sys_wifi_scanOrConnect_thead(void *arg) {

	ESP_LOGI(TAG, "wfScan A");
	g_scanThreadFlg = MY_WIFI_SCAN_STAT_RUNNING;
	int devCnt = NETWORK_DEFAULT_SCAN_LIST_SIZE;
	g_scanResultNum = 0;

	tz_wifi_dev_t * pWifiResultList = heap_caps_malloc_dflt(sizeof(tz_wifi_dev_t) * NETWORK_DEFAULT_SCAN_LIST_SIZE);
	memset(pWifiResultList, 0, sizeof(tz_wifi_dev_t) * NETWORK_DEFAULT_SCAN_LIST_SIZE);
	
	switch (g_wifiOpType) {
		case BOLL_WIFI_OP_SCAN: {
			int scanRet = tz_wifi_scan(pWifiResultList, &devCnt);
			ESP_LOGI(TAG, "wfScan ret %d, devN %d", scanRet, devCnt);
			if (scanRet >= 0) { // success
				if (devCnt < 0) {
					ESP_LOGE(TAG, "wrong devN:%d", devCnt);
				}
				g_scanResultTmMSec = zb_get_time_ms_since_boot();
				ESP_LOGI(TAG, "wsScan done");
				for (int sn = 0; sn < devCnt; sn++) {
					ESP_LOGI(TAG, "wf No%2d %d %s",  sn, pWifiResultList[sn].signal_qua,  pWifiResultList[sn].wifi_name);
				}
				//todo, 去重
				memcpy(g_wifiResultList, pWifiResultList, sizeof(tz_wifi_dev_t) * devCnt);
				g_scanResultNum = devCnt;
				ESP_LOGI(TAG, "wfScan done E");
			}
			else {
				ESP_LOGE(TAG, "wsScan fail!");
			}
			g_wifi_scan_finished = true;
			break;
		}
		case BOLL_WIFI_OP_CONNECT: {
			tz_wifi_dev_t stWifiDev = { 0 };
			strlcpy(stWifiDev.wifi_name, g_wifiInfForCnct.ssid, sizeof(stWifiDev.wifi_name));
			strlcpy(stWifiDev.security, g_wifiInfForCnct.security, sizeof(stWifiDev.security));
			int cnctRet = tz_wifi_connect(&stWifiDev, g_wifiInfForCnct.password);
			ESP_LOGI(TAG, "cnctRet %d", cnctRet);
			break;
		}
		default:
			break;
	}

	if (pWifiResultList) {
		heap_caps_free(pWifiResultList);  pWifiResultList = NULL;
	}
	g_scanThreadFlg = 0;
	ESP_LOGE(TAG, "wsScan end");
	return NULL;
}


/* 
 * @param opType  0 scan, 1 connect 
 * @return  > 0 success; < 0 fail; 0 - no need;
 */
static int sys_wifi_request_scanOrConnect(int8_t opType, void * parmA, void * parmB, void *parmC)     // 原来为 esp32s3_wifi_scan_enable()
{
	pthread_attr_t attr;
	pthread_t threadId = 0;

	int iFuncRet = -1;
	do {
		if (g_scanThreadFlg) {
			iFuncRet = 0;  break;
		}
		// 如果 reg 线程未创建, 则创建.
		g_scanThreadFlg = MY_WIFI_SCAN_STAT_STARTING;
		if (pthread_attr_init(&attr) != 0) {
			ESP_LOGE(TAG, "pthread_attr_init err\n");
			iFuncRet = -5;  break;
		}
		if (pthread_attr_setstacksize(&attr, 1024*80) != 0) {
			ESP_LOGE(TAG, "Fail to set stack siz\n");
			iFuncRet = -7;	break;
		}

		g_wifiOpType = opType;
		switch (opType) {
			case BOLL_WIFI_OP_SCAN: {
				g_wifi_scan_finished = false;
				break;
			}
			case BOLL_WIFI_OP_CONNECT: {
				strlcpy(g_wifiInfForCnct.ssid, (const char *)parmA, sizeof(g_wifiInfForCnct.ssid));
				strlcpy(g_wifiInfForCnct.password, (const char *)parmB, sizeof(g_wifiInfForCnct.password));
				ESP_LOGI(TAG, "secrt: %s", parmC);
				strlcpy(g_wifiInfForCnct.security, (const char *)parmC, sizeof(g_wifiInfForCnct.security));
				break;
			}
			default:
				break;
		}
		if (pthread_create(&threadId, &attr, sys_wifi_scanOrConnect_thead, 0) < 0) {
			printf("Error, fail to create reg thread\n");
			iFuncRet = -9;	break;
		}
		// g_threadId = threadId;

		pthread_attr_destroy(&attr);
		iFuncRet = 1;
	}while (0);

	ESP_LOGI(TAG, "reqScn ret %d", iFuncRet);
	return iFuncRet;
}


int sys_wifi_request_scan(void)
{
	return sys_wifi_request_scanOrConnect(BOLL_WIFI_OP_SCAN, NULL, NULL, NULL);
}

int sys_wifi_request_connect(const char * pcSsid, const char * pcPassword, const char * pcSecurity)
{
	return sys_wifi_request_scanOrConnect(BOLL_WIFI_OP_CONNECT, pcSsid, pcPassword, pcSecurity);
}


int sys_wifi_has_scan_result(void)
{
	int iFuncRet = -1;
	do {
		if (g_scanThreadFlg) {
			iFuncRet = -3;  break;
		}

		int16_t resultNum = g_scanResultNum;
		if (resultNum <= 0) {  // no result.
			iFuncRet = -7;  break;
		}
		iFuncRet = 1;
	} while (0);
	
	return (iFuncRet > 0 ? 1 : 0);
}


int sys_wifi_result_is_not_expired(int periodMs)
{
	if (periodMs <= 0)
		return 0;
	if (0 == g_scanResultTmMSec)
		return 0;
	int64_t tmMsNow = zb_get_time_ms_since_boot();
	
	return ((tmMsNow >= g_scanResultTmMSec) && (tmMsNow < g_scanResultTmMSec + periodMs));
}


// @return > 0 means num of got, <= 0 means fail; 
int sys_wifi_read_scan_result(tz_wifi_dev_t * pWifiDevs, int countMax)
{
	int iFuncRet = -1;
	do {
		if ((NULL == pWifiDevs) || (countMax < 1)) {
			iFuncRet = -3;  break;
		}
		if (g_scanThreadFlg) { // the scanTask is running now.
			iFuncRet = -5;	break;
		}
		int16_t resultNum = g_scanResultNum;
		if (resultNum <= 0) {  // no result.
			iFuncRet = 0;	break;
		}
		if (resultNum > countMax) {
			resultNum = countMax;
		}
		memcpy(pWifiDevs, g_wifiResultList, sizeof(tz_wifi_dev_t) * resultNum);
		if (g_scanThreadFlg) {
			iFuncRet = -11;	break;
		}
		iFuncRet = resultNum;
	} while (0);

	if (iFuncRet < 0) {
		ESP_LOGW(TAG, "readScn fail %d", iFuncRet);
	}
	else {
		ESP_LOGI(TAG, "readScn ret %d", iFuncRet);
	}
	return iFuncRet;	
}

#endif

