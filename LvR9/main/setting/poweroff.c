#include <lvgl/lvgl.h>
#include "../utils/misc_utils.h"
#include "../include/img_a_tlb.h"
#include "../data_parse/img_res.h"
#include "../mainmenu/tlb_main_img.h"
#include "../utils/global_parm.h"
#include "poweroff.h"
#include "./app_setting_base.h"



//---------------------------------------------------------------

lv_timer_t* timer_power;/* 创建一个定时器 */
static const char* text_time[7] = {"", "5分钟","10分钟","15分钟","20分钟","30分钟", "60分钟"};
static const int8_t acAutoOffTbl[] = { (int8_t)5, (int8_t)10, (int8_t)15, (int8_t)20, (int8_t)30, (int8_t)60 };

static int8_t curLine = 1;   //当前line
static lv_obj_t* lines[7] = { 0 }; //焦点控制line
static lv_obj_t*  labels[7] = { 0 }; 
static lv_obj_t* g_check[7] = { 0 }; //复选框
static int check_line = 0;
extern  const lv_img_dsc_t g_XXX_img_dsc_tbl[];

static bool timeflag;

static uint8_t checkFlag;

static lv_SETTING_poweroff* pow = NULL;

//----------------------------------------------------------------------

void poweroff_screen_init(void);
static void pop_init(void);
static void timer_cb(lv_timer_t* timer);
void EVENT_FREE_POWEROFF(void);
extern int16_t setPrjGlobalParm(SYS_PARM_TYPE type, int16_t bufLen, void* buf);


//----------------------------------------------------------------------
static const char * TAG = "pOffCfg";

static int get_checkTime(void)
{
	int tmMinute = getPrjGlobalParm(ZB_PARM_POWEROFF_FLAG, 0, NULL);
	int sn = 0;
	for (; sn < M_ARRAY_LEN(acAutoOffTbl); sn++)
	{
		if (tmMinute <= acAutoOffTbl[sn])
			break;
	}
	if (sn >= M_ARRAY_LEN(acAutoOffTbl))
	{
		sn--;
	}
	return sn;
}


// 在文件头部添加样式定义
static lv_style_t selected_border_style;

static void update_selection_style(int new_line) {
    // 清除所有行的选中状态
    for (int i = 1; i <= M_ARRAY_LEN(acAutoOffTbl); i++) {
        lv_obj_remove_style(lines[i], &selected_border_style, 0);
    }

    // 为新选中行添加样式
    if (new_line >= 1 && new_line <= M_ARRAY_LEN(acAutoOffTbl)) {
        lv_obj_add_style(lines[new_line], &selected_border_style, 0);
    }
}


static int set_checkTime(int check_line)
{
	if (check_line >= 1) {
		check_line--;
	}
	int tmMinute = acAutoOffTbl[check_line];
	int ret = setPrjGlobalParm(ZB_PARM_POWEROFF_FLAG, 1, &tmMinute);
	return 1;
}

int PageResource_Setting_poweroff(lv_event_code_t msg, int param)
{
    switch (msg) {
    case EVENT_FREE_AP_RES: {
        if (pow != NULL)
        {
            curLine = 1;
            if (pow->ui_Screen1 != NULL)
            {
                lv_obj_del(pow->ui_Screen1);
            }
            heap_caps_free(pow);
            pow = NULL;
        }
    }
    default:
        break;
    }
    return 0;
}


void EVENT_FREE_POWEROFF(void) {
    if (pow != NULL)
    {
        curLine = 1;
        if (pow->ui_Screen1 != NULL)
        {
            lv_obj_del(pow->ui_Screen1);
        }
        heap_caps_free(pow);
        pow = NULL;
    }
    
}

static void delCheck(int checkline)
{
    if (checkline>0)
    {
        if ((checkFlag != checkline)&&(pow->ui_Checkbox != NULL))
        {
            lv_obj_del(pow->ui_Checkbox);
            pow->ui_Checkbox = NULL;
            pop_init();
            pow->ui_Checkbox = lv_checkbox_create(lines[checkline]);
            lv_checkbox_set_text(pow->ui_Checkbox, "");
            lv_obj_set_x(pow->ui_Checkbox, 87);
            lv_obj_set_y(pow->ui_Checkbox, 1);
            lv_obj_set_align(pow->ui_Checkbox, LV_ALIGN_CENTER);
            lv_obj_set_style_radius(pow->ui_Checkbox, 90, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_add_state(pow->ui_Checkbox, LV_STATE_CHECKED);
            set_checkTime(check_line);
        }
        else if ((checkFlag != checkline) && (pow->ui_Checkbox == NULL))
        {
            pop_init();
            pow->ui_Checkbox = lv_checkbox_create(lines[checkline]);
            lv_checkbox_set_text(pow->ui_Checkbox, "");
            lv_obj_set_x(pow->ui_Checkbox, 87);
            lv_obj_set_y(pow->ui_Checkbox, 1);
            lv_obj_set_align(pow->ui_Checkbox, LV_ALIGN_CENTER);
            lv_obj_set_style_radius(pow->ui_Checkbox, 90, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_add_state(pow->ui_Checkbox, LV_STATE_CHECKED);
            set_checkTime(check_line);
        }

        else if ((checkFlag == checkline) && (pow->ui_Checkbox != NULL))
        {
            lv_obj_del(pow->ui_Checkbox);
            pow->ui_Checkbox = NULL;
            set_checkTime(0);
        }
        else if ((checkFlag == checkline) && (pow->ui_Checkbox == NULL))
        {
            pop_init();
            pow->ui_Checkbox = lv_checkbox_create(lines[checkline]);
            lv_checkbox_set_text(pow->ui_Checkbox, "");
            lv_obj_set_x(pow->ui_Checkbox, 87);
            lv_obj_set_y(pow->ui_Checkbox, 1);
            lv_obj_set_align(pow->ui_Checkbox, LV_ALIGN_CENTER);
            lv_obj_set_style_radius(pow->ui_Checkbox, 90, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_add_state(pow->ui_Checkbox, LV_STATE_CHECKED);
            set_checkTime(check_line);
        }


        checkFlag = checkline;
    }
}

static void checkShow(int checkline)
{

    //if (checkline > 0)
    //{
    //    pow->ui_Checkbox = lv_checkbox_create(lines[checkline]);
    //    lv_checkbox_set_text(pow->ui_Checkbox, "");
    //    lv_obj_set_x(pow->ui_Checkbox, 87);
    //    lv_obj_set_y(pow->ui_Checkbox, 1);
    //    lv_obj_set_align(pow->ui_Checkbox, LV_ALIGN_CENTER);

    //    lv_obj_set_style_radius(pow->ui_Checkbox, 90, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    //    lv_obj_add_state(pow->ui_Checkbox, LV_STATE_CHECKED);
    //}
 }

static void pwroff_popWin_cb(lv_event_t* evt) {
	lv_event_code_t evtCode = evt->code;
	if (LV_EVENT_DELETE == evtCode) {
		if (timer_power) {
			lv_timer_del(timer_power);	timer_power = NULL;
		}
	}
}


static void _pwrOff_lst_item_click_cb(lv_event_t * evt)
{
    lv_obj_t* obj = lv_event_get_target(evt);
    int32_t userData = (int32_t)lv_obj_get_event_user_data(obj, _pwrOff_lst_item_click_cb); 
    curLine = userData;
    
    // 更新选中状态
    check_line = curLine;
    update_selection_style(check_line);
    set_checkTime(check_line);
    
    // 显示提示弹窗
    if (timer_power) {
        lv_timer_del(timer_power);
        timer_power = NULL;
    }
    timer_power = lv_timer_create(timer_cb, 3000, NULL);
    pop_init();
}

static void poweroff_cb(lv_event_t* evt)
{
    int keyVal = 0;
    short lineNoPrev = curLine;
    short imgflag = 0;
    if (pow && (pow->pop == NULL))
    {
        lv_event_code_t evtCode = evt->code;
        if (LV_EVENT_KEY_STAGE == evt->code) {
            keyVal = lv_event_get_key(evt);
            if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_REPEAT) {
                keyVal &= ~LV_EX_FLAG_KEY_MASK;
                evtCode = LV_EVENT_KEY;
            }
        }
        if (LV_EVENT_KEY == evtCode) {
            if (!keyVal) {
                keyVal = lv_event_get_key(evt);
            }
            if (LV_KEY_UP == keyVal || LV_KEY_DOWN == keyVal) {

                if (lineNoPrev < 1 && LV_KEY_UP == keyVal) {
                    lineNoPrev = 1;
                }
                if (lineNoPrev > 5 && LV_KEY_DOWN == keyVal) {
                    lineNoPrev = 5;
                }
                short diff = (LV_KEY_UP == keyVal ? -1 : 1);
                short lineNo = lineNoPrev + diff;
                imgflag = lineNo;

                bool bNeedUnfocus = false;
                if (lineNo < 1)
                {
                    lineNo = 1;
                }
                else if (lineNo > 5)
                {
                    lineNo = 5;
                }

                else {
                    curLine = lineNo;
                    lv_group_focus_obj(lines[lineNo]);



                }
                if (bNeedUnfocus) {
                    lv_obj_clear_state(lines[lineNo], LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);
                }


                lv_event_stop_processing(evt);
            }

            else if (LV_KEY_ENTER == keyVal) {


                check_line = curLine;
                timer_power = lv_timer_create(timer_cb, 3000, NULL);
                lv_timer_enable(timer_power);

                delCheck(check_line);

            }
            else if (LV_KEY_ESC == keyVal) {
                lv_indev_keypad_wait_release();
                lv_event_stop_processing(evt);
                lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
            }
        }
        else if (LV_EVENT_DELETE == evtCode) {
        }

    }
    else
    { // 有 pop 信息框时
        lv_event_code_t evtCode = evt->code;
        if (LV_EVENT_KEY_STAGE == evtCode) {
            keyVal = lv_event_get_key(evt);
            if ((LV_KEY_ESC|LV_EX_FLAG_KEY_SHORT) == keyVal) {
                lv_obj_del(pow->pop);  pow->pop = NULL;
            }
        }
    }
}


/* 定时器回调函数 */
void timer_cb(lv_timer_t* timer) {
   // lv_obj_clean(pop);
    if (pow && (pow->pop != NULL))
    {
        lv_obj_del(pow->pop);  pow->pop = NULL;
    }
    if (timer_power) {
        lv_timer_del(timer_power);   timer_power = NULL;
    }
}


static lv_style_t style_dialog;
static void create_dialog_style(void) {
    lv_style_init(&style_dialog);
    lv_style_set_bg_color(&style_dialog, lv_color_hex(0x000000));
    lv_style_set_bg_opa(&style_dialog, LV_OPA_80);
    lv_style_set_border_width(&style_dialog, 0);
    lv_style_set_clip_corner(&style_dialog, true);
    lv_style_set_radius(&style_dialog, 20);
}


static void pop_init(void)
{
    if (pow->pop != NULL)
    {
        lv_obj_del(pow->pop);  pow->pop = NULL;
    }
    pow->pop = lv_obj_create(pow->ui_Screen1);

    create_dialog_style();
    lv_obj_add_style(pow->pop, &style_dialog, 0);
    lv_obj_set_size(pow->pop, 264, 106);
    lv_obj_set_pos(pow->pop, 38, 127);
    //lv_obj_set_width(pow->pop, 150);
    //lv_obj_set_height(pow->pop, 150);
    //lv_obj_set_x(pow->pop, 1);
    //lv_obj_set_y(pow->pop, 9);
    lv_obj_set_align(pow->pop, LV_ALIGN_CENTER);
    lv_obj_clear_flag(pow->pop, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_align(pow->pop, LV_ALIGN_CENTER);
    lv_obj_set_style_radius(pow->pop, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(pow->pop, lv_color_hex(0x000000),0);
    lv_obj_set_style_border_opa(pow->pop, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(pow->pop, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(pow->pop);
    lv_obj_add_event_cb(pow->pop, pwroff_popWin_cb, LV_EVENT_DELETE, NULL);

    //pow->power = lv_img_create(pow->pop);
    //lv_obj_set_pos(pow->power, 50, 20);
    //lv_img_dsc_t* power_icon = &g_XXX_img_dsc_tbl[MMN_powroff_logo - 1];
    //lv_img_set_src(pow->power, power_icon);


    pow->poptext = lv_label_create(pow->pop);
    lv_label_set_text_fmt(pow->poptext, "设备会在熄屏(且空闲)\n%s后自动关机", text_time[curLine]);
    lv_obj_set_style_text_color(pow->poptext, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(pow->poptext);

    
    //lv_obj_set_style_text_font(poptext, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);


}

static void _pwrOff_esc_btn_click_cb(lv_event_t * evt)
{
	lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_RET_STNG_MENU);
	return;
}

void poweroff_screen_init(void)
{
    pow = heap_caps_malloc_dflt(sizeof(lv_SETTING_poweroff));
    memset(pow, 0, sizeof(lv_SETTING_poweroff));

    lv_style_init(&selected_border_style);
    lv_style_set_border_color(&selected_border_style, lv_color_hex(0x008CFF));
    lv_style_set_border_width(&selected_border_style, 2);
    lv_style_set_border_opa(&selected_border_style, LV_OPA_COVER);

    pow->ui_Screen1 = lv_obj_create(lv_scr_act());
    lv_obj_set_size(pow->ui_Screen1, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(pow->ui_Screen1, lv_color_hex(STNG_AP_BG_COLOR_HEX), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(pow->ui_Screen1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(pow->ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_border_width(pow->ui_Screen1, 0, 0);
    lv_obj_set_style_radius(pow->ui_Screen1, 0, 0);

    stng_createEscBtn(pow->ui_Screen1, _pwrOff_esc_btn_click_cb, 0);

    pow->ui_Panelframe = lv_obj_create(pow->ui_Screen1);
    lv_obj_set_width(pow->ui_Panelframe, SCREEN_WIDTH);
    lv_obj_set_height(pow->ui_Panelframe, SCREEN_HEIGHT - 40);
    lv_obj_set_x(pow->ui_Panelframe, 1);
    lv_obj_set_y(pow->ui_Panelframe, 32);
    lv_obj_set_align(pow->ui_Panelframe, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(pow->ui_Panelframe, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(pow->ui_Panelframe, lv_color_hex(STNG_AP_BG_COLOR_HEX), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(pow->ui_Panelframe, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_outline_width(pow->ui_Panelframe, 0, 0);
    lv_obj_set_style_border_width(pow->ui_Panelframe, 0, 0);

    pow->ui_Panel1 = lv_obj_create(pow->ui_Panelframe);
    lv_obj_set_width(pow->ui_Panel1, SCREEN_WIDTH);
    lv_obj_set_height(pow->ui_Panel1, SCREEN_HEIGHT - 70);
    lv_obj_set_x(pow->ui_Panel1, 1);
    lv_obj_set_y(pow->ui_Panel1, 32);
    lv_obj_set_align(pow->ui_Panel1, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_flex_flow(pow->ui_Panel1, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(pow->ui_Panel1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(pow->ui_Panel1, 12, LV_PART_MAIN);
    lv_obj_set_style_radius(pow->ui_Panel1, 19, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(pow->ui_Panel1, lv_color_hex(0xF2F2F2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(pow->ui_Panel1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(pow->ui_Panel1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(pow->ui_Panel1, 0,0);
    lv_obj_set_style_pad_top(pow->ui_Panel1, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(pow->ui_Panel1, 5, LV_PART_MAIN);
    lv_obj_set_style_outline_width(pow->ui_Panel1, 0, 0);

    for (int i = 1; i <= M_ARRAY_LEN(acAutoOffTbl); i++) { 
        pow->btn_tim = lv_btn_create(pow->ui_Panel1);
      lv_obj_set_width(pow->btn_tim, SCREEN_WIDTH - 30);
      lv_obj_set_height(pow->btn_tim, 50);
     // lv_obj_set_x(pow->btn_tim, -55);
     // lv_obj_set_y(pow->btn_tim, 106);
      lv_obj_set_align(pow->btn_tim, LV_ALIGN_CENTER);
      lv_obj_set_style_bg_color(pow->btn_tim, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_opa(pow->btn_tim, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_shadow_width(pow->btn_tim, 0, 0);
      lines[i] = pow->btn_tim;

      pow->btn_label = lv_label_create(pow->btn_tim);
      lv_obj_set_width(pow->btn_label, LV_SIZE_CONTENT);
      lv_obj_set_height(pow->btn_label, LV_SIZE_CONTENT);
      lv_obj_align(pow->btn_label, LV_ALIGN_LEFT_MID, 10, 0);
      lv_label_set_text(pow->btn_label, text_time[i]);
      lv_obj_set_style_text_color(pow->btn_label, lv_color_hex(0x242424), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_opa(pow->btn_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    //  lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

      labels[i] = pow->btn_label;

      lv_obj_add_event_cb(pow->btn_tim, _pwrOff_lst_item_click_cb, LV_EVENT_SHORT_CLICKED, (void *)i);

    }
    check_line = get_checkTime();
    update_selection_style(check_line);

    pow->ui_Label1 = lv_label_create(pow->ui_Screen1);
    lv_obj_set_width(pow->ui_Label1, LV_SIZE_CONTENT);
    lv_obj_set_height(pow->ui_Label1, LV_SIZE_CONTENT);
    lv_obj_align(pow->ui_Label1, LV_ALIGN_TOP_MID, 0, 20);
    lv_label_set_text(pow->ui_Label1, "定时关机");

    if (check_line != 0)
    {
        curLine = check_line;
        lv_group_focus_obj(lines[check_line]);
    }
    //else {
    //    check_line = 3;
    //    curLine = 3;
    //    set_checkTime(check_line);
    //    lv_group_focus_obj(lines[check_line]);
    //}
    checkShow(check_line);
   // sys_setCurMainWinObj(pow->ui_Panel1);

    lv_group_add_obj(lv_group_get_default(), pow->ui_Panel1);
    lv_obj_add_event_cb(pow->ui_Panel1, poweroff_cb, LV_EVENT_ALL, NULL);
   // sys_setCurMainWinAndCb(pow->ui_Panel1, poweroff_cb);
}

