#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BTManage.h"

void show_menu(void) {
    printf("\n蓝牙控制菜单:\n");
    printf("1. 开启蓝牙\n");
    printf("2. 关闭蓝牙\n");
    printf("3. 获取蓝牙状态\n");
    printf("4. 扫描音频设备\n");
    printf("5. 显示已配对设备\n");
    printf("6. 配对并连接设备\n");
    printf("7. 断开设备连接\n");
    printf("8. 移除配对设备\n");
    printf("0. 退出\n");
    printf("请选择: ");
}

int main() {
    BluetoothData data = {0};
    data.loop = g_main_loop_new(NULL, FALSE);
    init_dbus(&data);
    register_agent(&data);  // 注册代理处理配对请求
    gboolean state = 0;
    int choice;
    char mac[18];
    int cc = 0;
    int device_count = 0;
    BTDevice devices[10];

    while (1) {
        show_menu();
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("输入无效，请重试\n");
            continue;
        }
        
        switch (choice) {
            case 1:
                set_bluetooth_power(&data, TRUE);
                break;
            case 2:
                set_bluetooth_power(&data, FALSE);
                break;
            case 3:
                state = get_bluetooth_status(&data);
                break;
            case 4:
                // 清空上一次扫描结果
                g_list_free_full(data.discovered_devices, g_free);
                data.discovered_devices = NULL;
                
                start_scan(&data);
                //扫描肯定需要的一个阻塞功能，使用的时候这部分可以开个线程去获取
                g_main_loop_run(data.loop);
                BTDevice devices[SCANF_COUNT];
                int device_count = get_scanned_devices(devices, SCANF_COUNT);
                printf("----------------------------------------\n");
                for (int i = 0; i < device_count; i++) {
                    printf("%d. 名称：%s,地址:%s\n", i + 1, devices[i].name, devices[i].address);
                }
                break;
            case 5:
                show_paired_devices(devices, &device_count,&data);
                printf("----------------------------------------\n");
                for(int i = 0;i < device_count;i ++)
                {
                    printf("  名称: %s\n", devices[i].name);
                    printf("  地址: %s\n", devices[i].address);
                    printf("  已连接: %s\n", devices[i].is_connected ? "是" : "否");
                }
                break;
            case 6:
                printf("输入MAC地址(格式: XX:XX:XX:XX:XX:XX): ");
                if (scanf("%17s", mac) == 1) {
                    pair_and_connect(&data, mac);
                }
                while (getchar() != '\n');
                break;
            case 7:
                printf("输入要断开的MAC地址: ");
                if (scanf("%17s", mac) == 1) {
                    disconnect_device(&data, mac);
                }
                while (getchar() != '\n');
                break;
            case 8:
                printf("输入要移除的MAC地址: ");
                if (scanf("%17s", mac) == 1) {
                    remove_device(&data, mac);
                }
                while (getchar() != '\n');
                break;
            case 0:
                stop_scan_safe(&data);
                g_list_free_full(data.discovered_devices, g_free);
                g_list_free_full(data.paired_devices, g_free);
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