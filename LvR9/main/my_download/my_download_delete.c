
#include <sys/stat.h>

#ifndef C_PC_SIM
#endif

#include <lvgl/lvgl.h>
//#include "../components/lvgl__lvgl/examples/lv_examples.h"
#include <lvgl/src/misc/lv_types.h>
#include "../lvgl_main.h"
#include "my_download.h"
//#include "../upd_data_parse/favorites.h"
#include "../data_parse/img_res.h"
#include "../utils/misc_utils.h"
#include "../ex_widget/app_msg.h"

//--------------------------------------
#define TAG "my_download_delete.c"

#define C_MAIN_WIN_W   SCREEN_WIDTH  //窗口宽度
#define C_FORM_PADDING_W  1          //设置窗口或控件的填充宽度


static lv_obj_t* img;
static lv_obj_t* mainWin;
static lv_obj_t* g_mainWin;

static lv_obj_t** label_tmp = NULL;  // note: num is NUMBER_FILES_MAX 
static lv_obj_t** g_btns = NULL; // note: num is NUMBER_FILES_MAX
static lv_obj_t** checkbox_albums = NULL;  // note: num is NUMBER_FILES_MAX
static bool * checkbox_status_albums;//复选框状态

static short g_curBtn = 0;//设为0，要不然第一次进入这个界面后，焦点在这个按钮上，却选不中复选框
static bool please_select_pop_up_presence_flag = 0;//请选择弹窗存在标记

//--------------------------------------
static void No_content_situation_window_layout(void);//无内容情况窗口布局
static void No_content_reminder(void);//暂无内容提示
static void delDlgSelectProc(void); //消息框回调函数中的 定时器回调函数：消息框的取消与确定按钮处理事件
static void _my_btn_checkbox_status_cb(lv_event_t* e);//按钮回调函数 控制复选框状态

static int delete_button_callback_function(lv_event_t* e);//删除按钮回调函数
static void _my_Confirm_delete_pop_ups(lv_event_t* e);//确定删除的弹窗 Confirm to delete pop-ups
static void _my_Confirm_delete_pop_upsevent_cb(lv_event_t* e);//消息框弹窗回调函数
static int delete_button_callback_function(lv_event_t* e);//删除按钮回调函数
static int Please_select_pop_up_prompt(lv_event_t* e);//请选择弹窗提示
static int Please_select_del(void);//请选择弹窗提示 销毁
static void bottom_tip_label(void);//底部的提示标签
static void btn_event_callback(lv_event_t* event);//按钮事件回调函数++
static void _my_btn_click_cb(lv_event_t* evt);//按钮回调函数——键盘控制
static void _myMainWinCb(lv_event_t* evt);//键盘处理回调函数
static int _my_msgbox_cb(lv_event_t* evt);//选择删除弹窗的取消按键关联
static void _myEvent_cb(lv_event_t* evt);//处理按键事件

#define MY_TOP_WIN_H    50
#define MY_LST_ITEM_W   LV_PCT(100)   //

//--------------------------------------

static const lv_coord_t col_dsc[] = { LV_GRID_FR(1),  LV_GRID_TEMPLATE_LAST };//列1
static const lv_coord_t row_dsc[] = { MY_TOP_WIN_H,  LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST }; //第一行高60  第二行剩下全部

//我的下载删除
int my_download_edit_scr_init(void)
{
    LV_LOG_USER("已进入：我的下载 删除\n");
    ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_MYD_scrNo, zb_getOpenedFileNum());

    //从主菜单进入时
    if (enter_flag_bit_button_callback_function == 0) {
        My_download_file_directory_data_processing(SD_CARD_FILE_NAME,1);//读取SD卡中的内容
        g_curBtn = 0;//焦点
    }
    //我的下载中无内容情况
    if (Number_secondary_file_dir <= 0) {
        No_content_situation_window_layout();//无内容情况窗口布局
        No_content_reminder();//暂无内容提示
    }
    ESP_LOGI(TAG, "%d：Number_secondary_file_dir = %d", __LINE__, Number_secondary_file_dir);
    if (Number_secondary_file_dir > NUMBER_FILES_MAX) {
        ESP_LOGW(TAG, "%d：Number_secondary_file_dir = %d", __LINE__, Number_secondary_file_dir);
        Number_secondary_file_dir = NUMBER_FILES_MAX;
    }
    //我的下载中有内容情况
    if (Number_secondary_file_dir > 0) {
        //背景
        img = lv_obj_create(lv_scr_act());
        lv_img_dsc_t* beijing = zb_img_res_load_one(NULL, MY_DOWNLOAD_IMG_BEIJING, IMG_FREE_TYPE_AUTO, NULL);
        //lv_img_set_src(img, beijing);
        lv_obj_set_style_bg_img_src(img, beijing, 0);//设置图形对象背景图片
        //lv_obj_set_style_bg_img_src(img, beijing, LV_STYLE_PART_MAIN);//LV_STYLE_PART_MAIN 这是对象的主要部分，通常用于设置对象的默认样式。
        lv_obj_set_size(img, SCREEN_WIDTH, SCREEN_HEIGHT);
        lv_obj_set_grid_dsc_array(img, col_dsc, row_dsc);//将之前定义的列和行描述应用到img对象上
        lv_obj_set_style_pad_row(img, 0, 0);//移除行内边距
        lv_obj_set_style_pad_column(img, 0, 0);//移除列内边距
        lv_obj_set_style_pad_top(img, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_bottom(img, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_left(img, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_right(img, 0, LV_PART_MAIN);
        lv_obj_set_scrollbar_mode(img, LV_SCROLLBAR_MODE_OFF);//不显示滚动条
        lv_obj_set_style_border_width(img, 0, 0);//边框宽度为0

        //顶部窗口
        lv_obj_t* top_window = lv_obj_create(img);
        lv_obj_set_grid_cell(top_window, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
        lv_obj_set_style_pad_all(top_window, 0, 0);//所有方向（上、下、左、右）的内边距为0
        lv_obj_set_style_bg_opa(top_window, LV_OPA_0, LV_PART_MAIN);//按钮透明
        lv_obj_set_style_outline_width(top_window, 0, 0);//轮廓宽度为0
        lv_obj_set_style_border_width(top_window, 0, 0);//边框宽度为0
        lv_obj_set_style_pad_row(top_window, 0, 0);//移除行内边距
        lv_obj_set_style_pad_column(top_window, 0, 0);//移除列内边距
        lv_obj_set_style_pad_top(top_window, 0, LV_PART_MAIN);//指定对象的主要部分或主体(默认)  | LV_PART_ITEMS指定对象中包含的子项或条目(常用列表)
        lv_obj_set_style_pad_left(top_window, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_right(top_window, 0, LV_PART_MAIN);
        lv_obj_set_scrollbar_mode(top_window, LV_SCROLLBAR_MODE_OFF);//不显示滚动条

        //头标签
        lv_obj_t* shz = lv_label_create(top_window);
        if (NULL != shz)
        {
            lv_obj_set_size(shz, 80, 31);                 // 设置控件大小
            lv_obj_align(shz, LV_ALIGN_TOP_LEFT, 20, 20);
            lv_label_set_text(shz, "我的下载");             // 初始显示 0
            lv_obj_set_style_text_color(shz, lv_color_hex(0xffffff), 0);//白色
        }

        //主窗口mainWin
        mainWin = lv_obj_create(img);
        lv_obj_set_grid_cell(mainWin, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);//第0列第1行垂直方向拉伸填充
        lv_obj_set_size(mainWin, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_pad_row(mainWin, 10, LV_PART_MAIN);//调整按钮之间的间距
        lv_obj_set_style_pad_column(mainWin, 10, LV_PART_MAIN);
        lv_obj_set_style_pad_top(mainWin, 15, LV_PART_MAIN); //调整按钮和边界的距离
        lv_obj_set_style_pad_bottom(mainWin, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_left(mainWin, 10, LV_PART_MAIN);
        lv_obj_set_style_pad_right(mainWin, 10, LV_PART_MAIN);
        lv_obj_set_flex_flow(mainWin, LV_FLEX_FLOW_COLUMN);//列布局
        lv_obj_set_style_radius(mainWin, 19, LV_PART_MAIN | LV_STATE_DEFAULT); //设置4个角的圆弧圆角
        //mainWin的滚动条
        //lv_obj_clear_flag(mainWin, LV_OBJ_FLAG_SCROLLABLE);//禁用滚动条 禁用之后不会滑动到下面 下面内容不显示
        lv_obj_set_scrollbar_mode(mainWin, LV_SCROLLBAR_MODE_OFF);//滚动条的不显示 
        lv_obj_set_scroll_dir(mainWin, LV_DIR_VER);//设置滚动方向  LV_DIR_HOR水平滚动，LV_DIR_VER垂直滚动

        //删除按钮
        static lv_style_t style_pr;//收藏样式
        lv_style_init(&style_pr);//初始化style_pr样式
        lv_style_set_img_recolor(&style_pr, lv_color_make(0xFF, 0x00, 0x00));//设置按下样式中图像的重新着色颜色为
        lv_obj_t* xuanze_btnimg = lv_imgbtn_create(top_window);//创建图片按钮
        lv_obj_remove_style_all(xuanze_btnimg);//从xuanze_btnimg对象上移除所有已应用的样式
        //lv_img_dsc_t* xuanze_tu = zb_img_res_load_one(NULL, MY_DOWNLOAD_IMG_SHANCHU, IMG_FREE_TYPE_AUTO, NULL);
        lv_img_dsc_t* xuanze_tu = zb_img_res_load_one(NULL, MY_DOWNLOAD_IMG_ABSHANCHU, IMG_FREE_TYPE_AUTO, NULL);
        lv_imgbtn_set_src(xuanze_btnimg, LV_IMGBTN_STATE_RELEASED, xuanze_tu, NULL, NULL);//设置图片源
        //lv_obj_align_to(xuanze_btnimg, top_window, LV_ALIGN_TOP_RIGHT, 50, 7);
        lv_obj_align_to(xuanze_btnimg, top_window, LV_ALIGN_TOP_RIGHT, 50, 13);
        lv_obj_add_style(xuanze_btnimg, &style_pr, LV_STATE_PRESSED);//为图像按钮在按下状态下添加样式
        lv_obj_add_event_cb(xuanze_btnimg, delete_button_callback_function, LV_EVENT_SHORT_CLICKED, NULL);//按键回调函数

        label_tmp = (lv_obj_t**)heap_caps_malloc_dflt(sizeof(lv_obj_t*) * NUMBER_FILES_MAX);
        memset(label_tmp, 0, sizeof(lv_obj_t*) * NUMBER_FILES_MAX);//避免野指针

        g_btns = (lv_obj_t**)heap_caps_malloc_dflt(sizeof(lv_obj_t*) * NUMBER_FILES_MAX);
        memset(g_btns, 0, NUMBER_FILES_MAX * sizeof(lv_obj_t*));

        checkbox_albums = (lv_obj_t**)heap_caps_malloc_dflt(sizeof(lv_obj_t*) * NUMBER_FILES_MAX);
        memset(checkbox_albums, 0, sizeof(lv_obj_t*) * NUMBER_FILES_MAX);

        checkbox_status_albums = (bool *)heap_caps_malloc_dflt(sizeof(bool) * NUMBER_FILES_MAX);
        memset(checkbox_status_albums, 0, sizeof(bool) * NUMBER_FILES_MAX);

        ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_MYD_scrNo, zb_getOpenedFileNum());
        //循环创建
        for (int i = 0; i < Number_secondary_file_dir; i++)
        {
            lv_obj_t * btnOne = lv_btn_create(mainWin);//创建按钮 
            g_btns[i] = btnOne;

            //把 btn 组件设置为 grid 容器, 达到让各项右边的复选框对齐的效果（btn 本身是容器类型，所以可以这样改, 而其他组件，如 img 不同）
			static const lv_coord_t col_dsc[] = { LV_GRID_FR(1), 42, LV_GRID_TEMPLATE_LAST };//两列
            static const lv_coord_t row_dsc[] = { LV_GRID_FR(1),  LV_GRID_TEMPLATE_LAST };//
            lv_obj_set_grid_dsc_array(btnOne, col_dsc, row_dsc);//

            label_tmp[i] = lv_label_create(btnOne);//创建标签
            lv_obj_set_grid_cell(label_tmp[i], LV_GRID_ALIGN_STRETCH, 0, 1,  LV_GRID_ALIGN_CENTER, 0, 1);//LV_GRID_ALIGN_CENTER  LV_GRID_ALIGN_STRETCH

            lv_label_set_text(label_tmp[i], second_level_directory[i]);
            lv_label_set_long_mode(label_tmp[i], LV_LABEL_LONG_DOT); // LV_LABEL_LONG_SCROLL_CIRCULAR);//滚动显示  ——》无效
            lv_obj_set_style_anim_speed(label_tmp[i], 50, 0);//滚动速度
            lv_obj_set_size(label_tmp[i], 170, 20);//必须设置大小滚动显示才有效
            lv_obj_add_event_cb(btnOne, _my_btn_checkbox_status_cb, LV_EVENT_SHORT_CLICKED, NULL);//按键控制，按钮被短按时被调用 LV_EVENT_SHORT_CLICKED
            lv_obj_set_size(btnOne, MY_LST_ITEM_W, 50);       // 设置控件大小
            lv_obj_align_to(label_tmp[i], btnOne, LV_ALIGN_LEFT_MID, 23, 0);//标签位置
            //lv_obj_center(label_tmp[i]);//居中
            //lv_obj_align(label_tmp[i], LV_ALIGN_LEFT_MID, 20, 0);
            lv_obj_set_style_radius(btnOne, 15, 15);//设置btnA按钮的样式圆角半径为0，即按钮没有圆角。
            lv_obj_set_style_bg_color(btnOne, lv_color_hex(0xf6f6f6), LV_STATE_DEFAULT);//默认状态 灰色
            lv_obj_set_style_bg_color(btnOne, lv_color_hex(0x79B9FF), LV_STATE_FOCUS_KEY);//键盘聚焦状态  蓝色
            lv_obj_set_style_bg_color(btnOne, lv_color_hex(0x79B9FF), LV_STATE_PRESSED);//已按下  
            lv_obj_set_style_text_color(btnOne, lv_color_hex(0x000000), 0);//默认状态 标签颜色为黑色
            lv_obj_set_style_text_color(btnOne, lv_color_hex(0xffffff), LV_STATE_FOCUS_KEY);//键盘聚焦状态下改变按钮里字体颜色 白色   继承
            //lv_obj_center(btnOne);//居中
            lv_obj_set_flex_align(btnOne, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);//在主轴、交叉轴和打包方向上，子对象都将被居中对齐

            //设置复选框样式
            static lv_style_t style_checkbox_ago;//点击前
            lv_style_init(&style_checkbox_ago);                       //初始化样式
            lv_style_set_radius(&style_checkbox_ago, LV_RADIUS_CIRCLE);//设置样式圆角
            lv_style_set_bg_color(&style_checkbox_ago, lv_color_hex(0xffffff));//背景颜色 白  绿 #74D355
            lv_style_set_border_color(&style_checkbox_ago, lv_color_hex(0x808080));//边框的颜色  灰  lv_color_make(0x00, 0x0, 0x00);
            static lv_style_t style_checkbox_back;//点击后
            lv_style_init(&style_checkbox_back);                       //初始化样式  
            lv_style_set_radius(&style_checkbox_back, LV_RADIUS_CIRCLE);//设置样式圆角
            lv_style_set_bg_color(&style_checkbox_back, lv_color_hex(0x74d355));//背景颜色 白  绿 #74D355
            lv_style_set_border_color(&style_checkbox_back, lv_color_hex(0x808080));//边框的颜色  灰  lv_color_make(0x00, 0x0, 0x00);
            //创建复选框部件和添加样式
            checkbox_albums[i] = lv_checkbox_create(btnOne);//创建复选框
            lv_obj_set_grid_cell(checkbox_albums[i], LV_GRID_ALIGN_CENTER, 0, 1,  LV_GRID_ALIGN_CENTER, 0, 1);//
            lv_checkbox_set_text(checkbox_albums[i], "");//设置文本内容
            //lv_obj_set_pos(checkbox_albums[i], 50, 5);
            //lv_obj_align_to(checkbox_albums[i], btnOne, LV_ALIGN_RIGHT_MID, -5, 0);//标签位置
            //lv_obj_set_style_pad_column(checkbox_albums[i], 0, LV_STATE_DEFAULT);//设置文本和勾选框的间距
            lv_obj_add_style(checkbox_albums[i], &style_checkbox_ago, LV_PART_INDICATOR | LV_STATE_DEFAULT); //添加样式  默认
            lv_obj_add_style(checkbox_albums[i], &style_checkbox_back, LV_PART_INDICATOR | LV_STATE_CHECKED); //添加样式  选中
            lv_obj_add_style(checkbox_albums[i], &style_checkbox_back, LV_PART_INDICATOR | LV_STATE_PRESSED); //添加样式  按下

        }

    }
    ESP_LOGI(TAG, "g_curBtn = %d,", g_curBtn);
    ESP_LOGI(TAG, "Number_secondary_file_dir = %d", Number_secondary_file_dir);
    if (g_curBtn >= 0) {
        ESP_LOGI(TAG, "g_btns = %p,", g_btns);
        if (g_curBtn >= Number_secondary_file_dir) {
            ESP_LOGI(TAG, "同一界面不同版面下，上一个版面最后的焦点g_curBtn值，大于当前版面的按钮数");
            g_curBtn = Number_secondary_file_dir - 1;
        }
        if (Number_secondary_file_dir != 0) { //防止在全部删除后，再聚焦会报错
            if (g_curBtn >= 1){ //不要让它滚动到第一个选择删除按钮
                lv_obj_scroll_to_view(g_btns[g_curBtn], LV_ANIM_OFF);//滚动到一个对象，直到它在它的父对象上可见
            }
            lv_group_focus_obj(g_btns[g_curBtn]); //将焦点设置到按钮对象上
            ESP_LOGI(TAG, "焦点已设置到按钮上");
            //如果是文件按钮
            if (g_curBtn >= 0) {
                ESP_LOGI(TAG, "lv_obj_has_state：%d", lv_obj_has_state(g_btns[g_curBtn], LV_STATE_FOCUS_KEY));
                if (lv_obj_has_state(g_btns[g_curBtn], LV_STATE_FOCUS_KEY) == 1) {
                    lv_label_set_long_mode(label_tmp[g_curBtn], LV_LABEL_LONG_SCROLL_CIRCULAR);
                }
                else {
                    lv_label_set_long_mode(label_tmp[g_curBtn], LV_LABEL_LONG_DOT);
                }
            }
        }
    }
    //g_curBtn = 0;
    lv_group_add_obj(lv_group_get_default(), img);//这行代码将winA对象添加到默认的对象组中。在LVGL中，对象组用于处理键盘输入和导航。
    lv_obj_add_event_cb(img, _myMainWinCb, LV_EVENT_ALL, NULL);//为mainWin窗口添加一个事件回调函数_myMainWinCb，该回调函数将处理所有类型的事件
    sys_setCurMainWinAndCb(img, _myMainWinCb);//自定义函数将mainWin作为当前主窗口。

    g_mainWin = img;

    return 0;
}

//删除界面 我的下载  删除界面
int my_download_edit_scr_free(void) {
    int openedN = zb_getOpenedFileNum();
    //ESP_LOGI(TAG, "%d exitAp, openedN %d", g_UCC_scrNo, openedN);
    if (g_mainWin != NULL) {
        lv_obj_del(g_mainWin);  g_mainWin = NULL;
    }
    return 0;
}

//无内容情况窗口布局 No content situation window layout
static void No_content_situation_window_layout(void) {
    LV_LOG_USER("已进入：无内容情况窗口布局\n");
    //背景
    img = lv_img_create(lv_scr_act());
    lv_img_dsc_t* beijing = zb_img_res_load_one(NULL, MY_DOWNLOAD_IMG_BEIJING, IMG_FREE_TYPE_AUTO, NULL);
    lv_img_set_src(img, beijing);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(img, SCREEN_WIDTH, SCREEN_HEIGHT);

    //内置界面
    mainWin = lv_obj_create(img);
    lv_obj_set_style_border_width(mainWin, 0, 0);//设置mainWin的边框宽度为0。
    lv_obj_set_size(mainWin, SCREEN_WIDTH, SCREEN_HEIGHT - 42);
    lv_obj_set_pos(mainWin, 0, 42);
    lv_obj_set_style_radius(mainWin, 19, LV_PART_MAIN | LV_STATE_DEFAULT); //设置4个角的圆弧圆角
    lv_obj_clear_flag(mainWin, LV_OBJ_FLAG_SCROLLABLE);//禁用滚动条 禁用之后不会滑动到下面 下面内容不显示
    lv_obj_set_scrollbar_mode(mainWin, LV_SCROLLBAR_MODE_OFF);//关闭滚动条的显示
    //lv_obj_set_flex_align(g_flexWinA, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    //头标签
    lv_obj_t* shz = lv_label_create(img);
    if (NULL != shz)
    {
        lv_obj_set_size(shz, 80, 31);                 // 设置控件大小
        lv_obj_align(shz, LV_ALIGN_TOP_LEFT, 15, 15);
        lv_label_set_text(shz, "我的下载");             // 初始显示 0
        lv_obj_set_style_text_color(shz, lv_color_hex(0xffffff), 0);//白色
    }
}

//暂无内容提示
static void No_content_reminder(void) {

    ESP_LOGI(TAG, "已进入：暂无内容No_content");

    //SD卡图标
    lv_obj_t* sdka = lv_img_create(mainWin);
    lv_img_dsc_t* sdka_tu = zb_img_res_load_one(NULL, MY_DOWNLOAD_IMG_CHUXUKATUBIAO, IMG_FREE_TYPE_AUTO, NULL);
    lv_img_set_src(sdka, sdka_tu);//设置图片源
    lv_obj_align_to(sdka, mainWin, LV_ALIGN_TOP_MID, 0, 37);
    //lv_obj_align(sdka, LV_ALIGN_CENTER, 0, 0);

    //SD卡图标下面的提示标签
    lv_obj_t* label_sdtishi = lv_label_create(mainWin);
    lv_obj_align_to(label_sdtishi, mainWin, LV_ALIGN_TOP_MID, -20, 138);
    lv_obj_set_size(label_sdtishi, SCREEN_WIDTH, 32);
    lv_label_set_recolor(label_sdtishi, true);  //使能颜色修改              
    lv_label_set_text(label_sdtishi, "#808080 暂无内容#");

    //提示标签
    lv_obj_t* label_tishi = lv_label_create(mainWin);
    lv_obj_align_to(label_tishi, mainWin, LV_ALIGN_TOP_MID, -85, 185);
    lv_obj_set_size(label_tishi, SCREEN_WIDTH, 50);
    lv_label_set_recolor(label_tishi, true);  //使能颜色修改              
    lv_label_set_text(label_tishi, "#000000 提示：\n展示内容为网盘下载内容以\n及拷贝至储存卡的内容#");
}

//删除按钮回调函数
static int delete_button_callback_function(lv_event_t* e) {
    ESP_LOGI(TAG, "已进入：删除按钮回调函数");
    //如果没有选择任何复选框 则提示
    //如果有选择复选框 确定删除的消息框弹窗
    int ret = -1;
    for (int i = 0; i < Number_secondary_file_dir; i++) {
        //if (lv_obj_has_state(checkbox_albums[i], LV_STATE_CHECKED) == 1) {
        if (checkbox_status_albums[i] == 1) {
            //如果有选择复选框 确定删除的消息框弹窗
            _my_Confirm_delete_pop_ups(e);
            ret = 1;
            break;
        }
    }
    if (ret != 1) {
        //请选择弹窗存在标记
        // 如果没有选择任何复选框 则提示
        if (!please_select_pop_up_presence_flag) {
            ESP_LOGI(TAG, "Number_secondary_file_dir = %d", Number_secondary_file_dir);
            if (Number_secondary_file_dir > 0) {
                ESP_LOGI(TAG, "%d：Number_secondary_file_dir > 0", __LINE__);
                Please_select_pop_up_prompt(e);
                ret = 0;
            }
            else{
                ESP_LOGI(TAG, "%d：Number_secondary_file_dir <= 0 不要出现弹窗",__LINE__);
            }
        }
    }
    return ret;
}

static lv_timer_t* Please_select_prompt_timer;//请选择定时器
static lv_obj_t* Please_select_prompt;//请选择提示
//请选择弹窗提示
static int Please_select_pop_up_prompt(lv_event_t* e) {
    LV_LOG_USER("已进入：请选择弹窗提示\n");

    //样式
    static lv_style_t style_load;
    lv_style_init(&style_load);
    lv_style_set_bg_color(&style_load, lv_color_hex(0x000000));
    lv_style_set_bg_opa(&style_load, LV_OPA_80);
    lv_style_set_border_width(&style_load, 0);
    lv_style_set_clip_corner(&style_load, true);
    lv_style_set_radius(&style_load, 20);

    // 创建对话框
    Please_select_prompt = lv_obj_create(lv_scr_act());
    lv_obj_add_style(Please_select_prompt, &style_load, 0);
    lv_obj_set_size(Please_select_prompt, 164, 66);
    lv_obj_set_pos(Please_select_prompt, 38, 127);

    please_select_pop_up_presence_flag = 1;//请选择弹窗存在标记

    lv_obj_t* label = lv_label_create(Please_select_prompt);
    lv_label_set_text(label, "请选择文件");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label);
    Please_select_prompt_timer = lv_timer_create(Please_select_del, 2000, NULL);

    return 0;
}
//请选择弹窗提示 销毁
static int Please_select_del(void) {
    LV_LOG_USER("已进入：请选择弹窗提示 销毁\n");
    // 关闭弹出窗口
    if (Please_select_prompt != NULL) {
        lv_msgbox_close_async(Please_select_prompt);
    }
    please_select_pop_up_presence_flag = 0;//请选择弹窗存在标记
    lv_timer_del(Please_select_prompt_timer); // 删除定时器

    return 0;
}

static lv_obj_t* msgbox;//消息框
//确定删除的消息框弹窗 Confirm to delete pop-ups
static void _my_Confirm_delete_pop_ups(lv_event_t* e) {

    ESP_LOGI(TAG, "已进入：确定删除弹窗_my_Confirm_delete_pop_ups");
#if 1    
    static lv_obj_t* slider_label;
    static const char* btns[] = { "  取消  ", "  确定  ", "" };

    //消息框整体
    //msgbox = lv_msgbox_create(lv_scr_act(), "     确认删除", "删除后可以通过下载\n重新获取资源", btns, false);
	msgbox = lv_msgbox_create(NULL, "     确认删除", "  删除后可以通过下载\n重新获取资源\n", btns, false);
    lv_obj_set_size(msgbox, 208, 187);
    lv_obj_center(msgbox);//位置                                                          
    lv_obj_set_style_border_width(msgbox, 0, LV_STATE_DEFAULT);//去除边框                         
    lv_obj_set_style_shadow_width(msgbox, 20, LV_STATE_DEFAULT);//设置阴影宽度                    
    lv_obj_set_style_shadow_color(msgbox, lv_color_hex(0xa9a9a9), LV_STATE_DEFAULT);//设置阴影颜色
    lv_obj_set_style_pad_top(msgbox, 18, LV_STATE_DEFAULT); //设置顶部填充                        
    lv_obj_set_style_pad_left(msgbox, 20, LV_STATE_DEFAULT); //设置左侧填充
    lv_obj_set_style_radius(msgbox, 19, LV_PART_MAIN | LV_STATE_DEFAULT); //设置4个角的圆弧圆角
    lv_obj_add_event_cb(msgbox, _my_Confirm_delete_pop_upsevent_cb, LV_EVENT_VALUE_CHANGED, NULL);//添加事件

    //消息框标题
    lv_obj_t* title = lv_msgbox_get_title(msgbox);//获取标题部分
    //lv_obj_center(title);
    lv_obj_align_to(title, msgbox, LV_ALIGN_TOP_MID, 0, 0);//按钮位置
    //lv_obj_set_style_text_font(title, &lv_font_montserrat_20, LV_STATE_DEFAULT);//设置字体             
    lv_obj_set_style_text_color(title, lv_color_hex(0x999999), LV_STATE_DEFAULT);//设置文本颜色：灰色

    //消息框主体
    lv_obj_t* content = lv_msgbox_get_content(msgbox);//获取主体部分
    //lv_obj_set_style_text_font(content, &lv_font_montserrat_20, LV_STATE_DEFAULT);//设置字体
    lv_obj_set_style_text_color(content, lv_color_hex(0x000000), LV_STATE_DEFAULT);//设置文本颜色：白色
    lv_obj_set_style_pad_top(content, 15, LV_STATE_DEFAULT);//设置顶部填充

    //消息框按钮
    lv_obj_t* btn = lv_msgbox_get_btns(msgbox);//获取按钮矩阵部分
    //lv_obj_set_size(btn, 200, 30);// 设置控件大小
    lv_obj_set_size(btn, 200, 35);// 设置控件大小
    lv_obj_set_style_radius(btn, 20, LV_PART_MAIN | LV_STATE_DEFAULT | LV_PART_ITEMS);//圆角
    //lv_obj_set_style_bg_color(btn, lv_color_hex(0x79B9FF), LV_STATE_DEFAULT | LV_PART_ITEMS);//默认状态  蓝色 0x79B9FF
    ////lv_obj_set_style_bg_color(btn, lv_color_hex(0x74D355), LV_PART_ITEMS);//默认状态 蓝色 #74d355
    //lv_obj_set_style_bg_color(btn, lv_color_hex(0xFF5858), LV_STATE_CHECKED | LV_PART_ITEMS);//选中  红色 0xFF5858 LV_STATE_FOCUS_KEY  ——》无效
    //lv_obj_set_style_bg_color(btn, lv_color_hex(0xFF5858), LV_STATE_PRESSED | LV_PART_ITEMS);//已按下 

    lv_obj_set_style_bg_color(btn, lv_color_hex(0x6058e0), LV_STATE_DEFAULT);//#6058e0  0xFF5858
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x6058e0), LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x6058e0), LV_STATE_FOCUSED);

    //lv_obj_set_style_bg_opa(btn, 0, LV_PART_ITEMS);//设置按钮背景透明度
    //lv_obj_set_style_shadow_width(btn, 0, LV_PART_ITEMS);//去除按钮阴影
    lv_obj_set_style_text_color(btn, lv_color_hex(0xffffff), LV_PART_ITEMS);//文本颜色 | LV_STATE_PRESSED
    lv_obj_set_style_pad_column(btn, 80, LV_PART_MAIN);//列间距
    
    //lv_obj_add_flag(msgbox, LV_OBJ_FLAG_HIDDEN);//隐藏对象
    lv_obj_add_event_cb(msgbox, _my_msgbox_cb, LV_EVENT_KEY, NULL);
#endif
}

#define MY_DLG_SEL_ID_CANCEL 0  //消息框中的取消
#define MY_DLG_SEL_ID_OK     1  //消息框中的确定
//消息框回调函数中的 定时器回调函数：
//消息框的取消与确定按钮处理事件
static void delDlgSelectProc(void) {
    ESP_LOGI(TAG, "delDlgSel proc");
    ESP_LOGI(TAG, "当前目录的数量Number_secondary_file_dir = %d", Number_secondary_file_dir);
    //打印一下该目录下所有的复选框状态
    for (int i = 0; i < Number_secondary_file_dir; i++) {
        ESP_LOGI(TAG, "文件%s 的复选框状态为： %d", second_level_directory[i], checkbox_status_albums[i]);
    }
    int8_t selId = zb_msgbox_sel_no_get();// 读取(之前记下的)消息框选项号
    //消息框中的取消处理：清除所有复选框状态
    if (MY_DLG_SEL_ID_CANCEL == selId) {

        //点击消息框中的取消 不要清除复选框状态
        //切换回焦点
        lv_group_focus_obj(g_mainWin);
        sys_setCurMainWinObj(g_mainWin);
        ESP_LOGI(TAG, "g_btns=%p", g_btns);
        ESP_LOGI(TAG, "g_curBtn=%d", g_curBtn);
        lv_group_focus_obj(g_btns[g_curBtn]);

        //for (int i = 0; i < Number_secondary_file_dir; i++) {
        //    checkbox_status_albums[i] = 0;
        //    lv_obj_clear_state(checkbox_albums[i], LV_STATE_CHECKED | LV_STATE_DISABLED);//清除复选框状态
        //}
    }
    //消息框中的确定处理：删除复选框选中的，销毁当前版面，重新加载
	else if (MY_DLG_SEL_ID_OK == selId) {
        for (int i = 0; i < Number_secondary_file_dir; i++) {
            //if (lv_obj_has_state(checkbox_albums[i], LV_STATE_CHECKED) == 1) {
            if (checkbox_status_albums[i] == 1) {
                //lv_obj_clear_state(checkbox_albums[i], LV_STATE_CHECKED | LV_STATE_DISABLED);//清除复选框状态
                ESP_LOGI(TAG, "当前删除的文件或文件夹是：%s", second_level_directory[i]);
                ESP_LOGI(TAG, "删除的绝对路径是：%s", second_son_absolute_path[i]);
                ESP_LOGI(TAG, "检索一下删除的这个文件或文件夹的子目录：");
                //Print_file_directory(second_son_absolute_path[i]);//遍历打印某一文件夹下的子文件目录
                ESP_LOGI(TAG, "remove____i = %d", i);

                int stMode = 0;
                zb_stat__st_mode(second_son_absolute_path[i], &stMode);
                if (S_IFDIR & stMode) {
                    int shanchudir = zb_del_dir(second_son_absolute_path[i]);
                    ESP_LOGI(TAG, "删除文件夹zb_del_dir的返回值为：%d", shanchudir);
                }
                else {
                    int shanchu = zb_remove(second_son_absolute_path[i]);//删除文件或者文件夹   只能删除空目录
                    ESP_LOGI(TAG, "删除文件或者文件夹zb_remove的返回值为：%d", shanchu);
                }

                ESP_LOGI(TAG, "remove____i = %d", i);
            }
            ESP_LOGI(TAG, "i = %d",i);
        }
        ESP_LOGI(TAG, "当前目录的数量Number_secondary_file_dir = %d", Number_secondary_file_dir);
		
        My_download_file_directory_data_processing(previous_level_directory, Current_SD_card_directory_depth);//读取SD卡中的内容
        
        ESP_LOGI(TAG, "读取SD卡中的内容OK ");
        lv_obj_del(g_mainWin);  g_mainWin = NULL;  // 销毁当前版面 
        ESP_LOGI(TAG, "销毁当前版面OK ");
        //重新加载的时候之前删除的
        my_download_edit_scr_init();
    }

}

//消息框弹窗回调函数
//重新加载的时候之前删除的  不让它显示   ——》在上数据的时候加
//键盘左右按钮和弹窗的取消和确定两个按钮关联
static void _my_Confirm_delete_pop_upsevent_cb(lv_event_t* e)
{
    ESP_LOGI(TAG, "已进入：弹窗回调函数_my_Confirm_delete_pop_upsevent_cb");
    lv_obj_t* obj = lv_event_get_current_target(e);//获取当前的目标对象（即消息框对象）
    uint16_t btnNo = lv_msgbox_get_active_btn(obj);// 获取当前消息框中被激活（被点击）的按钮的编号
    LV_LOG_USER("Button (No%d) %s clicked", (int)btnNo, lv_msgbox_get_active_btn_text(obj));//编号 文本

    if (btnNo == 0)//获取按钮索引
    {
        ESP_LOGI(TAG, "取消"); //清除复选框状态
    }
    else if (btnNo == 1) {
        ESP_LOGI(TAG, "确定"); //删除选中的复选框按钮
    }
    zb_msgbox_sel_no_save(btnNo);//删除按钮编号
    //定时器：
    //delDlgSelectProc:一个可以传递给回调函数的指针。这允许你在创建定时器时附加任何类型的数据，并在回调函数中访问它。
    lv_timer_create(zb_ex_cmd_timer_cb, 0, delDlgSelectProc);//定时器：发送数据、
    lv_msgbox_close(obj);//关闭消息框
}


//选择删除弹窗的取消按键关联
static int _my_msgbox_cb(lv_event_t* evt) {
    LV_LOG_USER("已进入：选择删除弹窗的取消按键关联\n");
    //选择删除后的弹窗中的 取消按钮要与ESC返回键关联
    lv_event_code_t evtCode = evt->code; //从evt中获取事件代码，并赋值
    if (LV_EVENT_KEY == evt->code) {  //检查事件代码
        int keyVal = lv_event_get_key(evt);//获取按键值
        if (keyVal == LV_KEY_ESC) {
            ESP_LOGI(TAG, "LV_KEY_ESC取消");

            //关闭消息框
            if (msgbox != NULL) {
                lv_msgbox_close(msgbox); msgbox = NULL;
            }
            //切换回焦点
            lv_group_focus_obj(g_mainWin);
            sys_setCurMainWinObj(g_mainWin);
            ESP_LOGI(TAG, "g_btns=%p", g_btns);
            ESP_LOGI(TAG, "g_curBtn=%d", g_curBtn);
            lv_group_focus_obj(g_btns[g_curBtn]);
        }

    }
    return 0;
}


//底部的提示标签
void bottom_tip_label(void) {

    lv_obj_t* label_bottm = lv_label_create(mainWin);
    //lv_obj_set_x(label_bottm, 0);
    //lv_obj_set_y(label_bottm, 285);
    lv_obj_align_to(label_bottm, mainWin, LV_ALIGN_TOP_MID, -65, SCREEN_HEIGHT-32-49);
    lv_obj_set_size(label_bottm, SCREEN_WIDTH, 32);
    lv_label_set_recolor(label_bottm, true);  //使能颜色修改              
    lv_label_set_text(label_bottm, "#666666 选中专辑后，按AB键统一删除#");

}

//按钮回调函数 控制复选框状态
static void _my_btn_checkbox_status_cb(lv_event_t* e) {
    ESP_LOGI(TAG, "已进入：按钮回调函数_my_btn_checkbox_status_cb");
    for (int i = 0; i < Number_secondary_file_dir; i++) {
        lv_obj_t* target = lv_event_get_target(e); //获取触发源 
        if (target == g_btns[i])//触发那个按钮，那个按钮就改变状态，其余的按钮事件状态不要变
        {
            if (lv_obj_has_state(checkbox_albums[i], LV_STATE_CHECKED) == 0) {

                lv_obj_add_state(checkbox_albums[i], LV_STATE_CHECKED | LV_STATE_DISABLED);//添加状态：默认选中且不可修改     //添加、清除复选框状态 
            }
            else if (lv_obj_has_state(checkbox_albums[i], LV_STATE_CHECKED) == 1) {

                lv_obj_clear_state(checkbox_albums[i], LV_STATE_CHECKED | LV_STATE_DISABLED);//清除复选框状态
            }
            checkbox_status_albums[i] = lv_obj_has_state(checkbox_albums[i], LV_STATE_CHECKED);//获取（判断）复选框状态 返回值：1选中 0非选中
            LV_LOG_ERROR("第%d个专辑复选框的状态为：%d", i, checkbox_status_albums[i]);
            break;
        }
    }

}


#define C_BTN_N_IN_LINE  3
#define C_SCROLL_STEP 12
static short g_isPause = 0;
static lv_obj_t* g_mainContentObj = 0;//主内容对象 g_mainContentObj 的高度
//键盘处理回调函数
static void _myMainWinCb(lv_event_t* evt) {
    int keyVal = 0;
    lv_event_code_t evtCode = evt->code; //从evt中获取事件代码，并赋值
    if (LV_EVENT_KEY_STAGE == evt->code) {  //检查事件代码
        keyVal = lv_event_get_key(evt);//获取按键值
        //如果按键值包含了重复按键的标记，则移除这个标记，并将事件代码更改为 LV_EVENT_KEY
        if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_REPEAT) {
            keyVal &= ~LV_EX_FLAG_KEY_MASK;
            evtCode = LV_EVENT_KEY;
        }
    }
    if (LV_EVENT_KEY == evtCode) { //判断是否是一个按键事件
        if (!keyVal) {
            keyVal = lv_event_get_key(evt); //重新从事件对象中获取按键值
        }
#if 0
        //检查是否是左右箭头键
        if (LV_KEY_LEFT == keyVal || LV_KEY_RIGHT == keyVal) {
            LV_LOG_USER("左右箭头键\n");
            short btnNoPrev = g_curBtn; //将当前选中的按钮索引赋值给 btnNoPrev
            //如果 btnNoPrev 小于0且按键是左箭头，则将 btnNoPrev 设置为 C_BTN_N_IN_LINE。
            if (btnNoPrev < 0 && LV_KEY_LEFT == keyVal) {
                btnNoPrev = C_BTN_N_IN_LINE;
            }
            //根据按键是左箭头还是右箭头，设置 diff 为 - 1或1。
            short diff = (LV_KEY_LEFT == keyVal ? -1 : 1);
            short btnNo = btnNoPrev + diff;//计算新的按钮索引 btnNo。
            bool bNeedUnfocus = false; //标记是否需要取消焦点。
            if (btnNo < 0 || btnNo >= C_BTN_N_IN_LINE) {//查新的按钮索引 btnNo 是否在有效范围内
                g_curBtn = -1;  //不在有效值范围内
                bNeedUnfocus = true;
            }
            else { //如果在有效范围内，更新 g_curBtn 并将焦点设置到新的按钮对象上
                g_curBtn = btnNo;
                lv_group_focus_obj(g_btns[btnNo]);
            }
            if (bNeedUnfocus) {//如果 bNeedUnfocus 为 true，则清除前一个按钮的焦点状态。
                lv_obj_clear_state(g_btns[btnNoPrev], LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);
            }
            lv_event_stop_processing(evt);//停止事件的进一步处理。
        }
        else
#endif
        //检查是否是上下箭头键
        if (LV_KEY_UP == keyVal || LV_KEY_DOWN == keyVal) {
            LV_LOG_USER("上下箭头键\n");
            ESP_LOGI(TAG, "g_curBtn = %d\n", g_curBtn);
            ESP_LOGI(TAG, "g_btns = %p\n", g_btns);
            ESP_LOGI(TAG, "二级目录文件的数量：%d\n", Number_secondary_file_dir);
            //我的下载中无内容情况
            if (Number_secondary_file_dir <= 0) {
                ESP_LOGI(TAG, "我的下载中无内容情况不要进入上下键");
            }
            else{
            if (g_curBtn >= 0) {
                if (g_btns[g_curBtn]) {
                    if (lv_obj_has_state(g_btns[g_curBtn], LV_STATE_FOCUS_KEY) == 0) {
                        lv_label_set_long_mode(label_tmp[g_curBtn], LV_LABEL_LONG_SCROLL_CIRCULAR);
                    }
                    else {
                        lv_label_set_long_mode(label_tmp[g_curBtn], LV_LABEL_LONG_DOT);
                    }
                }
            }
            if (LV_KEY_DOWN == keyVal) {   //判断向下的按键是否按下
                g_curBtn = (g_curBtn + 1) % Number_secondary_file_dir;//NUM_CHINESE_FILES;  // debug
                ESP_LOGI(TAG, "向下g_curBtn = %d\n", g_curBtn);
            }
            else if (LV_KEY_UP == keyVal) {  //上
                g_curBtn = (g_curBtn + Number_secondary_file_dir - 1) % Number_secondary_file_dir;
                ESP_LOGI(TAG, "向上g_curBtn = %d\n", g_curBtn);
            }
            if (g_curBtn >= 0) {
                if (g_btns[g_curBtn]) {
                    lv_group_focus_obj(g_btns[g_curBtn]); //将焦点设置到按钮对象上
                    ESP_LOGI(TAG, "lv_obj_has_state：%d", lv_obj_has_state(g_btns[g_curBtn], LV_STATE_FOCUS_KEY));
                    if (lv_obj_has_state(g_btns[g_curBtn], LV_STATE_FOCUS_KEY) == 1) {
                        lv_label_set_long_mode(label_tmp[g_curBtn], LV_LABEL_LONG_SCROLL_CIRCULAR);
                    }
                    else {
                        lv_label_set_long_mode(label_tmp[g_curBtn], LV_LABEL_LONG_DOT);
                    }
                }
            }
            //lv_group_focus_obj(g_btns[g_curBtn]); //将焦点设置到按钮对象上
#if 0
            lv_coord_t contentH = 800; // non-std, lv_obj_get_height(objSpans);
            //计算可滚动的最大距离 maxScroll，即内容高度减去主内容对象 g_mainContentObj 的高度。
            lv_coord_t maxScroll = contentH - lv_obj_get_height(g_mainContentObj);
            if (maxScroll < 0) {
                maxScroll = 0;//如果 maxScroll 小于0，则将其设置为0，表示没有可滚动的空间
            }
            //获取主内容对象 g_mainContentObj 当前的滚动位置 posY
            //初始化新的滚动位置 scrollYNew 为-1，并设置布尔变量 bChg 为 false，用于标记是否更改了滚动位置
            lv_coord_t posY = lv_obj_get_scroll_y(g_mainContentObj);
            lv_coord_t scrollYNew = -1;
            bool bChg = false;

            LV_LOG_USER("key prs in exampleApA");
            //如果按下的是下箭头键，并且当前滚动位置小于最大滚动距离，则向下滚动 C_SCROLL_STEP 的距离。
            if (LV_KEY_DOWN == keyVal) {
                if (posY < maxScroll) {
                    scrollYNew = posY + C_SCROLL_STEP;
                    bChg = true;
                }
            }
            //如果按下的是上箭头键，并且当前滚动位置大于0，则向上滚动 C_SCROLL_STEP 的距离，并确保新的滚动位置不小于0。
            else if (LV_KEY_UP == keyVal) {
                if (posY > 0) {
                    scrollYNew = posY - C_SCROLL_STEP;
                    if (scrollYNew < 0) {
                        scrollYNew = 0;
                    }
                    bChg = true;
                    }
                }
            //如果 bChg 为 true，即滚动位置发生了改变，则滚动主内容对象 g_mainContentObj 到新的位置 scrollYNew，并关闭动画。
            if (bChg) {
                lv_obj_scroll_to_y(g_mainContentObj, scrollYNew, LV_ANIM_OFF);
            }
#endif
            lv_event_stop_processing(evt); //停止事件的进一步处理
            }

        }
        //如果按下的是回车键，并且当前没有选中的按钮（g_curBtn < 0），则切换 g_isPause 的状态，
        else if (LV_KEY_ENTER == keyVal) {
            LV_LOG_USER("回车键\n");
            if (g_curBtn < 0) {
                g_isPause = !g_isPause;
                //updateStateView();//更新状态视图
                lv_event_stop_processing(evt);//停止当前事件的处理，不再执行后续的事件回调函数。
                lv_indev_keypad_wait_release();//等待键盘输入设备的按键释放
            }
        }
        else if (LV_KEY_ESC == keyVal) {
            LV_LOG_ERROR("Esc键，keyVal = %d\n", keyVal);
            //退出时关闭相应文件

            ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_MYD_scrNo, zb_getOpenedFileNum());
            if (g_curBtn < 0) {
                g_isPause = !g_isPause;
                //updateStateView();//更新状态视图
                lv_event_stop_processing(evt);//停止当前事件的处理，不再执行后续的事件回调函数
                lv_indev_keypad_wait_release();//等待键盘输入设备的按键释放
            }
            lv_event_stop_processing(evt);
            //lv_obj_del(g_mainWin);  g_mainWin = NULL;
            MYD_switchScr(C_ZB_AP_MYD_SON_A, 0);
        }
        else if (LV_KEY_END == keyVal) {
            LV_LOG_ERROR("END键，keyVal = %d\n", keyVal);
            if (g_curBtn < 0) {
                g_isPause = !g_isPause;
                //updateStateView();//更新状态视图

                lv_event_stop_processing(evt);//停止当前事件的处理，不再执行后续的事件回调函数
                lv_indev_keypad_wait_release();//等待键盘输入设备的按键释放
            }

        }
        else if (LV_KEY_EX_FUNC1 == keyVal) { 
            LV_LOG_ERROR("F1键，keyVal = %d\n", keyVal);
            g_isPause = !g_isPause;
            lv_event_stop_processing(evt);
            lv_indev_keypad_wait_release();

            delete_button_callback_function(evt);
            //_my_Confirm_delete_pop_ups(evt);//删除按钮回调函数
        }
        else if (LV_KEY_NEXT == keyVal) {  //9
        LV_LOG_ERROR("Tab键，keyVal = %d\n", keyVal);
        if (g_curBtn < 0) {
            lv_event_stop_processing(evt);//停止当前事件的处理，不再执行后续的事件回调函数
            lv_indev_keypad_wait_release();//等待键盘输入设备的按键释放
        }
        bottom_tip_label();//提示语
        }
    }
    else if (LV_EVENT_DELETE == evtCode) {
        ESP_LOGI(TAG, "%d：_ckbox_Cb对象正在被删除 删除组件", __LINE__);
        //ESP_LOGI(TAG, "编译日期与时间: %s，%s\n", __DATE__, __TIME__);
        //ESP_LOGI(TAG, "当前所在行号:%d\r\n", __LINE__);

        MDLD_doFreeIfNotNull(heap_caps_free, checkbox_status_albums);
        MDLD_doFreeIfNotNull(heap_caps_free, label_tmp);
        MDLD_doFreeIfNotNull(heap_caps_free, g_btns);
        MDLD_doFreeIfNotNull(heap_caps_free, checkbox_albums);
    }

}

