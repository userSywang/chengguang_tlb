#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <glib.h>
#include <gio/gio.h>

#define SCANF_COUNT 10

typedef struct {    
    GDBusConnection *conn;
    GDBusProxy *adapter_proxy;
    GDBusProxy *om_proxy;
    GDBusProxy *agent_manager_proxy;
    GMainLoop *loop;
    GList *discovered_devices;
    GList *paired_devices;
    guint scan_timeout_id;
    guint scan_signal_id;
    gboolean scanning;
} BluetoothData;

//这个检索的都是默认已配对过的，gboolean is_paired;        // 是否已配对
typedef struct {
    char name[128];            // 设备名称
    char address[64];         // MAC 地址
    gboolean is_connected;     // 是否已连接    1是已连接，0是没连接
} BTDevice;

// 扫描只能扫出十个
BTDevice sacn_devices[SCANF_COUNT];


// 初始化 D-Bus，只要调用蓝牙都需要初始化dbus
void init_dbus(BluetoothData *data);
//获取扫描设备信息
int get_scanned_devices(BTDevice *devices, int max_count) ;

void stop_scan_safe(BluetoothData *data);
//扫描
void start_scan(BluetoothData *data);
//设置蓝牙开关    TRUE是开，false是FALSE
void set_bluetooth_power(BluetoothData *data, gboolean power);
//获取蓝牙的开关状态
gboolean get_bluetooth_status(BluetoothData *data);
//获取已连接的设备
void show_paired_devices(BTDevice *devices,int *count, BluetoothData *data);
//连接蓝牙
void pair_and_connect(BluetoothData *data, const char *mac);
//取消连接
void disconnect_device(BluetoothData *data, const char *mac);
void register_agent(BluetoothData *data);
//删除设备，历史设备就找不到了
void remove_device(BluetoothData *data, const char *mac);

#endif // BLUETOOTH_H