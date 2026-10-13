//=========================================================
// bluetooth.h
//
// 
//Date: 2024-04-02
//========================================================

#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H

#ifdef __cplusplus
extern "C" {
#endif
#include "../../trd/bluetooth//include/tz_bt_lib.h"  
#include <lvgl/lvgl.h>


    // 蓝牙状态
    typedef enum {
        BLUETOOTH_OFF,
        BLUETOOTH_ON,
        BLUETOOTH_CONNECTING,
        BLUETOOTH_CONNECTED
    } bluetooth_state_t;

    // 设备状态
    typedef enum {
        BT_STATE_DISCONNECTED,  // 未连接
        BT_STATE_CONNECTING,    // 连接中
        BT_STATE_CONNECTED,     // 已连接
        BT_STATE_FAILED,         // 连接失败
        BT_STATE_DISCONNECTING  //断开设备中
    } bt_device_state_t;


    // 设备详情页操作类型
    typedef enum {
        BT_DETAIL_DISCONNECT,   // 断开连接
        BT_DETAIL_IGNORE        // 忽略设备
    } bt_detail_action_t;

    // 蓝牙设备结构（向前声明）
    typedef struct bt_device_t bt_device_t;

    // 蓝牙设置结构
    typedef struct {
        lv_obj_t* cont;
        lv_obj_t* title_label;

        // 蓝牙开关
        lv_obj_t* bt_switch_cont;
        lv_obj_t* bt_switch;
        uint8_t bluetooth_sw_state;

        // 状态显示
        lv_obj_t* status_label;

        // 设备列表
        lv_obj_t* device_list_cont;

        // 刷新按钮
        lv_obj_t* refresh_btn;

        // 设备详情页
        lv_obj_t* detail_page;
        bt_device_t* current_detail_device; // 当前显示的设备详情

        volatile bool thread_running; // 新增线程运行标志
    } bluetooth_connect_set_t;

    // 蓝牙设备结构
    struct bt_device_t {
        char mac[18];  // 新增MAC地址字段

        char name[256];
        bluetooth_state_t state;
        lv_obj_t* btn;
        lv_obj_t* name_label;
        lv_obj_t* status_label;

        // 用于设备详情页
        lv_obj_t* detail_btn; // 设备详情页的按钮
    };


    // bluetooth.h 新增内容
    typedef enum {
        BT_CMD_INIT_DBUS,
        BT_CMD_SET_POWER,
        BT_CMD_SCAN,
        BT_CMD_CONNECT,
        BT_CMD_DISCONNECT,
        BT_CMD_REMOVE,
        BT_CMD_GET_CONNECTED,
        BT_CMD_LIST_PAIRED
    } bt_cmd_t;

    typedef struct {
        bt_cmd_t cmd;
        union {
            struct {
                int power;
            } set_power;
            struct {
                int enable;
                int scan_time;
            } scan;
            struct {
                char mac[18];
            } device;
        } params;
    } bt_command_t;

    typedef enum {
        BT_EVENT_SCAN_RESULT,
        BT_EVENT_CONNECT_RESULT,
        BT_EVENT_DISCONNECT_RESULT,
        BT_EVENT_DEVICE_LIST,
        BT_EVENT_STATUS_UPDATE
    } bt_event_t;

    typedef struct {
        bt_event_t event;
        union {
            struct {
                TZ_BT_CONECTED_T* devices;
                int count;
            } scan_result;
            struct {
                char mac[18];
                bool success;
            } connect_result;
            struct {
                char mac[18];
                bool success;
            } disconnect_result; 
            struct {
                TZ_BT_PAIRED_T* devices;
                int count;
            } device_list;
        } data;
    } bt_message_t;




















    /** 蓝牙配置初始化 */
    extern void bluetooth_setting_entry(void);

    /** 蓝牙配置销毁 */
    extern void bluetooth_setting_exit(void);

    /** 创建设备详情页 */
    extern void create_device_detail_page(bt_device_t* device);

    /** 关闭设备详情页 */
    extern void close_device_detail_page(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif