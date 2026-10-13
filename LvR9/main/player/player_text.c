/*******************************************************
                    歌词显示页面程序
******************************************************/

#include "player_text.h"
#include "../data_support/db_manager.h"
#include <sys/stat.h>
#include "../utils/global_parm.h"
#include "../zb_base.h"
#include "./jump_dic.h"
#include "../utils/zb_aud_api.h"
#include "../utils/style_tz.h"
#include "../mainmenu/mainmenu_base.h"
#ifndef C_PC_SIM
#include "LvR9/trd/tz_audio_player/include/tz_audio_player.h"
#include "LvR9/trd/tz_audio_player/include/tz_pcm_audio.h"
#include "LvR9/main/online_res_ap/cpp_api_bridge.h"
#endif

#define PASS_PATH  0
#define recentlyPlayed_define 0
#define collect_define 0

#define lowndoad_player_test 1


extern int beAllowedJumpDict(const char* pcDatumPath);

extern void dictScreen_del(void);
void create_playlist_screen(void* parm);
void create_repeat_settings_ui(lv_obj_t* parent);
/* 事件回调函数声明 */
static void all_play_click_handler(lv_event_t* e);
static void list_item_click_handler(lv_event_t* e);
static void repeat_switch_check(void);
void player_switchScr(PlayerScrNo scrNoNew, void* parm);
bool player_has_valid_data(void);
static int play_text_sentence_parse(char* path, uint16_t play_num);
#define TAG "playTxt"
//========================================
#define	PLAYER_V2 	1  //新版本

#define	PLAYER_BACKPLAY 	0  //背景播放
#define	AB_PS_TIME		0  //用时间还是句子定位 1 -- 时间，0 --句子

//#define	PLAYER_REPEAT_BTN_SN	1	// 

#define		BTN_SEL_COLOR	0xFFD600

#define M_IS_PLY_ALARM_MOD()   (play_text_group != 0)

//========================================
//最返播放支持播下一个
//extern Record * recordCol;  // 记录
//extern Music * micCol;
static uint8_t playlist_sn;   		//最近播放和收藏当前播放的位置
extern uint8_t collect_album_flag;   	//收藏专辑标记

//----------------------
// 有的书本单元句子太多，内存会不够用，分多次加载。
#define	 LOAD_SENTENCE_MULT	0		//多次加载

#define	 LOAD_SENTENCE_NUM	50		//单次最多加载的数里
#define	 SENTENCE_PAGE_NUM	5		//估计一页显示多少条数据，不能比 LOAD_SENTENCE_NUM 大。

extern int play_chapter_page_init(int16_t active_item);
extern bool current_textbook_is_quanjie();
static void playlist_win_del();
static void func_page_release();
int play_next_unit(void);
int play_pre_unit(void);
static int play_new_unit_info_load(void);
//========================================

extern int16_t ap_toggle_num;
extern int play_list_clicked_btn_num;
static PlayerScrNo g_player_scrNo = -1; // 当前页面编号
//========================================

static lv_obj_t* play_func_btn_tl[8] = { 0 }; //定义窗体OBJ 在播放器中的焦点序号
//定义窗体OBJ 在播放器中的焦点序号
#define	PLAYER_TEXT_WIN_SN			0	// 
#define	PLAYER_PLAY_PAUSE_BTN_SN	3	// 
#define	PLAYER_COLLECT_BTN_SN		7	// 
#define	PLAYER_SPEED_BTN_SN			30	// 
#define	PLAYER_FIND_BTN_SN			40	// 
#define	PLAYER_CYCLE_BTN_SN			50	// 

static lv_obj_t* play_pause_label = NULL;

static player_book_info_t play_text_book_info = { 0 };
player_unit_info_t play_unit_info = { 0 };

static ab_cycle_param_t ab_param = { 0 };
static player_func_t player_func = { 0 };

static lv_timer_t* refresh_timer = NULL;
static lv_obj_t* total_time_label = NULL;
static lv_obj_t* current_time_label = NULL;
static lv_obj_t* play_slider = NULL;
static lv_obj_t* text_cont = NULL;
static lv_obj_t* title_label = NULL;
static lv_obj_t* col_btn = NULL;
static uint8_t player_state_flag = 0;
static uint8_t player_interval_flag = 0;
static uint8_t g_extra = 0;
static lv_obj_t* player_bg = NULL;;
uint8_t player_backstage_flag = 0;

lv_group_t* play_text_group = NULL;
uint32_t  g_alarmTmStart;  // 响闹开始播放的时间, 单位为 ms (离开播放版面、用户暂停或超时暂停后,它就变为 0)

//功能页
lv_obj_t* func_scr = NULL;

//userly
bool ls_loopmode_play = true;
TZ_PLAYSTATE_T curr_loopmode_play_state = TZ_PLAYIDLE;

static RepeatParams params = { NULL };

uint64_t  g_songTmTotal = 0;
uint64_t  g_songTmCur = 0;
static bool is_user_dragging = false;  // 标记用户是否正在拖动进度条

#define btn_size 50

bool MMN_ENTER_STAT = 0;

// 播放器参数配置
static player_para_set_t play_para = {
    .speed = 1,
    .repeat_enabled = false,
    .loopmode = LOOP_MODE_ORDER,
    .last_random = -1,
    .loop_times = LOOP_TIMES_2,
};

#if PLAYER_V2
play_function_img_info_t playFuncInfo = {  //这个目前定义为播放页功能项
    .elementNum = 5,
    .imgNum = {PLAYER_IMG_PLAYER_FOLLOW, PLAYER_IMG_PLAYER_PREV, PLAYER_IMG_PLAYER_PLAY, PLAYER_IMG_PLAYER_NEXT,PLAYER_IMG_FUNC_BTN},//PLAYER_IMG_FUDU1
    .offset_x = 64,
    .offset_y = 160,
    .event_cb = {read_aloud_btn_click_cb,previous_track_btn_click_cb,play_or_pause_btn_click_cb,next_track_btn_click_cb,function_btn_click_cb,playlist_btn_click_cb,back_btn_click_cb,collect_btn_click_cb}, //loop_btn_click_cb
};

play_function_img_info_t menuFuncInfo = {  //这个定义为菜单功能选项
    .elementNum = 5,
    .imgNum = {PLAYER_IMG_PLAYER_FOLLOW, PLAYER_IMG_PLAYER_PREV, PLAYER_IMG_PLAYER_PAUSE, PLAYER_IMG_PLAYER_NEXT},//PLAYER_IMG_FUDU1
    .offset_x = 64,
    .offset_y = 160,
    //.elmentStr = {"暂停", "收藏", "倍速", "查词", "循环" }, //"复读"
    .event_cb = {play_or_pause_btn_click_cb,collect_btn_click_cb, speed_btn_event_cb, find_btn_click_cb, single_cycle_btn_click_cb}, //loop_btn_click_cb
};

play_function_img_info_t listFuncInfo = {  //这个定义为播放列表功能选项
    .elementNum = 3,
    .imgNum = {PLAYER_IMG_PLAYER_FOLLOW, PLAYER_IMG_PLAYER_PREV, PLAYER_IMG_PLAYER_PAUSE, PLAYER_IMG_PLAYER_NEXT},
    .event_cb = {list_return_btn_click_cb,collect_btn_click_cb}, //loop_btn_click_cb
};


//----- 定义图片按钮序号-------------
#define	PLAYER_PLAY_PAUSE_IMG_SN	0	// 
#define	PLAYER_COLLECT_IMG_SN		1	// 
#define	PLAYER_SPEED_IMG_SN			2	// 
#define	PLAYER_FIND_IMG_SN			3	// 
#define	PLAYER_CYCLE_IMG_SN			4	// 
#define	PLAYER_LOOP_IMG_SN			4	// 暂时不用

#else

play_function_img_info_t playFuncInfo = {
    .elementNum = 5,
    .imgNum = {PLAYER_IMG_SHOUCANG1, PLAYER_IMG_BEISHU1, PLAYER_IMG_CHACI, PLAYER_IMG_XUNHUAN1, PLAYER_IMG_FUDU1},
    .offset_x = 45,
    .offset_y = 0,
    .elmentStr = {"收藏", "倍速", "查词", "循环", "复读"},
    .event_cb = {collect_btn_click_cb, speed_btn_click_cb, find_btn_click_cb, single_cycle_btn_click_cb, loop_btn_click_cb}, //
};

#endif
static const int8_t play_speed[5] = { MUSIC_PLAY_SPEED_0_50, MUSIC_PLAY_SPEED_0_75,MUSIC_PLAY_SPEED_1_00, MUSIC_PLAY_SPEED_1_25, MUSIC_PLAY_SPEED_1_50 };
static const uint32_t speed_img[5] = { PLAYER_IMG_BEISHU1, PLAYER_IMG_BEISHU2, PLAYER_IMG_BEISHU3, PLAYER_IMG_BEISHU4, PLAYER_IMG_BEISHU5 };

static bool dialogFlag = 1;


static lv_obj_t* dialog; //弹窗
static lv_timer_t* timer_power;

//=======================================================================================
static void ab_param_clean(void);
static void ab_cycle_repos(uint64_t posTm, TZ_PLAYSTATE_T audStat);
static int play_text_cont_load_part_init(uint16_t playIndex);
static int play_text_cont_load_part_check(int16_t current_sentence_sn, int direction);
static int play_text_win_free_for_internal(void);
void play_or_pause_func(void);
void change_collect_btn_img(void);

typedef int (*FN_PLAYER_SCREEN_INIT)(void* parm);
typedef int (*FN_PLAYER_SCREEN_FREE)(void);
static const struct PLAYER_SCR_INF {
    FN_PLAYER_SCREEN_INIT init;  // 页面初始化函数
    FN_PLAYER_SCREEN_FREE free;  // 页面释放函数
};
//=======================================================================================

#if lowndoad_player_test

#define MAX_MP3_FILES 100
#define MAX_NAME_LEN 256

#ifdef C_PC_SIM
void tz_play_audio(const char* audioname, int64_t start, int64_t end,bool stat);
void tz_play_pause_audio();
void tz_play_resume_audio();
void tz_play_stop_audio();
void tz_play_set_volume(int new_volume);
void tz_play_set_speed(float new_speed);
void tz_play_set_start_end_time(int64_t start, int64_t end);
TZ_PLAYSTATE_T get_play_audio_state();
#endif
TZ_PLAYSTATE_T player_state;

#ifndef C_PC_SIM
#define PLAYER_TEST_FILE_PATH               "/resource"

#else
#define PLAYER_TEST_FILE_PATH                "/userdata/_data"
#endif
#define _S_IFMT   0xF000 // File type mask
#define _S_IFREG  0x8000 // Regular
#define NUMBER_FILES_MAX 50 // 示例值
static char* second_son_absolute_path[NUMBER_FILES_MAX];
static char* second_level_directory[NUMBER_FILES_MAX];
#define MDLD_FILE_NAME_BUF_LEN  256
#define PATH_BYTE_SIZE 300  //路径字节大小
static int Number_secondary_file_dir = 0;
int music_play_cur = 0;

int music_count = 0;

#define _S_ISTYPE(mode, mask)  (((mode) & _S_IFMT) == (mask))
#define S_ISREG(mode) _S_ISTYPE((mode), _S_IFREG)
#define S_ISDIR(mode) _S_ISTYPE((mode), _S_IFDIR)


void free_music_file_data() {
    for (int i = 0; i < Number_secondary_file_dir; i++) {
        heap_caps_free(second_son_absolute_path[i]);
        heap_caps_free(second_level_directory[i]);
        second_son_absolute_path[i] = NULL;
        second_level_directory[i] = NULL;
    }
    Number_secondary_file_dir = 0;
}

#endif
const char* getDatumBasePath(const char* pcPath2ndPart) {

    const char* pcBasePath = NULL;
    if (ap_toggle_num == C_ZB_AP_TEXTBOOK_ENG || ap_toggle_num == C_ZB_AP_TEXTBOOK_CN)
    {
#ifndef C_PC_SIM
        pcBasePath = "/oem/_data/textbook";
#else
        pcBasePath = "/userdata/_data/textbook";
#endif
        return pcBasePath;
    }
    else if (ap_toggle_num == C_ZB_AP_MY_DOWNLOAD) {
        ESP_LOGI(TAG, "==============\n");
        pcBasePath = "/userdata/_data/textbook";
        return pcBasePath;
    }
    else         //if(player_class_num == DATUM_CHINESE_CLASSIC) {
    {
        pcBasePath = PLAYER_DATA_PATH;
            return pcBasePath;

    }
}
#ifdef C_PC_SIM
void tz_play_audio(const char* audioname, int64_t start, int64_t end) {}
// 暂停播放
void tz_play_pause_audio() {}
// 继续播放
void tz_play_resume_audio() {}
// 停止播放
void tz_play_stop_audio() {}
// 设置音量0-100
void tz_play_set_volume(int new_volume) {}
// 设置播放速度
void tz_play_set_speed(float new_speed) {}
// 新增：动态设置开始时间的函数 时间设置为ms start：开始时间 end结束时间
void tz_play_set_start_end_time(int64_t start, int64_t end) {}
//新增:播放状态
TZ_PLAYSTATE_T get_play_audio_state() {}

void tz_play_stop_mem_audio() {}
void tz_play_pause_mem_audio() {}
void tz_play_resume_mem_audio() {}

// MP3数据 播放状态
//TZ_PLAYSTATE_T get_play_audio_state() {}

//调用者使用
//extern void audio_play_state(TZ_PLAYSTATE_T state);
////调用者使用state current_time:s audio_time:s
#endif

// 获取当前页面编号
PlayerScrNo get_current_player_screen(void) {
    return g_player_scrNo;
}

// 判断是否在特定页面
bool is_player_screen_active(PlayerScrNo scrNo) {
    return g_player_scrNo == scrNo;
}

/**
 * @brief 返回当前播放的路径还有标题
 */
void player_get_current_media_info(char* path_buf, char* title_buf) {

    if (path_buf) *path_buf = '\0';
    if (title_buf) *title_buf = '\0';

 
    if (play_text_book_locate.path[0] == '\0') {
      
        return;
    }

    if (path_buf) {
        size_t path_size = sizeof(play_text_book_locate.path);
        strncpy(path_buf, play_text_book_locate.path, path_size - 1);
        path_buf[path_size - 1] = '\0';
    }

    if (!title_buf ||
        !play_text_book_info.pTile ||
        play_text_state.cur_play_unit_num < 0 ||
        play_text_state.cur_play_unit_num >= play_text_book_info.cnt) {
     
        return;
    }

    const char* title_str = play_text_book_info.pTile[play_text_state.cur_play_unit_num];
    if (!title_str || *title_str == '\0') {

        return;
    }

    size_t max_title_size = sizeof(play_text_book_locate.title);
    strncpy(title_buf, title_str, max_title_size - 1);
    title_buf[max_title_size - 1] = '\0';
}

/**
 * @brief 统一播放控制函数（支持本地文件、在线资源和流数据播放）
 * @param index 播放索引
 * @param path 路径或资源标识
 * @param is_stream 是否流数据播放
 * @return 成功返回0，失败返回错误码
 */
#if 1
static int play_media_unit(int index) {
    // 参数校验
    if (index < 0) {
        LV_LOG_ERROR("Invalid play parameters");
        return -1;
    }
    //if (!player_func.next_unit_enable)
    //{
    //    ESP_LOGI("play_next_unit", "!player_func.next_unit_enable\r\n");
    //    return -1;
    //}
    //if (play_text_book_info.cnt <= 0)
    //{
    //    ESP_LOGI("play_next_unit", "play_text_book_info.cnt <= 0\r\n");
    //    return -2;
    //}
    // 不能停止当前播放，会断错误
    //tz_play_stop_audio();
    
    // 更新全局状态
    play_para.current_index = index;
    play_text_state.cur_play_unit_num = index;

    ESP_LOGI("play_media_unit", "---------------A------------\\r\n");
    int ret = play_new_unit_info_load();
    ESP_LOGI("play_media_unit", "---------------B------------\\r\n");
    if (ret)
    {
        return -3;
    }

    // 处理不同类型的播放资源
    if (!current_playlist.items) {
        // 流数据播放：直接使用已加载的 play_unit_info
#ifndef C_PC_SIM
        if (play_unit_info.fp && play_unit_info.audLen > 0) {
            aud_file_part_play(NULL, play_unit_info.fp,
                play_unit_info.audOffset,
                play_unit_info.audLen,
                play_unit_info.encParm,
                play_para.speed);
            LV_LOG_USER("Stream playback started");
        }
        else {
            LV_LOG_ERROR("No valid stream data available");
            return -3;
        }
#else
        g_songTmTotal = 120 * 1000;
        LV_LOG_USER("Simulated stream playback");
#endif
        }
    else {

        PlaylistItem* item = &current_playlist.items[play_text_state.cur_play_unit_num];

        if (item->source_type == MUSIC_SOURCE_TYPE_SD) {
            strncpy(play_text_book_locate.path, item->location,
                sizeof(item->location) - 1);
            // 本地文件处理
            char buf[200] = { 0 };
            snprintf(buf, sizeof(buf), "%s/album.mnu", item->location);

            // 更新歌曲信息
            if (update_song_info_in_book_info(&play_text_book_info, index, buf) != 0) {
                LV_LOG_ERROR("Failed to update song info");
                return -2;
            }

            // 解析歌词文本
            if (play_text_sentence_parse(item->location, index) != 0) {
                LV_LOG_ERROR("Sentence parsing failed");
                return -3;
            }

#ifndef C_PC_SIM
            // 播放本地文件
            aud_file_part_play(NULL, play_unit_info.fp,
                play_unit_info.audOffset,
                play_unit_info.audLen,
                play_unit_info.encParm,
                play_para.speed);
#endif
        }
        else {
            if (!player_acquire_request_lock(index, item->location, item->source_type)) {
                LV_LOG_WARN("Request lock acquisition failed");
                return -4;
            }
            request_media_playback(item->location, item->source_type);
        }

  
        if (!player_func.play_exit) {
            if (current_playlist.items && current_playlist.items[index].title) {
                lv_label_set_text(title_label, current_playlist.items[index].title);
            }
            
        }
    }
    change_collect_btn_img();
    if (refresh_timer) {
        lv_timer_resume(refresh_timer);
    }

    return 0;
}

#else
 /* 统一播放控制函数 */
static int play_media_unit(int index, const char* path)
{
    int  ret = 0;
    // 参数校验
    if (index < 0 || index >= music_count || !path) {
        LV_LOG_ERROR("Invalid play parameters");
        ret = -1;
        return ret;
    }

    // 停止当前播放
    tz_play_stop_audio();

    // 更新全局状态
    play_para.current_index = index;

    tz_play_audio(path, 0, -1, true);

    return ret;
}



#endif



/* 定时器回调函数 */
static void timer_cb(lv_timer_t* timer) {

    if (dialog != NULL)
    {
        lv_obj_del(dialog);  dialog = NULL;

    }
    if (timer != NULL)
    {
        lv_timer_del(timer);
    }
    dialogFlag = 1;
}


static lv_style_t style_dialog;
static void create_dialog_style(void) {
    lv_style_init(&style_dialog);
    lv_style_set_bg_color(&style_dialog, lv_color_hex(0x000000));
    lv_style_set_bg_opa(&style_dialog, LV_OPA_80);
    lv_style_set_border_width(&style_dialog, 0);
    lv_style_set_clip_corner(&style_dialog, true);
    lv_style_set_radius(&style_dialog, 15);
}


static void create_dialog_text(const char* text) {

    create_dialog_style();

    dialog = lv_obj_create(lv_scr_act());
    lv_obj_add_style(dialog, &style_dialog, 0);
    lv_obj_set_size(dialog, 164, 66);
    lv_obj_set_pos(dialog, 38, 127);


    lv_obj_t* label = lv_label_create(dialog);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label);

    timer_power = lv_timer_create(timer_cb, 2000, NULL);
    lv_timer_enable(timer_power);
}


void initialize_repeat_params(RepeatParams* params, uint32_t interval, LoopTimes repeat_count){
    params->interval = interval;
    params->repeat_count = repeat_count;
    params->current_count = 0;
    params->is_repeating = true;
    params->cnt_time = 0;
    params->cnt_sec  = 0;

   LV_LOG_USER("Initialize RepeatParams: interval=%u, repeat_count=%u\n", params->interval, params->repeat_count);
}

/**
 * @brief 修改收藏按钮的图标(用于判断)
 */
void change_collect_btn_img(void)
{
    play_text_state.collect_id = 0;
#if 1
    if (!player_func.play_exit)
    {
        lv_img_dsc_t* collect_pImgDsc;

        MusicSourceType source_type;
        if (current_playlist.items)
        {
            source_type = current_playlist.items[play_text_state.cur_play_unit_num].source_type;
        }
        else
        {
            source_type = MUSIC_SOURCE_TYPE_SD;
        }
        ESP_LOGI("change_collect_btn_img","play_text_book_locate.playNum:%d,play_text_state.cur_play_unit_num:%d\r\n", play_text_book_locate.playNum, play_text_state.cur_play_unit_num);
#ifndef C_PC_SIM
        play_text_state.collect_id = db_service_get_id(C_ZB_AP_COLLECT, source_type, play_text_state.cur_play_unit_num, play_text_book_locate.path, play_text_book_info.pTile[play_text_state.cur_play_unit_num]);//这里加入是否收藏判断
#endif
        ESP_LOGI(TAG, "collect or not :%d\r\n", play_text_state.collect_id);
        if (play_text_state.collect_id >0)
        {
            ESP_LOGI(TAG, "shoucang2 index_list :%d", play_text_state.collect_id);
            collect_pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_COL, IMG_FREE_TYPE_AUTO, NULL);
        }
        else
        {
            ESP_LOGI(TAG, "shoucang1 index_list :%d", play_text_state.collect_id);
            collect_pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_UNCOL, IMG_FREE_TYPE_AUTO, NULL);
        }
        lv_obj_set_style_bg_img_src(col_btn, collect_pImgDsc, 0);
    }
#endif
}

/**
 * @brief 播放状态数据初始化
 */
static void play_text_state_init(void)
{
    play_text_state.current_play_sentence_num = 0;
    play_text_state.pre_index = -1;
    play_text_state.pre_label = NULL;
   // play_text_state.last_state = TZ_PLAYIDLE;
}
//===================================================
// flag:	0---play;
//			1-- pause
//===================================================
static void set_play_or_pause_btn_img(int flag)
{
    if (!player_func.play_exit)
    {
        lv_img_dsc_t* pImgDsc;

        if (flag)
        {
            pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_PAUSE, IMG_FREE_TYPE_AUTO, NULL);
            lv_obj_set_style_bg_img_src(play_func_btn_tl[PLAYER_PLAY_PAUSE_BTN_SN], pImgDsc, 0);
            //  lv_label_set_text(play_pause_label, "暂停");
        }
        else
        {
            pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_PLAY, IMG_FREE_TYPE_AUTO, NULL);
            lv_obj_set_style_bg_img_src(play_func_btn_tl[PLAYER_PLAY_PAUSE_BTN_SN], pImgDsc, 0);
            //  lv_label_set_text(play_pause_label, "播放");
        }
    }
}

/***********************************************************************************/
//
//播放器页回调
//
//
/************************************************************************************/

/**
 * @brief  跟读处理回调
 * @param e
 */
static void read_aloud_btn_click_cb(lv_event_t* e)
{
#if 1
    lv_obj_t* obj = lv_event_get_target(e);

    LV_LOG_USER("read_aloud_btn_click_cb\r\n");


    //if ((!play_unit_info.cnt) || (!player_func.record_enable))
    //{
    //    break;
    //}
    tz_play_stop_audio();

    play_text_win_free_for_internal();
    play_repeat_page_init(0);

    lv_indev_wait_release(lv_indev_get_act());
#endif
}

/**
 * @brief  下一曲处理回调
 * @param e
 */
static void next_track_btn_click_cb(lv_event_t* e) //download test  04.27
{
    lv_indev_wait_release(lv_indev_get_act());
    lv_obj_t* obj = lv_event_get_target(e);
    LV_LOG_USER("next_track_btn_click_cb\r\n");

    play_next_unit();

}

/**
 * @brief  上一曲处理回调
 * @param e
 */
static void previous_track_btn_click_cb(lv_event_t* e) //download test  04.27
{
    lv_indev_wait_release(lv_indev_get_act());
    lv_obj_t* obj = lv_event_get_target(e);

    LV_LOG_USER("previous_track_btn_click_cb\r\n");

    play_pre_unit();

}
/**
 * @brief  功能键处理回调
 * @param e
 */
static void function_btn_click_cb(lv_event_t* e)
{
    lv_indev_wait_release(lv_indev_get_act());
    lv_obj_t* obj = lv_event_get_target(e);

    LV_LOG_USER("function_btn_click_cb\r\n");

#if 0
    func_chos_page_init(NULL);
#else
    player_switchScr(PLAYER_SCR_FUNC_CHOOSE, NULL);
#endif
}

/**
 * @brief  播放列表处理回调
 * @param e
 */
static void playlist_btn_click_cb(lv_event_t* e)
{
    lv_indev_wait_release(lv_indev_get_act());
    lv_obj_t* obj = lv_event_get_target(e);

    LV_LOG_USER("playlist_btn_click_cb\r\n");
#if 0
    create_playlist_screen(lv_scr_act());
#else
    player_switchScr(PLAYER_SCR_PLAYLIST, NULL);

#endif
}

/**
 * @brief  返回处理回调
 * @param e
 */
static void back_btn_click_cb(lv_event_t* e)
{
    lv_indev_wait_release(lv_indev_get_act());
    if (ap_toggle_num == -1)
    {

#if play_main_online_page_init_has
        if (player_class_num == ENGLISH || player_class_num == CHINESE_TEXTBOOK)
            play_main_online_page_init(player_class_num, 1);
        else
            play_main_page_init(player_class_num);
#else
        if (MMN_ENTER_STAT)
        {
            MMN_ENTER_STAT = false;
            zb_requestSwitchAp(C_ZB_AP_MAINMENU);
        }
        else
        {
            lv_timer_create(zb_ex_cmd_timer_cb, 0, (void*)PLY_CMD_PLY_LST_RET);
        }
#endif
    }
    else
    {
#if 0
        //play_text_page_free();
#else
        play_text_win_free_keep();
#endif
        if (MMN_ENTER_STAT)
        {
            MMN_ENTER_STAT = false;
            zb_requestSwitchAp(C_ZB_AP_MAINMENU);
        }
        else
        {
            lv_timer_create(zb_ex_cmd_timer_cb, 0, (void*)PLY_CMD_PLY_LST_RET);
        }
        ap_toggle_num = -1;
    }
    
}

/**
 * @brief  播放暂停处理回调
 * @param e
 */
static void play_or_pause_btn_click_cb(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target(e);


#ifndef C_PC_SIM
    play_or_pause_func();
#else
    static int play_flag = 0;

    play_flag ^= 1;
    set_play_or_pause_btn_img(play_flag);

#endif
    LV_LOG_USER("play_or_pause_btn_click_cb\r\n");
    lv_indev_wait_release(lv_indev_get_act());
}

/**
 * @brief 收藏按钮回调
 * @param e
 */
static void collect_btn_click_cb(lv_event_t* e)
{
    lv_indev_wait_release(lv_indev_get_act());

#if 1
#ifndef C_PC_SIM
    MusicSourceType source_type;
    if (current_playlist.items)
    {
        source_type =   current_playlist.items[play_text_state.cur_play_unit_num].source_type;
    }
    else
    {
        source_type = MUSIC_SOURCE_TYPE_SD;
    }
    if (!play_text_state.collect_id)
    {
        ESP_LOGI("collect_btn_click_cb", "before add_collect \r\n");
        play_text_state.collect_id = add_to_collection(COLLECTION_TYPE_MUSIC, play_text_book_locate.path, play_text_book_info.pTile[play_text_state.cur_play_unit_num], source_type, play_text_state.cur_play_unit_num, g_songTmTotal / 1000);
        ESP_LOGI("collect_btn_click_cb", "after add_collect \r\n");
    }
    else
    {
        ESP_LOGI("collect_btn_click_cb","before remove \r\n");
        remove_from_collection(play_text_state.collect_id);
        play_text_state.collect_id = 0;
        ESP_LOGI("collect_btn_click_cb", "after remove \r\n");
    }
#else
    bool ret = 0;
#endif
    //int ret = addMusicCol_collect(play_text_book_locate.path, play_text_book_info.pTile[play_text_state.cur_play_unit_num], play_text_state.cur_play_unit_num, 0);
    lv_img_dsc_t* collect_pImgDsc = NULL;
    if (play_text_state.collect_id)
    {
        LV_LOG_USER("add collect audio succeed \r\n");
        collect_pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_COL, IMG_FREE_TYPE_AUTO, NULL);
    }
    else
    {
        LV_LOG_USER("cancel collect audio \r\n");
        collect_pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_UNCOL, IMG_FREE_TYPE_AUTO, NULL);
    }
    lv_obj_set_style_bg_img_src(col_btn, collect_pImgDsc, 0);

#endif

}

/***********************************************************************************/
//
//功能页回调
//
//
/************************************************************************************/

/**
 * @brief 查词设置按钮回调
 * @param e
 */
static void find_btn_click_cb(lv_event_t* e)
{
    LV_LOG("%s [%d] player_func.find_enable: %d \n", __FUNCTION__, __LINE__, player_func.find_enable);

    //	LV_LOG("%s [%d] path: %s \n", __FUNCTION__,__LINE__, play_text_book_locate.path );

    if (!player_func.find_enable)
    {
        return;
    }

    if (!beAllowedJumpDict(play_text_book_locate.path))
    {
        return;
    }
#ifndef C_PC_SIM
    tz_play_stop_audio();
#endif
    play_text_win_free_for_internal();
    zb_global_str_write(play_unit_info.pOrginal[play_text_state.current_play_sentence_num]);
    dictScreen_init();
}

/**
 * @brief 单曲循环设置按钮回调
 * @param e
 */
static void single_cycle_btn_click_cb(lv_event_t* e)
{

    play_para.loopmode = (play_para.loopmode + 1) % 2;

}

/**
 * @brief 重复设置按钮回调
 * @param e
 */
static void loop_btn_click_cb(lv_event_t* e)
{
    static uint8_t i = 0;
    uint32_t buf[] = { PLAYER_IMG_FUDU1, PLAYER_IMG_FUDU2, PLAYER_IMG_FUDU3, PLAYER_IMG_FUDU4, PLAYER_IMG_FUDU5 };
    lv_obj_t* obj = lv_event_get_target(e);
    i = (i + 1) % 5;
    lv_img_dsc_t* pImgDsc = zb_img_res_load_one(NULL, buf[i], IMG_FREE_TYPE_AUTO, NULL);
    lv_obj_set_style_bg_img_src(obj, pImgDsc, 0);
    playFuncInfo.imgNum[PLAYER_LOOP_IMG_SN] = buf[i];
    play_para.loop_times = i + 1;
}

/**
 * 文本解析
 */
static int play_text_sentence_parse(char* path, uint16_t play_num)
{
    char buf[200] = { 0 };
    const char* pcBasePath = NULL;
    LV_LOG_USER("play_text_sentence_parse A");
#ifndef PASS_PATH
     pcBasePath = getDatumBasePath(path);
     snprintf(buf, sizeof(buf), "%s%s/%s", pcBasePath, path, play_text_book_info.pFileName[play_num]);
#else
     snprintf(buf, sizeof(buf), "%s/%s", path, play_text_book_info.pFileName[play_num]);
#endif

    
     LV_LOG_USER("buf: %s",buf);
     LV_LOG_USER("play_text_sentence_parse B");
    free_player_unit_info_t(&play_unit_info);
    LV_LOG_USER("play_text_sentence_parse C");
    int ret = player_get_unit_info(buf, &play_unit_info);
    LV_LOG_USER("play_text_sentence_parse D");
    if (ret)
    {
        // mbox = lv_msgbox_create(NULL, "警告", "数据解析错误，请按返回键播放下一首", play_text_win, false);
        if (!player_func.play_exit)
        {
            lv_label_set_text_fmt(title_label, "%s数据解析错误，请播放下一首", play_text_book_info.pTile[play_text_state.cur_play_unit_num]);
            lv_obj_clean(text_cont);
        }
        player_func.play_enable = false;
        ESP_LOGI(PLA, "%s", "data pare fail\n");
        return -1;
    }
    LV_LOG_USER("play_text_sentence_parse F");
    player_func.play_enable = true;
    return 0;
}

/**
 * @brief 文本加载， 由于有些单元数据太大，一次性不加载完。当显示到加载缓存中的最后一个时，再继续加载剩余部分
 */
static int play_text_cont_load_part_init(uint16_t playIndex)
{
#if LOAD_SENTENCE_MULT	
    return 0;
    lv_obj_t* text_orginal_label = NULL;

    lv_label_set_text(title_label, play_text_book_info.pTile[playIndex]);
    lv_obj_clean(text_cont);
    play_unit_info.load_s_no = 0;
    ESP_LOGI(PLA, "%s, [%d], play_unit_info.cnt: %d \n", __FUNCTION__, __LINE__, play_unit_info.cnt);
    if (play_unit_info.cnt)
    {
        if (play_unit_info.cnt >= LOAD_SENTENCE_NUM)
        {
            play_unit_info.load_e_no = LOAD_SENTENCE_NUM;
        }
        else
        {
            play_unit_info.load_e_no = play_unit_info.cnt;
        }
        ESP_LOGI(PLA, "%s, [%d],load_s_no: %d, load_e_no: %d \n", __FUNCTION__, __LINE__, play_unit_info.load_s_no, play_unit_info.load_e_no);
        for (int i = play_unit_info.load_s_no; i < play_unit_info.load_e_no; i++)
        {
            text_orginal_label = lv_label_create(text_cont);
            lv_obj_set_style_text_line_space(text_orginal_label, 7, 0);
            lv_obj_set_width(text_orginal_label, 320 - 24);
            lv_label_set_long_mode(text_orginal_label, LV_LABEL_LONG_WRAP);
            lv_obj_set_style_text_align(text_orginal_label, LV_TEXT_ALIGN_LEFT, 0);
            lv_obj_set_style_text_color(text_orginal_label, lv_color_hex(player_style.lyric_text_color), LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(text_orginal_label, LV_OPA_60, 0);
           
            if (play_unit_info.pTranslation[i])
            {
                if (play_unit_info.pOrginal[i])
                {
                    ESP_LOGI(PLA, "S: %d, %s, %s \n", i, play_unit_info.pOrginal[i], play_unit_info.pTranslation[i]);
                    lv_label_set_text_fmt(text_orginal_label, "%s\n%s\n", play_unit_info.pOrginal[i], play_unit_info.pTranslation[i]);
                }
                else
                {
                    lv_label_set_text_static(text_orginal_label, "NULL");
                    ESP_LOGI(PLA, "orginal sentence is null\n");
                }
            }
            else
            {
                if (play_unit_info.pOrginal[i])
                {
                    ESP_LOGI(PLA, "O: %d, %s \n", i, play_unit_info.pOrginal[i]);
                    lv_label_set_text_static(text_orginal_label, play_unit_info.pOrginal[i]);
                }
                else
                {
                    lv_label_set_text_static(text_orginal_label, "NULL");
                    ESP_LOGI(PLA, "orginal sentence is null\n");
                }
            }

            // printf("time = %d\n", play_unit_info.ptInTm[i]);
        }
    }
    else
    {
        lv_img_dsc_t* pImgDsc;
        //        lv_obj_t* img1 = lv_img_create(text_cont);
        if (play_text_book_locate.cover != -1)
        {
            pImgDsc = zb_img_res_load_one(NULL, play_text_book_locate.cover, IMG_FREE_TYPE_AUTO, NULL);
        }
        else
        {
            pImgDsc = zb_img_res_load_one(NULL, player_style.no_content_img, IMG_FREE_TYPE_AUTO, NULL);
        }
        lv_obj_set_style_bg_img_src(text_cont, pImgDsc, 0);
        //        lv_img_set_src(img1, pImgDsc);
        //        lv_obj_set_flex_align(text_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    }
    return 0;
#endif
}

/**
 * @brief 文本加载， 由于有些单元数据太大，一次性不加载完。当显示到加载缓存中的最后一个时，再继续加载剩余部分
 *  current_sentence_sn  --- 当前播放句子
 *  direction  ---- 加载检测方向,    -1 向前翻， 1 向后
 *  return 0 -- 不需要再次加载，1 --  已重新加载。
 */
static int play_text_cont_load_part_check(int16_t current_sentence_sn, int direction)
{
#if LOAD_SENTENCE_MULT	
    lv_obj_t* text_orginal_label = NULL;

    if (play_unit_info.cnt == 0)
    {
        return(0);
    }
    int start_no = play_unit_info.load_s_no;
    int end_no = play_unit_info.load_e_no;

    ESP_LOGI(PLA, "%s, [%d], current_sentence_sn: %d, direction: %d \n", __FUNCTION__, __LINE__, current_sentence_sn, direction);
    if (direction == -1)
    {
        if ((current_sentence_sn) < start_no)
        {
            start_no = current_sentence_sn;
            if (start_no + LOAD_SENTENCE_NUM < play_unit_info.cnt)
            {
                end_no = start_no + LOAD_SENTENCE_NUM;
            }
            else
            {
                end_no = play_unit_info.cnt;
            }
        }
        else
        {
            // 不用加载
            ESP_LOGI(PLA, "%s, [%d], load_s_no: %d, load_e_no: %d, 不用加载！ \n", __FUNCTION__, __LINE__, play_unit_info.load_s_no, play_unit_info.load_e_no);
            return(0);
        }
    }
    else
    {
        if (current_sentence_sn + SENTENCE_PAGE_NUM > end_no)
        {
            if (current_sentence_sn + LOAD_SENTENCE_NUM <= play_unit_info.cnt)
            {
                end_no = current_sentence_sn + LOAD_SENTENCE_NUM;
                start_no = current_sentence_sn;
            }
            else
            {
                end_no = play_unit_info.cnt;
                if (play_unit_info.cnt >= LOAD_SENTENCE_NUM)
                {
                    start_no = play_unit_info.cnt - LOAD_SENTENCE_NUM;
                }
                else
                {
                    start_no = 0;
                }
            }
        }
        else if (current_sentence_sn < start_no)  //不在原范围内
        {
            start_no = current_sentence_sn;
            if (start_no + LOAD_SENTENCE_NUM < play_unit_info.cnt)
            {
                end_no = start_no + LOAD_SENTENCE_NUM;
            }
            else
            {
                end_no = play_unit_info.cnt;
            }

        }
        else
        {
            //不用加载
            ESP_LOGI(PLA, "%s, [%d], load_s_no: %d, load_e_no: %d, 不用加载！ \n", __FUNCTION__, __LINE__, play_unit_info.load_s_no, play_unit_info.load_e_no);
            return(0);

        }
    }
    if ((play_unit_info.load_s_no == start_no) && (play_unit_info.load_e_no == end_no))
    {
        //不用加载
        ESP_LOGI(PLA, "%s, [%d], load_s_no: %d, load_e_no: %d, 不用加载！ \n", __FUNCTION__, __LINE__, play_unit_info.load_s_no, play_unit_info.load_e_no);
        return(0);
    }
    play_unit_info.load_s_no = start_no;
    play_unit_info.load_e_no = end_no;
    lv_obj_clean(text_cont);

    ESP_LOGI(PLA, "%s, [%d],load_s_no: %d, load_e_no: %d \n", __FUNCTION__, __LINE__, play_unit_info.load_s_no, play_unit_info.load_e_no);
    for (int i = play_unit_info.load_s_no; i < play_unit_info.load_e_no; i++)
    {
        text_orginal_label = lv_label_create(text_cont);
        lv_obj_set_style_text_line_space(text_orginal_label, 5, 0);
        lv_obj_set_width(text_orginal_label, 208);
        lv_label_set_long_mode(text_orginal_label, LV_LABEL_LONG_WRAP);
        if (play_unit_info.pTranslation[i])
        {
            if (play_unit_info.pOrginal[i])
            {
                ESP_LOGI(PLA, "%s [%d], S:%d, %s, %s \n", __FUNCTION__, __LINE__, i, play_unit_info.pOrginal[i], play_unit_info.pTranslation[i]);
                lv_label_set_text_fmt(text_orginal_label, "%s\n%s\n", play_unit_info.pOrginal[i], play_unit_info.pTranslation[i]);
            }
            else
            {
                lv_label_set_text_static(text_orginal_label, "NULL");
                ESP_LOGI(PLA, "orginal sentence is null\n");
            }
        }
        else
        {
            if (play_unit_info.pOrginal[i])
            {
                ESP_LOGI(PLA, "%s [%d] O: %d, %s \n", __FUNCTION__, __LINE__, i, play_unit_info.pOrginal[i]);
                lv_label_set_text_static(text_orginal_label, play_unit_info.pOrginal[i]);
            }
            else
            {
                lv_label_set_text_static(text_orginal_label, "NULL");
                ESP_LOGI(PLA, "orginal sentence is null\n");
            }
        }
        lv_obj_set_style_text_align(text_orginal_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(text_orginal_label, lv_color_hex(player_style.lyric_text_color), LV_STATE_DEFAULT);

    }
    play_text_state.pre_label = NULL; //重新加载后需要重新赋值
    play_text_state.pre_index = -1;

    return 1;
#endif
}

static void update_sentence_styles(int current_index) {
   
    for (int i = 0; i < play_unit_info.cnt; i++) {
        lv_obj_t* label = lv_obj_get_child(text_cont, i);
        if (!label) continue;

        if (i < current_index) {
            // 已播放句子样式
         //   lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
#ifndef C_PC_SIM
            lv_label_set_cjk_font(label, 20, FT_FONT_STYLE_NORMAL);
#endif
            lv_obj_set_style_text_opa(label, LV_OPA_60, 0);
        }
        else if (i == current_index) {
            //当前句子样式
          //  lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
#ifndef C_PC_SIM
            lv_label_set_cjk_font(label, 22, FT_FONT_STYLE_NORMAL);
#endif
            lv_obj_set_style_text_opa(label, LV_OPA_100, 0);
        }
        else {
            //未播放句子样式
          //  lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
#ifndef C_PC_SIM
            lv_label_set_cjk_font(label, 20, FT_FONT_STYLE_NORMAL);
#endif
            lv_obj_set_style_text_opa(label, LV_OPA_80, 0);
        }
    }
}


static void sentence_click_handler(lv_event_t* e) {
    lv_obj_t* label = lv_event_get_target(e);
    int sentence_index = (int)(intptr_t)lv_event_get_user_data(e);

    LV_LOG_USER("CLICK SENTENCE\r\n");
    // 跳转到该句子的时间点
    if (sentence_index < play_unit_info.cnt) {
        if (sentence_index != play_text_state.current_play_sentence_num)
        {
            lv_timer_resume(refresh_timer);
            // 更新当前播放句子
            play_text_state.current_play_sentence_num = sentence_index;
            uint32_t seek_time = play_unit_info.ptInTm[sentence_index];
            tz_play_set_start_end_time(seek_time, -1);

            update_sentence_styles(sentence_index);

            // 滚动到可见区域
            lv_obj_scroll_to_view(label, LV_ANIM_ON);
        }
    }
}


/**
 * @brief 文本加载，一次性加载
 */
static int play_text_cont_load(uint16_t playIndex)
{
    ESP_LOGI(TAG, "%s A", __func__);
    lv_obj_t* text_orginal_label = NULL;
    
    if (play_text_book_info.pTile[playIndex]&& title_label)
    {
        lv_label_set_text(title_label, play_text_book_info.pTile[playIndex]);
    }
    ESP_LOGI(TAG, "text_cont:%p", text_cont);
    if(text_cont) lv_obj_clean(text_cont);
   // ESP_LOGI(TAG, "%s B", __func__);
    if (play_unit_info.cnt)
    {
        ESP_LOGI(TAG, "%s C", __func__);
        for (int i = 0; i < play_unit_info.cnt; i++)
        {
           // ESP_LOGI(TAG, "%s D", __func__);
            text_orginal_label = lv_label_create(text_cont);
            lv_obj_set_style_text_line_space(text_orginal_label, 7, 0);
            lv_obj_set_width(text_orginal_label, 296);
            lv_label_set_long_mode(text_orginal_label, LV_LABEL_LONG_WRAP);
            lv_obj_set_style_text_align(text_orginal_label, LV_TEXT_ALIGN_LEFT, 0);
            lv_obj_set_style_text_color(text_orginal_label, lv_color_hex(0XFFFFFF), LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(text_orginal_label, LV_OPA_80, LV_STATE_DEFAULT);
            lv_obj_add_flag(text_orginal_label, LV_OBJ_FLAG_CLICKABLE);
#ifndef C_PC_SIM
            lv_label_set_cjk_font(text_orginal_label, 18, FT_FONT_STYLE_NORMAL);
#endif
            lv_obj_add_event_cb(text_orginal_label, sentence_click_handler, LV_EVENT_CLICKED, (void*)(intptr_t)i);
            if (play_unit_info.pTranslation[i])
            {
               // ESP_LOGI(TAG, "%s E", __func__);
                if (play_unit_info.pOrginal[i])
                {
                    lv_label_set_text_fmt(text_orginal_label, "%s\n%s\n", play_unit_info.pOrginal[i], play_unit_info.pTranslation[i]);
                }
                else
                {
                    lv_label_set_text_static(text_orginal_label, "NULL");
                    ESP_LOGI(PLA, "orginal sentence is null\n");
                }
            }
            else
            {
                //ESP_LOGI(TAG, "%s F", __func__);
                if (play_unit_info.pOrginal[i])
                {
                    lv_label_set_text_static(text_orginal_label, play_unit_info.pOrginal[i]);
                }
                else
                {
                    lv_label_set_text_static(text_orginal_label, "NULL");
                    ESP_LOGI(PLA, "orginal sentence is null\n");
                }
            }

            // printf("time = %d\n", play_unit_info.ptInTm[i]);
        }
    }
    else
    {
        ESP_LOGI(TAG, "%s G", __func__);
        lv_img_dsc_t* pImgDsc;
        if (player_bg)
        {
            LV_LOG_USER("player_bg is null \r\n");
            lv_obj_del(player_bg);
            player_bg = NULL;
        }
        player_bg = lv_img_create(play_text_win);
        if (play_text_book_locate.cover != -1)
        {
            pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_IMG, IMG_FREE_TYPE_AUTO, NULL);
        }
        else
        {
            pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_IMG, IMG_FREE_TYPE_AUTO, NULL);
        }
        lv_img_set_src(player_bg, pImgDsc);
        lv_obj_align(player_bg, LV_ALIGN_CENTER, 0, 0);
       // lv_obj_set_flex_align(text_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    }
    ESP_LOGI(TAG, "%s Z", __func__);
    return 0;
}

/**
 * @brief 获取播放当前时间
 * @param ttlTm 总时间
 * @param posTm 当前播放时间
 */
static void time_label_refresh(uint64_t ttlTm, uint64_t posTm)
{
    static uint8_t curtime, alltime;

    uint8_t second = 0;
    uint8_t minute = 0;
    posTm /= 1000; // ms to s
    second = posTm % 60;
    minute = posTm / 60;

    if (curtime == posTm)
    {
        //ESP_LOGI("time_label_refresh","curtime == posTm \r\n");
        return;
    }

    curtime = posTm;

    lv_label_set_text_fmt(current_time_label, "%02hu:%02hu", minute, second);
    ttlTm /= 1000;
    second = ttlTm % 60;
    minute = ttlTm / 60;
    lv_label_set_text_fmt(total_time_label, "%02hu:%02hu", minute, second);

    if (ttlTm && !is_user_dragging)
    {
        lv_slider_set_value(play_slider, posTm * 100 / ttlTm, LV_ANIM_OFF);
    }
}

// 在播放器中调用
void load_lrc_lyrics(const char* audio_path) {
#define MAX_PATH_LEN 256
    char lrc_path[MAX_PATH_LEN];
    snprintf(lrc_path, sizeof(lrc_path), "%s", audio_path);

    player_unit_info_t lrc_info = { 0 };
    if (lrc_parse_file(lrc_path, &lrc_info) == 0) {
        ESP_LOGI("PLAYER", "Loaded %d lyric lines", lrc_info.cnt);

        for (int i = 0;i < lrc_info.cnt; i++) {

          //  LV_LOG_USER("%s",
          //      lrc_info.pOrginal[i] ? lrc_info.pOrginal[i] : "NULL");

            LV_LOG_USER("%s\n%s",
                lrc_info.pOrginal[i],
                lrc_info.pTranslation[i]);
        }

    }
}


/**
 * @brief 切换歌曲的信息加载
 */
static int play_new_unit_info_load(void)
{
#ifndef C_PC_SIM
    //tz_play_stop_audio();
#endif

#if 0  //等融合了数据可以用
    if (current_playlist.items[play_text_state.cur_play_unit_num].source_type == MUSIC_SOURCE_TYPE_SD)
#else
LV_LOG_USER("ap_toggle_num :%d",ap_toggle_num);
#if 1
    if((!current_playlist.items)||ap_toggle_num == C_ZB_AP_TEXTBOOK_CN|| ap_toggle_num == C_ZB_AP_TEXTBOOK_ENG)
#else
    if(0)
#endif
#endif
    {
        LV_LOG_USER("path : % s, cur_play_unit_num : % d\n", play_text_book_locate.path, play_text_state.cur_play_unit_num);
        int ret = play_text_sentence_parse(play_text_book_locate.path, play_text_state.cur_play_unit_num);
        if (ret)
        {
            if (refresh_timer)
            {
                lv_timer_pause(refresh_timer);
            }
            ESP_LOGI(PLA, "sentence parse fail\n");
            return -1;
        }
        if (!player_func.play_exit)
        {
            play_text_state_init();
#if LOAD_SENTENCE_MULT
            play_text_cont_load_part_init(play_text_state.cur_play_unit_num);
#else
            play_text_cont_load(play_text_state.cur_play_unit_num);
#endif
        }
    }
    else{
        ESP_LOGI("load_lrc_lyrics", "ready enter load_lrc_lyrics\n");
#ifndef C_PC_SIM
        load_lrc_lyrics(current_playlist.items[play_text_state.cur_play_unit_num].location);   
#else
        load_lrc_lyrics("/userdata/_data/TEST_乌龟与野兔.lrc");
#endif
        ESP_LOGI("load_lrc_lyrics", "enter load_lrc_lyrics succeed\n");
    }
    lv_timer_resume(refresh_timer);
    return 0;
}

/**
 * @brief time & text refresh
 */

void text_and_time_refresh(void) 
{
    uint64_t ttlTm = 0;
    uint64_t posTm = 0;
    uint16_t index = 0;

   // TZ_PLAYSTATE_T audStat = get_play_audio_state();
   // LV_LOG_USER("get_play_audio_state :%d", player_state);


#ifndef C_PC_SIM
    if (player_state == TZ_PLAYING)
#else
    if (1)
#endif
    {

        ttlTm = g_songTmTotal;
        posTm = g_songTmCur;
       // LV_LOG_USER("text_and_time_refresh posTm:%llu, ttlTm: %llu\n", g_songTmCur, g_songTmTotal);
        play_text_state.cur_time_ps = posTm; //记录当前时间
        time_label_refresh(ttlTm, posTm);
        set_play_or_pause_btn_img(0);

        if (!current_playlist.items) //2025.06.12  目前字幕没有统一，只能先做区分
        {
            if (play_unit_info.cnt > 0)
            {
#if 1		//二分查找法

#if 1
                uint16_t left = 0, right = play_unit_info.cnt;
                int flag = 0;
                if ((int)posTm >= play_unit_info.ptInTm[play_unit_info.cnt - 1]) //防止当前时间大于最后一句时间标签
                {
                    index = play_unit_info.cnt - 1;
                    LV_LOG("%s [%d], posTm:%d last sentence.\n", __FUNCTION__, __LINE__, (int)posTm);
                }
                else
                {
                    while (left < right)
                    {
                        index = (right + left) / 2;

                        //					LV_LOG("%s [%d], posTm:%d, left: %d - %ld, right:%d - %ld, index: %d - %ld \n", __FUNCTION__, __LINE__, (int)posTm, left, play_unit_info.ptInTm[left], right,  play_unit_info.ptInTm[right], index, play_unit_info.ptInTm[index]);
                        if (((int)posTm >= play_unit_info.ptInTm[index]) && ((int)posTm < play_unit_info.ptInTm[index + 1]))
                        {
                            flag = 1;
                            break;
                        }
                        else if ((int)posTm > play_unit_info.ptInTm[index])
                        {
                            left = index;
                        }
                        else
                        {
                            right = index;
                        }

                    }

                }

                play_text_state.current_play_sentence_num = index;
                if (flag == 0)
                {
                    //				LV_LOG("%s [%d], 未找到 flag : %d.\n", __FUNCTION__, __LINE__, flag);
                  //  LV_LOG("%s [%d], current_sentence_num: %d, posTm:%d, %ld - %ld flag: %d.\n", __FUNCTION__, __LINE__, play_text_state.current_play_sentence_num, (int)posTm, play_unit_info.ptInTm[index], play_unit_info.ptInTm[index + 1], flag);

                }
                else
                {
                    //  LV_LOG("%s [%d], current_sentence_num: %d, posTm:%d, %ld - %ld flag: %d.\n", __FUNCTION__, __LINE__, play_text_state.current_play_sentence_num, (int)posTm, play_unit_info.ptInTm[index], play_unit_info.ptInTm[index + 1], flag);
                }
#endif
#else
                for (index = 1; index < play_unit_info.cnt; index++)
                {
                    if (((int)posTm >= play_unit_info.ptInTm[index - 1]) && ((int)posTm < play_unit_info.ptInTm[index]))
                    {
                        break;
                    }
                }
                play_text_state.current_play_sentence_num = index - 1;
#endif
#if LOAD_SENTENCE_MULT			
                int ret = play_text_cont_load_part_check(play_text_state.current_play_sentence_num, 1);

                if (play_text_state.current_play_sentence_num != play_text_state.pre_index)
                {
                    int text_index = play_text_state.current_play_sentence_num - play_unit_info.load_s_no;
                    lv_obj_t* obj = lv_obj_get_child(text_cont, text_index);
                    if (obj == NULL)
                    {
                        return;
                    }
                    lv_obj_scroll_to_view(obj, LV_ANIM_OFF);
                    lv_obj_set_style_text_color(obj, lv_color_hex(player_style.text_focus_color), 0);
                    if (play_text_state.pre_label)
                    {
                        lv_obj_set_style_text_color(play_text_state.pre_label, lv_color_hex(player_style.lyric_text_color), 0);
                    }
                    play_text_state.pre_label = obj;
                    play_text_state.pre_index = play_text_state.current_play_sentence_num;
                }

#else

#if 1
                if (play_text_state.current_play_sentence_num != play_text_state.pre_index)
                {

                    update_sentence_styles(play_text_state.current_play_sentence_num);

                    lv_obj_t* current_label = lv_obj_get_child(text_cont, play_text_state.current_play_sentence_num);
                    //if (current_label) {
                    //    lv_obj_set_style_text_color(current_label, lv_color_hex(0xFFD600), 0); // 高亮色
                    //    lv_obj_set_style_text_opa(current_label, LV_OPA_100, 0);
                    //}
                    if (current_label == NULL)
                    {
                        return;
                    }
                    lv_obj_scroll_to_view(current_label, LV_ANIM_OFF);
                    //  lv_timer_resume(refresh_timer);
                     // tz_play_set_start_end_time(play_unit_info.ptInTm[play_text_state.current_play_sentence_num], -1);
                      //lv_obj_set_style_text_opa(current_label, LV_OPA_100, 0);
                      //lv_obj_set_style_text_color(current_label, lv_color_hex(0XFFFFFF), 0);
                      //if (play_text_state.pre_label)
                      //{
                      //    lv_obj_set_style_text_opa(play_text_state.pre_label, LV_OPA_60, 0);
                      //    lv_obj_set_style_text_color(play_text_state.pre_label, lv_color_hex(0XFFFFFF), 0);
                      //}
                    play_text_state.pre_label = current_label;

                    play_text_state.pre_index = play_text_state.current_play_sentence_num;
                }
#endif
#endif
            }
            ab_cycle_repos(posTm, player_state);
        }
    }
#ifndef C_PC_SIM
    else if (player_state == TZ_PLAYSTOPPED)
    {
        set_play_or_pause_btn_img(1);
        if (ab_param.cycle_enable)
        {
            ab_cycle_repos(posTm, player_state);
            return;
        }
        //ESP_LOGI(TAG, "%s [%d] \n", __FUNCTION__, __LINE__);
        ab_param_clean();
    }
#endif

}






/**
 * @brief 连续播放刷新
 */
static void play_continue_func(void)
{
   
    return;
#ifdef C_PC_SIM
    player_state = TZ_PLAYING;
#endif
    if (g_player_ui_state.update_callback) {
        g_player_ui_state.update_callback(player_state);
    }

    if (player_state == play_text_state.last_state) {
        return;
    }
    LV_LOG_USER("current_state :%d last_state:%d", player_state, play_text_state.last_state);

    if ((play_text_state.last_state == TZ_PLAYSTOPPED|| play_text_state.last_state == TZ_PLAYERROR)
        && player_state == TZ_PLAYIDLE)
    {
        LV_LOG_USER("enter play_continue_func\r\n");

        // 单曲循环
        if (play_para.loopmode == LOOP_MODE_SINGLE) {
            LV_LOG_USER("loopmode:LOOP_MODE_SINGLE\r\n");
            tz_play_set_start_end_time(0, -1);
            return;
        }

        // 正常播放下一曲
        if (play_next_unit() != 0) {
            // handle_playback_error();
        }
    }
    play_text_state.last_state = player_state;
}

/**
 * @brief 播放当前时间的标签刷新
 * @param t
 */
static void refresh_timer_cb(lv_timer_t* t)
{
#if 0
    //[------ sugy 20241202 在响闹播放时, 判断(用户未操作过的情况下)播放时间是否足够久了;
    if (M_IS_PLY_ALARM_MOD() && (g_alarmTmStart != 0))
    {
        int64_t timeUsCur = zb_get_time_us_since_boot();
        if (timeUsCur >= 1000LL * (g_alarmTmStart + 1000 * 120)) {
            g_alarmTmStart = 0;
#ifndef C_PC_SIM
            if (player_func.play_enable)
            {
                TZ_PLAYSTATE_T playStat = get_play_audio_state();
                //ESP_LOGI(TAG, "%s [%d], playStat:%d \n", __FUNCTION__, __LINE__, playStat);
                if (TZ_PLAYING == playStat)
                {
                    ESP_LOGI(TAG, "alm ply reach time");
                    play_or_pause_func();
                }
            }
#endif
        }
    }
#endif

#if 1
    if (!player_func.play_exit)
    {
        text_and_time_refresh();
        //LV_LOG_USER("enter timer\r\n");
    }
#endif

      //play_continue_func();

      return;
}

static void async_update_play_button(void* data) {
#define  MMN_play         69
#define  MMN_pause         73
    AsyncUpdateData* update_data = (AsyncUpdateData*)data;

    if (!update_data->play_btn || !lv_obj_is_valid(update_data->play_btn)) {
        free(update_data);
        return;
    }

    const lv_img_dsc_t* img_src = NULL;
    switch (update_data->state) {
    case TZ_PLAYING:
        img_src = g_XXX_img_dsc_tbl + MMN_pause - 1;
        break;
    case TZ_PLAYPAUSED:
    case TZ_PLAYSTOPPED:
        img_src = g_XXX_img_dsc_tbl + MMN_play - 1;
        break;
    default: break;
    }

    if (img_src) {
        lv_img_set_src(update_data->play_btn, img_src);
    }

    free(update_data);
}

// 修改状态更新函数
void update_play_button_state(TZ_PLAYSTATE_T state, lv_obj_t* play_btn) {
    AsyncUpdateData* data = malloc(sizeof(AsyncUpdateData));
    if (!data) return;

    data->state = state;
    data->play_btn = play_btn;

    lv_async_call(async_update_play_button, data);
}


//lv_async_call(async_key_event_to_call, data);
void audio_play_timestate(TZ_PLAYSTATE_T state, int64_t current_time, int64_t audio_time)
{

    g_songTmTotal = audio_time;
    g_songTmCur = current_time;
    //time_label_refresh(g_songTmTotal, g_songTmCur);

   // printf("----------------A--------------------\r\n");
 
    /*if (player_state != state)
    {
        if (is_main_menu_active() && g_player_ui_state.home_play_btn) {
            update_play_button_state(state, g_player_ui_state.home_play_btn);
        }
    }*/

    player_state = state;

    if (ls_loopmode_play) //userly
    {
        if (curr_loopmode_play_state == state)
        {
            return;
        }
        if (curr_loopmode_play_state == TZ_PLAYERROR) //播放错误退出
        {
            return;
        }
        ESP_LOGI(PLA, "audio_state %d %lld\n", state, audio_time);
        if ((state == TZ_PLAYSTOPPED || state == TZ_PLAYERROR) && (audio_time == 0.0))
        {
            ESP_LOGI(PLA, "audio_state %d %lld\n", state, audio_time);
            // 单曲循环
            if (play_para.loopmode == LOOP_MODE_SINGLE) {
                LV_LOG_USER("loopmode:LOOP_MODE_SINGLE\r\n");
                tz_play_set_start_end_time(0, -1);
                return;
            }

            // 正常播放下一曲
            if (play_next_unit() != 0) {
                // handle_playback_error();
            }
            ESP_LOGI(PLA, "audio_state %d %lld\n", state, audio_time);
        }
        curr_loopmode_play_state = state;
    }

}

/**
 * @brief 播放和暂停功能
 * @param
 */
void play_or_pause_func(void)
{
    ESP_LOGI(PLA, "audio pause\n");
    // 添加数据检查
    if (!player_has_valid_data()) {
        LV_LOG_ERROR("No valid data to play or pause\r\n");
        return -1;
    }
        LV_LOG("%s [%d], playing_state:%d \n", __FUNCTION__, __LINE__, player_state);
        if (TZ_PLAYING == player_state)
        {
            ESP_LOGI(PLA, "audio pause1\n");

            tz_play_pause_audio();


             player_interval_flag = 0;
            lv_timer_pause(refresh_timer);
            
             set_play_or_pause_btn_img(1);
            
            g_alarmTmStart = 0;
        }
        else if (TZ_PLAYPAUSED == player_state)
        {
            ESP_LOGI(PLA, "audio playing1\n");

            tz_play_resume_audio();

            player_interval_flag = 1;
            lv_timer_resume(refresh_timer);

             set_play_or_pause_btn_img(0);
            
        }
        else // if (TZ_PLAYING != playing_state)
        {
                set_play_or_pause_btn_img(0);
            
            //ESP_LOGI(PLA, "playagin, enter aud_file_part_play, play_text_state.cur_time_ps: %ld", play_text_state.cur_time_ps);
            //int32_t startTmMs = 0;
            //if (TZ_PLAYSTOPPED == playing_state)
            //{
            //    startTmMs = play_text_state.cur_time_ps;
            //}
            ////如果是休眠后（播放器会重新初始化,状态为：MUSIC_STATUS_STOP）再次播放，需要重新在原来暂停位置播放，其它的状态就重头开始。
            //aud_file_part_play_ex(NULL, play_unit_info.fp, play_unit_info.audOffset, play_unit_info.audLen,
            //    play_unit_info.encParm, 0, play_para.speed, startTmMs);
            //player_interval_flag = 1;
            //player_state_flag = PLAYER_BACKSTAGE_RUNNING;
            //lv_timer_resume(refresh_timer);
            //set_play_or_pause_btn_img(1);
        }
}

/**
 * @brief ab参数清除
 */
static void ab_param_clean(void)
{
  //  ESP_LOGI(PLA, "%s, [%d]\n", __FUNCTION__, __LINE__);
    ab_param.cycle_enable = 0;
    ab_param.start_pos = 0;
    ab_param.end_pos = 0;
    ab_param.flag = 0;

    params.current_count = 0;
    params.interval = 0;
    params.is_repeating = false;
    params.repeat_count = 0;

    play_para.repeat_enabled = 0;
}

#define A_POS_SENTENCE_DEVIATION 1200 // A点在离句首多大范围内会被当作句首位置 
#define AB_INTERVAL_TM_MS_MIN 1000    // AB 区间最小时长 


/** AB 设置时间校正(以便于用户选句)
 *  sugy 20241203
 */
static uint32_t ab_time_adjust(uint32_t tmPos)
{
    if (play_unit_info.cnt <= 0) {  // 是不带字幕的素材,不做时间校对.
        return tmPos;
    }
    uint16_t sentenceNo = play_text_state.current_play_sentence_num;
    if (sentenceNo >= play_unit_info.cnt)
        return tmPos;
    int32_t tmSentenceSt = play_unit_info.ptInTm[sentenceNo];

    if (0 == ab_param.flag)
    {
        if (tmPos < tmSentenceSt + A_POS_SENTENCE_DEVIATION && tmPos > tmSentenceSt) {
            tmPos = tmSentenceSt;
        }
        else if (tmPos >= 80) {
            tmPos -= 80;
        }
    }
    else {
        if (tmPos < tmSentenceSt + A_POS_SENTENCE_DEVIATION && tmPos > tmSentenceSt) {
            if (tmSentenceSt >= ab_param.start_pos + AB_INTERVAL_TM_MS_MIN) {
                tmPos = tmSentenceSt;
            }
        }
    }
    return tmPos;
}

/* 静态回调函数：设置播放区间 */
static void async_set_ab_time(void* arg) {
    AsyncData* data = (AsyncData*)arg;
    if (data) {
        tz_play_set_start_end_time(data->start_time, data->end_time);
        lv_timer_resume(data->refresh_timer);
        free(data);
    }
}

/* 静态回调函数：恢复播放 */
static void timer_resume_audio(lv_timer_t* timer) {
    if (timer) {
        tz_play_resume_audio();
        lv_timer_resume(refresh_timer);
        lv_timer_del(timer);
    }
}


#if 0
static RepeatParams repeat_params;
//复读功能函数
void stop_repeating(void)
{
    if (repeat_params.timer != NULL) {
        lv_timer_del(repeat_params.timer);
        repeat_params.timer = NULL;
    }

    repeat_params.is_repeating = false;
}
#endif


/**
 * @brief ab复读功能
 * @param
 */
static void ab_cycle_func(void) 
{
    ESP_LOGI(PLA, "%s, [%d]\n", __FUNCTION__, __LINE__);
    uint32_t ttlTm = 0, posTm = 0;

    if (play_unit_info.cnt <= 0)
    {
        LV_LOG_USER("play_unit_info.cnt <= 0\r\n");
        return;
    }
    TZ_PLAYSTATE_T playStat = get_play_audio_state();
    LV_LOG_USER("playStat :%d\r\n", playStat);

        // 仅在播放状态允许设置
    if (playStat != TZ_PLAYING) {
        LV_LOG_USER("请在播放状态下设置AB点");
        return;
    }
    if (TZ_PLAYING == playStat)
    {
        if (!ab_param.flag)
        {
            initialize_repeat_params(&params, play_para.cycle_gap, play_para.loop_times);

            ab_param.tt_len = ttlTm;

            ab_param.start_pos = play_unit_info.ptInTm[play_text_state.current_play_sentence_num];

            ab_param.play_num_bak = play_text_state.cur_play_unit_num;

            if (play_text_state.cur_play_unit_num == ab_param.play_num_bak)
            {
               // ab_param.end_pos = play_text_state.current_play_sentence_num;
                LV_LOG_USER("ab_cycle_func start time :%llu\r\n", ab_param.start_pos);
                LV_LOG_USER("ab_cycle_func end time :%llu\r\n", ab_param.end_pos);

                ab_param.cycle_enable = 1;

            }
            if (ab_param.end_pos + 1 >= play_unit_info.cnt) {
                ab_param.end_pos = play_unit_info.audLen;
                LV_LOG_USER("进入复读检查 end_time: %llu", ab_param.end_pos);
            }
            else {
                ab_param.end_pos = play_unit_info.ptInTm[play_text_state.current_play_sentence_num + 1] - 200;
                LV_LOG_USER("进入复读检查 end_time: %llu", ab_param.end_pos);
            }

            ab_param.cycle_enable = 1;
            ab_param.flag = true;
          
        
        }
    }

}


/* AB复读位置处理 */
static void ab_cycle_repos(uint64_t posTm, TZ_PLAYSTATE_T audStat) {

    if (!ab_param.cycle_enable) return;

    // 播放完成或超出B点
    if (audStat == TZ_PLAYSTOPPED || posTm >= ab_param.end_pos) {
       
        if (params.repeat_count != LOOP_INFINITE &&
            ++params.current_count >= params.repeat_count)
        {
            ab_param_clean();
            LV_LOG_USER("AB复读完成");
            return;
        }

        lv_timer_pause(refresh_timer);
        // 句末暂停
        if (player_func.pause_end_enable) {
            play_or_pause_func();
            LV_LOG_USER("句末暂停中...");
        }
       
        
        /* 修改后的异步调用 */
        AsyncData* async_data = malloc(sizeof(AsyncData));
        if (async_data) {
            async_data->start_time = ab_param.start_pos;
            async_data->end_time = ab_param.end_pos;
            async_data->refresh_timer = refresh_timer;
            lv_async_call(async_set_ab_time, async_data);
            LV_LOG_USER("进入AB复读回调\r\n");
        }



        // 间隔时间
        if (params.interval > 0) {
            LV_LOG_USER("进入AB复读的设置间隔时间\r\n");
            tz_play_pause_audio();
            lv_timer_pause(refresh_timer);
            lv_timer_t* resume_timer = lv_timer_create(timer_resume_audio,
                params.interval,
                NULL);
            lv_timer_set_repeat_count(resume_timer, 1); // 设置单次执行
        }
    }

}
static int calculate_prev_index(int total)
{
    if (total <= 0) return 0; 

    int prev_index = play_text_state.cur_play_unit_num;

    switch (play_para.loopmode) {
    case LOOP_MODE_ORDER:
        prev_index--; 
        break;

    case LOOP_MODE_RANDOM: {
        if (total <= 1) break;
        int new_random;
        do {
            new_random = rand() % total;
        } while (new_random == play_para.last_random);
        prev_index = play_para.last_random = new_random;
        break;
    }

    case LOOP_MODE_SINGLE:
 
        break;

    case LOOP_MODE_ALL:
    default:
        prev_index--; // 循环模式递减
        break;
    }

    /* 统一越界处理 */
    if (prev_index < 0) {
        prev_index += total; // 负数时回绕到末尾
    }
    prev_index %= total;    

    LV_LOG_USER("Prev index: %d/%d (mode:%d)",
        prev_index, total - 1, play_para.loopmode);

    return prev_index;
}


static int calculate_next_index(int total_items)
{
    if (total_items <= 0) return 0;

    int next_index = play_text_state.cur_play_unit_num;

    switch (play_para.loopmode) {
    case LOOP_MODE_ORDER:
        next_index++;
        break;

    case LOOP_MODE_RANDOM: {
        if (total_items <= 1) break;
        int new_random;
        do {
            new_random = rand() % total_items;
        } while (new_random == play_para.last_random);
        next_index = play_para.last_random = new_random;
        break;
    }

    case LOOP_MODE_SINGLE:
        // 保持当前索引不变
        break;

    case LOOP_MODE_ALL:
    default:
        next_index++;
        break;
    }

    /* 统一越界处理 */
    if (next_index >= total_items) {
       // if(play_para.loopmode == LOOP_MODE_ORDER) 
        next_index = 0;  // 超过最大值时重置为0
    }

    LV_LOG_USER("Next index: %d/%d (mode:%d)",
        next_index, total_items - 1, play_para.loopmode);

    return next_index;
}


//======================================================================
/**
 * @brief 上一首播放，最近播放和收藏模块特殊处理。
 * @param
 */
 //======================================================================
int play_pre_unit(void)
{
    ESP_LOGI("play_pre_unit", "ready prev music\r\n ");
    // 添加数据检查
    if (!player_has_valid_data()) {
        LV_LOG_ERROR("No valid data to play next");
        return -1;
    }
    int num = 0;
    music_count = play_text_book_info.cnt;

    num = calculate_prev_index(music_count);
    lv_timer_resume(refresh_timer);

    play_media_unit(num);

}
//======================================================================
/**
 * @brief 下一首播放, 最近播放和收藏模块特殊处理。
 * @param
 */
 //======================================================================
int play_next_unit(void)
{
    ESP_LOGI("play_next_unit","ready next music\r\n ");
    // 添加数据检查
    if (!player_has_valid_data()) {
        LV_LOG_ERROR("No valid data to play next");
        return -1;
    }
    int num = 0;
    music_count = play_text_book_info.cnt;

    num = calculate_next_index(music_count);

    ESP_LOGI("play_next_unit", "---------------C------------\\r\n");
    return play_media_unit(num);
    ESP_LOGI("play_next_unit", "---------------D------------\\r\n");

}

/**
 * @brief 快进功能
 * @param
 */
static void fast_forward_unit(void)
{
    uint32_t ttlTm = 0, posTm = 0;
    int newPosTm = 0;
    ESP_LOGI("player", "%s [%d] ,", __FUNCTION__, __LINE__);
#ifndef C_PC_SIM
    //TZ_PLAYSTATE_T audStat = get_play_audio_state();
    //audio_stream_get_play_time(&ttlTm, &posTm);
    //int fw_step = ((ttlTm / 20) > 3000) ? (ttlTm / 20) : 3000;  //按百分之5%
    ////    if ((int)posTm + MY_AUD_FOR_BACK_DIFF < play_unit_info.audLen)
    //if ((int)posTm + fw_step < play_unit_info.audLen)
    //{
    //    newPosTm = posTm + fw_step;
    //    audio_stream_player_seek(newPosTm);

    //}
    //else
    //{
    //    newPosTm = posTm;
    //}
    //if (MUSIC_STATUS_PAUSE == audStat)
    //{
    //    tz_play_resume_audio();
    //    player_interval_flag = 1;
    //    lv_timer_resume(refresh_timer);
    //    set_play_or_pause_btn_img(1);

    //}
#endif
}

/**
 * @brief 快退功能
 * @param
 */
static void fast_backward_unit(void)
{
    uint32_t ttlTm = 0, posTm = 0;
    int newPosTm = 0;
    int bw_step = 0;
#ifndef C_PC_SIM
    //TZ_PLAYSTATE_T audStat = get_play_audio_state();
    //ESP_LOGI("player", "%s [%d] ,", __FUNCTION__, __LINE__);
    //audio_stream_get_play_time(&ttlTm, &posTm);
    //bw_step = ((ttlTm / 20) > 3000) ? (ttlTm / 20) : 3000;
    ////    if ((int)posTm >= MY_AUD_FOR_BACK_DIFF)

    //if ((int)posTm >= bw_step)
    //{
    //    newPosTm = posTm - bw_step;

    //}
    //else
    //{
    //    newPosTm = 0;
    //}
    //audio_stream_player_seek(newPosTm);
    //if (MUSIC_STATUS_PAUSE == audStat)
    //{
    //    tz_play_resume_audio();
    //    player_interval_flag = 1;
    //    lv_timer_resume(refresh_timer);
    //    set_play_or_pause_btn_img(1);

    //}

#endif
}

/**
 * @brief 上一句播放
 * @param
 */
static void play_pre_sentence(void)
{
#ifndef C_PC_SIM
    TZ_PLAYSTATE_T audStat = get_play_audio_state();
    if (audStat == TZ_PLAYING || audStat == MUSIC_STATUS_PAUSE)
    {
        if (play_unit_info.cnt)
        {
            if (play_text_state.current_play_sentence_num > 0)
            {
                play_text_state.current_play_sentence_num = (play_text_state.current_play_sentence_num - 1 + play_unit_info.cnt) % play_unit_info.cnt;
#if LOAD_SENTENCE_MULT
                play_text_cont_load_part_check(play_text_state.current_play_sentence_num, -1);
#endif
                lv_timer_resume(refresh_timer);
                tz_play_resume_audio();
                audio_stream_player_seek(play_unit_info.ptInTm[play_text_state.current_play_sentence_num]);

                if (audStat == MUSIC_STATUS_PAUSE)
                {
                    set_play_or_pause_btn_img(1);
                }

            }
        }
        else
        {
            fast_backward_unit();
        }
    }
#endif
}

/**
 * @brief 下一句播放
 * @param
 */
static void play_next_sentence(void)
{
#ifndef C_PC_SIM
    TZ_PLAYSTATE_T audStat = get_play_audio_state();
    if (audStat == TZ_PLAYING || audStat == MUSIC_STATUS_PAUSE)
    {
        if (play_unit_info.cnt)
        {
            if (play_text_state.current_play_sentence_num < play_unit_info.cnt)
            {
                play_text_state.current_play_sentence_num = (play_text_state.current_play_sentence_num + 1) % play_unit_info.cnt;
#if LOAD_SENTENCE_MULT
                play_text_cont_load_part_check(play_text_state.current_play_sentence_num, 1);
#endif
                lv_timer_resume(refresh_timer);
                tz_play_resume_audio();
                tz_play_set_start_end_time(play_unit_info.ptInTm[play_text_state.current_play_sentence_num],-1);
                if (audStat == MUSIC_STATUS_PAUSE)
                {
                    set_play_or_pause_btn_img(1);
                }
            }
        }
        else
        {
            fast_forward_unit();
        }
    }
#endif
}

/**
 * @brief 主窗口key事件回调
 * @param e
 */
static void play_text_win_key_cb(lv_event_t* e)
{
    int keyVal = 0;
    int total_btn_num = M_ARRAY_LEN(play_func_btn_tl);


    lv_event_code_t evtCode = e->code;
    keyVal = lv_event_get_key(e);
    ESP_LOGI("player", "%s [%d] ,keyVal:%x ", __FUNCTION__, __LINE__, keyVal);
    if (evtCode == LV_EVENT_KEY_STAGE)
    {
        // 长按
        if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_LONG)
        {
#if 0
            keyVal &= ~LV_EX_FLAG_KEY_MASK;
            switch (keyVal)
            {
            }
#endif
        }
        else if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_REPEAT)  //重复按键
        {
#if 0
            keyVal &= ~LV_EX_FLAG_KEY_MASK;
            switch (keyVal)
            {

            }
#endif
        }
        // 短按
        else if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_SHORT)
        {
            keyVal &= ~LV_EX_FLAG_KEY_MASK;
            switch (keyVal)
            {
            case LV_KEY_ENTER: // 播放或暂停
            {
#if 0
                lv_indev_keypad_wait_release();
                LV_LOG("%s [%d],focus: %p, play_text_win:%p\n", __FUNCTION__, __LINE__, lv_group_get_focused(lv_group_get_default()), play_text_win);
                if (lv_group_get_focused(lv_group_get_default()) == play_text_win ||
                    (play_text_group != NULL && lv_group_get_focused(play_text_group) == play_text_win)) // sugy 240716 add group check nul.
                {
                    LV_LOG("%s [%d], play_text_group: %p \n", __FUNCTION__, __LINE__, lv_group_get_focused(play_text_group));
                    play_or_pause_func();
                }
#endif
                break;
            }
            case LV_KEY_END:
            case LV_KEY_EX_FUNC1: // ab复读
            {
                lv_indev_keypad_wait_release();
                LV_LOG(" LV_KEY_EX_FUNC1\n");
#if 0
                ab_cycle_func();
#endif

                break;
            }
            case     LV_KEY_EX_MEDIA_PREV:
                lv_indev_keypad_wait_release();
                LV_LOG_USER("PRESS LV_KEY_EX_MEDIA_PREV \r\n");
                play_pre_unit();

                break;
            case    LV_KEY_EX_MEDIA_NEXT:
                lv_indev_keypad_wait_release();
                LV_LOG_USER("PRESS LV_KEY_EX_MEDIA_NEXT\r\n");
                play_next_unit();

                
                break;
#if 0
            case   LV_KEY_EX_MEDIA_PLAY_PAUSE:
                lv_indev_keypad_wait_release();
                LV_LOG_USER("PRESS LV_KEY_EX_MEDIA_PLAY_PAUSE\r\n");
                play_or_pause_func();

                break;
#endif
            }
        }
    }
    else if (LV_EVENT_KEY == evtCode)
    {
        switch (keyVal)
        {
        case LV_KEY_ESC:
        {
            LV_LOG_USER("------------------------\r\n");
            if (ap_toggle_num == -1)
            {
                lv_indev_keypad_wait_release();
                play_text_win_free_keep();
                //play_main_page_init(player_class_num);
                if (MMN_ENTER_STAT)
                {
                    MMN_ENTER_STAT = false;
                    zb_requestSwitchAp(C_ZB_AP_MAINMENU);
                }
                else
                {
                    lv_timer_create(zb_ex_cmd_timer_cb, 0, (void*)PLY_CMD_PLY_LST_RET);
                    LV_LOG_USER("click cmd return event\r\n");
                }
                
            }
            else
            {
                lv_indev_keypad_wait_release();
                //tz_play_stop_audio();
                //free_music_file_data();
                play_list_page_free();
#if 0
                //play_text_page_free();
#else
                play_text_win_free_keep();
#endif
                if (MMN_ENTER_STAT)
                {
                    MMN_ENTER_STAT = false;
                    zb_requestSwitchAp(C_ZB_AP_MAINMENU);
                }
                else
                {
                    lv_timer_create(zb_ex_cmd_timer_cb, 0, (void*)PLY_CMD_PLY_LST_RET);
                }
                ap_toggle_num = -1;
               
            }
            break;
        }


#if 0
        case LV_KEY_HOME:
        {
            lv_indev_keypad_wait_release();
            if ((!play_unit_info.cnt) || (!player_func.record_enable))
            {
                playlist_win_del();
                break;
            }

#ifndef C_PC_SIM
            tz_play_stop_audio();
#endif
            play_text_win_free_for_internal();
            play_repeat_page_init(0);
            break;
        }
#endif
        }
    }
}


static void play_text_win_del_cb(lv_event_t* e)
{
    int keyVal = 0;

    lv_event_code_t evtCode = e->code;
    keyVal = lv_event_get_key(e);
    if (evtCode == LV_EVENT_DELETE) {
#if 0  //wsy 25.06.11 
        if (refresh_timer) {
            lv_timer_del(refresh_timer);  refresh_timer = NULL;
        }
#endif
        if (play_text_group != NULL) {  // sugy 240716 move zb_group_pop here.
            zb_group_pop(play_text_group);   play_text_group = NULL;
        }
    }
}

/**
 * @brief 播放界面及资源释放
 */
int play_text_page_free(void)
{
    LV_LOG_USER("%s[%d]\n", __FUNCTION__, __LINE__);
#ifndef C_PC_SIM
    //tz_play_stop_audio();
    player_state_flag = PLAYER_BACKSTAGE_STOP;
#endif
    if (NULL == play_text_win) {
        dictScreen_del(); // 销毁跳查版面 
    }

    ab_param_clean();
    free_player_book_info_t(&play_text_book_info);
    free_player_unit_info_t(&play_unit_info);
    if (play_text_win)
    {
        lv_obj_del(play_text_win);
        play_text_win = NULL;
    }
    LV_LOG_USER("delete refresh_timer");

    if (dialog != NULL)
    {
        lv_obj_del(dialog);
        dialog = NULL;

    }
    if (play_repeat_win != NULL) //如果在跟读录音版面，销毁此版面。
    {
        play_repeat_page_free();
    }
    playlist_win_del();  //播放列表
    player_backstage_flag = 0;
    return 0;
}

/**
 * @brief 仅界面的释放播放内部使用,进入词典跳查，录音评测时使用,不清除AB复读状态
 */
static int play_text_win_free_for_internal(void)
{
#ifndef C_PC_SIM
    //   tz_play_stop_audio();
#endif
    ESP_LOGI(TAG, "%s [%d] \n", __FUNCTION__, __LINE__);
    if (ab_param.flag < 2)//如果AB复读设置不完整，也清除掉。如果设置完整就保留。
    {
        ab_param_clean();
    }

    if (play_text_win)
    {
        ESP_LOGI(TAG, "%s [%d] \n", __FUNCTION__, __LINE__);
        lv_obj_del(play_text_win);
        play_text_win = NULL;
    }
    // printf("delete refresh_timer2\n");
    if (refresh_timer)
    {
        ESP_LOGI(TAG, "%s [%d] \n", __FUNCTION__, __LINE__);
        lv_timer_del(refresh_timer);
        refresh_timer = NULL;
    }
    if (dialog != NULL)
    {
        lv_obj_del(dialog);  dialog = NULL;

    }
    return 0;
}

/**
 * @brief 仅界面的释放
 */
int play_text_win_free(void)
{
#ifndef C_PC_SIM
    tz_play_stop_audio();
#endif
    ESP_LOGI(TAG, "%s [%d] \n", __FUNCTION__, __LINE__);
    ab_param_clean();
    if (player_bg)
    {
        lv_obj_del(player_bg);
        player_bg = NULL;
    }
    if (play_text_win)
    {
        ESP_LOGI(TAG, "%s [%d] \n", __FUNCTION__, __LINE__);
        lv_obj_del(play_text_win);
        play_text_win = NULL;
    }
    // printf("delete refresh_timer2\n");
#if 0
    if (refresh_timer)
    {
        ESP_LOGI(TAG, "%s [%d] \n", __FUNCTION__, __LINE__);
        lv_timer_del(refresh_timer);
        refresh_timer = NULL;
    }
#endif
    if (dialog != NULL)
    {
        lv_obj_del(dialog);  dialog = NULL;

    }
    return 0;
}

/**
 * @brief 界面的释放但保持播放
 */
int play_text_win_free_keep(void)
{
    ESP_LOGI(TAG, "%s [%d] \n", __FUNCTION__, __LINE__);
    ab_param_clean();
    if (player_bg)
    {
        lv_obj_del(player_bg);
        player_bg = NULL;
    }
    if (play_text_win)
    {
        lv_obj_del(play_text_win);
        play_text_win = NULL;
    }
    if (dialog != NULL)
    {
        lv_obj_del(dialog);  dialog = NULL;

    }
    player_func.play_exit = true;
    return 0;
}




lv_obj_t* player_logo_scr = NULL;


static void slider_event_handler(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    switch (code) {
    case LV_EVENT_PRESSED:
        is_user_dragging = true;
        LV_LOG_USER("press slider\r\n");
        break;

    case LV_EVENT_RELEASED:
    case LV_EVENT_PRESS_LOST:
        is_user_dragging = false;
        // 跳转逻辑放在释放时触发（更友好）
        if (code == LV_EVENT_RELEASED) {
            int16_t val = lv_slider_get_value(slider);

            uint64_t seek_time = ((uint64_t)g_songTmTotal * val) / 100;
            g_songTmCur = seek_time;
#ifndef   C_PC_SIM
            lv_timer_resume(refresh_timer);
            tz_play_set_start_end_time((uint64_t)seek_time, g_songTmTotal);

            time_label_refresh(g_songTmTotal, seek_time);
#endif
            LV_LOG_USER("Seek to: %d%% (%llu ms)", val, seek_time);
        }
        break;

    case LV_EVENT_VALUE_CHANGED:
        if (is_user_dragging) {
            // 实时更新拖动时的显示（不触发实际跳转）
            int16_t val = lv_slider_get_value(slider);
            uint64_t show_time = ((uint64_t)g_songTmTotal * val) / 100;
            time_label_refresh(g_songTmTotal, show_time);
            LV_LOG_USER("Dragging position: %d%%", val);
        }
        break;
    }
}
bool stat_init = 0;



static void label_draw_part_event_cb(lv_event_t* e) {
    lv_obj_t* label = lv_event_get_target(e);
    lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);

    if (dsc->part != LV_PART_MAIN) return;

    uint64_t* p_time_ms = (uint64_t*)lv_obj_get_user_data(label);
    if (p_time_ms == NULL) {

        lv_draw_label(dsc->draw_ctx, &dsc->label_dsc, &dsc->draw_area, "00:00", NULL);
        dsc->label_dsc = NULL; 
        return;
    }

    uint64_t time_ms = *p_time_ms;
    uint64_t total_seconds = time_ms / 1000;
    int minutes = total_seconds / 60;  
    int seconds = total_seconds % 60;

    char time_buf[6];
    snprintf(time_buf, sizeof(time_buf), "%02d:%02d", minutes, seconds);

  
    lv_draw_label(dsc->draw_ctx, &dsc->label_dsc, &dsc->draw_area, time_buf, NULL);
    dsc->label_dsc = NULL;  
}




/**
 * @brief 仅页面初始化
 */
void play_text_win_init(void* parm)
{
    g_player_scrNo = 0;
#define btn_move_y 5
    if (play_text_win)
    {
        lv_obj_del(play_text_win);
        play_text_win = NULL;
    }

    play_text_win = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(play_text_win);
    lv_obj_set_size(play_text_win, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_pad_all(play_text_win, 0, 0);
    lv_obj_clear_flag(play_text_win, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_style_bg_color(play_text_win, lv_color_hex(0x1A143B), 0);
     // lv_obj_add_event_cb(play_text_win, play_text_win_key_cb, LV_EVENT_KEY, NULL);

    lv_obj_add_event_cb(play_text_win, play_text_win_key_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(play_text_win, play_text_win_del_cb, LV_EVENT_DELETE, NULL);
    sys_setCurMainWinAndCb(play_text_win, play_text_win_key_cb);
    play_func_btn_tl[PLAYER_TEXT_WIN_SN] = play_text_win;

    //背景
    lv_obj_t* img1 = lv_img_create(play_text_win);
    lv_img_dsc_t* pImgDsc = zb_img_res_load_one(NULL, player_style.page_bg_img2, IMG_FREE_TYPE_AUTO, NULL);
    lv_img_set_src(img1, pImgDsc);

    // 标题
    title_label = lv_label_create(play_text_win);
    lv_obj_set_size(title_label, 170, 30);
    lv_label_set_text(title_label, "");
#ifndef C_PC_SIM
    lv_label_set_cjk_font(title_label, 20, FT_FONT_STYLE_NORMAL);
#endif
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_DOT);//LV_LABEL_LONG_SCROLL_CIRCULAR
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, -10, 15);
    // lv_obj_set_style_text_font(title_label, FONT_TITLE, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);


    lv_obj_t* player_return = lv_obj_create(play_text_win);
    lv_obj_set_size(player_return, btn_size, btn_size);
    lv_obj_set_style_radius(player_return, 17, 0);
    lv_obj_set_style_bg_color(player_return, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(player_return, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(player_return, 0, 0);
    lv_obj_add_style(player_return, get_global_style(STYLE_BTN_BASE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_img_dsc_t* return_pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_RETURN, IMG_FREE_TYPE_AUTO, NULL);
    lv_obj_set_style_bg_img_src(player_return, return_pImgDsc, 0);
    lv_obj_align(player_return, LV_ALIGN_TOP_LEFT, 0, btn_move_y);
    lv_obj_add_event_cb(player_return, playFuncInfo.event_cb[6], LV_EVENT_CLICKED, NULL);//back_btn_click_cb

    lv_obj_t* menu = lv_obj_create(play_text_win);
    lv_obj_set_size(menu, btn_size, btn_size);
    lv_obj_set_style_radius(menu, 17, 0);
    lv_obj_set_style_bg_color(menu, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(menu, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(menu, 0, 0);
    lv_obj_add_style(menu, get_global_style(STYLE_BTN_BASE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_img_dsc_t* menu_pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_MENU, IMG_FREE_TYPE_AUTO, NULL);
    lv_obj_set_style_bg_img_src(menu, menu_pImgDsc, 0);
    lv_obj_align(menu,LV_ALIGN_TOP_RIGHT, 0, btn_move_y);
    lv_obj_add_event_cb(menu, playFuncInfo.event_cb[5], LV_EVENT_CLICKED, NULL);

    col_btn = lv_obj_create(play_text_win);
    lv_obj_set_size(col_btn, btn_size, btn_size);
    lv_obj_set_style_radius(col_btn, 17, 0);
    lv_obj_set_style_bg_color(col_btn, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(col_btn, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(col_btn, 0, 0);
    lv_img_dsc_t* col_img = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_UNCOL, IMG_FREE_TYPE_AUTO, NULL);
    lv_obj_add_style(col_btn, get_global_style(STYLE_BTN_BASE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(col_btn, col_img, 0);
    lv_obj_align_to(col_btn, menu, LV_ALIGN_OUT_LEFT_MID, 0, 0);
    lv_obj_add_event_cb(col_btn, playFuncInfo.event_cb[7], LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(col_btn, playFuncInfo.event_cb[7], LV_EVENT_PRESSED, NULL);



    if (current_playlist.items)//lowndoad_player_test
    {
        lv_obj_t* player_img = lv_img_create(play_text_win);
        lv_obj_align(player_img, LV_ALIGN_CENTER, 0, 0);

        lv_img_dsc_t* pImgDsc_img = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_IMG, IMG_FREE_TYPE_AUTO, NULL);
        lv_img_set_src(player_img, pImgDsc_img);
    }
#if 0
    else if (0)
    {

        player_logo_scr = lv_obj_create(play_text_win);
        lv_obj_remove_style_all(player_logo_scr);
        lv_obj_set_style_bg_color(player_logo_scr, lv_color_hex(0x252276), 0);
        lv_obj_set_size(player_logo_scr, lv_pct(100), lv_pct(50));
        lv_obj_set_style_pad_all(player_logo_scr, 0, 0);
        lv_obj_set_scroll_snap_x(player_logo_scr, LV_SCROLL_SNAP_CENTER);
        lv_obj_set_flex_flow(player_logo_scr, LV_FLEX_FLOW_ROW);
        lv_obj_set_scroll_dir(player_logo_scr, LV_DIR_HOR);
        lv_obj_align(player_logo_scr, LV_ALIGN_CENTER, 0, 0);

        lv_obj_t* screen1 = lv_obj_create(player_logo_scr);
        lv_obj_t* screen2 = lv_obj_create(player_logo_scr);

        lv_obj_set_size(screen1, lv_pct(100), lv_pct(100));
        lv_obj_set_size(screen2, lv_pct(100), lv_pct(100));
        lv_obj_clear_flag(screen1, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(screen2, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(screen1, lv_color_hex(0x252276), 0);
        lv_obj_set_style_bg_color(screen2, lv_color_hex(0x252276), 0);
        lv_obj_set_style_border_width(screen2, 0, 0);
        lv_obj_set_style_border_width(screen1, 0, 0);
        lv_obj_t* player_img = lv_img_create(screen1);
        lv_obj_align(player_img, LV_ALIGN_CENTER, 0, 0);

        lv_img_dsc_t* pImgDsc_img = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_IMG, IMG_FREE_TYPE_AUTO, NULL);
        lv_img_set_src(player_img, pImgDsc_img);

        col_btn = lv_btn_create(player_img);
        lv_obj_set_size(col_btn, btn_size, btn_size);
        lv_obj_set_style_radius(col_btn, 17, 0);
        lv_obj_set_style_bg_color(col_btn, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_bg_opa(col_btn, LV_OPA_0, 0);
        lv_obj_set_style_shadow_width(col_btn, 0, 0);
        lv_img_dsc_t* col_img = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_UNCOL, IMG_FREE_TYPE_AUTO, NULL);
        lv_obj_set_style_bg_img_src(col_btn, col_img, 0);
        lv_obj_align_to(col_btn, player_img, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
        lv_obj_add_event_cb(col_btn, playFuncInfo.event_cb[7], LV_EVENT_CLICKED, NULL);



        text_cont = lv_obj_create(screen2);
        lv_obj_set_scrollbar_mode(text_cont, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_size(text_cont, lv_pct(100), lv_pct(100));
        lv_obj_align_to(text_cont, screen2, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_pad_left(text_cont, 12, 0);
        lv_obj_set_style_bg_opa(text_cont, LV_OPA_0, LV_STATE_DEFAULT);
        lv_obj_set_style_border_opa(text_cont, LV_OPA_0, LV_STATE_DEFAULT);
        lv_obj_set_flex_flow(text_cont, LV_FLEX_FLOW_COLUMN);
    }
#endif
    else
    {
        text_cont = lv_obj_create(play_text_win);
        lv_obj_set_scrollbar_mode(text_cont, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_size(text_cont, lv_pct(100), lv_pct(65));
        lv_obj_align(text_cont, LV_ALIGN_CENTER, 0, -30);
        lv_obj_add_flag(text_cont, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_pad_left(text_cont, 12, 0);
        lv_obj_set_style_bg_opa(text_cont, LV_OPA_0, LV_STATE_DEFAULT);
        lv_obj_set_style_border_opa(text_cont, LV_OPA_0, LV_STATE_DEFAULT);
        lv_obj_set_flex_flow(text_cont, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_scroll_dir(text_cont, LV_DIR_VER);  // 垂直滚动
        if (ap_toggle_num != C_ZB_AP_TEXTBOOK_ENG) lv_obj_set_style_pad_row(text_cont, 50, LV_PART_MAIN);
    }

    // 播放进度条
    play_slider = lv_slider_create(play_text_win);
    lv_obj_set_style_bg_color(play_slider, lv_color_hex(0xffffff), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(play_slider, lv_color_hex(0xffffff), LV_PART_KNOB);
    lv_obj_set_style_bg_color(play_slider, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(play_slider, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_size(play_slider, 200, 6);
    lv_obj_align_to(play_slider, play_text_win, LV_ALIGN_BOTTOM_MID, 0, -80);

    lv_slider_set_range(play_slider, 0, 100);
    lv_obj_add_event_cb(play_slider, slider_event_handler, LV_EVENT_ALL, NULL);

    // 播放当前时间
    current_time_label = lv_label_create(play_text_win);
    lv_label_set_text(current_time_label, "00:00");
    lv_obj_set_style_text_font(current_time_label, FONT_BODY, 0);
    lv_obj_set_style_text_color(current_time_label, lv_color_hex(0xffffff), 0);
    lv_obj_align_to(current_time_label, play_slider, LV_ALIGN_OUT_LEFT_MID, -8, 0);

    // 播放当总时间
    total_time_label = lv_label_create(play_text_win);
    lv_label_set_text_fmt(total_time_label, "00:00");
    lv_obj_set_style_text_font(total_time_label, FONT_BODY, 0);
    lv_obj_set_style_text_color(total_time_label, lv_color_hex(0xffffff), 0);
    lv_obj_align_to(total_time_label, play_slider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    //lv_obj_set_user_data(current_time_label, &g_songTmCur); // 设置用户数据为当前时间的地址
   // lv_obj_add_event_cb(current_time_label, label_draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

   // lv_obj_set_user_data(total_time_label, &g_songTmTotal); // 设置用户数据为总时间的地址
   // lv_obj_add_event_cb(total_time_label, label_draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

    lv_obj_t* play_function_btn;
    lv_obj_t* play_function_label;
    for (int i = 1; i < playFuncInfo.elementNum; i++) //todo 暂时去掉跟读  原为0
    {

#if 1
        play_function_btn = lv_obj_create(play_text_win);
        play_func_btn_tl[i + 1] = play_function_btn;
        lv_obj_set_size(play_function_btn, 60, 75);
#if 1
        lv_obj_set_style_shadow_width(play_function_btn, 0, 0);
        lv_obj_set_style_bg_color(play_function_btn, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_bg_opa(play_function_btn, LV_OPA_0, 0);
        lv_obj_add_style(play_function_btn, get_global_style(STYLE_BTN_BASE), LV_PART_MAIN | LV_STATE_DEFAULT);
#else
        lv_obj_add_style(play_function_btn, get_global_style(STYLE_BTN_OPA), LV_PART_MAIN | LV_STATE_DEFAULT);
#endif
        lv_img_dsc_t* pImgDsc = NULL;

        pImgDsc = zb_img_res_load_one(NULL, playFuncInfo.imgNum[i], IMG_FREE_TYPE_AUTO, NULL);

        //lv_obj_set_style_bg_img_ofs_y(play_function_btn, -15, 0);
        //lv_style_set_image_offset_y(&btn_img_style, -10);
       // lv_img_set_offset_y(pImgDsc, -10);
        lv_obj_set_style_bg_img_src(play_function_btn, pImgDsc, 0);


        lv_obj_add_event_cb(play_function_btn, playFuncInfo.event_cb[i], LV_EVENT_CLICKED, NULL);
        lv_obj_set_pos(play_function_btn, playFuncInfo.offset_x * i, 248 + playFuncInfo.offset_y);
#else
        play_function_btn = lv_img_create(play_text_win);
        play_func_btn_tl[i + 1] = play_function_btn;
        const lv_img_dsc_t* pImgDsc = zb_img_res_load_one(NULL, playFuncInfo.imgNum[i], IMG_FREE_TYPE_AUTO, NULL);
        lv_img_set_src(play_function_btn, pImgDsc);
        lv_obj_set_size(play_function_btn, pImgDsc->header.w + 10 * 2, pImgDsc->header.h + 10 * 2);
        //lv_obj_set_pos(play_function_btn, 16, STNG_ESC_BTN_LOC_Y);
        lv_obj_set_style_border_width(play_function_btn,12, 0);
        lv_obj_set_style_radius(play_function_btn, 4, 0);
        lv_obj_add_flag(play_function_btn, LV_OBJ_FLAG_CLICKABLE);
        
        lv_obj_set_style_opa(play_function_btn, LV_OPA_60, 0);
        lv_obj_set_style_opa(play_function_btn, LV_OPA_100, LV_STATE_FOCUS_KEY);
        lv_obj_set_style_opa(play_function_btn, LV_OPA_80, LV_STATE_PRESSED);
        // lv_color_t colorBorderFcs = lv_color_white();
        lv_obj_set_style_border_opa(play_function_btn, LV_OPA_50, 0);
        lv_obj_set_pos(play_function_btn, playFuncInfo.offset_x* i, 252 + playFuncInfo.offset_y);
        lv_obj_add_event_cb(play_function_btn, playFuncInfo.event_cb[i], LV_EVENT_CLICKED, NULL);
#endif
    }

    refresh_timer = lv_timer_create(refresh_timer_cb, 220, NULL);
    lv_timer_resume(refresh_timer);


  //  lv_obj_add_event_cb(play_text_win, scroll_end_cb, LV_EVENT_SCROLL_END, NULL);
   
}

/**
 * @brief 专辑播放功能配置
 */
void play_normal_config(void)
{
    player_func.pre_unit_enable = true;
    player_func.next_unit_enable = true;
    player_func.play_continu_enable = true;
    player_func.record_enable = true;
    player_func.play_exit = false;
    player_func.find_enable = true;
}

/**
 * @brief 专辑播放功能配置
 */
void play_album_config(void)
{
    player_func.pre_unit_enable = true;
    player_func.next_unit_enable = true;
    player_func.play_continu_enable = true;
    player_func.record_enable = false;
    player_func.play_exit = false;
    player_func.find_enable = false;
}

/**
 * @brief 单曲播放功能配置
 */
void play_single_config(void)
{
    player_func.pre_unit_enable = false;
    player_func.next_unit_enable = false;
    player_func.play_continu_enable = false;
    player_func.record_enable = false;
    player_func.play_exit = false;
    player_func.find_enable = false;
}


/**
 * @brief 最近播放功能配置
 */
void play_recently_or_collect_config(void)
{
    player_func.pre_unit_enable = true;  //播放下一个记录，不是书中的下一篇
    player_func.next_unit_enable = true;
    player_func.play_continu_enable = true;
    player_func.record_enable = false;
    player_func.play_exit = false;
    player_func.find_enable = true;
}


/**
 * @brief 播放页面初始化 (内置数据接口)
 * @param
 *  parm:1 创建新输入组, 0 仍使用默认组
 *  extra:1 保留后台播放，0 销毁后台播放
 * @return
 *  0:正常创建
 *  <0:创建异常
 */
int play_text_page_init(int parm, int extra)  
{
        free_global_playlist();
        ESP_LOGI(TAG, "%s A", __func__);
#if 0
        //[------- sugy 20240724 add for 学情统计 
        int16_t listenArticlesCnt = getPrjGlobalParm(ZB_PARM_LISTEN_ARTICLES, 0, NULL);
        if (listenArticlesCnt < 0)
            listenArticlesCnt = 0;
        if ((uint16_t)listenArticlesCnt < 32767) {
            listenArticlesCnt++;
            setPrjGlobalParm(ZB_PARM_LISTEN_ARTICLES, sizeof(int16_t), &listenArticlesCnt);
            ESP_LOGI(TAG, "lisnArtcCnt: %hd", listenArticlesCnt);
        }
#endif
        //]--------
    
        play_text_page_free();
        play_text_win_init(NULL);
        ESP_LOGI(TAG, "%s G, par %d", __func__, parm);
    
        g_alarmTmStart = 0;

        sys_setCurMainWinAndCb(play_text_win, play_text_win_key_cb);
    
        play_text_state.current_loop_times = 1;  //默认循环播放下一曲目
        play_text_state.cur_play_unit_num = play_text_book_locate.playNum;
        play_text_state_init();
        ESP_LOGI(TAG, "%s K", __func__);
        play_normal_config();
    
        // 目录解析
        free_player_book_info_t(&play_text_book_info);
        ESP_LOGI(TAG, "%s M", __func__);

#if 1 //这段封装且在调用接口前实现
        char buf[200] = { 0 };
    
        const char* pcBasePath = NULL;
#ifndef PASS_PATH
        pcBasePath = getDatumBasePath(play_text_book_locate.path); 
        snprintf(buf, sizeof(buf), "%s%s/album.mnu", pcBasePath, play_text_book_locate.path);
#else
        snprintf(buf, sizeof(buf), "%s/album.mnu", play_text_book_locate.path);
#endif
        
       
        printf("pcBasePath= %s\n", pcBasePath);
        printf("buf= %s\n", buf);
        printf("----------------------------------------");
        printf("play_text_book_locate.path = %s\n", play_text_book_locate.path);
        int ret = player_get_book_inf(buf, &play_text_book_info);
        if (ret)
        {
            ESP_LOGI(PLA, "%s", "get book menu fail");
            return -1;
        }
        ESP_LOGI(TAG, "%s P", __func__);
        change_collect_btn_img();
        // 解析和显示歌词文本
        ret = play_text_sentence_parse(play_text_book_locate.path, play_text_book_locate.playNum);
        if (ret)
        {
            ESP_LOGI(PLA, "sentece parse fail\n");
            return -2;
        }
        ESP_LOGI(TAG, "%s R", __func__);
    #if LOAD_SENTENCE_MULT
        play_text_cont_load_part_init(play_text_book_locate.playNum);
    #else
        play_text_cont_load(play_text_book_locate.playNum);
    #endif
#endif 
        //zb_mem_print();
        ESP_LOGI(TAG, "%s T  %d", __func__, play_unit_info.audLen);
        if (play_unit_info.audLen > 0)
        {
    #ifndef C_PC_SIM
           // tz_play_stop_audio();
            g_extra = extra;
            ESP_LOGI("player_text", "%s [%d] audOffset: %ld, audLen:%ld, encParm:%d, speed:%d  ", __FUNCTION__, __LINE__, play_unit_info.audOffset, play_unit_info.audLen, play_unit_info.encParm, play_para.speed);
            aud_file_part_play(NULL, play_unit_info.fp, play_unit_info.audOffset, play_unit_info.audLen, play_unit_info.encParm, play_para.speed);
            player_interval_flag = 1;
            player_state_flag = PLAYER_BACKSTAGE_RUNNING;
    #else
            g_songTmTotal = 120 * 1000;  // non-std 
            player_interval_flag = 1;
            player_state_flag = PLAYER_BACKSTAGE_RUNNING;
    #endif
        }
        if (refresh_timer)
        {
            lv_timer_resume(refresh_timer);
        }
        return 0;
}

/**
 * @brief 播放页面返回，保留资源直接使用
 */
int play_text_return_init(int parm)
{
    ESP_LOGI("player_text", "%s [%d] ", __FUNCTION__, __LINE__);
    play_text_win_init(NULL);
    lv_group_add_obj(lv_group_get_default(), play_text_win);
    sys_setCurMainWinAndCb(play_text_win, play_text_win_key_cb);

    if (ap_toggle_num != C_ZB_AP_MY_DOWNLOAD)
    {
#if LOAD_SENTENCE_MULT
        play_text_cont_load_part_init(play_text_state.cur_play_unit_num);
#else
        play_text_cont_load(play_text_state.cur_play_unit_num);
#endif

        play_text_state.pre_index = -1;
        play_text_state.pre_label = NULL;
        if (play_unit_info.audLen > 0)
        {
#ifndef C_PC_SIM
            int32_t aud_offset = play_unit_info.ptInTm[play_text_state.current_play_sentence_num];
            tz_play_stop_audio();
           
            aud_file_part_play(NULL, play_unit_info.fp, play_unit_info.audOffset, play_unit_info.audLen, play_unit_info.encParm, play_para.speed);
            audio_stream_player_seek(aud_offset);
            player_state_flag = PLAYER_BACKSTAGE_RUNNING;
#endif
        }
    }
    change_collect_btn_img();
    if (refresh_timer)
    {
        lv_timer_resume(refresh_timer);
    }
    return 0;
}

/**
 * 主题选择
 */

static void select_topic(char* path)
{
#if 0
    if (strstr(path, "古诗古文"))
    {
        player_style = player_style;
    }
    else
    {
        player_style = player_normal_style;
    }
#endif
}


/**
 * @brief 初始化专辑播放页面( 播放其他模块的在线url)
 * @param playlist 播放列表对象指针
 * @param start_index 起始播放位置
 * @param ap_num 专辑分页编号
 * @return 成功返回0，失败返回错误码
 */
int play_album_online_page_init(Playlist* playlist, uint16_t start_index, int16_t ap_num, bool extra)
{
   
    if (!playlist || !playlist->items || playlist->count == 0) {
        return -1;
    }
    int i = 0;

    // 释放之前可能存在的播放列表资源
    free_global_playlist();
    play_text_page_free();

    play_text_state_init();
    ESP_LOGI(TAG, "%s K", __func__);
    play_normal_config();
    memset(&play_text_book_info, 0, sizeof(play_text_book_info));
    
    current_playlist.count = playlist->count;
    current_playlist.current_index = start_index;
    current_playlist.items = malloc(playlist->count * sizeof(PlaylistItem));
    if (!current_playlist.items) return -2;


    play_text_book_info.cnt = playlist->count;
    play_text_book_info.pTile = malloc(playlist->count * sizeof(char*));
    play_text_book_info.pFileName = malloc(playlist->count * sizeof(char*));

    if (!play_text_book_info.pTile || !play_text_book_info.pFileName) {
        free(current_playlist.items);
        free(play_text_book_info.pTile);
        free(play_text_book_info.pFileName);
        return -3;
    }

    for (i = 0; i < playlist->count; i++) {
        play_text_book_info.pTile[i] = NULL;
        play_text_book_info.pFileName[i] = NULL;
        current_playlist.items[i].location = NULL;
        current_playlist.items[i].title = NULL;
    }

    for (i = 0; i < playlist->count; i++) {
        // 深拷贝播放列表项
        current_playlist.items[i].source_type = playlist->items[i].source_type;
        current_playlist.items[i].cover = playlist->items[i].cover;
        current_playlist.items[i].playNum = playlist->items[i].playNum;

        // 拷贝位置字符串
        if (playlist->items[i].location) {
            current_playlist.items[i].location = strdup(playlist->items[i].location);
            if (!current_playlist.items[i].location) goto memory_error;
        }
        else {
            current_playlist.items[i].location = NULL;
        }


        if (playlist->items[i].title) {
            current_playlist.items[i].title = strdup(playlist->items[i].title);
            if (!current_playlist.items[i].title) goto memory_error;


            play_text_book_info.pTile[i] = strdup(current_playlist.items[i].title); 
            if (!play_text_book_info.pTile[i]) goto memory_error;

           // LV_LOG_USER("pTile[%d]:%s\r\n", i, play_text_book_info.pTile[i]);
        }
        else {
            current_playlist.items[i].title = NULL;
            play_text_book_info.pTile[i] = NULL;
            LV_LOG_USER("no title\r\n");
        }

        if (playlist->items[i].location) {
            play_text_book_info.pFileName[i] = strdup(current_playlist.items[i].location);
            if (!play_text_book_info.pFileName[i]) goto memory_error;
        }
        else {
            play_text_book_info.pFileName[i] = NULL;
        }
    }

    // 获取当前播放项
    PlaylistItem* current = &current_playlist.items[start_index];


    play_para.loopmode = 0;
    ap_toggle_num = ap_num;


    play_list_book_locate.cover = current->cover;
    play_text_book_info.cnt = playlist->count;

    play_text_win_init(NULL);
    ESP_LOGI("play_album_online_page_init","current->source_type:%d\r\n", current->source_type);
    if (current->source_type == MUSIC_SOURCE_TYPE_SD) {

        ESP_LOGE(TAG, "current->playNum:%d", current->playNum);
        play_text_book_locate.playNum = current->playNum;
        ESP_LOGE(TAG, "current->location:%s", current->location);
        strlcpy(play_text_book_locate.path,
            current->location ? current->location : "",
            sizeof(play_text_book_locate.path));

        ESP_LOGE(TAG, "play_text_book_locate.path:%s", play_text_book_locate.path);
        ESP_LOGI(TAG, "%s M", __func__);
        char buf[200] = { 0 };

        snprintf(buf, sizeof(buf), "%s/album.mnu", play_text_book_locate.path);

        change_collect_btn_img();


        ESP_LOGI(TAG, "%s R", __func__);
        if (update_song_info_in_book_info(&play_text_book_info, play_text_book_locate.playNum, buf) == 0) {
            // int ret = player_get_book_inf(buf, &play_text_book_info);
            ESP_LOGI(TAG, "Song info updated successfully");
    }
        else {
            ESP_LOGE(TAG, "play_text_book_locate.playNum :%d", play_text_book_locate.playNum);
            ESP_LOGE(TAG, "buf :%s", buf);
            ESP_LOGE(TAG, "Failed to update song info");
            goto memory_error;
        }
        ESP_LOGI(TAG, "%s S", __func__);

        if ((ap_num != C_ZB_AP_COLLECT) && (ap_num != C_ZB_AP_MY_DOWNLOAD))
        {
#if LOAD_SENTENCE_MULT
            play_text_cont_load_part_init(play_text_book_locate.playNum);
#else
            play_text_cont_load(play_text_book_locate.playNum);
#endif
        }
        ESP_LOGI(TAG, "%s P", __func__);

        // 解析和显示歌词文本
        int ret = play_text_sentence_parse(play_text_book_locate.path, play_text_book_locate.playNum);
        if (ret)
        {
            ESP_LOGI(PLA, "sentece parse fail\n");
            goto memory_error;
        }
        ESP_LOGI(TAG, "%s R", __func__);

#ifndef C_PC_SIM
        aud_file_part_play(NULL, play_unit_info.fp, play_unit_info.audOffset, play_unit_info.audLen, play_unit_info.encParm, play_para.speed);
        player_state_flag = PLAYER_BACKSTAGE_RUNNING;
#endif
    } 
    else if(current->source_type == RES_BAIDU_CLOUD)
    {
        ESP_LOGE(TAG, "current->playNum:%d", current->playNum);
        play_text_book_locate.playNum = current->playNum;
        change_collect_btn_img();
        play_text_state.cur_play_unit_num = start_index;
        snprintf(play_text_book_locate.path, sizeof(play_text_book_locate.path),
            "%s", current->location);
        player_request_init();
        if (!player_acquire_request_lock(play_text_state.cur_play_unit_num, current->location, current->source_type)) {
            LV_LOG_WARN("Request lock acquisition failed");
            return;
        }

        request_media_playback(current->location, current->source_type);
        ESP_LOGI("request_media_playback", "request media playback succeed\r\n");
        
    }
    else
    {
        ESP_LOGE(TAG, "ONLINE_RESOURSE--------------A-----------------\r\n");
        ESP_LOGE(TAG, "current->playNum:%d", current->playNum);
        play_text_book_locate.playNum = current->playNum;
        change_collect_btn_img();
        play_text_state.cur_play_unit_num = start_index;
        snprintf(play_text_book_locate.path, sizeof(play_text_book_locate.path),
            "%s", current->location);
        player_request_init();
        ESP_LOGE(TAG, "ONLINE_RESOURSE--------------B-----------------\r\n");
        if (!player_acquire_request_lock(play_text_state.cur_play_unit_num, current->location, current->source_type)) {
            LV_LOG_WARN("Request lock acquisition failed");
            return;
        }
        ESP_LOGE(TAG, "ONLINE_RESOURSE--------------C-----------------\r\n");
        request_media_playback(current->location, current->source_type);
        ESP_LOGI("request_media_playback", "request media playback succeed\r\n");

    }
    ESP_LOGI(TAG, "current->title :%s\r\n", current->title);
    
    strlcpy(play_text_book_locate.title, current->title, sizeof(play_text_book_locate.title));
    lv_label_set_text(title_label, play_text_book_locate.title);

    
     g_alarmTmStart = 0;

    sys_setCurMainWinAndCb(play_text_win, play_text_win_key_cb);



    return 0;

memory_error:
    // 错误处理：释放已分配的资源
    for (int j = 0; j < playlist->count; j++) {
        free(current_playlist.items[j].location);
        free(current_playlist.items[j].title);
        free(play_text_book_info.pTile[j]);
        free(play_text_book_info.pFileName[j]);
    }
    free(current_playlist.items);
    free(play_text_book_info.pTile);
    free(play_text_book_info.pFileName);

    current_playlist.count = 0;
    play_text_book_info.cnt = 0;
    memset(&current_playlist, 0, sizeof(Playlist));
    memset(&play_text_book_info, 0, sizeof(player_book_info_t));
    return -4;


}

int player_is_play()
{
    if ((play_text_book_locate.path == NULL) || (play_text_book_locate.path[0] == '\0'))
    {
        ESP_LOGI("player_is_play", "player not play \r\n");
        return 0;
    }
    return 1;
}

bool player_has_valid_data(void) {
    return (current_playlist.items != NULL && current_playlist.count > 0) ||
        (play_text_book_info.cnt > 0);
}

void MMN_player_enter(void)
{
    MMN_ENTER_STAT = true;
    MMN_player_page_init();
}


/**
 * @brief   主页面进入的播放接口
 * @param
 *          path 路径，例如（必读名著/格列佛游记）
 *          loop_times 设置专辑播放的循环次数,最大不要大于5次，不需要循环次数设为1
 *          ap_num 调用本接口的函数的AP号（在main\utils\misc_utils.h的宏定义）
 */
int MMN_player_page_init()
{ 
    if ((play_text_book_locate.path == NULL) || (play_text_book_locate.path[0] == '\0'))
    {
        ESP_LOGI("MMN_player", "no music\r\n");
        return -1;
    }
    player_func.play_exit = 0;
    
    play_text_win_init(NULL);
    ESP_LOGI(TAG, "%s G,", __func__);

    sys_setCurMainWinAndCb(play_text_win, play_text_win_key_cb);

    ESP_LOGI(TAG, "%s K", __func__);

#if 1 //这段封装且在调用接口前实现
    char buf[200] = { 0 };

    const char* pcBasePath = NULL;

 
    pcBasePath = getDatumBasePath(play_text_book_locate.path);

    snprintf(buf, sizeof(buf), "%s%s/album.mnu", pcBasePath, play_text_book_locate.path);

    change_collect_btn_img();


    ESP_LOGI(TAG, "%s R", __func__);
#if LOAD_SENTENCE_MULT
    play_text_cont_load_part_init(play_text_book_locate.playNum);
#else
    play_text_cont_load(play_text_book_locate.playNum);
#endif
#endif 
    

    ESP_LOGI(TAG, "%s T  %d", __func__, play_unit_info.audLen);
    if (refresh_timer)
    {
        lv_timer_resume(refresh_timer);
    }
    return 0;
}







#if 1
/**
 * @brief   其他ap跳转播放函数(需要下载的流播放数据)
 * @param
 *          path 路径，例如（必读名著/格列佛游记）
 *          loop_times 设置专辑播放的循环次数,最大不要大于5次，不需要循环次数设为1
 *          ap_num 调用本接口的函数的AP号（在main\utils\misc_utils.h的宏定义）
 */
int play_album_page_init(char* path, uint16_t play_num, int16_t ap_num)
{
    ESP_LOGI("play_album_page_init","--A--\r\n");
    free_global_playlist();
    ESP_LOGI("play_album_page_init", "--B--\r\n");
    play_para.loopmode = 0;
    ap_toggle_num = ap_num;
    play_list_book_locate.cover = -1;
    play_text_book_locate.playNum = play_num;
    play_text_book_locate.cover = play_list_book_locate.cover;
    strncpy(play_text_book_locate.path, path, sizeof(play_text_book_locate.path) - 1);
    strncpy(play_list_book_locate.path, path, sizeof(play_text_book_locate.path) - 1);
    ESP_LOGI("play_album_page_init", "--C--\r\n");
    play_text_page_init(0, 1);
    ESP_LOGI("play_album_page_init", "--D--\r\n");
    play_album_config();
    player_backstage_flag = 0;
    ESP_LOGI("play_album_page_init", "--E--\r\n");
    return 0;
}
#else
/**
 * @brief 专辑播放函数, 现用于响闹播放
 * @param
 *          path 路径，例如（必读名著/格列佛游记）
 *          loop_times 设置专辑播放的循环次数,最大不要大于5次，不需要循环次数设为1
 *          ap_num 调用本接口的函数的AP号（在main\utils\misc_utils.h的宏定义）
 */
int play_album_page_init(char* path, uint8_t loop_times, int16_t ap_num)
{
    play_para.loopmode = 0;  // 响闹时播放模式为多曲模式 
    select_topic(path);
    ap_toggle_num = ap_num;
    play_list_book_locate.cover = -1;
    play_text_book_locate.cover = play_list_book_locate.cover;
    strncpy(play_text_book_locate.path, path, sizeof(play_text_book_locate.path) - 1);
    play_text_book_locate.playNum = 0;
    play_text_page_init(1, 0);
    play_album_config();
    player_backstage_flag = 0;
    return 0;
}
#endif

/**
 * @brief 歌曲播放函数
 * @param
 *          path 路径，例如（必读名著/book002）
 *          play_num 播放的第几首歌曲
 *          ap_num 调用本接口的函数的AP号（在main\utils\misc_utils.h的宏定义）
 *          creat_group 1:重新创建输入组 0：采用默认输入组， （注意：除了磨耳朵调用需要重新创建输入组，其他调用都不需要）
 */
int play_single_page_init(char* path, uint16_t play_num, int16_t ap_num, uint8_t creat_group)
{
    printf("path = %s,num = %d\n", path, play_num);
    select_topic(path);
    ap_toggle_num = ap_num;
    play_list_book_locate.cover = -1;
    play_text_book_locate.cover = play_list_book_locate.cover;
    strncpy(play_text_book_locate.path, path, sizeof(play_text_book_locate.path) - 1);
    play_text_book_locate.playNum = play_num;
    play_text_page_init(creat_group, 0);
    play_single_config();
    player_backstage_flag = 0;
    return 0;
}


/**
 * @brief 最近播放列表歌曲播放函数,需增加下一曲下一曲的功能
 * @param
 *          path 路径，例如（必读名著/book002）
 *          play_num 播放的第几首歌曲
 *          ap_num 调用本接口的函数的AP号（在main\utils\misc_utils.h的宏定义）
 *          creat_group 1:重新创建输入组 0：采用默认输入组， （注意：除了磨耳朵调用需要重新创建输入组，其他调用都不需要）
 */
int play_recently_or_collect_page_init(char* path, uint16_t play_num, int16_t ap_num, uint8_t list_num, uint8_t creat_group)
{
    LV_LOG("%s [%d] path = %s, num = %d, ap_num: %d\n", __FUNCTION__, __LINE__, path, play_num, ap_num);
    select_topic(path);
    ap_toggle_num = ap_num;
    collect_album_flag = 0;
    play_list_book_locate.cover = -1;
    play_text_book_locate.cover = play_list_book_locate.cover;
    strncpy(play_text_book_locate.path, path, sizeof(play_text_book_locate.path) - 1);
    play_text_book_locate.playNum = play_num;
    playlist_sn = list_num;
    play_text_page_init(creat_group, 0);
    play_recently_or_collect_config();
    player_backstage_flag = 0;
    return 0;
}

/**
 * @brief 我的下载歌曲播放函数
 * @param
 *          path 路径，就是路径
 *          play_num 播放的第几首歌曲
 *          ap_num 调用本接口的函数的AP号（在main\utils\misc_utils.h的宏定义）
 *          creat_group 1:重新创建输入组 0：采用默认输入组， （注意：除了磨耳朵调用需要重新创建输入组，其他调用都不需要）
 */
int mydownload_page_init(int16_t ap_num, uint8_t creat_group)
{
    LV_LOG("%s [%d] ap_num: %d\n", __FUNCTION__, __LINE__, ap_num);
    //select_topic(path);
    ap_toggle_num = ap_num;
    collect_album_flag = 0;
    play_list_book_locate.cover = -1;
    play_text_book_locate.cover = play_list_book_locate.cover;
    music_count = 0;
    music_play_cur = 0;
    play_text_page_init(creat_group, 0);
    play_recently_or_collect_config();
    player_backstage_flag = 0;

    player_func.play_enable = true;

    free_music_file_data();
    music_count = music_file_directory_data_processing(NULL, 0);
    for (int i = 0; i < music_count; i++) {
        printf("MP3文件[%d]: %s\n", i, second_son_absolute_path[i]);
    }
    
    tz_play_set_volume(30);
    music_play_cur = 0;
    tz_play_audio(second_son_absolute_path[music_play_cur], 0, -1, true);


    return 0;
}



/**
 * @brief 搜索播放初始化
 */
int play_search_page_init(char* class_path, uint16_t class_num, uint16_t list_num, int16_t ap_num)
{
    char buf[60] = { 0 };
    char temp[100] = { 0 };
    strcpy(buf, class_path);
    sprintf(temp, "%s%s/%s.mnu", PLAYER_DATA_PATH, class_path, class_path);
    struct LSNSPK_MENU* class_menu = LSNSPK_menu_open(temp);
    if (!class_menu)
    {
        return -1;
    }
    strcat(buf, "/");
    strcat(buf, class_menu->astAlbumTbl[class_num].m_ptitle);
    LSNSPK_menu_close(class_menu);
    class_menu = NULL;
    play_single_page_init(buf, list_num, ap_num, 0);
    return 0;
}


/*****************************************************************************************/
/*@brief 创建播放列表界面
/* @param parent 父对象
/* @return
/*****************************************************************************************/
/* 全局样式定义 */
static lv_style_t title_style;
static lv_style_t list_btn_style;

lv_obj_t* cont_playlist = NULL;
static player_book_info_t play_list_book_info = { 0 };

/**
 * @brief 专辑列表页面释放
 */
int play_list_page_free(void)
{
    LV_LOG_USER("%s[%d]\n", __FUNCTION__, __LINE__);
    free_player_book_info_t(&play_list_book_info);
    if (play_list_win)
    {
        lv_obj_del(play_list_win);
        play_list_win = NULL;
    }
    if (dialog != NULL)
    {
        lv_obj_del(dialog);  dialog = NULL;

    }
    LV_LOG_USER("%s[%d]\n", __FUNCTION__, __LINE__);
    return 0;
}

static void list_return_btn_click_cb(lv_event_t* e) //需要做到页面切换
{
    LV_LOG_USER("list_return_btn_click_cb");
#if 0
    playlist_win_del();
#else
    player_switchScr(PLAYER_SCR_MAIN, NULL);
#endif
}

static void playlist_win_del()
{
    if (cont_playlist)
    {
        lv_obj_del(cont_playlist);
        cont_playlist = NULL;
    }
}


int music_file_directory_data_processing(char* path, int Dir_depth) {
    LV_LOG_USER("已进入：音频文件目录数据处理\n");

    const char* target_path = PLAYER_TEST_FILE_PATH;
    LV_LOG_USER("处理路径：%s", target_path);

    LV_LOG_USER("开始目录处理\n");
    char* sd_card_directory = zb_strdup(target_path);
    if (!sd_card_directory) {
        ESP_LOGE(TAG, "内存分配失败！");
        Number_secondary_file_dir = 0;
        return -1;
    }

    lv_fs_dir_t stDir = { 0 };
    lv_fs_res_t STAT = STAT = lv_fs_dir_open(&stDir, sd_card_directory);
    if (STAT != LV_FS_RES_OK) {
        LV_LOG_USER("STAT %d", STAT);
        ESP_LOGE(TAG, "打开目录失败！");
        Number_secondary_file_dir = 0;
        heap_caps_free(sd_card_directory);
        return -1;
    }

    int file_count = 0;
    char d_name[MDLD_FILE_NAME_BUF_LEN];


    memset(second_son_absolute_path, 0, sizeof(second_son_absolute_path));
    memset(second_level_directory, 0, sizeof(second_level_directory));

    while (lv_fs_dir_read(&stDir, d_name) == LV_FS_RES_OK) {

        if (strcmp(d_name, ".") == 0 || strcmp(d_name, "..") == 0) continue;


        char full_path[PATH_BYTE_SIZE];
        snprintf(full_path, sizeof(full_path), "%s/%s", sd_card_directory, d_name);


        int st_mode = 0;
        if (zb_stat__st_mode(full_path, &st_mode) < 0) continue;


        if (S_ISREG(st_mode)) {
            const char* ext = strrchr(d_name, '.');
            if (ext && strcasecmp(ext, ".mp3") == 0) {
                if (file_count >= NUMBER_FILES_MAX) {
                    ESP_LOGW(TAG, "超过最大文件数量限制");
                    break;
                }


                second_son_absolute_path[file_count] = heap_caps_malloc_dflt(PATH_BYTE_SIZE);
                second_level_directory[file_count] = heap_caps_malloc_dflt(MDLD_FILE_NAME_BUF_LEN);

                if (!second_son_absolute_path[file_count] || !second_level_directory[file_count]) {
                    ESP_LOGE(TAG, "内存分配失败");
                    break;
                }

                strlcpy(second_son_absolute_path[file_count], full_path, PATH_BYTE_SIZE);
                strlcpy(second_level_directory[file_count], d_name, MDLD_FILE_NAME_BUF_LEN);

                file_count++;
            }
        }
    }


    lv_fs_dir_close(&stDir);
    heap_caps_free(sd_card_directory);



    Number_secondary_file_dir = file_count;
    ESP_LOGI(TAG, "共找到%d个MP3文件", file_count);
    return file_count;
}

static void playlist_page_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_KEY) {
        int32_t c = *((int32_t*)lv_event_get_param(e));

        if (c == LV_KEY_ESC) {
            lv_indev_keypad_wait_release();
            ESP_LOGI("func_page_event_cb", "func page click esc\r\n");
            player_switchScr(PLAYER_SCR_MAIN, NULL);

        }
    }
}

static lv_style_t list_btn_style_default;  // 默认状态样式
static lv_style_t list_btn_style_pressed; // 按下状态样式
void create_playlist_screen(void* parm) {
    /* 创建基础容器 */

#if 1
    music_count = play_text_book_info.cnt; //测试不同入口给同样的数据接口
#endif

    if (cont_playlist)
    {
        lv_obj_del(cont_playlist);
        cont_playlist = NULL;
    }
    cont_playlist = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont_playlist, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(cont_playlist, 0, 0);
    lv_obj_set_style_bg_color(cont_playlist, lv_color_hex(0x1A143B), 0);
    lv_obj_set_flex_flow(cont_playlist, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_top(cont_playlist, 10, 0);
    lv_obj_set_style_pad_bottom(cont_playlist, 10, 0);
    lv_obj_set_style_border_width(cont_playlist, 0, 0);
    lv_obj_clear_flag(cont_playlist, LV_OBJ_FLAG_SCROLLABLE); // 禁用容器自身滚动

    sys_setCurMainWinAndCb(cont_playlist, playlist_page_event_cb);

    /* 初始化样式 */
    lv_style_init(&title_style);
    // lv_style_set_text_font(&title_style, &lv_font_montserrat_20);
    lv_style_set_text_color(&title_style, lv_color_hex(0xffffff));
    //lv_style_set_text_color(&title_style, lv_color_hex(0x333333));

    lv_style_init(&list_btn_style);
    // lv_style_set_bg_opa(&list_btn_style, LV_OPA_0);
    lv_style_set_pad_all(&list_btn_style, 10);
    lv_style_set_radius(&list_btn_style, 10);
    lv_style_set_border_width(&list_btn_style, 0);
    lv_style_set_shadow_width(&list_btn_style, 0);
    lv_style_set_bg_color(&list_btn_style, lv_color_hex(0x302B4E));
    lv_style_set_text_color(&list_btn_style, lv_color_hex(0xffffff));

    /* 标题区域 */
    lv_obj_t* title_cont = lv_obj_create(cont_playlist);
    lv_obj_set_size(title_cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(title_cont, LV_OPA_0, 0);
    lv_obj_set_style_border_opa(title_cont, LV_OPA_0, 0);
    lv_obj_clear_flag(title_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(title_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_t* title_label = lv_label_create(title_cont);
    lv_label_set_text(title_label, "播放列表");
    lv_obj_add_style(title_label, &title_style, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, -5, 0);


    lv_obj_t* count_label = lv_label_create(title_cont);
#if !lowndoad_player_test

    lv_label_set_text(count_label, "共8篇");

#else

    lv_label_set_text_fmt(count_label, "共%d首", music_count);
#endif
    lv_obj_add_style(count_label, &title_style, 0);
    lv_obj_align_to(count_label, title_label, LV_ALIGN_OUT_RIGHT_MID, 60, 0);

    lv_obj_t* return_btn = lv_btn_create(title_cont);
    lv_obj_set_size(return_btn, btn_size, btn_size);
    lv_obj_set_style_radius(return_btn, 17, 0);
    lv_obj_set_style_bg_color(return_btn, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(return_btn, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(return_btn, 0, 0);
    lv_img_dsc_t* return_pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_RETURN, IMG_FREE_TYPE_AUTO, NULL);
    lv_obj_set_style_bg_img_src(return_btn, return_pImgDsc, 0);
    lv_obj_align_to(return_btn, title_label, LV_ALIGN_OUT_LEFT_MID, -70, 0);
    lv_obj_add_event_cb(return_btn, listFuncInfo.event_cb[0], LV_EVENT_CLICKED, NULL);

    /* 全部播放按钮 */
    lv_obj_t* all_play = lv_obj_create(cont_playlist);
    lv_obj_set_size(all_play, LV_PCT(60), 50);
    lv_obj_set_style_bg_color(all_play, lv_color_hex(0x1A143B), 0);
    lv_obj_set_style_bg_opa(all_play, LV_OPA_0, 0);
    lv_obj_set_style_border_width(all_play, 0, 0);
    lv_obj_set_style_shadow_width(all_play, 0, 0);
    lv_obj_clear_flag(all_play, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* all_play_btn = lv_btn_create(all_play);
    lv_obj_set_size(all_play_btn, btn_size, btn_size);
    lv_obj_set_style_bg_color(all_play_btn, lv_color_hex(0x1A143B), 0);
    lv_obj_set_style_bg_opa(all_play_btn, LV_OPA_0, 0);
    lv_obj_set_style_border_width(all_play_btn, 0, 0);
    lv_obj_set_style_shadow_width(all_play_btn, 0, 0);
    lv_img_dsc_t* all_play_btn_pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_ALLPLAY, IMG_FREE_TYPE_AUTO, NULL);
    lv_obj_set_style_bg_img_src(all_play_btn, all_play_btn_pImgDsc, 0);
    lv_obj_align(all_play_btn, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_event_cb(all_play_btn, listFuncInfo.event_cb[0], LV_EVENT_CLICKED, NULL);

    lv_obj_t* btn_label = lv_label_create(all_play);
#if !lowndoad_player_test
    lv_label_set_text(btn_label, "全部播放(8)");

#else

    lv_label_set_text_fmt(btn_label, "全部播放(%d)", music_count);
#endif
    lv_obj_add_style(btn_label, &title_style, 0);
    lv_obj_align_to(btn_label, all_play_btn, LV_ALIGN_OUT_RIGHT_MID, 0, 0);


    /* 列表容器 */
    lv_obj_t* list_cont = lv_obj_create(cont_playlist);
    lv_obj_set_size(list_cont, LV_PCT(100), LV_PCT(80));
    lv_obj_set_style_bg_opa(list_cont, LV_OPA_0, 0);
    lv_obj_set_style_border_width(list_cont, 0, 0);
    lv_obj_set_style_shadow_width(list_cont, 0, 0);
    lv_obj_set_flex_flow(list_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(list_cont, 15, 0);
    lv_obj_set_style_pad_top(list_cont, 10, 0);
    lv_obj_set_style_pad_bottom(list_cont, 20, 0);
    lv_obj_set_style_bg_opa(list_cont, LV_OPA_0, 0);
    lv_obj_set_scrollbar_mode(list_cont, LV_SCROLLBAR_MODE_OFF);

    lv_style_init(&list_btn_style_default);
    lv_style_set_bg_color(&list_btn_style_default, lv_color_hex(0x302B4E));         

    lv_style_init(&list_btn_style_pressed);
    lv_style_set_bg_color(&list_btn_style_pressed, lv_color_hex(0x008CFF)); 

    for (int i = 0; i < music_count; i++) {
        lv_obj_t* item_btn = lv_btn_create(list_cont);
        lv_obj_set_size(item_btn, 296, 48);
        lv_obj_add_style(item_btn, &list_btn_style, 0);

       // lv_obj_set_style_bg_color(item_btn, lv_color_hex(0x302B4E), LV_PART_MAIN | LV_STATE_DEFAULT);

       // lv_obj_add_style(item_btn, &list_btn_style_default, LV_PART_MAIN | LV_STATE_DEFAULT);
       // lv_obj_add_style(item_btn, &list_btn_style_pressed, LV_PART_MAIN | LV_STATE_PRESSED);


        lv_obj_add_event_cb(item_btn, list_item_click_handler, LV_EVENT_CLICKED, i);

        lv_obj_t* label = lv_label_create(item_btn);
#if lowndoad_player_test
#if 1
        char* pTitle = play_text_book_info.pTile[i];
        if (strchr(pTitle, 0x05))
            pTitle = strchr(pTitle, 0x05) + 1;
        lv_label_set_text(label, pTitle);
#else
        lv_label_set_text(label, play_text_book_info.pTile[i]);

#endif
        lv_obj_align_to(label, item_btn, LV_ALIGN_LEFT_MID,10,0);
        lv_obj_clear_flag(label,LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(label, LV_SCROLLBAR_MODE_OFF);
        // lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);



#else

        // 显示带序号的文件名
        char display_name[128];
        snprintf(display_name, sizeof(display_name), "%d. %s",
            i + 1,
            second_son_absolute_path[i]);
        lv_label_set_text(label, display_name);
#endif
    }

}

/* 事件处理函数实现 */
static void all_play_click_handler(lv_event_t * e) {
    LV_LOG_USER("全部播放点击");
    // 添加播放全部逻辑
}

static void list_item_click_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_target(e);
    //uint32_t index = (uint32_t)lv_event_get_user_data(e);

    play_list_clicked_btn_num = lv_obj_get_child_id(obj);

    if (g_player_request_state.is_requesting) {
        LV_LOG_WARN("Request in progress, ignoring click");
        return;
    }

    if (code == LV_EVENT_CLICKED)
    {
        play_media_unit(play_list_clicked_btn_num);
        lv_indev_keypad_wait_release();

        LV_LOG_USER("play_text_book_locate.playNum : %d", play_text_book_locate.playNum);
        play_text_book_locate.playNum = play_text_state.cur_play_unit_num;
        player_switchScr(PLAYER_SCR_MAIN, NULL);
    }
}





/*****************************************************************************************/
/*@brief 创建变速设置界面
/* @param parent 父对象
/* @return
/*****************************************************************************************/

static lv_obj_t* selected_play_speed_btn = NULL;  // 当前选中的播放速度按钮
static float play_speed_num = 0;                // 记录当前播放速度

static void speed_btn_event_cb(lv_event_t * e) {
    lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* label = lv_obj_get_child(btn, 0);
    const char* txt = lv_label_get_text(label);


    if (selected_play_speed_btn != NULL) {
        lv_obj_t* prev_label = lv_obj_get_child(selected_play_speed_btn, 0);
        lv_obj_set_style_text_opa(prev_label, LV_OPA_50, 0);
    }

 
    lv_obj_set_style_text_opa(label, LV_OPA_100, 0);
    selected_play_speed_btn = btn;

    LV_LOG_USER("播放速度: %s", txt);

    if (strcmp(txt, "0.5x") == 0) {
        play_speed_num = 0.5;
    }
    else if (strcmp(txt, "0.75x") == 0) {
        play_speed_num = 0.75;
    }
    else if (strcmp(txt, "1x") == 0) {
        play_speed_num = 1;
    }
    else if (strcmp(txt, "1.25x") == 0) {
        play_speed_num = 1.25;
    }
    else if (strcmp(txt, "1.5x") == 0) {
        play_speed_num = 1.5;
    }
    LV_LOG_USER("play_speed_num: %0.2f", play_speed_num);

    play_para.speed = play_speed_num;
    //play_para.speed = play_speed[play_speed_num];
   // audio_stream_set_play_speed(play_speed[play_speed_num]);
    tz_play_set_speed(play_speed_num);

}
void create_function_speed_ui(lv_obj_t * parent) {
    lv_obj_t* func_scr_panel_speed = lv_obj_create(parent);
    lv_obj_set_style_border_width(func_scr_panel_speed, 0, 0);
    lv_obj_set_size(func_scr_panel_speed,280, 100);
    lv_obj_set_style_radius(func_scr_panel_speed, 13, 0);
    lv_obj_set_scrollbar_mode(func_scr_panel_speed, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(func_scr_panel_speed, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(func_scr_panel_speed, LV_OPA_10, 0);
    lv_obj_set_style_text_color(func_scr_panel_speed, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_pad_top(func_scr_panel_speed, 15, 0);
    //lv_obj_set_flex_flow(func_scr_panel_speed, LV_FLEX_FLOW_COLUMN);
    //lv_obj_set_flex_align(func_scr_panel_speed, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);



    lv_obj_t* speed_btn_list_label = lv_label_create(func_scr_panel_speed);
    lv_obj_set_size(speed_btn_list_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_text_color(speed_btn_list_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(speed_btn_list_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(speed_btn_list_label, "倍速设置");
    lv_obj_align_to(speed_btn_list_label, func_scr_panel_speed, LV_ALIGN_TOP_LEFT,10,3);

    lv_obj_t* speed_btn_list = lv_obj_create(func_scr_panel_speed);
    lv_obj_set_size(speed_btn_list, lv_pct(92), lv_pct(60));
    //lv_obj_set_style_bg_opa(speed_btn_list, LV_OPA_50, 0);
    lv_obj_set_style_border_width(speed_btn_list, 0, 0);
    lv_obj_set_style_radius(speed_btn_list, 0, 0);
    lv_obj_set_style_bg_opa(speed_btn_list, LV_OPA_0, 0);
    lv_obj_set_scrollbar_mode(speed_btn_list, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(speed_btn_list, lv_color_hex(0xffffff), 0);

    lv_obj_set_flex_flow(speed_btn_list, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_row(speed_btn_list, 14, 0);
    
    lv_obj_align_to(speed_btn_list, speed_btn_list_label, LV_ALIGN_OUT_BOTTOM_LEFT,-5,10);

    //lv_obj_set_flex_align(speed_btn_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);




    char* speed_mode[] = { "0.5x","0.75x","1x","1.25x","1.5x" };


    for (int i = 0; i < 5; i++)
    {
        lv_obj_t* btn = lv_btn_create(speed_btn_list);
        lv_obj_set_size(btn, 50, 30);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_0, 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);
        lv_obj_add_event_cb(btn, menuFuncInfo.event_cb[2], LV_EVENT_CLICKED, NULL);//speed_btn_event_cb


        lv_obj_t* text = lv_label_create(btn);
        lv_obj_set_size(text, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_text_opa(text, LV_OPA_50, 0);
        lv_obj_set_style_text_color(text, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_align(text, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_font(text, &lv_font_montserrat_16,0);

        float num = 0.5 + 0.25 * i;
        lv_label_set_text_fmt(text, "%s", speed_mode[i]);
        lv_obj_center(text);
   
        if (num == play_para.speed)//根据设定数据提前高亮 
        {
            LV_LOG_USER("play_para.speed num:%d",i);
            lv_obj_set_style_text_opa(text, LV_OPA_100, 0);
            selected_play_speed_btn = btn;
        }
    }
}

/*****************************************************************************************/
/*@brief 创建功能设置界面
/* @param parent 父对象
/* @return
/*****************************************************************************************/

static lv_obj_t* selected_play_mode_btn = NULL;  // 当前选中的播放模式按钮
static int8_t play_mode_num = 0;                    // 记录当前播放模式（0=未选，1=顺序播放，2=随机播放...）

static void play_mode_btn_event_cb(lv_event_t * e) {
    lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* label = lv_obj_get_child(btn, 0);
    const char* txt = lv_label_get_text(label);

    // 更新之前选中的按钮透明度为30%
    if (selected_play_mode_btn != NULL) {
        lv_obj_t* prev_label = lv_obj_get_child(selected_play_mode_btn, 0);
        lv_obj_set_style_text_opa(prev_label, LV_OPA_50, 0);
    }

    // 设置新选中的按钮透明度为100%
    lv_obj_set_style_text_opa(label, LV_OPA_100, 0);
    selected_play_mode_btn = btn;

    LV_LOG_USER("播放模式: %s", txt);

    // 根据选中的按钮设置模式
    if (strcmp(txt, "顺序播放") == 0) {
        play_mode_num = 1;
    }
    else if (strcmp(txt, "随机播放") == 0) {
        play_mode_num = 2;
    }
    else if (strcmp(txt, "单曲循环") == 0) {
        play_mode_num = 3;
    }
    else if (strcmp(txt, "列表播放") == 0) {
        play_mode_num = 4;
    }

    LV_LOG_USER("play_mode_num: %d", play_mode_num);
    play_para.loopmode = play_mode_num;

}

void create_play_mode_ui(lv_obj_t * parent) {

    lv_obj_t* func_scr_panel_setting = lv_obj_create(parent);
    lv_obj_set_style_pad_all(func_scr_panel_setting, 0, 0);
    lv_obj_set_style_border_width(func_scr_panel_setting, 0, 0);
    lv_obj_set_size(func_scr_panel_setting, 280, 131);
    lv_obj_set_style_radius(func_scr_panel_setting, 13, 0);
    lv_obj_set_scrollbar_mode(func_scr_panel_setting, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(func_scr_panel_setting, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(func_scr_panel_setting, LV_OPA_10, 0);
    lv_obj_set_style_text_color(func_scr_panel_setting, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_pad_top(func_scr_panel_setting, 15, 0);
   

    lv_obj_t* cont_title = lv_label_create(func_scr_panel_setting);
    lv_label_set_text(cont_title, "循环模式");
    lv_obj_set_size(cont_title, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_text_color(cont_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(cont_title, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align_to(cont_title, func_scr_panel_setting, LV_ALIGN_TOP_LEFT, 10, 3);

    lv_obj_t* cont_mode = lv_obj_create(func_scr_panel_setting);
    lv_obj_set_style_pad_all(cont_mode, 0, 0);
    lv_obj_set_size(cont_mode, LV_PCT(100), LV_PCT(70));
    lv_obj_set_style_bg_opa(cont_mode, LV_OPA_0, 0);
    lv_obj_set_style_border_width(cont_mode, 0, 0);
    lv_obj_clear_flag(cont_mode, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(cont_mode, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(cont_mode, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont_mode, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cont_mode, 0, 0);
    lv_obj_set_style_pad_column(cont_mode, 25, 0);
    lv_obj_align_to(cont_mode, cont_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

    const char* modes[] = { "顺序播放", "随机播放", "单曲循环", "列表播放" };
    for (int i = 0; i < 4; i++) {
        lv_obj_t* btn = lv_btn_create(cont_mode);
        lv_obj_set_size(btn, 70, 37);


        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2f3243), 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_0, 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);

        lv_obj_add_event_cb(btn, play_mode_btn_event_cb, LV_EVENT_CLICKED, NULL);


        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, modes[i]);
        lv_obj_set_style_text_opa(label, LV_OPA_50, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
#ifndef C_PC_SIM
        lv_label_set_cjk_font(label, 16, FT_FONT_STYLE_NORMAL);
#endif
        lv_obj_align(label,LV_ALIGN_BOTTOM_MID,0,0);

        if (i == (play_para.loopmode-1))//test  play_para.loopmode
        {
            lv_obj_set_style_text_opa(label, LV_OPA_100, 0);
            selected_play_mode_btn = btn;
            LV_LOG_USER("play_para.loopmode:%d", i);
        }
    }

}
/*****************************************************************************************/
/*@brief 创建单句复读设置界面
/* @param parent 父对象
/* @return
/*****************************************************************************************/

static lv_obj_t* cycle_label;          // 循环次数标签
static lv_obj_t* interval_label;       // 间隔标签
static lv_obj_t* pause_label;          // 句末暂停标签
static lv_obj_t* sw_pause;             // 句末暂停开关
static lv_obj_t* cycle_btns[4]={ NULL };        // 循环次数按钮数组
static lv_obj_t* interval_btns[5];     // 间隔按钮数组


static lv_obj_t* selected_cycle_btn = NULL;  // 跟踪当前选中的按钮
static uint8_t cycle_gap_num = 0;

static const LoopButtonConfig cycle_buttons[] = {
{"2次", LOOP_TIMES_2},
{"3次", LOOP_TIMES_3},
{"4次", LOOP_TIMES_4},
{"无限", LOOP_INFINITE}
};

/* 事件回调实现 */
static void cycle_btn_event_cb(lv_event_t * e) {
    lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* label = lv_obj_get_child(btn, 0);
    const char* txt = lv_label_get_text(label);

    // 查找对应的配置项
    LoopTimes new_mode = LOOP_INFINITE;
    for (int i = 0; i < 4; i++) {
        if (strcmp(txt, cycle_buttons[i].text) == 0) {
            new_mode = cycle_buttons[i].value;
            break;
        }
    }

    
    if (selected_cycle_btn == btn) {
        new_mode = LOOP_INFINITE;  // 点击已选按钮切换为无限
    }
    else {
       
        if (selected_cycle_btn) {
            lv_obj_set_style_text_opa(lv_obj_get_child(selected_cycle_btn, 0), LV_OPA_50, 0);
        }

        
        selected_cycle_btn = btn;
        lv_obj_set_style_text_opa(lv_obj_get_child(btn, 0), LV_OPA_100, 0);
    }

    play_para.loop_times = new_mode;
    initialize_repeat_params(&params, play_para.cycle_gap, play_para.loop_times);
    LV_LOG_USER("Loop times set to: %d", new_mode);
}

static lv_obj_t* selected_interval_btn = NULL;  // 跟踪当前选中的间隔按钮
static int interval_mode_num = 0;               // 记录当前选中的间隔模式




static void interval_btn_event_cb(lv_event_t * e) {
    lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* label = lv_obj_get_child(btn, 0);
    const char* txt = lv_label_get_text(label);





    if (selected_interval_btn != NULL) {
        lv_obj_t* prev_label = lv_obj_get_child(selected_interval_btn, 0);
        lv_obj_set_style_text_opa(prev_label, LV_OPA_50, 0);
    }



    lv_obj_set_style_text_opa(label, LV_OPA_100, 0);
    

    LV_LOG_USER("间隔设置: %s", txt);

    if (strcmp(txt, "0s") == 0) {
        interval_mode_num = 0;
    }
    else if (strcmp(txt, "1s") == 0) {
        interval_mode_num = 1;
    }
    else if (strcmp(txt, "2s") == 0) {
        interval_mode_num = 2;
    }
    else if (strcmp(txt, "3s") == 0) {
        interval_mode_num = 3;
    }
    else if (strcmp(txt, "4s") == 0) {
        interval_mode_num = 4;
    }

    LV_LOG_USER("mode_num: %d", interval_mode_num);
    play_para.cycle_gap = interval_mode_num;

    if (btn != selected_interval_btn) {
        initialize_repeat_params(&params, play_para.cycle_gap, play_para.loop_times);
        selected_interval_btn = btn;
    }
}

static void pause_switch_event_cb(lv_event_t * e) {
    bool state = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    LV_LOG_USER("句末暂停: %s", state ? "ON" : "OFF");

    player_func.pause_end_enable = state;
}



lv_obj_t* func_scr_panel_repeat = NULL;


static void repeat_switch_check(void)
{

    if (play_para.repeat_enabled) {

        lv_obj_clear_flag(cycle_label, LV_OBJ_FLAG_HIDDEN);
        for (int i = 0; i < 4; i++) lv_obj_clear_flag(cycle_btns[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(interval_label, LV_OBJ_FLAG_HIDDEN);
        for (int i = 0; i < 5; i++) lv_obj_clear_flag(interval_btns[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(pause_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sw_pause, LV_OBJ_FLAG_HIDDEN);

        lv_obj_set_size(func_scr_panel_repeat, 280, 262);
    }
    else {

        //lv_obj_add_flag(cycle_label, LV_OBJ_FLAG_HIDDEN);
        //for (int i = 0; i < 4; i++) lv_obj_add_flag(cycle_btns[i], LV_OBJ_FLAG_HIDDEN);
        //lv_obj_add_flag(interval_label, LV_OBJ_FLAG_HIDDEN);
        //for (int i = 0; i < 5; i++) lv_obj_add_flag(interval_btns[i], LV_OBJ_FLAG_HIDDEN);
        //lv_obj_add_flag(pause_label, LV_OBJ_FLAG_HIDDEN);
        //lv_obj_add_flag(sw_pause, LV_OBJ_FLAG_HIDDEN);


        create_repeat_settings_ui(func_scr);

        lv_obj_set_size(func_scr_panel_repeat, 280, lv_pct(11));
        lv_obj_scroll_to_y(func_scr, 0, LV_ANIM_OFF);
    }
   
}

static void sycle_switch_event_cb(lv_event_t* e) {
    bool state = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    LV_LOG_USER("复读开关: %s", state ? "ON" : "OFF");


   // play_para.repeat_enabled = state;//06.13 目前复读还不完全

    if (play_para.repeat_enabled)
    {
#if 0
        ab_cycle_func();
#endif
#ifdef C_PC_SIM
        initialize_repeat_params(&params, play_para.cycle_gap, play_para.loop_times);
#endif
    }
    else   ab_param_clean();
    repeat_switch_check();
}

void draw_line_on_screen(lv_obj_t* parent, int x, int y) {
    // 定义线的点数组 (起点和终点)
    static lv_point_t line_points[] = {
        {0, 0},      
        {254, 0}    
    };
    lv_obj_t* line = lv_line_create(parent);

    
    lv_obj_set_pos(line, x,y);  

  
    lv_line_set_points(line, line_points, 2);

    
    static lv_style_t line_style;
    lv_style_init(&line_style);
    lv_style_set_line_width(&line_style, 1);          
    lv_style_set_line_color(&line_style, lv_color_hex(0xFFFFFF)); 
    lv_style_set_line_rounded(&line_style, false);      
    lv_style_set_line_opa(&line_style, LV_OPA_10);

    lv_obj_add_style(line, &line_style, LV_PART_MAIN);

    lv_obj_clear_flag(line, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(line, LV_OBJ_FLAG_CLICKABLE);
}

void create_repeat_settings_ui(lv_obj_t * parent) {

#define GIRD_LAYOUT 0
    if (func_scr_panel_repeat)
    {
        lv_obj_del(func_scr_panel_repeat);
        func_scr_panel_repeat = NULL;
    }

    func_scr_panel_repeat = lv_obj_create(parent);
    lv_obj_set_style_pad_all(func_scr_panel_repeat, 0, 0);
    lv_obj_set_style_border_width(func_scr_panel_repeat, 0, 0);
    lv_obj_set_scrollbar_mode(func_scr_panel_repeat, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(func_scr_panel_repeat, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(func_scr_panel_repeat, 280, 262);
    lv_obj_set_style_radius(func_scr_panel_repeat, 13, 0);
    lv_obj_set_scrollbar_mode(func_scr_panel_repeat, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(func_scr_panel_repeat, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(func_scr_panel_repeat, LV_OPA_10, 0);
    lv_obj_set_style_text_color(func_scr_panel_repeat, lv_color_hex(0xFFFFFF), 0);

#if GIRD_LAYOUT
    lv_obj_set_layout(func_scr_panel_repeat, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_all(func_scr_panel_repeat, 15, 0);
    lv_obj_set_style_pad_row(func_scr_panel_repeat, 20, 0);
    lv_obj_set_style_pad_column(func_scr_panel_repeat, 15, 0);


    /* 定义网格列（4列布局） */
    static lv_coord_t col_dsc[] = { // LV_GRID_CONTENT
        45,  
        45,
        45,
        45,
        45,
        LV_GRID_TEMPLATE_LAST
    };

    /* 定义网格行（自适应高度） */
    static lv_coord_t row_dsc[] = {
        LV_GRID_CONTENT,  // 标题行
        LV_GRID_CONTENT,  // 循环次数标签
        LV_GRID_CONTENT,  // 循环次数按钮
        LV_GRID_CONTENT,  // 间隔标签
        LV_GRID_CONTENT,  // 间隔按钮
        LV_GRID_CONTENT,  // 句末暂停
        LV_GRID_TEMPLATE_LAST
    };
    lv_obj_set_grid_dsc_array(func_scr_panel_repeat, col_dsc, row_dsc);
#endif

    lv_obj_t* title = lv_label_create(func_scr_panel_repeat);
    lv_label_set_text(title, "单句复读设置");
    lv_obj_align_to(title, func_scr_panel_repeat, LV_ALIGN_TOP_LEFT, 13, 17);
#if GIRD_LAYOUT
    lv_obj_set_grid_cell(title,
        LV_GRID_ALIGN_START, 0, 4,
        LV_GRID_ALIGN_CENTER, 0, 1);
#endif
    lv_obj_t* sw_repeat = lv_switch_create(func_scr_panel_repeat);
    lv_obj_set_ext_click_area(sw_repeat, 15);
    lv_obj_add_event_cb(sw_repeat, sycle_switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
#if GIRD_LAYOUT
    lv_obj_set_grid_cell(sw_repeat,
        LV_GRID_ALIGN_START, 3, 2,
        LV_GRID_ALIGN_CENTER, 0, 1);
#else 
    lv_obj_align_to(sw_repeat, func_scr_panel_repeat, LV_ALIGN_TOP_RIGHT, -12, 13);
#endif
    if (play_para.repeat_enabled) lv_obj_add_state(sw_repeat, LV_STATE_CHECKED);
    else  lv_obj_clear_state(sw_repeat, LV_STATE_CHECKED);
    //------------------------------------------
    // 循环次数设置
    //------------------------------------------
    cycle_label = lv_label_create(func_scr_panel_repeat);
    lv_label_set_text(cycle_label, "循环次数");

#if GIRD_LAYOUT
    lv_obj_set_grid_cell(cycle_label,
        LV_GRID_ALIGN_START, 0, 4,
        LV_GRID_ALIGN_CENTER, 1, 1);
#else 
    lv_obj_align_to(cycle_label, func_scr_panel_repeat, LV_ALIGN_TOP_LEFT, 14, 55);

#endif

    for (uint8_t i = 0; i < 4; i++) {
        cycle_btns[i] = lv_btn_create(func_scr_panel_repeat);
        lv_obj_set_size(cycle_btns[i], 68, 50);
        lv_obj_set_style_bg_color(cycle_btns[i], lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(cycle_btns[i], LV_OPA_0, 0);
        lv_obj_set_style_shadow_width(cycle_btns[i], 0, 0);
        lv_obj_add_event_cb(cycle_btns[i], cycle_btn_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t* label = lv_label_create(cycle_btns[i]);
        lv_label_set_text(label, cycle_buttons[i].text);
        lv_obj_set_style_text_opa(label, LV_OPA_50, 0);
#if 0
        lv_obj_center(label);
#else
        lv_obj_align(label,LV_ALIGN_CENTER,-5,0);
#endif
#if GIRD_LAYOUT
        lv_obj_set_grid_cell(cycle_btns[i],
            LV_GRID_ALIGN_CENTER, i, 1,
            LV_GRID_ALIGN_CENTER, 2, 1);
#else
        lv_obj_align_to(cycle_btns[i], func_scr_panel_repeat, LV_ALIGN_TOP_LEFT,(i*70), 80);
#endif
        if (cycle_buttons[i].value == play_para.loop_times)
        {
            
            lv_obj_set_style_text_opa(label, LV_OPA_100, 0);
            selected_cycle_btn = cycle_btns[i];
        }
    }

    //------------------------------------------
    // 循环间隔设置
    //------------------------------------------
    interval_label = lv_label_create(func_scr_panel_repeat);
    lv_label_set_text(interval_label, "循环间隔");
#if GIRD_LAYOUT
    lv_obj_set_grid_cell(interval_label,
        LV_GRID_ALIGN_START, 0, 5,
        LV_GRID_ALIGN_CENTER, 3, 1);
#else
    lv_obj_align_to(interval_label, func_scr_panel_repeat, LV_ALIGN_TOP_LEFT, 14, 136);

#endif
    const char* interval_txt[] = { "0s", "1s", "2s", "3s", "4s" };
    for (int i = 0; i < 5; i++) {
        interval_btns[i] = lv_btn_create(func_scr_panel_repeat);
        lv_obj_set_size(interval_btns[i], 50, 30);
        lv_obj_set_style_bg_color(interval_btns[i], lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(interval_btns[i], LV_OPA_0, 0);
        lv_obj_set_style_shadow_width(interval_btns[i], 0, 0);
        lv_obj_add_event_cb(interval_btns[i], interval_btn_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t* label = lv_label_create(interval_btns[i]);
        lv_obj_set_style_text_opa(label, LV_OPA_50, 0);
        lv_label_set_text(label, interval_txt[i]);
        lv_obj_center(label);
#if GIRD_LAYOUT
        lv_obj_set_grid_cell(interval_btns[i],
            LV_GRID_ALIGN_CENTER, i, 1,
            LV_GRID_ALIGN_CENTER, 4, 1);
#else
        lv_obj_align_to(interval_btns[i], func_scr_panel_repeat, LV_ALIGN_TOP_LEFT, 2 + (i * 57), 168);

#endif
        if (i == play_para.cycle_gap)//test  
        {
            lv_obj_set_style_text_opa(label, LV_OPA_100, 0);
            selected_interval_btn = interval_btns[i];
        }
    }
    draw_line_on_screen(func_scr_panel_repeat, 13, 122);
    draw_line_on_screen(func_scr_panel_repeat, 13, 203);

    //------------------------------------------
    // 句末暂停设置
    //------------------------------------------
    pause_label = lv_label_create(func_scr_panel_repeat);
    lv_label_set_text(pause_label, "句末暂停");
#if GIRD_LAYOUT
    lv_obj_set_grid_cell(pause_label,
        LV_GRID_ALIGN_START, 0, 2,
        LV_GRID_ALIGN_CENTER, 5, 1);
#else
    lv_obj_align_to(pause_label, func_scr_panel_repeat, LV_ALIGN_TOP_LEFT, 13, 215);

#endif
    sw_pause = lv_switch_create(func_scr_panel_repeat);
    lv_obj_add_event_cb(sw_pause, pause_switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
#if GIRD_LAYOUT
    lv_obj_set_grid_cell(sw_pause,
        LV_GRID_ALIGN_START, 3, 2,
        LV_GRID_ALIGN_CENTER, 5, 1);
#else
    lv_obj_set_ext_click_area(sw_pause, 15);
    lv_obj_align_to(sw_pause, func_scr_panel_repeat, LV_ALIGN_TOP_RIGHT, -19, 215);
#endif
    if (player_func.pause_end_enable) lv_obj_add_state(sw_pause, LV_STATE_CHECKED);
    else lv_obj_clear_state(sw_pause, LV_STATE_CHECKED);
}


static void func_page_release()
{
    // 添加复读设置页面的释放
    if (func_scr_panel_repeat) {
        lv_obj_del(func_scr_panel_repeat);
        func_scr_panel_repeat = NULL;
    }

    // 重置所有静态指针
    selected_play_speed_btn = NULL;
    selected_play_mode_btn = NULL;
    selected_cycle_btn = NULL;
    selected_interval_btn = NULL;

    // 原有释放逻辑
    if (func_scr) {
        lv_obj_del(func_scr);
        func_scr = NULL;
    }
}


/**
 * @brief func页面按键回调
 */
static void func_page_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_KEY) {
        int32_t c = *((int32_t*)lv_event_get_param(e));

        if (c == LV_KEY_ESC) {
            lv_indev_keypad_wait_release();
            LV_LOG_USER("func page click esc\r\n");
#if 0
            func_page_release();
#else
            player_switchScr(PLAYER_SCR_MAIN, NULL);
#endif
        }
    }
}

/**
 * @brief del键回调函数
 */
static void del_chospage_event_cb(lv_event_t * e) {
    lv_indev_wait_release(lv_indev_get_act());
    ESP_LOGI(TAG, "del_chospage_event_cb");
#if 0
    func_page_release();
#else
    player_switchScr(PLAYER_SCR_MAIN, NULL);
#endif
}
/**
 * @brief 功能键新建窗口页面初始化
 */
static void func_chos_page_init(void* parm)
{
    // 重置静态变量，防止野指针
    selected_play_speed_btn = NULL;
    selected_play_mode_btn = NULL;
    selected_cycle_btn = NULL;
    selected_interval_btn = NULL;

    // 确保之前的页面完全释放
    func_page_release();  // 调用释放函数确保清理

    func_scr = lv_obj_create(lv_scr_act());
    lv_obj_set_style_pad_all(func_scr, 0, 0);
    lv_obj_set_style_pad_top(func_scr, 0, 0);
    lv_obj_set_size(func_scr, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_left(func_scr, 20, 0);
    lv_obj_set_style_bg_color(func_scr, lv_color_hex(0x1C1739), 0);
    lv_obj_set_flex_flow(func_scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(func_scr, 12, 0);
    lv_obj_set_style_pad_bottom(func_scr, 27, 0);
    lv_obj_set_style_border_width(func_scr, 0, 0);
    lv_obj_set_style_radius(func_scr, 0, 0);
    lv_obj_set_scrollbar_mode(func_scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_align(func_scr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* list = lv_obj_create(func_scr);
    lv_obj_set_style_bg_color(list, lv_color_hex(0x1C1739), 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_size(list, lv_pct(100), 55);
    lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* more_label = lv_label_create(list);
    lv_obj_set_size(more_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_text_color(more_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(more_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(more_label, LV_ALIGN_BOTTOM_LEFT,0,0);
    lv_label_set_text(more_label, "更多");

    lv_obj_t* del_btn = lv_obj_create(list);
    lv_obj_set_size(del_btn, btn_size+40, btn_size+80);
    //lv_obj_set_style_radius(del_btn, 17, 0);
    lv_obj_set_style_bg_color(del_btn, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_shadow_width(del_btn, 0, 0);
    lv_obj_set_style_border_width(del_btn, 0, 0);
    lv_obj_set_style_bg_opa(del_btn, LV_OPA_0, 0);
    lv_obj_t* img_del = lv_img_create(del_btn);
    lv_img_dsc_t* pImgDsc_del = zb_img_res_load_one(NULL, PLAYER_IMG_PLAYER_DEL, IMG_FREE_TYPE_AUTO, NULL);
    lv_img_set_src(img_del, pImgDsc_del);
    lv_obj_align(img_del,LV_ALIGN_RIGHT_MID, -20,0);
   // lv_obj_set_style_bg_img_src(del_btn, pImgDsc_del, 0);
    lv_obj_align_to(del_btn, more_label, LV_ALIGN_OUT_RIGHT_MID, SCREEN_WIDTH - 145, 0);
    lv_obj_add_event_cb(del_btn, del_chospage_event_cb, LV_EVENT_CLICKED, NULL);

    sys_setCurMainWinAndCb(func_scr, func_page_event_cb);

    create_function_speed_ui(func_scr);
    create_play_mode_ui(func_scr);
#if 0  //07.25 todo 暂时屏蔽
    //create_repeat_settings_ui(func_scr);
   // repeat_switch_check();
#endif
}


/**
 * @brief 获取后台播放器状态
 * @return -1 :表示没有后台播放
 *         1，表示播放状态
 *         2，表示暂停状态
 */
int get_player_backstage_state(void)
{
#ifndef C_PC_SIM



#else
    return -1;
#endif
}

/**
 * @brief 获取后台播放单元的名字
 * @return 后台有播放返回播放的歌曲名称，没有后台播放返回NULL
 */
char* get_backstage_play_unit_name(void)
{
    if (player_state_flag == PLAYER_BACKSTAGE_RUNNING)
    {
        if (play_text_book_info.pBookInf && play_text_book_info.pTile)
        {
            return play_text_book_info.pTile[play_text_state.cur_play_unit_num];
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief 后台播放或暂停
 * @return
 *          1：表示播放状态
 *          2：表示暂停状态
 */
int play_or_pause_backstage_unit(void)
{
#ifndef C_PC_SIM
    int ret = 0;
    if (player_state_flag == PLAYER_BACKSTAGE_RUNNING)
    {
        TZ_PLAYSTATE_T audStat = get_play_audio_state();
        switch (audStat)
        {
        case MUSIC_STATUS_PAUSE:
        {
            ESP_LOGI(PLA, "audio pause2\n");
            tz_play_resume_audio();
            if (refresh_timer)
            {
                lv_timer_resume(refresh_timer);
            }
            player_interval_flag = 1;
            ret = 1;
            break;
        }
        case TZ_PLAYING:
        {
            ESP_LOGI(PLA, "audio playing2\n");
            tz_play_pause_audio();
            lv_timer_pause(refresh_timer);
            player_interval_flag = 0;
            ret = 2;
            break;
        }
        default:
        {
            ret = 2;
            break;
        }
        }
    }
    else
    {
        ret = 2;
    }
    return ret;
#else
    return 2;
#endif
}

static const struct PLAYER_SCR_INF playerScrInfTbl[PLAYER_SCR_COUNT] = {
    [PLAYER_SCR_MAIN] = { MMN_player_page_init, play_text_win_free_keep },
    [PLAYER_SCR_PLAYLIST] = { create_playlist_screen, playlist_win_del },
    [PLAYER_SCR_FUNC_CHOOSE] = { func_chos_page_init, func_page_release },
};



void player_switchScr(PlayerScrNo scrNoNew, void* parm) {

    if (g_player_scrNo == scrNoNew) return;
    // 检查新页面编号是否有效
    if (scrNoNew < 0 || scrNoNew >= PLAYER_SCR_COUNT) {
        ESP_LOGE("PLAYER", "Invalid screen number: %d", scrNoNew);
        return;
    }

    // 获取新页面的初始化函数
    FN_PLAYER_SCREEN_INIT fnInit = playerScrInfTbl[scrNoNew].init;
    if (!fnInit) {
        ESP_LOGE("PLAYER", "Initialization function for screen %d is NULL", scrNoNew);
        return;
    }

    // 如果当前有页面，则释放当前页面
    if (g_player_scrNo >= 0) {
        FN_PLAYER_SCREEN_FREE fnFree = playerScrInfTbl[g_player_scrNo].free;
        if (fnFree) {
            fnFree();
        }
    }

    // 初始化新页面
    fnInit(parm);

    // 更新当前页面编号
    g_player_scrNo = scrNoNew;

}
