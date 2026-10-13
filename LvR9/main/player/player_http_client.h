#ifndef PLAYER_HTTP_CLIENT_H
#define PLAYER_HTTP_CLIENT_H

#include <string.h>
#include <stdlib.h>

#include "../data_parse/cJSON.h"
#include "../utils/misc_utils.h"

typedef enum {
    REQUEST_NONE,           // 未使用
    REQUEST_TOKEN,          // 1.	获取token鉴权   ///authUserInfo
    REQUEST_DEV_ACTIVATE,   // 设备激活     ///api/sdk/audio/dev_activate
    REQUEST_BOOK_VER,       // 获取音频版本年级列表   ///api/sdk/audio/query_book_version
    REQUEST_BOOK_VER_ENG,       // 
    REQUEST_BOOK_LIST,      // 根据年级、科目、版本获取书本列表 ///api/sdk/audio/query_aduio_booklist
    REQUEST_BOOK_CATALOGUE, // 课本音频目录及内容接口  ///api/sdk/audio/get_book_catalogue
    REQUEST_BOOK_AUDIO,     // 获取书本音频版本接口  ///api/sdk/audio/query_bookaudio_ver

    REQUEST_UNIT_JSON,      // 获取json数据：https://d.namibox.com/3rd-sdk/202406190705/tape1a_002002/6537442fe900b1d935d4d4c31a767fd3.json
    REQUEST_EN_WORD_AUD_URL,   // 获取单词语音 url
    REQUEST_BOOK_READY,      // 后台准备 book 的数据
    REQUEST_BOOK_DOWN,       // 根据后台返回的链接，下载 book 数据
    REQUEST_OBTAIN_DEV_SN,   // 获取设备序列号 
    REQUEST_LOCK_SN,         // 请求锁定设备序列号
} player_rq_enum;



#define  NET_RSLT_QUERY_TOKEN     10001
#define  NET_RSLT_DEV_ACTIVATE    10002
#define  NET_RSLT_QUERY_BOOK_CHN_VER  1003
#define  NET_RSLT_QUERY_BOOK_ENG_VER  1004
#define  NET_RSLT_WORD_AUD        10041
#define  NET_RSLT_OBTAIN_DEV_SN   10042
#define  NET_RSLT_LOCK_DEV_SN     10043


#define  NET_RSLT_BOOK_LIST       20001
#define  NET_RSLT_BOOK_READY      20002
#define  NET_RSLT_BOOK_DOWN       20003




void play_send_post_request(char *jstr,char *url);


extern int play_http_post_create(int rq_num);

/** 请求终止 player 的 http 请求 
 */
extern void play_http_request_stop(void);


extern char g_token[];
extern char* local_response_buffer;
extern char * g_plyHttpRsltBuf;


extern volatile int8_t  g_activateRet; 

#define NET_ACTIVATE_RET_FAIL  2
#define NET_ACTIVATE_RET_TOKEN_OUT_OF_DATE  3



#ifndef C_PC_SIM
#define IS_JC_TOKEN_READY()  (strlen(g_token) == 32)
#else
#define IS_JC_TOKEN_READY()  0
#endif

#endif

