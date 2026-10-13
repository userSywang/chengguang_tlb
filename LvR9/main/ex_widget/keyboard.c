#include"keyboard.h"

// 样式定义
static lv_style_t style_kb_bg;
static lv_style_t style_kb_btn;
static lv_style_t style_kb_btn_pr;
static lv_style_t style_kb_special;
static lv_style_t style_kb_active;
static lv_style_t style_switch_menu;
static lv_style_t style_switch_btn;

// 图片资源声明
LV_IMG_DECLARE(icon_keyboard);
LV_IMG_DECLARE(icon_microphone);
LV_IMG_DECLARE(icon_backspace);
LV_IMG_DECLARE(icon_enter);
LV_IMG_DECLARE(icon_shift);
LV_IMG_DECLARE(icon_caps);
LV_IMG_DECLARE(icon_voice);

// 键盘布局映射
static const char* kb_map_en_lower[] = {
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
    "a", "s", "d", "f", "g", "h", "j", "k", "l", "\n",
    "#shift", "z", "x", "c", "v", "b", "n", "m", "#back", "\n",
    "#123", "#voice", "#space", ".", "#enter", ""
};

static const char* kb_map_en_upper[] = {
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "\n",
    "A", "S", "D", "F", "G", "H", "J", "K", "L", "\n",
    "#caps", "Z", "X", "C", "V", "B", "N", "M", "#back", "\n",
    "#123", "#voice", "#space", ".", "#enter", ""
};

static const char* kb_map_number[] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",
    "-", "/", ":", ";", "(", ")", "$", "&", "@", "\"", "\n",
    "#sym", ".", ",", "?", "!", "'", "#back", "\n",
    "#abc", "#voice", "#space", "#enter", ""
};

static const char* kb_map_symbol[] = {
    "[", "]", "{", "}", "#", "%", "^", "*", "+", "=", "\n",
    "_", "\\", "|", "~", "<", ">", "€", "£", "¥", "•", "\n",
    "#abc", ".", ",", "?", "!", "'", "#back", "\n",
    "#123", "#voice", "#space", "#enter", ""
};

static const char* kb_map_pinyin[] = {
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
    "a", "s", "d", "f", "g", "h", "j", "k", "l", "\n",
    "#zh_en", "z", "x", "c", "v", "b", "n", "m", "#back", "\n",
    "#123", "#voice", "#space", ".", "#enter", ""
};

static const char* kb_map_voice[] = {
    "#mic", "\n",
    "#keyboard", ""
};


// 控制映射
static const lv_btnmatrix_ctrl_t kb_ctrl_default[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    2 | LV_BTNMATRIX_CTRL_POPOVER, 0, 0, 0, 0, 0, 0, 0, 2 | LV_BTNMATRIX_CTRL_POPOVER,
    2 | LV_BTNMATRIX_CTRL_POPOVER, 2 | LV_BTNMATRIX_CTRL_POPOVER, 5, 0, 2 | LV_BTNMATRIX_CTRL_POPOVER
};

static const lv_btnmatrix_ctrl_t kb_ctrl_voice[] = {
    10 | LV_BTNMATRIX_CTRL_POPOVER,
    0,
    2 | LV_BTNMATRIX_CTRL_POPOVER, 0
};

// 初始化样式
void init_keyboard_styles(void) {
    // 键盘背景样式
    lv_style_init(&style_kb_bg);
    lv_style_set_bg_color(&style_kb_bg, lv_color_hex(0x2C2C2E));
    lv_style_set_radius(&style_kb_bg, 12);
    lv_style_set_pad_all(&style_kb_bg, 5);
    lv_style_set_border_width(&style_kb_bg, 0);
    lv_style_set_shadow_width(&style_kb_bg, 20);
    lv_style_set_shadow_ofs_y(&style_kb_bg, 2);
    lv_style_set_shadow_opa(&style_kb_bg, LV_OPA_30);

    // 普通按键样式
    lv_style_init(&style_kb_btn);
    lv_style_set_bg_opa(&style_kb_btn, LV_OPA_100);
    lv_style_set_bg_color(&style_kb_btn, lv_color_hex(0x3A3A3C));
    lv_style_set_text_color(&style_kb_btn, lv_color_white());
    //lv_style_set_text_font(&style_kb_btn, &lv_font_montserrat_20);
    lv_style_set_pad_all(&style_kb_btn, 4);
    lv_style_set_radius(&style_kb_btn, 6);

    // 按键按下状态样式
    lv_style_init(&style_kb_btn_pr);
    lv_style_set_bg_color(&style_kb_btn_pr, lv_color_hex(0x48484A));
    //lv_style_set_transition(&style_kb_btn_pr, &lv_transition_del_linear, 80, 80);

    // 特殊按键样式
    lv_style_init(&style_kb_special);
    lv_style_set_bg_color(&style_kb_special, lv_color_hex(0x4A4A4D));
    lv_style_set_text_color(&style_kb_special, lv_color_hex(0x9A9A9C));

    // 激活状态按键
    lv_style_init(&style_kb_active);
    lv_style_set_bg_color(&style_kb_active, lv_color_hex(0x0A84FF));
    lv_style_set_text_color(&style_kb_active, lv_color_white());

    // 切换菜单样式
    lv_style_init(&style_switch_menu);
    lv_style_set_bg_color(&style_switch_menu, lv_color_hex(0x1C1C1E));
    lv_style_set_radius(&style_switch_menu, 12);
    lv_style_set_pad_all(&style_switch_menu, 8);
    lv_style_set_shadow_width(&style_switch_menu, 20);
    lv_style_set_shadow_opa(&style_switch_menu, LV_OPA_40);

    // 切换按钮样式
    lv_style_init(&style_switch_btn);
    lv_style_set_bg_opa(&style_switch_btn, LV_OPA_0);
    lv_style_set_text_color(&style_switch_btn, lv_color_white());
   // lv_style_set_text_font(&style_switch_btn, &lv_font_montserrat_16);
    lv_style_set_pad_ver(&style_switch_btn, 12);
    lv_style_set_pad_hor(&style_switch_btn, 15);
}

// 创建键盘切换菜单
lv_obj_t* create_switch_menu(lv_obj_t* parent) {
    lv_obj_t* menu = lv_obj_create(parent);
    lv_obj_add_style(menu, &style_switch_menu, 0);
    lv_obj_set_size(menu, LV_PCT(70), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(menu, LV_OBJ_FLAG_HIDDEN);

    // 添加切换选项
    const char* labels[] = { "英文键盘", "数字键盘", "拼音输入", "符号键盘", "语音输入" };
    kb_mode_t modes[] = { KB_MODE_EN_LOWER, KB_MODE_NUMBER, KB_MODE_PINYIN, KB_MODE_SYMBOL, KB_MODE_VOICE };

    for (int i = 0; i < 5; i++) {
        lv_obj_t* btn = lv_btn_create(menu);
        lv_obj_add_style(btn, &style_switch_btn, 0);

        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, labels[i]);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);

        // 存储模式信息
        lv_obj_set_user_data(btn, (void*)(intptr_t)modes[i]);

        // 添加分隔线（最后一个除外）
        if (i < 4) {
            lv_obj_t* line = lv_line_create(menu);
            static lv_point_t points[] = { {0, 0}, {220, 0} };
            lv_line_set_points(line, points, 2);
            lv_obj_set_style_line_width(line, 1, 0);
            lv_obj_set_style_line_color(line, lv_color_hex(0x444444), 0);
        }
    }

    return menu;
}

// 键盘绘制回调
void draw_keyboard_event_cb(lv_event_t* e) {
    lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
    if (dsc->part != LV_PART_ITEMS) return;

    custom_keyboard_t* kb = lv_event_get_user_data(e);
    const char* txt = lv_btnmatrix_get_btn_text(kb->btnmatrix, dsc->id);
    if (!txt) return;

    //// 处理特殊按键图标
    //if (strcmp(txt, "#back") == 0) {
    //    // 绘制删除图标
    //    dsc->label_dsc->text = "S:" LV_SYMBOL_BACKSPACE;
    //    dsc->label_dsc->color = lv_color_hex(0xAAAAAA);
    //}
    //else if (strcmp(txt, "#shift") == 0) {
    //    // 绘制Shift图标
    //    dsc->label_dsc->text = "S:" LV_SYMBOL_UP;
    //    dsc->label_dsc->color = lv_color_hex(0xCCCCCC);
    //}
    //else if (strcmp(txt, "#caps") == 0) {
    //    // 绘制Caps图标
    //    dsc->label_dsc->text = "S:" LV_SYMBOL_KEYBOARD;
    //    dsc->label_dsc->color = lv_color_hex(0x0A84FF);
    //}
    //else if (strcmp(txt, "#voice") == 0) {
    //    // 绘制语音图标
    //    dsc->label_dsc->text = "S:" LV_SYMBOL_AUDIO;
    //    dsc->label_dsc->color = lv_color_hex(0x34C759);
    //}
    //else if (strcmp(txt, "#mic") == 0) {
    //    // 绘制麦克风大图标
    //    dsc->label_dsc->text = "S:" LV_SYMBOL_MIC;
    //    dsc->label_dsc->font = &lv_font_montserrat_28;
    //    dsc->label_dsc->color = lv_color_hex(0x34C759);
    //}
    //else if (strcmp(txt, "#keyboard") == 0) {
    //    // 绘制键盘切换图标
    //    dsc->label_dsc->text = "S:" LV_SYMBOL_KEYBOARD;
    //}
    //else if (txt[0] == '#') {
    //    // 通用特殊按键样式
    //    lv_obj_set_style_local_bg_color(dsc->obj, LV_PART_ITEMS, dsc->id,
    //        lv_color_hex(0x424245), 0);
    //    dsc->label_dsc->color = lv_color_hex(0x0A84FF);
    //}

    //// 高亮显示当前模式切换按钮
    //if (strcmp(txt, "#123") == 0 && kb->current_mode == KB_MODE_NUMBER) {
    //    lv_obj_set_style_local_bg_color(dsc->obj, LV_PART_ITEMS, dsc->id,
    //        lv_color_hex(0x0A84FF), 0);
    //    dsc->label_dsc->color = lv_color_white();
    //}
    //else if (strcmp(txt, "#sym") == 0 && kb->current_mode == KB_MODE_SYMBOL) {
    //    lv_obj_set_style_local_bg_color(dsc->obj, LV_PART_ITEMS, dsc->id,
    //        lv_color_hex(0x0A84FF), 0);
    //    dsc->label_dsc->color = lv_color_white();
    //}
    //else if (strcmp(txt, "#abc") == 0 &&
    //    (kb->current_mode == KB_MODE_EN_LOWER || kb->current_mode == KB_MODE_EN_UPPER)) {
    //    lv_obj_set_style_local_bg_color(dsc->obj, LV_PART_ITEMS, dsc->id,
    //        lv_color_hex(0x0A84FF), 0);
    //    dsc->label_dsc->color = lv_color_white();
    //}
    //else if (strcmp(txt, "#zh_en") == 0 && kb->current_mode == KB_MODE_PINYIN) {
    //    lv_obj_set_style_local_bg_color(dsc->obj, LV_PART_ITEMS, dsc->id,
    //        lv_color_hex(0x0A84FF), 0);
    //    dsc->label_dsc->color = lv_color_white();
    //}
    //else if (strcmp(txt, "#voice") == 0 && kb->current_mode == KB_MODE_VOICE) {
    //    lv_obj_set_style_local_bg_color(dsc->obj, LV_PART_ITEMS, dsc->id,
    //        lv_color_hex(0x0A84FF), 0);
    //    dsc->label_dsc->color = lv_color_white();
    //}
}

// 键盘事件处理
void keyboard_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    custom_keyboard_t* kb = lv_event_get_user_data(e);
    lv_obj_t* obj = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        uint16_t btn_id = lv_btnmatrix_get_selected_btn(obj);
        if (btn_id == LV_BTNMATRIX_BTN_NONE) return;

        const char* txt = lv_btnmatrix_get_btn_text(obj, btn_id);
        if (!txt) return;

        // 处理特殊按键
        if (strcmp(txt, "#back") == 0) {
            if (kb->target_ta) {
                lv_textarea_del_char(kb->target_ta);
            }
        }
        else if (strcmp(txt, "#shift") == 0) {
            kb->current_mode = KB_MODE_EN_UPPER;
            lv_btnmatrix_set_map(kb->btnmatrix, kb_map_en_upper);
        }
        else if (strcmp(txt, "#caps") == 0) {
            kb->caps_lock = !kb->caps_lock;
            if (kb->caps_lock) {
                lv_btnmatrix_set_map(kb->btnmatrix, kb_map_en_upper);
            }
            else {
                lv_btnmatrix_set_map(kb->btnmatrix, kb_map_en_lower);
            }
        }
        else if (strcmp(txt, "#enter") == 0) {
            if (kb->target_ta) {
                lv_textarea_add_char(kb->target_ta, '\n');
            }
        }
        else if (strcmp(txt, "#space") == 0) {
            if (kb->target_ta) {
                lv_textarea_add_char(kb->target_ta, ' ');
            }
        }
        else if (strcmp(txt, "#123") == 0) {
            kb->current_mode = KB_MODE_NUMBER;
            lv_btnmatrix_set_map(kb->btnmatrix, kb_map_number);
            lv_btnmatrix_set_ctrl_map(kb->btnmatrix, kb_ctrl_default);
        }
        else if (strcmp(txt, "#abc") == 0) {
            if (kb->caps_lock) {
                kb->current_mode = KB_MODE_EN_UPPER;
                lv_btnmatrix_set_map(kb->btnmatrix, kb_map_en_upper);
            }
            else {
                kb->current_mode = KB_MODE_EN_LOWER;
                lv_btnmatrix_set_map(kb->btnmatrix, kb_map_en_lower);
            }
        }
        else if (strcmp(txt, "#sym") == 0) {
            kb->current_mode = KB_MODE_SYMBOL;
            lv_btnmatrix_set_map(kb->btnmatrix, kb_map_symbol);
            lv_btnmatrix_set_ctrl_map(kb->btnmatrix, kb_ctrl_default);
        }
        else if (strcmp(txt, "#zh_en") == 0) {
            kb->current_mode = KB_MODE_EN_LOWER;
            lv_btnmatrix_set_map(kb->btnmatrix, kb_map_en_lower);
        }
        else if (strcmp(txt, "#voice") == 0) {
            kb->current_mode = KB_MODE_VOICE;
            lv_btnmatrix_set_map(kb->btnmatrix, kb_map_voice);
            lv_btnmatrix_set_ctrl_map(kb->btnmatrix, kb_ctrl_voice);
        }
        else if (strcmp(txt, "#keyboard") == 0) {
            // 显示键盘切换菜单
            if (kb->switch_menu) {
                lv_obj_clear_flag(kb->switch_menu, LV_OBJ_FLAG_HIDDEN);
                lv_obj_align_to(kb->switch_menu, obj, LV_ALIGN_OUT_BOTTOM_LEFT,
                    lv_obj_get_style_pad_left(obj, LV_PART_MAIN),
                    5);
            }
        }
        else if (strcmp(txt, "#mic") == 0) {
            // 语音输入处理
            start_voice_input();
        }
        else {
            // 普通文本输入
            if (kb->target_ta && txt[0] != '#') {
                lv_textarea_add_text(kb->target_ta, txt);

                // 在拼音模式自动切换回英文
                if (kb->current_mode == KB_MODE_PINYIN) {
                    kb->current_mode = KB_MODE_EN_LOWER;
                    lv_btnmatrix_set_map(kb->btnmatrix, kb_map_en_lower);
                }
            }
        }

        // 刷新键盘
        lv_obj_invalidate(obj);
    }
}

// 创建自定义键盘
custom_keyboard_t* create_custom_keyboard(lv_obj_t* parent) {
    static bool styles_initialized = false;
    if (!styles_initialized) {
        init_keyboard_styles();
        styles_initialized = true;
    }

    custom_keyboard_t* kb = lv_mem_alloc(sizeof(custom_keyboard_t));
    memset(kb, 0, sizeof(custom_keyboard_t));

    // 创建容器
    kb->container = lv_obj_create(parent);
    lv_obj_remove_style_all(kb->container);
    lv_obj_set_size(kb->container, LV_PCT(100), 220);
    lv_obj_set_flex_flow(kb->container, LV_FLEX_FLOW_COLUMN);
    lv_obj_align(kb->container, LV_ALIGN_BOTTOM_MID, 0, 0);

    // 创建键盘切换菜单
    kb->switch_menu = create_switch_menu(lv_layer_top());

    // 创建按钮矩阵
    kb->btnmatrix = lv_btnmatrix_create(kb->container);
    lv_obj_add_style(kb->btnmatrix, &style_kb_bg, 0);
    lv_obj_set_size(kb->btnmatrix, LV_PCT(100), LV_PCT(85));

    // 设置默认样式
    lv_obj_add_style(kb->btnmatrix, &style_kb_btn, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_add_style(kb->btnmatrix, &style_kb_btn_pr, LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_add_style(kb->btnmatrix, &style_kb_special, LV_PART_ITEMS | LV_STATE_DEFAULT);

    // 初始为英文小写键盘
    kb->current_mode = KB_MODE_EN_LOWER;
    kb->caps_lock = false;
    lv_btnmatrix_set_map(kb->btnmatrix, kb_map_en_lower);
    lv_btnmatrix_set_ctrl_map(kb->btnmatrix, kb_ctrl_default);

    // 设置按键高度
   // lv_btnmatrix_set_btn_width(kb->btnmatrix, 5, 5); // 空格键宽度
   // lv_btnmatrix_set_one_checked(kb->btnmatrix, true);

    // 添加事件
    lv_obj_add_event_cb(kb->btnmatrix, keyboard_event_cb, LV_EVENT_VALUE_CHANGED, kb);
    lv_obj_add_event_cb(kb->btnmatrix, draw_keyboard_event_cb, LV_EVENT_DRAW_PART_BEGIN, kb);

    // 添加切换菜单的事件处理
    lv_obj_add_event_cb(kb->switch_menu, switch_menu_event_cb, LV_EVENT_VALUE_CHANGED, kb);

    return kb;
}

// 切换菜单事件处理
void switch_menu_event_cb(lv_event_t* e) {
    lv_obj_t* menu = lv_event_get_target(e);
    custom_keyboard_t* kb = lv_event_get_user_data(e);

    uint16_t btn_id = lv_btnmatrix_get_selected_btn(menu);
    if (btn_id == LV_BTNMATRIX_BTN_NONE) return;

   // lv_obj_t* btn = lv_btnmatrix_get_btn_controller(menu, btn_id);
    
    lv_obj_t* btn = NULL;
    if (!btn) return;

    // 获取存储的模式
    kb_mode_t mode = (kb_mode_t)(intptr_t)lv_obj_get_user_data(btn);

    // 更新键盘模式
    switch (mode) {
    case KB_MODE_EN_LOWER:
        lv_btnmatrix_set_map(kb->btnmatrix, kb_map_en_lower);
        lv_btnmatrix_set_ctrl_map(kb->btnmatrix, kb_ctrl_default);
        kb->current_mode = KB_MODE_EN_LOWER;
        break;
    case KB_MODE_NUMBER:
        lv_btnmatrix_set_map(kb->btnmatrix, kb_map_number);
        lv_btnmatrix_set_ctrl_map(kb->btnmatrix, kb_ctrl_default);
        kb->current_mode = KB_MODE_NUMBER;
        break;
    case KB_MODE_PINYIN:
        lv_btnmatrix_set_map(kb->btnmatrix, kb_map_pinyin);
        lv_btnmatrix_set_ctrl_map(kb->btnmatrix, kb_ctrl_default);
        kb->current_mode = KB_MODE_PINYIN;
        break;
    case KB_MODE_SYMBOL:
        lv_btnmatrix_set_map(kb->btnmatrix, kb_map_symbol);
        lv_btnmatrix_set_ctrl_map(kb->btnmatrix, kb_ctrl_default);
        kb->current_mode = KB_MODE_SYMBOL;
        break;
    case KB_MODE_VOICE:
        lv_btnmatrix_set_map(kb->btnmatrix, kb_map_voice);
        lv_btnmatrix_set_ctrl_map(kb->btnmatrix, kb_ctrl_voice);
        kb->current_mode = KB_MODE_VOICE;
        break;
    default:
        break;
    }

    // 隐藏菜单
    lv_obj_add_flag(menu, LV_OBJ_FLAG_HIDDEN);
}

// 设置关联的文本区域
void keyboard_set_textarea(custom_keyboard_t* kb, lv_obj_t* ta) {
    kb->target_ta = ta;
}

// 使用示例
void setup_ui(lv_obj_t* parent) {
    // 创建文本区域
    lv_obj_t* textarea = lv_textarea_create(parent);
    lv_obj_set_size(textarea, LV_PCT(90), 100);
    lv_obj_align(textarea, LV_ALIGN_TOP_MID, 0, 20);

    // 创建键盘
    custom_keyboard_t* kb = create_custom_keyboard(parent);

    // 关联文本区域
    keyboard_set_textarea(kb, textarea);

    // 添加全局点击事件以关闭切换菜单
    lv_obj_add_event_cb(lv_scr_act(), global_click_cb, LV_EVENT_CLICKED, kb);
}

// 全局点击事件关闭菜单
void global_click_cb(lv_event_t* e) {
    custom_keyboard_t* kb = lv_event_get_user_data(e);
    if (kb->switch_menu && !lv_obj_has_flag(kb->switch_menu, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(kb->switch_menu, LV_OBJ_FLAG_HIDDEN);
    }
}

// 语音输入处理（伪代码）
void start_voice_input(void) {
//    // 显示语音输入UI
//    show_voice_input_ui();
//
//    // 启动语音识别
//#if defined(USE_VOICE_RECOGNITION)
//    start_voice_recognition();
//#else
//// 模拟语音识别结果
//    const char* result = "这是语音输入的文本";
//    if (kb->target_ta) {
//        lv_textarea_add_text(kb->target_ta, result);
//    }
//#endif
//
//    // 切换回正常键盘
//    kb->current_mode = KB_MODE_EN_LOWER;
//    lv_btnmatrix_set_map(kb->btnmatrix, kb_map_en_lower);
}