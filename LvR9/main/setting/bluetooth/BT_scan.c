#include <glib.h>
#include <gio/gio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLUEZ_SERVICE "org.bluez"
#define ADAPTER_INTERFACE "org.bluez.Adapter1"
#define DEVICE_INTERFACE "org.bluez.Device1"

// 支持的音频服务 UUID
static const char *audio_uuids[] = {
    "0000110a-0000-1000-8000-00805f9b34fb", // A2DP Source
    "0000110b-0000-1000-8000-00805f9b34fb", // A2DP Sink
    "0000110c-0000-1000-8000-00805f9b34fb", // Headset (HSP)
    "0000110e-0000-1000-8000-00805f9b34fb", // Hands-Free Audio Gateway
    "0000111e-0000-1000-8000-00805f9b34fb", // Hands-Free Unit
    NULL
};

typedef struct {
    GDBusConnection *conn;
    GList *discovered_devices; // 已发现设备路径列表
    GDBusProxy *adapter_proxy;
    GDBusProxy *om_proxy;
    GMainLoop *loop;
} BluetoothData;

// 判断是否为音频设备
gboolean is_audio_device(GDBusProxy *dev_proxy) {
    if (!dev_proxy)
        return FALSE;

    GVariant *name_var = g_dbus_proxy_get_cached_property(dev_proxy, "Name");
    const char *name = name_var ? g_variant_get_string(name_var, NULL) : "";

    gboolean is_audio_by_name = name && (
        strstr(name, "AirPods") || strstr(name, "Beats") ||
        strstr(name, "Buds") || strstr(name, "Headset") ||
        strstr(name, "Earbuds") || strstr(name, "WH-") ||
        strstr(name, "WF-") || strstr(name, "FreeBuds") ||
        strstr(name, "Xiaomi") || strstr(name, "Huawei")
    );

    if (is_audio_by_name) {
        g_clear_pointer(&name_var, g_variant_unref);
        return TRUE;
    }

    GVariant *uuids_variant = g_dbus_proxy_get_cached_property(dev_proxy, "UUIDs");
    if (!uuids_variant) {
        g_clear_pointer(&name_var, g_variant_unref);
        return FALSE;
    }

    GStrv uuids = g_variant_dup_strv(uuids_variant, NULL);
    g_variant_unref(uuids_variant);

    gboolean is_audio = FALSE;
    for (int i = 0; audio_uuids[i] != NULL; i++) {
        for (char **uuid = uuids; *uuid != NULL; uuid++) {
            if (strcasecmp(*uuid, audio_uuids[i]) == 0) {
                is_audio = TRUE;
                break;
            }
        }
        if (is_audio) break;
    }

    g_strfreev(uuids);
    g_clear_pointer(&name_var, g_variant_unref);
    return is_audio;
}

// InterfacesAdded 信号回调
void on_interfaces_added(GDBusConnection *connection,
                         const gchar *sender_name,
                         const gchar *object_path,
                         const gchar *interface_name,
                         const gchar *signal_name,
                         GVariant *parameters,
                         gpointer user_data)
{
    BluetoothData *data = (BluetoothData *)user_data;
    if (!g_variant_is_of_type(parameters, G_VARIANT_TYPE("(oa{sa{sv}})"))) return;

    const gchar *path;
    GVariant *interfaces_dict;
    g_variant_get(parameters, "(&o@a{sa{sv}})", &path, &interfaces_dict);

    GVariantIter interfaces_iter;
    g_variant_iter_init(&interfaces_iter, interfaces_dict);

    const gchar *interface;
    GVariant *properties_dict;
    while (g_variant_iter_next(&interfaces_iter, "{&s@a{sv}}", &interface, &properties_dict)) {
        if (g_strcmp0(interface, DEVICE_INTERFACE) == 0) {
            gboolean exists = FALSE;
            for (GList *iter = data->discovered_devices; iter != NULL; iter = iter->next) {
                if (g_strcmp0((const char*)iter->data, path) == 0) {
                    exists = TRUE;
                    break;
                }
            }
            if (!exists) {
                data->discovered_devices = g_list_append(data->discovered_devices, g_strdup(path));
            }
        }
        g_clear_pointer(&properties_dict, g_variant_unref);
    }
    g_clear_pointer(&interfaces_dict, g_variant_unref);
}

// 安全停止扫描
void stop_scan_safe(BluetoothData *data) {
    GError *error = NULL;
    if (data->adapter_proxy) {
        g_dbus_proxy_call_sync(data->adapter_proxy, "StopDiscovery", NULL,
                               G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
        if (error) {
            g_printerr("停止扫描失败: %s\n", error->message);
            g_error_free(error);
        }
        g_object_unref(data->adapter_proxy);
        data->adapter_proxy = NULL;
    }
}

// 超时停止扫描并展示结果
gboolean stop_scan(gpointer user_data) {
    BluetoothData *data = (BluetoothData *)user_data;

    stop_scan_safe(data);

    int count = 0;
    printf("\n=== 扫描完成 ===\n");
    for (GList *iter = data->discovered_devices; iter != NULL; iter = iter->next) {
        const char *path = (const char *)iter->data;
        GDBusProxy *dev_proxy = g_dbus_proxy_new_sync(
            data->conn,
            G_DBUS_PROXY_FLAGS_NONE,
            NULL,
            BLUEZ_SERVICE,
            path,
            DEVICE_INTERFACE,
            NULL,
            NULL
        );
        if (dev_proxy && is_audio_device(dev_proxy)) {
            GVariant *name = g_dbus_proxy_get_cached_property(dev_proxy, "Name");
            GVariant *addr = g_dbus_proxy_get_cached_property(dev_proxy, "Address");

            printf("%d. %s (%s)\n", ++count,
                   name ? g_variant_get_string(name, NULL) : "未知设备",
                   addr ? g_variant_get_string(addr, NULL) : "未知地址");

            g_clear_pointer(&name, g_variant_unref);
            g_clear_pointer(&addr, g_variant_unref);
        }
        g_clear_object(&dev_proxy);
    }

    if (count == 0) {
        printf("未找到任何音频输出设备。\n");
    }

    g_main_loop_quit(data->loop); // 返回主菜单
    return G_SOURCE_REMOVE;
}

// 启动扫描（接受 BluetoothData * 参数）
void start_scan(BluetoothData *data) {
    GError *error = NULL;

    // 创建 adapter proxy
    data->adapter_proxy = g_dbus_proxy_new_sync(
        data->conn,
        G_DBUS_PROXY_FLAGS_NONE,
        NULL,
        BLUEZ_SERVICE,
        "/org/bluez/hci0",
        ADAPTER_INTERFACE,
        NULL,
        &error
    );
    if (!data->adapter_proxy) {
        g_printerr("无法创建 adapter proxy: %s\n", error->message);
        g_error_free(error);
        return;
    }

    // 启动扫描
    GVariant *result = g_dbus_proxy_call_sync(
        data->adapter_proxy,
        "StartDiscovery",
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error
    );
    if (!result) {
        g_printerr("启动扫描失败: %s\n", error->message);
        g_error_free(error);
        return;
    }
    g_variant_unref(result);

    // 设置扫描时间为 30 秒后停止
    g_timeout_add_seconds(15, stop_scan, data);
}

// 初始化 D-Bus，只要调用蓝牙都需要初始化dbus的
void init_dbus(BluetoothData *data) {
    GError *error = NULL;
    data->conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    if (!data->conn) {
        g_printerr("连接 D-Bus 失败: %s\n", error->message);
        g_error_free(error);
        exit(1);
    }

    // 注册信号监听
    g_dbus_connection_signal_subscribe(data->conn, BLUEZ_SERVICE,
                                       "org.freedesktop.DBus.ObjectManager", "InterfacesAdded",
                                       NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE,
                                       on_interfaces_added, data, NULL);
}

/* 显示菜单 */
void show_menu(void) {
    printf("\n蓝牙控制菜单:\n");
    printf("4. 扫描音频设备\n");
    printf("0. 退出\n");
    printf("请选择: ");
}

/* 主函数 */
int main() {
    BluetoothData data = {0};
    data.loop = g_main_loop_new(NULL, FALSE);
    init_dbus(&data);

    int choice;
    while (1) {
        show_menu();
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');  // 清除无效输入
            printf("输入无效，请重试\n");
            continue;
        }

        switch (choice) {
            case 4:
                // 清空上一次扫描结果
                g_list_free_full(data.discovered_devices, g_free);
                data.discovered_devices = NULL;

                start_scan(&data);
                g_main_loop_run(data.loop);
                break;
            case 0:
                stop_scan_safe(&data);
                g_clear_object(&data.om_proxy);
                g_clear_object(&data.adapter_proxy);
                g_clear_object(&data.conn);
                g_main_loop_unref(data.loop);
                printf("程序退出\n");
                return 0;
            default:
                printf("无效选项\n");
        }
    }
}