#ifndef _WIFI_CONNECT_H_
#define _WIFI_CONNECT_H_

#include <stdint.h>
#include <stdbool.h>

#ifndef C_PC_SIM
#include "../../../trd/tz_sys_lib/include/cvi_tz_sys.h"
#endif

//--------------------------------

/*********************
 *      DEFINES
 *********************/
#define NETWORK_DEFAULT_SCAN_LIST_SIZE 	(25)		//扫描选择最大数量
//#define NETWORK_LIST_ITEM_MAX_NUM		30		//最大数量
#define NETWORK_RECENT_LIST_MAX_NUM		5		//最大数量


#ifdef C_PC_SIM
typedef struct {
	char wifi_name[64];
	char security[64];
	int signal_qua;
	uint8_t	lock;	//是否有密码
} tz_wifi_dev_t;
#endif


typedef struct {
	char  wifi_name[64];  // wifi 设备名称 
	int8_t 	signal_qua;	//信号强度 ,  0~100 越大越强 
	uint8_t	lock;	//是否有密码
} network_wifi_info_t;


typedef struct {
#ifdef LV_OBJ_H
	lv_obj_t * obj;
#else
	void * obj;
#endif
	network_wifi_info_t * info; //当前选择的wifi
	uint8_t	rssi_level;			//信号级别
	int8_t 	conn_status;	//是否连接；
} network_obj_list_t;


//------已连接成功过的记录信息-------
typedef struct {
	char  ssid[64];			//名称
	char  password[64];		//password
	char  security[64];
} wifi_recent_info_t;


/*
 */
extern void esp_wifi_task_init(void);
extern void esp_wifi_task_init_ex(int16_t parm);

extern bool sys_get_wifi_link_state(void);
extern bool sys_is_wifi_state_on(void);



bool sys_get_wifi_scan_finished_state(void);




//-------------------------------------------------------------

/**
 * @return  > 0 success; < 0 fail; 0 - no need;
 */
int sys_wifi_request_scan(void);     // 原来为 esp32s3_wifi_scan_enable()


/**
 * @return > 0 means has result.
 */
int sys_wifi_has_scan_result(void);


/**
 * @return > 0 means num of got, <= 0 means fail; 
 */
int sys_wifi_read_scan_result(tz_wifi_dev_t * pWifiDevs, int countMax);


/**
 * @return  > 0 success; < 0 fail;
 */
int sys_wifi_request_connect(const char * pcSsid, const char * pcPassword, const char * pcSecurity);


#endif

