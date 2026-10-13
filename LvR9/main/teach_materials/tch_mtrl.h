#ifndef TCH_MTRL_H
#define TCH_MTRL_H

#include <stdio.h>
#include "../utils/misc_utils.h"
#include <string.h>
#include <sys/stat.h>
#include "../mainmenu/tlb_main_img.h"


#ifndef C_PC_SIM
#include <curl/curl.h>
#include <pthread.h>
#endif

enum {
    TCH_MTRL_TEXT_ENG,
    TCH_MTRL_TEXT_CN,
};

typedef enum  {
    FUN_FORWARD,
    FUN_BACKWARD,
    FUN_CONFIRM,
    FUN_CANCEL,
} FUN_DIRECTION;
#define		TCH_TCH_MTRL			0
#define		TCH_CFG_SETTING			1
#define 	TCH_CONTENT				2
#define 	TCH_DOWNLOAD			3

#define NET_BOOK_PATH "http://speed2.bolledu.net/BOLLTVVideo/upload/data/chenguang-tlb/download/books"

#ifndef C_PC_SIM
#define BOOK_PATH "oem"
#else
#define BOOK_PATH "userdata"
#endif

#define TCH_MTRL_MNU_ITEM_N  4  // 测试项总数 

#define TCH_MTRL_ESC_BTN_LOC_Y  10
#define TOP_BTN_BORDER 7
#define TOP_BTN_TOP_SPACE_H 8

#define TCH_HEADER_HEIGHT 56
#define BOOK_LIST_RADIUS_SIZE 12

typedef struct split_book {
    uint16_t file_cnt;
    uint16_t unit_sum;
    uint32_t header_size;
    uint32_t file_size;
    uint32_t pos;		// 记录文件指针位置
    uint32_t* addr_lst;
} down_split_book_t;


// 下载进度回调结构体
typedef struct {
    char url[ZB_MAX_PATH];                   // 下载URL
    char save_path[ZB_MAX_PATH];             // 保存路径
    lv_obj_t* progress_bar;
    lv_obj_t* cover_obj;
    bool is_downloading;
    bool cancel_requested;
    uint32_t total_bytes;
    uint32_t downloaded_bytes;
    //lv_timer_t *timer;
#ifndef C_PC_SIM
    pthread_t thread;
    pthread_mutex_t mutex;
#endif
    FILE* fp;
    down_split_book_t down_split;
} download_data_t;

extern const lv_img_dsc_t  g_XXX_img_dsc_tbl[];

extern lv_obj_t  * g_tchMnuBgWin;
extern uint8_t g_tchCurWin;

int tch_mtrl_ap_win_deinit(void);
void tch_mtrl_win_init(FUN_DIRECTION direction);
void tch_mtrl_content_init(const char* name);
void tch_mtrl_download_init(const char* name);

void tch_mtrl_content_del(void);
void tch_mtrl_download_del(void);
void tch_mtrl_cfg_setting_del(FUN_DIRECTION fun, const char* version);


int tch_mtrl_eng_ap_init(void);
int tch_mtrl_eng_ap_handler(lv_event_code_t msg, int param);
int tch_mtrl_cn_ap_init(void);
int tch_mtrl_cn_ap_handler(lv_event_code_t msg, int param);

void _tch_mtrl_scr_cb(lv_event_t* evt);

uint16_t return_g_subjectSel();

//——————————

#define TCH_MTRL_AP_BG_COLOR_HEX  0xffffff

#endif