#ifndef _PLAYER_TEXT_H
#define _PLAYER_TEXT_H

#include "player.h"
#include "../include/key_driver.h"
#include "../../main/utils/misc_utils.h"
#include "jump_dic.h"

#define MY_AUD_FOR_BACK_DIFF 10000

typedef enum {
    PLAYER_SCR_MAIN = 0,          // 主播放页面
    PLAYER_SCR_PLAYLIST,          // 播放列表页面
    PLAYER_SCR_FUNC_CHOOSE,       // 功能选择页面
    PLAYER_SCR_REPEAT,            // 跟读页面
    PLAYER_SCR_COUNT              // 页面总数
} PlayerScrNo;



typedef enum {
    LOOP_MODE_ORDER = 1,  //顺序播放  (只播放一次列表)
    LOOP_MODE_RANDOM,     //随机播放
    LOOP_MODE_SINGLE,     //单曲循环
    LOOP_MODE_ALL         //列表播放  (循环播放列表)
} LoopMode;

typedef enum {
    LOOP_TIMES_2 = 2,
    LOOP_TIMES_3 = 3,
    LOOP_TIMES_4 = 4,
    LOOP_INFINITE = 5,  // 0表示无限
} LoopTimes;
// 按钮配置结构体
typedef struct {
    const char* text;
    LoopTimes value;
} LoopButtonConfig;

// 播放界面的收藏，倍速等按钮的功能信息
typedef struct
{
    uint8_t elementNum;
    uint32_t imgNum[8];
    char* elmentStr[8];
    uint16_t offset_x;
    uint16_t offset_y;
    void (*event_cb[8])(lv_event_t* e);
} play_function_img_info_t;


/**
 * @brief 播放器播放参数设置
 */
typedef struct
{
    float speed;        // 播放速度
    LoopMode loopmode;     //循环模式
    LoopTimes loop_times;   // 循环次数
    uint8_t cycle_gap;    // 循环间隔

    int current_index;     // 当前播放索引
    int cur_loop_count;        // 当前循环次数
    int last_random;       // 最后随机索引（用于随机模式）
    bool repeat_enabled;   // 复读开关状态
} player_para_set_t;





/**
 * @brief 播放功能的设置
 */
typedef struct
{
    bool play_enable;         // 播放enable
    bool next_unit_enable;    // 下一首enable
    bool pre_unit_enable;     // 上一首enable
    bool play_continu_enable; // 继续播放enable
    bool record_enable;       // 录音使能
    bool play_exit;           // 播放退出
    bool find_enable;         // 查词使能
    bool repeat_single_enable; // 单句复读enable
    bool pause_end_enable;     // 句末暂停enable
} player_func_t;

/**
 * @brief 管理AB复读功能的状态
 */
typedef struct
{
    uint64_t start_pos;     // ab循环播放的起点位置（时间戳或句子编号）
    uint64_t end_pos;       // ab循环播放的终点位置（时间戳或句子编号）
    uint32_t tt_len;        // 播放内容的总时长（毫秒为单位）
    uint16_t play_num_bak;  // 用于保存当前播放单元编号的备份
    uint8_t flag;           // 状态标志，用于控制ab循环播放的流程状态（例如：0表示未设置起点，1表示已设置起点但未设置终点，2表示已设置终点等）
    uint8_t cycle_enable;   // 循环播放使能标志，1表示启用循环播放，0表示未启用    
} ab_cycle_param_t;          //复读功能键参数设置

typedef struct {
    uint32_t start_pos; // 复读起始位置
    uint32_t interval;  // 循环间隔时间（秒）
    uint32_t repeat_count; // 循环次数（0 表示无限循环）  应该-1表示无限循环
    uint32_t current_count; // 当前已循环次数
    bool is_repeating; // 是否正在复读
    lv_timer_t* timer; // 定时器指针

    int cnt_time;
    int cnt_sec;
} RepeatParams;

enum
{
    PLAYER_BACKSTAGE_STOP,
    PLAYER_BACKSTAGE_RUNNING,
} player_backstage_state_e;


/* 定义异步回调数据结构 */
typedef struct {
    uint32_t start_time;
    uint32_t end_time;
    lv_timer_t* refresh_timer;
} AsyncData;

extern int zb_global_str_write(const char* pcText);

static void play_or_pause_btn_click_cb(lv_event_t* e);
static void collect_btn_click_cb(lv_event_t* e);
static void speed_btn_event_cb(lv_event_t* e);
static void find_btn_click_cb(lv_event_t* e);
static void loop_btn_click_cb(lv_event_t* e);
static void single_cycle_btn_click_cb(lv_event_t* e);


static void back_btn_click_cb(lv_event_t* e);
static void playlist_btn_click_cb(lv_event_t* e);
static void function_btn_click_cb(lv_event_t* e);
static void previous_track_btn_click_cb(lv_event_t* e);
static void next_track_btn_click_cb(lv_event_t* e);
static void read_aloud_btn_click_cb(lv_event_t* e);

extern play_text_state_t play_text_state;

//播放列表
static void list_return_btn_click_cb(lv_event_t* e);

int play_text_win_free_keep(void);

int play_text_search_init(char* class_path, uint16_t class_num, uint16_t list_num, int16_t page_num);
int play_single_page_init(char* path, uint16_t play_num, int16_t ap_num, uint8_t creat_group);
int play_recently_or_collect_page_init(char* path, uint16_t play_num, int16_t ap_num, uint8_t list_num, uint8_t creat_group);

static void func_chos_page_init();
void start_repeating(uint32_t start_pos, uint32_t end_pos, uint32_t interval, uint32_t repeat_count);
static void repeat_timer_callback(lv_timer_t* timer);
#endif
