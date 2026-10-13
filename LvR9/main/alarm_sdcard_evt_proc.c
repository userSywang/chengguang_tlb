/*
	8563 rtc test demo 
*/

#include <time.h>
//#include "key_driver.h"
#include "../main/include/key_driver.h"
#include "../main/include/app_rtc.h"



#include "./data_parse/img_res.h"
#include "./zb_base.h"
#include "./utils/misc_utils.h"


#include "zb_base.h"
#include "lvgl_main.h"


extern void zb_prj_img_res_load(void);

extern void zb_set_alarm_tm_reach_flag(void);


extern volatile int32_t  g_userNoOpTimeCnt;
extern volatile int32_t  g_pwrAutoOffTimeCnt;


extern int32_t g_alarm_time_sec;

//----------------------------------------------

static const char *TAG = "alarm_sdcard";

static volatile bool g_isFsInSdOk = false;


/** 判断 TF 卡里的文件系统是否已加载 ok 
 * return: 1 - ok, 0 not ready 
 */
int isFatInSdcardReady() {
	return g_isFsInSdOk;
}

void ensureBaseFolder(void) {
	if (!zb_isFileExist(GLB_PARAM_PATH)) {
		zb_mkdir((GLB_PARAM_PATH), 0777);
	}
	return;
}


static void alarm_sdcard_callback_cb(int status, void *user_data ) {
	ESP_LOGW(TAG , "  alarm_sdcard_callback_cb status= %d  \n" , status);
	switch(status){
		case ALARM_TRIGGER_EVENT: {
		#if PRJ_RTC_TYPE == PRJ_RTC_TYPE_USE_EXT_IC
			ESP_LOGI(TAG, "alarm evt ~~~~~, %ld", g_alarm_time_sec);
			long tmCur = time(NULL);
			int tmDiff = (tmCur - g_alarm_time_sec);
			if (tmDiff < 0) {
				tmDiff = -tmDiff;
			}
			if (tmDiff > 60) {
				app_rtc_clear_status();
				ESP_LOGW(TAG, "alarm evt, err trig,diff %d", tmDiff);
				break;
			}
			zb_set_alarm_tm_reach_flag(); // send msg to mainTask for do alarm thing.
		#endif
			app_rtc_clear_status();
			break;
		}
		case SDCARD_INSERT_EVENT:
#if 0
			ESP_LOGI(TAG, "tf in evt");
			g_userNoOpTimeCnt = 0;
			g_pwrAutoOffTimeCnt = 0;
			vTaskDelay(pdMS_TO_TICKS(200));
			int iRet = esp32s3_sdcard_init();
			ESP_LOGI(TAG, "sd init ret %d", iRet);
			if (0 == iRet) {
				g_isFsInSdOk = true;  // todo, should check 
				zb_prj_img_res_load();

				ensureBaseFolder();
			}
			else {
				ESP_LOGI(TAG, "fail to sdcrdInit %d", iRet);
			}
#endif
			break;
		case SDCARD_REMOVE_EVENT:
			ESP_LOGI(TAG, "tf out evt A");
			g_userNoOpTimeCnt = 0;
			g_pwrAutoOffTimeCnt = 0;
			bool isFsInSdBak = g_isFsInSdOk;
			g_isFsInSdOk = false;

			if (isFsInSdBak) { // is real tf out evt (not no TF card when pwrOn).
				zb_record_request_stop();
				music_player_stop();
				audio_stream_stop_play();
				zb_img_res_close((zb_img_file_res_t *)-1);
				uint8_t apNo = zb_getApNo();
				if ((apNo != C_ZB_AP_MAINMENU) && (apNo != C_ZB_AP_PRODUCT_TST)) {
					ESP_LOGI(TAG, "req pwrOff S");
					zb_request_power_off(pwr_off_cause_key_power);
				}
			}
			//esp32s3_sdcard_deinit();
			break;
	}
}


void alarm_sdcard_event_reg(void * queue) {
	ESP_LOGI(TAG, "almTfEvtReg");
	//set_alarm_sdcard_event_callback(alarm_sdcard_callback_cb , queue);
}

