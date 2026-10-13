#ifndef _PLAYER_MAIN_H
#define _PLAYER_MAIN_H

#include "player.h"

#define VERSION_SIZE 60

/*
// 权限获取数据
typedef struct
{
    int code;
    char token[50];
} nmbox_token_t;

// 激活数据
typedef struct
{
    int code;
} nmbox_active_t;

//教材选择
typedef struct
{
    char subject[30];
    char grade[30];
    char book_versions[60];
    char book_id[30];
    char audio_id[30];
} nmbox_grade_sel_t;

// 获取年级列表数据
typedef struct
{
    int code;
    char authorize_version[30];
    uint8_t book_ver_cnt; // 出版社数量
    char *book_versions;  // 出版社
} nmbox_grade_list_t;

//获取书籍列表数据
typedef struct 
{
    int code;
    uint8_t book_num;
    char term[2][30];
    char book_id[2][30];
}nmbox_book_list_t;
 
typedef struct {
    uint8_t  lang;          // 当前的科目
    uint8_t  depth;         // 章节的深度
    uint8_t  curApLayer;    // 当前的应用所在的位置：  年级列表 | 章节列表 | 单元列表  ： PLAYER_OL_GRADE... 
    uint8_t  curGrade;      // 当前的年级索引号
    uint16_t curChapter;    // 当前的章索引号，从零开始
    uint16_t curSection;    // 当前的节索引号，从零开始
} nmbox_t;

enum {
    PLAYER_OL_NONE = 0,
    PLAYER_OL_GRADE = 1,
    PLAYER_OL_CHAPTER = 2,
    PLAYER_OL_UNIT = 3,
};
*/


#endif
