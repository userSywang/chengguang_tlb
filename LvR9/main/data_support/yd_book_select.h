#ifndef YD_BOOK_SELECT_H
#define YD_BOOK_SELECT_H
#ifdef __cplusplus
extern "C" {
#endif
//#include <stdio.h>
#include "../player/player_data_parse.h"
#include "../data_parse/analyze_listening.h"
#include "res_type.h"
#include <string.h>
#include <lvgl/lvgl.h>
#include <sys/stat.h>
#include "../data_support/db_manager.h"

#define MAX_LEN_BOOK_NAME 100
#define MAX_VERSION_NUM    60

#define RES_NET_BOOKS_PATH "http://speed2.bolledu.net/BOLLTVVideo/upload/data/chenguang-tlb/download/books"
#ifndef C_PC_SIM
#define YD_BOOK_PATH "oem"
#else
#define YD_BOOK_PATH "userdata"
#endif

/*
/oem/_data/textbook/box
/oem/_data/textbook/boll
/oem/_data/textbook/cover

/userdata/_data/textbook/,        // 在线: box  // boll
/userdata/_data/yd/",              // 有道在线资源

*/

//typedef enum {
//	RES_NONE,
//	RES_CG_BUILDIN,		// 内置
//	RES_CG_ONLINE,		// 在线
//	RES_YD_ONLINE,		// 有道在线资源
//	RES_YD_BUILDIN,		// 有道内置资源
//	RES_BAIDU_CLOUD,	// 百度网盘
//} e_res_store;


// 博尔资源定义
typedef enum {
	BOOK_ENG,			//英语教材
	BOOK_CHN,			//语文教材
	BOOK_SONG,			//英文儿歌
	BOOK_SITU_DIALOG,	//情景对话 Situational Dialogues
	BOOK_CHINA_CULT,	//国学熏听 Chinese Culture Listening
	BOOK_ANCI_POETRY,	//古诗随听 Listen to ancient poetry with ease
	BOOK_ERR,
} e_book_genre;

typedef enum {
	STAGE_PUPIL,		//小学
	STAGE_JUNIOR,		//初中
	STAGE_SENIOR,		//高中
} e_stage;

typedef struct yd_book_info {
	//lv_img_dsc_t* pimg_dsc;
	char grade[20];
	char name[MAX_LEN_BOOK_NAME];
	char cover_url[MAX_LEN_BOOK_NAME];
	char book_url[MAX_LEN_BOOK_NAME];
}yd_book_info_t;

typedef struct yd_bk_node{
	yd_book_info_t data;
	struct yd_bk_node* next;
} yd_bk_node;


typedef struct {
	uint16_t file_cnt;
	uint16_t unit_sum;
	uint32_t header_size;
	uint32_t file_size;
	uint32_t pos;		// 记录文件指针位置
	uint32_t* addr_lst;
} yd_split_book_t;

extern const char* gc_res_store_path[];

//extern yd_bk_node* g_yd_bkHead;
/*
* 根据科目、学段，获取当前所有的版本
* subject：博尔资源类别
* stage： 学段
* version_num： 存放版本的数量
* 返回： 指针数组，版本的列表
*/
extern const char** yd_read_versions(e_book_genre subject, e_stage stage, uint16_t* version_num);
//释放版本数组， 退出模块前调用
extern void yd_book_versions_free();

/*
* 根据科目、学段、版本，获取当前所有的书
* subject：博尔资源类别
* stage： 学段
* version_name： 版本名字
* version_num： 存放版本的数量
* 返回： 指针链表，是书的列表
*/
extern yd_bk_node* yd_read_books(e_book_genre subject, e_stage stage, const char* version_name, uint16_t* book_num);
//释放书的资源， 退出模块前调用
extern void yd_book_node_free(void);

/*
* 检查资源是否已下载，已下载返回 true 否则返回false，同时把待下载的或已下载的本地资源路径（local_path）返回
* res_youdao: true:是有道资源，否则 晨光
* url: 有道资源url 或者教材路径
* local_path： 返回本地路径
* book_id: 专辑id
*/
extern bool check_res_down(e_res_store res_genre, const char* url, char** local_path, uint8_t book_id, bool create_dir);
//* 拼接出晨光下载的链接 *//
extern int get_cg_net_url(char* part_path, char** net_url);

extern bool get_cg_album_path(const char* url, char** local_path);

bool get_res_path(e_res_store res_genre, const char* url, char** local_path);

/*
*  拆book.bin 生成album.bin, unit001.bin ........
*  book_path: 原数据
*  dest_path: 目标目录,如果目标目录为NULL，则拆到book_path 同一级目录下
*/
extern int yd_split_cg_book(const char* book_path, const char* dest_path);


/*
*  以下两个接口来源于其它源文件
*/

/*
* book_path: 博尔资源的路径
* player_book_info_t： 存放博尔资源的显示列表，其中信息可以用于定位音频
* from player/player_data_parse.c
*/
//extern int player_get_book_inf(char* book_path, player_book_info_t* pmit);


/*
* pcAlbumPath: 专辑路径
* from data_parase/analyze_listening.c
*/
//extern struct LSNSPK_MENU* LSNSPK_menu_open(const char* pcAlbumPath);

/*
* 下载完成时提供的调用接口，用来保存下载的信息
* title: 显示名
* res_genre: 资源存储类别
* url_path:  请求下载的url链接
*/
//下载完成调用传参数：、类别【网盘，有道，教材】、下载路径
extern int save_down_record(const char* title, e_res_store res_genre, const char* url_path);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif