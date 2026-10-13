#ifndef AI_CHAT_H
#define AI_CHAT_H

extern "C" {
#include <stdio.h>
#include "../utils/misc_utils.h"
#include <string.h>
#include <sys/stat.h>
#include "../mainmenu/tlb_main_img.h"
#include <lvgl/lvgl.h>
#include "../zb_base.h"
#include "../ex_widget/app_msg.h"
#include "../include/img_a_tlb.h"
#include "../data_parse/img_res.h"
}
#include <memory>
#include "json_parse_struct.h"

#define		MMENU_AI_CHAT			0	//
#define 	MMENU_AI_CHAT_VIEW		1	//
#define 	MMENU_AI_CHAT_NETWORK_DISCON		2	//

#define AICHAT_ESC_BTN_LOC_Y  10
#define TOP_BTN_BORDER 7
#define TOP_BTN_TOP_SPACE_H 8

extern const lv_img_dsc_t  g_XXX_img_dsc_tbl[];

extern lv_obj_t  * g_chatMnuBgWin;
extern uint8_t g_chatCurWin;
extern bool g_isInSubItem;
extern int8_t g_gradeSel;

extern std::unique_ptr<GetDefaultTopicResponse> g_defaultTopicRes;
extern std::unique_ptr<GenerateDialogResponse> g_generateDialogRes;
extern std::unique_ptr<GenerateTopicResponse> g_generateTopicRes;
extern std::unique_ptr<GenerateRecommendationResponse> g_generateRecommendationRes;
extern std::vector<HistoryItem> g_chatHistory;

extern std::unique_ptr<EnZhTranslationResponse> g_enzhTranslationRes;
extern std::unique_ptr<AsrResponse> g_AsrRes;
extern std::unique_ptr<TtsResponse> g_TtsRes;


int ai_chat_ap_win_deinit(void);

int ai_chat_menu_scr_init(int parm);
int ai_chat_view_init(uint8_t selGrade,std::string selClass,std::string selTopic);
void ai_chat_view_del(lv_event_t* e); 

	
void _ai_chat_scr_cb(lv_event_t* evt);
void aiChat_createEscBtn(lv_obj_t * parent, lv_event_cb_t clickCb, int extra);

bool get_is_net_connected();
bool ai_chat_get_default_topic();
bool ai_chat_generate_topic(std::string& topic);
bool ai_chat_generate_dialog(const std::string& taskId,
    const Scene& scene,
    const std::string& userLevel,
    const std::vector<HistoryItem>& history);
bool ai_chat_generate_recommendation(const std::string& taskId,
    const Scene& scene,
    const std::string& userLevel,
    const std::vector<HistoryItem>& history,
    const std::string& count,
    const std::vector<std::string>& indexArr);
bool ai_chat_enzh_translation(const std::string& q);
bool ai_chat_asr();
bool ai_chat_tts(const std::string& q,const std::string& language);

void wav_file_check_and_delete();
void ai_chat_take_audio_start_5s();
void play_tts_audio();

//——————————

#define AICHAT_AP_BG_COLOR_HEX  0xffffff

const unsigned int scene_icon[3][25]{
    {
        AI_CHAT_IMG_LLEVELICON1,
        AI_CHAT_IMG_LLEVELICON2,
        AI_CHAT_IMG_LLEVELICON3,
        AI_CHAT_IMG_LLEVELICON4,
        AI_CHAT_IMG_LLEVELICON5,
        AI_CHAT_IMG_LLEVELICON6,
        AI_CHAT_IMG_LLEVELICON7,
        AI_CHAT_IMG_LLEVELICON8,
        AI_CHAT_IMG_LLEVELICON9,
        AI_CHAT_IMG_LLEVELICON10,
        AI_CHAT_IMG_LLEVELICON11,
        AI_CHAT_IMG_LLEVELICON12,
        AI_CHAT_IMG_LLEVELICON13,
        AI_CHAT_IMG_LLEVELICON14,
        AI_CHAT_IMG_LLEVELICON15,
        AI_CHAT_IMG_LLEVELICON16,
        AI_CHAT_IMG_LLEVELICON17,
        AI_CHAT_IMG_LLEVELICON18,
        AI_CHAT_IMG_LLEVELICON19,
        AI_CHAT_IMG_LLEVELICON20,
        AI_CHAT_IMG_LLEVELICON21,
        AI_CHAT_IMG_LLEVELICON22,
        AI_CHAT_IMG_LLEVELICON23,
        AI_CHAT_IMG_LLEVELICON24,
        AI_CHAT_IMG_LLEVELICON25
    },
    {
        AI_CHAT_IMG_MLEVELICON1,
        AI_CHAT_IMG_MLEVELICON2,
        AI_CHAT_IMG_MLEVELICON3,
        AI_CHAT_IMG_MLEVELICON4,
        AI_CHAT_IMG_MLEVELICON5,
        AI_CHAT_IMG_MLEVELICON6,
        AI_CHAT_IMG_MLEVELICON7,
        AI_CHAT_IMG_MLEVELICON8,
        AI_CHAT_IMG_MLEVELICON9,
        AI_CHAT_IMG_MLEVELICON10,
        AI_CHAT_IMG_MLEVELICON11,
        AI_CHAT_IMG_MLEVELICON12,
        AI_CHAT_IMG_MLEVELICON13,
        AI_CHAT_IMG_MLEVELICON14,
        AI_CHAT_IMG_MLEVELICON15,
        AI_CHAT_IMG_MLEVELICON16,
        AI_CHAT_IMG_MLEVELICON17,
        AI_CHAT_IMG_MLEVELICON18,
        AI_CHAT_IMG_MLEVELICON19,
        AI_CHAT_IMG_MLEVELICON20,
        AI_CHAT_IMG_MLEVELICON21,
        AI_CHAT_IMG_MLEVELICON22,
        AI_CHAT_IMG_MLEVELICON23
    },
    {
        AI_CHAT_IMG_HLEVELICON1,
        AI_CHAT_IMG_HLEVELICON2,
        AI_CHAT_IMG_HLEVELICON3,
        AI_CHAT_IMG_HLEVELICON4,
        AI_CHAT_IMG_HLEVELICON5,
        AI_CHAT_IMG_HLEVELICON6,
        AI_CHAT_IMG_HLEVELICON7,
        AI_CHAT_IMG_HLEVELICON8,
        AI_CHAT_IMG_HLEVELICON9,
        AI_CHAT_IMG_HLEVELICON10,
        AI_CHAT_IMG_HLEVELICON11,
        AI_CHAT_IMG_HLEVELICON12,
        AI_CHAT_IMG_HLEVELICON13,
        AI_CHAT_IMG_HLEVELICON14,
        AI_CHAT_IMG_HLEVELICON15,
        AI_CHAT_IMG_HLEVELICON16,
        AI_CHAT_IMG_HLEVELICON17,
        AI_CHAT_IMG_HLEVELICON18,
        AI_CHAT_IMG_HLEVELICON19,
        AI_CHAT_IMG_HLEVELICON20
    }
};

#endif