//========================================================[]
//Name: bright_setting.c
//Function: Setting brightness  
//Author: Xie Zhenzong 
//Date: 2024-04-02
//========================================================[]
/********************** 
     INCLUDES
 *********************/
#include <string.h>
 
#include "bright_setting.h"
//#include "bright_setting_img.h"
#include "../utils/global_parm.h"

#include "../data_parse/img_res.h"

#include "../mainmenu/tlb_main_img.h"
#include "./app_setting_base.h"
#include "../include/img_a_tlb.h"
#ifndef C_PC_SIM
#include "../../trd/tz_sys_lib/include/cvi_tz_sys.h"
#endif


/*********************
 *      DEFINES
 *********************/


#define	BRIGHT_MAX		255		//最大亮度值
#define	BRIGHT_MIN		10



/**********************
 *  STATIC PROTOTYPES
 **********************/


/**********************
 *  STATIC VARIABLES
 **********************/
//static lv_obj_t* set_cont;

static lv_brightness_set_t * br_set = NULL;

const brightness_time_str_t set_time[4]={
	{30,"30秒"},
	{60,"1分钟"},
	{180,"3分钟"},
	{300,"5分钟"}
};

#define SCR_OFF_LST_ITM_N  M_ARRAY_LEN(set_time)


/**********************
 *      MACROS
 **********************/
//----------- 主版面 ---------------//

// 在文件开头添加以下宏定义
#if DARK_MODE
#define DARK_BG_COLOR        0x000000  // 纯黑背景
#define DARK_TEXT_COLOR      lv_color_white() // 白色文字
#define DARK_BUTTON_BG_COLOR 0x212121  // 按钮背景色（深灰）
#else
#define DARK_BG_COLOR        BG_COLOR  // 使用原来的背景色
#define DARK_TEXT_COLOR      lv_color_black() // 黑色文字
#define DARK_BUTTON_BG_COLOR 0xFFFFFF  // 白色按钮背景
#endif

#define BG_COLOR        0xF2F2F2

#define BR_TITLE_BG_W		SCREEN_WIDTH
#define BR_TITLE_BG_H		60

#define	BR_TITLE_STR	"调节亮度"
#define	AUTO_CLOS_LCD_STR	"自动锁屏"

#define BR_TITLE_W		90
#define BR_TITLE_H		30

#define BR_TITLE_Y		15

#define MAIN_BG_W		SCREEN_WIDTH
#define MAIN_BG_H		(SCREEN_HEIGHT - BR_TITLE_BG_H + 40)
#define MAIN_BG_X		0
#define MAIN_BG_Y		42

#define BR_AREA_W		(SCREEN_WIDTH - 24)
#define BR_AREA_H		50
#define BR_AREA_X		(11)
#define BR_AREA_Y		(75)

#define BR_SLIDER_W		(SCREEN_WIDTH * 6 / 10)
#define BR_SLIDER_H		10
#define BR_SLIDER_X		(48)
#define BR_SLIDER_Y		(0)


#define BR_DEC_X		(15)  //亮度减小图标
#define BR_DEC_Y		(0)

#define BR_INC_X		(-10) //亮度加小图标
#define BR_INC_Y		(0)

#define DOWN_FLAG_X		(SCREEN_WIDTH - 50) //向下小图标
#define DOWN_FLAG_Y		(145)


#define BR_STIME_W		(220)
#define BR_STIME_H		50
#define BR_STIME_X		((SCREEN_WIDTH-BR_STIME_W)/2)
#define BR_STIME_Y		(80+47)

#define SHUT_SCR_W		80  //息屏时间
#define SHUT_SCR_H		30
#define SHUT_SCR_X		-40
#define SHUT_SCR_Y		50


#define TIME_LAB_W		60 // 分钟数
#define TIME_LAB_H		30
#define TIME_LAB_X		140
#define TIME_LAB_Y		145


#define MAIN_BG_COLOR	0xF1EFFF  //主背景色
	
#define TITLE_BG_COLOR		0x9E97FF  //主标题背景色
#define SUB_TITLE_BG_COLOR	0xFFFFFF  //次标题背景色
	
#define SEL_COLOR	0x9E97FF  	//矩形 选中框颜色

//-----------------------------
// 自动息屏版面

#define AS_TITLE_W		240
#define AS_TITLE_H		50

#define AS_TITLE_X	11
#define AS_TITLE_Y	15

#define AS_TABLE_W		SCREEN_WIDTH		//设置分钟数表格
#define AS_TABLE_H		(SCREEN_HEIGHT-AS_TITLE_H)
#define AS_TABLE_X		0
#define AS_TABLE_Y		(20)

/*******************************************
 *    fuctions declare
 ******************************************/


void bsp_lcd_set_pwm_duty(int duty);


void setting_brigthness_entry(void);
void setting_brigthness_delete(void);

void savePrjGlobalParm(void);
int16_t setPrjGlobalParm(SYS_PARM_TYPE type, int16_t bufLen, void * buf);



/**********************
 *   extra  FUNCTIONS
 **********************/
static uint16_t get_sys_lcd_bright(void)
{
	LV_LOG_USER("enter get_sys_lcd_bright");
#ifndef C_PC_SIM
	uint16_t bright;
	int ret = tz_get_lcd_brightness(&bright);
	LV_LOG_USER("ret:%d tz_get_lcd_brightness:%d ", ret, bright);

	//uint16_t bright = getPrjGlobalParm(ZB_PARM_SCR_BRIGHTNESS, 0, NULL);
	if( bright>= 0)
	{
		return bright;
	}

#endif
	return 50;
}

//设置系统亮度
static uint16_t set_sys_lcd_bright(void)
{
	LV_LOG_USER("br_set->bright :%d", br_set->bright);
#ifndef C_PC_SIM	
	uint16_t bright = br_set->bright;

	if (bright < BRIGHT_MIN) bright = BRIGHT_MIN;

	LV_LOG_USER("set_sys_lcd_bright:%d ", bright);

	tz_set_lcd_brightness(bright);  // debug

#endif	
	setPrjGlobalParm(ZB_PARM_SCR_BRIGHTNESS, 2, &br_set->bright);
	return 1;
}

// 返回秒数   second
static uint16_t get_sys_shout_scr_time(void)
{
#ifndef C_PC_SIM
	int offSec = 0;
	int ret = tz_get_screenoff_time(&offSec);
	LV_LOG_USER("tz_get_screenoff_time:%d ", ret);
	if (offSec > 0)
	{
		return offSec;
	}
#else
	uint16_t sec = getPrjGlobalParm(ZB_PARM_SCR_AUTO_OFF_TIME, 0, NULL) ; 
	return sec;
#endif
	return 60;//默认一分钟
}

static int set_sys_shout_scr_time(void)
{
	int iFuncRet = -1;
#ifndef C_PC_SIM
	iFuncRet = tz_set_screenoff_time(br_set->time_sec);
#else
	iFuncRet = setPrjGlobalParm(ZB_PARM_SCR_AUTO_OFF_TIME, 2, &br_set->time_sec);
#endif
	return iFuncRet;
}

/**********************
 *   STATIC  FUNCTIONS
 **********************/
static void table_draw_cb(lv_event_t * e) 
{
	lv_obj_t * obj = lv_event_get_user_data(e);
	lv_obj_draw_part_dsc_t * draw_dec = lv_event_get_draw_part_dsc(e);
	lv_area_t * p_area = draw_dec->draw_area;
	
   LV_LOG("%s [%d] \n", __FUNCTION__,__LINE__);

   if(draw_dec->part == LV_PART_ITEMS)
   	{
//	   draw_dec->label_dsc->align = LV_TEXT_ALIGN_CENTER;
//	   draw_dec->rect_dsc->bg_color = lv_palette_main(LV_PALETTE_RED) ;//lv_color_mix(lv_palette_main(LV_PALETTE_BLUE), draw_dec->rect_dsc->bg_color, LV_OPA_20);
//	   draw_dec->rect_dsc->bg_opa = LV_OPA_COVER;
//   draw_dec->rect_dsc->border_color = lv_palette_main(LV_PALETTE_GREY) ;
//   draw_dec->rect_dsc->border_opa = LV_OPA_50;
   
   	//LV_LOG("part: %05x,type:%d, id: %d: border_color: %x, border_width:%d\r\n", (int)(draw_dec->part), (int)(draw_dec->type), (int)(draw_dec->id),(int)(draw_dec->rect_dsc->border_color),(int)(draw_dec->rect_dsc->border_width));
   }
}
static void auto_shout_lcd_time_win_del(void)
{
	set_sys_shout_scr_time();
	br_set->stime_set_flag = 0;
	lv_obj_del(br_set->as_win);
	br_set->as_win = NULL;
}

static void _stng_autoOff_lst_scr_quit(lv_obj_t * obj) {
	char * str = NULL;

	int16_t row = br_set->m_as_selNo;
	str = lv_table_get_cell_value(obj, row, 0);
	br_set->time_sec =	set_time[row].sec;
	br_set->time_str = str;
	LV_LOG("%s,[%d]: %s, sec: %d \r\n ", __FUNCTION__,__LINE__, str, br_set->time_sec);
	lv_label_set_text(br_set->minu_sec_label, br_set->time_str);

	auto_shout_lcd_time_win_del();
}

static void auto_shout_lcd_time_cb(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* obj = lv_event_get_target(e); 

	if (code == LV_EVENT_CLICKED || code == LV_EVENT_KEY) {

		int16_t row = (int16_t)(intptr_t)lv_obj_get_user_data(obj);


		for (int i = 0; i < SCR_OFF_LST_ITM_N; i++) {
			lv_obj_add_flag(br_set->check_icons[i], LV_OBJ_FLAG_HIDDEN);
		}


		lv_obj_clear_flag(br_set->check_icons[row], LV_OBJ_FLAG_HIDDEN);
		br_set->current_selected_row = row;

		br_set->time_sec = set_time[row].sec;

		//auto_shout_lcd_time_win_del();
	}
	else if (code == LV_EVENT_KEY) {
		int32_t c = *((int32_t*)lv_event_get_param(e));
		if (c == LV_KEY_ESC) {
			auto_shout_lcd_time_win_del();
		}
	}
	 //_stng_autoOff_lst_scr_quit(obj);
}

//自动息屏时间设置
static void auto_shout_lcd_time_win(void)
{
	LV_LOG("%s,[%d] \r\n ", __FUNCTION__,__LINE__);
	if(	br_set->as_win != NULL)		return;
	br_set->stime_set_flag = 1;  
	br_set->as_win = lv_obj_create(br_set->cont);
	lv_obj_set_size(br_set->as_win, SCREEN_WIDTH-24, 220);
	lv_obj_set_style_pad_all(br_set->as_win,0, 0);
	lv_obj_set_style_border_width(br_set->as_win, 0, 0);
	lv_obj_set_style_radius(br_set->as_win, 12, 0);
	lv_obj_set_style_bg_color(br_set->as_win, lv_color_hex(DARK_BUTTON_BG_COLOR), 0);
	lv_obj_set_style_bg_opa(br_set->as_win, LV_OPA_COVER, 0);
	
	lv_obj_align(br_set->as_win , LV_ALIGN_CENTER, 0, 60);
	lv_obj_clear_flag(br_set->as_win, LV_OBJ_FLAG_SCROLLABLE);
	
	br_set->as_table = lv_obj_create(br_set->as_win);
	lv_obj_set_flex_flow(br_set->as_table ,LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_top(br_set->as_table,10,0);
	lv_obj_set_style_pad_bottom(br_set->as_table, 10, 0);

	lv_obj_set_style_pad_row(br_set->as_table, 20, 0);  
	lv_obj_set_style_pad_column(br_set->as_table, 0, 0);
	
    lv_obj_set_size(br_set->as_table, AS_TABLE_W, AS_TABLE_H);
	lv_obj_set_style_pad_all(br_set->as_table,0, 0);
	lv_obj_set_style_border_width(br_set->as_table, 0, 0);
	lv_obj_set_style_radius(br_set->as_table, 0, 0);
	lv_obj_set_style_outline_width(br_set->as_table, 0, 0);
	lv_obj_set_style_outline_width(br_set->as_table, 0, LV_STATE_FOCUS_KEY);  //选中时不显示边框状态

	lv_obj_align(br_set->as_table, LV_ALIGN_TOP_LEFT, 0, AS_TABLE_Y);


	lv_obj_clear_flag(br_set->as_table, LV_OBJ_FLAG_SCROLLABLE);
	

	for (int16_t itemNo = 0; itemNo < SCR_OFF_LST_ITM_N; itemNo++) {
		lv_obj_t* objItem = lv_btn_create(br_set->as_table);
		//lv_obj_remove_style_all(objItem);
		lv_obj_set_size(objItem, SCREEN_WIDTH - 30, 32);
		lv_obj_set_style_bg_color(objItem, lv_color_white(), 0);
		lv_color_t colorPrs = lv_color_hex(0xf0f0f0);
		lv_obj_set_style_bg_color(objItem, colorPrs, LV_STATE_PRESSED);
		lv_color_t colorFocus = lv_color_hex(0xfdf5e6);
		lv_obj_set_style_bg_color(objItem, colorFocus, LV_STATE_FOCUS_KEY);
		lv_obj_set_style_border_width(objItem, 0, 0);
		lv_obj_set_style_outline_width(objItem, 0, 0);
		lv_obj_set_style_shadow_width(objItem, 0, 0);

		lv_obj_t* lblItem = lv_label_create(objItem);
		lv_label_set_text(lblItem, set_time[itemNo].str);
		lv_obj_set_style_text_color(lblItem, lv_color_black(), 0);
		//lv_obj_set_style_pad_top(lblItem, 5, 0);
		lv_obj_set_style_border_opa(lblItem, LV_OPA_TRANSP, 0);
		lv_obj_set_style_border_opa(lblItem, LV_OPA_COVER, LV_STATE_FOCUS_KEY);
		lv_obj_set_style_border_side(lblItem, LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_RIGHT, 0);
		lv_obj_set_style_border_width(lblItem, 10, 0);
		lv_obj_align(lblItem, LV_ALIGN_LEFT_MID, 20, 0);

		br_set->check_icons[itemNo] = lv_img_create(objItem);
		lv_img_dsc_t* imgCheck_sec = zb_img_res_load_one(NULL, SETTING_IMG_CHECK, IMG_FREE_TYPE_AUTO, NULL);
		lv_img_set_src(br_set->check_icons[itemNo], imgCheck_sec);
		lv_obj_align(br_set->check_icons[itemNo], LV_ALIGN_RIGHT_MID, -20, 0);
		lv_obj_add_flag(br_set->check_icons[itemNo], LV_OBJ_FLAG_HIDDEN);

		lv_obj_add_event_cb(objItem, auto_shout_lcd_time_cb, LV_EVENT_CLICKED, NULL);
		lv_obj_add_event_cb(objItem, auto_shout_lcd_time_cb, LV_EVENT_KEY, NULL);

		lv_obj_set_user_data(objItem, (void*)(intptr_t)itemNo);
	}

		
	lv_obj_add_flag(br_set->as_table, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_flag(br_set->as_table, LV_OBJ_FLAG_CLICK_FOCUSABLE);

	
//    lv_obj_add_event_cb(br_set->as_table, table_draw_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);	
    lv_obj_add_event_cb(br_set->as_table, auto_shout_lcd_time_cb, LV_EVENT_KEY, NULL);	
    lv_obj_add_event_cb(br_set->as_table, auto_shout_lcd_time_cb, LV_EVENT_CLICKED, NULL);	
    lv_obj_add_event_cb(br_set->as_table, auto_shout_lcd_time_cb, LV_EVENT_VALUE_CHANGED, NULL);	
	lv_group_focus_obj(br_set->as_table);
	

}


static void brightness_init(void)
{

	int i = 0;
	if( br_set != NULL)	return;
	LV_LOG("%s,[%d] \r\n ", __FUNCTION__,__LINE__);
	
	br_set = lv_mem_alloc(sizeof(lv_brightness_set_t));
	memset(br_set, 0, sizeof(lv_brightness_set_t));
	
	br_set->bright = get_sys_lcd_bright();
	LV_LOG_USER("get_sys_lcd_bright br_set->bright :%d", br_set->bright);
	
	//br_set->time_sec = get_sys_shout_scr_time();
	
	//int n = M_ARRAY_LEN(set_time); //sizeof(set_time/set_time[0])
	//for(i = 0; i< M_ARRAY_LEN(set_time); i++)
	//{
	//	if(set_time[i].sec == br_set->time_sec)
	//	{
	//		br_set->time_str = set_time[i].str;
	//		br_set->time_index = i;
	//		break;
	//	}
	//}
	//br_set->time_str = set_time[i].str;
	//LV_LOG("%s,[%d] bright :%d, time: %s \r\n ",  __FUNCTION__,__LINE__, br_set->bright, br_set->time_str);
	//
	return;

}


static void _stng_esc_btn_click_cb(lv_event_t * evt)
{
	//lv_obj_t* obj = lv_event_get_target(evt);
	lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
	return;
}


static void bright_draw_main(lv_event_t* e)
{
	lv_obj_t * obj = lv_event_get_target(e);
	lv_draw_ctx_t * draw_ctx = lv_event_get_draw_ctx(e);

	LV_LOG("%s,[%d] \r\n ", __FUNCTION__,__LINE__);

	//------标题背景------//
	lv_area_t rect_area ; 
	/*	
	rect_area.x1 = 0;
	rect_area.y1 = 0;
	rect_area.x2 = BR_TITLE_BG_W - 1;
	rect_area.y2 = BR_TITLE_BG_H - 1;
	
	lv_draw_rect_dsc_t title_rect;
	lv_draw_rect_dsc_init(&title_rect);
	 lv_obj_init_draw_rect_dsc(obj, LV_PART_ITEMS, &title_rect);
	title_rect.bg_opa = LV_OPA_COVER;
	title_rect.bg_color = lv_color_hex(TITLE_BG_COLOR);

	lv_draw_rect(draw_ctx, &title_rect, &rect_area);
*/
	//------设置主背景------//
/*
	rect_area.x1 = MAIN_BG_X;
	rect_area.y1 = MAIN_BG_Y;
	rect_area.x2 = rect_area.x1 + MAIN_BG_W - 1;
	rect_area.y2 = rect_area.y1 + MAIN_BG_H - 1;

	lv_draw_rect_dsc_t main_rect;
	
	lv_draw_rect_dsc_init(&main_rect);
	
	main_rect.radius = 20;
	main_rect.bg_opa = LV_OPA_COVER;
	main_rect.bg_color = lv_color_hex(MAIN_BG_COLOR);
		
	lv_draw_rect(draw_ctx, &main_rect, &rect_area);
*/	
	//------设置亮度条矩形------//

	br_set->br_rect_area.x1 = BR_AREA_X;
	br_set->br_rect_area.y1 = BR_AREA_Y;
	br_set->br_rect_area.x2 = br_set->br_rect_area.x1 + BR_AREA_W - 1;
	br_set->br_rect_area.y2 = br_set->br_rect_area.y1 + BR_AREA_H - 1;

	lv_draw_rect_dsc_init(&br_set->br_rect);
	
	br_set->br_rect.radius = 12;
	br_set->br_rect.bg_opa = LV_OPA_COVER;
	br_set->br_rect.bg_color = lv_color_hex(0xFFFFFF);

	if(br_set->sel == 0)
	{
		br_set->br_rect.outline_width = 3;	
		br_set->br_rect.outline_color = lv_color_hex(SEL_COLOR); //选中框颜色
		br_set->br_rect.outline_opa = LV_OPA_50;
	}	
	
	lv_draw_rect(draw_ctx, &br_set->br_rect, &br_set->br_rect_area);


	//------设置关屏时间矩形------//

	br_set->stime_rect_area.x1 = BR_STIME_X;
	br_set->stime_rect_area.y1 = BR_STIME_Y;
	br_set->stime_rect_area.x2 = br_set->stime_rect_area.x1 + BR_STIME_W - 1;
	br_set->stime_rect_area.y2 = br_set->stime_rect_area.y1 + BR_STIME_H - 1;

	
	lv_draw_rect_dsc_init(&br_set->stime_rect);
	
	br_set->stime_rect.radius = 20;
	br_set->stime_rect.bg_opa = LV_OPA_COVER;
	br_set->stime_rect.bg_color = lv_color_hex(0xFFFFFF);
	
	if(br_set->sel == 1)
	{
		br_set->stime_rect.outline_width = 3;	
		br_set->stime_rect.outline_color = lv_color_hex(SEL_COLOR); //选中框颜色
		br_set->stime_rect.outline_opa = LV_OPA_50;
	}	
		
	lv_draw_rect(draw_ctx, &br_set->stime_rect, &br_set->stime_rect_area);

//	LV_LOG("%s [%d] \n", __FUNCTION__,__LINE__);
	return;
}


static void show_bright_slider(int value)
{
	LV_LOG("FUNCTION:%s,[%d]", __FUNCTION__,__LINE__);
	lv_slider_set_value(br_set->slider, value, LV_ANIM_OFF);
	set_sys_lcd_bright(); // 设置 lcd 亮度

}


static void bright_setting_cb(lv_event_t* e)
{
   lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* f_obj = lv_event_get_target(e);

	static uint8_t sel_item = 0;		// 0 --设置亮度，1--设置关屏时间
	
	static uint8_t set_time_flag = 0; //设置面板是否激活
	
	LV_LOG("%s [%d] code : %d\n", __FUNCTION__,__LINE__, code);

	if (code == LV_EVENT_CLICKED) 
	{
		LV_LOG_USER("Clicked\n");

		lv_obj_add_state(f_obj, LV_STATE_FOCUS_KEY);  //加此状态后才会有选中框效果

		lv_group_focus_obj(f_obj);

		LV_LOG_USER("lv_group_focus_obj : %p\n", f_obj);

	 
	}
	else if (code == LV_EVENT_VALUE_CHANGED) 
	{
		LV_LOG_USER("Toggled\n");
	}
#if 1
	else if (code == LV_EVENT_KEY)
	{

		char c = *((char*)lv_event_get_param(e));


		LV_LOG("LV_EVENT_KEY: %d\n", c);
/*		if (c == LV_KEY_RIGHT)
		{
			if((br_set->sel == 0) && (br_set->stime_set_flag == 0))
			{
				br_set->bright = lv_slider_get_value(br_set->slider);
				if (br_set->bright < BRIGHT_MAX)
				{
					br_set->bright += 10;
				}
				if (br_set->bright > BRIGHT_MAX)
				{
					br_set->bright = BRIGHT_MAX;
				}
				show_bright_slider(br_set->bright);
			}
			else if(br_set->sel == 1)
			{
				if(br_set->time_index < M_ARRAY_LEN(set_time)-1)						
				{
					br_set->time_index += 1;
					br_set->time_str = set_time[br_set->time_index].str;					
					br_set->time_sec =	set_time[br_set->time_index].sec;					
					lv_label_set_text(br_set->minu_sec_label, br_set->time_str);
					lv_obj_invalidate(br_set->cont);					
					set_sys_shout_scr_time();
				}
				
			}
			
		}
		else if (c == LV_KEY_LEFT)
		{
			if((br_set->sel == 0) && (br_set->stime_set_flag == 0))
			{
				br_set->bright = lv_slider_get_value(br_set->slider);
				if(br_set->bright > BRIGHT_MIN + 10)
				{
					br_set->bright -= 10;
				}
				else {
					br_set->bright = BRIGHT_MIN;
				}
				show_bright_slider(br_set->bright);
			}
			else if(br_set->sel == 1)
			{
				if(br_set->time_index > 0)						
				{
					br_set->time_index -= 1;
					br_set->time_str = set_time[br_set->time_index].str;
					br_set->time_sec =	set_time[br_set->time_index].sec;
					lv_label_set_text(br_set->minu_sec_label, br_set->time_str);
					lv_obj_invalidate(br_set->cont);					
					set_sys_shout_scr_time();
				}
				
			}
		}
		if (c == LV_KEY_DOWN)
		{
			br_set->sel ^= 1; 
			lv_obj_invalidate(br_set->cont);
				

		}
		else if (c == LV_KEY_UP)
		{
			br_set->sel ^= 1; 
			lv_obj_invalidate(br_set->cont);
		}	*/	
/*		if (c == LV_KEY_ENTER)
		{
			if(br_set->sel == 1)  auto_shout_lcd_time_win();
		}
		else*/ 
		if(c == LV_KEY_ESC)
		{
			LV_LOG("%s [%d]\n", __FUNCTION__,__LINE__);
			lv_indev_keypad_wait_release();
			lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
		}


	}
#endif


}


static void _stng_scrOffArea_click_cb(lv_event_t * evt)
{
	//lv_obj_t* obj = lv_event_get_target(evt);
	auto_shout_lcd_time_win();
	return;
}


static void _bright_cfg_slider_event_cb(lv_event_t * e)
{
	lv_obj_t * slider = lv_event_get_target(e);

	br_set->bright = lv_slider_get_value(slider);
	LV_LOG_USER("br_set->bright :%d", br_set->bright);
	set_sys_lcd_bright();
}

static void bright_set_main_constructor(void)
{
	//----------大背景 --------
		br_set->cont = lv_obj_create(lv_scr_act());
	
		lv_obj_set_size(br_set->cont, SCREEN_WIDTH, SCREEN_HEIGHT); 
		lv_obj_align(br_set->cont, LV_ALIGN_TOP_LEFT, 0, 0);
		
		lv_obj_set_style_pad_all(br_set->cont,0, 0);
		lv_obj_set_style_border_width(br_set->cont, 0, 0);

		lv_obj_set_style_radius(br_set->cont, 0, 0);
		lv_obj_clear_flag(br_set->cont, LV_OBJ_FLAG_SCROLLABLE);
		lv_obj_set_style_bg_color(br_set->cont, lv_color_hex(DARK_BG_COLOR), 0);
		
		//lv_obj_add_event_cb(br_set->cont, bright_draw_main, LV_EVENT_DRAW_MAIN, br_set->cont);
	
	// ----- ------------ 亮度标题	----------
		br_set->title_label = lv_label_create(br_set->cont);
		lv_obj_set_size(br_set->title_label, BR_TITLE_W, BR_TITLE_H);	
		lv_obj_set_style_pad_all(br_set->title_label,0, 0);
		lv_obj_set_style_border_width(br_set->title_label, 0, 0);
		lv_obj_set_style_text_color(br_set->title_label, DARK_TEXT_COLOR, 0);
		lv_label_set_text(br_set->title_label, BR_TITLE_STR);
		lv_obj_align(br_set->title_label,  LV_ALIGN_TOP_MID, 0, BR_TITLE_Y);

		stng_createEscBtn(br_set->cont, _stng_esc_btn_click_cb, 0);
	
	//------ 亮度设置区 --------
	
		lv_obj_t* list = lv_obj_create(br_set->cont);
		lv_obj_set_size(list, (SCREEN_WIDTH - 24), 50);
		lv_obj_align(list, LV_ALIGN_TOP_LEFT, 11, 120);

		lv_obj_set_style_bg_color(list, lv_color_hex(DARK_BUTTON_BG_COLOR), 0);
		lv_obj_set_style_bg_opa(list, LV_OPA_COVER, 0);
		lv_obj_set_style_pad_all(list, 0, 0);
		lv_obj_set_style_border_width(list, 0, 0);
		lv_obj_set_style_radius(list, 12, 0);

		lv_obj_set_style_outline_width(list, 0, 0);
		lv_obj_set_style_outline_width(list, 0, LV_STATE_CHECKED);
		lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLLABLE);
		lv_obj_align_to(list, br_set->title_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);


	
		lv_obj_t * img1  = lv_img_create(list);		  
		lv_img_dsc_t * pImgDsc = &g_XXX_img_dsc_tbl[MMN_light_dec - 1];
		lv_img_set_src(img1, pImgDsc); 
		lv_obj_align_to(img1, list, LV_ALIGN_LEFT_MID, BR_DEC_X, BR_DEC_Y);
		
		lv_obj_t * img2 = lv_img_create(list);
		pImgDsc = &g_XXX_img_dsc_tbl[MMN_light_inc - 1];
		lv_img_set_src(img2, pImgDsc); 		
		lv_obj_align_to(img2, list, LV_ALIGN_RIGHT_MID, BR_INC_X, BR_INC_Y);
		 

		br_set->slider = lv_slider_create(list);  //亮度进度条		
		lv_obj_set_size(br_set->slider, BR_SLIDER_W, BR_SLIDER_H);		 
		lv_obj_align_to(br_set->slider, list, LV_ALIGN_CENTER, 0, BR_SLIDER_Y);
		lv_slider_set_range(br_set->slider, BRIGHT_MIN, BRIGHT_MAX);

		lv_slider_set_value(br_set->slider, br_set->bright, LV_ANIM_OFF);

		lv_obj_add_event_cb(br_set->slider, _bright_cfg_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	
#if DARK_MODE
		// 设置滑动条背景色
		lv_obj_set_style_bg_color(br_set->slider, lv_color_hex(0x424242), LV_PART_MAIN);
		lv_obj_set_style_bg_opa(br_set->slider, LV_OPA_COVER, LV_PART_MAIN);
		//// 设置滑动条指示器颜色
		//lv_obj_set_style_bg_color(br_set->slider, lv_color_hex(0x9E97FF), LV_PART_INDICATOR);
		//lv_obj_set_style_bg_opa(br_set->slider, LV_OPA_COVER, LV_PART_INDICATOR);
		// 设置滑动条旋钮颜色
		lv_obj_set_style_bg_color(br_set->slider, lv_color_white(), LV_PART_KNOB);
		lv_obj_set_style_bg_opa(br_set->slider, LV_OPA_COVER, LV_PART_KNOB);
#endif
		//lv_obj_add_event_cb(objScrOffTmBg, _stng_scrOffArea_click_cb, LV_EVENT_CLICKED, NULL);
	
//----------------------------------------------------------//

		
		lv_group_focus_obj(br_set->cont);
	LV_LOG("%s [%d]\n", __FUNCTION__,__LINE__);

	return;
}

/**********************
*   GLOBAL FUNCTIONS
**********************/

void setting_brigthness_del(void)
{
	LV_LOG("%s [%d]\n", __FUNCTION__,__LINE__);

	if (br_set != NULL)
	{
		lv_obj_del(br_set->cont);
	    lv_mem_free(br_set);
	    br_set = NULL;
//#ifndef	C_PC_SIM	
//		savePrjGlobalParm();
//#endif
	}

}

void setting_brigthness_entry(void)
{

	brightness_init();

	bright_set_main_constructor();
	show_bright_slider(br_set->bright);
//----------------------------------------------------------//

	lv_group_add_obj(lv_group_get_default(), br_set->cont);  //xzz
	
	lv_obj_add_flag(br_set->cont, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_flag(br_set->cont, LV_OBJ_FLAG_CLICK_FOCUSABLE);
	
	//lv_obj_add_event_cb(br_set->cont, bright_setting_cb, LV_EVENT_KEY, NULL);
	sys_setCurMainWinAndCb(br_set->cont, bright_setting_cb);
	
	lv_group_focus_obj(br_set->cont);

	return;
	
	
}
 

 
