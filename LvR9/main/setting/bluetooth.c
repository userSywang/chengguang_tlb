//========================================================[]
//Name: bluetooth.c
//Function: Bluetooth settings
//========================================================[]

/**********************
     INCLUDES
 *********************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "bluetooth.h"
#include "../ex_widget/app_msg.h"
#include "../utils/misc_utils.h"
#include "../data_parse/img_res.h"
#include "../mainmenu/tlb_main_img.h"
#include "../utils/global_parm.h"
#include "./app_setting_base.h"
#ifndef C_PC_SIM
#include "../../trd/bluetooth//include/tz_bt_lib.h"  
#include "../../trd/tz_sys_lib/include/cvi_tz_sys.h"
#include <pthread.h>
#include <sys/eventfd.h>
#endif

#include <lvgl/lvgl.h>

 /*********************
   *      DEFINES
   *********************/
#define TAG "BluetoothCfg"

   //----------- 主版面 ---------------//
#define	BLUETOOTH_TITLE_STR			"配置蓝牙"
#define BLUETOOTH_TITLE_H		30
#define BLUETOOTH_TITLE_Y		20

#define BLUETOOTH_SWITCH_STR		"蓝牙"

// 设备列表相关
#define BT_DEVICE_BTN_WIDTH     280
#define BT_DEVICE_BTN_HEIGHT    52
#define BT_DEVICE_SPACING       12
#define BT_LIST_MARGIN_TOP      80
#define BT_LIST_MARGIN_LEFT     ((SCREEN_WIDTH - BT_DEVICE_BTN_WIDTH) / 2)

// 状态文本
#define BT_STATUS_TEXT_CLOSED   "要查看可用设备，请打开蓝牙"
#define BT_STATUS_TEXT_SEARCHING "正在搜索设备..."
#define BT_STATUS_TEXT_NOT_FOUND "找不到蓝牙设备"
#define BT_STATUS_CONNECTING    "正在连接，请稍后..."
#define BT_STATUS_CONNECTED     "已连接"
#define BT_STATUS_FAILED        "未连接"

// 设备状态颜色
#define BT_COLOR_CONNECTED      0x008CFF
#define BT_COLOR_DEFAULT        0xFFFFFF

// 状态标签高度
#define BT_STATUS_LABEL_HEIGHT  21

// 刷新按钮位置
#define REFRESH_BTN_X 276
#define REFRESH_BTN_Y 10

#define BT_DETAIL_BTN_WIDTH      280
#define BT_DETAIL_BTN_HEIGHT     52
#define BT_DETAIL_SPACING        12
#define BT_DETAIL_MARGIN_TOP     80
#define BT_DETAIL_MARGIN_LEFT    ((SCREEN_WIDTH - BT_DETAIL_BTN_WIDTH) / 2)

#define MAX_BT_DEVICES 10

// 设备详情页按钮文本
#define BT_DETAIL_DISCONNECT_STR "断开连接"
#define BT_DETAIL_IGNORE_STR     "忽略此设备"

// 设备详情页标题
#define BT_DETAIL_TITLE_FMT      "配置蓝牙"

// 设备状态颜色 (暗色模式下使用更亮的蓝色)
#if DARK_MODE
#define BT_COLOR_CONNECTED      0x4DA6FF
#define BT_COLOR_DEFAULT        0xFFFFFF
#else
#define BT_COLOR_CONNECTED      0x008CFF
#define BT_COLOR_DEFAULT        0x000000
#endif



/**********************
 *  STATIC VARIABLES
 **********************/
static bluetooth_connect_set_t* network;

// 模拟蓝牙设备列表
static const char* bt_devices[] = {
    "蓝牙耳机-A1",
    "智能手表-S2",
    "车载音响-C3",
    "智能手环-B4"
};

#ifndef C_PC_SIM  
TZ_BT_CONECTED_T scanned_devices[10];
#endif

#ifndef C_PC_SIM
static pthread_mutex_t msg_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t msg_queue_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t cmd_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cmd_queue_cond = PTHREAD_COND_INITIALIZER;
static pthread_t bt_thread;
#define MAX_MSG_QUEUE 20

static bt_message_t msg_queue[MAX_MSG_QUEUE];
static int msg_queue_head = 0;
static int msg_queue_tail = 0;
static int cmd_queue_head = 0;
static int cmd_queue_tail = 0;



static lv_timer_t* bt_update_timer;

// 命令队列
#define MAX_QUEUE_SIZE 10
static bt_command_t cmd_queue[MAX_QUEUE_SIZE];



static bt_device_t bt_device_list[10];
static uint8_t bt_device_count = 0;

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void bluetooth_switch_event_cb(lv_event_t* e);
static void refresh_button_event_cb(lv_event_t* e);
static void bt_device_event_cb(lv_event_t* e);
static void update_bluetooth_status(void);
static void create_bt_device_list(void);
static void update_bt_device_ui(uint8_t index);
static void simulate_bt_connection(uint8_t index);
static void update_device_states(void);
static void handle_scanned_devices(TZ_BT_CONECTED_T* devices, int count);
void create_device_detail_page(bt_device_t* device);
void close_device_detail_page(void);

// 定时器回调函数
static void search_timer_cb(lv_timer_t* timer);
static void connection_timer_cb(lv_timer_t* timer);





// 发送命令到后台线程
static void send_bt_command(bt_command_t cmd) {
    pthread_mutex_lock(&cmd_queue_mutex);

    // 检查队列是否已满
    int next_head = (cmd_queue_head + 1) % MAX_QUEUE_SIZE;
    if (next_head == cmd_queue_tail) {
        ESP_LOGW(TAG, "BT command queue full, dropping command");
        pthread_mutex_unlock(&cmd_queue_mutex);
        return;
    }

    cmd_queue[cmd_queue_head] = cmd;
    cmd_queue_head = next_head;

    // 唤醒工作线程
    pthread_cond_signal(&cmd_queue_cond);
    pthread_mutex_unlock(&cmd_queue_mutex);

    // 移除原有的 eventfd 通知
    // uint64_t value = 1;
    // write(bt_cmd_fd, &value, sizeof(value));
}

// 后台线程处理函数
static void* bt_thread_func(void* arg) {
    while (network->thread_running) {

        pthread_mutex_lock(&cmd_queue_mutex);
        // 等待命令
        while (cmd_queue_head == cmd_queue_tail && network->thread_running) {
            pthread_cond_wait(&cmd_queue_cond, &cmd_queue_mutex);
        }

        if (!network->thread_running) {
            pthread_mutex_unlock(&cmd_queue_mutex);
            break; // 退出线程
        }

        // 获取命令
        bt_command_t cmd = cmd_queue[cmd_queue_tail];
        cmd_queue_tail = (cmd_queue_tail + 1) % MAX_QUEUE_SIZE;
        pthread_mutex_unlock(&cmd_queue_mutex);

        // 处理命令
        bt_message_t msg = { 0 };

        ESP_LOGI("bt_thread_func","cmd.cmd:%d\r\n", cmd.cmd);

        switch (cmd.cmd) {
        case BT_CMD_INIT_DBUS:
            msg.event = BT_EVENT_STATUS_UPDATE;
            tz_bt_init_dbus_connection();
            break;

        case BT_CMD_SET_POWER:
            msg.event = BT_EVENT_STATUS_UPDATE;
            tz_bt_set_power_on_of_off(cmd.params.set_power.power);
            break;

        case BT_CMD_SCAN: {
            TZ_BT_CONECTED_T* devices = malloc(MAX_BT_DEVICES * sizeof(TZ_BT_CONECTED_T));
            if (devices == NULL) {
                ESP_LOGE(TAG, "Failed to allocate memory for scan results");
                continue;
            }
            int count = tz_bt_handle_scan(
                cmd.params.scan.enable,
                cmd.params.scan.scan_time,
                devices,
                MAX_BT_DEVICES
            );
            ESP_LOGI(TAG, "BT_CMD_SCAN: count=%d", count);
            msg.event = BT_EVENT_SCAN_RESULT;
            msg.data.scan_result.devices = devices;
            msg.data.scan_result.count = count;
            break;
        }

        case BT_CMD_CONNECT: {
            ESP_LOGI("BT_CMD_CONNECT","cmd.params.device.mac :%s", cmd.params.device.mac);
            bool success = tz_bt_connect_device(cmd.params.device.mac);
            ESP_LOGI("BT_CMD_CONNECT", "connect success or not :%d", success);
            msg.event = BT_EVENT_CONNECT_RESULT;
            strncpy(msg.data.connect_result.mac, cmd.params.device.mac, sizeof(msg.data.connect_result.mac));
            msg.data.connect_result.mac[sizeof(msg.data.connect_result.mac) - 1] = '\0';
            msg.data.connect_result.success = success;
            break;
        }

        case BT_CMD_DISCONNECT: {
            ESP_LOGI(TAG, "Disconnecting device with MAC: %s", cmd.params.device.mac);

            bool success = tz_bt_disconnect_device(cmd.params.device.mac);

            // 准备消息
            msg.event = BT_EVENT_DISCONNECT_RESULT;
            strncpy(msg.data.disconnect_result.mac, cmd.params.device.mac,
                sizeof(msg.data.disconnect_result.mac));
            msg.data.disconnect_result.mac[sizeof(msg.data.disconnect_result.mac) - 1] = '\0';
            msg.data.disconnect_result.success = success;

            break;
        }

        case BT_CMD_REMOVE:
            msg.event = BT_EVENT_STATUS_UPDATE;
            tz_bt_remove_device(cmd.params.device.mac);
            break;

        case BT_CMD_GET_CONNECTED: {
            TZ_BT_CONECTED_T* devices = malloc(MAX_BT_DEVICES * sizeof(TZ_BT_CONECTED_T));
            if (devices == NULL) {
                ESP_LOGE(TAG, "Failed to allocate memory for connected devices");
                continue;
            }
            int count = tz_bt_get_connected_devices(devices, MAX_BT_DEVICES);
            msg.event = BT_EVENT_DEVICE_LIST;
            msg.data.device_list.devices = (TZ_BT_PAIRED_T*)devices; 
            msg.data.device_list.count = count;
            break;
        }

        case BT_CMD_LIST_PAIRED: {
            TZ_BT_PAIRED_T* devices = malloc(MAX_BT_DEVICES * sizeof(TZ_BT_PAIRED_T));
            if (devices == NULL) {
                ESP_LOGE(TAG, "Failed to allocate memory for paired devices");
                continue;
            }
            int count = tz_bt_list_paired_devices(devices, MAX_BT_DEVICES);
            msg.event = BT_EVENT_DEVICE_LIST;
            msg.data.device_list.devices = devices;
            msg.data.device_list.count = count;
            break;
        }

        default:
            // 未知命令，跳过
            continue;
        }

        // 将消息放入消息队列
        pthread_mutex_unlock(&cmd_queue_mutex);
        int next_head = (msg_queue_head + 1) % MAX_MSG_QUEUE;
        if (next_head == msg_queue_tail) {
            ESP_LOGW(TAG, "Message queue full, dropping message");
            // 如果消息队列满了，我们需要释放分配的内存（如果有）
            if (msg.event == BT_EVENT_SCAN_RESULT ||
                msg.event == BT_EVENT_DEVICE_LIST) {
                free(msg.data.scan_result.devices); // 对于BT_EVENT_DEVICE_LIST，我们也可以这样释放，因为结构体中的指针字段在相同位置
            }
        }
        else {
            msg_queue[msg_queue_head] = msg;
            msg_queue_head = next_head;
            pthread_cond_signal(&msg_queue_cond);
        }
    }

    ESP_LOGI(TAG, "BT thread exited cleanly");
    return NULL;
}


static bt_device_t* find_device_by_mac(const char* mac) {
    if (mac == NULL) return NULL;

    for (int i = 0; i < bt_device_count; i++) {
        if (strcmp(bt_device_list[i].mac, mac) == 0) {
            return &bt_device_list[i];
        }
    }
    return NULL;
}


static void update_device_list(TZ_BT_PAIRED_T* devices, int count) {

    for (int i = 0; i < bt_device_count; i++) {
        lv_obj_del(bt_device_list[i].btn);
        bt_device_list[i].btn = NULL;
    }
    bt_device_count = 0;

    // 添加新设备
    for (int i = 0; i < count && i < MAX_BT_DEVICES; i++) {
        bt_device_t* device = &bt_device_list[bt_device_count];

        // 创建设备按钮
        device->btn = lv_btn_create(network->device_list_cont);
        lv_obj_set_size(device->btn, BT_DEVICE_BTN_WIDTH, BT_DEVICE_BTN_HEIGHT);
        lv_obj_set_style_radius(device->btn, 12, 0);
        lv_obj_set_style_bg_color(device->btn, lv_color_white(), 0);
        lv_obj_set_style_border_width(device->btn, 0, 0);
        lv_obj_set_style_shadow_width(device->btn, 0, 0);

        // 设备名称标签
        device->name_label = lv_label_create(device->btn);
        lv_label_set_text(device->name_label, devices[i].name);
        lv_obj_set_style_text_color(device->name_label, lv_color_hex(BT_COLOR_DEFAULT), 0);
        lv_obj_align(device->name_label, LV_ALIGN_LEFT_MID, 20, 0);

        // 状态标签
        device->status_label = lv_label_create(device->btn);
        lv_label_set_text(device->status_label, devices[i].connected ? BT_STATUS_CONNECTED : "");
        lv_obj_set_style_text_color(device->status_label, lv_color_hex(0x787878), 0);
        lv_obj_align(device->status_label, LV_ALIGN_LEFT_MID, 20, 15);
        if (devices[i].connected) {
            lv_obj_clear_flag(device->status_label, LV_OBJ_FLAG_HIDDEN);
        }
        else {
            lv_obj_add_flag(device->status_label, LV_OBJ_FLAG_HIDDEN);
        }

        // 保存设备信息
        strncpy(device->mac, devices[i].mac, sizeof(device->mac) - 1);
        strncpy(device->name, devices[i].name, sizeof(device->name) - 1);
        device->state = devices[i].connected ? BT_STATE_CONNECTED : BT_STATE_DISCONNECTED;

        // 添加事件回调
        lv_obj_add_event_cb(device->btn, bt_device_event_cb,
            LV_EVENT_CLICKED, (void*)(intptr_t)bt_device_count);

        // 设置位置
        lv_obj_set_pos(device->btn, 0,
            bt_device_count * (BT_DEVICE_BTN_HEIGHT + BT_DEVICE_SPACING));

        bt_device_count++;
    }

    // 调整列表容器高度
    lv_obj_set_height(network->device_list_cont,
        bt_device_count * (BT_DEVICE_BTN_HEIGHT + BT_DEVICE_SPACING));

    // 更新状态标签
    if (bt_device_count > 0) {
        lv_label_set_text(network->status_label, "");
    }
    else {
        lv_label_set_text(network->status_label, BT_STATUS_TEXT_NOT_FOUND);
    }
}



// 定时器回调：处理来自后台线程的消息
static void bt_update_cb(lv_timer_t* timer) {
    pthread_mutex_lock(&msg_queue_mutex);

    // 处理队列中的所有消息
    while (msg_queue_tail != msg_queue_head) {
        bt_message_t msg = msg_queue[msg_queue_tail];
        msg_queue_tail = (msg_queue_tail + 1) % MAX_MSG_QUEUE;

        // 解锁以便其他线程可以添加消息
        pthread_mutex_unlock(&msg_queue_mutex);
        switch (msg.event) {
        case BT_EVENT_SCAN_RESULT:
            ESP_LOGI("BT_EVENT_SCAN_RESULT", "RECIVE EVENT\r\n");
            handle_scanned_devices(msg.data.scan_result.devices, msg.data.scan_result.count);
            free(msg.data.scan_result.devices);
            break;

        case BT_EVENT_CONNECT_RESULT: {
            ESP_LOGI("BT_EVENT_CONNECT_RESULT", "RECIVE EVENT, mac=%s, success=%d",
                msg.data.connect_result.mac, msg.data.connect_result.success);
            bt_device_t* device = find_device_by_mac(msg.data.connect_result.mac);
            if (device) {
                device->state = msg.data.connect_result.success ?
                    BT_STATE_CONNECTED : BT_STATE_FAILED;
                update_bt_device_ui(device - bt_device_list);
            }
            break;
        }
        
        case BT_EVENT_DISCONNECT_RESULT: {
            ESP_LOGI("BT_EVENT_DISCONNECT_RESULT",
                "Disconnect result: mac=%s, success=%d",
                msg.data.disconnect_result.mac,
                msg.data.disconnect_result.success);

            bt_device_t* device = find_device_by_mac(msg.data.disconnect_result.mac);
            if (device) {
                if (msg.data.disconnect_result.success) {
                    device->state = BT_STATE_DISCONNECTED;
                }
                else {
                    device->state = BT_STATE_FAILED;
                    ESP_LOGE(TAG, "Failed to disconnect from device: %s", device->mac);
                }
                update_bt_device_ui(device - bt_device_list);
            }
            break;
        }


        case BT_EVENT_DEVICE_LIST:
            ESP_LOGI("BT_EVENT_DEVICE_LIST", "RECIVE EVENT, count=%d", msg.data.device_list.count);
            update_device_list(msg.data.device_list.devices, msg.data.device_list.count);
            free(msg.data.device_list.devices);
            break;

        case BT_EVENT_STATUS_UPDATE:
            ESP_LOGI("BT_EVENT_STATUS_UPDATE", "RECIVE EVENT");
            update_device_states();
            break;

        }
        // 重新加锁以处理下一条消息
        pthread_mutex_lock(&msg_queue_mutex);
    }
    pthread_mutex_unlock(&msg_queue_mutex);
}

























#ifndef C_PC_SIM
// 处理扫描到的设备
static void handle_scanned_devices(TZ_BT_CONECTED_T* devices, int count) {
    ESP_LOGI("handle_scanned_devices", "bt_device_count :%d", bt_device_count);
    ESP_LOGI("handle_scanned_devices", "count :%d", count);
    for (int i = 0; i < count && bt_device_count < MAX_BT_DEVICES; i++) {
        // 检查设备是否已存在
        bool exists = false;
        for (int j = 0; j < bt_device_count; j++) {
            if (strcmp(devices[i].mac, bt_device_list[j].mac) == 0) {
                exists = true;
                break;
            }
        }
        ESP_LOGI("handle_scanned_devices","before add device list\r\n");
        // 添加新设备
        if (!exists) {
            bt_device_t* device = &bt_device_list[bt_device_count];
            ESP_LOGI("handle_scanned_devices", "add device list  A\r\n");
            ESP_LOGI("handle_scanned_devices", "device->btn pointer : %p\r\n", device->btn);
            // 创建设备UI
            device->btn = lv_btn_create(network->device_list_cont);
            ESP_LOGI("handle_scanned_devices", "add device list  A A\r\n");
            lv_obj_set_size(device->btn, BT_DEVICE_BTN_WIDTH, BT_DEVICE_BTN_HEIGHT);
            lv_obj_set_style_radius(device->btn, 12, 0);
            lv_obj_set_style_bg_color(device->btn, lv_color_hex(0x212121), 0);
            lv_obj_set_style_border_width(device->btn, 0, 0);
            lv_obj_set_style_shadow_width(device->btn, 0, 0);

            ESP_LOGI("handle_scanned_devices", "add device list  B\r\n");

            // 设备名称标签
            device->name_label = lv_label_create(device->btn);
            lv_label_set_text(device->name_label, devices[i].name);
            lv_obj_set_style_text_color(device->name_label, lv_color_hex(BT_COLOR_DEFAULT), 0);
            lv_obj_align(device->name_label, LV_ALIGN_LEFT_MID, 20, 0);

            ESP_LOGI("handle_scanned_devices", "add device list  C\r\n");

            // 状态标签
            device->status_label = lv_label_create(device->btn);
            lv_label_set_text(device->status_label, "");
            lv_obj_set_style_text_color(device->status_label, lv_color_hex(0x787878), 0);
            lv_obj_align(device->status_label, LV_ALIGN_LEFT_MID, 20, 15);
            lv_obj_add_flag(device->status_label, LV_OBJ_FLAG_HIDDEN);

            ESP_LOGI("handle_scanned_devices", "add device list  D\r\n");

            ESP_LOGI("devices[].mac", "devices[i].mac:%s\r\n", devices[i].mac);
            ESP_LOGI("devices[].name", "devices[i].name:%s\r\n", devices[i].name);

            // 保存设备信息
            strncpy(device->mac, devices[i].mac, sizeof(device->mac)-1);
            ESP_LOGI("devices[].mac", "devices[%d].mac:%s\r\n",i, devices[i].mac);

            ESP_LOGI("deveice.name", "devices[].name pointer:%p\r\n",devices[i].name);

            strncpy(device->name, devices[i].name, sizeof(device->name) - 1);

            ESP_LOGI("devices[].name", "devices[%d].name:%s\r\n",i, devices[i].name);
            device->state = BT_STATE_DISCONNECTED;

            ESP_LOGI("handle_scanned_devices", "add device list  E\r\n");

            // 添加事件回调
            lv_obj_add_event_cb(device->btn, bt_device_event_cb,
                LV_EVENT_CLICKED, (void*)(intptr_t)bt_device_count);

            ESP_LOGI("handle_scanned_devices", "add device list  F\r\n");

            // 设置位置
            lv_obj_set_pos(device->btn, 0,
                i * (BT_DEVICE_BTN_HEIGHT + BT_DEVICE_SPACING));

            bt_device_count++;
        }
        ESP_LOGI("handle_scanned_devices", "after add device list\r\n");
    }
    ESP_LOGI("handle_scanned_devices", "bt_device_count :%d", bt_device_count);

    // 更新列表高度
    lv_obj_set_height(network->device_list_cont,
        bt_device_count * (BT_DEVICE_BTN_HEIGHT + BT_DEVICE_SPACING));

    ESP_LOGI("handle_scanned_devices", "before update status label\r\n");
    // 更新状态标签
    if (count > 0) {
        
        lv_label_set_text(network->status_label, "");
    }
    else {
        lv_label_set_text(network->status_label, BT_STATUS_TEXT_NOT_FOUND);
    }
    ESP_LOGI("handle_scanned_devices", "after update status label\r\n");
}
#endif



// 返回按钮回调
static void _bt_esc_btn_click_cb(lv_event_t* evt) {
    lv_indev_keypad_wait_release();
    lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
}

// 蓝牙开关事件处理
static void bluetooth_switch_event_cb(lv_event_t* e) {
    lv_obj_t* sw = lv_event_get_target(e);
    int is_on = lv_obj_has_state(sw, LV_STATE_CHECKED);

#ifndef C_PC_SIM 
    tz_bt_set_power_on_of_off(is_on ? 1 : 0);
#endif

    if (is_on) {

        lv_label_set_text(network->status_label, BT_STATUS_TEXT_SEARCHING);
        
        bt_command_t cmd = {
            .cmd = BT_CMD_SET_POWER,
            .params.set_power.power = 1
        };
        send_bt_command(cmd);

        cmd.cmd = BT_CMD_SCAN;
        cmd.params.scan.enable = 1;
        cmd.params.scan.scan_time = 10;
        send_bt_command(cmd);
        }
    else {
        for (int i = 0; i < bt_device_count; i++) {
            if (bt_device_list[i].btn) {
                lv_obj_del(bt_device_list[i].btn);
                bt_device_list[i].btn = NULL;
            }
        }
        bt_device_count = 0; // 重置设备计数

        // 更新列表高度
        lv_obj_set_height(network->device_list_cont, 0);

        // 更新状态标签
        lv_label_set_text(network->status_label, BT_STATUS_TEXT_CLOSED);


        bt_command_t cmd = {
            .cmd = BT_CMD_SET_POWER,
            .params.set_power.power = 0
        };
        send_bt_command(cmd);
    }
}

// 刷新按钮事件
static void refresh_button_event_cb(lv_event_t* e) {
    bt_command_t cmd = {
        .cmd = BT_CMD_LIST_PAIRED
    };
    send_bt_command(cmd);
}

// 搜索定时器回调
static void search_timer_cb(lv_timer_t* timer) {
    create_bt_device_list();

    update_device_states();
    if (timer)
    {
        lv_timer_del(timer);
        timer = NULL;
    }
}

// 创建设备列表
static void create_bt_device_list(void) {

#ifndef C_PC_SIM  // 真机环境
    bt_command_t cmd = {
        .cmd = BT_CMD_LIST_PAIRED
    };
    send_bt_command(cmd);
#else  // 仿真环境
    uint8_t device_count = sizeof(bt_devices) / sizeof(bt_devices[0]);

    for (int i = 0; i < device_count; i++) {
        bt_device_t* device = &bt_device_list[bt_device_count];

        // 创建设备按钮
        device->btn = lv_btn_create(network->device_list_cont);
        lv_obj_set_size(device->btn, BT_DEVICE_BTN_WIDTH, BT_DEVICE_BTN_HEIGHT);
        lv_obj_set_style_radius(device->btn, 12, 0);
        lv_obj_set_style_bg_color(device->btn, lv_color_hex(DARK_LIST_ITEM_COLOR), 0);
        lv_obj_set_style_border_width(device->btn, 0, 0);
        lv_obj_set_style_shadow_width(device->btn, 0, 0);

        // 设备名称标签
        device->name_label = lv_label_create(device->btn);
        lv_label_set_text(device->name_label, bt_devices[i]);
        lv_obj_set_style_text_color(device->name_label, DARK_TEXT_COLOR, 0);
        lv_obj_align(device->name_label, LV_ALIGN_LEFT_MID, 20, 0);

        // 状态标签
        device->status_label = lv_label_create(device->btn);
        lv_label_set_text(device->status_label, "");
        lv_obj_set_style_text_color(device->status_label, lv_color_hex(0x787878), 0);
        lv_obj_align(device->status_label, LV_ALIGN_LEFT_MID, 20, 15);
        lv_obj_add_flag(device->status_label, LV_OBJ_FLAG_HIDDEN);

        // 保存设备信息
        strncpy(device->name, bt_devices[i], sizeof(device->name) - 1);

        device->state = BT_STATE_DISCONNECTED;
        strcpy(device->mac, ""); // 仿真环境没有真实MAC

        // 添加事件回调
        lv_obj_add_event_cb(device->btn, bt_device_event_cb,
            LV_EVENT_CLICKED, (void*)(intptr_t)bt_device_count);

        // 设置位置
        lv_obj_set_pos(device->btn, 0,
            bt_device_count * (BT_DEVICE_BTN_HEIGHT + BT_DEVICE_SPACING));

        bt_device_count++;
    }
#endif

    // 调整列表容器高度
    lv_obj_set_height(network->device_list_cont,
        bt_device_count * (BT_DEVICE_BTN_HEIGHT + BT_DEVICE_SPACING));

    // 更新状态标签
    if (bt_device_count > 0) {
        lv_label_set_text(network->status_label, "");
    }
    else {
        lv_label_set_text(network->status_label, BT_STATUS_TEXT_NOT_FOUND);
    }
}
// 设备点击事件
static void bt_device_event_cb(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    uint8_t index = (uint8_t)(intptr_t)lv_event_get_user_data(e);
    if (index >= bt_device_count) return;
    ESP_LOGI("bt_device_event_cb", "enter bt_device_event_cb\r\n");
    bt_device_t* device = &bt_device_list[index];
    ESP_LOGI("bt_device_event_cb", "bt_device_list->state :%d", bt_device_list->state);
    ESP_LOGI("bt_device_event_cb","device STAT :%d", device->state);
    if (device->state != BT_STATE_CONNECTED)//|| device->state == BT_STATE_FAILED
    {
        ESP_LOGI("bt_device_event_cb", "device->mac :%s", device->mac);

        bt_command_t cmd;
        cmd.cmd = BT_CMD_CONNECT;

        strncpy(cmd.params.device.mac, device->mac, sizeof(cmd.params.device.mac) - 1);

        send_bt_command(cmd);

        // 立即更新为连接中状态
        device->state = BT_STATE_CONNECTING;
        update_bt_device_ui(index);
    }
    else if (device->state == BT_STATE_CONNECTED)
    {
        ESP_LOGI("bt_device_event_cb", "enter create_device_detail_page\r\n");
        create_device_detail_page(device);
    }

}

// 更新设备UI
static void update_bt_device_ui(uint8_t index) {
    bt_device_t* device = &bt_device_list[index];


    lv_obj_clear_flag(device->status_label, LV_OBJ_FLAG_HIDDEN);


    switch (device->state) {
    case BT_STATE_CONNECTING:
        lv_label_set_text(device->status_label, BT_STATUS_CONNECTING);
        lv_obj_set_height(device->btn, BT_DEVICE_BTN_HEIGHT + BT_STATUS_LABEL_HEIGHT);
        break;

    case BT_STATE_CONNECTED:
        lv_label_set_text(device->status_label, BT_STATUS_CONNECTED);
        lv_obj_set_style_text_color(device->name_label, lv_color_hex(BT_COLOR_CONNECTED), 0);
        lv_obj_set_style_text_color(device->status_label, lv_color_hex(BT_COLOR_CONNECTED), 0);
        break;

    case BT_STATE_FAILED:
        lv_label_set_text(device->status_label, BT_STATUS_FAILED);
        lv_obj_set_style_text_color(device->status_label, lv_color_hex(0xFF0000), 0);
        break;

    case BT_STATE_DISCONNECTED:
  
        lv_obj_add_flag(device->status_label, LV_OBJ_FLAG_HIDDEN);

        lv_obj_set_style_text_color(device->name_label, lv_color_hex(BT_COLOR_DEFAULT), 0);
 
        lv_obj_set_height(device->btn, BT_DEVICE_BTN_HEIGHT);
 
        lv_obj_align(device->name_label, LV_ALIGN_LEFT_MID, 20, 0);
        break;
    
    case BT_STATE_DISCONNECTING://断不能用状态文本，因为还没跳到那

        ESP_LOGI("BT_STATE_DISCONNECTING","disconnect device\r\n");
        //lv_label_set_text(device->status_label, "断开中...");
        //lv_obj_set_style_text_color(device->status_label, lv_color_hex(0x787878), 0);

        close_device_detail_page();
        break;


    default:
        break;
    }


    lv_obj_align(device->name_label, LV_ALIGN_TOP_LEFT, 20, 10);
    lv_obj_align(device->status_label, LV_ALIGN_TOP_LEFT, 20, 35);
}

// 模拟蓝牙连接过程
static void simulate_bt_connection(uint8_t index) {
   
    lv_timer_t* timer = lv_timer_create(connection_timer_cb, 2000, (void*)(intptr_t)index);
}

// 连接定时器回调
static void connection_timer_cb(lv_timer_t* timer) {
    uint8_t index = (uint8_t)(intptr_t)timer->user_data;
    bt_device_t* device = &bt_device_list[index];

    // 随机决定
    if (rand() % 100 > 20) { 
        device->state = BT_STATE_CONNECTED;
    }
    else {
        device->state = BT_STATE_FAILED;
    }

    update_bt_device_ui(index);
    lv_timer_del(timer);
    timer = NULL;
}

// 初始化蓝牙设置
static void bluetooth_setting_init(void) {
    if (network != NULL) return;

    network = heap_caps_malloc_dflt(sizeof(bluetooth_connect_set_t));
    memset(network, 0, sizeof(bluetooth_connect_set_t));

    // 初始化蓝牙状态
    memset(bt_device_list, 0, sizeof(bt_device_list));
    network->bluetooth_sw_state = 0;
    bt_device_count = 0;
}

static void update_device_states(void) {
#ifndef C_PC_SIM
    TZ_BT_CONECTED_T connected_devices[5];
    int count = tz_bt_get_connected_devices(connected_devices, 5);

    if (count < 0) {
        ESP_LOGE(TAG, "获取连接设备失败!");
        return;
    }
    bt_device_count = count;
    ESP_LOGI("update_device_states","count:%d", count);

    // 更新所有设备状态
    for (int i = 0; i < bt_device_count; i++) {
        bt_device_list[i].state = BT_STATE_DISCONNECTED;

        // 检查是否已连接
        for (int j = 0; j < count; j++) {
            if (strcmp(bt_device_list[i].mac, connected_devices[j].mac) == 0) {
                bt_device_list[i].state = BT_STATE_CONNECTED;
                break;
            }
        }

        // 更新UI
        update_bt_device_ui(i);
    }
#endif
}


// 蓝牙设置页面构建
static void bluetooth_setting_constructor(void) {

    network->cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(network->cont, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align(network->cont, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(network->cont, lv_color_hex(DARK_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(network->cont, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(network->cont, 0, 0);
    lv_obj_set_style_border_width(network->cont, 0, 0);
    lv_obj_set_style_radius(network->cont, 0, 0);
    lv_obj_clear_flag(network->cont, LV_OBJ_FLAG_SCROLLABLE);

    // 标题
    network->title_label = lv_label_create(network->cont);
    lv_obj_set_size(network->title_label, LV_SIZE_CONTENT, BLUETOOTH_TITLE_H);
    lv_obj_set_style_text_color(network->title_label, DARK_TEXT_COLOR, 0);
    lv_label_set_text(network->title_label, BLUETOOTH_TITLE_STR);
    lv_obj_align(network->title_label, LV_ALIGN_TOP_MID, 0, BLUETOOTH_TITLE_Y);

    // 返回按钮
    stng_createEscBtn(network->cont, _bt_esc_btn_click_cb, 0);

    // 刷新按钮
    network->refresh_btn = lv_img_create(network->cont);
    lv_img_dsc_t* refresh_img = zb_img_res_load_one(NULL, SETTING_IMG_REFRESH, IMG_FREE_TYPE_AUTO, NULL);
    lv_img_set_src(network->refresh_btn, refresh_img);
    lv_obj_set_style_pad_top(network->refresh_btn, 10, 0);
    lv_obj_align(network->refresh_btn, LV_ALIGN_TOP_LEFT, REFRESH_BTN_X, REFRESH_BTN_Y);
    lv_obj_add_flag(network->refresh_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(network->refresh_btn, 30);
    lv_obj_add_event_cb(network->refresh_btn, refresh_button_event_cb, LV_EVENT_CLICKED, NULL);

    // 蓝牙开关容器
    network->bt_switch_cont = lv_obj_create(network->cont);
    lv_obj_set_size(network->bt_switch_cont, 280, 52);
    lv_obj_align(network->bt_switch_cont, LV_ALIGN_TOP_LEFT, BT_LIST_MARGIN_LEFT, BT_LIST_MARGIN_TOP);
    lv_obj_set_style_bg_color(network->bt_switch_cont, lv_color_hex(DARK_CONTAINER_COLOR), 0);
    lv_obj_set_style_radius(network->bt_switch_cont, 12, 0);
    lv_obj_set_style_border_width(network->bt_switch_cont, 0, 0);
    lv_obj_set_style_shadow_width(network->bt_switch_cont, 0, 0);

    // 蓝牙开关标签
    lv_obj_t* bt_label = lv_label_create(network->bt_switch_cont);
    lv_label_set_text(bt_label, BLUETOOTH_SWITCH_STR);
    lv_obj_set_style_text_color(bt_label, DARK_TEXT_COLOR, 0);
    lv_obj_align(bt_label, LV_ALIGN_LEFT_MID, 20, 0);

    // 蓝牙开关
    network->bt_switch = lv_switch_create(network->bt_switch_cont);
    lv_obj_set_size(network->bt_switch, 48, 25);
    lv_obj_align(network->bt_switch, LV_ALIGN_RIGHT_MID, -20, 0);
    lv_obj_add_event_cb(network->bt_switch, bluetooth_switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

   bool stat = tz_bt_is_adapter_powered_on();
   ESP_LOGI("bluetooth_setting_constructor","tz_bt_is_adapter_powered_on :%d", stat);

   if (stat)lv_obj_add_state(network->bt_switch, LV_STATE_CHECKED);
   else   lv_obj_clear_state(network->bt_switch, LV_STATE_CHECKED);
    // 状态标签
    network->status_label = lv_label_create(network->cont);
    lv_label_set_text(network->status_label, BT_STATUS_TEXT_CLOSED);
    lv_obj_set_style_text_color(network->status_label, lv_color_hex(0x787878), 0);
    //lv_obj_set_style_text_font(network->status_label, &lv_font_montserrat_16, 0);
    lv_obj_align(network->status_label, LV_ALIGN_TOP_MID, 0, BT_LIST_MARGIN_TOP + 70);

    // 设备列表容器
    network->device_list_cont = lv_obj_create(network->cont);
    lv_obj_remove_style_all(network->device_list_cont);
    lv_obj_set_style_bg_color(network->device_list_cont, lv_color_hex(DARK_LIST_BG_COLOR), 0);
    lv_obj_set_size(network->device_list_cont, BT_DEVICE_BTN_WIDTH, 0);
    lv_obj_set_pos(network->device_list_cont, BT_LIST_MARGIN_LEFT, BT_LIST_MARGIN_TOP + 100);
    lv_obj_set_flex_flow(network->device_list_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(network->device_list_cont, BT_DEVICE_SPACING, 0);
}





// 设备详情页事件回调
static void detail_action_cb(lv_event_t* e) {
    bt_detail_action_t action = (bt_detail_action_t)(intptr_t)lv_event_get_user_data(e);
    ESP_LOGI("detail_action_cb","network->current_detail_device->name :%s", network->current_detail_device->name);
    bt_device_t* device = network->current_detail_device;   
    uint8_t index = device - bt_device_list;
    ESP_LOGI("detail_action_cb", "index :%d", index);

    if (action == BT_DETAIL_DISCONNECT) {
        bt_command_t cmd;
        cmd.cmd = BT_CMD_DISCONNECT;
        strncpy(cmd.params.device.mac, device->mac, sizeof(cmd.params.device.mac) - 1);
        cmd.params.device.mac[sizeof(cmd.params.device.mac) - 1] = '\0';
        send_bt_command(cmd);

        // 不要立即关闭详情页，等待断开结果
        // 可以在这里设置状态为"断开中..."
        device->state = BT_STATE_DISCONNECTING;
        update_bt_device_ui(index);
    }

    if (action == BT_DETAIL_IGNORE)
    {
        bt_command_t cmd = {
            .cmd = BT_CMD_REMOVE,
            .params.device.mac = device->mac
        };
        send_bt_command(cmd);

        for (int i = index; i < bt_device_count - 1; i++) {
            bt_device_list[i] = bt_device_list[i + 1];

#if 0
            // 更新按钮的事件回调数据（索引）
            lv_obj_remove_event_cb(bt_device_list[i].btn, bt_device_event_cb);
            lv_obj_add_event_cb(bt_device_list[i].btn, bt_device_event_cb,
                LV_EVENT_CLICKED, (void*)(intptr_t)i);

            // 更新按钮位置
            lv_obj_set_pos(bt_device_list[i].btn, 0,
                i * (BT_DEVICE_BTN_HEIGHT + BT_DEVICE_SPACING));

            // 调整列表高度
            lv_obj_set_height(network->device_list_cont,
                bt_device_count * (BT_DEVICE_BTN_HEIGHT + BT_DEVICE_SPACING));
#endif
        }

        // 最后一个设备清空
        memset(&bt_device_list[bt_device_count - 1], 0, sizeof(bt_device_t));
        bt_device_count--;

       // close_device_detail_page();
    }
        
        
    
}

// 关闭设备详情页
void close_device_detail_page(void) {
    if (network->detail_page) {
        lv_obj_del(network->detail_page);
        network->detail_page = NULL;
        network->current_detail_device = NULL;
    }
}

// 设备详情页返回按钮回调
static void detail_back_cb(lv_event_t* e) {
    lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
}

// 创建设备详情页
void create_device_detail_page(bt_device_t* device) {

    close_device_detail_page();


    network->current_detail_device = device;

    ESP_LOGI("create_device_detail_page", "device->name :%s", device->name);
    network->detail_page = lv_obj_create(lv_scr_act());
    lv_obj_set_size(network->detail_page, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(network->detail_page, lv_color_hex(DARK_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(network->detail_page, LV_OPA_COVER, 0);
    lv_obj_align(network->detail_page, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_border_width(network->detail_page, 0, 0);

    stng_createEscBtn(network->detail_page, detail_back_cb, 0);


    lv_obj_t* title = lv_label_create(network->detail_page);
    lv_label_set_text(title, "配置蓝牙");
    lv_obj_set_style_text_color(title, lv_color_hex(0x18191B), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, BLUETOOTH_TITLE_Y);


    lv_obj_t* device_cont = lv_obj_create(network->detail_page);
    lv_obj_set_size(device_cont, BT_DEVICE_BTN_WIDTH, BT_DEVICE_BTN_HEIGHT + BT_STATUS_LABEL_HEIGHT);
    lv_obj_align(device_cont, LV_ALIGN_TOP_LEFT, BT_LIST_MARGIN_LEFT, BT_LIST_MARGIN_TOP);
    lv_obj_set_style_bg_color(device_cont, lv_color_hex(DARK_CONTAINER_COLOR), 0);
    lv_obj_set_style_radius(device_cont, 12, 0);
    lv_obj_set_style_border_width(device_cont, 0, 0);
    lv_obj_set_style_shadow_width(device_cont, 0, 0);

    // 设备名称标签
    lv_obj_t* device_label = lv_label_create(device_cont);
    lv_label_set_text(device_label, device->name);
    lv_obj_set_style_text_color(device_label, DARK_TEXT_COLOR, 0); 
    lv_obj_align(device_label, LV_ALIGN_TOP_LEFT, 20, 10);


    // 设备状态标签
    lv_obj_t* status_label = lv_label_create(device_cont);
    lv_label_set_text(status_label, BT_STATUS_CONNECTED);
    lv_obj_set_style_text_color(status_label, DARK_TEXT_COLOR, 0);
#if 0
    lv_obj_align(status_label, LV_ALIGN_LEFT_MID, 20, 0);
#else
    lv_obj_align(status_label, LV_ALIGN_LEFT_MID, 20, 15);
#endif


    // 创建操作按钮容器
    lv_obj_t* action_cont = lv_obj_create(network->detail_page);
    lv_obj_remove_style_all(action_cont);
    lv_obj_set_size(action_cont, BT_DEVICE_BTN_WIDTH,
        (BT_DEVICE_BTN_HEIGHT * 2) + BT_DEVICE_SPACING);
    lv_obj_align_to(action_cont, device_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, BT_DEVICE_SPACING);
    lv_obj_set_flex_flow(action_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(action_cont, BT_DEVICE_SPACING, 0);
    lv_obj_set_style_bg_color(action_cont, lv_color_hex(DARK_BG_COLOR), 0);

    // 断开连接按钮
    lv_obj_t* disconnect_btn = lv_btn_create(action_cont);
    lv_obj_set_size(disconnect_btn, BT_DEVICE_BTN_WIDTH, BT_DEVICE_BTN_HEIGHT);
    lv_obj_set_style_radius(disconnect_btn, 12, 0);
    lv_obj_set_style_border_width(disconnect_btn, 0, 0);
    lv_obj_set_style_shadow_width(disconnect_btn, 0, 0);
    lv_obj_set_style_bg_color(disconnect_btn, lv_color_hex(DARK_CONTAINER_COLOR), 0);
    lv_obj_set_style_border_width(disconnect_btn, 0, 0);
    lv_obj_add_event_cb(disconnect_btn, detail_action_cb, LV_EVENT_CLICKED,
        (void*)BT_DETAIL_DISCONNECT);

    lv_obj_t* disconnect_label = lv_label_create(disconnect_btn);
    lv_label_set_text(disconnect_label, BT_DETAIL_DISCONNECT_STR);
    lv_obj_set_style_text_color(disconnect_label, DARK_TEXT_COLOR, 0);
    lv_obj_center(disconnect_label);


    lv_obj_t* ignore_btn = lv_btn_create(action_cont);
    lv_obj_set_size(ignore_btn, BT_DEVICE_BTN_WIDTH, BT_DEVICE_BTN_HEIGHT);
    lv_obj_set_style_border_width(ignore_btn, 0, 0);
    lv_obj_set_style_shadow_width(ignore_btn, 0, 0);
    lv_obj_set_style_radius(ignore_btn, 12, 0);
    lv_obj_set_style_bg_color(ignore_btn, lv_color_hex(DARK_CONTAINER_COLOR), 0);
    lv_obj_set_style_border_width(ignore_btn, 0, 0);
    lv_obj_add_event_cb(ignore_btn, detail_action_cb, LV_EVENT_CLICKED,
        (void*)BT_DETAIL_IGNORE);

    lv_obj_t* ignore_label = lv_label_create(ignore_btn);
    lv_label_set_text(ignore_label, BT_DETAIL_IGNORE_STR);
    lv_obj_set_style_text_color(ignore_label, DARK_TEXT_COLOR, 0);
    lv_obj_center(ignore_label);
}












/**********************
*   GLOBAL FUNCTIONS
**********************/

void bluetooth_setting_exit(void) {
    ESP_LOGI(TAG, "Exiting Bluetooth settings...");
#ifndef C_PC_SIM
    ESP_LOGE("bluetooth_setting_exit", "-------A -------- \r\n");
    //tz_bt_handle_scan(0, 0, NULL, 0); // 停止扫描

    //tz_bt_set_power_on_of_off(0);
    ESP_LOGE("bluetooth_setting_exit", "-------B -------- \r\n");
    network->thread_running = false;

    pthread_mutex_lock(&cmd_queue_mutex);
    pthread_cond_signal(&cmd_queue_cond); // 唤醒等待的线程
    pthread_mutex_unlock(&cmd_queue_mutex);
    // 取消后台线程
    pthread_join(bt_thread, NULL);
    ESP_LOGE("bluetooth_setting_exit", "-------C -------- \r\n");
#endif
 
    if (bt_update_timer) {
        lv_timer_del(bt_update_timer);
        bt_update_timer = NULL;
    }

    // 5. 清理消息队列
    pthread_mutex_lock(&msg_queue_mutex);
    while (msg_queue_tail != msg_queue_head) {
        bt_message_t msg = msg_queue[msg_queue_tail];
        msg_queue_tail = (msg_queue_tail + 1) % MAX_MSG_QUEUE;

        // 释放动态分配的内存
        switch (msg.event) {
        case BT_EVENT_SCAN_RESULT:
        case BT_EVENT_DEVICE_LIST:
            free(msg.data.scan_result.devices);
            break;
        default:
            break;
        }
    }
    pthread_mutex_unlock(&msg_queue_mutex);

    close_device_detail_page();
    ESP_LOGE("bluetooth_setting_exit", "-------D -------- \r\n");
    if (network != NULL) {
        for (int i = 0; i < bt_device_count; i++) {
            if (bt_device_list[i].btn) {
                lv_obj_del(bt_device_list[i].btn);
                bt_device_list[i].btn = NULL;
            }
        }
        ESP_LOGE("bluetooth_setting_exit", "-------E -------- \r\n");
        bt_device_count = 0;  

        if (network->cont) {
            lv_obj_del(network->cont);
            network->cont = NULL;
        }
        ESP_LOGE("bluetooth_setting_exit", "-------F -------- \r\n");

        heap_caps_free(network);
        network = NULL;
    }
    ESP_LOGE("bluetooth_setting_exit", "-------G -------- \r\n");
}

void bluetooth_setting_entry(void) {

    // 初始化消息队列
    msg_queue_head = 0;
    msg_queue_tail = 0;

    bluetooth_setting_init();
    // 设置线程运行标志
    network->thread_running = true;

    // 创建后台线程
    pthread_create(&bt_thread, NULL, bt_thread_func, NULL);

    // 创建定时器处理消息
    bt_update_timer = lv_timer_create(bt_update_cb, 50, NULL);


    bluetooth_setting_constructor();

    sys_setCurMainWinAndCb(network->cont, _bt_esc_btn_click_cb);

}
#else
void bluetooth_setting_entry(void) {
    return;
    
}


void bluetooth_setting_exit(void) {
    return;
}
#endif