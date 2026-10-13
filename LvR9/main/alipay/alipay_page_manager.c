#include "alipay_page_manager.h"

// 当前页面和上次页面状态
static alipay_page_t g_current_page = ALIPAY_PAGE_MAX;
static alipay_page_t g_previous_page = ALIPAY_PAGE_MAX;

// 页面函数表 (具体实现在alipay_menu.c)
extern const alipay_page_info_t page_table[];

// 页面切换函数
void alipay_page_switch(alipay_page_t new_page, int param) {
    // 检查页面是否有效
    if (new_page >= ALIPAY_PAGE_MAX) {
        return;
    }

    // 保存当前页面状态
    g_previous_page = g_current_page;

    // 释放当前页面资源
    if (g_current_page != ALIPAY_PAGE_MAX && page_table[g_current_page].free) {
        page_table[g_current_page].free();
    }

    // 初始化新页面
    if (page_table[new_page].init) {
        if (page_table[new_page].init(param) != 0) {
            // 失败时恢复前一页
            if (g_previous_page != ALIPAY_PAGE_MAX && page_table[g_previous_page].init) {
                page_table[g_previous_page].init(0);
                g_current_page = g_previous_page;
            }
            return;
        }
    }

    // 更新当前页面状态
    g_current_page = new_page;
}

// 初始化页面管理器
void alipay_page_init_manager(void) {
    // 默认显示主菜单页面
    alipay_page_switch(ALIPAY_PAGE_MENU, 0);
}


void alipay_page_free_all(void) {

    for (alipay_page_t page = ALIPAY_PAGE_MENU; page < ALIPAY_PAGE_MAX; page++) {
        if (page_table[page].free) {
            page_table[page].free();
        }
    }

    // 重置页面状态
    g_current_page = ALIPAY_PAGE_MAX;
    g_previous_page = ALIPAY_PAGE_MAX;
}


// 获取当前页面
alipay_page_t alipay_get_current_page(void) {
    return g_current_page;
}