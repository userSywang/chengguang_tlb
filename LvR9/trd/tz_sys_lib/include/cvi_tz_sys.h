#ifndef __CVI_TZ_SYS_H__
#define __CVI_TZ_SYS_H__ 

/*
 * cvi_tz_sys.h - define export management header file
 *
 * Copyright (C) 2025
 * version:1.0.1
 * Author: ly@tz
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

	//对应按键key event
	typedef enum
	{
		tz_key_unkown = 0,
		tz_key_f2 = 1,             //未启用
		tz_key_forward = 2,        //播放上一首
		tz_key_power = 3,          //电源power
		tz_key_vol_down = 4,       //音量减
		tz_key_vol_up = 5,         //音量加
		tz_key_home = 6,           //home
		tz_key_back = 7,           //播放下一首
		tz_key_sos = 8,            //SOS
		tz_key_playorpause = 9,    //播放or暂停
		// for DHP key event
		tz_key_typec_dhp_down = 11, //耳机音量减
		tz_key_typec_dhp_up = 12,   //耳机音量加
		tz_key_typec_dhp_pause = 13, //耳机暂停

	} tz_key_id_t;

	//对应按下，抬起event
	typedef enum
	{
		tz_key_act_release = 0,      //按下释放
		tz_key_act_press = 1,        //按下事件
		tz_key_act_multiple = 2,
		tz_key_act_longpress,        //长按下
		tz_key_act_ignore,
	} tz_key_action_t;

	typedef struct {
		bool on;
		bool link;
		bool internet_connect;
		bool airkiss_info_valid;
		char ssid[32];
		int  signal_qua;

	}tz_wifi_status_t;

	typedef struct {
		char wifi_name[64];
		char wifi_name_hex[256];
		char security[64];
		int signal_qua;
	}tz_wifi_dev_t;

	typedef struct {
		char mac[18];
		char sn_str[17];
		char dev_name[18];
		char version[18];
	}tz_system_info_t;



	/***************************** 物理按键注册下发*******************************/
	/**
	 * 物理按键监听 程序启动时调用一次。
	 */
	void tz_reg_asyn_key_event();

	/**
	 * 物理按键监听 释放监听。
	 */
	void tz_unreg_asyn_key_event();

	/**
	 * 使用者，实现 下发监听物理按键状态信息。
	 * @param  bCharging 充电状态：
	 * @param  lastPower 电量：
	 */
	extern void tz_asyn_key_event_msg(tz_key_id_t id, tz_key_action_t action);

	
	/***************************** 电池注册下发*******************************/
	/**
	 * 电池监听 程序启动时调用一次。
	 */
	void tz_reg_asyn_battery_event();

	/**
	 * 电池监听 释放监听。
	 */
	void tz_unreg_asyn_battery_event();

	/**
	 * 使用者，实现 下发监听电池状态信息。
	 * @param  bCharging 充电状态：
	 * @param  lastPower 电量：
	 */
	extern void tz_asyn_battery_event_msg(bool bCharging, int lastPower);

	/***************************** wifi注册下发*******************************/
	/**
	 * wifi监听 程序启动时调用一次。
	 */
	void tz_reg_asyn_wifi_event();
	/**
	 * wifi监听 释放监听。
	 */
	void tz_unreg_asyn_wifi_event();
	/**
	 * 使用者，实现 下发监听wifi状态信息。
	 * @param  connect wifi连接状态：true 连接 false 断开
	 * @param  signal_qua 信号强度:0-100
	 */
	extern void tz_asyn_wifi_event_msg(bool connect, int signal_qua);


	/***************************** 录音 功能*******************************/
	
	/**
	 * @description: 打开录音设备
	 * @return  0 success
	 */
	int tz_capture_open();

	/**
	 * @description: 获取录音pcm数据
	 * @param {char*} 传入cap_buffer[1024]
	 * @param {int *} cap_len 返回实际大小
	 * @return  0 success
	 */
	int tz_capture_pcm_read(char* cap_buffer, int* cap_len);

	/**
	 * @description: 关闭录音设备
	 * @return  0 success
	 */
	int tz_capture_close();

	/***************************** 播放 录音 相关 功能*******************************/
	/**
	* @description: 设置播放或者录音的声音增益大小
	* @param {int} spk_or_cap 播放传入AUDIO_GAIN_SPK，录音传入AUDIO_GAIN_CAPTURE
	* @param {int} gain 最大值0-23
	* @return {int} 0 成功
	*/
	int tz_set_audio_volume(int spk_or_cap, int gain);

	/**
	* @description: 设置喇叭开启或关闭
	* @return {int} 0 关闭 1 开启
	*/
	int tz_spk_en_ctrl(int on);


	/***************************** 系统 功能*******************************/

	/***************************** 系统 -wifi 功能*******************************/
	/**
	* @description: wifi 开关
	* @on  1:开 0：关
	*/
	int tz_set_wifi_onoff(bool on);
	/**
	* @description: wifi 状态
	* @wifi_status {*}
	*/
	int tz_get_wifi_status(tz_wifi_status_t* wifi_status);
	/**
	* @description: wifi 获取wifi列表
	* @wifi_list  wifi列表
	* @dev_count  wifi列表个数
	*/
	int tz_wifi_scan(tz_wifi_dev_t wifi_list[64], int* dev_count);
	/**
	* @description: wifi 连接
	* @wifi_dev  wifi配置
	* @password  wifi列表个数
	*/
	int tz_wifi_connect(tz_wifi_dev_t* wifi_dev, char* password);
	/**
	* @description: wifi 移除
	* @wifi_dev  wifi列表
	*/
	int tz_wifi_remove(tz_wifi_dev_t* wifi_dev);

	/***************************** 系统 设置 功能*******************************/
	/**
	* @description: 是否开启屏幕背光
	* @onoff  1;开启 0;关闭
	*/
	int tz_set_backlight_onoff(int onoff);
	/**
	* @description: 设置屏幕亮度
	* @brightness 0-255 
	*/
	int tz_set_lcd_brightness(int brightness);
	/**
	* @description: 获取屏幕亮度
	* @brightness 0-7
	*/
	int tz_get_lcd_brightness(int* brightness);
	/**
	* @description: 设置关闭屏幕时间
	* @seconds  设置单毫秒s
	*/
	int tz_set_screenoff_time(int seconds);
	/**
	* @description: 设置获取屏幕时间
	* @seconds  设置单毫秒s
	*/
	int tz_get_screenoff_time(int* seconds);
	/**
	* @description: 获取系统信息
	*/
	int tz_get_system_info(tz_system_info_t* sys_info);
	/**
	* @description: 获取电池状态
	* @power               电量
	* @charger_insert      充电状态
	*/
	int tz_get_battery_info(int* power, bool* charger_insert);
	/**
    * @description: 恢复出厂设置
    * @reboot_sec 等待重启时间：秒               
    */
	void tz_factory_reset(int reboot_sec);


	/***************************** ota升级 功能*******************************/
	/**
	* @description: 查询是否有更新
	* @check 是否更新
	* @new_version 版本号
	* @update_info 更新内容
	* @return 0 正常
	*/
	int tz_parse_ota_check(int* check, char* new_version, char* update_info);
	/**
	* @description: 查询更新固件大小
	* @filesize 文件大小
	*/
	int tz_parse_ota_filesize(int* filesize);
	/**
	* @description: 下载ota状态
	* @dlstatus 1完成下载 0下载中
	*/
	int tz_parse_ota_dlstatus(int* dlstatus);
	/**
	* @description: 下载ota进度
	* @progress 0-100
	*/
	int tz_parse_ota_progress(int* progress);
	/**
	* @description: 查询下载程序是否运行 /usr/bin/ota_download_app &
	* @return 1 在运行，0不在运行
	*/
	int tz_is_ota_process_running();


	/***************************注意：extern 用户实现，在LVGL界面显示需跳转到LVGL主线程********************/


#ifdef __cplusplus
}
#endif

#endif
