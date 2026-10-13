
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <lvgl/lvgl.h>
#include "../utils/misc_utils.h"

#ifndef C_PC_SIM  //播放音乐文件
#include "../include/audio_stream.h"
#include "../include/audio_media_def.h"
#endif


//---------------------------------------------

const char * TAG = "spcFn";

int getAlarmDatumPath(int step, char * pcPath, int bufLen)
{
    LV_ASSERT(pcPath);
    ESP_LOGI(TAG, "getAlmDatPath A");
	(*pcPath) = '\0';
	return 0;
}

void setParmForAlarmPlayer(void) {
	ESP_LOGI(TAG, "%s A", __func__);
	return;
}

/**  更新最近一次响闹设置,使其生效(可触发响闹事件).
 */
int zb_sys_update_next_alarm(void) {
	ESP_LOGI(TAG, "%s A", __func__);
	return 0;
}

//响闹后调用此函数关闭
//对指定(一定是一次性的)响闹计划项的关闭
void turnOffLastAlarmForOnceMode(void) {
	ESP_LOGI(TAG, "%s A", __func__);
	return;
}

/*
 * @return not zero means be changed.
 */
int correctAlarmPlayerParmIn(void) {
	ESP_LOGI(TAG, "%s A", __func__);
	return 0;
}

/** 打开响闹播放界面 
 */
int alarmPlayer_start(void) {
	ESP_LOGI(TAG, "%s A", __func__);
	return -1;
}



/**
 * @brief 获取后台播放器状态
 * @return -1 :表示没有后台播放
 *         1，表示播放状态
 *         2，表示暂停状态
 */
//int get_player_backstage_state(void)
//{
//	ESP_LOGI(TAG, "%s A", __func__);
//	return -1;
//}

/**
 * @brief 后台播放或暂停
 * @return
 *          1：表示播放状态
 *          2：表示暂停状态
 */
//int play_or_pause_backstage_unit(void)
//{
//	ESP_LOGI(TAG, "%s A", __func__);
//	return 2;
//}

#if 0
/**
 * @brief 获取后台播放单元的名字
 * @return 后台有播放返回播放的歌曲名称，没有后台播放返回NULL
 */
char * get_backstage_play_unit_name(void) {
	ESP_LOGI(TAG, "%s A", __func__);
	return NULL;
}
#endif

#if 0
void network_setting_exit(void) {
	ESP_LOGI(TAG, "netwrkStgExt A");
	return;
}
#endif

int prp_loginScr_init(int extra) {
	ESP_LOGI(TAG, "%s A", __func__);
	return -1;
}

void prp_loginScr_deinit(void) {
	ESP_LOGI(TAG, "%s A", __func__);
	return;
}

#if 0
void network_setting_entry(void) {
	ESP_LOGI(TAG, "%s A", __func__);
	return;
}

void lv_msgbox_ensure_be_del(void)
{
	ESP_LOGI(TAG, "%s A", __func__);
}
#endif

int64_t esp_timer_get_time(void)
{
	ESP_LOGI(TAG, "%s A", __func__);
	return clock();
}
void zb_record_request_stop(void)
{
	ESP_LOGI(TAG, "%s A", __func__);
	return;
}

#if 0
int audio_stream_stop_play(void)
{
	ESP_LOGI(TAG, "%s A", __func__);
	return 0;
}
#endif

#ifndef C_PC_SIM
int music_player_set_play_uri(const char *uri, music_source_type_t type)
{
	ESP_LOGI(TAG, "%s A", __func__);
	return 0;
}

int music_player_destroy()
{
	ESP_LOGI(TAG, "%s A", __func__);
	return 0;
}
#endif

#ifdef C_PC_SIM
int music_player_stop(void)
{
	ESP_LOGI(TAG, "%s A", __func__);
	return 0;
}
#endif

void app_rtc_clear_status(void)
{
	ESP_LOGI(TAG, "%s A", __func__);
}
//void alarm_sdcard_event_reg(void* queue)
//{
//	ESP_LOGI(TAG, "%s A", __func__);
//}
int  set_alarm_sdcard_event_callback(void* callback, void* user_data)
{
	ESP_LOGI(TAG, "%s A", __func__);
	return 0;
}
int esp_wifi_sta_get_rssi(int* rssi)
{
	ESP_LOGI(TAG, "%s A", __func__);
	if (rssi) {
		(*rssi) = 0;
	}
	return 0;
}
 /*bool zb_is_wifi_connect()
{
	 return 0;
}*/
 int get_battery_voltage_charge(int* voltage, int* charge_status)
 {
	 if (voltage) {
		 (*voltage) = 3900;
	 }
	 if (charge_status) {
		 (*charge_status) = 0;
	 }
	 return 0;
 }

 	

#ifdef C_PC_SIM
int sys_wifi_request_scan(void)
{
	return 0;
}
#endif


//当前下载任务是否成功
bool http_download_ret(void)
{
	return	1;
}

//当前下载任务是否结束
bool http_download_finised_state(void)
{
	return 1;
}





