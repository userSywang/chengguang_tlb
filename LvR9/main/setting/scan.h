#ifndef SCAN_BIND_H
#define SCAN_BIND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl/lvgl.h>

    /*********************
     *      DEFINES
     *********************/
#define SCAN_STATE_COUNT 8

     /*********************
      *      TYPEDEFS
      *********************/
    typedef enum {
        SCAN_STATE_INITIAL,          // 初始状态
        SCAN_STATE_QR_LOADING,       // 二维码加载中
        SCAN_STATE_QR_DISPLAY,       // 二维码显示
        SCAN_STATE_QR_ERROR,         // 二维码异常
        SCAN_STATE_SCAN_SUCCESS,     // 扫码成功
        SCAN_STATE_GENERAL_LOADING,  // 通用加载
        SCAN_STATE_TIMEOUT,          // 30秒超时
        SCAN_STATE_SCAN_FAIL,        // 扫码失败
        SCAN_STATE_BIND_SUCCESS      // 绑定成功
    } ScanState;

    typedef struct {
        lv_obj_t* cont;             // 主容器
        lv_obj_t* title_label;       // 标题
        lv_obj_t* qr_img;            // 二维码图像
        lv_obj_t* status_label;      // 状态文本
        lv_obj_t* action_btn;        // 操作按钮
        lv_obj_t* loading_anim;      // 加载动画
        lv_obj_t* IMEI;              //IMEI文本
        lv_obj_t* reload_btn;          // 重新加载按钮
        lv_obj_t* network_switch_btn;  // 网络切换按钮
        lv_obj_t* back_btn;            // 返回按钮
        lv_obj_t* success_panel;       // 成功面板

        uint8_t network_check_count;  // 网络检测次数计数器
        ScanState current_state;     // 当前状态
        bool network_available;      // 网络状态
        lv_timer_t* timeout_timer;   // 超时定时器
        char user_nickname[32];      // 用户昵称
        lv_timer_t* delay_timer;
    } ScanBindContext;

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* SCAN_BIND_H */