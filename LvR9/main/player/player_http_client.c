#include "player_http_client.h"
#include "player_main_online.h"

#include "../data_parse/cJSON.h"
#include "../zb_base.h"
#include "../utils/misc_utils.h"
#include "../utils/global_parm.h"

#define APPID ("88f4f2dbdb39241eecd8aa03353141f4")
#define BOOK_API_ACEESS_KEY "bdxW4kYQ5BSUvaWHzChnFpnjjmnGey8vKGf"
#define TOKEN_URL ("http://thirdparty.bolledu.net/authUserInfo")
#define RES_URL ("http://thirdparty.bolledu.net/audio/api")
#define DEV_URL ("http://thirdparty.bolledu.net/api/sdk/audio/dev_activate")
#define BOOK_URL ("http://tinglb.bolledu.net:8080/nanoboxBookRes")

static char gcSerialNo[ZB_PROD_SN_LEN + 1] = { 0 };
#define QSN_MODEL "cgtlb"
#define QSN_ACCESSKEY  "66854f1b110143269dbffdd806fa66eb"
#define ZB_PROD_SN_LEN 16

extern int zb_global_str_read(char* pcBuf, int bufLen);
extern const char* bksl_get_grade_nmbox(int grade);

#define LV_EVENT_READY 31
#define TOKEN_LENGTH    (33)
#define HTTP_POST_LEN (1024)
#define MAX_HTTP_OUTPUT_BUFFER (1024 * 20)
#define PLY_HTTP_RSLT_BUF_LEN  (1024 * 16)

static const char* TAG = "PLAYER_HTTP_CLIENT";
char* local_response_buffer = NULL;
char* g_plyHttpRsltBuf;
static volatile bool g_plyHttpRun;
static int plyHttpReqNo = 0;
char g_devId[ZB_PROD_SN_LEN + 1];
char g_token[TOKEN_LENGTH] = "";
player_rq_enum g_rq = REQUEST_NONE;
volatile int8_t  g_activateRet;

//int zb_readProductSerialNo(char* buf, int bufLen) {
//    return 0; 
//}

char* getProductSN(void) {
    return gcSerialNo; 
}

char* serialize_token_jstr(void) {
    return cJSON_PrintUnformatted(cJSON_CreateObject()); 
}

char* serialize_active_jstr(void) {
    return cJSON_PrintUnformatted(cJSON_CreateObject()); 
}

static char* serialize_grade_list_jstr(int16_t subject) {
    return cJSON_PrintUnformatted(cJSON_CreateObject()); 
}

char* serialize_book_list_jstr(void) {
    return cJSON_PrintUnformatted(cJSON_CreateObject()); 
}

char* serialize_aud_menu_jstr(void) {
    return cJSON_PrintUnformatted(cJSON_CreateObject()); 
}

char* serialize_aud_addr_jstr(void) {
    return cJSON_PrintUnformatted(cJSON_CreateObject()); 
}

char* serialize_book_ready_jstr(void) {
    return cJSON_PrintUnformatted(cJSON_CreateObject()); 
}

char* serialize_my_jstr(void) {
    return cJSON_Print(cJSON_CreateObject()); 
}

char* serialize_word_query_jstr(const char* pcWord) {
    return cJSON_PrintUnformatted(cJSON_CreateObject()); 
}

int serialize_obtain_sn_parm(char* destBuf, int bufLen) {
    return 0; 
}

int serialize_lock_sn_parm(char* destBuf, int bufLen) {
    return 0; 
}

cJSON* get_cjson_value_by_path(char* json, const char* path) {
    return NULL; 
}


#ifndef C_PC_SIM
int _ply_getDeviceMac(bool isForce) { return 0; }
static void play_send_get_request(char* url) {}
void play_send_post_request_ext(char* jstr, char* url, int timeout) {}
void play_send_post_request(char* jstr, char* url) {}
static void play_http_task(void* pvParameters) {}
int play_http_post_create(int rq_num) { return 0; }
#endif


#define nmbox_t_has 0
#if nmbox_t_has
extern nmbox_book_list_t nmbox_book_list;
extern nmbox_t nmbox;
extern char publisher[];
#endif


#define NET_ACTIVATE_RET_FAIL 2
#define NET_RSLT_QUERY_TOKEN 1
#define NET_RSLT_DEV_ACTIVATE 2
#define NET_RSLT_QUERY_BOOK_CHN_VER 3
#define NET_RSLT_QUERY_BOOK_ENG_VER 4
#define NET_RSLT_OBTAIN_DEV_SN 5
#define NET_RSLT_LOCK_DEV_SN 6
#define NET_RSLT_WORD_AUD 7
#define NET_RSLT_BOOK_LIST 8
#define NET_RSLT_BOOK_READY 9


extern bool zb_is_wifi_connect(void);