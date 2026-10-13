
#include <lvgl/lvgl.h>
#include <lvgl/src/misc/lv_types.h>
#include "../lvgl_main.h"

#include "../baidu_netdisk/web_disk_playback_display.h"
#include "my_download.h"
#include "../data_parse/img_res.h"
#include "../mainmenu/tlb_main_img.h"

#include "../utils/misc_utils.h"

extern int _disk_playback_get_cur_sel(void);
extern int MDLD_dld_data_RAM_alloc(void);
extern void MDLD_dld_data_RAM_free(void);

//————
#define TAG "my_download_select.c"

#define C_MAIN_WIN_W   SCREEN_WIDTH  //窗口宽度
#define C_FORM_PADDING_W  1          //设置窗口或控件的填充宽度

#define M_ARRAY_LEN(arr)  (sizeof(arr)/sizeof(arr[0]))


//--------------------------------------
static lv_obj_t* img = NULL;
static lv_obj_t* mainWin = NULL;
static lv_obj_t* g_mainWin = NULL;


static lv_obj_t** label_tmp = NULL;
static lv_obj_t** btn = NULL;
static lv_obj_t** g_btns = NULL;

static short g_curBtn = -1;


int my_download_select_scr_free(void);

static void No_content_situation_window_layout(void);//无内容情况窗口布局
static void There_content_situation_window_layout(void);//有内容情况窗口布局
static void No_content_reminder(void);//暂无内容
static void _my_imgbtn_xuanze_cb(lv_event_t* e);//选择按钮回调函数
static void btn_event_callback(lv_event_t* event);//按钮事件回调函数++
static void _myMainWinCb(lv_event_t* evt);//键盘处理回调函数
static void _myEvent_cb(lv_event_t* evt);//处理按键事件
static int _my_btn_second_level_directory_cb(lv_event_t* evt);//按钮回调函数 二级目录
static int exit_dir_interfaces_all_levels(void);//退出各级目录或者界面

bool enter_flag_bit_button_callback_function = 0;//由按钮回调函数进入标志位
int Current_SD_card_directory_depth = 1;//当前SD卡目录深度层数   以SD卡名称为1

int16_t Save_focus_val_all_lev_dir[DIRECTORY_LEVEL_MAX + 1]; //保存各级目录的焦点值
static char* save_absolute_paths_dir_all_levels[DIRECTORY_LEVEL_MAX + 1];//保存各级目录的绝对路径

#define MY_TOP_WIN_H    50
#define MY_LST_ITEM_W   LV_PCT(100)   // 195

//--------------------------------------
static const lv_coord_t col_dsc[] = { LV_GRID_FR(1),  LV_GRID_TEMPLATE_LAST };//列1
static const lv_coord_t row_dsc[] = { MY_TOP_WIN_H,  LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST }; //第一行高60  第二行剩下全部

static int myDld_sel_RAM_alloc(void)
{
	for (int16_t level = 1; level < DIRECTORY_LEVEL_MAX + 1; level++) {
		save_absolute_paths_dir_all_levels[level] = (char*)heap_caps_malloc_dflt(PATH_BYTE_SIZE);//各级目录绝对路径开辟空间
	}

	for (int16_t level = 1; level < DIRECTORY_LEVEL_MAX + 1; level++) {
		Save_focus_val_all_lev_dir[level] = 1;
	}
	return 0;
}

static void myDld_sel_RAM_free(void)
{
	for (int16_t level = 1; level < DIRECTORY_LEVEL_MAX + 1; level++) {
		MDLD_doFreeIfNotNull(heap_caps_free, save_absolute_paths_dir_all_levels[level]);
	}
	return;
}

static void myDld_esc_btn_click_cb(lv_event_t evt)  // todo
{
	exit_dir_interfaces_all_levels();//退出目录或者界面	
}

void mydld_createEscBtn(lv_obj_t * parent, lv_event_cb_t clickCb, int extra)
{
#define DLD_ESC_BTN_LOC_Y  10
#define TOP_BTN_BORDER 7
	lv_obj_t * img = lv_img_create(parent);
	const lv_img_dsc_t * pImgDsc = g_XXX_img_dsc_tbl + MMN_icn_esc - 1;
	lv_img_set_src(img, pImgDsc);
	lv_obj_set_size(img, pImgDsc->header.w + TOP_BTN_BORDER * 2, pImgDsc->header.h + TOP_BTN_BORDER * 2);
	lv_obj_set_pos(img, 16, DLD_ESC_BTN_LOC_Y);
	lv_obj_set_style_border_width(img, TOP_BTN_BORDER, 0);
	lv_obj_set_style_radius(img, 4, 0);
	lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);

	lv_obj_set_style_opa(img, LV_OPA_60, 0);
	lv_obj_set_style_opa(img, LV_OPA_100, LV_STATE_FOCUS_KEY);
	lv_obj_set_style_opa(img, LV_OPA_80, LV_STATE_PRESSED);
	lv_obj_set_style_border_opa(img, LV_OPA_0, 0);
	lv_obj_add_event_cb(img, clickCb, LV_EVENT_CLICKED, NULL);
}


//我的下载选择版面(lst or noContent)
int my_download_select_scr_init(void)
{
    LV_LOG_USER("已进入：我的下载 A\n");

    //Print_directory_tree(SD_CARD_FILE_NAME,1);//打印出指定目录的目录树
    //xor_encryption_main(); //文件内容加密与解密——》异或加密

    ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_MYD_scrNo, zb_getOpenedFileNum());

    g_MYD_scrNo = 1;
    int openedN = zb_getOpenedFileNum();
    ESP_LOGI(TAG, "%d initAp, openedN %d", g_MYD_scrNo, openedN);

    //从主菜单进入时，读取SD卡中的内容
    //从按钮回调函数进入时，那个按钮的子目录,就不传SD卡的名称了
    //从主菜单进入时
    if (enter_flag_bit_button_callback_function == 0) {
        strcpy(save_absolute_paths_dir_all_levels[1], SD_CARD_FILE_NAME);
        ESP_LOGI(TAG, "save_absolute_paths_dir_all_levels第1级目录的绝对路径：%s\n", save_absolute_paths_dir_all_levels[1]);
        Current_SD_card_directory_depth = 1;//目录深度
        
        My_download_file_directory_data_processing(SD_CARD_FILE_NAME, Current_SD_card_directory_depth);
        
        g_curBtn = 0;//焦点

        strcpy(previous_level_directory, SD_CARD_FILE_NAME);
        ESP_LOGI(TAG, "首级目录的previous_level_directory:%s\n", previous_level_directory);

#if 0  // #ifdef C_PC_SIM  //PC端测试
        Number_secondary_file_dir = 1;
#endif
    }

    //Print_directory_tree("/userdata",0);//打印出指定目录的目录树

    //我的下载中无内容情况
    if (Number_secondary_file_dir <= 0) {
        No_content_situation_window_layout();//无内容情况窗口布局
        No_content_reminder();//暂无内容提示
    }
    //我的下载中有内容情况
    if (Number_secondary_file_dir > 0) {
        There_content_situation_window_layout();//有内容情况窗口布局
        ESP_LOGI(TAG, "%d：AP=%d，当前打开的文件数量:%d", __LINE__, g_MYD_scrNo, zb_getOpenedFileNum());

        label_tmp = (lv_obj_t**)heap_caps_malloc_dflt(NUMBER_FILES_MAX * sizeof(lv_obj_t*));
        memset(label_tmp, 0, NUMBER_FILES_MAX * sizeof(lv_obj_t*));//避免野指针

        btn = (lv_obj_t**)heap_caps_malloc_dflt(NUMBER_FILES_MAX * sizeof(lv_obj_t*));
        memset(btn, 0, NUMBER_FILES_MAX * sizeof(lv_obj_t*));//避免野指针

        g_btns = (lv_obj_t**)heap_caps_malloc_dflt(sizeof(lv_obj_t*) * NUMBER_FILES_MAX);
        memset(g_btns, 0, NUMBER_FILES_MAX * sizeof(lv_obj_t*));//避免野指针

        ESP_LOGI(TAG, "%d：Number_secondary_file_dir = %d", __LINE__, Number_secondary_file_dir);
        if (Number_secondary_file_dir > NUMBER_FILES_MAX) {
            ESP_LOGW(TAG, "%d：Number_secondary_file_dir = %d", __LINE__, Number_secondary_file_dir);
            Number_secondary_file_dir = NUMBER_FILES_MAX;
        }

        //循环创建
        for (int i = 0; i < Number_secondary_file_dir; i++)
        {
            btn[i] = lv_btn_create(mainWin);//创建按钮
            lv_obj_t * lblOne = lv_label_create(btn[i]);//创建标签 
            label_tmp[i] = lblOne;
            lv_label_set_text(lblOne, second_level_directory[i]);
            lv_label_set_long_mode(lblOne, LV_LABEL_LONG_DOT); // LV_LABEL_LONG_SCROLL_CIRCULAR);//滚动显示  ——》无效
            lv_obj_set_style_anim_speed(lblOne, 50, 0);//滚动速度
            lv_obj_set_size(lblOne, SCREEN_WIDTH - 70, 21);//必须设置大小滚动显示才有效
            lv_obj_set_style_border_width(lblOne, 0, 0);
            lv_obj_add_event_cb(btn[i], _my_btn_second_level_directory_cb, LV_EVENT_SHORT_CLICKED, NULL);//按键控制，按钮被短按时被调用 LV_EVENT_SHORT_CLICKED
            lv_obj_set_size(btn[i], MY_LST_ITEM_W, 50);       // 设置控件大小
            //lv_obj_align_to(btn1, mainWin, LV_ALIGN_TOP_MID, 0, 0);//按钮位置   不需要
            lv_obj_align_to(label_tmp[i], btn[i], LV_ALIGN_LEFT_MID, 23, 0);//标签位置
            lv_obj_set_style_radius(btn[i], 15, 15);//设置btnA按钮的样式圆角半径为0，即按钮没有圆角。
            lv_obj_set_style_bg_color(btn[i], lv_color_hex(0xf6f6f6), LV_STATE_DEFAULT);//默认状态 灰色
            lv_obj_set_style_bg_color(btn[i], lv_color_hex(0x79B9FF), LV_STATE_FOCUS_KEY);//键盘聚焦状态  蓝色
            lv_obj_set_style_bg_color(btn[i], lv_color_hex(0x79B9FF), LV_STATE_PRESSED);//已按下  
            lv_obj_set_style_text_color(btn[i], lv_color_hex(0x000000), 0);//默认状态 标签颜色为黑色
            lv_obj_set_style_text_color(btn[i], lv_color_hex(0xffffff), LV_STATE_FOCUS_KEY);//键盘聚焦状态下改变按钮里字体颜色 白色   继承
            g_btns[i] = btn[i];

        }
    }

    ESP_LOGI(TAG, "g_curBtn = %d,", g_curBtn);
    ESP_LOGI(TAG, "Number_secondary_file_dir = %d", Number_secondary_file_dir);
    if (g_curBtn >= 0) {
        ESP_LOGI(TAG, "g_btns = %p,", g_btns);
        if (g_curBtn >= Number_secondary_file_dir) {
            ESP_LOGI(TAG, "同一界面不同版面下，上一个版面最后的焦点g_curBtn值，大于当前版面的按钮数");
            g_curBtn = Number_secondary_file_dir-1;
        }
        if (Number_secondary_file_dir != 0) { //防止在上一界面全部删除后，再聚焦会报错
            if (g_curBtn >= 1) { //不要让它滚动到第一个选择删除按钮
                lv_obj_scroll_to_view(g_btns[g_curBtn], LV_ANIM_OFF);//滚动到一个对象，直到它在它的父对象上可见
            }
            lv_group_focus_obj(g_btns[g_curBtn]); //将焦点设置到按钮对象上
            ESP_LOGI(TAG, "焦点已设置到按钮上");
            //如果是文件按钮 设置滚动
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
    sys_setCurMainWinAndCb(img, _myMainWinCb);//自定义函数将mainWin作为当前主窗口。
    lv_obj_add_event_cb(img, _myMainWinCb, LV_EVENT_ALL, NULL);//为mainWin窗口添加一个事件回调函数_myMainWinCb，该回调函数将处理所有类型的事件

    mydld_createEscBtn(img, myDld_esc_btn_click_cb, 0);

    g_mainWin = img;
    return 0;
}

int zb_ap_my_download_select_init(void)
{
    ESP_LOGI(TAG, "initAp A");
    MDLD_dld_data_RAM_alloc();
    myDld_sel_RAM_alloc();

    my_download_select_scr_init();
    return 0;
}


//删除主界面:我的下载
//lv_event_code_t msg：第一个参数是LVGL事件代码，用于区分不同的事件。
//int param：第二个参数是整型参数，可能包含与事件相关的附加信息
int zb_ap_my_download_hanlder(lv_event_code_t msg, int param) {
    LV_LOG_USER("已进入：删除主界面:我的下载zb_example_ap_my_download_hanlder\n");
    switch (msg) {
    case EVENT_FREE_AP_RES:
        if (g_mainWin) {
            lv_obj_del(g_mainWin);  g_mainWin = NULL;
        }
    #ifndef C_PC_SIM
        music_player_stop();
        music_player_destroy();
    #endif
        my_download_select_scr_free();
        my_download_edit_scr_free();
        zb_example_ap_my_download_display_free();
        MDLD_dld_data_RAM_free();
    #if 0 // todo
        zb_ap_web_disk_playback_display_free();
    #endif
        myDld_sel_RAM_free();
        break;
     case EVENT_EX_CMD:    //自定义事件：清除小部件
        ESP_LOGI(TAG, "recv EX_CMD %x", param);
        if ((uint32_t)param > 0x100u) {  //无符号32位整数，其值为256（十六进制的100）
            zb_func_void fnA = (zb_func_void)param;
            (*fnA)();
        }
        else {
            if (SWITCH_PLAYER_MYDOWN == param) {  // todo, after music player, switch screen;
                ESP_LOGI(TAG, "%d：param = 0x14 待办事项，音乐播放后，切换画面", __LINE__);
                lv_group_focus_obj(g_mainWin);//切换回焦点 ??
                sys_setCurMainWinObj(g_mainWin);//自定义函数将g_mainWin作为当前主窗口。
                //
                int selNo = _disk_playback_get_cur_sel();
                short selNoBak = Save_focus_val_all_lev_dir[Current_SD_card_directory_depth];
                Save_focus_val_all_lev_dir[Current_SD_card_directory_depth] = selNo;
                if (selNo < 0) {
                    selNo = 0;
                }
                lv_group_focus_obj(g_btns[selNo]); //将焦点设置到按钮对象上
                g_curBtn = selNo;
                //关闭滚动动画
                lv_obj_scroll_to_view(g_btns[g_curBtn], LV_ANIM_OFF);//滚动到一个对象，直到它在它的父对象上可见
                if (selNo != selNoBak) {
                    lv_label_set_long_mode(label_tmp[selNoBak], LV_LABEL_LONG_DOT);
                    lv_label_set_long_mode(label_tmp[selNo], LV_LABEL_LONG_SCROLL_CIRCULAR);
                }
                ESP_LOGI(TAG, "%d：焦点已设置到按钮上",__LINE__);
            }
        }
        break;
    default:
        break;
    }
    return 0;
}

//删除界面 我的下载  选择界面
int my_download_select_scr_free(void) {
    int openedN = zb_getOpenedFileNum();
    //ESP_LOGI(TAG, "%d exitAp, openedN %d", g_UCC_scrNo, openedN);
    if (g_mainWin != NULL) {
        lv_obj_del(g_mainWin);  g_mainWin = NULL;
    }
    return 0;
}

//按钮回调函数 二级目录second-level directory
//获取是哪个按钮触发的，
//  如果是文件夹：进入下一级目录  二——》三
//  如果是文件：
//          判断.txt显示
//          判断.mp3播放
static int _my_btn_second_level_directory_cb(lv_event_t* evt) {
    LV_LOG_USER("已进入：_my_btn_second_level_directory_cb\n");

    lv_obj_t* target_pinyin_types = lv_event_get_target(evt); //获取触发源
    for (int i = 0; i < Number_secondary_file_dir; i++) {
        if (target_pinyin_types == btn[i]) { //触发那个按钮，就处理那个按钮的
            ESP_LOGI(TAG, "触发的是第%d个按钮", i);
            ESP_LOGI(TAG, "——————————————————————————");
            //存焦点
            Save_focus_val_all_lev_dir[Current_SD_card_directory_depth] = g_curBtn;//保存各级目录的焦点值
            ESP_LOGI(TAG, "目录深度：%d，存入的焦点：%d", Current_SD_card_directory_depth, g_curBtn);
            for (int i = 1; i <= DIRECTORY_LEVEL_MAX; i++) {
                ESP_LOGI(TAG, "存焦点Save_focus_val_all_lev_dir[%d] = %d",i, Save_focus_val_all_lev_dir[i]);
            }
            g_curBtn = 0;//下一级焦点从0开始
            ESP_LOGI(TAG, "——————————————————————————");

            //由按钮回调函数进入标志位
            enter_flag_bit_button_callback_function = 1;
            //遍历子目录信息
            Print_file_directory(second_level_directory[i]);
            //获取子目录信息
            ESP_LOGI(TAG, "绝对路径是：%s", second_son_absolute_path[i]);
            strlcpy(previous_level_directory, second_son_absolute_path[i], PATH_BYTE_SIZE);
            ESP_LOGI(TAG, "选择界面最后定位路径previous_level_directory:%s\n", previous_level_directory);

            struct stat buf;
            int st_mode = 0;
            int result = zb_stat__st_mode(second_son_absolute_path[i], &st_mode);
            if (result >= 0) {
                ESP_LOGI(TAG, "文件%s", second_son_absolute_path[i]);
            }
            else {
                ESP_LOGE(TAG, "文件%s打开失败！", second_son_absolute_path[i]);
                return -1;
            }

            ESP_LOGI(TAG, "buf.st_mode:0x%X", st_mode);
            //如果这个按钮是文件夹：
            if (S_IFDIR & st_mode) {
                ESP_LOGI(TAG, "~这个按钮是文件夹~");
                //保存各级目录的绝对路径
                Current_SD_card_directory_depth++;
                if (Current_SD_card_directory_depth > DIRECTORY_LEVEL_MAX) {
                    ESP_LOGW(TAG, "当前目录深度%d大于最大目录深度%d", Current_SD_card_directory_depth, DIRECTORY_LEVEL_MAX);
                    Current_SD_card_directory_depth = DIRECTORY_LEVEL_MAX;
                }
                //
                strlcpy(save_absolute_paths_dir_all_levels[Current_SD_card_directory_depth], second_son_absolute_path[i], PATH_BYTE_SIZE);
                ESP_LOGI(TAG, "第%d级目录的绝对路径：%s\n", Current_SD_card_directory_depth, save_absolute_paths_dir_all_levels[Current_SD_card_directory_depth]);

                // 我的下载文件目录数据处理
                My_download_file_directory_data_processing(second_son_absolute_path[i], Current_SD_card_directory_depth);
                if (g_mainWin != NULL) {
                    lv_obj_del(g_mainWin);  g_mainWin = NULL;  // 销毁当前版面
                }
                my_download_select_scr_init();//重新加载
                break;
            }
            //如果这个按钮是文件：文件显示和播放
            else if(S_IFREG & st_mode) {
                ESP_LOGI(TAG, "~fileItm~");
                //存文件路径
                Baidu_displays_absolute_path_file_played = (char*)heap_caps_malloc_dflt(PATH_BYTE_SIZE);
                ESP_LOGI(TAG, "当前所在行号:%d\r\n", __LINE__);
                strlcpy(Baidu_displays_absolute_path_file_played, second_son_absolute_path[i], PATH_BYTE_SIZE);//要显示播放的文件绝对路径
                ESP_LOGI(TAG, "当前所在行号:%d\r\n", __LINE__);
                ESP_LOGI(TAG, "进入播放器时的焦点Current_SD_card_directory_depth = %d,", Current_SD_card_directory_depth);
                ESP_LOGI(TAG, "进入播放器时的焦点Save_focus_val_all_lev_dir[Current_SD_card_directory_depth] = %d,", Save_focus_val_all_lev_dir[Current_SD_card_directory_depth]);
                ESP_LOGI(TAG, "当前所在行号:%d\r\n", __LINE__);
                player_toggle_layout_flag_bit = 3;//列表到——》播放器
                zb_ap_web_disk_playback_display_init();//——》播放器
            }
            break;
        }

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
    lv_obj_set_size(mainWin, SCREEN_WIDTH, SCREEN_HEIGHT- 42);
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


#ifndef C_PC_SIM
#include <stdio.h>
#endif	
//有内容情况窗口布局 There is a content situation window layout
static void There_content_situation_window_layout(void) {
    LV_LOG_USER("已进入：有内容情况窗口布局\n");

#if 0 // ifndef C_PC_SIM 
    ESP_LOGI(TAG, "%d：--堆栈信息--",__LINE__);
    ESP_LOGI(TAG, "SRAM当前空闲内存：%8d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGI(TAG, "SRAM总大小：%8d", heap_caps_get_total_size(MALLOC_CAP_INTERNAL));
    ESP_LOGI(TAG, "PSRAM当前空闲内存：%8d", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGI(TAG, "PSRAM总大小：%8d", heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
#endif

    //背景
    ESP_LOGI(TAG, "%d：~!@##$$$", __LINE__);
    img = lv_obj_create(lv_scr_act());
    ESP_LOGI(TAG, "%d：~!@##$$$", __LINE__);
    lv_img_dsc_t* beijing = zb_img_res_load_one(NULL, MY_DOWNLOAD_IMG_BEIJING, IMG_FREE_TYPE_AUTO, NULL);
    lv_obj_set_style_bg_img_src(img, beijing, 0);//设置图形对象背景图片
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

    ESP_LOGI(TAG, "%d：~!@##$$$", __LINE__);

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

    ESP_LOGI(TAG, "%d：~!@##$$$", __LINE__);

    //头标签
    lv_obj_t* shz = lv_label_create(top_window);
    if (NULL != shz)
    {
        lv_obj_set_size(shz, 80, 31);                 // 设置控件大小
        lv_obj_align(shz, LV_ALIGN_TOP_LEFT, 20, 20);
        lv_label_set_text(shz, "我的下载");             // 初始显示 0
        lv_obj_set_style_text_color(shz, lv_color_hex(0xffffff), 0);//白色
    }

    ESP_LOGI(TAG, "%d：~!@##$$$", __LINE__);

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

    ESP_LOGI(TAG, "%d：~!@##$$$",__LINE__);

    //选择按钮
    static lv_style_t style_pr;//收藏样式
    lv_style_init(&style_pr);//初始化style_pr样式
    lv_style_set_img_recolor(&style_pr, lv_color_make(0xFF, 0x00, 0x00));//设置按下样式中图像的重新着色颜色为
    lv_obj_t* xuanze_btnimg = lv_imgbtn_create(top_window);//创建图片按钮
    lv_obj_remove_style_all(xuanze_btnimg);//从xuanze_btnimg对象上移除所有已应用的样式
    //lv_img_dsc_t* xuanze_tu = zb_img_res_load_one(NULL, MY_DOWNLOAD_IMG_XUANZE, IMG_FREE_TYPE_AUTO, NULL);
    lv_img_dsc_t* xuanze_tu = zb_img_res_load_one(NULL, MY_DOWNLOAD_IMG_ABBIANJI, IMG_FREE_TYPE_AUTO, NULL);
    lv_imgbtn_set_src(xuanze_btnimg, LV_IMGBTN_STATE_RELEASED, xuanze_tu, NULL, NULL);//设置图片源
    //lv_obj_align_to(xuanze_btnimg, top_window, LV_ALIGN_TOP_RIGHT, 50, 7);
    lv_obj_align_to(xuanze_btnimg, top_window, LV_ALIGN_TOP_RIGHT, 50, 13);
    lv_obj_add_style(xuanze_btnimg, &style_pr, LV_STATE_PRESSED);//为图像按钮在按下状态下添加样式
    lv_obj_add_event_cb(xuanze_btnimg, _my_imgbtn_xuanze_cb, LV_EVENT_SHORT_CLICKED, NULL);//按键回调函数

    ESP_LOGW(TAG, "%d：有内容情况窗口布局OK", __LINE__);
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
    lv_obj_set_size(label_tishi, SCREEN_WIDTH, LV_SIZE_CONTENT);
    lv_label_set_recolor(label_tishi, true);  //使能颜色修改              
    lv_label_set_text(label_tishi, "#000000 提示：\n展示内容为网盘下载内容以\n及拷贝至储存卡的内容#");
}

//选择按钮回调函数
static void _my_imgbtn_xuanze_cb(lv_event_t* e) {

    ESP_LOGI(TAG, "已进入：选择按钮回调函数_my_imgbtn_xuanze_cb");

    if (Number_secondary_file_dir > 0) {
        MYD_switchScr(C_ZB_AP_MYD_SON_B, 0);
    }
    else {
        ESP_LOGE(TAG, "实际下载文件数量小于0，不可有MYD_switchScr(C_ZB_AP_MYD_SON_B, 0);");
    }
    //MYD_switchScr(C_ZB_AP_MYD_SON_C, 0);
}

//退出各级目录或者界面
static int exit_dir_interfaces_all_levels(void) {

    ESP_LOGI(TAG, "已进入：退出各级目录或者界面exit_dir_interfaces_all_levels");
    ESP_LOGI(TAG, "Current_SD_card_directory_depth目录的深度为：%d", Current_SD_card_directory_depth);
    save_absolute_paths_dir_all_levels[Current_SD_card_directory_depth][0] = '\0';
    Current_SD_card_directory_depth--;
    //如果上一级名称与SD卡名称相同，或者 目录深度为1，返回主菜单
    if (Current_SD_card_directory_depth == 0) {
        ESP_LOGI(TAG, "退主菜单");
        ESP_LOGI(TAG, "Current_SD_card_directory_depth目录的深度为：%d", Current_SD_card_directory_depth);
        //lv_obj_del(g_mainWin);  g_mainWin = NULL;
        enter_flag_bit_button_callback_function = 0;//退出时 还原 由按钮回调函数进入标志位

        //清空焦点
        for (int i = 1; i <= DIRECTORY_LEVEL_MAX; i++) {
            Save_focus_val_all_lev_dir[i] = 0;
        }

        zb_requestSwitchAp(C_ZB_AP_MAINMENU);//切换到另一个应用程序或模式  主菜单
    }
    else {
        //退目录
        //把存储的相应级别目录 给 当前级别目录
        ESP_LOGI(TAG, "退目录");
        ESP_LOGI(TAG, "save_absolute_paths_dir_all_levels第1级目录的绝对路径：%s", save_absolute_paths_dir_all_levels[1]);
        ESP_LOGI(TAG, "Current_SD_card_directory_depth目录的深度为：%d", Current_SD_card_directory_depth);
        
        ESP_LOGI(TAG, "——————————————————————————");
        //取焦点
        g_curBtn = Save_focus_val_all_lev_dir[Current_SD_card_directory_depth];//保存各级目录的焦点值
        ESP_LOGI(TAG, "目录深度：%d，取出的焦点：%d", Current_SD_card_directory_depth, g_curBtn);
        for (int i = 1; i <= DIRECTORY_LEVEL_MAX; i++) {
            ESP_LOGI(TAG, "取焦点Save_focus_val_all_lev_dir[%d] = %d", i, Save_focus_val_all_lev_dir[i]);
        }
        //退出时清除后面的焦点
        for (int i = Current_SD_card_directory_depth+1; i <= DIRECTORY_LEVEL_MAX; i++) {
            Save_focus_val_all_lev_dir[i] = 0;
        }
        for (int i = 1; i <= DIRECTORY_LEVEL_MAX; i++) {
            ESP_LOGI(TAG, "取焦点清理后Save_focus_val_all_lev_dir[%d] = %d", i, Save_focus_val_all_lev_dir[i]);
        }
        ESP_LOGI(TAG, "——————————————————————————");

        //打印各级路径
        for (int i = 1; i <= Current_SD_card_directory_depth; i++) {
            ESP_LOGI(TAG, "save_absolute_paths_dir_all_levels第%d级目录的绝对路径：%s", i, save_absolute_paths_dir_all_levels[i]);
        }
        //Current_SD_card_directory_depth--;
        ESP_LOGI(TAG, "即将进入的目录的深度为：%d", Current_SD_card_directory_depth);

        ESP_LOGI(TAG, "======即将退入的目录：%s", save_absolute_paths_dir_all_levels[Current_SD_card_directory_depth]);
        //ESP_LOGI(TAG, "子级目录名称：%s", second_level_directory[Current_SD_card_directory_depth - 1]);
        //strcpy(second_level_directory[Current_SD_card_directory_depth-1], save_absolute_paths_dir_all_levels[Current_SD_card_directory_depth]);
        //ESP_LOGI(TAG, "++++++即将进入的目录：%s", second_level_directory[Current_SD_card_directory_depth - 1]);
        //My_download_file_directory_data_processing(second_level_directory[Current_SD_card_directory_depth-1], Current_SD_card_directory_depth);// 我的下载文件目录数据处理
        My_download_file_directory_data_processing(save_absolute_paths_dir_all_levels[Current_SD_card_directory_depth], Current_SD_card_directory_depth);// 我的下载文件目录数据处理
        ESP_LOGI(TAG, "!!!!!!!即将退入的目录：%s", save_absolute_paths_dir_all_levels[Current_SD_card_directory_depth]);

        lv_obj_del(g_mainWin);  g_mainWin = NULL;//销毁当前版面，重新加载
        my_download_select_scr_init();//重新加载
    }

    return 0;
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
        //检查是否是左右箭头键
        if (LV_KEY_LEFT == keyVal || LV_KEY_RIGHT == keyVal) {
            LV_LOG_USER("左右箭头键\n");
        }
        //检查是否是上下箭头键
        if (LV_KEY_UP == keyVal || LV_KEY_DOWN == keyVal) {
            LV_LOG_USER("上下箭头键\n");
            ESP_LOGI(TAG, "g_curBtn = %d\n", g_curBtn);

            //我的下载中无内容情况
            if (Number_secondary_file_dir <= 0) {
                ESP_LOGI(TAG, "我的下载中无内容情况不要进入上下键");
            }
            else {
                short selNoPrev = g_curBtn;
                if (LV_KEY_DOWN == keyVal) {   //判断向下的按键是否按下
                    g_curBtn = (g_curBtn + 1) % Number_secondary_file_dir;//NUM_CHINESE_FILES;  // debug
                    ESP_LOGI(TAG, "向下g_curBtn = %d\n", g_curBtn);
                }
                else if (LV_KEY_UP == keyVal) {  //上
                    g_curBtn = (g_curBtn + Number_secondary_file_dir - 1) % Number_secondary_file_dir;
                    ESP_LOGI(TAG, "向上g_curBtn = %d\n", g_curBtn);
                }

                if (g_btns[g_curBtn]) {
                    lv_group_focus_obj(g_btns[g_curBtn]); //将焦点设置到按钮对象上
                    ESP_LOGI(TAG, "lv_obj_has_state：%d", lv_obj_has_state(g_btns[g_curBtn], LV_STATE_FOCUS_KEY));

                    if (g_curBtn != selNoPrev) {
                        lv_label_set_long_mode(label_tmp[selNoPrev], LV_LABEL_LONG_DOT);
                        lv_label_set_long_mode(label_tmp[g_curBtn], LV_LABEL_LONG_SCROLL_CIRCULAR);
                    }
                }
                //lv_group_focus_obj(g_btns[g_curBtn]); //将焦点设置到按钮对象上
            }

            lv_event_stop_processing(evt); //停止事件的进一步处理
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
            ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_MYD_scrNo, zb_getOpenedFileNum());
            if (g_curBtn < 0) {
                g_isPause = !g_isPause;
                //updateStateView();//更新状态视图
                lv_event_stop_processing(evt);//停止当前事件的处理，不再执行后续的事件回调函数
                lv_indev_keypad_wait_release();//等待键盘输入设备的按键释放
            }
            lv_event_stop_processing(evt);
            lv_indev_keypad_wait_release();
            exit_dir_interfaces_all_levels();//退出目录或者界面
        }
        else if (LV_KEY_END == keyVal) {
            LV_LOG_ERROR("END键，keyVal = %d\n", keyVal);
            if (g_curBtn < 0) {
                g_isPause = !g_isPause;
                //updateStateView();//更新状态视图

                lv_event_stop_processing(evt);//停止当前事件的处理，不再执行后续的事件回调函数
                lv_indev_keypad_wait_release();//等待键盘输入设备的按键释放
            }
            MYD_switchScr(C_ZB_AP_MYD_SON_C, 0);

        }
        else if (LV_KEY_EX_FUNC1 == keyVal) {  //调用收藏按钮
            LV_LOG_ERROR("F1键，keyVal = %d\n", keyVal);
            g_isPause = !g_isPause;
            lv_event_stop_processing(evt);
            lv_indev_keypad_wait_release();

            _my_imgbtn_xuanze_cb(evt);//选择按钮回调函数
        }
    }
    else if (LV_EVENT_DELETE == evtCode) {
        ESP_LOGI(TAG, "%d：_ckbox_Cb对象正在被删除 删除组件", __LINE__);
        ESP_LOGI(TAG, "编译日期与时间: %s，%s\n", __DATE__, __TIME__);
        if (label_tmp != NULL) {
            heap_caps_free(label_tmp);  label_tmp = NULL;
        }
        if (btn != NULL) {
            heap_caps_free(btn);  btn = NULL;
        }
        if (g_btns != NULL) {
            heap_caps_free(g_btns);  g_btns = NULL;
        }
    }

}


/**
* 各版面的定义表
 */
const struct MYD_SCR_INF my_downloadScrInfTbl[] = {
    { NULL, NULL },
    { my_download_select_scr_init, my_download_select_scr_free },
    { my_download_edit_scr_init, my_download_edit_scr_free },
    { zb_example_ap_my_download_display_init,zb_example_ap_my_download_display_free },
    { NULL, NULL }
};

int8_t  g_MYD_scrNo;  // start from 1


/*
* 切换版面
 */
void MYD_switchScr(uint8_t scrNoNew, int parm)
{
    if (scrNoNew < 0 || scrNoNew >= (sizeof(my_downloadScrInfTbl) / sizeof(my_downloadScrInfTbl[0]))) {
        return;
    }
    //if (scrNoNew == g_MYD_scrNo)
    //   return;
    if (g_MYD_scrNo >= 0) {
        FN_MYD_SCREEN_FREE fnFree = my_downloadScrInfTbl[g_MYD_scrNo].m_fn_free;
        if (fnFree) {  // free res of old screen
            (*fnFree)();
        }
    }
    g_MYD_scrNo = scrNoNew;
    FN_MYD_SCREEN_INIT fnInit = my_downloadScrInfTbl[g_MYD_scrNo].m_fn_init;
    if (fnInit) {  // free res of old screen
        (*fnInit)();
    }
    return;
}

