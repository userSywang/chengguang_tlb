/*******************************************************
                    跟读页面程序
******************************************************/

#include "player_repeat.h"
#include "../utils/encode_utils.h"


static uint16_t current_sentence_num = 0;
static uint16_t total_sentence_num = 0;


#define btn_size 50
static follow_context_t g_follow_ctx = { 0 };

static const lv_img_dsc_t* state_imgs[] = {
    &enterpress,          // IDLE状态
    &speaking1,          // RECORDING状态帧0
    &speaking2,          // RECORDING状态帧1
    &speaking3,          // RECORDING状态帧2
    &speaking4,        // EVALUATING状态
    &speak_out_icon             // RESULT状态
};

static void update_ui_state();
static void record_timer_cb(lv_timer_t* t);
static void follow_start_record(void);
void follow_stop_record();
int player_repeat_follow_page_init(int parm);
void player_repeat_handle_evaluate_result(int score, int pron, int fluency, const char* feedback);

/**
 * @brief 上一个句子显示
 */
static void locate_pre_sentence(void)
{
    current_sentence_num = (current_sentence_num - 1 + total_sentence_num) % total_sentence_num;
    lv_label_set_text_fmt(g_follow_ctx.sentence_num_label, "%hu/%hu", current_sentence_num + 1, total_sentence_num);
    lv_label_set_text(g_follow_ctx.sentence_num_label, play_unit_info.pOrginal[current_sentence_num]);
}

/**
 * @brief 下一个句子显示
 */
static void locate_next_sentence(void)
{
    current_sentence_num = (current_sentence_num + 1) % total_sentence_num;
    lv_label_set_text_fmt(g_follow_ctx.sentence_num_label, "%hu/%hu", current_sentence_num + 1, total_sentence_num);
    lv_label_set_text(g_follow_ctx.sentence_num_label, play_unit_info.pOrginal[current_sentence_num]);
}

/**
 * @brief 播放当前句子
 */
static void play_or_stop_current_sentence(void)
{
    uint32_t ttlTm = 0, posTm = 0;
    music_play_status_t playStat;

    printf("current_sentence_num = %d,ptInTm = %d\n", (int)current_sentence_num, (int)play_unit_info.ptInTm[current_sentence_num]);
}

/**
 * @brief 播放
 */
static void play_timer_cb(lv_timer_t *t)
{

}

//================================================

// 设置当前单词
void player_repeat_set_word(const char* word) {
    g_follow_ctx.current_word = word;
    if (g_follow_ctx.word_label) {
        lv_label_set_text(g_follow_ctx.word_label, word ? word : "");
    }
}

// 喇叭按钮点击回调
static void _speak_out_big_click_cb(lv_event_t* evt) {
    if (g_follow_ctx.current_word) {
        // TODO: 调用TTS播放单词发音
        // Ap_tts_gen(current_word, "en", NULL);
    }
}

static void on_repeat_img_pressed(lv_event_t* e) {
    if (g_follow_ctx.state == FOLLOW_IDLE) {
        follow_start_record();
    }
}

// 录音按钮释放回调
static void on_repeat_img_released(lv_event_t* e) {
    if (g_follow_ctx.state == FOLLOW_RECORDING) {
        follow_stop_record();
    }
}

// 重试按钮点击回调
static void on_retry_btn_clicked(lv_event_t* e) {
    g_follow_ctx.state = FOLLOW_IDLE;
    update_ui_state();
}


// 定义定时器回调函数
static void eval_timer_cb(lv_timer_t *timer)
{
    // 生成随机的评分数据
    int score = 70 + rand() % 30;
    int pron = 60 + rand() % 40;
    int fluency = 65 + rand() % 35;
    const char* feedback = "发音基本正确，注意元音长度";
    
    player_repeat_handle_evaluate_result(score, pron, fluency, feedback);
    
    lv_timer_del(timer);
}


//增加录音动态显示
static void record_timer_cb(lv_timer_t *t)
{
    g_follow_ctx.anim_frame++;
    update_ui_state();

}

/**
 * @brief 启动录音
 */
static void follow_start_record(void)
{
	printf("%s:[%d]\n", __FUNCTION__, __LINE__);
    if (g_follow_ctx.state != FOLLOW_IDLE) return;

    g_follow_ctx.state = FOLLOW_RECORDING;

    if (!g_follow_ctx.anim_timer) {
        g_follow_ctx.anim_timer = lv_timer_create(record_timer_cb, 300, NULL);
    }

    update_ui_state();

}

// 停止录音
void follow_stop_record() {
    if (g_follow_ctx.state != FOLLOW_RECORDING) return;

    g_follow_ctx.state = FOLLOW_EVALUATING;

    if (g_follow_ctx.anim_timer) {
        lv_timer_pause(g_follow_ctx.anim_timer);
    }

    update_ui_state();

    printf("停止录音，开始评估...\n");

    lv_timer_t* eval_timer = lv_timer_create(eval_timer_cb, 3000, NULL);
}

// 开始录音
void audio_record_start() {

}


/**
 * @brief 播放录音
 */
static void play_record(void)
{

}

// 处理评估结果
void player_repeat_handle_evaluate_result(int score, int pron, int fluency, const char* feedback) {
    g_follow_ctx.state = FOLLOW_RESULT;
    g_follow_ctx.result.score = score;
    g_follow_ctx.result.pronunciation = pron;
    g_follow_ctx.result.fluency = fluency;
    strncpy(g_follow_ctx.result.feedback, feedback,
        sizeof(g_follow_ctx.result.feedback) - 1);

    // 更新UI
    char score_str[32];
    snprintf(score_str, sizeof(score_str), "得分: %d", score);
    lv_label_set_text(g_follow_ctx.score_label, score_str);

    lv_label_set_text(g_follow_ctx.feedback_label, feedback);
    update_ui_state();
}

/**
 * @brief 页面key事件
 */
static void play_repeat_win_key_cb(lv_event_t *e)
{
    int keyVal = 0;
    lv_event_code_t evtCode = e->code;
    keyVal = lv_event_get_key(e);
    if (evtCode == LV_EVENT_KEY_STAGE)
    {
        // 长按
        if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_LONG)
        {
            keyVal &= ~LV_EX_FLAG_KEY_MASK;
            switch (keyVal)
            {
            case LV_KEY_EX_RECORD:
            {
                printf("%s:[%d],start record\n", __FUNCTION__, __LINE__);
                follow_start_record();
                break;
            }
            }
        }
        // 长按弹起
        else if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_LONG_UP)
        {
            keyVal &= ~LV_EX_FLAG_KEY_MASK;
            switch (keyVal)
            {
	            case LV_KEY_EX_RECORD:
	            {
#ifndef C_PC_SIM
	               // zb_record_request_stop();
#endif
			printf("%s:[%d]\n", __FUNCTION__, __LINE__);
	            }
            }
        }
    }
    else if (LV_EVENT_KEY == evtCode)
    {
        switch (keyVal)
        {
        case LV_KEY_ESC:
        {
            play_repeat_page_free();
            play_text_return_init(0);
            break;
        }
        }
    }
}

static void repeat_return_btn_click_cb(lv_event_t* e) //需要做到页面切换
{
    play_repeat_page_free();
    play_text_return_init(0);
    LV_LOG_USER("repeat_return_btn_click_cb\r\n");
}



/**
 * @brief 跟读界面释放
 */
int play_repeat_page_free(void)
{
#ifndef C_PC_SIM
    tz_play_stop_audio();
   // zb_record_request_stop();
    // music_player_destroy();
    printf("play_repeat_page_free1\n");
#endif
    printf("play_repeat_page_free2\n");
    if (play_repeat_win)
    {
        lv_obj_del(play_repeat_win);
        play_repeat_win = NULL;
    }

    if (g_follow_ctx.anim_timer) {
        lv_timer_del(g_follow_ctx.anim_timer);
        g_follow_ctx.anim_timer = NULL;
    }

    if (g_follow_ctx.win) {
        lv_obj_del(g_follow_ctx.win);
        g_follow_ctx.win = NULL;
    }

    memset(&g_follow_ctx, 0, sizeof(follow_context_t));
    return 0;
}


// 更新UI状态
static void update_ui_state() {
    // 更新状态图标
    switch (g_follow_ctx.state) {
    case FOLLOW_IDLE:
        lv_img_set_src(g_follow_ctx.state_img, state_imgs[0]);
       
        lv_obj_add_flag(g_follow_ctx.result_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_follow_ctx.score_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_follow_ctx.feedback_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_follow_ctx.retry_btn, LV_OBJ_FLAG_HIDDEN);
        break;
    case FOLLOW_RECORDING:
        
        lv_img_set_src(g_follow_ctx.state_img,
            state_imgs[1 + (g_follow_ctx.anim_frame % 3)]);
      
        lv_obj_add_flag(g_follow_ctx.result_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_follow_ctx.score_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_follow_ctx.feedback_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_follow_ctx.retry_btn, LV_OBJ_FLAG_HIDDEN);
        break;
    case FOLLOW_EVALUATING:
        lv_img_set_src(g_follow_ctx.state_img, state_imgs[4]);
       
        //lv_obj_add_flag(g_follow_ctx.result_label, LV_OBJ_FLAG_HIDDEN);
        //lv_obj_add_flag(g_follow_ctx.score_label, LV_OBJ_FLAG_HIDDEN);
        //lv_obj_add_flag(g_follow_ctx.feedback_label, LV_OBJ_FLAG_HIDDEN);
        //lv_obj_add_flag(g_follow_ctx.retry_btn, LV_OBJ_FLAG_HIDDEN);
        break;
    case FOLLOW_RESULT:
        lv_img_set_src(g_follow_ctx.state_img, state_imgs[5]);
        
        lv_obj_clear_flag(g_follow_ctx.result_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_follow_ctx.score_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_follow_ctx.feedback_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_follow_ctx.retry_btn, LV_OBJ_FLAG_HIDDEN);
       
        char score_str[32];
        snprintf(score_str, sizeof(score_str), "得分: %d", g_follow_ctx.result.score);
        lv_label_set_text(g_follow_ctx.score_label, score_str);
        lv_label_set_text(g_follow_ctx.feedback_label, g_follow_ctx.result.feedback);
        break;
    }
}

// 初始化跟读页面
int player_repeat_follow_page_init(int parm) {

    if (g_follow_ctx.win) {
        play_repeat_page_free();
    }


    memset(&g_follow_ctx, 0, sizeof(follow_context_t));


    const char* word = play_unit_info.pOrginal[current_sentence_num];
    g_follow_ctx.current_word = word;

 
    g_follow_ctx.win = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_follow_ctx.win, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(g_follow_ctx.win, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(g_follow_ctx.win, 0, 0);
    lv_obj_set_style_radius(g_follow_ctx.win, 0, 0);

    sys_setCurMainWinAndCb(g_follow_ctx.win, play_repeat_win_key_cb);

   
    lv_obj_t* top_title_win = lv_obj_create(g_follow_ctx.win);
    lv_obj_set_size(top_title_win, LV_PCT(100), 60);
    lv_obj_align(top_title_win, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(top_title_win, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(top_title_win, 0, 0);
    lv_obj_set_style_radius(top_title_win, 0, 0);

   
    g_follow_ctx.return_btn = lv_btn_create(top_title_win);
    lv_obj_set_size(g_follow_ctx.return_btn, 50, 50);
    lv_obj_set_style_radius(g_follow_ctx.return_btn, 17, 0);
    lv_obj_set_style_bg_color(g_follow_ctx.return_btn, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(g_follow_ctx.return_btn, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(g_follow_ctx.return_btn, 0, 0);
    lv_img_dsc_t* pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_FLOW_DEL, IMG_FREE_TYPE_AUTO, NULL);
    lv_obj_set_style_bg_img_src(g_follow_ctx.return_btn, pImgDsc, 0);
    lv_obj_align(g_follow_ctx.return_btn, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_event_cb(g_follow_ctx.return_btn, repeat_return_btn_click_cb, LV_EVENT_CLICKED, NULL);

  
    g_follow_ctx.title_label = lv_label_create(top_title_win);
    lv_label_set_text(g_follow_ctx.title_label, "跟读");
    lv_obj_align(g_follow_ctx.title_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(g_follow_ctx.title_label, lv_color_white(), 0);

    // 句子数量标签
    g_follow_ctx.sentence_num_label = lv_label_create(top_title_win);
    lv_label_set_text_fmt(g_follow_ctx.sentence_num_label, "%hu/%hu", current_sentence_num + 1, total_sentence_num);
    lv_obj_align(g_follow_ctx.sentence_num_label, LV_ALIGN_TOP_RIGHT, -12, 11);
    lv_obj_set_style_text_color(g_follow_ctx.sentence_num_label, lv_color_hex(0xffffff), 0);

    // 喇叭按钮
    lv_obj_t* speak_out_img = lv_img_create(g_follow_ctx.win);
    lv_img_dsc_t* audpImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_FLOW_VOICE, IMG_FREE_TYPE_AUTO, NULL);
    lv_img_set_src(speak_out_img, audpImgDsc);
    lv_obj_align(speak_out_img, LV_ALIGN_TOP_MID, 0, 120);
    lv_obj_add_event_cb(speak_out_img, _speak_out_big_click_cb, LV_EVENT_CLICKED, NULL);

    // 单词标签
    g_follow_ctx.word_label = lv_label_create(g_follow_ctx.win);
    lv_label_set_text(g_follow_ctx.word_label, word ? word : "");
    lv_obj_set_style_text_color(g_follow_ctx.word_label, lv_color_white(), 0);
    lv_obj_align(g_follow_ctx.word_label, LV_ALIGN_CENTER, 0, -10);

    // 录音按钮
    g_follow_ctx.state_img = lv_img_create(g_follow_ctx.win);
    pImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_FLOW_SPEAKPRESS, IMG_FREE_TYPE_AUTO, NULL);
    lv_img_set_src(g_follow_ctx.state_img, pImgDsc);
    lv_obj_align(g_follow_ctx.state_img, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_add_flag(g_follow_ctx.state_img, LV_OBJ_FLAG_CLICKABLE);
   
    lv_obj_add_event_cb(g_follow_ctx.state_img, on_repeat_img_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(g_follow_ctx.state_img, on_repeat_img_released, LV_EVENT_RELEASED, NULL);


    g_follow_ctx.result_label = lv_label_create(g_follow_ctx.win);
    lv_label_set_text(g_follow_ctx.result_label, "评估结果");
    lv_obj_set_style_text_color(g_follow_ctx.result_label, lv_color_white(), 0);
    lv_obj_align(g_follow_ctx.result_label, LV_ALIGN_CENTER, 0, -50);
    lv_obj_add_flag(g_follow_ctx.result_label, LV_OBJ_FLAG_HIDDEN);

    // 得分
    g_follow_ctx.score_label = lv_label_create(g_follow_ctx.win);
    lv_label_set_text(g_follow_ctx.score_label, "");
    lv_obj_set_style_text_color(g_follow_ctx.score_label, lv_color_white(), 0);
    lv_obj_align_to(g_follow_ctx.score_label, g_follow_ctx.result_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    lv_obj_add_flag(g_follow_ctx.score_label, LV_OBJ_FLAG_HIDDEN);

    // 反馈
    g_follow_ctx.feedback_label = lv_label_create(g_follow_ctx.win);
    lv_label_set_text(g_follow_ctx.feedback_label, "");
    lv_obj_set_style_text_color(g_follow_ctx.feedback_label, lv_color_white(), 0);
    lv_obj_align_to(g_follow_ctx.feedback_label, g_follow_ctx.score_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    lv_obj_add_flag(g_follow_ctx.feedback_label, LV_OBJ_FLAG_HIDDEN);

    // 重试按钮
    g_follow_ctx.retry_btn = lv_btn_create(g_follow_ctx.win);
    lv_obj_set_size(g_follow_ctx.retry_btn, 120, 40);
    lv_obj_align(g_follow_ctx.retry_btn, LV_ALIGN_BOTTOM_MID, 0, -80);
    lv_obj_t* retry_label = lv_label_create(g_follow_ctx.retry_btn);
    lv_label_set_text(retry_label, "再试一次");
    lv_obj_center(retry_label);
    lv_obj_add_event_cb(g_follow_ctx.retry_btn, on_retry_btn_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(g_follow_ctx.retry_btn, LV_OBJ_FLAG_HIDDEN);

    g_follow_ctx.state = FOLLOW_IDLE;
    update_ui_state();

#ifndef C_PC_SIM
    play_or_stop_current_sentence();
#endif

    return 0;
}


/**
 * @brief 跟读界面
 */
int play_repeat_page_init(int parm)
{
    LV_LOG_USER("cur word:%s", play_unit_info.pOrginal[current_sentence_num]);
    player_repeat_follow_page_init(play_unit_info.pOrginal[current_sentence_num]); 
   
#ifndef C_PC_SIM
    play_or_stop_current_sentence();
#endif
    return 0;
}


void send_request_async(TtsRequest* req, void (*callback)()) {
    // 实现实际的网络请求
    // 请求完成后调用回调函数
    printf("Sending TTS request...\n");
    if (callback) callback();
}

void Ap_tts_gen(const char* q, const char* language, void (*callback)()) {
#if 0
    TtsRequest req = { 0 };

    char* q_str = (char*)malloc(strlen(q) + 4);
    sprintf(q_str, "%s。", q);
    printf("input q = %s\n", q_str);

    int errorNo;
    // 填充请求结构
    req.q = q_str;
    req.appKey = (char*)TTS_APP_KEY;
    req.signType = "v3";
    req.curtime = get_current_time();
    req.salt = get_random_salt();
    req.sign = get_sign(&req, q_str, TTS_APP_SECRET);

    url_encode_t* purl = zb_urlEnc_init("https://tts.api.com", &errorNo);

    // 2. 添加路径
    zb_urlEnc_setPath(purl, "v1/synthesize");

    // 3. 添加查询参数
    zb_urlEnc_addQueryParm(purl, "text", req.q);
    
    req.speed = "1.0";
    req.volume = "1.0";
    req.format = "mp3";

    // 根据语言选择声音
    if (language != NULL && strcmp(language, "en") == 0) {
        req.voiceName = "youxiaomei";
    }
    else {
        req.voiceName = "youxiaoxun";
    }

    // 发送异步请求
    send_request_async(&req, callback);

    // 清理内存
    free(req.curtime);
    free(req.salt);
    free(req.sign);
    free((void*)req.q); // 释放URL编码后的字符串
    free(q_str);
#endif
}


