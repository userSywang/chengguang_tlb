//————————————————————————————————————————————————
// 音乐播放界面 
//————————————————————————————————————————————————


//
//播放音乐文件.mp3、wav

#include <lvgl/lvgl.h>
#include <lvgl/examples/lv_examples.h>
#include <lvgl/src/misc/lv_types.h>


#include "../include/audio_media_def.h"
#include "../lvgl_main.h"
#include "web_disk_playback_display.h"

//#include "../upd_data_parse/favorites.h"
#include "../data_parse/img_res.h"
#include "../utils/misc_utils.h"
#include "../data_parse/aud_lyric.h"

#ifndef C_PC_SIM  //播放音乐文件
#include "../../trd/tz_audio_player/include/tz_audio_player.h"
#endif

#ifndef C_PC_SIM  //SD卡读取
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#endif


int music_player_get_play_time(int32_t * cur_pos_ms, int32_t * total_time_ms);

//------------------------------------------------------


#define FONT_CHRONOGRAPH_VALUE (&lv_font_montserrat_14)  //字体 播放器上的时间

#define FAST_ADVANCE_TIME_DIFF_MIN 3000   //快进快退的最小时间差 

#define PLYBACK_FILES_MAX 100

#define PLY_DISP_BG_TYPES_N  11

#define PLY_MODE_NO_DFLT  1

extern int player_toggle_layout_flag_bit;//播放器切换版面标志位

extern char* get_last_filename(const char* path);//根据路径提取最后一项的文件名

//------------------------------------------------------

// 播放功能的设置
typedef struct
{
    bool play_enable;         // 播放enable
    bool next_unit_enable;    // 下一首enable
    bool pre_unit_enable;     // 上一首enable
    bool play_continu_enable; // 继续播放enable
    bool record_enable;       // 录音使能
    bool play_exit;           // 播放退出
    bool find_enable;         // 查词使能
}web_disk_player_func_t;

// 播放器播放参数设置
typedef struct
{
    uint8_t speed;        // 播放速度
    //uint8_t loop_times;   // 循环次数
    bool single_cycle;    // 单曲循环
    bool list_order;      // 列表顺序
} web_disk_player_para_set_t;

typedef struct 
{
	char * m_absolutePathTbl[PLYBACK_FILES_MAX];  //预播放的绝对路径, 每项的 buff 长度为 PATH_BYTE_SIZE 
} plyDispScr_inf_t;


#define SONG_HAS_LRC()    (g_pAudLrc->hFile != NULL)

//------------------------------------------------------

#define TAG "web_disk_playback_display.c"

#define BD_PLY_REFESH_TMR_PERIOD   250

//------------------
int8_t  g_WEB_DISK_scrNo; // non-std, for debug 
int player_toggle_layout_flag_bit = 0;//播放器切换版面标志位

static lv_obj_t* img;
static lv_obj_t* mainWin;
static lv_obj_t* g_mainWin;
static lv_obj_t* top_window;

static lv_obj_t* pause_start_btn;//播放、暂停 按钮
static lv_obj_t* pause_start_label;//播放、暂停 文本
static lv_obj_t* g_lblPlayMode;
static lv_obj_t* current_time_label = NULL;//播放当前时间
static lv_obj_t* total_time_label = NULL;//播放当前总时间
static lv_obj_t* heading_label;//歌曲名标签
static lv_obj_t* ui_Slider1;//滑动条
static lv_timer_t* refresh_timer = NULL;//刷新时间
web_disk_player_func_t web_disk_player_func = {0}; //播放功能设置



//播放器播放参数设置
static web_disk_player_para_set_t web_disk_player_para_set = {
#ifndef C_PC_SIM
    .speed = MUSIC_PLAY_SPEED_1_00,  // 播放速度
#endif
    .single_cycle = false, // 单曲循环
    .list_order = false,   // 列表顺序
};


static plyDispScr_inf_t * g_plyDispInf = NULL; // 播放界面的全局信息
char* Baidu_displays_absolute_path_file_played = NULL;//要显示播放的文件绝对路径, bufLen 为 PATH_BYTE_SIZE 
char* file_name = NULL; // 要显示播放的文件名
static int Number_Songs_parent_directory = 0;//上级目录中的歌曲数量
static int Current_value_playback = 0; //当前正在播放的值, <0 表示未初始化
static int8_t g_playOrderMode = -1;// 播放顺序模式
static int8_t g_plyDispBgNo = 0; // 背景选择号, 自0起

static T_AUD_LYRIC * g_pAudLrc;
static int16_t  g_curSentenceNo;
static lv_obj_t * g_imgNoSrc;
static lv_obj_t * g_lrcLabels[2];

static uint32_t g_ttlTm = 0;//音乐总时间
static uint32_t g_posTm = 0;//当前播放时间
static uint32_t g_ttlTmSecDisp = 0;
static uint32_t g_posTmSecDisp = 0;

//--------------------------------------
static void updateLrcCurSentence(int32_t curTmMs);
static void _myMainWinCb(lv_event_t* evt);//键盘处理回调函数
static void _myMainWinDelCb(lv_event_t* evt);//删除对象处理函数
//static int Displays_contents_specified_file(const char* file_display);//显示指定文件的内容

static int web_disk_Player_background_selection(int backValue);//播放器背景选择
static int web_disk_player_components(lv_obj_t* parent_form);//百度网盘播放器部件布局
static int _my_btn_pause_start_cb(lv_event_t* evt);//播放、暂停按钮回调函数
static int refresh_text_timer_cb(struct _lv_timer_t * tmr);//刷新文本定时器
static void time_label_refresh(uint32_t ttlTm, uint32_t posTm);//时间标签刷新
static void play_or_pause_func(void);//播放或暂停功能
static void play_normal_config(void);//播放的功能配置
static int Previous_play_function(void);//上一首播放
static int Next_play_function(void);//下一首播放
static int Read_absolute_path_SD_all_files(const char* path);//读取保存SD卡的上级目录下的所有文件绝对路径
static int read_files_in_parent_directory(const char* path);//读取父目录中的文件
static char* join_path(const char* dir, const char* file);//辅助函数，将目录名和文件名合并为一个完整的路径
static int switch_songs_info_proc(int Current_value_playback);//切换歌曲信息
static int fast_forward_function(void);     //快进功能
static int fast_rewind_function(void);      //快退功能
static int sequential_play_functio(void);   //顺序播放功能
static int single_cycle_function(void);     //单曲循环播放功能


//----------------------------------------------
/**
* 根据路径提取最后一项的文件名
* 参数：
*	path：路径
* 返回值：
*	最后一项的文件名
* 例如：
*	传入路径："/A/B/C/D/E"
*	返回值：E
*/
char* get_last_filename(const char* path) {
	//LV_LOG_USER("已进入：根据路径提取最后一项的文件名\n");
	static char last_filename[256]; // 假设文件名不超过255个字符
	const char* last_slash = strrchr(path, '/');
	if (last_slash == NULL) {
		// 如果没有找到'/'，则整个路径就是文件名
		strncpy(last_filename, path, sizeof(last_filename) - 1);
		last_filename[sizeof(last_filename) - 1] = '\0'; // 确保字符串以null结尾
	}
	else {
		// 从最后一个'/'开始复制剩余的部分
		strncpy(last_filename, last_slash + 1, sizeof(last_filename) - 1);
		last_filename[sizeof(last_filename) - 1] = '\0'; // 确保字符串以null结尾
	}
	return last_filename;
}

static void updatePlayModeBySelNo(void)
{
	const char  * aPlayModeTxtTbl[] = { "单曲模式", "多曲循环", "单曲循环" };

	ESP_LOGI(TAG, "playOrderMod= %hd", (short)g_playOrderMode);
	if (g_playOrderMode < 0 || g_playOrderMode >= 3) {
		g_playOrderMode = PLY_MODE_NO_DFLT;
	}
	switch (g_playOrderMode) {
		case 0: {
			ESP_LOGI(TAG, "单曲");
			web_disk_player_para_set.single_cycle = false;// 单曲循环
			web_disk_player_para_set.list_order = false; // 列表顺序
			break;
		}
		case 1: {
			ESP_LOGI(TAG, "列表顺序");
			web_disk_player_para_set.single_cycle = false;// 单曲循环
			web_disk_player_para_set.list_order = true; // 列表顺序
			break;
		}
		default: {
			ESP_LOGI(TAG, "单曲循环");
			web_disk_player_para_set.single_cycle = true;// 单曲循环
			web_disk_player_para_set.list_order = false; // 列表顺序
			break;
		}
	}
	lv_label_set_text_static(g_lblPlayMode, aPlayModeTxtTbl[g_playOrderMode]);
	lv_obj_set_size(g_lblPlayMode, LV_SIZE_CONTENT, 20);
	lv_obj_align(g_lblPlayMode, LV_ALIGN_BOTTOM_LEFT, 8, -22);
	
}

static void  showOrHideLrcObjs(void)
{
	lv_obj_t * objLrc = lv_obj_get_parent(g_lrcLabels[0]);
	if (SONG_HAS_LRC()) {
		lv_obj_clear_flag(objLrc, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(g_imgNoSrc, LV_OBJ_FLAG_HIDDEN);
		lv_label_set_text(g_lrcLabels[0], " ");
		lv_label_set_text(g_lrcLabels[1], " ");
	}
	else {
		lv_obj_clear_flag(g_imgNoSrc, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(objLrc, LV_OBJ_FLAG_HIDDEN);
	}
	return;
}

static int getCurSongIdxInList(void) {
#if 1
	int songIdx = -1;
	if (!g_plyDispInf || !g_plyDispInf->m_absolutePathTbl[0]) {
		return 0;
	}
	
	for (int i = 0; i < Number_Songs_parent_directory; i++) {
		if (strcmp(Baidu_displays_absolute_path_file_played, g_plyDispInf->m_absolutePathTbl[i]) == 0) {
			songIdx = i;
			//ESP_LOGI(TAG, "%d：正在播放的是：%s,是第%d个", __LINE__, g_plyDispInf->m_absolutePathTbl[i], Current_value_playback);
			break;
		}
	}
	if (songIdx < 0) {
		songIdx = 0;
	}
	return songIdx;
#endif
}


// 音频播放版面 
int zb_ap_web_disk_playback_display_init(void)
{
    LV_LOG_USER("已进入：百度网盘显示播放zb_ap_web_disk_playback_display_init\n");
    ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_WEB_DISK_scrNo, zb_getOpenedFileNum());


    g_plyDispInf = (plyDispScr_inf_t *) heap_caps_malloc_dflt(sizeof(plyDispScr_inf_t));
    memset(g_plyDispInf, 0, sizeof(plyDispScr_inf_t));
    g_pAudLrc = (T_AUD_LYRIC *) heap_caps_malloc_dflt(sizeof(T_AUD_LYRIC));
    memset(g_pAudLrc, 0, sizeof(T_AUD_LYRIC));

    //背景
    img = lv_obj_create(lv_scr_act());
    lv_obj_set_size(img, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_scrollbar_mode(img, LV_SCROLLBAR_MODE_OFF);//不显示滚动条
    lv_obj_set_style_border_width(img, 0, 0);//边框宽度为0

    if (!((g_plyDispBgNo >= 0) && (g_plyDispBgNo < PLY_DISP_BG_TYPES_N))) {
        g_plyDispBgNo = 0;
    }
    web_disk_Player_background_selection(g_plyDispBgNo);//播放器背景选择
    web_disk_player_components(img);//百度网盘播放器部件
    play_normal_config();//播放的功能配置
    lv_group_add_obj(lv_group_get_default(), img);//这行代码将winA对象添加到默认的对象组中。在LVGL中，对象组用于处理键盘输入和导航。
    
    lv_group_focus_obj(img);//对焦一个对象(散焦电流) ——》切换版面不销毁

    sys_setCurMainWinAndCb(img, _myMainWinCb);//自定义函数将mainWin作为当前主窗口。
    lv_obj_add_event_cb(img, _myMainWinDelCb, LV_EVENT_DELETE, NULL);//对象正在被删除

    Current_value_playback = -1;
    g_mainWin = img;

    return 0;
}

//删除播放界面 
int zb_ap_web_disk_playback_display_free(void) {
    ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_WEB_DISK_scrNo, zb_getOpenedFileNum());
    if (g_mainWin) {
        lv_obj_del(g_mainWin);  g_mainWin = NULL;
    }
    return 0;
}

int _disk_playback_get_cur_sel(void) {
	if (Current_value_playback < 0) {
		Current_value_playback = getCurSongIdxInList();
	}
	return Current_value_playback;
}


#if 0
//显示指定文件的内容
static int Displays_contents_specified_file(const char* file_display) {
    ESP_LOGI(TAG, "已进入：显示指定文件的内容 Displays_contents_specified_file\n");
    ESP_LOGI(TAG, "要显示内容的文件是file_display：%s\n", file_display);

    //打开文件
    int file_length = zb_getFileLength(file_display);
    ESP_LOGI(TAG, "该文件长度%d\n", file_length);
    FILE* pfile = zb_fopen(file_display, "rb");
    if (pfile == NULL) {
        ESP_LOGE(TAG, "打开文件失败!\n");
        return -1;
    }
    fseek(pfile, 0, SEEK_SET);

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

    file_name = (char*)heap_caps_malloc_dflt(PATH_BYTE_SIZE);
    strncpy(file_name, get_last_filename(file_display), PATH_BYTE_SIZE - 1);
    file_name[PATH_BYTE_SIZE - 1] = '\0'; // 确保字符串以null结尾
    ESP_LOGI(TAG, "文件名： %s", file_name);

    //头标签——》中：文件名
    lv_obj_t* heading_label = lv_label_create(mainWin);
    if (NULL != heading_label)
    {
        //lv_obj_set_size(heading_label, 240, 31);                 // 设置控件大小
        lv_obj_align(heading_label, LV_ALIGN_TOP_MID, 15, 15);
        lv_label_set_text(heading_label, file_name);
        lv_obj_set_style_text_color(heading_label, lv_color_hex(0x000000), 0);//白色
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
    if (NULL != text_label) {
        lv_label_set_text(text_label, text_buffer);
    }
    if (file_length <= 0) {
        //ESP_LOGE(TAG, "文件%s长度为%d，无内容\n", file_display, file_length);
        lv_label_set_text(text_label, "此文件无内容");
    }

    heap_caps_free(text_buffer);  text_buffer = NULL;
    fclose(pfile);  pfile = NULL;
    return 0;
}
#endif

static void _slide_val_chg_cb(lv_event_t * evt)
{
	if (evt->code != LV_EVENT_VALUE_CHANGED)
		return;

	int32_t posPct = lv_slider_get_value(ui_Slider1);
	int newPosTm = ((int64_t)posPct * g_ttlTm) / 100;
	if (newPosTm < g_ttlTm) {
		ESP_LOGE(TAG, "播放 seekTo %d", newPosTm);
	#ifndef C_PC_SIM
		tz_play_set_start_end_time(newPosTm, g_ttlTm);//播放偏移
	#endif
	}
}


//百度网盘播放器部件布局
static int web_disk_player_components(lv_obj_t * parent_form) {
    LV_LOG_USER("已进入：百度网盘播放器部件布局Baidu_web_disk_player_components\n");

    //歌曲名
    heading_label = lv_label_create(parent_form);
    lv_obj_set_size(heading_label, SCREEN_WIDTH - 40, 50);
    lv_obj_align(heading_label, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_set_style_text_align(heading_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(heading_label, lv_color_hex(0xffffff), 0);//白色
    lv_label_set_long_mode(heading_label, LV_LABEL_LONG_SCROLL_CIRCULAR);//滚动模式
    lv_obj_set_style_anim_speed(heading_label, 100, 0);//滚动速度
    
    Read_absolute_path_SD_all_files(Baidu_displays_absolute_path_file_played);//读取保存SD卡的上级目录下的所有文件绝对路径

    file_name = (char*)heap_caps_malloc_dflt(PATH_BYTE_SIZE);
    strncpy(file_name, get_last_filename(Baidu_displays_absolute_path_file_played), PATH_BYTE_SIZE - 1);
    file_name[PATH_BYTE_SIZE - 1] = '\0'; // 确保字符串以null结尾
    ESP_LOGI(TAG, "文件名： %s", file_name);
    lv_label_set_text(heading_label, file_name);

    //歌词
    // 暂无内容
    lv_obj_t* noConLay = lv_obj_create(img);
    lv_img_dsc_t* noCon = zb_img_res_load_one(NULL, BAIDUZIYUAN_IMG_XIAOLIAN, IMG_FREE_TYPE_AUTO, NULL);
    lv_obj_set_style_bg_img_src(noConLay, noCon, 0);
    lv_obj_align_to(noConLay, parent_form, LV_ALIGN_TOP_MID, 0, 87);
    g_imgNoSrc = noConLay;

    lv_obj_t * lrcBg = lv_obj_create(img);
    lv_obj_set_size(lrcBg, SCREEN_WIDTH - 4 * 2, 120); 
    lv_obj_set_style_border_width(lrcBg, 0, 0);
    lv_obj_align_to(lrcBg, parent_form, LV_ALIGN_TOP_LEFT, 3, 80);
    lv_obj_set_flex_flow(lrcBg, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(lrcBg, LV_DIR_NONE);
    g_lrcLabels[0] = lv_label_create(lrcBg);
    g_lrcLabels[1] = lv_label_create(lrcBg);
    lv_color_t txtColor = lv_color_white();
    lv_obj_set_style_text_color(g_lrcLabels[0], txtColor, 0);
    lv_color_t nextTxtColor = lv_color_hex(0xa0a0a0);
    lv_obj_set_style_text_color(g_lrcLabels[1], nextTxtColor, 0);
    lv_obj_set_style_text_align(g_lrcLabels[0], LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_align(g_lrcLabels[1], LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(lrcBg, 0, LV_OPA_0);
    lv_obj_set_style_bg_opa(g_lrcLabels[0], 0, LV_OPA_0);
    lv_obj_set_style_bg_opa(g_lrcLabels[1], 1, LV_OPA_0);

    //滑动条
    ui_Slider1 = lv_slider_create(parent_form);
    lv_slider_set_range(ui_Slider1, 0, 100);//设置滑动条的最小值和最大值
    lv_slider_set_value(ui_Slider1, 0, LV_ANIM_OFF);//在滑动条上设置一个新值 不使用动画效果立即更新滑块的显示位置
    if (lv_slider_get_mode(ui_Slider1) == LV_SLIDER_MODE_NORMAL) //获取滑块模式 普通模式
        lv_slider_set_left_value(ui_Slider1, 50, LV_ANIM_OFF);//为滑块的左旋钮设置一个新值 不使用动画效果
    lv_obj_set_width(ui_Slider1, SCREEN_WIDTH - 60);
    lv_obj_set_height(ui_Slider1, 10);
    lv_obj_align_to(ui_Slider1, parent_form, LV_ALIGN_TOP_MID, 0, 240);
    //设置滑块的样式
    // 默认部分 LV_PART_MAIN
    lv_obj_set_style_bg_color(ui_Slider1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);//设置滑块主区域的背景颜色为 LV_PART_MAIN滑动条的背景  LV_STATE_DEFAULT默认状态
    lv_obj_set_style_bg_opa(ui_Slider1, 100, LV_PART_MAIN | LV_STATE_DEFAULT);//设置滑块主区域的背景颜色不透明度为255（即完全不透明）
    // 指示器部分 LV_PART_INDICATOR
    lv_obj_set_style_bg_color(ui_Slider1, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT); //设置滑块指示器区域的背景颜色为橙色  LV_PART_INDICATOR滑块的当前值 LV_STATE_DEFAULT默认状态
    lv_obj_set_style_bg_opa(ui_Slider1, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);//设置滑块指示器区域的背景颜色不透明度为255
    lv_obj_set_style_outline_color(ui_Slider1, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);//设置滑块指示器区域的轮廓颜色为橙色  0xFF843D
    lv_obj_set_style_outline_opa(ui_Slider1, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);//将滑块控件指示器部分的轮廓不透明度设置为255（完全不透明）
    // 拖动部分 LV_PART_KNOB
    lv_obj_set_style_bg_color(ui_Slider1, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);//设置滑块指示器区域的背景颜色为   LV_PART_KNOB滑块中可以被用户拖动的部分 LV_STATE_DEFAULT默认状态
    lv_obj_set_style_bg_opa(ui_Slider1, 255, LV_PART_KNOB | LV_STATE_DEFAULT);//设置滑块指示器区域的背景颜色不透明度为255
    lv_obj_add_event_cb(ui_Slider1, _slide_val_chg_cb, LV_EVENT_VALUE_CHANGED, NULL); // todo

    // 播放当前时间
    current_time_label = lv_label_create(parent_form);
    lv_label_set_text(current_time_label, "00:00");
    lv_obj_set_style_text_font(current_time_label, FONT_CHRONOGRAPH_VALUE, 0);
    lv_obj_set_style_text_color(current_time_label, lv_color_hex(0xffffff), 0);
    lv_obj_align_to(current_time_label, ui_Slider1, LV_ALIGN_OUT_LEFT_MID, 20, 15);//LV_ALIGN_LEFT_MID内部左边缘 LV_ALIGN_OUT_LEFT_MID左边缘

    // 播放当总时间
    total_time_label = lv_label_create(parent_form);
    lv_label_set_text_fmt(total_time_label, "%s", "00:00");
    lv_obj_set_style_text_font(total_time_label, FONT_CHRONOGRAPH_VALUE, 0);
    lv_obj_set_style_text_color(total_time_label, lv_color_hex(0xffffff), 0);
    lv_obj_align_to(total_time_label, ui_Slider1, LV_ALIGN_OUT_RIGHT_MID, -20, 15);

    //播放、暂停按钮
    pause_start_btn = lv_obj_create(parent_form);//创建按钮
    lv_obj_add_flag(pause_start_btn, LV_OBJ_FLAG_CLICKABLE);//明确对象可点击
    lv_obj_set_scrollbar_mode(pause_start_btn, LV_SCROLLBAR_MODE_OFF);//不显示滚动条
    lv_obj_align(pause_start_btn, LV_ALIGN_TOP_MID, 0, 270);
    lv_obj_set_size(pause_start_btn, 20, 20);
    lv_obj_set_style_bg_opa(pause_start_btn, LV_OPA_0, LV_PART_MAIN);//按钮透明
    lv_obj_set_style_outline_opa(pause_start_btn, LV_OPA_0, 0); //轮廓透明度为完全透明
    lv_obj_set_style_outline_width(pause_start_btn, 0, 0);//轮廓宽度为0
    lv_obj_set_style_border_opa(pause_start_btn, LV_OPA_0, 0); //边框透明度为完全透明
    lv_obj_set_style_border_width(pause_start_btn, 0, 0);//边框宽度为0
    lv_obj_add_event_cb(pause_start_btn, _my_btn_pause_start_cb, LV_EVENT_SHORT_CLICKED, NULL);//按键控制，按钮被短按时被调用
    lv_img_dsc_t* xing1 = zb_img_res_load_one(NULL, COMMON_IMG_ZANTING, IMG_FREE_TYPE_AUTO, NULL);
    lv_img_dsc_t* xing2 = zb_img_res_load_one(NULL, COMMON_IMG_BOFANG, IMG_FREE_TYPE_AUTO, NULL);
    //lv_obj_set_style_bg_img_src(pause_start_btn, xing1, LV_STATE_CHECKED);//暂停    选中状态
    lv_obj_set_style_bg_img_src(pause_start_btn, xing1, LV_STATE_CHECKED);//播放    按下状态
    lv_obj_set_style_bg_img_src(pause_start_btn, xing2, 0);//平时，播放 默认状态
    pause_start_label = lv_label_create(parent_form);
    lv_label_set_text(pause_start_label, "播放");
    //lv_obj_set_style_text_font(pause_start_label, FONT_CHRONOGRAPH_VALUE, 0);
    lv_obj_set_style_text_color(pause_start_label, lv_color_hex(0xffffff), 0);
    lv_obj_align_to(pause_start_label, pause_start_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    lv_obj_t * lblPlayMode = lv_label_create(parent_form);
    lv_label_set_text_static(lblPlayMode, ".");
    lv_obj_align(lblPlayMode, LV_ALIGN_BOTTOM_LEFT, 8, -22);
    lv_obj_set_style_text_color(lblPlayMode, lv_color_hex(0xffffff), 0);
    g_lblPlayMode = lblPlayMode;

    lv_obj_t * lblPlayPrompt = lv_label_create(parent_form);
    lv_label_set_text_static(lblPlayPrompt, "F1键切换");
    lv_obj_align(lblPlayPrompt, LV_ALIGN_BOTTOM_RIGHT, -8, -22);
    lv_obj_set_style_text_color(lblPlayPrompt, lv_color_hex(0xb0b0b0), 0);

    refresh_timer = lv_timer_create(refresh_text_timer_cb, BD_PLY_REFESH_TMR_PERIOD, NULL);
    lv_timer_set_repeat_count(refresh_timer, -1);//设置定时器执行次数

    //ESP_LOGI(TAG, "bfr prs lrc");
    int ret = Aud_LyrOpen(g_pAudLrc, Baidu_displays_absolute_path_file_played, 1);
    ESP_LOGI(TAG, "lrc open ret %d, sentN:%hd, hdl %p", ret, g_pAudLrc->lyrIdx.cntIdx, g_pAudLrc->hFile);
    showOrHideLrcObjs();
    updatePlayModeBySelNo();

    ESP_LOGI(TAG, "文件路径：%s", Baidu_displays_absolute_path_file_played);
#ifndef C_PC_SIM 
    tz_play_audio(Baidu_displays_absolute_path_file_played, 0, -1);
    ESP_LOGI(TAG, " plyAud ");
#endif

    return 0;
}

//播放、暂停按钮回调函数(切换按钮、timer 状态)
static int _my_btn_pause_start_cb(lv_event_t* evt) {
    LV_LOG_USER("已进入：播放、暂停按钮回调函数\n");

    play_or_pause_func();//播放或暂停功能
    if (lv_obj_has_state(pause_start_btn, LV_STATE_CHECKED) == 0) {
        lv_obj_add_state(pause_start_btn, LV_STATE_CHECKED);
        lv_label_set_text(pause_start_label, "暂停");
        lv_timer_pause(refresh_timer);//暂停计数器
        ESP_LOGW(TAG, "暂停");
    }
    else if (lv_obj_has_state(pause_start_btn, LV_STATE_CHECKED) == 1) {
        lv_obj_clear_state(pause_start_btn, LV_STATE_CHECKED);
        lv_label_set_text(pause_start_label, "播放");
        lv_timer_resume(refresh_timer);//恢复计数器
        ESP_LOGW(TAG, "播放");
    }
    bool seldel = lv_obj_has_state(pause_start_btn, LV_STATE_CHECKED);//图标按钮状态 返回值：1选中 0非选中
    LV_LOG_ERROR("1选中暂停 0非选中播放 图标按钮状态：seldel = % d\n", seldel);
    return 0;
}


//刷新文本定时器
static int refresh_text_timer_cb(struct _lv_timer_t * tmr) {
    //LV_LOG_USER("已进入：刷新文本定时器\n");
    uint32_t ttlTime = 0, posTime = 0;
#ifndef C_PC_SIM
    TZ_PLAYSTATE_T audStat = get_play_audio_state();
    if (audStat == TZ_PLAYING) {
        music_player_get_play_time(&posTime, &ttlTime);//获取音乐总时间和当前播放时间
    }
#else
    ttlTime = 3172000;//音乐总时间  3172000
    posTime = posTime + 200;//当前播放时间
    if (ttlTime < posTime) { //音乐总时间 < 当前播放时间
        posTime = 0;
        ESP_LOGI(TAG, "当前播放时间posTm = %d", posTime);
    }
    ESP_LOGI(TAG, "当前播放时间posTm = %d", posTime);
#endif
    g_ttlTm = ttlTime;
    g_posTm = posTime;
    time_label_refresh(ttlTime, posTime);//时间标签刷新
	if (SONG_HAS_LRC()) {
		updateLrcCurSentence(posTime);
	}
    return 0;
}

/**
 * 时间标签刷新
 * 参数：
 *   ttlTm 总时间
 *   posTm 当前播放时间
 */
static void time_label_refresh(uint32_t ttlTm, uint32_t posTm){
    LV_LOG_USER("已进入：时间标签刷新\n");  
#ifndef C_PC_SIM
    TZ_PLAYSTATE_T audStat = get_play_audio_state();
    uint8_t apNo = zb_getApNo();
    if (C_ZB_AP_BD_NET_DISK == apNo || TZ_PLAYING == audStat) {
        zb_clear_autoPwrOffCnt();//清除休眠计时
        if (C_ZB_AP_BD_NET_DISK == apNo) {
            zb_clear_autoScreenOffCnt();
        }
    }
    if (TZ_PLAYSTOPPED == audStat || TZ_PLAYPAUSED == audStat) {
        ESP_LOGI(TAG, "~-audStat %d.", (int)audStat);
        return;
    }
#endif
    uint8_t second = 0;
    uint8_t minute = 0;
    ttlTm /= 1000;
    posTm /= 1000; // ms to s
    if (ttlTm != g_ttlTmSecDisp || posTm != g_posTmSecDisp) {
        g_ttlTmSecDisp = ttlTm;
        g_posTmSecDisp = posTm;
        second = posTm % 60;
        minute = posTm / 60;
        //ESP_LOGI(TAG, "%d——》%02d:%02d",__LINE__, posTm / 60 , posTm % 60);
        lv_label_set_text_fmt(current_time_label, "%02d:%02d", minute, second);//%02hu 无符号短整型
        ESP_LOGI(TAG, "%02d:%02d", minute, second);
        second = ttlTm % 60;
        minute = ttlTm / 60;
        lv_label_set_text_fmt(total_time_label, "%02d:%02d", minute, second);
        ESP_LOGI(TAG, "%02d:%02d", minute, second);
        if (ttlTm){
            lv_slider_set_value(ui_Slider1, posTm * 100 / ttlTm, LV_ANIM_OFF);//在滑块上设置一个新值
        }
    }
}


//.写一个刷新指定句(两个 label_obj) 的函数, updateLrcCurSentence();
static void updateLrcCurSentence(int32_t curTmMs)
{
	if (!SONG_HAS_LRC())
		return;
	int getLrcRet = Aud_LyrGetLyrIdx(g_pAudLrc, curTmMs);
	if (getLrcRet < 1 && getLrcRet > 3) {
		ESP_LOGW(TAG, "getLrc ret %d", getLrcRet);
		return;
	}
	int16_t sentenceNo = g_pAudLrc->lyrIdx.pcRead;
	if (sentenceNo == g_curSentenceNo) { // still in orevious sentence.
		return;
	}

	ESP_LOGI(TAG, "getLrc to sentNo %hd", sentenceNo);
	g_curSentenceNo = sentenceNo;
	char acLrcTxt[240];
	int16_t txtLen = sizeof(acLrcTxt) - 1;
	acLrcTxt[0] = '\0';
	int ret = Aud_LyrGetByNo(g_pAudLrc, sentenceNo, acLrcTxt, &txtLen);
	ESP_LOGI(TAG, "getLrcByNo ret %d", ret);
	lv_label_set_text(g_lrcLabels[0], acLrcTxt);
	lv_obj_set_size(g_lrcLabels[0], LV_PCT(100), LV_SIZE_CONTENT);
	if (sentenceNo + 1 < g_pAudLrc->lyrIdx.cntIdx) {
		txtLen = sizeof(acLrcTxt) - 1;
		acLrcTxt[0] = '\0';
		Aud_LyrGetByNo(g_pAudLrc, sentenceNo + 1, acLrcTxt, &txtLen);
		lv_label_set_text(g_lrcLabels[1], acLrcTxt);
		lv_obj_set_size(g_lrcLabels[1], LV_PCT(100), LV_SIZE_CONTENT);
	}
	else {
		lv_label_set_text(g_lrcLabels[1], " ");
		lv_obj_set_size(g_lrcLabels[1], LV_PCT(100), LV_SIZE_CONTENT);
	}
	return;
}

/**
 * 播放或暂停功能
 */
static void play_or_pause_func(void){
    LV_LOG_USER("已进入：播放或暂停功能 enable %d", (int)web_disk_player_func.play_enable);
#ifndef C_PC_SIM
    if (web_disk_player_func.play_enable){
        TZ_PLAYSTATE_T playStat = get_play_audio_state();
        ESP_LOGI(TAG, "音频 stat %d", (int)playStat);
        //播放
        if (TZ_PLAYING == playStat) {
            ESP_LOGI(TAG, "音频暂停\n");
            tz_play_pause_audio();//暂停音频播放器
            lv_timer_pause(refresh_timer);//暂停计数器
        }
        //暂停
        else if (TZ_PLAYPAUSED == playStat) {
            ESP_LOGI(TAG, "音频播放\n");
            tz_play_resume_audio();//恢复音频播放器
            lv_timer_resume(refresh_timer);//恢复计数器
        }
        else  { // sugy 20241126 for: 睡眠后唤醒时(按 resume 键)继续播放 
            ESP_LOGI(TAG, "playagin, cur_time_ps: %ld, stat:%d, tmr %p", g_posTm, playStat, refresh_timer);
            // 
            tz_play_audio(Baidu_displays_absolute_path_file_played, g_posTm, -1);
            if (TZ_PLAYSTOPPED == playStat) { //是休眠后,再次播放 
                // music_player_set_seek(g_posTm); 
            }
            lv_timer_resume(refresh_timer);//恢复计数器
            lv_timer_reset(refresh_timer);
        }
    }
#else

#endif
}

/**
* 切换歌曲信息
* 参数：
*   Current_value_playback：要切换到的值
*/
static int switch_songs_info_proc(int Current_value_playback) {
    LV_LOG_USER("已进入：切换歌曲信息\n");

    Aud_LyrClose(g_pAudLrc);

    //把当前播放的文件路径保存一下
#ifndef C_PC_SIM
    Baidu_displays_absolute_path_file_played[0] = '\0';
    strncat(Baidu_displays_absolute_path_file_played, g_plyDispInf->m_absolutePathTbl[Current_value_playback], PATH_BYTE_SIZE - 1);
    ESP_LOGI(TAG, "%d：将要播放的文件路径：%s,是第%d个", __LINE__, Baidu_displays_absolute_path_file_played, Current_value_playback);
#endif

    //歌曲名更换
#ifndef C_PC_SIM 
    strncpy(file_name, get_last_filename(Baidu_displays_absolute_path_file_played), PATH_BYTE_SIZE - 1);
    file_name[PATH_BYTE_SIZE - 1] = '\0'; // 确保字符串以null结尾
    ESP_LOGI(TAG, "文件名： %s", file_name);
    lv_label_set_text(heading_label, file_name);
#else
    lv_label_set_text(heading_label, "切换歌曲后 歌曲的名称 Abominable capitalist");
#endif

    //停止上次的播放
#ifndef C_PC_SIM
    tz_play_stop_audio();//停止播放
    ESP_LOGI(TAG, "tz_play_stop_audio");
#endif

    //播放本次的
#ifndef C_PC_SIM 
    tz_play_audio(Baidu_displays_absolute_path_file_played, 0, -1);
    int playRetValue = 0;
    ESP_LOGI(TAG, "%d：tz_play_audio 返回值 %d", __LINE__, playRetValue);
#endif

    //销毁上次定时器
    if (refresh_timer != NULL) {
        lv_timer_del(refresh_timer); refresh_timer = NULL;
    }

    g_ttlTmSecDisp = 0;
    Aud_LyrOpen(g_pAudLrc, Baidu_displays_absolute_path_file_played, 1);
    showOrHideLrcObjs();

    g_ttlTm = 0;//音乐总时间
    g_posTm = 0;//当前播放时间
    g_ttlTmSecDisp = 0;
    //创建本次的定时器
    refresh_timer = lv_timer_create(refresh_text_timer_cb, BD_PLY_REFESH_TMR_PERIOD, NULL);
    lv_timer_set_repeat_count(refresh_timer, -1);//设置定时器执行次数
    return 0;
}


//上一首播放
static int Previous_play_function(void) {
    LV_LOG_USER("已进入：上一首播放\n");
    //读取上一首的信息 绝对路径

    //根据当前正在播放歌曲的绝对路径 找出是 m_absolutePathTbl[i]中的第几个
    //  路径对比  字符串对比  获取正在播放的值
    int idx = getCurSongIdxInList();
    Current_value_playback = (idx + Number_Songs_parent_directory - 1) % Number_Songs_parent_directory;//0-5 1-0 5-4
    switch_songs_info_proc(Current_value_playback); //切换歌曲信息
    return 0;
}

//下一首播放
static int Next_play_function(void) {
    LV_LOG_USER("已进入：下一首播放\n");

    int idx = getCurSongIdxInList();
    Current_value_playback = (idx + 1) % Number_Songs_parent_directory;//0-1  5-0
    switch_songs_info_proc(Current_value_playback);//切换
    return 0;
}

//顺序播放功能
static int sequential_play_function(void) {
    LV_LOG_USER("已进入：顺序播放功能\n");

    //本首歌曲播放完成之后，顺序自动播放下一首
    Next_play_function();
    return 0;
}

//随机播放功能
#if 0
static int random_play_function(void) {
    LV_LOG_USER("已进入：随机播放功能\n");
#ifndef C_PC_SIM
    for (int i = 0; i < Number_Songs_parent_directory; i++) {
        if (strcmp(Baidu_displays_absolute_path_file_played, g_plyDispInf->m_absolutePathTbl[i]) == 0) {
            Current_value_playback = i;
            ESP_LOGI(TAG, "%d：正在播放的是：%s,是第%d个", __LINE__, g_plyDispInf->m_absolutePathTbl[i], Current_value_playback);
            break;
        }
    }
#endif 

    Current_value_playback = lv_rand(0, Number_Songs_parent_directory-1);
    switch_songs_info_proc(Current_value_playback);//切换

    return 0;
}
#endif

//单曲循环播放功能
static int single_cycle_function(void) {
    LV_LOG_USER("已进入：单曲循环播放功能\n");
#ifndef C_PC_SIM
    for (int i = 0; i < Number_Songs_parent_directory; i++) {
        if (strcmp(Baidu_displays_absolute_path_file_played, g_plyDispInf->m_absolutePathTbl[i]) == 0) {
            Current_value_playback = i;
            ESP_LOGI(TAG, "%d：正在播放的是：%s,是第%d个", __LINE__, g_plyDispInf->m_absolutePathTbl[i], Current_value_playback);
            break;
        }
    }
#endif 
    switch_songs_info_proc(Current_value_playback);//切换
    return 0;
}

/**
 * 播放器背景选择
 * 参数：
 *   backValue:背景值
 *      0：天蓝色图片背景  我的下载 百度网盘 默认图片背景
 *      1：古诗古文图片背景
 *      2：暗灰色图片背景
 *      ……
 */
static int web_disk_Player_background_selection(int backValue) {
    LV_LOG_USER("已进入：播放器背景选择\n");
    lv_img_dsc_t* beijing_tu;
    //img = lv_obj_create(lv_scr_act());
    //lv_obj_set_size(img, SCREEN_WIDTH, SCREEN_HEIGHT);
    //lv_obj_set_scrollbar_mode(img, LV_SCROLLBAR_MODE_OFF);//不显示滚动条
    //lv_obj_set_style_border_width(img, 0, 0);//边框宽度为0
    switch (backValue) {
        case 0: {
            //暗灰色图片背景
            /* beijing_tu = zb_img_res_load_one(NULL, PLAYER_IMG_BEIJINGNORMAL1, IMG_FREE_TYPE_AUTO, NULL);
            lv_obj_set_style_bg_img_src(img, beijing_tu, 0);//设置图形对象背景图片 */
            lv_color_t bgColor = lv_color_hex(0x404040);
            lv_obj_set_style_bg_color(img, bgColor, 0);
            break;
        }
        case 1: {
            //古诗古文图片背景
            beijing_tu = zb_img_res_load_one(NULL, PLAYER_IMG_BEIJINGPOEM2, IMG_FREE_TYPE_AUTO, NULL);
            lv_obj_set_style_bg_img_src(img, beijing_tu, 0);//设置图形对象背景图片
            break;
        }
        case 2: {
            //天蓝色图片背景
            beijing_tu = zb_img_res_load_one(NULL, MY_DOWNLOAD_IMG_BEIJING, IMG_FREE_TYPE_AUTO, NULL);
            lv_obj_set_style_bg_img_src(img, beijing_tu, 0);//设置图形对象背景图片
            break;
        }
        case 3: {
            //自定义颜色背景 水平渐变  蓝——》粉
            static lv_style_t style;
            lv_style_init(&style);
            lv_style_set_radius(&style, 5);//圆角半径
            lv_style_set_bg_opa(&style, LV_OPA_COVER);//背景透明度为完全覆盖，即不透明
            lv_style_set_bg_color(&style, lv_color_hex(0x04dae2));//背景颜色 浅蓝
            lv_style_set_bg_grad_color(&style, lv_color_hex(0xfb04ff));//设置样式的渐变背景颜色为0xfb04ff，这是一个粉红色。
            lv_style_set_bg_grad_dir(&style, LV_GRAD_DIR_HOR);//设置渐变的方向为水平
            lv_style_set_bg_main_stop(&style, 0);//设置渐变的起始点在0的位置，即渐变的开始
            lv_style_set_bg_grad_stop(&style, 255);//设置渐变的结束点在255的位置，即渐变的结束
            lv_obj_add_style(img, &style, LV_PART_MAIN);//背景矩形
            break;
        }
        case 4: {
            //自定义颜色背景 垂直渐变 蓝——》绿
            static lv_style_t style;
            lv_style_init(&style);
            lv_style_set_radius(&style, 5);//圆角半径
            lv_style_set_bg_opa(&style, LV_OPA_COVER);//背景透明度为完全覆盖，即不透明
            lv_style_set_bg_color(&style, lv_color_hex(0x04dae2));//背景颜色 浅蓝
            lv_style_set_bg_grad_color(&style, lv_color_hex(0x007acc));//设置样式的渐变背景颜色为0xfb04ff，这是一个粉红色。#577430  #007acc
            lv_style_set_bg_grad_dir(&style, LV_GRAD_DIR_VER);//设置渐变的方向为垂直
            lv_style_set_bg_main_stop(&style, 0);//设置渐变的起始点在0的位置，即渐变的开始
            lv_style_set_bg_grad_stop(&style, 255);//设置渐变的结束点在255的位置，即渐变的结束
            lv_obj_add_style(img, &style, LV_PART_MAIN);//背景矩形
            break;
        }
        case 5: {
            //古诗古文图片背景 土黄色
            beijing_tu = zb_img_res_load_one(NULL, PLAYER_IMG_BEIJINGPOEM1, IMG_FREE_TYPE_AUTO, NULL);
            lv_obj_set_style_bg_img_src(img, beijing_tu, 0);//设置图形对象背景图片
            break;
        }
        case 6: {
            //自然拼读图片背景 绿色
            beijing_tu = zb_img_res_load_one(NULL, ZIRANPINDU_IMG_BEIJING, IMG_FREE_TYPE_AUTO, NULL);
            lv_obj_set_style_bg_img_src(img, beijing_tu, 0);//设置图形对象背景图片
            break;
        }
        case 7: {
            //word 蓝色
            beijing_tu = zb_img_res_load_one(NULL, WORD_IMG_BACKGROUND, IMG_FREE_TYPE_AUTO, NULL);
            lv_obj_set_style_bg_img_src(img, beijing_tu, 0);//设置图形对象背景图片
            break;
        }
        case 8: {
            //search 黄色
            beijing_tu = zb_img_res_load_one(NULL, SEARCH_IMG_BACKGROUND, IMG_FREE_TYPE_AUTO, NULL);
            lv_obj_set_style_bg_img_src(img, beijing_tu, 0);//设置图形对象背景图片
            break;
        }
        case 9: {
            //认识汉字 绿色
            beijing_tu = zb_img_res_load_one(NULL, RENSHIHANZI_IMG_SETTING_1, IMG_FREE_TYPE_AUTO, NULL);
            lv_obj_set_style_bg_img_src(img, beijing_tu, 0);//设置图形对象背景图片
            break;
        }
        case 10: {
            //pyxx 绿色
            beijing_tu = zb_img_res_load_one(NULL, PYXX_IMG_LVLX_BACKGROUND, IMG_FREE_TYPE_AUTO, NULL);
            lv_obj_set_style_bg_img_src(img, beijing_tu, 0);//设置图形对象背景图片
            break;
        }
        default: {
            break;
        }
    }

    return 0;
}

/**
 * 播放的功能配置
 */
static void play_normal_config(void){
    LV_LOG_USER("已进入：播放的功能配置\n");
    web_disk_player_func.play_enable = true;        //播放或暂停功能激活
    web_disk_player_func.pre_unit_enable = true;    // 下一首enable
    web_disk_player_func.next_unit_enable = true;   // 上一首enable
    web_disk_player_func.play_continu_enable = true;// 继续播放enable
    web_disk_player_func.record_enable = false;     // 录音使能
    web_disk_player_func.play_exit = false;         // 播放退出
    web_disk_player_func.find_enable = false;       // 查词使能
}

/**
* 读取保存SD卡的上级目录下的所有文件绝对路径
* 参数：
*   path：路径
* 返回值：
*   file_count：父目录中的文件数量
* 说明：
*   实现读取保存SD卡的上级目录下的所有文件绝对路径
*   例如传入："A/B/C"，读取出"A/B/"目录下的所有文件的绝对路径，并保存在数组中
*/
static int Read_absolute_path_SD_all_files(const char* path) {
    LV_LOG_USER("已进入：读取保存SD卡的上级目录下的所有文件绝对路径\n");
    ESP_LOGI(TAG, "传入的路径：%s", path);
    
    // 进行分配的内存
    //指针数组，存储多个指针，数组的每一个元素就是指针{ *, *, *, *}，所以你需要为每个元素单独分配内存
    for (int i = 0; i < PLYBACK_FILES_MAX; i++) {
        g_plyDispInf->m_absolutePathTbl[i] = heap_caps_malloc_dflt(PATH_BYTE_SIZE);
    }
    int file_count = read_files_in_parent_directory(path);//读取父目录中的歌曲数量

    Number_Songs_parent_directory = file_count;//上级目录中的歌曲数量

    ESP_LOGI(TAG, "找到的文件数量：%d", file_count);
#if 0
    // 打印文件路径
    int outputNum = (file_count <= 10 ? file_count : 10);
    for (int i = 0; i < outputNum; ++i) {
        ESP_LOGI(TAG, "rA absPathTbl[%d]= %s\n", i, g_plyDispInf->m_absolutePathTbl[i]);
    }
#endif
    return file_count;
}

/**
* 读取父目录中的文件
* 参数：
*   path：绝对路径
* 返回值：
*   count：返回找到的文件数量
*/
static int read_files_in_parent_directory(const char* path) {
    LV_LOG_USER("已进入：读取父目录中的文件\n");
    
#ifndef C_PC_SIM
    DIR* dir;
    struct dirent* entry;
    char* parent_path = zb_strdup(path); // 复制路径以进行修改
    char* last_slash = strrchr(parent_path, '/'); // 找到最后一个'/'的位置
    int count = 0;

    if (last_slash == NULL) {
        ESP_LOGE(TAG, "传入的路径无效");
        if (parent_path != NULL) { heap_caps_free(parent_path); parent_path = NULL; }
        return 0;
    }

    *last_slash = '\0'; // 删除最后一个'/'之后的所有内容，获取上级目录

    dir = opendir(parent_path); // 打开上级目录
    if (dir == NULL) {
        ESP_LOGE(TAG, "无法打开目录");
        if (parent_path != NULL) { heap_caps_free(parent_path); parent_path = NULL; }
        return 0;
    }

    while ((entry = readdir(dir)) != NULL && count < PLYBACK_FILES_MAX) { // 遍历目录中的所有条目
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // 跳过当前目录和上级目录
        }
        // 检查文件扩展名是否为.mp3或.wav
        const char* dot = strrchr(entry->d_name, '.');
        if (!dot || (strcmp(dot, ".mp3") != 0 && strcmp(dot, ".wav") != 0)) {
            continue; // 如果不是，跳过这个文件
        }

        // 获取文件的绝对路径
        char full_path[PATH_MAX];
        full_path[0] = '\0';

        // 构建文件的绝对路径
        const char * pPathNew = join_path(parent_path, entry->d_name);
        if (realpath(pPathNew, full_path) == NULL) {
            ESP_LOGE(TAG, "无法获取文件的真实路径");
            if (pPathNew != NULL) { 
                heap_caps_free(pPathNew);  pPathNew = NULL;
            }
            continue;
        }
        if (pPathNew != NULL) { 
            heap_caps_free(pPathNew);  pPathNew = NULL;
        }

        g_plyDispInf->m_absolutePathTbl[count][0] = '\0';
        strncat(g_plyDispInf->m_absolutePathTbl[count], full_path, PATH_BYTE_SIZE - 1);
        if (count < 10) {
            ESP_LOGI(TAG, "rDir absPathTbl[%d]= %s", count, g_plyDispInf->m_absolutePathTbl[count]);
        }
        count++;
    }

    closedir(dir);
    heap_caps_free(parent_path);


    return count; // 返回找到的文件数量
#else
    return 6;
#endif
}

/**
* 辅助函数，将目录名和文件名合并为一个完整的路径
* 参数：
*   dir：目录名
*   file：文件名
*/
static char* join_path(const char* dir, const char* file) {
    size_t dir_len = strlen(dir);
    size_t file_len = strlen(file);
    char* path = heap_caps_malloc_dflt(dir_len + file_len + 2); // +2 for '/' and '\0'
    if (path == NULL) {
        ESP_LOGE(TAG, "无法为路径分配内存");
        return NULL;
    }
    snprintf(path, dir_len + file_len + 2, "%s/%s", dir, file);
    return path;
}

static void _seekAudBySentenceNo(int16_t sentenceNo)
{
	ESP_LOGI(TAG, "seek to sentNo %hd", sentenceNo);
	do {
		if (sentenceNo < 0)
			break;
		if (sentenceNo >= g_pAudLrc->lyrIdx.cntIdx)
			break;
		int newPosTm = g_pAudLrc->lyrIdx.idxTbl[sentenceNo][0];
	#ifndef C_PC_SIM
		tz_play_set_start_end_time(newPosTm, -1);
	#endif
	} while (0);
	return;
}


//快进功能
static int fast_forward_function(void) {
    LV_LOG_USER("已进入：快进功能\n");
#ifndef C_PC_SIM
    music_player_get_play_time(&g_posTm, &g_ttlTm);
#else
    g_ttlTm = 1000 * 60;
    if (g_posTm < 0 || g_posTm >= g_ttlTm) {
        g_posTm = 0;
    }
#endif
    if ((int)g_posTm + FAST_ADVANCE_TIME_DIFF_MIN < (int)g_ttlTm) {
        int fw_step = g_ttlTm / 30;
        if (fw_step < FAST_ADVANCE_TIME_DIFF_MIN) {
            fw_step = FAST_ADVANCE_TIME_DIFF_MIN;
        }
        int newPosTm = g_posTm + fw_step;
        if (newPosTm < g_ttlTm) {
            ESP_LOGE(TAG, "播放快进偏移；%d", newPosTm);
        #ifndef C_PC_SIM
            tz_play_set_start_end_time(newPosTm, -1);//播放偏移
        #endif
        }
    }
    return 0;
}

//快退功能
static int fast_rewind_function(void) {
    LV_LOG_USER("已进入：快退功能\n");
#ifndef C_PC_SIM
        music_player_get_play_time(&g_posTm, &g_ttlTm);
#else
        g_ttlTm = 1000 * 60;
        if (g_posTm < 0 || g_posTm >= g_ttlTm) {
            g_posTm = 0;
        }
#endif
    if ((int)g_posTm > 0) {
        int fw_step = g_ttlTm / 30;
        if (fw_step < FAST_ADVANCE_TIME_DIFF_MIN) {
            fw_step = FAST_ADVANCE_TIME_DIFF_MIN;
        }
        int newPosTm = g_posTm - fw_step;
        if (newPosTm < 0) {
			newPosTm = 0;
        }
        ESP_LOGE(TAG, "播放快退偏移；%d", newPosTm);
    #ifndef C_PC_SIM
        tz_play_set_start_end_time(newPosTm, -1);//播放偏移
    #endif
    }
    return 0;
}

//删除对象事件处理函数
static void _myMainWinDelCb(lv_event_t* evt) {
    LV_LOG_ERROR("已进入：对象正在被删除\n");

    lv_event_code_t evtCode = evt->code;
    int keyVal = lv_event_get_key(evt);
    if (evtCode == LV_EVENT_DELETE) {
        //对象正在被删除
        ESP_LOGI(TAG, "%d:从播放器版面——》列表内容版面", __LINE__);
        ESP_LOGI(TAG, "当前所在行号:%d  播放器切换版面标志位: %d\r\n", __LINE__, player_toggle_layout_flag_bit);
    #ifndef C_PC_SIM
        tz_play_stop_audio();//停止播放
    #endif
        if (Current_value_playback < 0) {
            Current_value_playback = getCurSongIdxInList();
        }

        //销毁定时器
        if (refresh_timer != NULL) {
            lv_timer_del(refresh_timer); refresh_timer = NULL;
        }
        if (file_name != NULL) {
            heap_caps_free(file_name); file_name = NULL;
        }
        // 清理分配的内存
        for (int i = 0; i < PLYBACK_FILES_MAX; i++) {
            if (g_plyDispInf->m_absolutePathTbl[i] != NULL) {
                heap_caps_free(g_plyDispInf->m_absolutePathTbl[i]);
                g_plyDispInf->m_absolutePathTbl[i] = NULL;
            }
        }
        //--------
        if (player_toggle_layout_flag_bit == 1) {//百度网盘已下载的主界面
            lv_timer_create(zb_ex_cmd_timer_cb, 0, (void *)SWITCH_PLAYER_WB_MAINCONT);//从播放器版面——》列表内容版面
        }
        else if (player_toggle_layout_flag_bit == 2) {//百度网盘已下载的子界面
            lv_timer_create(zb_ex_cmd_timer_cb, 0, (void *)SWITCH_PLAYER_WB_TEXTBOOK);//从播放器版面——》列表内容版面
        }
        else if (player_toggle_layout_flag_bit == 3) {//我的下载AP 
            lv_timer_create(zb_ex_cmd_timer_cb, 0, (void *)SWITCH_PLAYER_MYDOWN);//从播放器版面——》列表内容版面
        }
        player_toggle_layout_flag_bit = 0;

        if (g_pAudLrc) {
            Aud_LyrClose(g_pAudLrc);
            heap_caps_free(g_pAudLrc);  g_pAudLrc = NULL;
        }
        heap_caps_free(g_plyDispInf);  g_plyDispInf = NULL;

        if (Baidu_displays_absolute_path_file_played != NULL) {
            heap_caps_free(Baidu_displays_absolute_path_file_played);  Baidu_displays_absolute_path_file_played = NULL;
        }
        g_lblPlayMode = NULL;
    }
}


//键盘处理回调函数
static void _myMainWinCb(lv_event_t* evt) {
    int keyVal = 0;
    lv_event_code_t evtCode = evt->code; //从evt中获取事件代码，并赋值
    keyVal = lv_event_get_key(evt);//获取按键值
    if (evtCode == LV_EVENT_KEY_STAGE){
        ESP_LOGI(TAG, "%d：for keyLong、keyRepeat、keyShort", __LINE__);
        // 长按
        if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_LONG)
        {
            ESP_LOGI(TAG, "长按");
            keyVal &= ~LV_EX_FLAG_KEY_MASK;
            switch (keyVal){
                case LV_KEY_LEFT:{
                    fast_rewind_function();//快退功能
                    break;
                }
                case LV_KEY_RIGHT:{
                    fast_forward_function();//快进功能
                    break;
                }
                case LV_KEY_UP:{
                    Previous_play_function();//上一首
                    if (lv_obj_has_state(pause_start_btn, LV_STATE_CHECKED) == 1) {
                        lv_obj_clear_state(pause_start_btn, LV_STATE_CHECKED);
                        lv_label_set_text(pause_start_label, "播放");
                        //lv_timer_resume(refresh_timer);//恢复计数器
                        ESP_LOGW(TAG, "%d：播放",__LINE__);  
                    }
                    break;
                }
                case LV_KEY_DOWN:{
                    Next_play_function();//下一首
                    if (lv_obj_has_state(pause_start_btn, LV_STATE_CHECKED) == 1) {
                        lv_obj_clear_state(pause_start_btn, LV_STATE_CHECKED);
                        lv_label_set_text(pause_start_label, "播放");
                        //lv_timer_resume(refresh_timer);//恢复计数器
                         ESP_LOGW(TAG, "%d：播放",__LINE__);  
                    }
                    break;
                }
            }
        }
        // 短按
        else if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_SHORT)
        {
            ESP_LOGI(TAG, "短按");
            keyVal &= ~LV_EX_FLAG_KEY_MASK;
            switch (keyVal){
                case LV_KEY_EX_MEDIA_PREV: { // 左切换
                    ESP_LOGI(TAG,"左切换");
                    fast_rewind_function();//快退功能
                    break;
                }
                case LV_KEY_EX_MEDIA_NEXT: { // 右切换
                    ESP_LOGI(TAG, "右切换");
                    fast_forward_function();//快进功能
                    break;
                }
                case LV_KEY_EX_MEDIA_PLAY_PAUSE: { // 播放或暂停
                    //lv_event_stop_processing(evt);//停止当前事件的处理，不再执行后续的事件回调函数
                    //lv_indev_keypad_wait_release();//等待键盘输入设备的按键释放
                    _my_btn_pause_start_cb(evt);//播放、暂停按钮回调函数
                    break;
                }
                case LV_KEY_UP: { // (对于有lrc的)上一句 
                    if (!SONG_HAS_LRC())
                        break;
                    int16_t sentenceNo = g_curSentenceNo;
                    if (sentenceNo <= 0)
                        break;
                    _seekAudBySentenceNo(sentenceNo - 1);
                    break;
                }
                case LV_KEY_DOWN: {// (对于有lrc的)下一句 
                    if (!SONG_HAS_LRC())
                        break;
                    int16_t sentenceNo = g_curSentenceNo;
                    if (sentenceNo + 1 >= g_pAudLrc->lyrIdx.cntIdx)
                        break;
                    _seekAudBySentenceNo(sentenceNo + 1);
                    break;
                }
                default:
                    break;
            }
        }
        else if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_REPEAT) {
            ESP_LOGI(TAG, "持续按");
            keyVal &= ~LV_EX_FLAG_KEY_MASK;
            switch (keyVal) {
                case LV_KEY_LEFT: { // 左切换
                    ESP_LOGI(TAG, "左切换");
                    fast_rewind_function();//快退功能
                    break;
                }
                case LV_KEY_RIGHT: { // 右切换
                    ESP_LOGI(TAG, "右切换");
                    fast_forward_function();//快进功能
                    break;
                }
                case LV_KEY_UP: { // 上一句
                    break;
                }
                case LV_KEY_DOWN: {// 下一句
                    break;
                }
                case LV_KEY_END: {
                    break;
                }
                case LV_KEY_EX_FUNC1: // ab复读
                {
                    break;
                }
            }
        }
    }

    if (LV_EVENT_KEY == evtCode) { //判断是否是一个按键事件
        ESP_LOGI(TAG, "是一个按键事件,keyVal=%d", keyVal);
        ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_WEB_DISK_scrNo, zb_getOpenedFileNum());

        //ESP_LOGI(TAG, "%d：停止当前事件的处理", __LINE__);
        //lv_event_stop_processing(evt);
        //ESP_LOGI(TAG, "%d：停止当前事件的处理", __LINE__);
        if (!keyVal) {
            keyVal = lv_event_get_key(evt); //重新从事件对象中获取按键值
        }

        if (LV_KEY_ESC == keyVal) {
            LV_LOG_ERROR("Esc键，keyVal = %d\n", keyVal);
            ESP_LOGI(TAG, "AP=%d，当前打开的文件数量:%d", g_WEB_DISK_scrNo, zb_getOpenedFileNum());
            lv_event_stop_processing(evt);//停止当前事件的处理，不再执行后续的事件回调函数
            lv_indev_keypad_wait_release();//等待键盘输入设备的按键释放
#ifndef C_PC_SIM
            tz_play_stop_audio();//停止播放
            ESP_LOGI(TAG, "tz_play_stop_audio ");
#endif
            if (g_mainWin) {
                lv_obj_del(g_mainWin);  g_mainWin = NULL;
                ESP_LOGI(TAG, "lv_obj_del：%d", __LINE__);
            }
            //销毁窗体 
        }
        else if (LV_KEY_EX_FUNC1 == keyVal) {
            LV_LOG_ERROR("F1键，keyVal = %d\n", keyVal);
            lv_event_stop_processing(evt);
            lv_indev_keypad_wait_release();
            //AB键 切换三种模式 列表循环 列表随机 单曲循环
            g_playOrderMode++;
            g_playOrderMode = g_playOrderMode % 3;
            updatePlayModeBySelNo();
            //ESP_LOGI(TAG, "%d：单曲循环web_disk_player_para_set.single_cycle = %d", __LINE__, web_disk_player_para_set.single_cycle);
            //ESP_LOGI(TAG, "%d：列表顺序web_disk_player_para_set.list_order = %d", __LINE__, web_disk_player_para_set.list_order);
        }
        else if (LV_KEY_EX_RECORD == keyVal) {
            LV_LOG_ERROR("F2录音键，keyVal = %d\n", keyVal);
            lv_event_stop_processing(evt);//停止当前事件的处理，不再执行后续的事件回调函数
            lv_indev_keypad_wait_release();//等待键盘输入设备的按键释放
            g_plyDispBgNo++;
            g_plyDispBgNo = g_plyDispBgNo % PLY_DISP_BG_TYPES_N;
            ESP_LOGI(TAG, "g_plyDispBgNo = %hd", (short)g_plyDispBgNo);
            if (g_plyDispBgNo >= 0 &&  g_plyDispBgNo < PLY_DISP_BG_TYPES_N) {
                ESP_LOGI(TAG, "No%d 背景", (int)g_plyDispBgNo);
                web_disk_Player_background_selection(g_plyDispBgNo);
            }
        }
    }

    if (EVENT_EX_AUD_PLAYER == evtCode) { //判断是否是一个播放事件
        LV_LOG_ERROR(" 播放器有状态变化 EVENT_EX_AUD_STREAM keyVal = %d\n", keyVal);
        int* pInt = (int*)lv_event_get_param(evt);//用于从事件对象中获取参数
        int parm = (pInt ? (*pInt) : 0);
        ESP_LOGI(TAG, "evtAudStrm 0x%x", parm);
        ESP_LOGI(TAG, "parm = %d", parm);
        if (MUSIC_STATUS_FINISH == parm) {  //判断是否播放完毕
            ESP_LOGE(TAG, "音乐已播放完毕");
            //播放模式
            ESP_LOGI(TAG, "%d：单曲循环web_disk_player_para_set.single_cycle = %d", __LINE__, web_disk_player_para_set.single_cycle);
            ESP_LOGI(TAG, "%d：列表顺序web_disk_player_para_set.list_order = %d", __LINE__, web_disk_player_para_set.list_order);
            // 单曲循环
            if (web_disk_player_para_set.single_cycle) {
                single_cycle_function();
            }
            // 列表顺序
            else if (web_disk_player_para_set.list_order) {
                sequential_play_function();//本首歌曲播放完成之后，顺序自动播放下一首
            }
            // 播放完成后暂停
            else {
                ESP_LOGI(TAG, "ply end.");
                if (C_ZB_AP_BD_NET_DISK != zb_getApNo()) {
                    ESP_LOGI(TAG, "pasue timer.-~");
                    lv_timer_pause(refresh_timer);
                }
                if (lv_obj_has_state(pause_start_btn, LV_STATE_CHECKED) == 0) {
                    _my_btn_pause_start_cb(NULL);
                }
                // play_or_pause_func(); //播放或暂停功能
            }
        }
        else if (MUSIC_STATUS_PLAYING == parm) {
            ESP_LOGI(TAG, "音乐正在播放");
            ESP_LOGI(TAG, "%d：单曲循环web_disk_player_para_set.single_cycle = %d", __LINE__, web_disk_player_para_set.single_cycle);
            ESP_LOGI(TAG, "%d：列表顺序web_disk_player_para_set.list_order = %d", __LINE__, web_disk_player_para_set.list_order);
        }

    }
}


