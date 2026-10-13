#ifndef _PLAYER_REPEAT_H
#define _PLAYER_REPEAT_H

//#include "../../components/lvgl/lvgl.h"
#include "../utils/misc_utils.h"
#ifndef C_PC_SIM
#include <sys/types.h>
#include <limits.h>
#include "../include/audio_stream.h"  
#include "../include/audio_record.h"
//#include "../managed_components/espressif__esp-dsp/modules/common/include/dsp_platform.h"
#include "../include/audio_media_def.h"
//#include "../include/audio_player.h"
//#include "app_codec.h"

//#include "../include/bsp/esp-bsp.h"
#endif

#include "../utils/zb_aud_api.h"
//#include "../utils/zb_record_api.h"
#include "player.h"
//#include "../include/key_driver.h"
//#include "../include/app_codec.h"


#define REP "playerRepeat"
#define MY_REC_SAMPLE_RATE 16000

extern player_unit_info_t play_unit_info;
extern play_text_state_t play_text_state;

static void play_record(void);

const char* TTS_APP_KEY = "YOUR_APP_KEY"; 
const char* TTS_APP_SECRET = "YOUR_APP_SECRET"; 

typedef struct {
    char* q;
    char* appKey;
    char* signType;
    char* curtime;
    char* salt;
    char* sign;
    char* speed;
    char* volume;
    char* format;
    char* voiceName;
} TtsRequest;



// 跟读状态枚举
typedef enum {
    FOLLOW_IDLE,        // 未开始
    FOLLOW_RECORDING,   // 录音中
    FOLLOW_EVALUATING,  // 评估中
    FOLLOW_RESULT       // 评估完成
} follow_state_t;

// 评估结果结构
typedef struct {
    int score;          // 得分 (0-100)
    int pronunciation;  // 发音评分
    int fluency;        // 流畅度
    char feedback[128]; // 评语
} follow_result_t;

// 跟读上下文
typedef struct {
    lv_obj_t* win;              // 主窗口
    lv_obj_t* state_img;        // 状态图标（喇叭/录音动画）
    lv_obj_t* word_label;       // 单词标签
    lv_obj_t* result_label;     // 结果标签
    lv_obj_t* score_label;      // 得分标签
    lv_obj_t* feedback_label;   // 反馈标签
    lv_obj_t* retry_btn;        // 重试按钮
    lv_obj_t* return_btn;       // 返回按钮
    lv_obj_t* title_label;      // 标题标签
    lv_obj_t* sentence_num_label; // 句子数量标签

    follow_state_t state;       // 当前状态
    const char* current_word;   // 当前单词
    follow_result_t result;     // 评估结果

    lv_timer_t* anim_timer;     // 动画定时器
    int anim_frame;             // 动画帧索引
} follow_context_t;



//void player_repeat_follow_page_free(void);
void player_repeat_set_word(const char* word);  // 设置当前单词


extern lv_img_dsc_t speak_out_big;  // 大喇叭图标
extern lv_img_dsc_t enterpress;

extern lv_img_dsc_t speaking1;
extern lv_img_dsc_t speaking2;
extern lv_img_dsc_t speaking3;
extern lv_img_dsc_t speaking4;
extern lv_img_dsc_t speak_out_icon;

#endif
