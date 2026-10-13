
#include "aicard_ui.h"

//mp3 playing
#include "trd/tz_audio_player/include/tz_audio_player.h"


//4G
#include "trd/ubus/include/tz_hal_quectel.h"
#include "trd/ubus/include/ubus_uloop_receiver.h"
#include "trd/tz_sys_lib/include/cvi_tz_sys.h"
#include "trd/tz_sys_lib/include/mlog.h"

typedef struct key_status
{
    int  id;
    int   action;
} key_status_t;


static char number[16] = "15012603570";
static char pinCode[32] = { 0 };
static char pukCode[32] = { 0 };
static int vol_level = 0;
static int state = 0;


void seed_phone_ask();
void seed_phone_call();
void  seed_phone_Nocall();
void* recv_phone_enent_hander(void* arg);

quectel_handler_t* m_phone_handler; //phone 句柄

lv_obj_t* m_label_display = NULL;

//----------------------------------------------

/* 回调函数，接收结构体作为参数 */
void async_key_event_to_call(void * ac)
{
    struct key_status* data = (struct key_status*)ac;
    
    LOGD("Received action=%d, id=%d\n", data->action, data->id);

    /* 如果结构体在堆上，记得释放内存 */
    free(data);
}

//sys
void tz_asyn_wifi_event_msg(bool connect, int signal_qua)
{
	printf("wifi_msg: connect:%d lastPower:%d\n", connect, signal_qua);
}

void tz_asyn_battery_event_msg(bool bCharging, int lastPower)
{
	printf("bat_msg: bCharging:%d lastPower:%d\n", bCharging, lastPower);
}

/* void tz_asyn_key_event_msg(tz_key_id_t id, tz_key_action_t action)
{
    printf("key_msg: tz_key_id_t:%d tz_key_action_t:%d\n", id, action);
    struct key_status* data = lv_mem_alloc(sizeof(struct key_status));
    data->action = action;
    data->id = id;
    lv_async_call(async_key_event_to_call, data);
} */

void test_mp3()
{
	
   //AVFormatContext *fmt_ctx = NULL;
   //avformat_open_input(&fmt_ctx, input_file, NULL, NULL);
   //avformat_find_stream_info(fmt_ctx, NULL);
   //int audio_stream_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	
}


lv_obj_t * create_activity(char * name)
{
    printf("create_activity \n");
    lv_obj_t * screen   = lv_scr_act();
    lv_obj_t * activity = lv_obj_create(screen);
    activity->user_data = name;

    // Set common style
    lv_obj_set_width(activity, lv_obj_get_width(screen));
    lv_obj_set_height(activity, lv_obj_get_height(screen));
    // lv_obj_set_height(activity, 200);
    lv_obj_set_style_border_width(activity, 0, 0);
    lv_obj_set_style_radius(activity, 0, 0);
    lv_obj_set_style_pad_all(activity, 0, 0);
    lv_obj_set_style_pad_row(activity, 0, 0);
    lv_obj_set_style_pad_column(activity, 0, 0);
    lv_obj_set_style_bg_color(activity, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_color(activity, lv_color_hex(0xCCCCCC), 0);


    return activity;
}

void on_click_item(lv_event_t* event)
{
    const char* text = (const char*)event->user_data;
    if (strcmp(text, "en") == 0)
    {
        printf("create_activity en\n");
    }
    else if (strcmp(text, "set") == 0)
    {
        printf("create_activity set\n");
    }
    else if (strcmp(text, "book") == 0)
    {
       printf("create_activity book\n");
    }
    else if (strcmp(text, "ask") == 0)
    {
        printf("create_activity book\n");
        seed_phone_ask();
    }
    else if (strcmp(text, "call") == 0)
    {
        printf("create_activity call\n");
        seed_phone_call();
    }
    else if (strcmp(text, "nocall") == 0)
    {
        printf("create_activity nocall\n");
        seed_phone_Nocall();
    }
    else if (strcmp(text, "play_mp3") == 0)
    {
        printf("create_activity play_mp3\n");
        tz_play_audio("/userdata/666.mp3", 0, -1);
    }
    else if (strcmp(text, "pause_mp3") == 0)
    {
        printf("create_activity pause_mp3\n");
        tz_play_pause_audio();
    }
    else if (strcmp(text, "resume_mp3") == 0)
    {
        printf("create_activity resume_mp3\n");
        tz_play_resume_audio();
    }
    else if (strcmp(text, "end_mp3") == 0)
    {
        printf("create_activity end_mp3\n");
        tz_play_stop_audio();
    }
    
    
}

void show_eng_page(lv_obj_t* tabEng)
{
#if 0
    //tabEng
    lv_obj_t* tabEng_obj_ai = lv_obj_create(tabEng);
    lv_obj_set_width(tabEng_obj_ai, 296); // 设置第一个标签的宽度  //TEXT_HOME_IDICT
    lv_obj_set_height(tabEng_obj_ai, 120);
    lv_obj_add_event_cb(tabEng_obj_ai, on_click_item, LV_EVENT_CLICKED, "en");

    lv_obj_t* tabEng_obj_en = lv_obj_create(tabEng);
    lv_obj_set_width(tabEng_obj_en, 142); // 设置第一个标签的宽度
    lv_obj_set_height(tabEng_obj_en, 128);

    lv_obj_align_to(tabEng_obj_en, tabEng_obj_ai, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    lv_obj_t* tabEng_obj_damao = lv_obj_create(tabEng);
    lv_obj_set_width(tabEng_obj_damao, 142); // 设置第一个标签的宽度
    lv_obj_set_height(tabEng_obj_damao, 128);

    lv_obj_align_to(tabEng_obj_damao, tabEng_obj_en, LV_ALIGN_OUT_RIGHT_TOP, 12, 0);

    lv_obj_t* tabEng_obj_waik = lv_obj_create(tabEng);
    lv_obj_set_width(tabEng_obj_waik, 296); // 设置第一个标签的宽度
    lv_obj_set_height(tabEng_obj_waik, 120);

    lv_obj_align_to(tabEng_obj_waik, tabEng_obj_en, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
#else

    lv_obj_t* tabEng_obj = lv_obj_create(tabEng);
    lv_obj_set_width(tabEng_obj, 296); // 设置第一个标签的宽度
    lv_obj_set_height(tabEng_obj, 200);
    //lv_obj_add_event_cb(tabEng_obj, on_click_item, LV_EVENT_CLICKED, "set");

    m_label_display = lv_label_create(tabEng_obj);
    lv_obj_set_size(m_label_display, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_label_set_text_fmt(m_label_display, "LV_SIZE_CONTENT");
    lv_obj_set_style_bg_color(m_label_display, lv_color_hex(0xFF00FF), LV_PART_ITEMS | LV_STATE_CHECKED);

    lv_obj_t* btnask = lv_btn_create(tabEng);
    lv_obj_set_width(btnask, 60); // 设置第一个标签的宽度
    lv_obj_set_height(btnask, 60);
    //lv_label_set_text(btncall, "call");
    lv_obj_align_to(btnask, tabEng_obj, LV_ALIGN_OUT_BOTTOM_LEFT, 20, 20);
    lv_obj_add_event_cb(btnask, on_click_item, LV_EVENT_CLICKED, "ask");

    lv_obj_t* label = lv_label_create(btnask);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_label_set_text_fmt(label, "ask");

   
    lv_obj_t* btncall = lv_btn_create(tabEng);
    lv_obj_set_width(btncall, 60); // 设置第一个标签的宽度
    lv_obj_set_height(btncall, 60);
    //lv_label_set_text(btncall, "call");
    lv_obj_align_to(btncall, tabEng_obj, LV_ALIGN_OUT_BOTTOM_LEFT, 20, 100);
    lv_obj_add_event_cb(btncall, on_click_item, LV_EVENT_CLICKED, "call");

    label = lv_label_create(btncall);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_label_set_text_fmt(label, "call");


    lv_obj_t* btnnocall = lv_btn_create(tabEng);
    lv_obj_set_width(btnnocall, 60); // 设置第一个标签的宽度
    lv_obj_set_height(btnnocall, 60);
    //lv_label_set_text(btnnocall, "nocall");
    lv_obj_align_to(btnnocall, btncall, LV_ALIGN_OUT_RIGHT_TOP, 20, 0);
    lv_obj_add_event_cb(btnnocall, on_click_item, LV_EVENT_CLICKED, "nocall");
    label = lv_label_create(btnnocall);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_label_set_text_fmt(label, "nocall");


#endif
}

void show_ch_page(lv_obj_t* tabCH)
{
#if 0
    //tabCH
    lv_obj_t* tabEng_obj_ch = lv_obj_create(tabCH);
    lv_obj_set_width(tabEng_obj_ch, 296); // 设置第一个标签的宽度
    lv_obj_set_height(tabEng_obj_ch, 160);

    lv_obj_t* tabEng_obj_gxtd = lv_obj_create(tabCH);
    lv_obj_set_width(tabEng_obj_gxtd, 142); // 设置第一个标签的宽度
    lv_obj_set_height(tabEng_obj_gxtd, 163);

    lv_obj_align_to(tabEng_obj_gxtd, tabEng_obj_ch, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    lv_obj_t* tabEng_obj_zwhb = lv_obj_create(tabCH);
    lv_obj_set_width(tabEng_obj_zwhb, 142); // 设置第一个标签的宽度
    lv_obj_set_height(tabEng_obj_zwhb, 163);

    lv_obj_align_to(tabEng_obj_zwhb, tabEng_obj_gxtd, LV_ALIGN_OUT_RIGHT_TOP, 12, 0);
#else

    lv_obj_t* btnplay_mp3 = lv_btn_create(tabCH);
    lv_obj_set_width(btnplay_mp3, 80); // 设置第一个标签的宽度
    lv_obj_set_height(btnplay_mp3, 50);
    //lv_label_set_text(btncall, "call");
    //lv_obj_align_to(btnplay_mp3, tabEng_obj, LV_ALIGN_OUT_BOTTOM_LEFT, 20, 20);
    lv_obj_add_event_cb(btnplay_mp3, on_click_item, LV_EVENT_CLICKED, "play_mp3");

    lv_obj_t* label = lv_label_create(btnplay_mp3);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_label_set_text_fmt(label, "play");

    lv_obj_t* btnpause_mp3 = lv_btn_create(tabCH);
    lv_obj_set_width(btnpause_mp3, 80); // 设置第一个标签的宽度
    lv_obj_set_height(btnpause_mp3, 60);
    //lv_label_set_text(btncall, "call");
    lv_obj_align_to(btnpause_mp3, btnplay_mp3, LV_ALIGN_OUT_RIGHT_TOP, 20, 0);
    lv_obj_add_event_cb(btnpause_mp3, on_click_item, LV_EVENT_CLICKED, "pause_mp3");

    label = lv_label_create(btnpause_mp3);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_label_set_text_fmt(label, "pause");

    lv_obj_t* btnresume_mp3 = lv_btn_create(tabCH);
    lv_obj_set_width(btnresume_mp3, 80); // 设置第一个标签的宽度
    lv_obj_set_height(btnresume_mp3, 60);
    //lv_label_set_text(btncall, "call");
    lv_obj_align_to(btnresume_mp3, btnpause_mp3, LV_ALIGN_OUT_RIGHT_TOP, 20, 0);
    lv_obj_add_event_cb(btnresume_mp3, on_click_item, LV_EVENT_CLICKED, "resume_mp3");

    label = lv_label_create(btnresume_mp3);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_label_set_text_fmt(label, "resume");

    lv_obj_t* btnend_mp3 = lv_btn_create(tabCH);
    lv_obj_set_width(btnend_mp3, 80); // 设置第一个标签的宽度
    lv_obj_set_height(btnend_mp3, 60);
    //lv_label_set_text(btncall, "call");
    lv_obj_align_to(btnend_mp3, btnplay_mp3, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_obj_add_event_cb(btnend_mp3, on_click_item, LV_EVENT_CLICKED, "end_mp3");

    label = lv_label_create(btnend_mp3);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_label_set_text_fmt(label, "end");

#endif
}

void show_app_page(lv_obj_t* tabAPP)
{
    //tabAPP
    lv_obj_t* tabAPP_obj_zfub = lv_obj_create(tabAPP);
    //lv_obj_set_pos(tabAPP_obj_zfub, 10, 0);
    lv_obj_set_width(tabAPP_obj_zfub, 85); // 设置第一个标签的宽度
    lv_obj_set_height(tabAPP_obj_zfub, 104);

    lv_obj_t* tabAPP_obj_addressbook = lv_obj_create(tabAPP);
    lv_obj_set_width(tabAPP_obj_addressbook, 85); // 设置第一个标签的宽度
    lv_obj_set_height(tabAPP_obj_addressbook, 104);
    lv_obj_align_to(tabAPP_obj_addressbook, tabAPP_obj_zfub, LV_ALIGN_OUT_RIGHT_TOP, 20, 0);
    lv_obj_add_event_cb(tabAPP_obj_addressbook, on_click_item, LV_EVENT_CLICKED, "book");

    lv_obj_t* label = lv_label_create(tabAPP_obj_addressbook);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_label_set_text(label, "BOOK");

    lv_obj_set_align(label, LV_ALIGN_BOTTOM_MID);

    lv_obj_t* tabAPP_obj_dictation = lv_obj_create(tabAPP);
    lv_obj_set_width(tabAPP_obj_dictation, 85); // 设置第一个标签的宽度
    lv_obj_set_height(tabAPP_obj_dictation, 104);
    lv_obj_align_to(tabAPP_obj_dictation, tabAPP_obj_addressbook, LV_ALIGN_OUT_RIGHT_TOP, 20, 0);

    lv_obj_t* tabAPP_obj_dictionary = lv_obj_create(tabAPP);
    lv_obj_set_width(tabAPP_obj_dictionary, 85); // 设置第一个标签的宽度
    lv_obj_set_height(tabAPP_obj_dictionary, 104);
    lv_obj_align_to(tabAPP_obj_dictionary, tabAPP_obj_zfub, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    lv_obj_t* tabAPP_obj_resources = lv_obj_create(tabAPP);
    lv_obj_set_width(tabAPP_obj_resources, 85); // 设置第一个标签的宽度
    lv_obj_set_height(tabAPP_obj_resources, 104);
    lv_obj_align_to(tabAPP_obj_resources, tabAPP_obj_dictionary, LV_ALIGN_OUT_RIGHT_TOP, 20, 0);

    lv_obj_t* tabAPP_obj_set = lv_obj_create(tabAPP);
    lv_obj_set_width(tabAPP_obj_set, 85); // 设置第一个标签的宽度
    lv_obj_set_height(tabAPP_obj_set, 104);
    lv_obj_align_to(tabAPP_obj_set, tabAPP_obj_resources, LV_ALIGN_OUT_RIGHT_TOP, 20, 0);
    lv_obj_add_event_cb(tabAPP_obj_set, on_click_item, LV_EVENT_CLICKED, "set");

    label = lv_label_create(tabAPP_obj_set);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_label_set_text(label, "SET");

    lv_obj_set_align(label, LV_ALIGN_BOTTOM_MID);
    //lv_label_set_cjk_font(label, 18);
}

void aicard_ui()
{
    lv_obj_t * activity = create_activity(NULL);
    int activity_width  = lv_obj_get_style_width(activity, LV_PART_MAIN);
	int activity_height  = lv_obj_get_style_height(activity, LV_PART_MAIN);

    LOGD("screen_width: %d, screen_height: %d\n", activity_width, activity_height);

    lv_obj_set_style_pad_all(activity, 0, 0);
    lv_obj_set_style_pad_top(activity, 26, 0);
    lv_obj_set_style_pad_row(activity, 0, 0);

    // 隐藏滚动条代码
    lv_obj_remove_style(activity, 0, LV_PART_SCROLLBAR);

    /*Create a Tab view object*/
    lv_obj_t* tabview;
    tabview = lv_tabview_create(activity, LV_DIR_TOP, 32);
    // 隐藏滚动条代码
    lv_obj_remove_style(tabview, 0, LV_PART_SCROLLBAR);

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t* tabEng = lv_tabview_add_tab(tabview, "EN");
    lv_obj_t* tabCH = lv_tabview_add_tab(tabview, "CH");
    lv_obj_t* tabAPP = lv_tabview_add_tab(tabview, "APP");

    lv_obj_set_style_bg_color(tabview, lv_color_hex(0xF5F7FA), 0);
    lv_tabview_set_act(tabview, 0, LV_ANIM_OFF);

    lv_obj_t* tab_btns = lv_tabview_get_tab_btns(tabview);
    //清空所有样式
    lv_obj_remove_style_all(tab_btns);
    lv_obj_set_width(tab_btns, 210); // 设置第一个标签的宽度
    lv_obj_set_height(tab_btns, 32);
    //lv_label_set_cjk_font(tabview, 18);
    //本地样式修改背景色
    //lv_obj_set_style_bg_color(tab_btns, lv_color_hex(0xFFFFFF), LV_PART_ITEMS | LV_STATE_CHECKED);
    //本地样式修改透明度
    //lv_obj_set_style_bg_opa(tab_btns, LV_OPA_0, LV_PART_ITEMS | LV_STATE_CHECKED);
    //修改选中时边框的颜色
    lv_obj_set_style_border_color(tab_btns, lv_palette_main(LV_PALETTE_GREEN), LV_PART_ITEMS | LV_STATE_CHECKED);
    //修改选中时边框的颜色透明度
    //lv_obj_set_style_border_opa(tab_btns, LV_OPA_0, LV_PART_ITEMS | LV_STATE_CHECKED);
    //本地样式修改选中时的文字颜色
    lv_obj_set_style_text_color(tab_btns, lv_color_hex(0x18191B), LV_PART_ITEMS | LV_STATE_CHECKED);
    //本地样式修改未选中时的文字颜色
    lv_obj_set_style_text_color(tab_btns, lv_color_hex(0x999999), 0);

    
	LOGD("测试电话功能\n");
    /*Add content to the tabs*/
    show_eng_page(tabEng);
    show_ch_page(tabCH);
    show_app_page(tabAPP);

    //ubug phone
    m_phone_handler = tz_hal_quectel_handler_new();
    //监听：接电话
    pthread_t thread_enent;
    pthread_create(&thread_enent, NULL, recv_phone_enent_hander, NULL);
    pthread_detach(thread_enent);

    //tz_hal_quectel_handler_destory(m_phone_handler);//退出释放
}


static const struct blobmsg_policy resp_policy[7] = {
    [0] = {.name = "event", .type = BLOBMSG_TYPE_STRING },
    [1] = {.name = "phone_number", .type = BLOBMSG_TYPE_STRING },
    [2] = {.name = "signal_strength", .type = BLOBMSG_TYPE_INT32 },
    [3] = {.name = "inserted_state", .type = BLOBMSG_TYPE_INT32 },
    [4] = {.name = "oper_name", .type = BLOBMSG_TYPE_INT32 },
    [5] = {.name = "network_type", .type = BLOBMSG_TYPE_INT32 },
    [6] = {.name = "call_state", .type = BLOBMSG_TYPE_INT32 },
};

typedef struct {
    int value;
    const char* event_msg;
    const char* phone_number;
} msg_data_t;

void event_msg_async_callback(void* data) {
    // 这里执行你的UI更新代码
    msg_data_t* msg_data = (msg_data_t*)data;
    printf("[event_msg_async_callback] message: %s\n", msg_data->event_msg);

    if (m_label_display!=NULL)
    {
        printf("[event_msg_async_callback] message: %s\n", msg_data->event_msg);
        lv_label_set_text_fmt(m_label_display, "event_msg:%s\nphone_number:%s\n",msg_data->event_msg, msg_data->phone_number);
    }

    free(msg_data); // 释放内存
    
}

static void handle_event(struct ubus_context* ctx, struct ubus_event_handler* ev,
    const char* type, struct blob_attr* msg) {
    struct blob_attr* tb[6];

    printf("[Server] Entering handle_event callback for event type: %s\n", type);


    if (blobmsg_parse(resp_policy, 7, tb, blob_data(msg), blob_len(msg)) != 0) {
        fprintf(stderr, "[Server] Failed to parse event message in handle_event\n");
        return;
    }

    if (tb[0]) {
        const char* event_msg = blobmsg_get_string(tb[0]);
        printf("[Server] Received event message: %s\n", event_msg);

        const char* phone_number_msg = blobmsg_get_string(tb[1]);

        msg_data_t* data = malloc(sizeof(msg_data_t));
        data->event_msg = event_msg;
        data->phone_number = phone_number_msg;
        lv_async_call(event_msg_async_callback, data);
    }
    else {
        printf("[Server] No 'message' field found in the received event\n");
    }
    

    if (tb[1]) {
        const char* phone_number_msg = blobmsg_get_string(tb[1]);
        printf("[Server] Received phone_number: %s\n", phone_number_msg);
    }
    else {
        printf("[Server] No 'phone_number' field found in the received event\n");
    }

    if (tb[2]) {
        const int signal_strength = blobmsg_get_u32(tb[2]);
        printf("[Server] Received signal_strength: %d\n", signal_strength);
    }
    else {
        printf("[Server] No 'signal_strength' field found in the received event\n");
    }

    if (tb[3]) {
        const int inserted_state = blobmsg_get_u32(tb[3]);
        printf("[Server] Received inserted_state: %d\n", inserted_state);
    }
    else {
        printf("[Server] No 'inserted_state' field found in the received event\n");
    }

    if (tb[4]) {
        const int oper_name = blobmsg_get_u32(tb[4]);
        printf("[Server] Received oper_name: %d\n", oper_name);
    }
    else {
        printf("[Server] No 'oper_name' field found in the received event\n");
    }

    if (tb[5]) {
        const int network_type = blobmsg_get_u32(tb[5]);
        printf("[Server] Received network_type: %d\n", network_type);
    }
    else {
        printf("[Server] No 'network_type' field found in the received event\n");
    }

    if (tb[6]) {
        const int call_state = blobmsg_get_u32(tb[6]);
        printf("[Server] Received call_state: %d\n", call_state);
    }
    else {
        printf("[Server] No 'call_state' field found in the received event\n");
    }

    printf("[Server] Leaving handle_event callback\n");
}

static struct ubus_event_handler obj_event_handler = {
    .cb = handle_event
};

static void ql_sigaction(int signo) {
    ubus_uloop_receiver_exit();
}

// 线程函数
void* recv_phone_enent_hander(void* arg)
{
    signal(SIGINT, ql_sigaction);
    signal(SIGTERM, ql_sigaction);
    signal(SIGALRM, ql_sigaction);

    ubus_uloop_receiver_event_init(&obj_event_handler, "tz_quectel_notify");
    return NULL;
}

void seed_phone_ask()
{
    int response_status = 0;
    
    if (!m_phone_handler) {
        printf("tz_hal_quectel_handler_new failed\n");
        return -1;
    }
    response_status = m_phone_handler->answer();
    printf("response_status:%d\n", response_status);
}

//主动拨打 实例
void seed_phone_call()
{
    int response_status = 0;
    //number = "10086";
    //quectel_handler_t* handler = tz_hal_quectel_handler_new();
    if (!m_phone_handler) {
        printf("tz_hal_quectel_handler_new failed\n");
        return -1;
    }
    response_status = m_phone_handler->call(number);
    printf("response_status:%d\n", response_status);


}
void seed_phone_Nocall()
{
    int response_status = 0;
    //quectel_handler_t* handler = tz_hal_quectel_handler_new();
    if (!m_phone_handler) {
        printf("tz_hal_quectel_handler_new failed\n");
        return -1;
    }
    response_status = m_phone_handler->hangup();
    printf("response_status:%d\n", response_status);
}




void pause_playback() {
}

void resume_playback() {
}


