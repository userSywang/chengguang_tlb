#ifndef _CONTACT_AP_H_
#define _CONTACT_AP_H_


#include "stdint.h"
#include "stdbool.h"
#include "contactAP_data.h"
#include <lvgl/lvgl.h>

// 定义屏幕类型
enum tbcd_screen_e {
    TBCD_SCR_NO_CONTENT,
    TBCD_SCR_ADD_CONTACT,
    TBCD_SCR_CALLING,
    TBCD_SCR_INCOMING_CALL,
    TBCD_SCR_SOS,
    TBCD_SCR_DIALPAD //拨号键盘
};


typedef enum {
    SOS_CALL_IDLE,      // 空闲状态
    SOS_CALL_ACTIVE,    // 正在拨打
    SOS_CALL_CONNECTED, // 已接通
    SOS_CALL_FAILED     // 拨打失败
} SOS_CallState;

// 定义电话状态机的事件类型
typedef enum {
    EVENT_CALL_OUT,   // 拨打事件
    EVENT_INCOMING,   // 来电事件
    EVENT_ANSWER,     // 接听事件
} CallEvent;

#define TBCD_CONTACT_MAX  4
#define TBCD_AP_TITLE "通讯录"

#define CONTACT_FILE_PATH                "/userdata/_data/contact_record.bin"


typedef struct {
    lv_obj_t* title;
    lv_obj_t* dialpad_cont;    // 拨号界面容器
    lv_obj_t* input_label;     // 输入显示标签
    lv_obj_t* grid;            // 数字键盘网格
    lv_obj_t* call_btn;        // 通话按钮
    lv_obj_t* clear_btn;       // 清除按钮

} TBCD_DIALPAD_INF;

// 新增结构体：导航栏相关组件
typedef struct {
    lv_obj_t* nav_bar;         // 导航栏容器
    lv_obj_t* contact_btn;     // 通讯录按钮
    lv_obj_t* dialpad_btn;     // 拨号键盘按钮
    lv_obj_t* contact_img;     // 通讯录图标
    lv_obj_t* dialpad_img;     // 拨号键盘图标
    lv_obj_t* contact_text;    // 通讯录标签
    lv_obj_t* dialpad_text;    // 拨号键盘标签

    lv_img_dsc_t* ADD_res;
    lv_img_dsc_t* DUALPAD_res;
} TBCD_NAV_INF;


// 联系人条目UI元素
typedef struct {
    lv_obj_t* item;      // 条目容器
    lv_obj_t* label;     // 文本标签
    lv_obj_t* del_btn;   // 删除按钮
    TBCD_CONTACT_RECORD_ITEM* data;       // 关联的数据
} ContactItem;


// 视图结构体
typedef struct {
    lv_obj_t* root;         // 视图根对象
    lv_obj_t* content;      // 内容容器
    bool is_created;        // 是否已创建
    void (*init)(void);    // 初始化函数
    void (*show)(void);     // 显示函数
    void (*hide)(void);     // 隐藏函数
} AppView;

// 联系人信息结构体（按功能模块分层）
typedef struct {
    // 共享数据
    TBCD_CONTACT_RECORD_ITEM contacts[TBCD_CONTACT_MAX];
    int16_t contact_count;
    char file_path[256];
    lv_obj_t* labelInf;
    lv_obj_t* root;
    lv_obj_t* g_tabview;

    // 通讯录列表页
    struct {
        lv_obj_t* labelPos;
        lv_obj_t* btnCall;
        lv_obj_t* btnSos;
        ContactItem* contactItem[TBCD_CONTACT_MAX];
    } contactList;

    // 新增联系人页
    struct {
        lv_obj_t* phone_input;
        lv_obj_t* dropdown;
        lv_obj_t* cancel_btn;
        lv_obj_t* complete_btn;
        lv_obj_t* avatar_containers[3];
        lv_obj_t* avatars[3];
        lv_img_dsc_t * avatar_img[3];
        lv_obj_t* keyboard;

        uint8_t selected_avatar_index;
        uint8_t curCont_Index;

        bool scrStat;//页面状态：1：新增  0：编辑 
    } addContact;

    // 通讯录界面组件
    struct {
        lv_obj_t* container;  // 通讯录主容器
        lv_obj_t* list;       // 联系人列表
        lv_obj_t* add_btn;    // 添加按钮
        lv_obj_t* return_btn; // 返回按钮
        // 其他通讯录特有组件...
    } contact_ui;

    // 拨号盘界面组件
    struct {
        lv_obj_t* container;  // 拨号盘主容器
        lv_obj_t* input_label;// 输入显示
        lv_obj_t* grid;       // 数字键盘
        lv_obj_t* call_btn;   // 拨号按钮
        // 其他拨号盘特有组件...
    } dialpad_ui;

    lv_timer_t* sos_timer; // 新增超时定时器
    SOS_CallState sos_state;
    uint8_t current_sos_index;
    TBCD_CONTACT_RECORD_ITEM sos_contacts[TBCD_CONTACT_MAX];


    TBCD_DIALPAD_INF dialpad;  // 拨号键盘组件
    TBCD_NAV_INF nav;          // 导航栏组件

} TBCD_CONTACT_INF;
// 全局变量
extern TBCD_CONTACT_INF g_pContactAp;


// 通话页
typedef struct {
    lv_obj_t* root;
    lv_obj_t* labelName;
    lv_obj_t* btnHangup;

    lv_timer_t* call_timer;      // 通话计时器
    uint32_t call_start_time;    // 通话开始时间戳（毫秒）
    lv_obj_t* call_time_label;   // 时间显示标签

} calling;

extern calling  calling_strct;

// SOS页
typedef struct {
    lv_obj_t* root;
    lv_obj_t* btnConfirm;
    lv_obj_t* btnCancel;
} sos;

extern sos  sos_strct;

typedef struct {
    lv_obj_t* popup;
    //lv_obj_t* phone_input; //号码来电
    lv_obj_t* compile_btn;
    lv_obj_t* call_btn;
    lv_obj_t* uncall_btn;
}TBCD_POP_UP_INF;

extern TBCD_POP_UP_INF* g_popUp;
extern TBCD_POP_UP_INF  g_popUp_ask;



// 屏幕切换
extern void tbcd_switchScr(int8_t scrNoNew, int parm);
extern int tbcd_calling_scr_init(CallEvent param);
extern int tbcd_sos_scr_init(int param);
// 数据操作
extern int tbcd_load_contact_record(const char* file_path);
extern int tbcd_save_contact_record(const char* file_path);
extern int tbcd_incoming_call_scr_init(int param);
extern void _tbcd_no_pickup_cb();

extern void free_calling_scr();
extern void update_calling_timer_create();

extern void seed_phone_Nocall();

#endif	