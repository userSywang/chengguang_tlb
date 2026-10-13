#ifndef _PLAYER_DATA_PARSE_H
#define _PLAYER_DATA_PARSE_H

#include <stdio.h>
#include <string.h>
#include "../utils/misc_utils.h"
//#include "../../components/lvgl/src/misc/lv_assert.h"
#include "../data_parse/analyze_listening.h"
#include "../data_parse/aud_lyric.h"

/**
 * @brief 目录信息(存储一本书（或一个专辑）的信息，包括多个单元（歌曲）的信息)
 */
typedef struct
{
    uint16_t cnt;     // 总文章数
    uint16_t playNum; // 当前要播放的文章
    char **pFileName; // 文件名
    char **pTile;     // 文章标题
    struct LSNSPK_book *pBookInf;
} player_book_info_t;


/**
 * @brief 当前播放单元(音频)的信息
 */
typedef struct
{
    uint16_t cnt;        // 句子总数
    int32_t audLen;      // 音频长度
    int32_t audOffset;   // 音频偏移位置
    FILE* fp;            // 播放文件指针
    int16_t encParm;     // 播放参数
    int32_t* ptInTm;     // 每句开始时间
    char** pOrginal;     // 原文文本
    char** pTranslation; // 翻译文本
    uint16_t load_s_no;  // 加载句子的开始序号, 有些单元数据太多，不一次性加载完。
    uint16_t load_e_no;  // 加载句子的结束序号
    
    bool  doesUseExtraSentence;  // 课文句子的 txtBuff 是否独立申请 
    struct LSNSPK_unit *pUnitInf;

    bool need_free_text;  // 标记文本是否需要单独释放
} player_unit_info_t;  


int lrc_parse_file(const char* path, player_unit_info_t* pst);
void lrc_unit_free(player_unit_info_t* pst);
int player_get_book_inf(char *path, player_book_info_t *pbit);
void free_player_book_info_t(player_book_info_t *pmit);
int player_get_unit_info(char *path, player_unit_info_t *pst);
void free_player_unit_info_t(player_unit_info_t *pst);

int update_song_info_in_book_info(player_book_info_t* pbook, int index, char* book_path);

#endif
