#ifndef TZ_BT_LIB_H
#define TZ_BT_LIB_H

/*
 * tz_bt_lib.h - define export management header file
 *
 * Copyright (C) 2025
 * version:1.0.0
 * Author: ly@
 */


#include <stdbool.h>
#include <time.h>
//#include <dbus/dbus.h>

#ifdef __cplusplus
extern "C" {
#endif

	//指令bluetoothctl ：power on/off ：scan on/off ：info mac地址 ：quit

	// 定义连接蓝牙设备信息结构
	typedef struct {
		char mac[18];
		char name[256];
	} TZ_BT_CONECTED_T;

	//蓝牙设备信息
	typedef struct {
		char mac[18];
		char name[256];
		bool paired;
		bool connected;
	} TZ_BT_PAIRED_T;

	//蓝牙设备详细信息
	typedef struct {
		char mac[18];           // MAC地址
		char name[256];         // 设备名称
		int rssi;               // 信号强度
		unsigned int device_class; // 设备类代码
		char uuids[512];        // 服务UUID列表
		char icon[64];          // 设备图标类型
		bool connected;         // 连接状态
		bool is_audio;          // 是否为音频设备
	} TZ_BT_DEVICE_INFO;


/**
* @description: 初始化dbus连接
* @return      成功
*/
bool tz_bt_init_dbus_connection();

/**
* @description: 释放 DBus 连接
*/
void tz_bt_release_dbus_connection();

/**
* @description: Bluetooth 开启
* @value 开关1:开 0:关
* @return      成功
*/
void tz_bt_set_power_on_of_off(int value);

/**
* @description: 判断Bluetooth 设备是否开启
* @return    true开启 false没有开启
*/
bool tz_bt_is_adapter_powered_on();

/**
* @description:  开启蓝牙设备扫描
* @enable 1:开始扫描 0:结束扫描
* @scan_time：设置扫描最长时间s
* @audio_devices:输出列表
* @max_audio_devices 设置audio_devices最大获取max_audio_devices个数
* @return  带有播放功能的蓝牙设备
*/
int tz_bt_handle_scan(int enable, int scan_time, TZ_BT_CONECTED_T* audio_devices, int max_audio_devices);

/**
* @description: 获取已连接成功的蓝牙播放设备
* @connected_devices 蓝牙设备说明
* @max_count 一般填1个 
* @return   成功连接的蓝牙设备个数
*/
int tz_bt_get_connected_devices(TZ_BT_CONECTED_T* connected_devices, int max_count);

/**
* @description: 列出以前已配对过的蓝牙设备
* @paired_devices 蓝牙设备说明
* @max_count 最大获取配对个数
* @return   已配对过蓝牙设备个数
*/
int tz_bt_list_paired_devices(TZ_BT_PAIRED_T* paired_devices, int max_count);

/**
* @description: 获取蓝牙设备详细信息
* @mac  蓝牙MAC
* @info 对应蓝牙的详细信息
*/
void tz_bt_get_device_info_by_mac(const char* mac, TZ_BT_DEVICE_INFO* info);

/**
* @description: 连接蓝牙设备
* @mac  蓝牙MAC
* @return 1：连接成功 0：连接失败
*/
int tz_bt_connect_device(const char* mac);

/**
* @description: 断开蓝牙设备连接
* @mac  蓝牙MAC
* @return 1：断开成功 0：断开失败
*/
int tz_bt_disconnect_device(const char* mac);

/**
* @description: 移除蓝牙设备 删除配对
* @mac  蓝牙MAC
* @return 1：移除成功 0：移除失败
*/
int tz_bt_remove_device(const char* mac);


#ifdef __cplusplus
}
#endif

#endif // TZ_BT_LIB_H
