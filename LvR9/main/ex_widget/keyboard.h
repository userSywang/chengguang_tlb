#include "lvgl/lvgl.h"

// 键盘模式枚举
typedef enum {
    KB_MODE_PINYIN,      // 拼音键盘（全字母布局）
    KB_MODE_EN_LOWER,    // 英文小写键盘
    KB_MODE_EN_UPPER,    // 英文大写键盘
    KB_MODE_NUMBER,      // 数字键盘（九宫格）
    KB_MODE_SYMBOL,       // 符号键盘
    KB_MODE_VOICE,      // 语音输入
    KB_MODE_COUNT       // 模式总数
} kb_mode_t;

// 键盘语言
typedef enum {
    KB_LANG_EN,     // 英文
    KB_LANG_ZH,     // 中文
    KB_LANG_COUNT   // 语言总数
} kb_language_t;

// 键盘数据结构
typedef struct {
    lv_obj_t* container;
    lv_obj_t* btnmatrix;
    lv_obj_t* switch_menu;
    kb_mode_t current_mode;
    kb_language_t current_lang;
    lv_obj_t* target_ta;
    bool caps_lock;
} custom_keyboard_t;



void setup_ui(lv_obj_t* parent);
void switch_menu_event_cb(lv_event_t* e);
void global_click_cb(lv_event_t* e);
void start_voice_input(void);
void set_keyboard_mode(custom_keyboard_t* kb, kb_mode_t mode);
void position_switch_menu(custom_keyboard_t* kb);