//————————————————————————————————————————————————
// 显示与播放
//————————————————————————————————————————————————

//我的下载中
//显示文件内容.txt、.bin、.dat、.mnu、.gitignore……
//显示图片.jpg、.png
//播放音乐文件.mp3、wav

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
#include "../data_parse/analyze_listening.h"

//————
#define TAG "my_download_display.c"

static lv_obj_t* img;
static lv_obj_t* mainWin;
static lv_obj_t* g_mainWin;
static lv_obj_t* top_window;

int zb_example_ap_my_download_display_init(void);//我的下载显示
int zb_example_ap_my_download_display_free(void);//删除界面 我的下载  显示界面

static void _myMainWinCb(lv_event_t* evt);//键盘处理回调函数
static int My_download_specified_file_processing(const char* filename);//我的下载指定文件处理
static int Plays_specified_music_file(const char* file_display);//播放指定音乐文件
static int Displays_specified_image_file(const char* file_display);//显示指定图片文件
static int Displays_contents_specified_file(const char* file_display);//显示指定文件的内容

#define MY_TOP_WIN_H    50
#define MY_LST_ITEM_W   LV_PCT(100)   // 195
static const lv_coord_t col_dsc[] = { LV_GRID_FR(1),  LV_GRID_TEMPLATE_LAST };//列1
static const lv_coord_t row_dsc[] = { MY_TOP_WIN_H,  LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST }; //第一行高60  第二行剩下全部


//我的下载显示
int zb_example_ap_my_download_display_init(void)
{
    LV_LOG_USER("已进入：我的下载显示zb_example_ap_my_download_display_init\n");
    ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_MYD_scrNo, zb_getOpenedFileNum());

    //背景
    img = lv_obj_create(lv_scr_act());
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

    //顶部窗口
    top_window = lv_obj_create(img);
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

    //头标签——》左
    lv_obj_t* shz = lv_label_create(img);
    if (NULL != shz)
    {
        lv_obj_set_size(shz, 80, 31);                 // 设置控件大小
        lv_obj_align(shz, LV_ALIGN_TOP_LEFT, 20, 20);
        lv_label_set_text(shz,"我的下载\n");             // 初始显示 0
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
    lv_obj_set_scroll_dir(mainWin, LV_DIR_ALL);//设置滚动方向  LV_DIR_HOR水平滚动，LV_DIR_VER垂直滚动  垂直+水平

    //我的下载指定文件处理
    My_download_specified_file_processing(previous_level_directory);

    lv_group_add_obj(lv_group_get_default(), img);//这行代码将winA对象添加到默认的对象组中。在LVGL中，对象组用于处理键盘输入和导航。
    sys_setCurMainWinObj(img);//自定义函数将mainWin作为当前主窗口。
    lv_obj_add_event_cb(img, _myMainWinCb, LV_EVENT_ALL, NULL);//为mainWin窗口添加一个事件回调函数_myMainWinCb，该回调函数将处理所有类型的事件

    g_mainWin = img;

    return 0;
}

//删除界面 我的下载  显示界面
int zb_example_ap_my_download_display_free(void) {
    ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_MYD_scrNo, zb_getOpenedFileNum());
    if (g_mainWin != NULL) {
        lv_obj_del(g_mainWin);  g_mainWin = NULL;
    }
    return 0;
}

//我的下载指定文件处理
static int My_download_specified_file_processing(const char* filename) {
    ESP_LOGI(TAG, "已进入：文件类型判断 File_type_judgment\n");
    ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_MYD_scrNo, zb_getOpenedFileNum());
    //从右面开始的第一次出现的位置 如果成功，返回该字符以及其后面的字符
    const char* extension = strrchr(filename, '.');
    if (extension == NULL) {
        ESP_LOGE(TAG, "没有找到文件的文件扩展名: %s\n", filename);
        return -1;
    }

    if (strcmp(extension, ".txt") == 0) {
        ESP_LOGI(TAG, "是一个.txt文件\n");
        Displays_contents_specified_file(filename);//显示指定文件内容
    }
    else if (strcmp(extension, ".bin") == 0) {
        ESP_LOGI(TAG, "是一个.bin文件\n");
        Displays_contents_specified_file(filename);//显示指定文件内容
    }
    else if (strcmp(extension, ".dat") == 0) {
        ESP_LOGI(TAG, "是一个.dat文件\n");
        Displays_contents_specified_file(filename);//显示指定文件内容
    }
    else if (strcmp(extension, ".mnu") == 0) {
        ESP_LOGI(TAG, "是一个.mnu文件\n");
        Displays_contents_specified_file(filename);//显示指定文件内容
    }
    else if (strcmp(extension, ".gitignore") == 0) {
        ESP_LOGI(TAG, "是一个.gitignore\n");
        Displays_contents_specified_file(filename);//显示指定文件内容
    }
    else if (strcmp(extension, ".jpg") == 0) {
        ESP_LOGI(TAG, "是一个.jpg文件\n");
        Displays_specified_image_file(filename);//显示指定图片文件
    }
    else if (strcmp(extension, ".png") == 0) {
        ESP_LOGI(TAG, "是一个.png文件\n");
        Displays_specified_image_file(filename);//显示指定图片文件
    }
    else if (strcmp(extension, ".mp3") == 0) {
        ESP_LOGI(TAG, "是一个.mp3文件\n");
        Plays_specified_music_file(filename);//播放指定音乐文件

    }
    else if (strcmp(extension, ".wav") == 0) {
        ESP_LOGI(TAG, "是一个.wav文件\n");
        Plays_specified_music_file(filename);//播放指定音乐文件

    }
    else {
        ESP_LOGE(TAG, "不支持的文件类型: %s\n", extension);
        Displays_contents_specified_file(filename);//显示指定文件内容
    }

    ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_MYD_scrNo, zb_getOpenedFileNum());
    return 0;
}

//播放指定音乐文件
static int Plays_specified_music_file(const char* file_display) {
    ESP_LOGI(TAG, "已进入：播放指定音乐文件 Plays_specified_music_file\n");

    //头标签——》中：文件名
    lv_obj_t* heading_label = lv_label_create(top_window);
    if (NULL != heading_label)
    {
        lv_obj_align(heading_label, LV_ALIGN_TOP_MID, 0, 20);
        lv_label_set_text(heading_label, file_display);
        lv_obj_set_style_text_color(heading_label, lv_color_hex(0xFFFFFF), 0);//白色
    }

    //内容窗口
    lv_obj_t* text_label = lv_label_create(mainWin);
    //lv_obj_set_size(text_label, 80, 31);                 // 设置控件大小
    lv_obj_align(text_label, LV_ALIGN_TOP_LEFT, 20, 20);
    lv_obj_set_style_text_color(text_label, lv_color_hex(0x000000), 0);//黑色
    lv_label_set_long_mode(text_label, LV_LABEL_LONG_SCROLL_CIRCULAR);//滚动
    if (NULL != text_label) {
        lv_label_set_text(text_label, "我的下载中音乐播放中……");
    }

    //播放
    ESP_LOGI(TAG, "播放前——AP=%d，当前打开的文件数量:%d", g_MYD_scrNo, zb_getOpenedFileNum());
#ifndef C_PC_SIM
    music_player_set_play_uri(file_display, 0);
#endif
    ESP_LOGI(TAG, "播放后——AP=%d，当前打开的文件数量:%d", g_MYD_scrNo, zb_getOpenedFileNum());

    return 0;
}

//显示指定图片文件
static int Displays_specified_image_file(const char* file_display) {
    ESP_LOGI(TAG, "已进入：显示指定图片文件 Displays_specified_image_file\n");

    //头标签——》中：文件名
    lv_obj_t* heading_label = lv_label_create(top_window);
    if (NULL != heading_label)
    {
        lv_obj_align(heading_label, LV_ALIGN_TOP_MID, 0, 20);
        lv_label_set_text(heading_label, file_display);
        lv_obj_set_style_text_color(heading_label, lv_color_hex(0xFFFFFF), 0);//白色
        lv_label_set_long_mode(heading_label, LV_LABEL_LONG_SCROLL_CIRCULAR);//滚动
    }

    lv_obj_t* assigned_picture = lv_img_create(mainWin);
    lv_img_set_src(assigned_picture, file_display);
    lv_obj_align(assigned_picture, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_long_mode(assigned_picture, LV_LABEL_LONG_SCROLL_CIRCULAR);//滚动
    //lv_obj_set_size(assigned_picture, 230, 270);

    return 0;
}

//显示指定文件的内容
static int Displays_contents_specified_file(const char* file_display) {
    ESP_LOGI(TAG, "已进入：显示指定文件的内容 Displays_contents_specified_file\n");
    ESP_LOGI(TAG, "要显示内容的文件是file_display：%s\n", file_display);

    //打开文件
    FILE* pfile = zb_fopen(file_display, "rb");
    if (pfile == NULL) {
        ESP_LOGE(TAG, "打开文件失败!\n");
        return -1;
    }

    //读取文件长度,读完后把文件指针复位到开头
    fseek(pfile, 0, SEEK_END);
    int file_length = ftell(pfile);
    rewind(pfile);
    ESP_LOGI(TAG, "该文件长度%d\n", file_length);

    //申请一段内存空间用来放文件中的文本内容，并置零
    int text_length = file_length * sizeof(char) + 1;
    char* text_buffer = (char*)heap_caps_malloc_dflt(text_length);
    memset(text_buffer, 0, text_length);

    //把文件文本内容读到申请的内存空间中
    int read_length = fread(text_buffer, 1, file_length, pfile);
    if (read_length != file_length) {
        ESP_LOGE(TAG, "读取文件失败!\n");
        return -2;
    }

    //打印文件文本内容，释放申请的内存空间，关闭文件流
    //printf(text_buffer);
    ESP_LOGI(TAG, "文件%s的文本内容为：\n%s\n", file_display, text_buffer);

    //头标签——》中：文件名
    lv_obj_t* heading_label = lv_label_create(top_window);
    if (NULL != heading_label)
    {
        //lv_obj_set_size(heading_label, 240, 31);                 // 设置控件大小
        lv_obj_align(heading_label, LV_ALIGN_TOP_MID, 0, 20);
        lv_label_set_text(heading_label, file_display);
        lv_obj_set_style_text_color(heading_label, lv_color_hex(0xFFFFFF), 0);//白色
    }

    //内容窗口
    lv_obj_t* text_label = lv_label_create(mainWin);
    //lv_obj_set_size(text_label, 80, 31);                 // 设置控件大小
    lv_obj_align(text_label, LV_ALIGN_TOP_LEFT, 20, 20);
    lv_obj_set_style_text_color(text_label, lv_color_hex(0x000000), 0);//黑色
    lv_label_set_long_mode(text_label, LV_LABEL_LONG_SCROLL_CIRCULAR);//滚动模式
    lv_obj_set_style_anim_speed(text_label, 500, 0);//滚动速度
    //lv_anim_set_values(lv_anim_t * a, int32_t start, int32_t end);
    // 默认的滚动方式是向左滚动.第二个参数是起始位置，第三个是结束位置，所以如果改变方向只要左右反一下就行了
    //lv_anim_set_values(&a, 0,-size.x - lv_font_get_glyph_width(font, ' ', ' ') * LV_LABEL_WAIT_CHAR_COUNT);//左滚动
    //lv_anim_set_values(&a,-size.x - lv_font_get_glyph_width(font, ' ', ' ') * LV_LABEL_WAIT_CHAR_COUNT,0);//右滚动
    if (NULL != text_label){
        lv_label_set_text(text_label, text_buffer);
    }
    if (file_length <= 0) {
        ESP_LOGE(TAG, "文件%s长度为%d，无内容\n", file_display, file_length);
        lv_label_set_text(text_label, "此文件无内容");
    }

    //text_buffer = heap_caps_realloc_dflt(text_buffer, text_length);
    heap_caps_free(text_buffer);
    fclose(pfile);


    return 0;
}


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
        ESP_LOGI(TAG, "是一个按键事件,keyVal=%d", keyVal);
        ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_MYD_scrNo, zb_getOpenedFileNum());
        if (!keyVal) {
            keyVal = lv_event_get_key(evt); //重新从事件对象中获取按键值
        }

        if (LV_KEY_ESC == keyVal) {
            LV_LOG_ERROR("Esc键，keyVal = %d\n", keyVal);
            ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_MYD_scrNo, zb_getOpenedFileNum());
            lv_event_stop_processing(evt);//停止当前事件的处理，不再执行后续的事件回调函数
            lv_indev_keypad_wait_release();//等待键盘输入设备的按键释放
#ifndef C_PC_SIM
            int aud = audio_stream_stop_play();//停止播放
            ESP_LOGI(TAG, "停止播放audio_stream_stop_play()返回值为：%d", aud);
#endif
            MYD_switchScr(C_ZB_AP_MYD_SON_A, 0);
        }
    }
}








