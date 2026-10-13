
#include <string.h>
#include <time.h>

#include "zb_base.h"
#include "lvgl_main.h"
#include <lvgl/lvgl.h>

#ifdef C_PC_SIM
#include <Windows.h>
#endif

#if 0
#else
#include <lvgl/lvgl.h>
#endif

#include "./utils/style_tz.h"
#include "./ex_widget/com_vol.h"
#include "./mainmenu/mainmenu_base.h"
#include "./data_parse/img_res.h"
#include "./utils/misc_utils.h"
#include "./utils/global_parm.h"
#include "./addressbook/contact_ap.h"
#include "./addressbook/contactAP_data.h"
#include "player/player.h"
#ifndef C_PC_SIM
#include <sys/ipc.h>
#include <sys/msg.h>

#include <fcntl.h>			 /* For O_* constants */
#include <sys/stat.h>		 /* For mode constants */
#include <unistd.h>
//#include <mqueue.h>


#include "../trd/tz_sys_lib/include/cvi_tz_sys.h"
#include "../trd/tz_audio_player/include/tz_audio_player.h"

#include <pthread.h>
#include <sched.h>
#endif



#include "../trd/ubus/include/tz_hal_quectel.h"
#ifndef C_PC_SIM
//mp3 playing
#include "LvR9/trd/tz_audio_player/include/tz_audio_player.h"



//4G
#include "LvR9/trd/ubus/include/ubus_uloop_receiver.h"
#include "LvR9/trd/tz_sys_lib/include/cvi_tz_sys.h"
#include "LvR9/trd/tz_sys_lib/include/mlog.h"
#endif

#if 0
extern const lv_img_dsc_t  g_logo_img_dsc_tbl[];
#endif

extern volatile bool  g_hasClockCorrectionByNtp;
extern void zb_initForNxCnctSyncTm(void);
extern bool zb_isTimeToSyncTime(void);
bool sim_need_update = false;

#if 1
#define SYS_TIME_SEC_MIN  1714100000L  // 系统时间戳的最小允许值 
#endif

#define USE_DEEP_SLEEP_AS_SHUTDOWN  1

#define SHOW_BG_PLAY_OP_DLG   0

//-------------------------

static const char * TAG = "lvgl_main:";

#include "./software_version.inc"

//-------------------------

#if 0
extern void aud_file_part_hook(music_play_status_t status);
extern int isFatInSdcardReady();
extern const char * app_get_hardware_version(void);

#endif


extern bool zb_is_wifi_connect();
extern bool sys_is_wifi_state_on(void);

int zb_get_time_init(void);

//------------------------- for alarm 

extern int32_t g_alarm_time_sec;

extern int doObjEvtCb(lv_obj_t * obj, lv_event_code_t evtCode, uint32_t parm);
extern void sys_sendKeyEvtToMainWinOrFocused(uint32_t keyVal);
extern bool lv_indev_keypad_is_wait_release(void);
extern void lv_indev_keypad_clear_wait_release(void);

void sys_sendKeyEvtToMainWin(uint32_t keyVal);
extern void lv_msgbox_ensure_be_del(void);

extern int Time_proximity_comparison_method(void);
extern int zb_getAlarmAudKindAndBookNo(int16_t * pwKind, int16_t * pwBookNo, int16_t * pwPlanNoForOM);
extern int zb_setAlarmDataInfByKindAndBookNo(int16_t dataKind, int16_t albumNo);
extern int zb_sys_update_next_alarm(void);
extern int correctAlarmPlayerParmIn(void);
extern int alarmPlayer_start(void);


extern void zb_alarmTimeProc(int16_t parmIn);
extern int zb_show_alarmPromptDlg(void);

extern int zb_show_bgPlayerDlg(void);

extern int zb_show_pwrOffConfirmDlg(void);
extern void zb_del_pwrOffConfirmDlg(void);
extern int is_pwrOffConfirmDlg_shown(void);

extern int sys_show_lowVoltageDlg(int16_t confirmCode, int16_t dlgType);

extern int sys_lowVoltDlg_getDuration(void);

//-------------------------


int example_lsnspk(lv_img_dsc_t ** ppImgDscAddr);
int example_dic_testA(void);

extern int json_test(void);

//------------------------------

typedef int (*func_zb_app_init)(void);  // return >= 0 mean success;
typedef int (*func_zb_app_handler)(lv_event_code_t evtCode, int param); // 返回值由具体消息约定; 


extern  int sendMsgToCurrentAp(lv_event_code_t evtCode, int param);


struct zb_app_info {
    func_zb_app_init zb_app_init;        // AP 初始化函数 
    func_zb_app_handler zb_app_handler;  // AP 事件处理函数 
     /* 说明: zb_app_handler 里需要处理的事件(消息): 
        . EVENT_FREE_AP_RES (AP资源销毁) 
        . EVENT_EX_CMD      (多用于辅助版面切换)
     */
};

//static void idleChkTimer(struct _lv_timer_t * tmr);
//------------------------------

#define SYNC_TIME_TRY_TIME_MAX_SEC  (60 * 2)

#define C_GLB_TEXT_BUF_LEN  200

#define LCD_DONT_AUTO_OFF_MARK  0x71

//------------------------------

#if 0
static QueueHandle_t g_mainQueue = NULL;
#else
extern bool lv_win32_quit_signal;
#endif

#ifndef C_PC_SIM
quectel_handler_t* state_handler;

CollectionService* service; //数据库表
#endif


// 其他模块不要直接修改该变量，只能通过 zb_setNewApNo();
static uint8_t g_zbApNo = C_ZB_INVALID_AP_NO;  // zb-AP 号, start from 1. 

static uint8_t g_zbApNoNew = C_ZB_INVALID_AP_NO;
static uint16_t g_zbApInPara;

static bool g_apHasReady;  // AP 是否已初始化(freeAp 后仍当作未初始化).

static int32_t g_apStartTmMs;

static bool g_1stApHasReady = false;

static int8_t g_msgboxSelNo;

static char * g_globalTxt;

char IMEI_ID[16];//imei号的id

static volatile lv_timer_t * g_timerForPwrOn = NULL;

static volatile enum zb_power_off_cause g_requestPowerOff = 0; 

player_ui_state_t g_player_ui_state = {
.home_play_btn = NULL,
.update_callback = NULL,
.is_home_active = false
};

//----------
#if 0
static SemaphoreHandle_t  g_bgTaskSema;
#endif
static volatile bool bg_task_run_flag = true;

volatile int32_t  g_userNoOpTimeCnt;  // 用户无操作累计时长, unit: ms 
volatile int32_t  g_pwrAutoOffTimeCnt; // 自动关机累计时长(自熄屏后计起), unit: ms
static int8_t  g_lcdDontAutoOffFlg;  // 其值为 LCD_DONT_AUTO_OFF_MARK 时,表示永不关机模式(此时自动关机参数失效) 


volatile bool g_scrOffFlag;  // true means screen off. 


static volatile int32_t g_1stNtpSyncTmSinceNetCnct;  // 网络连接后,第一次同步间的时刻 对应的自开机以来的时长 (unit: ms)

static volatile bool g_alarmTmReachFlag = false;
static bool g_alarmBeIgnored;

static bool g_reqAlmPrompt;

//----------
//static int16_t g_idleStatCnt;
static int8_t g_requestToSleep;

static volatile int8_t g_reqPromptLowVoltage; // 1 show lowBattery prompt, 2 means powerOff
volatile bool g_hasPromptLowVoltage;

//增ly
extern lv_obj_t* g_pCurMainWinObj;
extern lv_event_cb_t g_mainWinMiscEvtCb; 

//------------------------------

void esp_wifi_task_init(void);


//------------------------------------------------------

#define IS_STUDY_TYPE_AP(apNo)  (((apNo < 32) && (g_studyTypeApPartA_mask & (1<<apNo))) || (apNo>=C_ZB_AP_PLAYER_TYPE_START && apNo<=C_ZB_AP_PLAYER_TYPE_END)) 

static const uint32_t g_studyTypeApPartA_mask = ((1<<C_ZB_AP_PINYIN_STUDY)|(1<<C_ZB_AP_NO_STUDYWORD)|(1<<C_ZB_AP_UCC_A)|(1<<C_ZB_AP_PHONICS_A));

static const uint32_t g_blockKeyAp_mask = ((1<<C_ZB_AP_PRODUCT_TST)|(1<<C_ZB_AP_SETTING));

//------------------------------------------------------

#ifndef C_PC_SIM
static const struct blobmsg_policy resp_policy[7] = {
	[0] = {.name = "event", .type = BLOBMSG_TYPE_STRING },
	[1] = {.name = "phone_number", .type = BLOBMSG_TYPE_STRING },
	[2] = {.name = "signal_strength", .type = BLOBMSG_TYPE_INT32 },
	[3] = {.name = "inserted_state", .type = BLOBMSG_TYPE_INT32 },
	[4] = {.name = "oper_name", .type = BLOBMSG_TYPE_INT32 },
	[5] = {.name = "network_type", .type = BLOBMSG_TYPE_INT32 },
	[6] = {.name = "call_state", .type = BLOBMSG_TYPE_INT32 },
};
#endif


static void get_device_id_imei()
{
#ifndef C_PC_SIM
	int response_status = 0;

	printf("get_device_id_imei( B\r\n");
	response_status = state_handler->get_device_id(IMEI_ID);
	LV_LOG_USER("IMEI_ID:%s", IMEI_ID);
	printf("response_status:%d\n", response_status);
#endif
}



typedef struct {
	char event_msg[64];
	char phone_number[32];
	int signal_strength;
	int inserted_state;      //
	//char oper_name[32];    // 
	int network_type;   //
	int call_state;     //
} msg_data_t;
static msg_data_t* msg_data_stat = NULL;


extern void _mmn_update_statbar_sim_state(int state);
extern void _mmn_update_statbar_sim_power(int power);
extern void _mmn_update_statbar_ref();


static void event_msg_async_callback(void* data) {
	// 这里执行你的UI更新代码
	msg_data_t* msg_data = (msg_data_t*)data;
	if (!msg_data) {
		printf("msg_data is NULL\n");
		return;
	}


	printf("[event_msg_async_callback] message: %s\n", msg_data->event_msg);

	if (msg_data_stat == NULL) {
		msg_data_stat = (msg_data_t*)malloc(sizeof(msg_data_t));
		if (msg_data_stat == NULL) {
			printf("Failed to allocate memory for msg_data_stat\n");
			free(msg_data);
			return;
		}
	}

	
	if(strcmp(msg_data->event_msg, event_sim_inserted) == 0) {
		_mmn_update_statbar_sim_state(msg_data->inserted_state);
		printf("msg_data->inserted_state %d\n",msg_data->inserted_state);
		_mmn_update_statbar_sim_power(msg_data->signal_strength);


		_mmn_update_statbar_ref();
	}
	if(strcmp(msg_data->event_msg, event_sim_info) == 0) {
		
		printf("msg_data->signal_strength %d\n",msg_data->signal_strength);	
		_mmn_update_statbar_sim_power(msg_data->signal_strength);

		_mmn_update_statbar_ref();
	}

		
	if (strcmp(msg_data->event_msg, event_in_call) == 0)
	{
		bool stat = tbcd_contact_has_phone(msg_data->phone_number, &popup_contact);
		if (stat)
		{
			if (g_scrOffFlag) {	// if screen is off, trun on screen

				if (is_pwrOffConfirmDlg_shown()) {
					zb_del_pwrOffConfirmDlg();
				}
#ifndef C_PC_SIM
				sys_switchScreenOnOff(1);
#endif
			}
			printf("[event_msg_async_callback] enter in_call \n");
			memcpy(&calling_contact, &popup_contact, sizeof(TBCD_CONTACT_RECORD_ITEM));
			tbcd_calling_scr_init(EVENT_INCOMING);
			//tbcd_incoming_call_scr_init(true);
		}
		else
		{
			printf("is not contactlist\r\n");
			seed_phone_Nocall();
		}
		}
	else if (strcmp(msg_data->event_msg, event_in_hangup) == 0) //挂电话
	{
		printf("[event_msg_async_callback] enter in_hangup \n");
		//sprintf(msg_data_stat->event_msg, "");

		if (g_pContactAp.sos_state == SOS_CALL_ACTIVE) {
			// 非主动挂断，继续拨打下一个
			g_pContactAp.current_sos_index++;
			sos_call_next_number();
		}
		_tbcd_no_pickup_cb();
		free_calling_scr();
	}
	else if (strcmp(msg_data->event_msg, event_out_call) == 0) //这里用于接收电话事件判断
	{
		printf("call_state %d\n", msg_data->call_state);
		if (msg_data->call_state == 1)
		{
			free_calling_scr();
			tbcd_calling_scr_init(EVENT_ANSWER);
			//update_calling_timer_create();
			printf("show your timer for calling\n");
		}
	}

	//}
	free(msg_data); // 释放内存

	}

static int safe_strcpy(char* dest, const char* src, size_t dest_size) {
	if (!dest || !src || dest_size == 0) return -1;
	strncpy(dest, src, dest_size - 1);
	dest[dest_size - 1] = '\0';
	return 0;
}

static void handle_event(struct ubus_context* ctx, struct ubus_event_handler* ev,
	const char* type, struct blob_attr* msg) {
#ifndef C_PC_SIM
	struct blob_attr* tb[ARRAY_SIZE(resp_policy)]; 

	static bool stat_id = false;

	if (!stat_id)
	{
		stat_id = true;
		get_device_id_imei();
	}

	if (blobmsg_parse(resp_policy, ARRAY_SIZE(resp_policy), tb, blob_data(msg), blob_len(msg)) != 0) {
		fprintf(stderr, "Parse failed\n");
		return;
	}

	//char oper_name[32];
	const char* event_msg       = tb[0] ? blobmsg_get_string(tb[0]) : "";
	const char* phone_number    = tb[1] ? blobmsg_get_string(tb[1]) : "";



	msg_data_t* data = malloc(sizeof(msg_data_t));
	if (!data) {
		fprintf(stderr, "Memory allocation failed\n");
		return;
	}

	safe_strcpy(data->event_msg, event_msg, sizeof(data->event_msg));
	safe_strcpy(data->phone_number, phone_number, sizeof(data->phone_number));
	data->signal_strength = tb[2] ? blobmsg_get_u32(tb[2]) : 0;
	data->inserted_state = tb[3] ? blobmsg_get_u32(tb[3]) : 0;
	data->network_type = tb[5] ? blobmsg_get_u32(tb[5]) : 0;
	data->call_state = tb[6] ? blobmsg_get_u32(tb[6]) : 0;

	lv_async_call(event_msg_async_callback, data);
#endif
}

#ifndef C_PC_SIM
static struct ubus_event_handler obj_event_handler = {
	.cb = handle_event
};
#endif
static void ql_sigaction(int signo) {
	ubus_uloop_receiver_exit();
}

static int simstate;
static int level, rssi, sinr;
#ifndef C_PC_SIM

static void async_update_cb(void* data) {

	_mmn_update_statbar_sim_state(simstate);
   _mmn_update_statbar_sim_power(level);

	_mmn_update_statbar_ref();

}


void* check_sim_thread(void* arg) {
	(void)arg; 

	while (1) {

		int response_status;
		if (state_handler)
		{
			char dev[30];
			tz_get_imei(dev);
			if (strlen(dev)>0) {

				response_status = state_handler->get_sim_state(&simstate);
				printf("成功获取SIM状态: %d\n", simstate);
				printf("dev %s\n", dev);

				_mmn_update_statbar_sim_state(simstate);

				
				if (state_handler->get_signal_strength(&level, &rssi, &sinr) == 0) {
					printf("信号强度：等级%d，强度%d dBm\n", level, rssi);
					_mmn_update_statbar_sim_power(level);
				}
				lv_async_call(async_update_cb, NULL);
				
				break;
			}
			else {
				printf("获取SIM状态失败，错误码: %d，2秒后重试...\n", response_status);
			}
		}
		

		sleep(2);
	}

	return NULL;
}

// 启动线程的函数
void start_sim_check_thread() {
	pthread_t sim_check_thread;
	int ret = pthread_create(&sim_check_thread, NULL, check_sim_thread, NULL);
	if (ret != 0) {
		perror("创建SIM检查线程失败");
		return;
	}

	// 分离线程（自动回收资源）
	pthread_detach(sim_check_thread);
	printf("SIM状态检测线程已启动\n");
}
#endif



// 线程函数
static void* recv_phone_enent_hander(void* arg)
{
#ifndef C_PC_SIM
	signal(SIGINT, ql_sigaction);
	signal(SIGTERM, ql_sigaction);
	signal(SIGALRM, ql_sigaction);

	ubus_uloop_receiver_event_init(&obj_event_handler, "tz_quectel_notify");
#endif
	return NULL;
}


#ifndef C_PC_SIM
/** lcd backlight on off  
 *@param - status : 1  pwm on , 0 pwm off 
 */
void tz_lcd_backlight_switch(bool status)
{
	if (status) { // 亮屏 
		uint16_t bright = getPrjGlobalParm(ZB_PARM_SCR_BRIGHTNESS, 0, NULL);
		tz_set_backlight_onoff(1);
		ESP_LOGI(TAG, "turnScrOn %d", bright);
	} else { // 熄屏 
		tz_set_backlight_onoff(0);
		ESP_LOGI(TAG, "turnScrOff");
	}
}
#endif



uint8_t zb_getNewApNo(void) {
    return g_zbApNoNew;
}

uint8_t zb_getApNo() {
    return g_zbApNo;
}

uint16_t zb_getApInParm() {
	return g_zbApInPara;
}

void zb_clear_autoScreenOffCnt(void) {
    g_userNoOpTimeCnt = 0;
}

void zb_clear_autoPwrOffCnt(void) {
    g_pwrAutoOffTimeCnt = 0; 
}

int32_t zb_get_autoScreenOffCnt(void) {
	return g_userNoOpTimeCnt;
}

bool sys_is_screenOn(void) {
	return !g_scrOffFlag;
}


const char* get_imei_id(void) {
	static const char empty[] = "UNKNOWN";


	IMEI_ID[sizeof(IMEI_ID) - 1] = '\0';

	return (IMEI_ID[0] != '\0') ? IMEI_ID : empty;
}

/* 
 * @param statNew : 1 - screenOn, 2 - screenOff;  0 - switch, 0x80 mask means force
 */
#if 0
void sys_switchScreenOnOff(int statNew)
{
	bool scrOffFlagN = false;
	bool isForce = (statNew & 0x80 ? 1 : 0);
	ESP_LOGI(TAG, "swtScr A %d", statNew);
	statNew &= 0x7f;
	if (0 == statNew) {
		scrOffFlagN = !g_scrOffFlag;
	}
	else {
		if (1 == statNew) { // screenOn
			scrOffFlagN = false;
		}
		else {
			scrOffFlagN = true;
		}
		if (!isForce && (scrOffFlagN == g_scrOffFlag)) {
			ESP_LOGI(TAG, "scrOnoff no chg, %d", scrOffFlagN);
			return;
		}
	}
	g_scrOffFlag = scrOffFlagN;
	ESP_LOGI(TAG, "switch scrTo %hd", (short)scrOffFlagN);
#if SHOW_BG_PLAY_OP_DLG
	if ((!scrOffFlagN) && (get_player_backstage_state() >= 0)) {
		zb_show_bgPlayerDlg();
	}
#endif
	vol_box_del();

#ifndef C_PC_SIM
	tz_lcd_backlight_switch(!scrOffFlagN);
#endif

	if (scrOffFlagN) { // screenOff
		zb_clear_autoPwrOffCnt();
		//g_idleStatCnt = 0;
	}
	else { // turn screen on
		zb_clear_autoScreenOffCnt();
	}
	return;
}
#else
 /*
  * @param statNew : 1 - screenOn, 其他值忽略
  */
void sys_switchScreenOnOff(int statNew)
{
	if ((statNew & 0x7f) != 1) {  // 屏蔽强制标志位后检查实际值
		return;
	}

	ESP_LOGI(TAG, "swtScr A %d", statNew);
	bool isForce = (statNew & 0x80 ? 1 : 0);  // 解析强制标志

	// 目标状态始终为 false（屏幕打开）
	bool scrOffFlagN = false;

	// 非强制模式下，若状态未改变则跳过
	if (!isForce && (scrOffFlagN == g_scrOffFlag)) {
		ESP_LOGI(TAG, "scrOnoff no chg, %d", scrOffFlagN);
		return;
}

	// 更新全局状态
	g_scrOffFlag = scrOffFlagN;
	ESP_LOGI(TAG, "switch scrTo %hd", (short)scrOffFlagN);

#if SHOW_BG_PLAY_OP_DLG
	// 仅当屏幕打开时显示背景播放对话框
	if (get_player_backstage_state() >= 0) {
		zb_show_bgPlayerDlg();
	}
#endif

	vol_box_del();  // 删除音量框（无论原状态如何）

#ifndef C_PC_SIM
	// 打开背光（因 scrOffFlagN=false 对应屏幕打开）
	tz_lcd_backlight_switch(true);
#endif

	// 屏幕打开时清除自动关闭计数
	zb_clear_autoScreenOffCnt();
}
#endif




//----------------------------------------------

static int getSysAutoPwrOffSec(void)
{
	int16_t  pwrAutoOffMinute = getPrjGlobalParm(ZB_PARM_POWEROFF_FLAG, 0, NULL);
	if (pwrAutoOffMinute < 2) {
		pwrAutoOffMinute = 2;
	}
	else if (pwrAutoOffMinute > 60) {
		pwrAutoOffMinute = 60;
	}

	return 60 * pwrAutoOffMinute;
}


//-----------------------

void scrOffStatVolumeKeyProc(lv_indev_t * i, uint32_t keyCode)
{
#if 0
	music_play_status_t playStat = audio_stream_get_play_status();
	music_play_status_t playStatB = music_player_state_get();
	if ((MUSIC_STATUS_PLAYING == playStat) || (MUSIC_STATUS_PLAYING == playStatB)) {
		int diff = (LV_KEY_EX_VOLUME_DEC == keyCode ? -1 : 1);
		volume_incOrDec(diff, 0);
		lv_indev_wait_release(i);
	}
#endif
	return;
}

//-----------------------
/**
 * 系统事件预处理 
 * @return  1 means has consume this evt, stop processing this key.  
 * Note: 这里 evtCode 一律用 LV_EVENT_KEY (而不能用 LV_EVENT_KEY_STAGE)
 */
int sys_evt_pre_process(lv_event_code_t evtCode, int param) {

	int iFuncRet = 0;
	switch (evtCode) {
		case LV_EVENT_KEY: {
			if (LV_KEY_EX_VOLUME_INC == param || LV_KEY_EX_VOLUME_DEC == param) {
				if (g_scrOffFlag) { // when screenOff
				#ifndef C_PC_SIM
					TZ_PLAYSTATE_T playStat = get_play_audio_state();
					if (TZ_PLAYING == playStat) {
						int diff = (LV_KEY_EX_VOLUME_DEC == param ? -1 : 1);
						volume_incOrDec(diff, 0);
					}
				#endif
					return 1;
				}
				if (is_com_vol_shown())
					return 0;
				bool bStopProcessing = false;
				if (sendMsgToCurrentAp(C_EVENT_DOES_BLOCK_VOLUME_WIN, 0)) { // 查询当前AP 是否需要 volKey
					ESP_LOGI(TAG, "keyVol B");
					break;
				}
				ESP_LOGI(TAG, "keyVol C");
				if (is_pwrOffConfirmDlg_shown()) {
					zb_del_pwrOffConfirmDlg();
				}
				ESP_LOGI(TAG, "keyVol D");
				lv_com_vol(-1);
				bStopProcessing = true;
				return (bStopProcessing ? 1 : 0);
			}
			else if ((LV_KEY_EX_POWER|LV_EX_FLAG_KEY_LONG) == param) {
				ESP_LOGI(TAG, "pwrKeyLg %d", (int)g_scrOffFlag);
				if (!g_scrOffFlag) { // if scrren is on, then show onoff dlg.
					if (((1 << g_zbApNo) & g_blockKeyAp_mask) &&
						sendMsgToCurrentAp(C_EVENT_DOES_BLOCK_SYS_POWER_KEY, 2)) {
						return 0;
					}
					if (g_1stApHasReady )//&& (zb_get_time_ms_since_boot() > 3000)
					{
						ESP_LOGI(TAG, "pwrkey longPrs");
						zb_show_pwrOffConfirmDlg();
					}
				}
				else {	// if screen is off, trun on screen
					if (is_pwrOffConfirmDlg_shown()) {
						zb_del_pwrOffConfirmDlg();
					}
				#ifndef C_PC_SIM
					sys_switchScreenOnOff(1);
				#endif
				}
				return 1;
			}
			else if ((LV_KEY_EX_POWER|LV_EX_FLAG_KEY_SHORT) == param) {
				ESP_LOGI(TAG, "pwr keyPrs *----");
				if (is_pwrOffConfirmDlg_shown()) {
					zb_del_pwrOffConfirmDlg();
				}
				if (g_scrOffFlag) {
				#ifndef C_PC_SIM
					sys_switchScreenOnOff(1);
				#endif
					return 1;
				}
				if ((1 << g_zbApNo) & g_blockKeyAp_mask) {
					if (sendMsgToCurrentAp(C_EVENT_DOES_BLOCK_SYS_POWER_KEY, 1))
						return 0;
				}
			#ifndef C_PC_SIM
				sys_switchScreenOnOff(0);
			#endif
				return 1;
			} 
		    else if ((LV_KEY_EX_FUNC1 | LV_EX_FLAG_KEY_SHORT)==param){//这个是sos功能键
				//play_or_pause_func();
				ESP_LOGI(TAG, "click sos func key .\r\n");
                return 1;
		    }
			else if ((LV_KEY_EX_MEDIA_PLAY_PAUSE | LV_EX_FLAG_KEY_SHORT) == param) {

				play_or_pause_func();
				//play_or_pause_backstage_unit();
				ESP_LOGI(TAG, "play_or_pause_func .\r\n");
				return 1;
			}
			else if ((LV_KEY_EX_MEDIA_PREV | LV_EX_FLAG_KEY_SHORT) == param) {
				play_pre_unit();
				ESP_LOGI(TAG, "click prev music .\r\n");
				return 1;
			}
			else if ((LV_KEY_EX_MEDIA_NEXT | LV_EX_FLAG_KEY_SHORT) == param) {
				play_next_unit();
				ESP_LOGI(TAG,"click next music .\r\n");
				return 1;
			}
			break;
		}

		default:
			break;
	}
	return iFuncRet;
}


void zb_msgbox_sel_no_save(int8_t selNo) {
	g_msgboxSelNo = selNo;
}

int8_t zb_msgbox_sel_no_get(void) {
	return g_msgboxSelNo;
}

int zb_global_str_write(const char * pcText) {
	if (!pcText)
		return -1;
	if (!g_globalTxt) {
		g_globalTxt = (char *)heap_caps_malloc_dflt(C_GLB_TEXT_BUF_LEN);
	}
	g_globalTxt[0] = '\0';
	strncat(g_globalTxt, pcText, C_GLB_TEXT_BUF_LEN - 1);
	return 0;
}

int zb_global_str_read(char * pcBuf, int bufLen) {
	if (!pcBuf) 
		return -1;
	(*pcBuf) = '\0';
	if (bufLen < 2)
		return -3;
	if (!g_globalTxt) // if ptr is null, sent space str
		return 0;
	pcBuf[0] = '\0';
	strncat(pcBuf, g_globalTxt, bufLen - 1);
	return 0;
}

//--------------------------------------------------------------
#define ZB_SCR_LAYER_MAX  5  // zb 屏幕层的最大层数 


struct zb_screen_layer {  // zb 屏幕层 
	lv_group_t * m_inputGroup;
	lv_obj_t * m_mainWindowObj;
	lv_event_cb_t * m_winMiscEvtCb;
	int32_t   m_layerFlag;  // bit0 为1表示新的一层接受语音通知消息;     bit3 为1 表示新层是否为整屏 
};

//------------------------------
static int8_t g_zb_layerCnt = 0;  // zb 屏幕层数 
static struct zb_screen_layer g_screenLayerTbl[ZB_SCR_LAYER_MAX];

//------------------------------
/** 添加一个屏幕层 
 * @parm  parm 的值 LAYER_FLG_RCV_AUD_EVT ... 
 * @return 负值表示失败 
 */
int zb_group_push(lv_group_t * group, int16_t parm) { 
	if (0 == group) {
		ESP_LOGE(TAG, "grpPush nul!");
	}
	LV_ASSERT(group);
	if (g_zb_layerCnt >= ZB_SCR_LAYER_MAX)
		return -5;

	int8_t  layer = g_zb_layerCnt;
	struct zb_screen_layer stScreenLayer = { 0 };
	stScreenLayer.m_inputGroup = group;
	stScreenLayer.m_layerFlag = parm;
	stScreenLayer.m_mainWindowObj = g_pCurMainWinObj;
	stScreenLayer.m_winMiscEvtCb = g_mainWinMiscEvtCb;
	g_screenLayerTbl[layer] = stScreenLayer;

	g_zb_layerCnt++;
	g_pCurMainWinObj = NULL;
	g_mainWinMiscEvtCb = NULL;

	ESP_LOGI(TAG, "grpPush W");
	lv_indev_t * indev_key = lv_indev_get_by_type(LV_INDEV_TYPE_KEYPAD);
	ESP_LOGI(TAG, "grpPush Y");
	if (indev_key) {
		lv_indev_set_group(indev_key, group);
	}
	else {
		ESP_LOGI(TAG, "grpPush nulDev");
	}
	ESP_LOGI(TAG, "grpPush Z");
	return 0;
}

/** 弹出一个屏幕层 
 * @return 负值表示失败, >= 0 表示 group 总层数;
 */
int zb_group_pop(bool doesDelGroup) { 
	ESP_LOGI(TAG, "layerPop A, %hd", (short)g_zb_layerCnt);
	if (g_zb_layerCnt <= 1)
		return -1;

	if (doesDelGroup) {
		lv_group_t * poppedGroup = g_screenLayerTbl[g_zb_layerCnt - 1].m_inputGroup;
		lv_group_del(poppedGroup);   poppedGroup = NULL; // 删除最顶层的 inGroup 
	}

	g_zb_layerCnt--;

	int8_t  layerCnt = g_zb_layerCnt;
	struct zb_screen_layer stScrLayer = g_screenLayerTbl[layerCnt - 1];
	lv_group_t * groupCur = stScrLayer.m_inputGroup;

	lv_indev_t * indev_key = lv_indev_get_by_type(LV_INDEV_TYPE_KEYPAD);
	if (indev_key) {
		lv_indev_set_group(indev_key, groupCur);
	}
#if 1 // def C_PC_SIM
	lv_indev_t * indev_tp = lv_indev_get_by_type(LV_INDEV_TYPE_POINTER);
	if (indev_tp) {
		lv_indev_set_group(indev_tp, groupCur);
	}
#endif
	sys_setCurMainWinAndCb(stScrLayer.m_mainWindowObj, stScrLayer.m_winMiscEvtCb);
	ESP_LOGI(TAG, "layerPop Z");
	return g_zb_layerCnt;
}


bool zb_is_groupOnTopLayer(lv_group_t * group) {
	LV_ASSERT(group);
	if (g_zb_layerCnt <= 0)
		return false;
	return group == g_screenLayerTbl[g_zb_layerCnt - 1].m_inputGroup;
}


bool zb_screen_layer_is_full(void) {
	return (g_zb_layerCnt >= ZB_SCR_LAYER_MAX);
}


void zb_setMainWin_hook(lv_obj_t * mainWin, lv_event_cb_t miscEvtCb)
{
	LV_ASSERT(g_zb_layerCnt >= 1);
	g_screenLayerTbl[g_zb_layerCnt - 1].m_mainWindowObj = mainWin;
	g_screenLayerTbl[g_zb_layerCnt - 1].m_winMiscEvtCb = miscEvtCb;
}



/** 判断 AP 层界面是否被其他系统版面完全盖住 (是否被响闹之类全屏间版面完全盖住);
 */
bool zb_isApScreenFullCovered(void) {
	if (g_zb_layerCnt >= 2) {
		if (g_zb_layerCnt >= 3) {
			if (g_screenLayerTbl[3 - 1].m_layerFlag & LAYER_FLG_FULL_SCR) {
				return true;
			}
		}
		if (g_screenLayerTbl[2 - 1].m_layerFlag & LAYER_FLG_FULL_SCR) {
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------

/** 用于辅助发送 EVENT_EX_CMD 消息的一次性 timer.
 */
void zb_ex_cmd_timer_cb(lv_timer_t * timer)
{
	ESP_LOGI(TAG, "exCmdTmrCb A");
	int para = (int)timer->user_data;
	sendMsgToCurrentAp(EVENT_EX_CMD, para);
	lv_timer_del(timer);
}


void zb_set_alarm_tm_reach_flag(void) {
	ESP_LOGI(TAG, "setAlmReachFlg");
	g_alarmTmReachFlag = true;
}


static int apSimple_handler(lv_event_code_t msg, int param) {
#if 0
    switch ((int)msg) {
    case C_EVENT_FREE_AP_RES:
        break;
    case EVENT_EX_CMD:
        ESP_LOGI(TAG, "evtCmd");
        break;
    default:
        break;
    }
#endif
    return 0;
}


static int apDemo_Widgets_init(void) {
    lv_img_dsc_t  * pImgDsc = NULL;

    //example_dic_testA();
    //example_lsnspk(&pImgDsc);
    //json_test();
    //lv_example_btnmatrix_2();

    lv_example_label_1();
    //lv_example_checkbox_1();
    //lv_example_grid_1();
    // lv_example_msgbox_1();

    //lv_demo_widgets();
    // lv_example_slider_1();
    // lv_example_slider_3();
    // lv_example_dropdown_2();
    // lv_example_list_1();
    //lv_example_img_1a(pImgDsc);
    // lv_example_png_1();
    //lv_example_checkbox_1();
    //lv_example_table_1();
    //lv_example_keyboard_1();
    //lv_file_explorer_simple_test();
    // lv_demo_keypad_encoder();
    // main_menu_ap_init();
    return 0;
}


#include "list_of_ap.inc"

#define C_ZB_AP_NO_MAX      M_ARRAY_LEN(g_zb_appInfoTbl)

#define IS_VALID_ZB_AP_NO(apNo)  (((apNo) >= 1) && ((apNo) <= M_ARRAY_LEN(g_zb_appInfoTbl)))

//----------------------------------------------[]
const char * get_tlb_cur_version(void)
{
	return(CUR_VERSION);	
}

int get_tlb_cur_version_no(void)
{
	return(CUR_VERSION_NO);	
}

void zb_request_power_off(enum zb_power_off_cause cause) {
	ESP_LOGI(TAG, "req powOff %hd", (short)cause);
	g_requestPowerOff = cause;
}


//-----------------------------
void zb_requestSwitchApIn(uint8_t apNoNew, uint16_t exParm) {
    if (!IS_VALID_ZB_AP_NO(apNoNew)) {
        return;
    }
	del_dialog();
    if (apNoNew == g_zbApNo) {
        ESP_LOGW(TAG, "reqSwtAp, warn! same apNo %hd", (short)apNoNew);
        return;
    }

    if (!g_zb_appInfoTbl[apNoNew - 1].zb_app_init) {
        ESP_LOGW(TAG, "ap item is spc! No%d", apNoNew);
        return;
    }
    g_zbApNoNew = apNoNew;
    g_zbApInPara = exParm;
    ESP_LOGI(TAG, "reqSwitchAp %d", (int)apNoNew);
}


/* 设置取消自动关机(每次开机后需重新设置)，用于产品演示 
 */
void zb_set_lcdDontAutoOffMode(int16_t confirmCode)
{
    g_lcdDontAutoOffFlg = (confirmCode == 16127 ?  LCD_DONT_AUTO_OFF_MARK : 0);
    ESP_LOGI(TAG, "lcdDontAOff turn to %hd", (short)g_lcdDontAutoOffFlg);
    //------------------
#if 0
    int16_t brightness = 100;
    bsp_lcd_set_pwm_duty(brightness);
    setPrjGlobalParm(ZB_PARM_SCR_BRIGHTNESS, 2, &brightness);  // 同时亮屏调到最大 
#endif
}

bool zb_is_lcdDontAutoOffMode(void)
{
	return (LCD_DONT_AUTO_OFF_MARK == g_lcdDontAutoOffFlg);
}


static int zb_enterAp(uint8_t apNoNew) {
    if (apNoNew < 1 || apNoNew > M_ARRAY_LEN(g_zb_appInfoTbl)) {
        ESP_LOGW(TAG, "warn! invalid apNo %d", (int)apNoNew);
        return -1;
    }
    if (apNoNew == g_zbApNo) {
        ESP_LOGW(TAG, "warn! enter same apNo %d", (int)apNoNew);
        return -3;
    }
    if (!g_zb_appInfoTbl[apNoNew - 1].zb_app_handler) {
        ESP_LOGW(TAG, "enter ap fail, no hdl");
        return -5;
    }

    func_zb_app_init app_init = g_zb_appInfoTbl[apNoNew - 1].zb_app_init;
    if (app_init != NULL) {
        if ((*app_init)() >= 0) {
            g_zbApNo = apNoNew;
            g_apHasReady = true;
            ESP_LOGW(TAG, "enter ap ok, %d\n", apNoNew);
            g_apStartTmMs = zb_get_time_ms_since_boot();
            return 0;
        }
    }
    return -1;
}

static void  callFreeApRes(uint8_t zbApNo) {
    if (!IS_VALID_ZB_AP_NO(zbApNo)) {
        return;
    }
    func_zb_app_handler app_hdl = g_zb_appInfoTbl[zbApNo - 1].zb_app_handler;
    ESP_LOGI(TAG, "apEnt, ap freeRes A,%d", zbApNo);
    if (app_hdl) {
        (*app_hdl)(C_EVENT_FREE_AP_RES, 0);  // 给AP发消息，释放资源
    }
    lv_msgbox_ensure_be_del();
    if (g_alarmBeIgnored) {
        ESP_LOGI(TAG, "aft alm ignr, upd it");
        g_alarmBeIgnored = false;
        zb_sys_update_next_alarm();
    }
    g_apHasReady = 0;
    if (IS_STUDY_TYPE_AP(zbApNo)) {
        int32_t timeCur = zb_get_time_ms_since_boot();
        int timeDiffMs = (timeCur - g_apStartTmMs);
        if (timeDiffMs >= 3000) { // 只要在该学习AP 里停留有 3秒以上,（统计学习时长时)就算上它.
            int timeDiffMinute = (timeDiffMs + 500) / (1000 * 60);
            if (timeDiffMinute <= 0) {
                timeDiffMinute = 1;
            }
            int16_t studyTmMinute = getPrjGlobalParm(ZB_PARM_STUDY_TIME_MINUTE, 0, NULL);
            if (studyTmMinute < 0)
                studyTmMinute = 0;
            studyTmMinute += timeDiffMinute;
            if ((uint16_t)studyTmMinute >= 32767) {
                studyTmMinute = 32767;
            }
            setPrjGlobalParm(ZB_PARM_STUDY_TIME_MINUTE, sizeof(int16_t), &studyTmMinute);
        }
    }

    ESP_LOGI(TAG, "apEnt, ap freeRes Z");
}

int sendMsgToCurrentAp(lv_event_code_t evtCode, int param) {
    int zbApNo = g_zbApNo;
    if (!(zbApNo >= 1 && zbApNo <= sizeof(g_zb_appInfoTbl) / sizeof(g_zb_appInfoTbl[0]))) {
        return 0;
    }
    func_zb_app_handler app_hdl = g_zb_appInfoTbl[zbApNo - 1].zb_app_handler;
    if (!app_hdl) return 0;

    int ret = (*app_hdl)(evtCode, param);  // 给AP发消息 
    return ret;
}

#if 0

/** 向主线程(当前主窗体)发送通知消息. 
 * Note: 一般供子程里调用 
 */
int zb_postMsgToMainTask(uint16_t msgCode, int parm) {
	if (!g_mainQueue)
		return -1;
	lvgl_queue_command_t cmd;
	cmd.type = (lvgl_queue_type_t)msgCode;
	cmd.isDirectMsg = (msgCode & 0x8000 ? 0 : 1);
	cmd.status = parm;
	ESP_LOGI(TAG, "postMsg %hd, parm:%d", msgCode, parm);
	xQueueSendToBack(g_mainQueue, &cmd, (TickType_t)0);
	return 0;
}
#endif


#ifndef C_PC_SIM


#define MAX_MSG_SIZE 100

// 消息结构体
typedef struct {
    long msg_type;
    char msg_text[MAX_MSG_SIZE];
} Message;



//--------

typedef struct
{
    long mEvtId;      //消息类型，必须大于0
    int32_t mEvtParmW;
    int32_t mEvtParmL;
    char mExtra[12]; //消息内容
    
} MSG_T;

static int g_key_msgqueue_id = -1;

int sys_keyMsgQueue_create(void) 
{
    int iFuncRet = -1;
#if 1
    key_t key;
    int msgid;
    pid_t pid;
    MSG_T s_msg, r_msg;
 
    key = ftok("/tmp/boll", 'c');
    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid < 0) {
        ESP_LOGW(TAG, "msgget error\n");
    } else {
        ESP_LOGI(TAG, "msgget success msgid:%d\n", msgid);
        g_key_msgqueue_id = msgid;
        iFuncRet = 0;
    }
#endif
	//---------------
#if 0
    #define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
    unsigned int prio;
    int     c,flags;
    mqd_t   mqd;
    struct mq_attr  attr;
    flags = O_RDWR|O_CREAT;
    printf("create mqueue.\n");
	
	//mqd = mq_open("/userdata/boll", flags, FILE_MODE,NULL))
    if ((mqd = mq_open("/testmQueueA", O_RDWR | O_CREAT, 0666, NULL)) == -1)
    {
        ESP_LOGE(TAG, "mq_open() error %d", errno);
    }
	else {
		ESP_LOGI(TAG, "mq_open() ok.");
	}
	g_key_msgqueue_id  = mqd;
#endif
	return iFuncRet;
}

int sys_keyMsgQueue_sendMsg(MSG_T * msg)
{
	if (g_key_msgqueue_id < 0) {
		sys_keyMsgQueue_create();
		if (g_key_msgqueue_id < 0) {
			ESP_LOGE(TAG, "crt mq fail!");
			return -1;
		}
	}
	//ESP_LOG(TAG, "");
	int ret = msgsnd(g_key_msgqueue_id, msg, sizeof(MSG_T) - sizeof(long), 0);  // IPC_NOWAIT
	ESP_LOGI(TAG, "sendKey ret %d", ret);
	if (ret < 0) {
		ESP_LOGE(TAG, "sndErr %d", errno);
	}
	return 0;
}

void tz_asyn_battery_event_msg(bool bCharging, int lastPower) {
	printf("bat_msg: bCharging:%d lastPower:%d\n", bCharging, lastPower);
	bool _isCharging;
	int batteryPower;
	_isCharging = bCharging;
	int8_t batLevel = 0;
	if (lastPower >= 90) {
		batLevel = 4;
	}
	else if (lastPower >= 60) {
		batLevel = 3;
	}
	else if (lastPower >= 40) {
		batLevel = 2;
	}
	else if (lastPower >= 20) {
		batLevel = 1;
	}
	batteryPower = batLevel;
	_mmn_update_statbar_bat(_isCharging, batteryPower);
	_mmn_update_statbar_ref();

}
void tz_asyn_wifi_event_msg(bool connect, int signal_qua) {
	printf("wifi_msg: connect:%d lastPower:%d\n", connect, signal_qua);
	bool _isWifiConnected;
	int _wifiSignalStrength;
	_isWifiConnected = connect;
	if (signal_qua >= 90) {
		_wifiSignalStrength = 4;
	}
	else if (signal_qua >= 70) {
		_wifiSignalStrength = 3;
	}
	else if (signal_qua >= 50) {
		_wifiSignalStrength = 2;
	}
	else if (signal_qua >= 30) {
		_wifiSignalStrength = 1;
	}
	else {
		_wifiSignalStrength = 0;
	}
	_mmn_update_statbar_wifi(_isWifiConnected, _wifiSignalStrength);
	_mmn_update_statbar_ref();
}

/**
 * 使用者，实现 下发监听物理按键状态信息。
 */
void tz_asyn_key_event_msg(tz_key_id_t id, tz_key_action_t action)
{
	ESP_LOGI(TAG, "keyEvt %d, type %d", (int)id, (tz_key_action_t)action);

	MSG_T stMsg = { 0 };
	stMsg.mEvtId = LV_EVENT_KEY_STAGE;
	stMsg.mEvtParmW = action;
	stMsg.mEvtParmL = id;
	sys_keyMsgQueue_sendMsg(&stMsg);
}
#endif


void lvgl_message_deal(lvgl_queue_command_t msg) {
	ESP_LOGW(TAG , " msg.type =%d msg.status =%d   " , msg.type  , msg.status);

	lv_obj_t * curMainWinObj = g_screenLayerTbl[0].m_mainWindowObj;
	if (NTFY_AUDIO_STREAM_PLAY_STATUS == msg.type || NTFY_AUDIO_PLAYER_PLAY_STATUS == msg.type) {
		if (g_zb_layerCnt >= 2 && (g_screenLayerTbl[g_zb_layerCnt - 1].m_layerFlag & LAYER_FLG_RCV_AUD_EVT)) {

			lv_obj_t* mainWinTry = NULL;//sys_getCurMainWinObj();
			if (mainWinTry) {  // 如果当前层(第2层以上的)窗体 接收语音消息，则(把通知 msg)发给它 
				curMainWinObj = mainWinTry;
			}
		}
	}

#if 0
	if (C_EVENT_EX_NETWORK == (int)msg.type) {
		#if 0
		if (1 == msg.status) {  // wifi if connected 
			ESP_LOGI(TAG, "rcvNetConnect");

			zb_initForNxCnctSyncTm();
			g_1stNtpSyncTmSinceNetCnct = zb_get_time_ms_since_boot();
			time_t curTm = 0;
			time(&curTm);
			if (!g_hasClockCorrectionByNtp || (curTm < SYS_TIME_SEC_MIN)) { // time < year 2024 
				if (!zb_is_wifi_connect()) {
					ESP_LOGW(TAG, "but wifi NOT cnnt!");
				}
				else if (0 == ltslp_get_sleep_status()) {
					ESP_LOGI(TAG, "sync time A1");
					sys_time_sync_init_ex();
				}
				else {
					ESP_LOGW(TAG, "but in ltSlp!");
				}
			}
		}
		#endif
		return;
	}
#endif	
	if (msg.isDirectMsg) {
		if (curMainWinObj) {
			ESP_LOGI(TAG, "directMsg %hd, mainWin %p", msg.type, curMainWinObj);
			doObjEvtCb(curMainWinObj, (lv_event_code_t)msg.type, msg.status);
			return;
		}
	}

	switch(msg.type) {
#if 0
		case NTFY_AUDIO_STREAM_PLAY_STATUS:
			if (curMainWinObj) {
				if (MUSIC_STATUS_PLAYING == msg.status) { // sugaoyi 20240521 add for: start play at time point.
					aud_file_part_hook(msg.status);
				}
				doObjEvtCb(curMainWinObj, EVENT_EX_AUD_STREAM, msg.status); 
			}
			break;
		case NTFY_AUDIO_PLAYER_PLAY_STATUS:
			if (curMainWinObj) {
				doObjEvtCb(curMainWinObj, EVENT_EX_AUD_PLAYER, msg.status); 
			}
			break;
		case NTFY_AUDIO_RECORDER_STATUS:
			if (curMainWinObj) {
				doObjEvtCb(curMainWinObj, EVENT_EX_RECORD, msg.status); 
			}
			break;
		case NTFY_EVAL_STATUS:
			if (curMainWinObj) {
				doObjEvtCb(curMainWinObj, EVENT_EX_EVAL, msg.status); 
			}
			break;
		case NTFY_ASR_STATUS:
			if (curMainWinObj) {
				doObjEvtCb(curMainWinObj, EVENT_EX_ASR, msg.status); 
			}
			break;
#endif
		default:
			break;
	}
}


//--------------------------------------------
#ifndef C_PC_SIM

static pthread_mutex_t bgTask_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t bgTask_cond = PTHREAD_COND_INITIALIZER;



static volatile int8_t g_bgTaskFlg;


static void * devSvr_bg_task(void *arg) {
#define SCR_AUTO_OFF_TM_MIN   30
#define SCR_AUTO_OFF_TM_MAX  300
	
#define SYS_BAT_VAL_WARN   20 //  需要提示低电的电压 
#define SYS_BAT_VAL_LIMIT  10 //  最低允许的电池电压 

	int16_t voltagePrev = 60;
	g_bgTaskFlg = 1;
	ESP_LOGI(TAG, "bgTsk A");

	int32_t tm_prev = zb_get_time_ms_since_boot();

	while (1) {
		if (!bg_task_run_flag)
			break;

		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += 5;  // 设置5秒后为超时时间点

		pthread_mutex_lock(&bgTask_mutex);
		pthread_cond_timedwait(&bgTask_cond, &bgTask_mutex, &ts);
		pthread_mutex_unlock(&bgTask_mutex);
		if (!bg_task_run_flag)
			break;
		//ESP_LOGI(TAG, "bgTsk F");
		//----------------------

		int batLevelPct = 0, chargeStat = 0;
		int getBatRet = tz_get_battery_info(&batLevelPct, &chargeStat);
		//ESP_LOGI(TAG, "getBat ret %d, bat %d, chrg %d", getBatRet, batLevelPct, chargeStat);
		if (0 == getBatRet) {
			if (!chargeStat && (batLevelPct <= SYS_BAT_VAL_LIMIT)) { // battery is too low
				if (voltagePrev <= SYS_BAT_VAL_WARN) {
					if (!g_scrOffFlag && !g_hasPromptLowVoltage) {
						g_reqPromptLowVoltage = 1;
					}
				}

				if (voltagePrev <= SYS_BAT_VAL_LIMIT) {
					if (!g_scrOffFlag) { // show lowPwrDlg before powerOff
						g_reqPromptLowVoltage = 2;
					}
				}
				else {
					ESP_LOGW(TAG, "chk lowVoltg lmt once");
				}
			}
			else if (!chargeStat && (batLevelPct <= SYS_BAT_VAL_WARN)) {
				if (voltagePrev > SYS_BAT_VAL_WARN) {
					ESP_LOGW(TAG, "chk lowVoltg prmpt once");
				}
				else if (!g_scrOffFlag && !g_hasPromptLowVoltage) {
					g_reqPromptLowVoltage = 1;
				}
			}
			if (g_reqPromptLowVoltage) {
				ESP_LOGW(TAG, "lowVotg A, %hd", (short)g_reqPromptLowVoltage);
			}
			voltagePrev = (int16_t)batLevelPct;
		}
		else {
			ESP_LOGI(TAG, "fail to getBat!");
		}

		//----------------------
		int32_t tm_cur = zb_get_time_ms_since_boot();
		int tmOnceNap = (int)(tm_cur - tm_prev);
		if (tmOnceNap < 0) {
			tmOnceNap = 10;
		}
		if (g_lcdDontAutoOffFlg != LCD_DONT_AUTO_OFF_MARK) {
			if (!g_scrOffFlag) { // when screen is on
				if (0 == g_userNoOpTimeCnt) {
					g_userNoOpTimeCnt = 1000; // start from 1000 ms
				}
				else {
					g_userNoOpTimeCnt += tmOnceNap;
				}
			}
			else {  // when screen is off
				if (chargeStat) {  // sugy 20250430 按颜工描述,充电时不要自动关机 (硬件上不允许)
					g_pwrAutoOffTimeCnt = 0;
				}
				else {
#if 0 //05.29 取消自动熄屏（演示）
					if (0 == g_pwrAutoOffTimeCnt) {
						g_pwrAutoOffTimeCnt = 1000;
					}
					else {

						g_pwrAutoOffTimeCnt += tmOnceNap;

					}
#else
					g_pwrAutoOffTimeCnt = 0;
#endif
				}
			}
		}
		else { // 处于永不熄屏(演示)模式时 
			g_userNoOpTimeCnt = 0;
			g_pwrAutoOffTimeCnt = 0;
		}

		int16_t  scrAutoOffTm = getPrjGlobalParm(ZB_PARM_SCR_AUTO_OFF_TIME, 0, NULL);
		if (scrAutoOffTm < SCR_AUTO_OFF_TM_MIN) {
			scrAutoOffTm = SCR_AUTO_OFF_TM_MIN;
		}
		else if (scrAutoOffTm > SCR_AUTO_OFF_TM_MAX) {
			scrAutoOffTm = SCR_AUTO_OFF_TM_MAX;
		}

		int  autoOffSecCfg = getSysAutoPwrOffSec(); // 
		//ESP_LOGI(TAG, "cur noOpTm:%ld, cfgOffTm:%d, cfgScrOff:%hd", g_userNoOpTimeCnt, autoOffSecCfg, scrAutoOffTm);
		int32_t tmCur = time(NULL);
		//ESP_LOGI(TAG, "cur noOpTm:%ld, cfgOffTm:%d, pwrAutoOffCnt:%ld, curTime: %ld, apNo:%hd, chrg:%d", 
		//		g_userNoOpTimeCnt, autoOffSecCfg, g_pwrAutoOffTimeCnt, tmCur,
		//		(short)g_zbApNo, chargeStat);
		if (0) { // not in sleep stat.
			if (g_scrOffFlag) { // screen is off
				if (g_pwrAutoOffTimeCnt >= autoOffSecCfg * 1000) {
					zb_request_power_off(pwr_off_cause_auto_off);  // 
				}
			}
			else {  // screen is on.
				//if (g_userNoOpTimeCnt > scrAutoOffTm * 1000) {
					//ESP_LOGI(TAG, "scr autOff");
				//	sys_switchScreenOnOff(2);
				//}
			}
		}
		//----------------------
		tm_prev = tm_cur;
	}

	ESP_LOGI(TAG, "bgTsk Z");
	g_bgTaskFlg = 0;
	return NULL;
}

static create_svr_bg_task(void ) 
{
	pthread_attr_t attr;
	pthread_t threadId = 0;

	int iFuncRet = -1;
	do {
		if (g_bgTaskFlg) {
			iFuncRet = 0;  break;
		}
		if (pthread_attr_init(&attr) != 0) {
			ESP_LOGE(TAG, "pthread_attr_init err\n");
			iFuncRet = -5;	break;
		}
		if (pthread_attr_setstacksize(&attr, 1024*80) != 0) {
			ESP_LOGE(TAG, "Fail to set stack siz\n");
			iFuncRet = -7;	break;
		}

		int priorityL = sched_get_priority_min(SCHED_FIFO);
		int priorityH = sched_get_priority_max(SCHED_FIFO);
		ESP_LOGI(TAG, "prior L %d, H %d", priorityL, priorityH);

		struct sched_param stPar = { 0 };
		pthread_attr_getschedparam(&attr, &stPar);
		stPar.sched_priority = 5; // debug
		int setParRet = pthread_attr_setschedparam(&attr, &stPar);
		if (setParRet != 0) {
			ESP_LOGW(TAG, "setSched fail %d", setParRet);
		}
		if (pthread_create(&threadId, &attr, devSvr_bg_task, 0) < 0) { 
			printf("Error, fail to create reg thread\n");
			iFuncRet = -13;	break;
		} 

		pthread_attr_destroy(&attr);
		iFuncRet = 1;
	}while (0);

	ESP_LOGI(TAG, "crtBgTsk ret %d", iFuncRet);
	return iFuncRet;

}
#endif

//--------------------------------------------

#define PATH_TLB_IMG_RES  ZB_PRESET_DATA_DIR_WITH_SEP "img/img_a_tlb.bin"


void req_jump_to_alarm_player(void) {

	ESP_LOGI(TAG, "reqJmp bfr updNxAlm");
	zb_sys_update_next_alarm();

	uint8_t apNo = zb_getApNo();
	if (apNo != C_ZB_AP_MAINMENU) {
		g_reqAlmPrompt = true;
		zb_requestSwitchAp(C_ZB_AP_MAINMENU);
	} else {
		ESP_LOGI(TAG, "reqJmpToAlmPly c");
		lv_msgbox_ensure_be_del();
		alarmPlayer_start();
	}
	ESP_LOGI(TAG, "req_jmp_to_alm Z");
}

void zb_prj_img_res_load() {
	zb_img_file_res_t * pImgRes = zb_img_res_open(PATH_TLB_IMG_RES);
	if (!pImgRes) {
		ESP_LOGW(TAG, "fail to load img:%s", PATH_TLB_IMG_RES);
	}
}


#if 0
void power_on_logo_show() {
	lv_obj_t * lv_act = lv_scr_act(); 
	lv_obj_t * logo_imageA  = NULL, * logo_imageB = NULL;

	lv_obj_set_style_bg_color(lv_act, lv_color_white(), 0);
	const lv_img_dsc_t * img_dsc = g_logo_img_dsc_tbl;
	logo_imageA = lv_img_create(lv_scr_act());
	lv_obj_set_size(logo_imageA, img_dsc->header.w, img_dsc->header.h);
	lv_img_set_src(logo_imageA, img_dsc);
	lv_obj_align(logo_imageA, LV_ALIGN_TOP_MID, 0, 102);

	img_dsc = g_logo_img_dsc_tbl + 1;
	logo_imageB = lv_img_create(lv_scr_act());
	lv_obj_set_size(logo_imageB, img_dsc->header.w, img_dsc->header.h);
	lv_img_set_src(logo_imageB, img_dsc);
	lv_obj_align(logo_imageB, LV_ALIGN_BOTTOM_MID, 0, -18);
	lv_task_handler(); 
}

#else

//----------------------------------------------
static inline void bg_task_request_release(void) {
	bg_task_run_flag = false;
#ifndef C_PC_SIM
	int tryRet = pthread_mutex_trylock(&bgTask_mutex);
	if (tryRet == 0) {
		pthread_cond_signal(&bgTask_cond);  // 发出 reg 处理请求
		pthread_mutex_unlock(&bgTask_mutex);
		ESP_LOGI(TAG, "bgTsk reqRel ");
	}
	else {
		ESP_LOGW(TAG, "bgTsk reqFail %d", tryRet);
	}
#endif
}


#ifdef C_PC_SIM
static DWORD g_uiThreadId;

int zb_postMsgToMainTask(uint16_t msgCode, int parm) 
{
	BOOL ret = PostThreadMessage(g_uiThreadId, msgCode, 0, parm);
	return 0;
}
#endif


#if 0
lv_indev_t* lv_indev_get_by_type(lv_indev_type_t type) {
	/*if (LV_INDEV_TYPE_KEYPAD == type) {
		return indev_key;
	}
	else if (LV_INDEV_TYPE_POINTER == type) {
		return indev_pointer;
	}*/
	ESP_LOGI(TAG, "%s A", __func__);
	return NULL;
}
#endif


void app_main_re(void)
{
	int task_delay_ms	= 30 ; 
	lvgl_queue_command_t cmd = { 0 };

#ifndef C_PC_SIM
	zb_get_time_init();
#endif

#ifdef C_PC_SIM
	g_uiThreadId = GetCurrentThreadId();
#endif
	lv_obj_clean(lv_scr_act());

#ifndef C_PC_SIM
	create_svr_bg_task();

	lv_group_t * group = NULL;
	ESP_LOGI(TAG, "main A_");

	group = lv_group_get_default();
	if (!group) {
		ESP_LOGI(TAG, "main A 1 crt ");
		group = lv_group_create();
	}
	if (group) {
		ESP_LOGI(TAG, "main A 3, g:%p ", group);
		lv_group_set_default(group);
		ESP_LOGI(TAG, "main A 5");
		lv_indev_t * p_indev = lv_indev_get_by_type(LV_INDEV_TYPE_KEYPAD);
		ESP_LOGI(TAG, "main A 7");
		if (p_indev) {
			lv_indev_set_group(p_indev, group);
		}
		else {
			ESP_LOGI(TAG, "main A, nul indev");
		}
	}
	ESP_LOGI(TAG, "lvglMainLoop A3");
	zb_group_push(group, LAYER_FLG_FULL_SCR|LAYER_FLG_RCV_AUD_EVT);
	bool isJustBoot = true;
#endif

	ESP_LOGI(TAG, "main C");
	loadPrjGlobalParm(NULL);
	ESP_LOGI(TAG, "main D");
	load_wifi_data(NULL);
	ESP_LOGI(TAG, "main E");
	globalParmTakeEffect();
	ESP_LOGI(TAG, "main G");

#ifndef C_PC_SIM
	bool isWifiOn = sys_is_wifi_state_on();
	int16_t wifiSw = isWifiOn;
	ESP_LOGI(TAG, "wifiOn:%hd", wifiSw);
	setPrjGlobalParm(ZB_PARM_WIFI_SW, 2, &wifiSw);  // 同步drv里的 wifi 开关到全局变量 
#endif
	//-------
	zb_prj_img_res_load();

	ESP_LOGI(TAG, "main H");
	zb_group_push(lv_group_get_default(), LAYER_FLG_FULL_SCR|LAYER_FLG_RCV_AUD_EVT);
	ESP_LOGI(TAG, "main J");

#ifndef C_PC_SIM
	pid_t parentProcessId = getppid();
	ESP_LOGI(TAG, "ppid %d", (int)parentProcessId);
	if (parentProcessId >= 2) { // 是由 shell 命令启动的 
		sys_switchScreenOnOff(0x80|1);
	}
#endif
	styles_init();
	g_zbApNo = C_ZB_INVALID_AP_NO;
	int enterRet = zb_enterAp(C_ZB_AP_MAINMENU);
	if (enterRet >= 0) {
		g_1stApHasReady = true;
	}
	ESP_LOGI(TAG, "main L");


#ifndef C_PC_SIM

	if (!tz_bt_init_dbus_connection()) {
		ESP_LOGE(TAG, "BT DBus init failed!");
	}

	init_font_engine();
	tz_init_awinic_global(); //音频算法
	sys_keyMsgQueue_create();

	tz_reg_asyn_key_event();
	tz_reg_asyn_battery_event();
	tz_reg_asyn_wifi_event();
	ESP_LOGI(TAG, "main L");
#endif

#ifndef C_PC_SIM
	//ubug phone
	if (access("/etc/zlog_tz.conf", F_OK) != -1)
	{
		LV_LOG_USER("/etc/zlog_tz.conf A \r\n");
	}
	else
	{
		LV_LOG_USER("/etc/zlog_tz.conf B \r\n");
	}

	if (!state_handler)
	{
		state_handler = tz_hal_quectel_handler_new();
		if (!state_handler) {

			LV_LOG_USER("create state_handler error\r\n");
			sleep(2);
			state_handler = tz_hal_quectel_handler_new();
			if (!state_handler) {
				LV_LOG_USER("create state_handler error 2\r\n");
			}

		}
		LV_LOG_USER("state_handler: %p", state_handler);
		pthread_t thread_enent;
		pthread_create(&thread_enent, NULL, recv_phone_enent_hander, NULL);
		pthread_detach(thread_enent);
    }

	start_sim_check_thread();
#endif

	get_device_id_imei();
	while (1)
	{
		int task_delay_ms = 0;
		task_delay_ms = lv_task_handler();
		if (task_delay_ms > 100) {
			task_delay_ms = 100;
		}
		else if (task_delay_ms < 10) {
			task_delay_ms = 10 ;
		}
	#ifndef C_PC_SIM
		task_delay_ms = usleep(task_delay_ms * 1000);
	#else
		Sleep(task_delay_ms);
	#endif

		if (g_requestPowerOff) {
			bool bQuitLoop = false;
			do {
			#ifndef C_PC_SIM  // sugy 20250430 按颜工描述,充电时不要(自动/低电)关机 (硬件上不允许)
				if ((pwr_off_cause_low_battery == g_requestPowerOff) || (pwr_off_cause_auto_off == g_requestPowerOff)) {
					int batLevelPct = 0, chargeStat = 0;
					int getBatRet = tz_get_battery_info(&batLevelPct, &chargeStat);
					if (chargeStat) {
						g_pwrAutoOffTimeCnt = 0;
						zb_request_power_off(0);
						break;
					}
				}
			#endif	

				if (g_apHasReady) {
					callFreeApRes(g_zbApNo); // 释放旧AP的资源(让其做收尾动作).
					bQuitLoop = true;
					ESP_LOGI(TAG, "power off key, aft freeAP");
					break;
				}
				else {
					g_requestPowerOff = 0;
					ESP_LOGW(TAG, "want powerOff, but not ready");
				}
			} while (0);
			if (bQuitLoop)
				break;
		}
		else if (g_alarmTmReachFlag) {
			ESP_LOGI(TAG, "got almTmReachFlg");
			// 磨耳朵响闹的内容要播放了 
			bool needUpdNxAlarm = true;
			//[----- 
			correctAlarmPlayerParmIn();
			//]----- 
			if (C_ZB_AP_PRODUCT_TST == g_zbApNo) {
				ESP_LOGW(TAG, "alm when prodTstAp, skip");
				needUpdNxAlarm = false;
			}
			else {
				correctAlarmPlayerParmIn();
				//--- todo, 若 is_com_vol_shown() 则 del volDlg. 
				int ret = sendMsgToCurrentAp(EVENT_EX_AP_PAUSE, 0);
				alarmPlayer_start();
			}

			if (needUpdNxAlarm) {
				ESP_LOGI(TAG, "updNxAlm");
				g_alarmBeIgnored = false;
				zb_sys_update_next_alarm();
			}
			g_alarmTmReachFlag = false;
		}

		if (g_reqPromptLowVoltage) {
			if (sys_show_lowVoltageDlg(LOW_POWER_DLG_CONFIRM_CODE, g_reqPromptLowVoltage) >= 0) {
				g_hasPromptLowVoltage = 1;
			}
			g_reqPromptLowVoltage = 0;
		}

		uint8_t zbApNo = g_zbApNo;
		uint8_t apNoNew = g_zbApNoNew;
		if (apNoNew != C_ZB_INVALID_AP_NO) { // now need switch AP. 
			if (g_apHasReady) {
				callFreeApRes(zbApNo); // 释放旧AP的资源(让其做收尾动作).
			}
			int openedN = zb_getOpenedFileNum();
			ESP_LOGI(TAG, "%s mainExitAp %d, openedN =%d", g_p____bar, zbApNo, openedN);
			zb_enterAp(apNoNew);
			g_zbApNoNew = C_ZB_INVALID_AP_NO;
		}

	#ifndef C_PC_SIM  // for receive net、audio msg.
		MSG_T stMsg = { 0 };
		while (1) {
			int rcvRet = msgrcv(g_key_msgqueue_id, &stMsg, sizeof(stMsg) - sizeof(long), 0, IPC_NOWAIT);
			if (rcvRet <= 0)
				break;
			ESP_LOGI(TAG, "~~~msgKey %d %hd, %d", stMsg.mEvtId, stMsg.mEvtParmW, stMsg.mEvtParmL);

			if (LV_EVENT_KEY_STAGE == stMsg.mEvtId) {
				uint32_t keyMask = 0;
				bool bDone = false;

				if (g_scrOffFlag) {
					if (stMsg.mEvtParmL != tz_key_vol_down && stMsg.mEvtParmL != tz_key_vol_up) {
						sys_switchScreenOnOff(1); // turn on screen.
						lv_indev_keypad_wait_release();
						bDone = true;
					}
				}
				
				if (!bDone) {
					bool bSkip = false;
					if (tz_key_act_release == stMsg.mEvtParmW) {
						if (lv_indev_keypad_is_wait_release()) {
							bSkip = true;
						}
						else {
							keyMask = LV_EX_FLAG_KEY_SHORT;
						}
					}
					else if (tz_key_act_longpress == stMsg.mEvtParmW) {
						keyMask = LV_EX_FLAG_KEY_LONG;
					}

					if (!bSkip) {
						uint32_t keyVal = (stMsg.mEvtParmL | keyMask);
						ESP_LOGI(TAG, "bfrSysEvt val 0x%x", keyVal);
						int procRet = sys_evt_pre_process(LV_EVENT_KEY, keyVal);
						if (0 == procRet) {
							ESP_LOGI(TAG, "sendKey toMainWin");
							sys_sendKeyEvtToMainWin(keyVal);
						}
					}
				}
				if (tz_key_act_release == stMsg.mEvtParmW) {
					lv_indev_keypad_clear_wait_release();
				}
			}
		}
	#else
		MSG msg = { 0 };
		while (1) {
			BOOL peekRet = PeekMessage(&msg, NULL, 0, 0, 1);
			if (!peekRet)
				break;
			ESP_LOGI(TAG, "--~~get msg one %d", msg.message);  // todo;
			lvgl_queue_command_t cmd = { 0 };
			cmd.isDirectMsg = 1; // debug
			cmd.type = msg.message;
			cmd.status = msg.lParam;
			lvgl_message_deal(cmd);
		}
	#endif
		
	}

	ESP_LOGI(TAG, "mainLoop End A");
	bg_task_request_release();

	ESP_LOGI(TAG, "mainLoop End A3");
	savePrjGlobalParm();
	save_wifi_data();
	ESP_LOGI(TAG, "mainLoop End B");

#ifdef C_PC_SIM
	lv_obj_clean(lv_scr_act());
	lv_obj_clean(lv_layer_top());
	lv_obj_t * bgLabel = lv_obj_create(lv_scr_act());
	lv_obj_set_size(bgLabel, LV_PCT(100), LV_PCT(100));
	lv_obj_set_style_bg_color(bgLabel, lv_color_black(), 0);
	lv_obj_set_style_bg_opa(bgLabel, LV_OPA_COVER, 0);
	lv_obj_invalidate(lv_scr_act());
	lv_task_handler();
	lv_refr_now(NULL);
#endif

#ifndef C_PC_SIM
	usleep(50*1000);	
#else
	Sleep(80);
#endif

#ifndef C_PC_SIM
	collection_service_destroy(service);
#endif

	ESP_LOGI(TAG, "mainLoop End z%s", g_p____bar);
#ifndef C_PC_SIM
	system("poweroff"); //关机 
#endif
	while (1);
}
#endif


