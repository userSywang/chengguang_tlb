#ifndef BOOK_SELECT_H
#define BOOK_SELECT_H

#include <stdio.h>
#include "../utils/misc_utils.h"
#include <string.h>
#include <lvgl/lvgl.h>
#include <sys/stat.h>
#include "../mainmenu/tlb_main_img.h"

#define MAX_LEN_BOOK_NAME 100
#define MAX_VERSION_NUM    50

typedef struct book_info {
	lv_img_dsc_t* pimg_dsc;
	char grade[20];
	char name[MAX_LEN_BOOK_NAME];
	char cover_url[MAX_LEN_BOOK_NAME];
	char book_url[MAX_LEN_BOOK_NAME];
}book_info_t;

typedef struct {
	book_info_t data;
	struct bk_node* next;
} bk_node;

extern bk_node* g_bkHead;
extern book_info_t* gp_cur_book;

//extern char** g_pversion;
extern char* get_version_by_id(char pos);
//extern char* get_book_name_by_bookbin(const char* book_url);
extern char* get_cur_book_name();
extern lv_img_dsc_t* get_cur_book_img_dsc();
extern int read_versions(char* subject, char* stage2grade);
extern int read_books(char* subject, char* stage2grade,char *version);
extern book_info_t* get_booknode_by_bookbin(const char* book_url);
extern void book_node_free(void);
extern uint16_t get_version_sum();

#endif