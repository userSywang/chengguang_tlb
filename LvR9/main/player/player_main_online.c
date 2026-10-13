/*******************************************************
                    书籍显示页面程序段
******************************************************/
#include <time.h>
#include "player.h"
#include "player_main.h"
#include "../zb_base.h"
#include "player_data_parse.h"
#include "player_http_client.h"
#include "player_main_online.h"
#include "./jump_dic.h"
#include "../data_parse/analyze_listening.h"
#include "../utils/global_parm.h"
#include "../utils/misc_utils.h"
#include "../ex_widget/app_msg.h"
//#include "../src/http/http_down_sd_file.h"

#ifndef C_PC_SIM
//#include <dirent.h>   // for opendir() 
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"

#include "player_main_online.h"


extern bool zb_is_wifi_connect(); 

extern void set_http_download_split(bool bset);
int http_stop_download_task(void);
extern int esp32s3_get_sdcard_capacity(uint64_t* total_space, uint64_t* free_space);
extern int http_download(char* url, char* save_path);
#endif

extern int textbook_quanjie_all_free();
extern int zb_book_data_manage_free(void);

//--------------------------------------

#define TAG "MAIN_ONLINE"

#define  BETWEEN_BOOKCOVER_TEXT_SPACE 8
#define  READY_PERCENT  (2)
#define  DOWN_PERCENT   (100 - READY_PERCENT)

//--------------------------------------
#if 0
bool   gCancelUserFlag = false;
lv_obj_t* gGradeItemObj = NULL;
lv_obj_t* gbar;
static download_cfg_t dct;
static dl_pc_param dpp;

nmbox_book_list_t nmbox_book_list = { 0 };
nmbox_t nmbox = { 0 };

char publisher[100];
lv_obj_t* item_obj;
char grade_path[350] = { 0 };
static lv_timer_t* gtimer;
static lv_obj_t* cancelMsgbox;
static int8_t g_cfgNetScrFlg;
static int8_t g_wantUpdGradeInfFlg;
static int8_t g_tmp_Term = 0;
static lv_obj_t* focus_obj;
static bool g_forceRefresh;

static plyOnlnApInf_t * g_plOlApInf;


//--------------------------------------

extern player_class_locate_t player_class_locate;
extern int player_class_num;
extern uint8_t play_main_clicked_btn_num;

extern int play_quanjie_unit_page_init(int parm);
extern const char* class_info_tl[];
extern char* g_plyHttpRsltBuf;

extern int16_t ap_toggle_num ;


extern int play_stop_http_client();
extern int zb_book_data_manage_init(const char *language);
extern void zb_clear_autoPwrOffCnt(void);
extern void zb_clear_autoScreenOffCnt(void);
extern char* urlEncode(const char* string);
lv_obj_t* create_down_bar(lv_obj_t* parent);
int fill_grage_list();

static void lv_example_msgbox_test(void);
static int empty_hint(const char *msg, uint8_t genre);

/**
 * @brief 解析返回的code
 * @param
 *  root: root数据
 *  salve_code:保存编码
 *  correct_code:正确code
 * @return
 *  0：成功
 * 非0：失败
 */
int parse_code(cJSON* root, int* salve_code, int correct_code)
{
    cJSON* code = NULL;
    cJSON* msg = NULL;
    if (root == NULL)
    {
        ESP_LOGI(PLA, "json parse fail! error:%s\n", cJSON_GetErrorPtr());
        return -1;
    }
    code = cJSON_GetObjectItem(root, "code");
    // printf("code = %s,%d\n",code->string,code->valueint);
    *salve_code = code->valueint;
    if (code->valueint != correct_code)
    {
        msg = cJSON_GetObjectItem(root, "msg");
        ESP_LOGI(PLA, "code is error! code:%d,msg:%s\n", code->valueint, msg->valuestring);
        return -2;
    }
    return 0;
}

int get_percent(int fileSize, int curSize, int start, int downPercent)
{
    int64_t f = ((int64_t)curSize << 12) / fileSize;
    int progress = start + ((f * downPercent) >> 12);
    return progress;
}

bool judge_exist_book_id(const char* root, const char* book_id) {
    char tmp_path[200];
    snprintf(tmp_path, sizeof(tmp_path), "%s/%s.ini", root, book_id);
    ESP_LOGI(TAG, "判断文件是否存在：%s ", tmp_path);
    if (zb_isFileExist(tmp_path) != 0)
        return true;
    else
        return false;
}

void print_stt(int line) {
    ESP_LOGI(PLA, "STT: %d ; line=[%d]\n", nmbox.downStatus, line);
}
int parse_bool_down_url_jstr(char* jstr, char* url_path) 
{
    int ret = 0;
    char tmp_path[250];
    cJSON* data = NULL;
    cJSON* url = NULL;
    cJSON* root = cJSON_Parse(jstr);
    int code;
    ret = parse_code(root, &code, 200);
    if (ret)
    {
        ESP_LOGI(PLA, "book list json parse error!\n");
        cJSON_Delete(root);
        return -1;
    }
    url = cJSON_GetObjectItem(root, "url");
    if (!cJSON_IsArray(url))
    {
        ESP_LOGI(PLA, "get array fail!\n");
        cJSON_Delete(root);
        return -2;
    }
    int size = cJSON_GetArraySize(url);
    if (size > 1) 
        ESP_LOGI(PLA, "url 的 大小不对，只应该有一个!\n");

    cJSON* temp = cJSON_GetArrayItem(url, 0);
    
    if (strlen(temp->valuestring) == 0)
    {
        ESP_LOGI(PLA, "get url fail!\n");
        cJSON_Delete(root);
        return -2;
    }
    else {
        strcpy(url_path, temp->valuestring);
    }
    
    cJSON_Delete(root);
    return 0;
}

/**
 * @brief 获取书籍列表数据
 * @param  subject: 1 - yuwen, 2 - english
 */
int parse_book_list_jstr(char* jstr, int16_t subject, char* publisher, nmbox_book_list_t* nmbox_book_list)
{
    int ret = 0;
    char tmp_path[250];
    cJSON* data = NULL;
    cJSON* term = NULL;
    cJSON* version = NULL;
    cJSON* book_id = NULL;
    cJSON* quan_jie = NULL;
    cJSON* root = cJSON_Parse(jstr);
    int exit_code = 0;
    ret = parse_code(root, &nmbox_book_list->code, 0);
    if (ret)
    {
        ESP_LOGI(PLA, "book list json parse error!\n");
        exit_code = -1;
        goto ParseExit;
    }
    data = cJSON_GetObjectItem(root, "data");
    if (!cJSON_IsArray(data))
    {
        ESP_LOGI(PLA, "get array fail!\n");
        exit_code = -2;
        goto ParseExit;
    }
    int size = cJSON_GetArraySize(data);
    if (size > MAX_BOOK_GRADE_NUM) {
        size = MAX_BOOK_GRADE_NUM;
    }
    nmbox_book_list->book_num = size;
    for (int i = 0; i < size; i++)
    {
        cJSON* temp = cJSON_GetArrayItem(data, i);
        term = cJSON_GetObjectItem(temp, "book_full_name");
        //printf("size term = %d\n", sizeof(nmbox_book_list->term[i]));
        strlcpy(nmbox_book_list->term[i], term->valuestring, sizeof(nmbox_book_list->term[i]));
        ESP_LOGI(TAG, "term = %s\n", nmbox_book_list->term[i]);
        
        book_id = cJSON_GetObjectItem(temp, "book_id");
        strlcpy(nmbox_book_list->book_id[i], book_id->valuestring, sizeof(nmbox_book_list->book_id[i]));
        printf("book_id = %s\n", nmbox_book_list->book_id[i]);

        if (0 == i) {
            version = cJSON_GetObjectItem(temp, "book_version");
            printf("version = %s,input_version=%s\n", version->valuestring, publisher);
            strlcpy(nmbox_book_list->version, version->valuestring, sizeof(nmbox_book_list->version));
        }

        quan_jie = cJSON_GetObjectItem(temp, "complete_solution");
        if (quan_jie) {
            nmbox_book_list->waiyan[i] = 1;
            ESP_LOGI(PLA, "quan_jie: TRUE\n");
        }
        else {
            ESP_LOGI(PLA, "quan_jie: FALSE\n");
            nmbox_book_list->waiyan[i] = 0;
        }

        int16_t datumType = (1 == subject ? DATUM_CHINESE_TEXTBOOK : DATUM_ENGLISH_TEXTBOOK);
        snprintf(tmp_path, sizeof(tmp_path), "%s%s/%s/%s.ini", PLAYER_ONLINE_DATA_PATH, class_info_tl[datumType], 
            version->valuestring, nmbox_book_list->book_id[i]);
        if (zb_isFileExist(tmp_path)) {
            nmbox_book_list->down[i] = 1;
            ESP_LOGI(PLA, "DOWN: TRUE: %s\n", tmp_path);
        }
        else
        {
            nmbox_book_list->down[i] = 0;
            ESP_LOGI(PLA, "DOWN: FALSE: %s\n", tmp_path);
        }
        //insert_bv_node(nmbox.curGrade, nmbox_book_list->book_id[i], nmbox_book_list->term[i]);
    }

ParseExit:
    cJSON_Delete(root);
    //int xxx = static_node_sum();
    return exit_code;
}

//#ifndef C_PC_SIM
//进度条回调
static dl_pc_func_t data_dl_file_progress_cb(void* param)
{
    //ESP_LOGI(TAG, "进入下载的回调函数");
    dl_pc_param* dl_f = (dl_pc_param*)param;
    
    dpp.file_size = dl_f->file_size;
    dpp.cur_len = dl_f->cur_len;
    //ESP_LOGI(TAG, "dl_pc_cb: file_size = %d, cur_len=%d", (int)dpp.file_size, (int)dpp.cur_len);
    return 0;
}
//#endif

extern lv_img_dsc_t* LSNSPK_get_cover_data(const char* pPathUnit, int* pErrNo);


void restore_before_down(lv_obj_t* parent, bool downSucess) {
    lv_obj_t* img_one = lv_obj_get_child(parent, 0);
    lv_obj_t* lab_two = lv_obj_get_child(parent, 1);
    if (downSucess) {
        lv_obj_set_style_img_recolor_opa(img_one, 0, 0);

        lv_img_dsc_t* pimg = get_book_cover_img(nmbox_book_list.version, nmbox_book_list.book_id[nmbox.curTerm]);
        lv_obj_clean(img_one);
        if (pimg) {
        	lv_img_set_src(img_one, pimg);
        }
    }
    lv_obj_del(gbar);// 删除 bar
    lv_obj_align_to(lab_two, img_one, LV_ALIGN_OUT_RIGHT_MID, BETWEEN_BOOKCOVER_TEXT_SPACE, 0);
}

//static void msgbox_cb(lv_timer_t* timer)
//{
//    // msg 弹出过后，背后的焦点窗口给变化了，设置回来
//    lv_group_focus_obj(lv_obj_get_child(item_obj, g_tmp_Term));
//    ESP_LOGI(TAG, "timer resotre old focus object!");
//}

static void msgbox_timeout_1_cb(lv_timer_t* timer)
{
    // msg 弹出过后，背后的焦点窗口给变化了，设置回来
    //lv_group_focus_obj(lv_obj_get_child(play_main_win, 0));
    lv_group_focus_obj(focus_obj);
}

static void pop_msg_downing(lv_event_t* e) {
    focus_obj = lv_event_get_target(e);
    app_msgbox_timeout("", "  正在下载文件...，\n      请稍候!", 2000, msgbox_timeout_1_cb, NULL);
    return;
}

static int query_token_if_need(void) {
	int funcRet = 0;
	if (!IS_JC_TOKEN_READY()) {
		int ret = play_http_post_create(REQUEST_TOKEN);
		if (ret < 0) {
			ESP_LOGE(TAG, "queryToken fail, ret %d", ret);
			funcRet = ret;
		}
		else {
			ESP_LOGE(TAG, "queryToken start");
			ret = 1;
		}
	}
	return funcRet;
}

//显示文件消息框弹窗回调函数
static void _confirm_enterNetCfg_cb(lv_event_t* evt)
{
    lv_event_code_t  evtCode = evt->code;
    ESP_LOGI(TAG, "%s st, %d", __func__, evtCode);
    lv_obj_t * obj = lv_event_get_current_target(evt);//获取当前的目标对象（即消息框对象）

    switch (evtCode) {
        case LV_EVENT_VALUE_CHANGED: {
            uint16_t btnNo = lv_msgbox_get_active_btn(obj);// 获取当前消息框中被激活（被点击）的按钮的编号
            const char * pcBtnTxt = lv_msgbox_get_active_btn_text(obj);
            ESP_LOGI(TAG, "Btn (No%d) %s clicked", (int)btnNo, pcBtnTxt);//编号 文本

            if (btnNo == 0) {//获取按钮索引
                ESP_LOGI(TAG, "continueToConn");
                lv_msgbox_close(obj);//关闭消息框
                sys_setCurMainWinObj(NULL);
                g_cfgNetScrFlg = 1;
                network_setting_entry();
            }
            else if (btnNo == 1) {
                ESP_LOGI(TAG, "cancel");
                lv_msgbox_close(obj);//关闭消息框 
                ESP_LOGI(TAG, "focus restoreTo %d", (int)g_tmp_Term);
                lv_obj_t * obj_cur = lv_obj_get_child(item_obj, g_tmp_Term);
                lv_group_focus_obj(obj_cur); // 恢复列表原选项 
            }
            break;
        }
        case LV_EVENT_KEY: {  // 取消键
            int keyVal = lv_event_get_key(evt);
            if (keyVal == LV_KEY_ESC) {
                lv_msgbox_ensure_be_del();  //关闭消息框 
                lv_obj_t * obj_cur = lv_obj_get_child(item_obj, g_tmp_Term);
                lv_group_focus_obj(obj_cur); // 恢复列表原选项 
                return ;
            }
            break;
        }
        default:
            break;
    }
    return;
}

static void _confirm_del_cb(lv_event_t* evt)
{
    ESP_LOGI(TAG, "%s st", __func__);
    lv_obj_t* obj = lv_event_get_current_target(evt);//获取当前的目标对象（即消息框对象）

    switch (evt->code) {
        case LV_EVENT_DELETE: {
            ESP_LOGI(TAG, "cfrmNetCfgl mbox del");
            //g_confirmCfgNetMbox = NULL;
        }
        break;
        default:
        break;
    }
    return;
}

static void showConfirmNetCfgMBox(void)
{
	static const char* btns[] = { "连接", "取消", "" };

	lv_obj_t * cfgNetMsgbox = NULL;
	cfgNetMsgbox = lv_msgbox_create(NULL, "", "下载同步教材需要连接网络,\n立即连接?", btns, false);
	
	lv_obj_add_event_cb(cfgNetMsgbox, _confirm_enterNetCfg_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(cfgNetMsgbox, _confirm_enterNetCfg_cb, LV_EVENT_KEY, NULL);
	lv_obj_t * mboxParent = lv_obj_get_parent(cfgNetMsgbox);
	lv_obj_add_event_cb(mboxParent, _confirm_del_cb, LV_EVENT_DELETE, NULL); // LV_EVENT_PREPROCESS
	lv_obj_center(cfgNetMsgbox);
	return;
}

static void grade_play_item_btn_click_cb(lv_event_t* e)
{
    char tmp_buf[200] = { 0 };
    lv_obj_t* obj = lv_event_get_target(e);
    
    nmbox.curTerm = lv_obj_get_index(obj);
    if (nmbox.downStatus != STATUS_NONE) {
        pop_msg_downing(e);
        //app_msgbox_timeout("", "正在下载文件...，\n请稍候!", 3000, msgbox_cb, NULL);
        return;
    }
    if (nmbox_book_list.down[nmbox.curTerm] == 1) { // 已经下载了数据，直接去浏览

        char book_version[50] = { 0 };
        snprintf(tmp_buf, sizeof(tmp_buf), "%s%s/%s/%s.ini", PLAYER_ONLINE_DATA_PATH, class_info_tl[player_class_num], publisher, nmbox_book_list.book_id[nmbox.curTerm]);
        
        read_book_version(tmp_buf, book_version, sizeof(book_version));
        snprintf(play_list_book_locate.path, sizeof(play_list_book_locate.path), "%s/%s/%s", class_info_tl[player_class_num], publisher, book_version);
        snprintf(play_list_book_locate.title, sizeof(play_list_book_locate.title), nmbox_book_list.term[nmbox.curTerm]);
		
		play_list_book_locate.cover = player_class_locate.cover;  //封面图片
		
        if (nmbox_book_list.waiyan[nmbox.curTerm] == 0) {
            play_list_page_init(0);
            play_main_clicked_btn_num = nmbox.curTerm;
        }
        else {
            play_quanjie_unit_page_init(0);
            play_main_clicked_btn_num = nmbox.curTerm;

        }
    }
    else {
        //bool dloadStartOk = false;
#ifndef C_PC_SIM
        if (!IS_JC_TOKEN_READY()) {
            int ret = query_token_if_need();
            if ((ret < 0) && !zb_is_wifi_connect()) {
                showConfirmNetCfgMBox();
            }
            else if (ret > 0) {
                app_msgbox_timeout("提示", "请稍侯", 2000|MSGBOX_TOAST_FLAG, NULL, NULL);
            }
            return;
        }

        ESP_LOGI(PLA, "cb== enter : REQUEST_BOOK_READY\n");
        int requesRet = play_http_post_create(REQUEST_BOOK_READY);
        if(requesRet == 0) {
            nmbox.downStatus = STATUS_DOWN_READY;
            print_stt(__LINE__);
            gGradeItemObj = obj;
            //dloadStartOk = true;
            gbar = create_down_bar(obj);
        }
        else if (requesRet == -1) {
            app_msgbox_timeout("", "正在下载中!", 2000, NULL, NULL);
        }
        else if (requesRet == -3) {
            showConfirmNetCfgMBox();
        }
#else

        //showConfirmNetCfgMBox();
        // 测试代码
        nmbox.downStatus = STATUS_DOWN_READY;
        gbar = create_down_bar(obj);
#endif
    }
}

/** 显示提示信息, 并刷新当前选择范围下的 书本清单 数据 
 */
static void showMsgAndQueryBookLst(void)
{
    ESP_LOGI(TAG, "showMsgAndQry A");
    empty_hint("请稍候,获取教材数据...", 3);
    ESP_LOGI(TAG, "file: %s not exist\n", grade_path);
#ifndef C_PC_SIM
    int ret = query_token_if_need();
    if (ret != 0) {
        ESP_LOGI(TAG, "start queryToken ret %d", ret);
    }
    else {
        play_http_post_create(REQUEST_BOOK_LIST);
    }
#endif
}

int play_main_online_page_free(void)
{
    if (play_main_win)
    {
        lv_obj_del(play_main_win);
        play_main_win = NULL;
    }
    if (gtimer) {
        lv_timer_del(gtimer);
        gtimer = NULL;
    }

    zb_book_data_manage_free();
    textbook_quanjie_all_free();
    //---------- 
    play_list_page_free();
    play_text_page_free();
    dictScreen_del(); //销毁词典跳查
    zb_img_cover_unload();

    if (g_plOlApInf) {
        heap_caps_free(g_plOlApInf);  g_plOlApInf = NULL;
    }
    return 0;
}

int play_main_online_page_hander(lv_event_code_t msg, int param)
{
    switch (msg)
    {
    case EVENT_FREE_AP_RES:
    {
        ESP_LOGI(PLA, "EVENT_FREE_AP_RES");
        play_main_online_page_free();
        break;
    }
    case EVENT_EX_CMD:
    {
        ESP_LOGI(PLA, "EVENT_CMD %d", param);
        if (NET_CONN_CMD_CODE == param) {
            if (g_cfgNetScrFlg) {
                network_setting_exit();
                sys_setCurMainWinObj(play_main_win);
                if (1 == g_wantUpdGradeInfFlg) {
                    g_wantUpdGradeInfFlg = 0;
                #ifndef C_PC_SIM
                    if (zb_is_wifi_connect())
                #endif
                    {
                        showMsgAndQueryBookLst();
                    }
                }
                else {
                    ESP_LOGI(TAG, "focus restoreTo %d", (int)g_tmp_Term);
                    lv_obj_t * obj_cur = lv_obj_get_child(item_obj, g_tmp_Term);
                    lv_group_focus_obj(obj_cur);
                }
                g_cfgNetScrFlg = 0;
            }
        }
        break;
    }
    default:
        break;
    }

    return 0;
}
char* get_basename(const char* path) {
    char* start = strrchr(path, '/'); // 查找最后一个'/'
    if (start) {
        return start + 1; // 返回'/'后的字符串
    }
    return path; // 如果没有'/'，则路径就是basename本身
}

// 返回文件名，basename: 最后一级的文件夹
char* xbasename(char* path, char *basename) {
    // 如果路径为空，返回空字符串
    basename[0] = '\0';
    if (path == NULL || *path == '\0') {
        return "";
    }

    // 从路径末尾开始向前查找最后一个路径分隔符
    char* plast = strrchr(path, '/'); // 查找最后一个'/'
    if (plast) {
        *plast = '\0';
        char* pfirst = strrchr(path, '/');
        if (pfirst) {
            strcpy(basename, pfirst + 1);
        }
        else {
            strcpy(basename, path);
        }
        *plast = '/';
        return plast + 1;
    }
    else
        return path;
}

static void play_main_online_win_key_cb(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target(e);
    switch ((int16_t)(e->code)) {
    case LV_EVENT_KEY: {
        int keyVal = lv_event_get_key(e);
        // printf("num == %d\n", (int)lv_group_get_obj_count(lv_group_get_default()));
        switch (keyVal)
        {
        case LV_KEY_UP:
        {
            lv_group_focus_prev(lv_group_get_default());
            lv_obj_t* fobj = lv_group_get_focused(lv_group_get_default());
            uint32_t idx = lv_obj_get_index(fobj);
            g_tmp_Term = (uint8_t)idx;
            ESP_LOGI(TAG, "-----------------idx = %ld\n", idx);
            break;
        }
        case LV_KEY_DOWN:
        {
            lv_group_focus_next(lv_group_get_default());
            lv_obj_t* fobj = lv_group_get_focused(lv_group_get_default());
            uint32_t idx = lv_obj_get_index(fobj);
            g_tmp_Term = (uint8_t)idx;
            ESP_LOGI(TAG, "-----------------idx = %ld\n", idx);
            break;
        }
        //-------------------test start-----------------------
        case LV_KEY_EX_FUNC1: // AB 键 用于删除教材数据
        {
#ifdef C_PC_SIM
            lv_obj_t* fobj = lv_group_get_focused(lv_group_get_default());
            uint32_t idx = lv_obj_get_index(fobj);
            ESP_LOGI(TAG, "-----------------idx = %ld\n", idx);

            char tmp_path[300] = { 0 };
            snprintf(tmp_path, sizeof(tmp_path), "%s%s/%s/%s.ini", PLAYER_ONLINE_DATA_PATH, class_info_tl[nmbox.lang], publisher, nmbox_book_list.book_id[g_tmp_Term]);
            if (zb_isFileExist(tmp_path)) {
                zb_remove(tmp_path);
                ESP_LOGI(TAG, "del file: success:  %s\n", tmp_path);
            }
            else
                ESP_LOGI(TAG, "file: %s not exist\n", tmp_path);
#endif
            break;
        }
        case LV_KEY_DEL:
        {
            //lv_example_msgbox_1();
            //lv_example_msgbox_test();
            //create_down_bar(obj);
            break;
        }
        //-------------------test end------------------------
        case LV_KEY_ESC:
        {
            ESP_LOGI(TAG, "status: %d \n", nmbox.downStatus);
            if (nmbox.downStatus != STATUS_NONE)
            {
                lv_example_msgbox_test();
                lv_event_stop_processing(e);
            }
            else
            {
                ESP_LOGI(TAG, "ESC:  2222");
                if (nmbox.curApLayer == PLAYER_OL_CHAPTER) {
                    play_main_online_page_free();
                    play_main_online_page_init(player_class_num, 0);
                }
                else {
                    zb_requestSwitchAp(C_ZB_AP_MAINMENU);
                }
            }
            break;
        }
        }
        break;
    }
    case LV_EVENT_KEY_STAGE: {
		int keyVal = lv_event_get_key(e);
		if ((LV_EX_FLAG_KEY_LONG_UP|LV_KEY_EX_RECORD) == keyVal) {
			ESP_LOGI(TAG, "will refresh grd fl");
			g_forceRefresh = true;
			break;
		}
		break;
    }
    case LV_EVENT_READY:
    {
        int* pInt = (int*)lv_event_get_param(e);
        int parm = (pInt ? (*pInt) : 0);
        ESP_LOGI(TAG, "receive readyEvt %d", parm);

        switch (parm) {
        case NET_RSLT_QUERY_TOKEN:
        {
#ifndef C_PC_SIM
            if (!IS_JC_TOKEN_READY()) {
                ESP_LOGE(TAG, "fail to get token");
            }
            else {
                if (!nmbox_book_list.book_num) { // 接着查询当前年级的书本列表 
                    play_http_post_create(REQUEST_BOOK_LIST);
                }
            }
#endif
            break;
        }
        case NET_RSLT_BOOK_LIST: // 获取到该年级的book列表
        {
#ifndef C_PC_SIM
            ESP_LOGI(TAG, "get buf: %s \n", g_plyHttpRsltBuf);
            int16_t subject = (DATUM_CHINESE_TEXTBOOK == player_class_num ?  1 : 2);
            int ret = parse_book_list_jstr(g_plyHttpRsltBuf, subject, publisher, &nmbox_book_list);
            if (ret != 0) // 如果网络请求失败或者其他错误，则退出
                break;
            
            ESP_LOGI(TAG, "get book num = %d \n", nmbox_book_list.book_num);
            play_main_clicked_btn_num = nmbox.curTerm;

            // 判断目录是否存在：
            snprintf(grade_path, sizeof(grade_path), "%s%s/%s", PLAYER_ONLINE_DATA_PATH, class_info_tl[nmbox.lang], publisher);
            if (zb_isFileExist(grade_path) == 0)
            {
                ESP_LOGI(TAG, "create dir: %s \n", grade_path);
                zb_mkdir(grade_path, 0);
            }
            else {
                ESP_LOGI(TAG, "dir %s already exist \n", grade_path);
            }

            // 写grd文件
            snprintf(grade_path, sizeof(grade_path), "%s%s/%s/grade_%d.grd", PLAYER_ONLINE_DATA_PATH, class_info_tl[nmbox.lang], publisher, nmbox.curGrade);

            FILE* grdFp = zb_fopen(grade_path, "wb");
            if (grdFp) {
                ESP_LOGI(TAG, "create file: %s \n", grade_path);
                zb_fwrite(g_plyHttpRsltBuf, 1, strlen(g_plyHttpRsltBuf), grdFp);
                zb_fclose(grdFp);
            }

            fill_grage_list();
#endif
            break;
        }
        case NET_RSLT_BOOK_READY: // 后台准备数据请求数据返回
        {
            // 获取到返回值： {\"msg\":\"success\",\"code\":200,\"url\":[\"http://thirdparty.bolledu.net/media/nimibox/清华大学版/tape1a_018002_202310212050/book.bin\"]}
            // 之后准备开始下载 一本书的数据：
            char url_path[250] = { 0 };
            char last_folder_name[100] = { 0 };
            ESP_LOGI(TAG, "后台请求数据返回：[line = %d]\n",  __LINE__);
            print_stt(__LINE__);
            //if (nmbox.downStatus == STATUS_NONE)
            //{
            //    //gCancelUserFlag = false;
            //    ESP_LOGI(TAG, "直接丢弃该返回数据：[line = %d]\n", __LINE__);
            //    break;
            //}
            if (gCancelUserFlag) {
                nmbox.downStatus = STATUS_NONE;
                gCancelUserFlag = false;
                print_stt(__LINE__);
                break;
            }

#ifndef C_PC_SIM
            int ret = parse_bool_down_url_jstr(g_plyHttpRsltBuf, url_path);
#else
            int ret = parse_bool_down_url_jstr("", url_path);
#endif
            // http://speed2.bolledu.net/BOLLTVVideo/upload/data/chenguang-tlb/download/books/nimibox/人教统编版/tape2a_002001_202408231002/book.bin
            //strcpy(url_path, "http://speed2.bolledu.net/BOLLTVVideo/upload/data/chenguang-tlb/download/books/boll/wyxbz3q/wyxbz3q_3_2/book.bin");
            if (ret == 0) {
                ESP_LOGI(TAG, "url_path: %s \n", url_path);

                char* basename = xbasename(url_path, last_folder_name);
                ESP_LOGI(TAG, "folder_name: %s, basename: %s \n", last_folder_name, basename);

                snprintf(grade_path, sizeof(grade_path), "%s%s/%s/%s", PLAYER_ONLINE_DATA_PATH, class_info_tl[player_class_num], publisher, last_folder_name);

                zb_mkdir(grade_path, 0); // 创建书的目录
                ESP_LOGI(TAG, "创建目录: %s[%s(%d)]\n", grade_path, __FUNCTION__, __LINE__);

                //sprintf(grade_path, "%s%s/%s/%s/book.bin", PLAYER_ONLINE_DATA_PATH, class_info_tl[player_class_num], publisher, last_folder_name);

                char* pret = urlEncode(url_path);
                strlcpy(g_plOlApInf->m_remoteUrl, pret, sizeof(g_plOlApInf->m_remoteUrl));
                heap_caps_free(pret);

                dct.download_url = g_plOlApInf->m_remoteUrl;
                dct.download_file_path = grade_path;
                dct.f_process_cb = data_dl_file_progress_cb;
                ESP_LOGI(TAG, "down_path: url_path:%s, destin_path:%s \n", g_plOlApInf->m_remoteUrl, grade_path);
                //gbar = create_down_bar(gGradeItemObj);
                nmbox.downStatus = STATUS_DOWN_BOOK;
                nmbox.downTerm = nmbox.curTerm;
                dpp.cur_len = 0;
                dpp.file_size = 0;
#ifndef C_PC_SIM
                set_http_download_split(true);
                http_download_sd_file(&dct);
                print_stt(__LINE__);
#endif
            }
            else {
                ESP_LOGI(TAG, "解析返回的下载链接数据有误: parse_bool_down_url_jstr[%d]:【%d】 \n", __LINE__, ret);
                //app_msgbox_timeout("", "正在准备数据，请稍候!", 5000, NULL, NULL);
#ifndef C_PC_SIM
                vTaskDelay(pdMS_TO_TICKS(1000));
                nmbox.downStatus = STATUS_DOWN_READY;
                play_http_post_create(REQUEST_BOOK_READY);
                ESP_LOGI(PLA, "失败，再来一次！");
                print_stt(__LINE__);
#endif
            }
            break;
        }
        /*case NET_RSLT_BOOK_DOWN:  // 数据下载完成
        {
            if (cancelMsgbox) { // 如果之前存在弹窗，则先销毁掉
                lv_msgbox_close(cancelMsgbox);
                cancelMsgbox = NULL;
            }
            ESP_LOGI(TAG, "下载完成, %s [%d]", __FUNCTION__, __LINE__);
            char ini_path[300] = { 0 };
            //strcpy(ini_path, dct.download_file_path);
            char* plast = strrchr(dct.download_file_path, '/');
            *plast = 0;
            sprintf(ini_path, "%s/%s.ini", dct.download_file_path, nmbox_book_list.book_id[nmbox.curTerm]);
            ESP_LOGI(TAG, "写 [%s] [%d]", ini_path, __LINE__);
            FILE* iniFp = zb_fopen(ini_path, "wb");
            if (iniFp) {
                plast++;
                zb_fwrite(plast, 1, strlen(plast), iniFp);
                ESP_LOGI(TAG, "写数据文件【%s】：%s\n", ini_path, plast);
                zb_fclose(iniFp);
            }
            else {
                ESP_LOGI(TAG, "打开文件失败：%s\n", ini_path);
            }
            *(plast - 1) = '/';
            restore_before_down(gtimer->user_data, true);
            nmbox_book_list.down[nmbox.curTerm] = 1; // 置标志位，表示已经下载了
            if (gtimer) {                             // 删除timer
                lv_timer_del(gtimer);
                ESP_LOGI(TAG, "DEL TIMER 11\n");
                gtimer = NULL;
            }
            nmbox.downStatus = STATUS_NONE;
            break;
        }*/
        default:
            break;
        }
        break;
    }
    case C_EVENT_EX_DOWNLOAD_STOP: {
        int* pInt = (int*)lv_event_get_param(e);
        int parm = (pInt ? (*pInt) : 0);
        ESP_LOGI(TAG, "-----dload stop %d", parm);

        if (cancelMsgbox) { // 如果之前存在弹窗，则先销毁掉
            lv_msgbox_close(cancelMsgbox);
            cancelMsgbox = NULL;
        }
        if (parm == 1) {
            ESP_LOGI(TAG, "下载完成, %s [%d]", __FUNCTION__, __LINE__);
            char ini_path[300] = { 0 };
            //strcpy(ini_path, dct.download_file_path);
            char* plast = strrchr(dct.download_file_path, '/');
            *plast = 0;
            snprintf(ini_path, sizeof(ini_path), "%s/%s.ini", dct.download_file_path, nmbox_book_list.book_id[nmbox.downTerm]);
            ESP_LOGI(TAG, "写 [%s] [%d]", ini_path, __LINE__);
            FILE* iniFp = zb_fopen(ini_path, "wb");
            if (iniFp) {
                plast++;
                zb_fwrite(plast, 1, strlen(plast), iniFp);
                ESP_LOGI(TAG, "写数据文件【%s】：%s\n", ini_path, plast);
                zb_fclose(iniFp);
            }
            else {
                ESP_LOGI(TAG, "打开文件失败：%s\n", ini_path);
            }
            *(plast - 1) = '/';
            restore_before_down(gtimer->user_data, true);
            nmbox_book_list.down[nmbox.downTerm] = 1; // 置标志位，表示已经下载了
        }
        else {
            ESP_LOGI(TAG, "下载失败, %s [%d]", __FUNCTION__, __LINE__);
            if (gtimer) {
                restore_before_down(gtimer->user_data, false);
            }
            app_msgbox_timeout(NULL, "下载失败，请重试", 1500, NULL, NULL);
            lv_group_focus_obj(focus_obj);
        }
        if (gtimer) {                             // 删除timer
            lv_timer_del(gtimer);
            ESP_LOGI(TAG, "DEL TIMER 11\n");
            gtimer = NULL;
        }
        nmbox.downStatus = STATUS_NONE;
        break;
    }
    case LV_EVENT_DELETE:
        if (gtimer) {
            lv_timer_del(gtimer);  gtimer = NULL;
            ESP_LOGI(TAG, "del tmr when objDel");
        }
        break;
    default:
        break;
    }
    return;
}

void lv_Arc_style(lv_obj_t* parent)
{
    static lv_style_t style;                        //创建样式
    lv_style_init(&style);                          //初始化样式
    lv_style_set_arc_color(&style, lv_palette_main(LV_PALETTE_RED)); //设置圆弧颜色
    lv_style_set_arc_opa(&style, LV_OPA_20);
    lv_style_set_arc_width(&style, 4);              //设置圆弧的宽度
    lv_obj_t* obj = lv_arc_create(parent);          //创建圆弧对象
    lv_obj_add_style(obj, &style, 0);               //将样式添加到圆弧中
    lv_obj_center(obj);                             //圆弧样式加载在中央     
}


// bar对象动画回调函数
static void obj_bar_anim_exec_callback(lv_timer_t* timer)
{
    zb_clear_autoPwrOffCnt();
    zb_clear_autoScreenOffCnt();
    //ESP_LOGI(TAG, "________timer________\n");
    if (gbar != NULL)
    {
        if (nmbox.downStatus == STATUS_DOWN_BOOK)
        {
            //ESP_LOGI(TAG, "timer---down: file_size = %ld, cur_len = %ld\n", dpp.file_size, dpp.cur_len);
            if (dpp.file_size >0 && dpp.cur_len >= dpp.file_size)
            {
                ESP_LOGI(TAG, "下载完成. %s [%d]",__FUNCTION__, __LINE__);
//#ifndef C_PC_SIM
//                zb_postMsgToMainTask(LV_EVENT_READY, NET_RSLT_BOOK_DOWN);
//#endif
            }
            else if (dpp.file_size > 0) {
                int value = get_percent(dpp.file_size, dpp.cur_len, READY_PERCENT, DOWN_PERCENT);
                ESP_LOGI(TAG, "timer__  file_size = %ld, cur_len = %ld,setValue = %d\n", dpp.file_size, dpp.cur_len, value);
                lv_bar_set_value(gbar, value, LV_ANIM_OFF); // 设置进度条对象的值
            }
        }
        else if (nmbox.downStatus == STATUS_DOWN_READY)
        {
            ESP_LOGI(TAG, "数据准备中...");
        }
        else {
            nmbox.downStatus = STATUS_NONE;
            //restore_before_down(gtimer->user_data, false); // 这一行会死掉
            ESP_LOGI(TAG, "enter : timer: file is empty status = [%d]\n", nmbox.downStatus);
        }
    }
}

lv_obj_t* create_down_bar(lv_obj_t* parent)
{
    if (false) {
        gbar = lv_label_create(parent);//创建bar背景样式
        //lv_obj_remove_style_all(gbar);          //移除样式
        lv_label_set_text(gbar, "请稍候...");
        lv_obj_set_style_text_color(gbar, lv_color_black(), 0);
        lv_obj_set_size(gbar, 120, 20);
        //lv_obj_set_style_border_width(gbar, 2, 0);
        //lv_obj_set_style_border_color(gbar,lv_color_black(),0);
        //gtimer = lv_timer_create(obj_bar_anim_exec_callback, 600, NULL);
        //gtimer->user_data = parent;
        //lv_bar_set_range(gbar, -0, 100);	//设置进度条的范围值

            //    //lv_obj_t *bar = create_down_bar(container);
    //    //lv_obj_align_to(bar, img, LV_ALIGN_OUT_RIGHT_BOTTOM, 8, 0);
        uint8_t num = lv_obj_get_child_cnt(parent);
        lv_obj_t* img_one = lv_obj_get_child(parent, 0);
        lv_obj_t* lab_two = lv_obj_get_child(parent, 1);
        //lv_obj_set_style_img_recolor_opa(img_one, 0, 0);
        //lv_obj_clean(img_one);
        //lv_obj_del(lv_obj_get_child(gtimer->user_data, 2));  // 删除 bar
        lv_obj_align_to(lab_two, img_one, LV_ALIGN_OUT_RIGHT_TOP, BETWEEN_BOOKCOVER_TEXT_SPACE, 2);
        lv_obj_align_to(gbar, img_one, LV_ALIGN_OUT_RIGHT_BOTTOM, BETWEEN_BOOKCOVER_TEXT_SPACE, -1);

        //lv_obj_set_size(gbar, 150, 20);           //设置样式尺寸
        //lv_bar_set_value(gbar, 0, LV_ANIM_ON);    //设置初始值
    }
    else {
        static lv_style_t style_bg;            //创建bar背景样式
        static lv_style_t style_indic;         //创建bar指示器样式

        lv_style_init(&style_bg);              //初始化样式
        lv_style_set_border_color(&style_bg, lv_palette_main(LV_PALETTE_BLUE));//设置边款颜色
        lv_style_set_border_width(&style_bg, 2);//设置边框宽度
        lv_style_set_pad_all(&style_bg, 6);     //设置内边距
        lv_style_set_radius(&style_bg, 6);      //设置圆角
        //lv_style_set_anim_time(&style_bg, 1000);//设置动画时间

        lv_style_init(&style_indic);           //初始化样式
        lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);//设置样式背景透明度
        lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));//设置背景颜色
        lv_style_set_radius(&style_indic, 3);   //设置圆角

        gbar = lv_bar_create(parent);//创建bar背景样式
        lv_obj_remove_style_all(gbar);          //移除样式
        lv_obj_add_style(gbar, &style_bg, 0);     //创添样式
        lv_obj_add_style(gbar, &style_indic, LV_PART_INDICATOR);//添加样式
        gtimer = lv_timer_create(obj_bar_anim_exec_callback, 1300, NULL);
        gtimer->user_data = parent;
        lv_bar_set_range(gbar, -0, 100);	//设置进度条的范围值

            //    //lv_obj_t *bar = create_down_bar(container);
    //    //lv_obj_align_to(bar, img, LV_ALIGN_OUT_RIGHT_BOTTOM, 8, 0);
        uint8_t num = lv_obj_get_child_cnt(parent);
        lv_obj_t* img_one = lv_obj_get_child(parent, 0);
        lv_obj_t* lab_two = lv_obj_get_child(parent, 1);
        //lv_obj_set_style_img_recolor_opa(img_one, 0, 0);
        //lv_obj_clean(img_one);
        //lv_obj_del(lv_obj_get_child(gtimer->user_data, 2));  // 删除 bar
        lv_obj_align_to(lab_two, img_one, LV_ALIGN_OUT_RIGHT_TOP, BETWEEN_BOOKCOVER_TEXT_SPACE, 2);
        lv_obj_align_to(gbar, img_one, LV_ALIGN_OUT_RIGHT_BOTTOM, BETWEEN_BOOKCOVER_TEXT_SPACE, -9);

        lv_obj_set_size(gbar, 150, 20);           //设置样式尺寸
        lv_bar_set_value(gbar, 0, LV_ANIM_ON);    //设置初始值
    }

    return gbar;
}

int nmbox_play_main_page_free(void)
{
    gCancelUserFlag = false;
    //memset(&nmbox, 0, sizeof(nmbox_t));
    return 0;
}


int fill_grage_list()
{
    lv_img_dsc_t* book_img = NULL;
    ESP_LOGI(TAG, "enter : fill_grage_list\n");

    uint32_t num = lv_obj_get_child_cnt(item_obj);
    if (num > 0) {
        lv_obj_t* child = lv_obj_get_child(item_obj, 0);
        lv_obj_del(child);
    }

    lv_obj_set_style_pad_top(item_obj, 15, 0);
    lv_obj_set_style_pad_left(item_obj, 15, 0);
    if (nmbox.lang == ENGLISH)
        book_img = zb_img_res_load_one(NULL, PLAYER_IMG_ENGBOOKCOVER, IMG_FREE_TYPE_AUTO, NULL);
    else
        book_img = zb_img_res_load_one(NULL, PLAYER_IMG_CHNBOOKCOVER, IMG_FREE_TYPE_AUTO, NULL);

    for (int i = 0; i < nmbox_book_list.book_num; i++)
    {
        // 创建一个容器来放置图片和文本
        lv_obj_t* container = lv_btn_create(item_obj);
        lv_obj_set_width(container, lv_disp_get_hor_res(NULL) - 30); // 设置容器宽度为屏幕宽度
        lv_obj_set_height(container, 61 + 4); // 设置容器高度
        lv_obj_align(container, LV_ALIGN_LEFT_MID, 0, 0); // 在屏幕左边中间位置对齐容器
        lv_obj_set_style_bg_color(container, lv_color_white(), 0);
        lv_obj_set_style_border_width(container, 1, 0);
        lv_obj_set_style_border_color(container, lv_color_hex(0x909090), 0);
        lv_obj_add_event_cb(container, grade_play_item_btn_click_cb, LV_EVENT_CLICKED, NULL);
        //lv_group_add_obj(lv_group_get_default(), container);

        // 在容器中添加图片
        lv_obj_t* img = lv_img_create(container);
        if (nmbox_book_list.down[i] == 1)
        {
            lv_img_dsc_t* pimg = get_book_cover_img(nmbox_book_list.version, nmbox_book_list.book_id[i]);
            if (pimg != NULL)
                lv_img_set_src(img, pimg);
            else
                lv_img_set_src(img, book_img);
        } else 
            lv_img_set_src(img, book_img);
        lv_obj_set_width(img, 48); // 设置图片宽度
        lv_obj_set_height(img, 61); // 设置图片高度
        lv_obj_align(img, LV_ALIGN_LEFT_MID, 0, 0); // 在容器左边中间位置放置图片
        if (nmbox_book_list.down[i] == 0)
        {
            lv_obj_t* down_img = lv_img_create(img);
            lv_img_set_src(down_img, zb_img_res_load_one(NULL, PLAYER_IMG_DOWNLOAD, IMG_FREE_TYPE_AUTO, NULL));
            //lv_obj_set_style_opa(down_img, LV_OPA_60, 0);
            lv_obj_set_style_img_recolor_opa(img, LV_OPA_60, 0);
            lv_obj_center(down_img);
            lv_obj_set_size(down_img, 17, 17);
            //lv_Arc_style(img);
        }

        // 在容器中添加文本
        lv_obj_t* label = lv_label_create(container);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(label, 140);
        //lv_obj_set_style_border_width(label, 2, 0);
        //lv_obj_set_style_border_color(label, lv_color_black(), 0);
        lv_obj_set_style_text_color(label, lv_color_black(), LV_STATE_DEFAULT);

        //if (nmbox_book_list.down[i] == 0) {
        //    lv_label_set_text(label, nmbox_book_list.term[i]); // 设置文本内容
        //    //lv_obj_align_to(label, img, LV_ALIGN_OUT_RIGHT_TOP, 8, 0); // 在图片的右边中间位置放置文本

        //    //lv_obj_t *bar = create_down_bar(container);
        //    //lv_obj_align_to(bar, img, LV_ALIGN_OUT_RIGHT_BOTTOM, 8, 0);
        //}
        //else {
        lv_label_set_text(label, nmbox_book_list.term[i]); // 设置文本内容
        lv_obj_align_to(label, img, LV_ALIGN_OUT_RIGHT_MID, BETWEEN_BOOKCOVER_TEXT_SPACE, 0); // 在图片的右边中间位置放置文本
    //}
        if (i == play_main_clicked_btn_num)
        {
            lv_obj_scroll_to_view(container, LV_ANIM_OFF);
            lv_group_focus_obj(container);
        }
    }
    if (nmbox_book_list.book_num == 0)
    {
#ifndef C_PC_SIM
        /*if (!zb_is_wifi_connect())
            app_msgbox_timeout("", "请先打开网络连接!", 3000, NULL, NULL);
        else*/
            app_msgbox_timeout("", "没找到该年级对应的教材!", 3000, NULL, NULL);
#else
        app_msgbox_timeout("", "没找到该年级对应的教材!", 3000, NULL, NULL);
#endif
    }

    return 0;
}

void book_manage_btn_click_cb(lv_event_t* e)
{
    if (nmbox.downStatus != STATUS_NONE) {
        pop_msg_downing(e);
        return;
    }

    play_main_page_free();
    zb_book_data_manage_init(class_info_tl[nmbox.lang]);
    return;
}

extern int testMtList(void);




static void timer_grade_cb(lv_timer_t * tmr)
{
    ESP_LOGI(TAG, "选择教材版本后退出！");
    play_main_online_page_free();
    zb_requestSwitchAp(C_ZB_AP_MAINMENU);
    if (gtimer) {
        lv_timer_del(gtimer);
        gtimer = NULL;
    }
}

/*
* 显示空的信息
* msg ：待显示字符串
* genre：种类：1：没有选择教材版本； 2：没有联网；3：从服务器获取数据，等待中...
*/
static int empty_hint(const char *msg, uint8_t genre) {
    if (true) {
        //lv_obj_t* container = lv_btn_create(item_obj);
        //lv_obj_set_width(container, lv_disp_get_hor_res(NULL) - 30); // 设置容器宽度为屏幕宽度
        //lv_obj_set_height(container, 61 + 4); // 设置容器高度
        //lv_obj_align(container, LV_ALIGN_CENTER, 0, 0); // 在屏幕左边中间位置对齐容器
        //lv_obj_t* watting_msg = lv_label_create(container);
        //lv_label_set_text(watting_msg, msg);
        //lv_obj_align(watting_msg, LV_ALIGN_CENTER, 0, 0); // 在屏幕左边中间位置对齐容器

        lv_obj_t* container = lv_obj_create(item_obj);

        lv_obj_set_style_bg_color(container, lv_color_make(128, 128, 128), 0);
        lv_obj_set_size(container, 200, 64);
        lv_obj_set_style_radius(container, 12, 0);

        lv_label_t* msg_label = lv_label_create(container);
        lv_label_set_text(msg_label, msg);
        lv_obj_set_style_text_color(msg_label, lv_color_white(), 0);
        lv_obj_set_style_align(msg_label, LV_ALIGN_CENTER, 0);
    }
    else {
        lv_obj_t* waitting = lv_img_create(item_obj);
        lv_img_set_src(waitting, zb_img_res_load_one(NULL, BAIDUZIYUAN_IMG_JIAZAI, IMG_FREE_TYPE_AUTO, NULL)); // 设置图片路径
        lv_obj_set_size(waitting, 186, 50);
        lv_obj_set_style_pad_top(item_obj, 100, 0);
        lv_obj_set_style_pad_left(item_obj, 25, 0);
        //lv_obj_align(waitting, LV_ALIGN_CENTER, 20, 120); // 在容器左边中间位置放置图片
    }
    if (genre == 1 || genre == 2) {
        gtimer = lv_timer_create(timer_grade_cb, 2000, NULL);
    }
    return 0;
}

static bool doesGrdFileNeedFlush(const char *grade_path, int isLongPeriod)
{
	int64_t timeA = zb_stat__mtime(grade_path);
	int64_t timeCur = (int64_t)time(NULL);
	ESP_LOGI(TAG, "~~~ fileTm: %lld, curTm: %lld", timeA, timeCur);

	int64_t diff = timeCur - timeA;
	int diffBase = (isLongPeriod ? 3600 * 8 : 60 * 10);
	if (diff > diffBase || diff < 0) {
		ESP_LOGI(TAG, "need flush");
		return true;
	}
	else
		return false;
}


/*
* 显示年级列表: show_grade
* direction: 1: 表示前进， 0： 表示后退
*/
int play_main_online_page_init(int classNum,char direction)
{
    char url_buf[300], temp[300];
    char path[300] = { "http://thirdparty.bolledu.net/media/nimibox/清华大学版/tape1a_018002_202310212050" };
    //char* basename = xbasename(path, temp);
    ////char *p = urlEncode("http://thirdparty.bolledu.net/media/nimibox/清华大学版/tape1a_018002_202310212050/book.bin");
    //parse_bool_down_url_jstr("{\"msg\":\"success\",\"code\":200,\"url\":[\"http://thirdparty.bolledu.net/media/nimibox/清华大学版/tape1a_018002_202310212050/book.bin\"]}",url_buf);    //get_basename(url_buf);
    //test_split_book();
    //xbasename(url_buf, temp);
    //testMtList();
    snprintf(temp, sizeof(temp), "%srsh3.bin", PLAYER_ONLINE_DATA_PATH);
	ap_toggle_num = -1;  //初始化播放类型

 #ifndef C_PC_SIM
    uint64_t total_s, total_f;
    esp32s3_get_sdcard_capacity(&total_s, &total_f);
    ESP_LOGI(TAG, "total = %lld; free = %lld", total_s, total_f);
    if (0) {
        //http_download("http://www.httpwatch.com/httpgallery/chunked/chunkedimage.aspx", temp);
        http_download("http://speed2.bolledu.net/BOLLTVVideo/upload/data/chenguang-tlb/download/tape3a_012002_202310220205/book.bin", temp);
    }
#endif // !C_PC_SIM
    play_main_page_free();
    nmbox_play_main_page_free();
    ESP_LOGI(TAG, "enter : online\n");
    g_plOlApInf = (plyOnlnApInf_t *) heap_caps_malloc_dflt(sizeof(plyOnlnApInf_t));

    if (nmbox.lang != classNum)
    {
        nmbox.curTerm = 0;
        nmbox.curGrade = 0;
    }

    nmbox.lang = classNum;
    nmbox.curApLayer = PLAYER_OL_GRADE;
    ESP_LOGI(TAG, "before : publisher\n");
    if (classNum == ENGLISH) {
        // TODO TEST
        getPrjGlobalParm(ZB_PARM_BOOK_PUBL_ENG, sizeof(publisher), publisher);
        nmbox.curGrade = getPrjGlobalParm(ZB_PARM_BOOK_GARDE_ENG, 0, NULL);
        //sprintf(publisher, "%s", "外研新标准3起");
        //nmbox.curGrade = 3;
    }
    else {
        getPrjGlobalParm(ZB_PARM_BOOK_PUBL_CHN, sizeof(publisher), publisher);
        nmbox.curGrade = getPrjGlobalParm(ZB_PARM_BOOK_GARDE_CHN, 0, NULL);
    }

    play_main_win = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(play_main_win);
    lv_obj_set_size(play_main_win, SCREEN_WIDTH, SCREEN_HEIGHT);
    ESP_LOGI(TAG, "%s line[%d]\n", __FUNCTION__, __LINE__);
    if (player_class_num != classNum)
    {
        play_main_clicked_btn_num = 0;
        play_list_clicked_btn_num = 0;
    }
    player_class_num = classNum;
    player_style = player_normal_style;

    ESP_LOGI(TAG, "%s line[%d]\n", __FUNCTION__, __LINE__);
    lv_obj_add_event_cb(play_main_win, play_main_online_win_key_cb, LV_EVENT_ALL, NULL);
    //lv_group_add_obj(lv_group_get_default(), play_main_win);
    sys_setCurMainWinObj(play_main_win);

    // 背景
    ESP_LOGI(TAG, "%s line[%d]\n", __FUNCTION__, __LINE__); 
    lv_obj_t* bgImg = lv_img_create(play_main_win);
    lv_img_dsc_t* pImgDsc = zb_img_res_load_one(NULL, player_style.page_bg_img1, IMG_FREE_TYPE_AUTO, NULL);
    lv_img_set_src(bgImg, pImgDsc);

    // 标题
    ESP_LOGI(TAG, "%s line[%d]\n", __FUNCTION__, __LINE__); 
    lv_obj_t* title_label = lv_label_create(play_main_win);
    lv_label_set_text(title_label, class_info_tl[classNum]);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(title_label, &lv_font_simsun_16_cjk, 0);
    lv_obj_set_pos(title_label, 15, 15);

    // 教材管理
    ESP_LOGI(TAG, "%s line[%d]\n", __FUNCTION__, __LINE__);
    //if (1) {
        lv_obj_t* book_manage_btn = lv_btn_create(play_main_win);
        lv_obj_set_size(book_manage_btn, 80, 30);
        lv_obj_align_to(book_manage_btn, title_label, LV_ALIGN_OUT_RIGHT_MID, 60, 0);
        lv_obj_set_style_bg_color(book_manage_btn, lv_color_white(), 0);
        lv_obj_set_style_border_width(book_manage_btn, 1, 0);
        lv_obj_set_style_border_color(book_manage_btn, lv_color_white(), 0);
        lv_obj_add_event_cb(book_manage_btn, book_manage_btn_click_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_t* book_manage_label = lv_label_create(book_manage_btn);
        lv_label_set_text(book_manage_label, "下载管理");
        lv_obj_set_style_text_color(book_manage_label, lv_color_black(), LV_STATE_DEFAULT);
        // lv_obj_set_style_text_font(title_label, &lv_font_simsun_16_cjk, 0);
        lv_obj_center(book_manage_label);
    //}

    // 项目列表
    ESP_LOGI(TAG, "%s line[%d]\n", __FUNCTION__, __LINE__);
    item_obj = lv_obj_create(play_main_win);
    lv_obj_set_size(item_obj, SCREEN_WIDTH, 267);
    lv_obj_align_to(item_obj, play_main_win, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(item_obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_top(item_obj, 15, 0);
    lv_obj_set_style_pad_left(item_obj, 15, 0);
    lv_obj_set_style_pad_row(item_obj, 15, 0);
    ESP_LOGI(TAG, "%s line[%d]\n", __FUNCTION__, __LINE__);
    char* bookBuf = NULL;

    snprintf(grade_path, sizeof(grade_path), "%s%s/%s/grade_%d.grd", PLAYER_ONLINE_DATA_PATH, class_info_tl[classNum], publisher, nmbox.curGrade);
    ESP_LOGI(TAG, "publisher = %s, grade=%d\n", publisher, nmbox.curGrade);

    bool doesShowNetCfgPromt = false;
    bool isWifiConnect = false;
#ifndef C_PC_SIM
    isWifiConnect = zb_is_wifi_connect();
#endif
    if (strlen(publisher) == 0) {
        empty_hint("请先到[教材选择]里\n选择版本信息", 1);
    }
    else if (zb_isFileExist(grade_path) != 0 && 
                (!isWifiConnect || (!g_forceRefresh && !doesGrdFileNeedFlush(grade_path, 1))))
    {
        ESP_LOGI(TAG, "file: %s exist\n", grade_path);
        read_grd_parse(grade_path, &nmbox_book_list);
        fill_grage_list();
    }
#ifndef C_PC_SIM
    else if (!zb_is_wifi_connect()) {
        doesShowNetCfgPromt = true;
    }
#endif
    else {
        showMsgAndQueryBookLst();
    }

    g_forceRefresh = false;
    player_class_locate.path = "";
    player_class_locate.cover = -1;

    if (direction == 0) { // 从教材管理返回
        lv_group_focus_obj(book_manage_btn);
    }
    if (doesShowNetCfgPromt) {
        g_wantUpdGradeInfFlg = 1;
        ESP_LOGI(TAG, "bfr mbox, bookN:%d", (int)nmbox_book_list.book_num);
        showConfirmNetCfgMBox();
    }
    //-------------------------
    return 0;
}

static void event_cb(lv_event_t* e)
{
    if (e->code == LV_EVENT_KEY) { // 取消键，删除对话框，保持之前的状态
        int keyVal = lv_event_get_key(e);
        ESP_LOGI(TAG, "ENTER:222, key = %d  \n", keyVal);
        if (keyVal == LV_KEY_ESC) {
            lv_msgbox_close(cancelMsgbox);
            cancelMsgbox = NULL;
            lv_group_focus_obj(focus_obj);
            return ;
        }
    }
    else if (e->code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t* obj = lv_event_get_current_target(e);
        LV_LOG_USER("Button %s clicked", lv_msgbox_get_active_btn_text(obj));
        uint16_t idx = lv_msgbox_get_active_btn(cancelMsgbox);
        LV_LOG_USER("clicked which btn: %d\n", idx);

        lv_msgbox_close(cancelMsgbox);
        cancelMsgbox = NULL;

        if (idx == 1) {
            if (gtimer) {
                restore_before_down(gtimer->user_data, false);
                lv_timer_del(gtimer);
                ESP_LOGI(TAG, "DEL TIMER 22\n");
                gtimer = NULL;
            }
            else {
                ESP_LOGI(TAG, "TIMER is NULL line = %d\n", __LINE__);
            }
            if (nmbox.downStatus == STATUS_DOWN_BOOK) { // 已经是下载 book.bin, 直接终止，后续没有请求返回
#ifndef C_PC_SIM
                http_stop_download_task();
                //ESP_LOGI(TAG, "删除目录： %s", dct.download_file_path);
                //if(zb_access(dct.download_file_path,0) == 0) {
                //    zb_rmdir(dct.download_file_path);

                //    char tmp_buf[260] = { 0 };
                //    sprintf(tmp_buf, "%s%s/%s/%s.ini", PLAYER_ONLINE_DATA_PATH, class_info_tl[player_class_num], publisher, nmbox_book_list.book_id[nmbox.curTerm]);
                //    ESP_LOGI(TAG, "删除文件： %s", tmp_buf);
                //    zb_remove(tmp_buf);
                //}
#endif
                ESP_LOGI(TAG, "cancel : STATUS_DOWN_BOOK\n");
                nmbox.downStatus = STATUS_NONE;
                print_stt(__LINE__);
            }
            else if (nmbox.downStatus == STATUS_DOWN_READY){
                gCancelUserFlag = true;  // 后续还有网络请求返回
                //nmbox.downStatus = STATUS_NONE;  // 直接给状态终止，请求消息返回不做处理
                ESP_LOGI(TAG, "cancel : STATUS_DOWN_READY\n");
                print_stt(__LINE__);
                #ifdef C_PC_SIM
                    nmbox.downStatus = STATUS_NONE;
                #endif
            }
        }
        else {
            LV_LOG_USER("----- continue ------, status = %d", nmbox.downStatus);
        }

        ESP_LOGI(TAG, "restore item focus: %d", g_tmp_Term);
        //lv_group_focus_obj(lv_obj_get_child(item_obj, g_tmp_Term));
        lv_group_focus_obj(focus_obj);
    }
}

static void lv_example_msgbox_test(void)
{
    ESP_LOGI(TAG, "enter msgbox");
    static const char* btns[] = { "取消", "确定", "" };

    focus_obj = lv_group_get_focused(lv_group_get_default());
    //cancelMsgbox = lv_msgbox_create(NULL, "       退出", "\n    正在下载教材，\n    确定要退出吗？\n", btns, false);
    cancelMsgbox = lv_msgbox_create(NULL, "", "\n    正在下载教材，\n    确定要退出吗？\n", btns, false);
    lv_obj_set_size(cancelMsgbox, 200, 130);

    lv_obj_add_event_cb(cancelMsgbox, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(cancelMsgbox, event_cb, LV_EVENT_KEY, NULL);
    lv_obj_center(cancelMsgbox);
    ESP_LOGI(TAG, "exit msgbox");
}

static void ensureTxtBookFolderExist(void) {
    if (!zb_isFileExist(PLAYER_ONLINE_DATA_PATH)) {
        zb_mkdir(PLAYER_ONLINE_DATA_PATH, 0777);
    }

    if (!zb_isFileExist(PLAYER_ONLINE_DATA_PATH "语文教材")) {
        zb_mkdir(PLAYER_ONLINE_DATA_PATH "语文教材", 0777);
    }
    if (!zb_isFileExist(PLAYER_ONLINE_DATA_PATH "英语教材")) {
        zb_mkdir(PLAYER_ONLINE_DATA_PATH "英语教材", 0777);
    }
    return;
}

int zb_ap_textbook_ap_init(void)
{
    ESP_LOGI(TAG, "txtbokAp init A");
    ensureTxtBookFolderExist();
    uint16_t apInParm = zb_getApInParm();
    play_main_online_page_init(apInParm, 1);
    return 0;
}

int zb_ap_textbook_ap_handler(lv_event_code_t msg, int param)
{
    play_main_online_page_hander(msg, param);
    return 0;
}



#endif

int read_book_version(const char* ini_path, char* book_verison, int bufLen) {
    FILE* fp = zb_fopen(ini_path, "rb");
    book_verison[0] = '\0';
    if (fp) {
        int rdLen = zb_fread(book_verison, 1, bufLen - 1, fp);
        if (rdLen >= 0 && rdLen < bufLen) {
            book_verison[rdLen] = '\0';
        }
        zb_fclose(fp);  fp = NULL;
    }
    else {
        return -1;
    }
    ESP_LOGI(TAG, "该本书的详细版本是：%s", book_verison);
    return 0;
}

struct lv_img_dsc_t* get_book_cover_img(char* version, char* book_id)
{
    //char* pSlash = NULL;
    //char mnu_buf[300], book_version[60] = { 0 };

    //if (book_id == NULL) {
    //    pSlash = strchr(version, '/');
    //    if (pSlash != NULL) {
    //        *pSlash = 0;
    //        book_id = pSlash + 1;
    //    }
    //}
    ////
    //snprintf(mnu_buf, sizeof(mnu_buf), "%s%s/%s/%s.ini", PLAYER_ONLINE_DATA_PATH, class_info_tl[player_class_num], version, book_id);
    //read_book_version(mnu_buf, book_version, sizeof(book_version));
    //snprintf(mnu_buf, sizeof(mnu_buf), "%s%s/%s/%s/album.mnu", PLAYER_ONLINE_DATA_PATH, class_info_tl[player_class_num], version, book_version);
    //lv_img_dsc_t* pimg = zb_img_cover_load_one(mnu_buf, NULL);

    //if (pSlash != NULL)
    //    *pSlash = '/';

    //return pimg;
    NULL;
}


int read_grd_parse(const char* path, nmbox_book_list_t* nblist) {
    //if (zb_isFileExist(path)) {
    //    FILE* gfp = zb_fopen(path, "rb");
    //    fseek(gfp, 0, SEEK_END);
    //    int text_length = ftell(gfp);
    //    char* bookBuf = (char*)heap_caps_malloc_dflt(text_length + 1);
    //    memset(bookBuf, 0, text_length + 1);
    //    rewind(gfp);

    //    fread(bookBuf, 1, text_length, gfp);
    //    zb_fclose(gfp);
    //    int16_t subject = (DATUM_CHINESE_TEXTBOOK == player_class_num ? 1 : 2);
    //    parse_book_list_jstr(bookBuf, subject, publisher, nblist);

    //    play_main_clicked_btn_num = nmbox.curTerm;
    //    heap_caps_free(bookBuf);
    //    return -1;
    //}
    //else {
    //    nblist->book_num = 0;
    //}
    return 0;
}

bool current_textbook_is_quanjie() {
    //if (player_class_num == ENGLISH || player_class_num == CHINESE_TEXTBOOK)
    //{
    //    if (nmbox_book_list.waiyan[nmbox.curTerm] == 1)
    //        return true;
    //    else
    //        return false;
    //}
    //else
        return false;
}