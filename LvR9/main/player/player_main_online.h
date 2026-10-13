#ifndef _PLAYER_MAIN_ONLINE_H
#define _PLAYER_MAIN_ONLINE_H

#include "player_http_client.h"

#define VERSION_SIZE 60
#define MAX_BOOK_GRADE_NUM  4

#define CANCEL_BIT_READY    (0x1)
#define CANCEL_BIT_DOWN     (0x2)
#define CANCEL_BIT_SPLIT    (0x4)

#define play_main_online_page_init_has 0
#define read_grd_parse_has 0

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
    char* book_versions;  // 出版社
} nmbox_grade_list_t;

//获取书籍列表数据
typedef struct
{
    int code;
    uint8_t book_num;   // 书的数量
    char version[80];   // 版本
    char down[MAX_BOOK_GRADE_NUM];       // 是否下载
    char waiyan[MAX_BOOK_GRADE_NUM];     // 是否是外研版数据:
    char term[MAX_BOOK_GRADE_NUM][80];   // 书名
    char book_id[MAX_BOOK_GRADE_NUM][30];// 书id
}nmbox_book_list_t;


// 书籍的章节数据
// 定义一个节的结构
typedef struct
{
    //"title": "Unit 1 School",
    //    "track_url" : "https://d.namibox.com/3rd-sdk/202406190705/tape1a_002002/7f514b57bb18c67eb5d3ed23b7b6e2ad.json",
    //    "audio_id" : "GQ5V0YWNLQGYMZJ3"
    float audioSize; // 声音长度，单位秒
    char  audioId[30];  // 声音的id
    char* pTitle;       // 章节 or 课文的标题
    char* pTrackUrl;    // 课文的 json 连接 
} nmbox_section_t;
// 定义一个章的结构
typedef struct {
    int number;             // 章的编号
    char* title;            // 章的标题
    int num_sections;       // 章中节的数量
    nmbox_section_t* sections;    // 指向节结构数组的指针
} nmbox_chapter_t;

enum {
    PLAYER_OL_NONE      = 0,
    PLAYER_OL_GRADE     = 1,
    PLAYER_OL_CHAPTER   = 2,
    PLAYER_OL_UNIT      = 3,
};

enum {
    STATUS_NONE = 0,
    STATUS_DOWN_READY = 1,
    STATUS_DOWN_BOOK,
    STATUS_DOWN_SPLIT,
    STATUS_FINISH,          // 完成
    STATUS_ERROR_SPLIT,     // 用户退出
    STATUS_ERROR_NOT_FND,   // 下载的book.bin 文件不存在
    STATUS_ERROR_CAPACITY,  // 创建文件失败，空间不足
};

typedef struct {
    uint8_t  downStatus;    // 下载资源的状态：0：空闲，1：后台正在准备资源；2：正在下载book；3： 下载完成
    //uint8_t  splitTotal;    // 总共的拆分数量
    //uint8_t  splitCurrent;  // 当前拆分的第几个
    uint8_t  lang;          // 当前的科目 // ENGLISH, 
    uint8_t  depth;         // 章节的深度
    uint8_t  curApLayer;    // 当前的应用所在的位置：  年级列表 | 章节列表 | 单元列表  ： PLAYER_OL_GRADE... 
    uint8_t  curGrade;      // 当前的年级索引号
    uint8_t  curTerm;       // 当前的学期索引号
    uint8_t  downTerm;      // 当前正在下载的学期索引号
    uint16_t curChapter;    // 当前的章索引号，从零开始
    uint16_t curSection;    // 当前的节索引号，从零开始
} nmbox_t;

typedef struct {
	char m_remoteUrl[350];
} plyOnlnApInf_t;


extern int play_main_online_page_init(int classNum, char direction);

extern int parse_book_list_jstr(char* jstr, int16_t subject, char* publisher, nmbox_book_list_t* nmbox_book_list);

extern int read_book_version(const char * ini_path, char * book_verison, int bufLen);


#endif

