#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>
#include "BTManage.h"

#define BLUEZ_SERVICE "org.bluez"
#define ADAPTER_INTERFACE "org.bluez.Adapter1"
#define DEVICE_INTERFACE "org.bluez.Device1"
#define DBUS_OM_INTERFACE "org.freedesktop.DBus.ObjectManager"

#define AGENT_MANAGER_INTERFACE "org.bluez.AgentManager1"
#define DBUS_PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"

#define HEADSET_UUID      "00001108-0000-1000-8000-00805f9b34fb" //头戴式耳机
#define UUID_A2DP_SINK    "0000110A-0000-1000-8000-00805F9B34FB" // 音频接收（耳机/音箱）
#define UUID_AVRCP_TARGET "0000110C-0000-1000-8000-00805F9B34FB" // 控制播放器（如手机）

static const char *audio_uuids[] = {
    "0000110a-0000-1000-8000-00805f9b34fb", // Advanced Audio Distribution Profile (A2DP)
    "0000110b-0000-1000-8000-00805f9b34fb", // Handsfree Profile (HFP)
    "0000110c-0000-1000-8000-00805f9b34fb", // Headset Profile (HSP)
    "0000111e-0000-1000-8000-00805f9b34fb", // AVCTP
    "00001108-0000-1000-8000-00805f9b34fb" //头戴式耳机
    "0000111f-0000-1000-8000-00805f9b34fb", // AVRCP
    NULL
};


// 初始化 D-Bus，只要调用蓝牙都需要初始化dbus的
void init_dbus(BluetoothData *data) {
    GError *error = NULL;
    data->conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    if (error) {
        fprintf(stderr, "DBus连接失败: %s\n", error->message);
        g_error_free(error);
        exit(1);
    }

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
    if (error) {
        fprintf(stderr, "创建适配器代理失败: %s\n", error->message);
        g_error_free(error);
        exit(1);
    }

    data->om_proxy = g_dbus_proxy_new_sync(
        data->conn,
        G_DBUS_PROXY_FLAGS_NONE,
        NULL,
        BLUEZ_SERVICE,
        "/",
        DBUS_OM_INTERFACE,
        NULL,
        &error
    );
    if (error) {
        fprintf(stderr, "创建对象管理器代理失败: %s\n", error->message);
        g_error_free(error);
        exit(1);
    }

    data->agent_manager_proxy = g_dbus_proxy_new_sync(
        data->conn,
        G_DBUS_PROXY_FLAGS_NONE,
        NULL,
        BLUEZ_SERVICE,
        "/org/bluez",
        AGENT_MANAGER_INTERFACE,
        NULL,
        &error
    );
    if (error) {
        fprintf(stderr, "创建AgentManager代理失败: %s\n", error->message);
        g_error_free(error);
        exit(1);
    }
}

int get_scanned_devices(BTDevice *devices, int max_count) {
    int count = 0;
    for (int i = 0; i < SCANF_COUNT && i < max_count; i++) {
        if (strlen(sacn_devices[i].address) > 0) {
            devices[count++] = sacn_devices[i];
        }
    }
    return count;
}

void stop_scan_safe(BluetoothData *data) {
    if (!data->scanning) {
        printf("没有正在进行的扫描。\n");
        return;
    }

    if (data->scan_timeout_id) {
        g_source_remove(data->scan_timeout_id);
        data->scan_timeout_id = 0;
    }

    if (data->scan_signal_id) {
        g_dbus_connection_signal_unsubscribe(data->conn, data->scan_signal_id);
        data->scan_signal_id = 0;
    }

    GError *error = NULL;
    GVariant *result = g_dbus_proxy_call_sync(
        data->adapter_proxy,
        "StopDiscovery",
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error
    );

    if (error) {
        if (!g_error_matches(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED)) {
            fprintf(stderr, "停止扫描失败: %s\n", error->message);
        }
        g_error_free(error);
    } else {
        g_variant_unref(result);
    }

    data->scanning = FALSE;
}
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

static gboolean scan_timeout_cb(gpointer user_data) {
    BluetoothData *data = (BluetoothData *)user_data;
    printf("\n=== 扫描完成 ===\n");

    GList *iter;
    int count = 0;
    for(int i = 0;i < SCANF_COUNT ; i++)
    {
        memset(&sacn_devices[i], 0, sizeof(BTDevice));
    }
    int j = 0;
    for (iter = data->discovered_devices; iter != NULL; iter = iter->next) {
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
                
            g_strlcpy(sacn_devices[j].name, g_variant_get_string(name, NULL), sizeof(sacn_devices[j].name));
            g_strlcpy(sacn_devices[j].address, g_variant_get_string(addr, NULL), sizeof(sacn_devices[j].address));
            j++;
            g_clear_pointer(&name, g_variant_unref);
            g_clear_pointer(&addr, g_variant_unref);
        }
        g_clear_object(&dev_proxy);
    }

    if (count == 0) {
        printf("未发现音频设备\n");
    }

    stop_scan_safe(data);
    g_main_loop_quit(data->loop); // 停止主事件循环
    return G_SOURCE_REMOVE;
}

void on_interfaces_added(GDBusConnection *connection,
                        const gchar *sender_name,
                        const gchar *object_path,
                        const gchar *interface_name,
                        const gchar *signal_name,
                        GVariant *parameters,
                        gpointer user_data)
{
    BluetoothData *data = (BluetoothData *)user_data;

    if (!g_variant_is_of_type(parameters, G_VARIANT_TYPE("(oa{sa{sv}})"))) {
        g_warning("收到无效信号格式");
        return;
    }

    const gchar *path = NULL;
    GVariant *interfaces_dict = NULL;
    g_variant_get(parameters, "(&o@a{sa{sv}})", &path, &interfaces_dict);

    GVariantIter interfaces_iter;
    g_variant_iter_init(&interfaces_iter, interfaces_dict);

    const gchar *interface;
    GVariant *properties_dict;
    while (g_variant_iter_next(&interfaces_iter, "{&s@a{sv}}", &interface, &properties_dict)) {
        if (g_strcmp0(interface, DEVICE_INTERFACE) == 0) {
            GError *error = NULL;
            GDBusProxy *dev_proxy = g_dbus_proxy_new_sync(
                data->conn,
                G_DBUS_PROXY_FLAGS_NONE,
                NULL,
                BLUEZ_SERVICE,
                path,
                DEVICE_INTERFACE,
                NULL,
                &error
            );

            if (!dev_proxy) {
                g_clear_error(&error);
                g_variant_unref(properties_dict);
                continue;
            }

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

            g_clear_object(&dev_proxy);
        }

        if (properties_dict) {
            g_variant_unref(properties_dict);
        }
    }

    g_clear_pointer(&interfaces_dict, g_variant_unref);
}

void start_scan(BluetoothData *data) {
    g_list_free_full(data->discovered_devices, g_free);
    data->discovered_devices = NULL;

    GError *error = NULL;
    GVariant *result = g_dbus_proxy_call_sync(
        data->adapter_proxy,
        "StartDiscovery",
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error
    );

    if (error) {
        fprintf(stderr, "扫描启动失败: %s\n", error->message);
        g_error_free(error);
        return;
    }
    g_clear_pointer(&result, g_variant_unref);

    data->scanning = TRUE;

    data->scan_timeout_id = g_timeout_add_seconds(12, scan_timeout_cb, data);

    data->scan_signal_id = g_dbus_connection_signal_subscribe(
        data->conn,
        BLUEZ_SERVICE,
        DBUS_OM_INTERFACE,
        "InterfacesAdded",
        NULL,
        NULL,
        G_DBUS_SIGNAL_FLAGS_NONE,
        on_interfaces_added,
        data,
        NULL
    );

    printf("正在扫描音频设备(12秒后自动停止)...\n");
}

void set_bluetooth_power(BluetoothData *data, gboolean power) {
    GError *error = NULL;
    GVariant *result = g_dbus_proxy_call_sync(
        data->adapter_proxy,
        "org.freedesktop.DBus.Properties.Set",
        g_variant_new("(ssv)", ADAPTER_INTERFACE, "Powered", g_variant_new_boolean(power)),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error
    );
    if (error) {
        fprintf(stderr, "设置电源状态失败: %s\n", error->message);
        g_error_free(error);
        return;
    }
    g_variant_unref(result);
    printf("蓝牙已%s\n", power ? "开启" : "关闭");
}

gboolean get_bluetooth_status(BluetoothData *data) {
    GError *error = NULL;
    GVariant *result = g_dbus_proxy_call_sync(
        data->adapter_proxy,
        "org.freedesktop.DBus.Properties.Get",
        g_variant_new("(ss)", ADAPTER_INTERFACE, "Powered"),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error
    );
    if (error) {
        fprintf(stderr, "获取状态失败: %s\n", error->message);
        g_error_free(error);
        return FALSE;
    }

    GVariant *inner;
    gboolean powered;
    g_variant_get(result, "(v)", &inner);
    powered = g_variant_get_boolean(inner);
    printf("蓝牙状态: %s\n", powered ? "已开启" : "已关闭");
    
    g_variant_unref(inner);
    g_variant_unref(result);
    return powered;
}
//获取已经连接的设备
void show_paired_devices(BTDevice *devices, int *count,BluetoothData *data) {
    GError *error = NULL;
    GVariant *result;
    //暂定十个，多了没办法，连那么多蓝牙干啥
    result = g_dbus_proxy_call_sync(
        data->om_proxy,
        "GetManagedObjects",
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error);

    if (!result) {
        fprintf(stderr, "调用 GetManagedObjects 失败: %s\n", error->message);
        g_error_free(error);
        return;
    }

    GVariantIter *objects;
    gchar *object_path;
    GVariant *interfaces_and_properties;

    g_variant_get(result, "(a{oa{sa{sv}}})", &objects);

    printf("已配对设备详细信息:\n");
    int i = 0;
    while (g_variant_iter_next(objects, "{&o@*}", &object_path, &interfaces_and_properties)) {
        GVariantIter *interface_iter;
        gchar *interface_name;
        GVariant *properties;

        g_variant_get(interfaces_and_properties, "a{sa{sv}}", &interface_iter);
        while (g_variant_iter_next(interface_iter, "{&s@*}", &interface_name, &properties)) {
            if (g_strcmp0(interface_name, "org.bluez.Device1") == 0) {
                GVariant *paired = g_variant_lookup_value(properties, "Paired", G_VARIANT_TYPE_BOOLEAN);
                GVariant *address = g_variant_lookup_value(properties, "Address", G_VARIANT_TYPE_STRING);
                GVariant *connected = g_variant_lookup_value(properties, "Connected", G_VARIANT_TYPE_BOOLEAN);
                GVariant *name = g_variant_lookup_value(properties, "Name", G_VARIANT_TYPE_STRING);

                if (paired && g_variant_get_boolean(paired)) {
                    const gchar *device_address = address ? g_variant_get_string(address, NULL) : "Unknown";
                    gboolean is_connected = connected ? g_variant_get_boolean(connected) : FALSE;
                    const gchar *device_name = name ? g_variant_get_string(name, NULL) : "Unknown";
                    
                    printf("  设备路径: %s\n", object_path);
                    printf("  名称: %s\n", device_name);
                    printf("  地址: %s\n", device_address);
                    printf("  已配对: 是\n");
                    printf("  已连接: %s\n", is_connected ? "是" : "否");
                    printf("----------------------------------------\n");
                    // 安全拷贝字符串
                    g_strlcpy(devices[i].name, device_name, sizeof(devices[i].name));
                    g_strlcpy(devices[i].address, device_address, sizeof(devices[i].address));
                    devices[i].is_connected = is_connected;
                    i++;
                }

                if (paired) g_variant_unref(paired);
                if (address) g_variant_unref(address);
                if (connected) g_variant_unref(connected);
                if (name) g_variant_unref(name);
            }
            g_variant_unref(properties);
        }

        g_variant_iter_free(interface_iter);
        g_variant_unref(interfaces_and_properties);
    }
    *count =  i;
    g_variant_iter_free(objects);
    g_variant_unref(result);
}

gboolean is_valid_mac(const char *mac) {
    int i;
    for (i = 0; i < 17; i++) {
        if ((i % 3 == 2 && mac[i] != ':') ||
            ((i % 3 != 2) && !g_ascii_isxdigit(mac[i]))) {
            return FALSE;
        }
    }
    return mac[17] == '\0';
}

gboolean device_exists(GDBusConnection *conn, const gchar *device_path, GError **error) {
    GVariant *result = g_dbus_connection_call_sync(
        conn,
        BLUEZ_SERVICE,
        "/",
        DBUS_OM_INTERFACE,
        "GetManagedObjects",
        NULL,
        G_VARIANT_TYPE("(a{oa{sa{sv}}})"),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        error
    );

    if (!result) {
        return FALSE;
    }

    GVariantIter *objects;
    g_variant_get(result, "(a{oa{sa{sv}}})", &objects);

    GVariant *interfaces;
    gchar *path;
    gboolean found = FALSE;

    while (g_variant_iter_next(objects, "{&o@a{sa{sv}}}", &path, &interfaces)) {
        if (g_strcmp0(path, device_path) == 0) {
            found = TRUE;
        }
        g_variant_unref(interfaces);
    }

    g_variant_unref(result);
    return found;
}

gboolean set_trust_property(GDBusConnection *conn, const gchar *device_path, GError **error) {
    GVariant *result = g_dbus_connection_call_sync(
        conn,
        BLUEZ_SERVICE,
        device_path,
        DBUS_PROPERTIES_INTERFACE,
        "Set",
        g_variant_new("(ssv)",
                      DEVICE_INTERFACE,
                      "Trusted",
                      g_variant_new_boolean(TRUE)),
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        error
    );

    if (result) {
        g_variant_unref(result);
        return TRUE;
    }

    return FALSE;
}

gboolean connect_profile(GDBusConnection *conn, const gchar *device_path, const gchar *uuid, GError **error) {
    GVariant *result = g_dbus_connection_call_sync(
        conn,
        BLUEZ_SERVICE,
        device_path,
        DEVICE_INTERFACE,
        "ConnectProfile",
        g_variant_new("(s)", uuid),
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        error
    );

    if (!result) {
        return FALSE;
    }

    g_variant_unref(result);
    return TRUE;
}

void pair_and_connect(BluetoothData *data, const char *mac) {
    char path[64];
    snprintf(path, sizeof(path), "/org/bluez/hci0/dev_%s", mac);
    for (int i = 0; path[i]; i++) if (path[i] == ':') path[i] = '_';

    GError *error = NULL;

    printf("尝试配对并连接 %s...\n", mac);

    if (!device_exists(data->conn, path, &error)) {
        fprintf(stderr, "设备不存在，请确保设备已开启并靠近本机。\n");
        if (error) {
            g_error_free(error);
        }
        return;
    }

    gboolean trust_set = set_trust_property(data->conn, path, &error);
    if (!trust_set) {
        fprintf(stderr, "设置信任状态失败: %s\n", error->message);
        g_error_free(error);
        return;
    }

    const char *profiles[] = {UUID_A2DP_SINK, UUID_AVRCP_TARGET};
    for (int i = 0; i < sizeof(profiles)/sizeof(profiles[0]); i++) {
        if (!connect_profile(data->conn, path, profiles[i], &error)) {
            if (g_strcmp0(profiles[i], UUID_A2DP_SINK) == 0) {
                fprintf(stderr, "连接 Profile (%s) 失败: %s\n", profiles[i], error->message);
                fprintf(stderr, "注意：A2DP 连接失败。可能是设备未启用音频输出模式，请在系统声音设置中选择 A2DP。\n");
            } else {
                fprintf(stderr, "连接 Profile (%s) 失败: %s\n", profiles[i], error->message);
            }
            g_error_free(error);
            error = NULL;
        } else {
            printf("成功连接 Profile (%s)\n", profiles[i]);
        }
    }
}

void disconnect_device(BluetoothData *data, const char *mac) {
    char path[64];
    snprintf(path, sizeof(path), "/org/bluez/hci0/dev_%s", mac);
    for (int i = 0; path[i]; i++) if (path[i] == ':') path[i] = '_';

    GError *error = NULL;
    GDBusProxy *dev_proxy = g_dbus_proxy_new_sync(
        data->conn,
        G_DBUS_PROXY_FLAGS_NONE,
        NULL,
        BLUEZ_SERVICE,
        path,
        DEVICE_INTERFACE,
        NULL,
        &error
    );
    if (error) {
        fprintf(stderr, "设备未找到: %s\n", error->message);
        g_error_free(error);
        return;
    }

    printf("正在断开 %s...\n", mac);
    GVariant *result = g_dbus_proxy_call_sync(
        dev_proxy,
        "Disconnect",
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error
    );
    if (error) {
        fprintf(stderr, "断开失败: %s\n", error->message);
        g_error_free(error);
    } else {
        printf("已断开连接\n");
        g_variant_unref(result);
    }
    g_object_unref(dev_proxy);
}
//如果蓝牙连接不上的时候有可能是没加这个
void register_agent(BluetoothData *data) {
    GError *error = NULL;
    GVariant *result = g_dbus_proxy_call_sync(
        data->agent_manager_proxy,
        "RegisterAgent",
        g_variant_new("(os)", "/test/agent", "NoInputNoOutput"),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error
    );
    if (error) {
        fprintf(stderr, "注册代理失败: %s\n", error->message);
        g_error_free(error);
        return;
    }
    g_variant_unref(result);

    result = g_dbus_proxy_call_sync(
        data->agent_manager_proxy,
        "RequestDefaultAgent",
        g_variant_new("(o)", "/test/agent"),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error
    );
    if (error) {
        fprintf(stderr, "设置默认代理失败: %s\n", error->message);
        g_error_free(error);
        return;
    }
    g_variant_unref(result);
}

void remove_device(BluetoothData *data, const char *mac) {
    char path[64];
    snprintf(path, sizeof(path), "/org/bluez/hci0/dev_%s", mac);
    for (int i = 0; path[i]; i++) if (path[i] == ':') path[i] = '_';

    GError *error = NULL;
    GVariant *result = g_dbus_proxy_call_sync(
        data->adapter_proxy,
        "RemoveDevice",
        g_variant_new("(o)", path),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error
    );
    if (error) {
        fprintf(stderr, "移除设备失败: %s\n", error->message);
        g_error_free(error);
    } else {
        printf("设备已移除\n");
        g_variant_unref(result);
    }
}