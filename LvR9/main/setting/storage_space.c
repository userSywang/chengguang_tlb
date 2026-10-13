//========================================================[]
//Name: storage_space.c
//Function: sd card storage space overview.
//========================================================[]
/********************** 
     INCLUDES
 *********************/
#include <string.h>
#include <math.h>

#include <lvgl/lvgl.h>
 
#include "../utils/global_parm.h"
#include "../utils/misc_utils.h"
#include "./app_setting_base.h"


/*********************
 *      DEFINES
 *********************/

typedef struct _st_sp_set {
	lv_obj_t * cont;
	lv_obj_t * title_label;
	uint64_t total_space;
	uint64_t used_space;

	int32_t used_cyl_w;  //可能空间的宽度
	lv_obj_t * total_label;
	lv_obj_t * used_label;

}storage_space_set_t;


/**********************
 *  STATIC PROTOTYPES
 **********************/


/**********************
 *  STATIC VARIABLES
 **********************/


static storage_space_set_t * sp_set = NULL;
#if DARK_MODE
static lv_style_t dark_style; // 新增暗色模式样式
#endif

/**********************
 *      MACROS
 **********************/
//----------- 主版面 ---------------//

#define SP_TITLE_BG_W		SCREEN_WIDTH
#define SP_TITLE_BG_H		60

#define	SP_TITLE_STR	"存储空间"

#define SP_TITLE_H		30
#define SP_TITLE_Y		20

#define MAIN_BG_W		SCREEN_WIDTH
#define MAIN_BG_H		(SCREEN_HEIGHT - SP_TITLE_BG_H + 40)
#define MAIN_BG_X		0
#define MAIN_BG_Y		42


#define	SP_TOTAL_STR	"总大小:"
#define	SP_USED_STR		"可用空间:"

//#define SPACE_TITLE_W		220
//#define SPACE_TITLE_H		40
#define SPACE_TITLE_X		11
#define SPACE_TITLE_Y		74

#define SP_CYLINDER_W		(220)  //总空间长度
#define SP_CYLINDER_H		(20)
#define SP_CYLINDER_X		(11)
#define SP_CYLINDER_Y		(125)


//====================================================[]
//   extra  FUNCTIONS 
//====================================================[]
int sys_get_sdcard_capacity( uint64_t *total_space , uint64_t *free_space) ;


//====================================================[]
//   fuctions declare
//====================================================[]




//====================================================[]
//   STATIC  FUNCTIONS
//====================================================[]
static const char * TAG = "stgSpc";

static void storage_space_get(void)
{
	uint64_t freeSpace = 0;
	int ret = sys_get_sdcard_capacity(&sp_set->total_space, &freeSpace);
	sp_set->used_space = (int64_t)sp_set->total_space - (int64_t)freeSpace;
	ESP_LOGI(TAG, "%s,[%d] total_space: %lld, used_space: %lld\r\n ", __FUNCTION__,__LINE__, sp_set->total_space, sp_set->used_space);
	return;
}


static void storage_space_init(void)
{

	int i = 0;
	if( sp_set != NULL)	return;
	LV_LOG("%s,[%d] \r\n ", __FUNCTION__,__LINE__);
	
	sp_set = lv_mem_alloc(sizeof(storage_space_set_t));
	memset(sp_set, 0, sizeof(storage_space_set_t));

	storage_space_get();
	return;

}


static void storage_space_setting_cb(lv_event_t* e)
{
   lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* f_obj = lv_event_get_target(e);

	
	LV_LOG("%s [%d] code : %d\n", __FUNCTION__,__LINE__, code);

	if (code == LV_EVENT_KEY)
	{

		char c = *((char*)lv_event_get_param(e));


		LV_LOG("LV_EVENT_KEY: %d\n", c);

		if (c == LV_KEY_ENTER)
		{
			
		}
		else if(c == LV_KEY_ESC)
		{
			LV_LOG("%s [%d]\n", __FUNCTION__,__LINE__);
			lv_indev_keypad_wait_release();
			lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
		}


	}



}

static void _sp_esc_btn_click_cb(lv_event_t * evt)
{
	lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
	return;
}

static void sp_set_main_constructor(void)
{
#if DARK_MODE
	// 初始化暗色模式样式
	lv_style_init(&dark_style);
	lv_style_set_bg_color(&dark_style, lv_color_hex(0x000000)); // 纯黑背景
	lv_style_set_text_color(&dark_style, lv_color_white());      // 白色文本
#endif
	//----------大背景 --------
	LV_LOG("%s [%d]\n", __FUNCTION__,__LINE__);
	sp_set->cont = lv_obj_create(lv_scr_act());

	lv_obj_set_size(sp_set->cont, SCREEN_WIDTH, SCREEN_HEIGHT); 
	lv_obj_align(sp_set->cont, LV_ALIGN_TOP_LEFT, 0, 0);
	
	lv_obj_set_style_pad_all(sp_set->cont,0, 0);
	lv_obj_set_style_border_width(sp_set->cont, 0, 0);

	lv_obj_set_style_radius(sp_set->cont, 0, 0);
	lv_obj_set_style_bg_opa(sp_set->cont, LV_OPA_COVER, 0);
#if DARK_MODE
	lv_obj_add_style(sp_set->cont, &dark_style, 0); // 应用暗色样式
	lv_obj_set_style_bg_color(sp_set->cont, lv_color_hex(0x000000), 0); // 纯黑背景
#else
	lv_obj_set_style_bg_color(sp_set->cont, lv_color_white(), 0); // 原白色
#endif

	lv_obj_clear_flag(sp_set->cont, LV_OBJ_FLAG_SCROLLABLE);

// ----------------
	stng_createEscBtn(sp_set->cont, _sp_esc_btn_click_cb, 0);

// ----- ------------ 标题	----------
	sp_set->title_label = lv_label_create(sp_set->cont);
	lv_obj_set_size(sp_set->title_label, LV_SIZE_CONTENT, SP_TITLE_H);	
	lv_obj_set_style_pad_all(sp_set->title_label,0, 0);
	lv_obj_set_style_border_width(sp_set->title_label, 0, 0);

	lv_label_set_text(sp_set->title_label, SP_TITLE_STR);
#if DARK_MODE
	lv_obj_set_style_text_color(sp_set->title_label, lv_color_white(), 0); // 白色文本
#else
	lv_obj_set_style_text_color(sp_set->title_label, lv_color_black(), 0); // 原黑色
#endif
	lv_obj_align(sp_set->title_label,  LV_ALIGN_TOP_MID, 0, SP_TITLE_Y);

	// ----- -----显示大小	----------
	char space_str[60];

	double t_decimal = (double)sp_set->total_space / 1024 /1024;
	double u_decimal = (double)sp_set->used_space / 1024 /1024;
	int int_total = ceil(t_decimal);
	int usedPct = (int)((u_decimal * 100) / t_decimal);
	
	sp_set->used_cyl_w = SP_CYLINDER_W * u_decimal/t_decimal;	
	
	printf("取整总大小: %d MB,    \n", int_total );
	
	//snprintf(space_str, sizeof(space_str), "已使用: #008cff %.2f GB#/ %.2f GB\n", u_decimal/1024, t_decimal/1024);
	snprintf(space_str, sizeof(space_str), "已使用: #008cff %.2f GB#/ 32 GB\n", u_decimal / 1024);

	ESP_LOGI(TAG, "总大小: %d GB \n已使用: %.2f MB, len:%d\n", int_total, u_decimal, strlen(space_str));

	ESP_LOGI(TAG, "SP_CYLINDER_W: %d,  used_w: %ld\n", SP_CYLINDER_W, sp_set->used_cyl_w);
	
	lv_obj_t * space_label = lv_label_create(sp_set->cont);
	//lv_obj_set_size(space_label, SPACE_TITLE_W, SPACE_TITLE_H);
	lv_obj_set_style_pad_all(space_label,0, 0);
	lv_obj_set_style_border_width(space_label, 0, 0);

	lv_label_set_recolor(space_label, true);
	lv_label_set_text(space_label, space_str);
#if DARK_MODE
	lv_obj_set_style_text_color(space_label, lv_color_white(), 0); // 白色文本
#else
	lv_obj_set_style_text_color(space_label, lv_color_hex(0), 0); // 原黑色
#endif
	lv_obj_set_style_text_line_space(space_label, 10, 0);
	lv_obj_align(space_label,  LV_ALIGN_TOP_LEFT, 20, SPACE_TITLE_Y);
	//------
	lv_obj_t * objBar = lv_bar_create(sp_set->cont);
	lv_bar_set_range(objBar, 0, 100);
	lv_bar_set_value(objBar, usedPct, LV_ANIM_OFF);
	lv_obj_align(objBar, LV_ALIGN_TOP_LEFT, 20, SPACE_TITLE_Y + 50);
	
#if DARK_MODE

	lv_obj_set_style_bg_color(objBar, lv_color_hex(0xE4E4E6), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(objBar, LV_OPA_COVER, LV_PART_MAIN);

	lv_obj_set_style_bg_color(objBar, lv_color_hex(0x008CFF), LV_PART_INDICATOR);
	lv_obj_set_style_bg_opa(objBar, LV_OPA_COVER, LV_PART_INDICATOR);
#endif

//----------------------------------------------------------//

	ESP_LOGI(TAG, "%s [%d]\n", __FUNCTION__,__LINE__);
	return;
}

/**********************
*   GLOBAL FUNCTIONS
**********************/

void setting_storage_space_del(void)
{
	LV_LOG("%s [%d]\n", __FUNCTION__,__LINE__);

	if (sp_set != NULL)
	{
		lv_obj_del(sp_set->cont);
	    lv_mem_free(sp_set);
	    sp_set = NULL;
	}

}

void setting_storage_space_entry(void)
{

	storage_space_init();

	sp_set_main_constructor();

	lv_group_add_obj(lv_group_get_default(), sp_set->cont);  //xzz
	
	lv_obj_add_flag(sp_set->cont, LV_OBJ_FLAG_CLICK_FOCUSABLE);
	
	sys_setCurMainWinAndCb(sp_set->cont, storage_space_setting_cb);
	lv_group_focus_obj(sp_set->cont);
	return;
}
 
 
 
