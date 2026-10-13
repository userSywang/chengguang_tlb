#ifndef ALIPAY_PAGE_MANAGER_H
#define ALIPAY_PAGE_MANAGER_H

#include "../lvgl_main.h"
#include <lvgl/lvgl.h>


// 页面类型枚举
typedef enum {
    ALIPAY_PAGE_MENU,        // 主菜单页面
    ALIPAY_PAGE_PAYMENT,     // 付款码页面 
    ALIPAY_PAGE_BIND,        // 扫码绑定页面
    ALIPAY_PAGE_SETTINGS,    // 新增设置页面
    ALIPAY_PAGE_MAX          // 页面总数
} alipay_page_t;



// 按钮属性结构体 280 * 48
typedef struct {
    const char* text;         // 按钮文本
    lv_color_t bg_color;      // 背景颜色
    lv_color_t text_color;    // 文字颜色
    lv_event_cb_t callback;   // 事件回调
} button_config_t;

// 文本标题属性结构体
typedef struct {
    const char* text;         // 文本内容
    int font_size;            // 字体大小
    lv_color_t text_color;    // 文字颜色
    lv_coord_t y_offset;      // Y轴偏移量
} text_config_t;






// 页面初始化函数类型
typedef int (*alipay_page_init_f)(int param);

// 页面资源释放函数类型
typedef int (*alipay_page_free_f)(void);

// 页面信息结构
typedef struct {
    alipay_page_init_f init;  // 页面初始化函数
    alipay_page_free_f free;  // 页面资源释放函数
} alipay_page_info_t;

// 页面管理函数
void alipay_page_switch(alipay_page_t new_page, int param);
void alipay_page_init_manager(void);
void alipay_page_free_all(void);

// 获取当前页面类型
alipay_page_t alipay_get_current_page(void);

// 图像资源ID定义 (根据实际项目调整)
enum {
    QRCODE_PAYMENT,     // 付款二维码
    QRCODE_BINDING      // 绑定二维码
};
#endif