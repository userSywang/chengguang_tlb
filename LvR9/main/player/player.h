#ifndef PLAYER_H
#define PLAYER_H
#if 0
#include <stdio.h>
#include <string.h>
#include "../../lvgl/lvgl.h"

#include "../utils/zb_aud_api.h"
#include "player_data_parse.h"
#include "../utils/misc_utils.h"
#ifdef C_PC_SIM
#include "../data_parse/cJSON.h"
#include "../../components/lv_drivers/win32drv/win32drv.h"
#endif
#include "../data_parse/img_res.h"
#include "../include/img_a_tlb.h"
#include "../include/audio_media_def.h"
#include "../../lvgl/src/core/lv_event.h"
#include "../data_parse/analyze_listening.h"
#else


#include <stdio.h>
#include <string.h>
#include "../../lvgl/lvgl.h"

#include "../utils/zb_aud_api.h"
#include "player_data_parse.h"
#include "../utils/misc_utils.h"
#ifdef C_PC_SIM
#include "../data_parse/cJSON.h"
#include "../../components/lv_drivers/win32drv/win32drv.h"
#else
#include "LvR9/trd/tz_audio_player/include/tz_audio_player.h"
#endif
#include "../data_parse/img_res.h"
#include "../include/img_a_tlb.h"
#include "../include/audio_media_def.h"
#include "../../lvgl/src/core/lv_event.h"
#include "../data_parse/analyze_listening.h"

#endif

#define SCREEN_WIDTH 320
#define SCREEN_HIGH 480
#define PLAYER_LIST_TITLE_H 55
#define TEXT_CONT_SCROLL_STEP_Y 80

#define PLA ("PLAYER")
#define FONT_TITLE (&lv_font_montserrat_14)
#define FONT_BODY (&lv_font_montserrat_14)
#ifndef C_PC_SIM
#define PLAYER_DATA_PATH ("")
#define PLAYER_ONLINE_DATA_PATH "/userdata/_data/textbook/" 
#else
#define PLAYER_DATA_PATH ("")
#define PLAYER_ONLINE_DATA_PATH "/userdata/_data/textbook/" 

#define lowndoad_player_test 1
#define btn_size 50

#endif

enum
{
    ENGLISH = 0,      // 英语教材
    OLD_POEM,         // 古诗古文
    FAMOUS_BOOK,      // 必读名著
    CHILD_SONG,       // 儿歌
    STORY_KINGDOM,    // 故事王国
    CHINESE_CLASSIC,  // 国学熏听
    BEAUT_ESSAY,      //  美文美句
    PAULIN_UNCLE,     // 保林叔叔
    CHINESE_TEXTBOOK, // 语文教材
    HYNEMAN_READ,     // 海尼曼阅读 
    HISTORICAL_STORY, // 历史故事 
    IDIOM_PARADISE,   // 成语乐园 
    BAIKE_ZHISHI,     // 百科知识 
    CAISHANG_JIAOYU,  // 财商教育 
    SITUATIONAL_DIALOGUE, // 情景对话 
};

/**
 * @brief 播放列表项结构(定义播放列表中的单个项目)
 */
typedef struct {
    MusicSourceType source_type;  // 资源来源
    char* location;               //路径
    int32_t cover;                // 封面标识
    char* title;                  // 标题
    uint16_t playNum;             // 播放序号
} PlaylistItem;

/**
 * @brief 播放列表结构 (管理整个播放列表)
 */
typedef struct {
    PlaylistItem* items;          // 播放项数组
    uint16_t count;               // 总项数
    uint16_t current_index;       // 当前播放索引
} Playlist;


// 主页播放路径定位
typedef struct
{
    char* path;
    int32_t cover;
} player_class_locate_t;

// 目录播放定位信息
typedef struct
{
    char path[500];
    char title[200];
    uint16_t playNum;
    int32_t cover;
} play_book_locate_t;

typedef struct {
    bool is_requesting;      // 是否正在请求媒体资源
    int requesting_index;    // 当前请求的索引
    uint8_t retry_count;     // 当前重试次数
    char* requesting_path;  // 保存请求时的路径
    MusicSourceType type;    //请求的音频类型
} PlayerRequestState;

// 播放状态
typedef struct
{
    uint16_t current_play_sentence_num;// 当前播放的句子序号
    uint16_t cur_play_unit_num;// 当前播放的单元序号（歌曲在播放列表中的索引） //music_play_cur
    uint8_t current_loop_times; // 当前播放时间（毫秒）
    uint64_t cur_time_ps;  // 当前音频播放位置, 休眠后恢复之用。
    lv_obj_t* pre_label; // 前一个句子的标签对象
    uint16_t pre_index;  // 前一个句子的序号（用于界面更新）
    TZ_PLAYSTATE_T last_state; // 上一次的播放状态

    uint16_t collect_id;//在数据库的位置(主要用于删除)
} play_text_state_t;

// 播放器风格
typedef enum PlayerStyle
{
    POEM_STYLE,   // 诗词风格
    PURPLE_STYLE, // 紫色风格
} player_style_e;

// 播放器风格
typedef struct
{
    uint32_t page_bg_img1;     // 页面背景1
    uint32_t page_bg_img2;     // 页面背景2
    uint32_t btn_def_color;    // 按钮默认颜色
    uint32_t btn_focus_color;  // 按钮聚焦颜色
    uint32_t text_def_color;   // 正文默认颜色
    uint32_t text_focus_color; // 正文聚焦颜色
    uint32_t record_btn_img1;  // 录音按钮图片1
    uint32_t record_motion_img[4];  // 录音按钮图片2
    uint32_t no_content_img;   // 无内容图片
    uint32_t aud_img;          // 音频图标

    uint32_t lyric_text_color; //歌词文本颜色
} player_style_t;


typedef struct {
    lv_obj_t* home_play_btn;     // 主页面播放按钮对象
    void (*update_callback)(TZ_PLAYSTATE_T); // 状态更新回调函数
    bool is_home_active;          // 主页面是否活跃
} player_ui_state_t;

typedef struct {
    TZ_PLAYSTATE_T state;
    lv_obj_t* play_btn;
} AsyncUpdateData;

extern player_ui_state_t g_player_ui_state;


extern player_style_t player_poem_style;
extern player_style_t player_normal_style;
extern player_style_t player_style;
extern player_style_t player_style_mao;
extern player_style_t player_style_chs_classic;

extern int play_main_page_init(int parm);
extern int play_list_page_init(int parm);
extern int play_text_page_init(int parm, int extra);
extern int play_text_return_init(int parm);
extern int play_repeat_page_init(int parm);
// int play_list_json_parse(char* path, char** pjson_str);

extern int play_main_page_free(void);
extern int play_list_page_free(void);
extern int play_text_page_free(void);
extern int play_text_win_free(void);
extern int player_repeat_follow_page_init(int parm);
extern int play_repeat_page_free(void);
extern int play_main_page_hander(lv_event_code_t msg, int param);

extern int play_main_online_page_init(int classNum, char direction);

extern int zb_ap_play_main_page_init(void);

extern int zb_ap_english_textbook_page_init(void);
extern int zb_ap_chinese_textbook_page_init(void);


extern int play_text_search_init(char* class_path, uint16_t class_num, uint16_t list_num, int16_t page_num);
extern int get_player_backstage_state(void);
extern char* get_backstage_play_unit_name(void);
extern int play_or_pause_backstage_unit(void);


extern void play_or_pause_func(void);
extern int play_next_unit(void);
extern int play_pre_unit(void);
extern int MMN_player_page_init();


extern lv_obj_t* play_main_win;   // 播放主页面容器
extern lv_obj_t* play_list_win;   // 播放列表页面容器
extern lv_obj_t* play_text_win;   // 播放文本内容页面容器
extern lv_obj_t* play_repeat_win; // 跟读页面容器

extern play_book_locate_t play_list_book_locate;
extern play_book_locate_t play_text_book_locate;
extern Playlist current_playlist;

extern CollectionService* s_collection_service;

extern int player_class_num;
extern int play_list_clicked_btn_num;
extern uint8_t play_main_clicked_btn_num;
extern uint8_t play_main_clicked_btn_num_pre;

extern player_class_locate_t player_class_locate;
extern int sendMsgToCurrentAp(lv_event_code_t evtCode, int param);

extern void media_playback_handler(const char* media_url);
extern void request_media_playback(const char* media_path, MusicSourceType source_type);


extern void initMusicArray(void);
extern void play_text_win_init(void* parm);

extern void free_global_playlist();
extern int play_album_page_init(char* path, uint16_t play_num, int16_t ap_num);
extern int play_album_online_page_init(Playlist* playlist, uint16_t start_index, int16_t ap_num, bool extra);
extern PlaylistItem create_playlist_item(MusicSourceType type, const char* location, const char* title, int32_t cover);

extern PlayerRequestState g_player_request_state;

extern void player_request_init();
extern bool player_acquire_request_lock(int index, const char* path,MusicSourceType source_type);
extern void player_release_request_lock();
extern int player_increment_retry_count();

extern void MMN_player_enter(void);
extern int player_is_play();
bool player_has_valid_data(void);
//替换声明
int audio_stream_stop_play(void);
int audio_stream_set_play_speed(music_play_speed_t speed);
int audio_stream_set_resume(void);
int audio_stream_set_pause(void);
music_play_status_t audio_stream_get_play_status(void);
void audio_stream_player_seek(int seek_ms);


void split_hash_path(const char* combined, char** hash, char** path);
void free_split_result(char* hash, char* path);



#endif
