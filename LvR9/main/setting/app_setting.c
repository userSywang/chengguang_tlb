//========================================================[]
//Name: app_setting.c
//Function: 
//========================================================[]
/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>
#include "../utils/style_tz.h"
#include "../zb_base.h"
#include "../utils/misc_utils.h"
#include "../ex_widget/app_msg.h"
#include "./app_setting_base.h"
#include "../data_parse/img_res.h"
//=========================================================[]
//=========================================================[]

int main_menu_get_scr_sel_no();
void setting_brigthness_entry(void);
void setting_brigthness_del(void);
void setting_my_device_entry(void);
void setting_my_device_del(void);
void network_setting_entry(void);
void network_setting_exit(void);
int isWifiStableState(void);

void setting_autooff_screen_entry(void);
void setting_autooff_screen_del(void);

void bluetooth_setting_entry(void);
void bluetooth_setting_exit(void);

void setting_storage_space_entry(void);
void setting_storage_space_del(void);
void setting_update_entry(void);
void setting_update_del(void);

void poweroff_screen_init(void);
void EVENT_FREE_POWEROFF(void);

void scan_bind_screen_entry(void);
void scan_bind_screen_del(void);

void qrcode_page_init(void);
void qrcode_page_deinit(void);

int prdTs_audLoop_scr_init(int parm);
int prdTs_audLoop_scr_deinit(void);
int prdTs_audLoop_scr_init(int parm);

extern int seting_hanlder_sub_my_device(int param);

static int _setng_choice_proc();

//=========================================================[]
/**********************
 *      MACROS
 **********************/

#if DARK_MODE
#define STNG_AP_BG_COLOR_HEX 0x000000   // 纯黑色背景
#define BUTTON_BG_COLOR_HEX  0x212121   // 按钮背景色
#define TEXT_COLOR           lv_color_white()  // 白色文字
#else
#define STNG_AP_BG_COLOR_HEX 0xFFFFFF   // 白色背景
#define BUTTON_BG_COLOR_HEX  0xFFFFFF   // 白色按钮背景
#define TEXT_COLOR           lv_color_black()  // 黑色文字
#endif

#define TEST_0630_BLUETOOTH 1

#if TEST_0630_BLUETOOTH 
#define		MMENU_SETTING			0	//          
#define		NETWORK_SETTING			1	//网络设置
#define     BULE_TOOTH              2   //蓝牙
#define		BRIGHTNESS_SETTING		3	//亮度设置
#define     AUTO_SCREEN_OFF         4   //自动息屏
#define		SCAN_CODE		        5	//扫码绑定
#define		UPDATE_SETTING			6	//系统升级
#define		STORE_SPACE_SETTING		7	//存储空间
#define		MY_DEV_SETTING			8	//我的设备
#define     WECHAT_SERVICE          9   //微信客服 
#else

#define		NETWORK_SETTING			1	//网络设置
#define		BRIGHTNESS_SETTING		2	//亮度设置  // 从3改为2
#define     AUTO_SCREEN_OFF         3   //自动息屏  // 从4改为3
#define		SCAN_CODE		        4	//扫码绑定  // 从5改为4
#define		UPDATE_SETTING			5	//系统升级  // 从6改为5
#define		STORE_SPACE_SETTING		6	//存储空间  // 从7改为6
#define		MY_DEV_SETTING			7	//我的设备  // 从8改为7
#define     WECHAT_SERVICE          8   //微信客服  // 从9改为8
#endif

#if TEST_0630_BLUETOOTH 
#define SETNG_MNU_ITEM_N  9  // 测试项总数 
#else
#define SETNG_MNU_ITEM_N  8  // 测试项总数 
#endif

#define  CMD_PAR_ENTER_SUB  0xc1

struct setng_menu_scr_inf {
	lv_obj_t  * aItemObjTbl[SETNG_MNU_ITEM_N];
};


//=========================================================[]

static bool g_isInSubItem;
static int8_t g_setngSel = 0;


static struct setng_menu_scr_inf * g_setMnuScrInf;

lv_obj_t  * g_setMnuBgWin;

//--------------------------------------
static const char * TAG = "apStng";



#if TEST_0630_BLUETOOTH
static const char* g_setngMenuStr =
"网络\n" "蓝牙\n" "亮度\n""自动息屏\n" "扫码绑定\n" "系统升级\n"  "存储空间\n" "我的设备\n""微信客服\n";
#else //05.29 暂时去掉定时关机选项
static const char* g_setngMenuStr =
"网络\n" "亮度\n""自动息屏\n" "扫码绑定\n" "系统升级\n"  "存储空间\n" "我的设备\n""微信客服\n";
#endif
#define STNG_ESC_BTN_LOC_Y  0
#define TOP_BTN_BORDER 12
#define TOP_BTN_TOP_SPACE_H 8
lv_obj_t* stng_createEscBtn(lv_obj_t * parent, lv_event_cb_t clickCb, int extra)
{
	lv_obj_t * img = lv_img_create(parent);
	const lv_img_dsc_t * pImgDsc = g_XXX_img_dsc_tbl + MMN_icn_esc - 1;
	lv_img_set_src(img, pImgDsc);
	lv_obj_set_size(img, pImgDsc->header.w + TOP_BTN_BORDER * 2, pImgDsc->header.h + TOP_BTN_BORDER * 2);
	lv_obj_set_pos(img, 16, STNG_ESC_BTN_LOC_Y);
	lv_obj_set_style_border_width(img, TOP_BTN_BORDER, 0);
	lv_obj_set_style_radius(img, 4, 0);
	lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);

	lv_obj_set_style_opa(img, LV_OPA_60, 0);
	lv_obj_set_style_opa(img, LV_OPA_100, LV_STATE_FOCUS_KEY);
	lv_obj_set_style_opa(img, LV_OPA_80, LV_STATE_PRESSED);
	// lv_color_t colorBorderFcs = lv_color_white();
	lv_obj_set_style_border_opa(img, LV_OPA_0, 0);
	lv_obj_add_event_cb(img, clickCb, LV_EVENT_CLICKED, NULL);

	return img;
}

void stng_createDelBtn(lv_obj_t* parent, lv_event_cb_t clickCb, int extra,int x,int y)
{
	lv_obj_t* img = lv_img_create(parent);
	const lv_img_dsc_t* pImgDsc = zb_img_res_load_one(NULL, SETTING_IMG_DEL, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(img, pImgDsc);
	lv_obj_set_size(img, pImgDsc->header.w + TOP_BTN_BORDER * 2, pImgDsc->header.h + TOP_BTN_BORDER * 2);
	lv_obj_set_pos(img, x, y);
	lv_obj_set_style_border_width(img, TOP_BTN_BORDER, 0);
	lv_obj_set_style_radius(img, 4, 0);
	lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);

	lv_obj_set_style_opa(img, LV_OPA_60, 0);
	lv_obj_set_style_opa(img, LV_OPA_100, LV_STATE_FOCUS_KEY);
	lv_obj_set_style_opa(img, LV_OPA_80, LV_STATE_PRESSED);
	// lv_color_t colorBorderFcs = lv_color_white();
	lv_obj_set_style_border_opa(img, LV_OPA_0, 0);
	lv_obj_add_event_cb(img, clickCb, LV_EVENT_CLICKED, NULL);
}



static void _setng_mnuItemClick_cb(lv_event_t * evt)
{
	lv_obj_t* obj = lv_event_get_target(evt);

	int32_t userData = (int32_t)lv_obj_get_event_user_data(obj, _setng_mnuItemClick_cb); 
	ESP_LOGI(TAG, "itm %ld clcked", userData);

	g_setngSel = (int8_t)userData;
	lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_ENTER_SUB);
	return;
}


static void _setng_mnuWinEvtCb(lv_event_t * evt) {
	lv_event_code_t evtCode = evt->code;

	int keyVal = 0;
	switch (evtCode) {
		case LV_EVENT_KEY_STAGE: {
			keyVal = lv_event_get_key(evt);
			if (((LV_EX_FLAG_KEY_MASK & keyVal) != LV_EX_FLAG_KEY_REPEAT) && ((LV_EX_FLAG_KEY_MASK & keyVal) != LV_EX_FLAG_KEY_SHORT)) {
				return;
			}
			keyVal &= ~LV_EX_FLAG_KEY_MASK;
			ESP_LOGI(TAG, "mnu key %x", keyVal);
			switch (keyVal) {
				case LV_KEY_UP:
				case LV_KEY_DOWN: {
					int16_t diff = (LV_KEY_UP == keyVal ? SETNG_MNU_ITEM_N - 1 : 1);
					int16_t itemNoN = (g_setngSel + diff) % SETNG_MNU_ITEM_N;
					lv_obj_clear_state(g_setMnuScrInf->aItemObjTbl[g_setngSel], LV_STATE_FOCUS_KEY);
					g_setngSel = itemNoN;
					lv_obj_add_state(g_setMnuScrInf->aItemObjTbl[itemNoN], LV_STATE_FOCUS_KEY);
					break;
				}
				case LV_KEY_ENTER:
					ESP_LOGI(TAG, "entKey prs, No %d", (int)g_setngSel);
					//lv_obj_del_async(g_setMnuBgWin);
					lv_timer_create(zb_ex_cmd_timer_cb, 0, CMD_PAR_ENTER_SUB);
					break;
				case LV_KEY_ESC:
					zb_requestSwitchAp(C_ZB_AP_MAINMENU);
					break;
#ifdef C_PC_SIM
				case LV_KEY_EX_FUNC1:
					create_dialog_with_timer(true, "连接失败", 2000,184,120);
					break;
#endif
				default: {
					break;
				}
			}
			break;
		}
		case LV_EVENT_DELETE: {
			if (g_setMnuScrInf) {
				heap_caps_free(g_setMnuScrInf);  g_setMnuScrInf = NULL;
			}
			g_setMnuBgWin = NULL;
			break;
		}
		default:
			break;
	}
}


static void _stng_lst_scr_esc_click_cb(lv_event_t * evt)
{
	zb_requestSwitchAp(C_ZB_AP_MAINMENU);
	return;
}

/*
 * @parm: 1 means  enter from mainMenu; 0 return from subItem.
 */
int setng_menu_scr_init(int parm) { // 入口菜单界面初始化 
	g_isInSubItem = false;
	g_setMnuScrInf = (struct setng_menu_scr_inf *)heap_caps_malloc_dflt(sizeof(struct setng_menu_scr_inf));
	memset(g_setMnuScrInf, 0, sizeof(struct setng_menu_scr_inf));

	lv_obj_t * mainBg = lv_obj_create(lv_scr_act());
	lv_obj_set_size(mainBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(mainBg, 0, 0);
	lv_obj_set_style_radius(mainBg, 0, 0);
	lv_obj_set_style_pad_all(mainBg, 0, 0);
	lv_color_t colorBg = lv_color_hex(STNG_AP_BG_COLOR_HEX);
	lv_obj_set_style_bg_color(mainBg, colorBg, 0);
	lv_obj_set_scroll_dir(mainBg, LV_DIR_NONE);
	lv_obj_clear_flag(mainBg, LV_OBJ_FLAG_SCROLLABLE);

	stng_createEscBtn(mainBg, _stng_lst_scr_esc_click_cb, 0);
	lv_obj_t * lblTitle = lv_label_create(mainBg);
	lv_label_set_text(lblTitle, "设置");
	lv_obj_align(lblTitle, LV_ALIGN_TOP_MID, 0, STNG_ESC_BTN_LOC_Y + TOP_BTN_BORDER+5);
	lv_obj_set_style_text_color(lblTitle, TEXT_COLOR, 0);

	//---------------
	lv_obj_t * mainWin = lv_obj_create(mainBg);
	lv_obj_set_size(mainWin, LV_PCT(100), SCREEN_HEIGHT - 60);
	lv_obj_align(mainWin, LV_ALIGN_BOTTOM_LEFT, 0, 0);

	lv_obj_set_style_radius(mainWin, 0, 0);
	lv_obj_set_scrollbar_mode(mainWin, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_flex_flow(mainWin, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_scroll_dir(mainWin, LV_DIR_VER);
	lv_obj_set_flex_align(mainWin, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_row(mainWin, 12, 0);
	lv_obj_set_style_border_width(mainWin, 0, 0);
	lv_obj_set_style_border_opa(mainWin, LV_OPA_0, 0);
	lv_obj_set_style_bg_opa(mainWin, LV_OPA_0, 0);

	int txtAllLen = strlen(g_setngMenuStr);
	char * pcTextWhole = (char *)heap_caps_malloc_dflt(txtAllLen + 1);
	memcpy(pcTextWhole, g_setngMenuStr, txtAllLen + 1);



	char * pcSaved = NULL;
	char * pcOnce = strtok_r(pcTextWhole, "\n", &pcSaved);
	for (int16_t itemNo = 0; itemNo < SETNG_MNU_ITEM_N; itemNo++) {

		lv_obj_t* objItem = lv_btn_create(mainWin);
		//lv_obj_remove_style_all(objItem);
		lv_obj_set_size(objItem, 280, 51);
		lv_obj_set_style_bg_color(objItem, lv_color_hex(BUTTON_BG_COLOR_HEX), 0);

		lv_obj_add_style(objItem, get_global_style(STYLE_BTN_BASE), LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_add_style(objItem, get_global_style(STYLE_BTN_BASE), LV_STATE_FOCUS_KEY | LV_STATE_FOCUSED);

		lv_obj_t* lblItem = lv_label_create(objItem);
		lv_label_set_text(lblItem, pcOnce);
		lv_obj_set_style_text_color(lblItem, TEXT_COLOR, 0);
		//lv_obj_set_style_pad_top(lblItem, 5, 0);
		lv_obj_set_style_border_opa(lblItem, LV_OPA_TRANSP, 0);
		lv_obj_set_style_border_opa(lblItem, LV_OPA_COVER, LV_STATE_FOCUS_KEY);
		lv_obj_set_style_border_side(lblItem, LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_RIGHT, 0);
		lv_obj_set_style_border_width(lblItem, 10, 0);
		lv_obj_set_style_outline_width(lblItem, 0, 0);
		lv_obj_align(lblItem, LV_ALIGN_LEFT_MID, 48, 0);
		//lv_obj_set_style_outline_width(lblItem, 8, 0);
		//lv_obj_set_style_outline_opa(lblItem, LV_OPA_TRANSP, 0);

		lv_obj_add_event_cb(objItem, _setng_mnuItemClick_cb, LV_EVENT_CLICKED, (void*)itemNo);

		g_setMnuScrInf->aItemObjTbl[itemNo] = objItem;
		pcOnce = strtok_r(NULL, "\n", &pcSaved);

		lv_obj_t* return_icon = lv_img_create(objItem);
		lv_img_dsc_t* pImgDsc_img = zb_img_res_load_one(NULL, SETTING_IMG_ARROW, IMG_FREE_TYPE_AUTO, NULL);
		lv_img_set_src(return_icon, pImgDsc_img);
		lv_obj_align_to(return_icon, objItem, LV_ALIGN_RIGHT_MID, -10, 0);


		lv_obj_t* icon = lv_img_create(objItem);

#if TEST_0630_BLUETOOTH
		lv_img_dsc_t* pImgDsc_icon = g_setting_img_dsc + itemNo;
#else
		int iconIndex = itemNo;
		if (itemNo != 0) {
			iconIndex = itemNo + 1;  // 非0项索引加1
		}
		lv_img_dsc_t* pImgDsc_icon = g_setting_img_dsc + iconIndex;
	


#endif
		
		lv_img_set_src(icon, pImgDsc_icon);
		lv_obj_align_to(icon, objItem, LV_ALIGN_LEFT_MID, 10, 0);

	}

	if (g_setngSel != 0) {  // non-std
		lv_obj_clear_state(g_setMnuScrInf->aItemObjTbl[0], LV_STATE_FOCUS_KEY);
	}
	lv_obj_add_state(g_setMnuScrInf->aItemObjTbl[g_setngSel], LV_STATE_FOCUS_KEY);
	heap_caps_free(pcTextWhole);  pcTextWhole = NULL;

	lv_obj_add_event_cb(mainWin, _setng_mnuWinEvtCb, LV_EVENT_DELETE, NULL);
	sys_setCurMainWinAndCb(mainWin, _setng_mnuWinEvtCb);
	g_setMnuBgWin = mainBg;
	return 0;
}


static int stng_menu_scr_deinit(void) {  // 入口菜单界面销毁 
	if (g_setMnuBgWin) {
		lv_obj_del(g_setMnuBgWin);  g_setMnuBgWin = NULL;
	}
	return 0;
}



int zb_setting_ap_init(void)
{
	setng_menu_scr_init(0);
	return 0;
}



static int _setng_choice_proc()
{
	LV_LOG("%s [%d] g_setngSel:%d \n", __FUNCTION__, __LINE__, g_setngSel);
	g_isInSubItem = true;

	switch (g_setngSel + 1) 
	{
		case NETWORK_SETTING:
			network_setting_entry();

			break;

#if TEST_0630_BLUETOOTH
		case BULE_TOOTH:
			bluetooth_setting_entry();
			break;
#endif
		case BRIGHTNESS_SETTING:
			setting_brigthness_entry();
			break;
#if 0
		case AUTO_ONOFF_SETTING:
			poweroff_screen_init();
			break;
#endif
		case AUTO_SCREEN_OFF:
			setting_autooff_screen_entry();
			break;

		case SCAN_CODE:
			scan_bind_screen_entry();
			break;

		case UPDATE_SETTING:
			setting_update_entry();
			break;
		case STORE_SPACE_SETTING:
			setting_storage_space_entry();
			break;
		//case AUDIO_TEST_ENTRY:
		//	prdTs_audLoop_scr_init(0);
		//	break;
		case MY_DEV_SETTING:
			setting_my_device_entry(); 
			break;

		case WECHAT_SERVICE:
			qrcode_page_init();
			break;

		default:
			ESP_LOGW(TAG, "invalid choice sub %d", g_setngSel);
			g_isInSubItem = false;
			break;
		break;
	}

	return (1);
}

void _stng_curScr_del(void) {
	if (!g_isInSubItem) {
		stng_menu_scr_deinit();
		return 0;
	}
	switch (g_setngSel + 1) 
	{
		case NETWORK_SETTING:
			network_setting_exit();
	
			break;
#if TEST_0630_BLUETOOTH
		case BULE_TOOTH:
			bluetooth_setting_exit();
			break;
#endif

		case SCAN_CODE:
			scan_bind_screen_del();
			break;

		case BRIGHTNESS_SETTING:
			setting_brigthness_del();
			break;
#if 0
		case AUTO_ONOFF_SETTING:
			EVENT_FREE_POWEROFF();
			break;
#endif
		case AUTO_SCREEN_OFF:
			setting_autooff_screen_del();
			break;

		case UPDATE_SETTING:
			setting_update_del();
			break;
		case STORE_SPACE_SETTING:
			setting_storage_space_del();
			break;
		//case AUDIO_TEST_ENTRY:
		//	prdTs_audLoop_scr_deinit();
		//	break;
		case MY_DEV_SETTING:
			setting_my_device_del();
			break;

		case WECHAT_SERVICE:
			qrcode_page_deinit();
			break;

		default:
	
		break;
	}
}

int zb_setting_ap_handler(lv_event_code_t msg, int param) 
{
	LV_LOG("%s,[%d] msg %d ", __FUNCTION__,__LINE__, (int)msg);
	switch ((int)msg) 
	{
	case C_EVENT_FREE_AP_RES:
		LV_LOG("%s,[%d] , g_setngSel: %d \r\n ", __FUNCTION__,__LINE__, g_setngSel);
		_stng_curScr_del();
		break;

	case C_EVENT_EX_CMD: {
		ESP_LOGI(TAG, "EVENT_EX_CMD A, par:%d", param);
		switch (param) {
			case CMD_PAR_ENTER_SUB: {
				if (g_setMnuBgWin) {
					lv_obj_del(g_setMnuBgWin);   g_setMnuBgWin = NULL;
				}
				_setng_choice_proc();
				break;
			}
			case CMD_PAR_RET_STNG_MENU: {
				_stng_curScr_del();
				setng_menu_scr_init(0);
			}
			case NET_CONN_CMD_CODE: {
				if (NETWORK_SETTING == g_setngSel + 1) {
					ESP_LOGI(TAG, "req exit NetCnn");
					//zb_requestSwitchAp(C_ZB_AP_MAINMENU);
				}
				break;
			}
			case MY_DEV_SETTING: {  // todo
				seting_hanlder_sub_my_device(param);
				break;
			}
			default:
				break;
		}
		break;
	}
	case C_EVENT_DOES_BLOCK_ALARM:
		return (UPDATE_SETTING == g_setngSel);

	case C_EVENT_DOES_BLOCK_SYS_POWER_KEY:
		if (2 == param) { // 2 means longPress evt (power key)
			if (UPDATE_SETTING == g_setngSel) {
				return 1; // 数据更新时 要屏蔽关机键 
			}
		}
		return 0;

	default:
		break;
	}
	return 0;
}

void set_g_setngSel_num(int num)
{
	g_setngSel = num;
}