
#include <time.h>
#include <lvgl/lvgl.h>
#include "../utils/misc_utils.h"
#include "../utils/global_parm.h"
#include "mainmenu_base.h"
#include "../zb_base.h"
#include "../lvgl_main.h"

#include "../player/player.h"
#include "../data_parse/img_res.h"
#include "../ex_widget/app_msg.h"
#include "../ex_widget/misc_draw.h"
#include "../addressbook/contact_ap.h"
#include "../addressbook/contactAP_data.h"
#include "statbar_icon/statbar_icon.h"
#include "../../trd/ubus/include/tz_hal_quectel.h"

#ifndef C_PC_SIM
#include "../../trd/tz_sys_lib/include/cvi_tz_sys.h"
#endif

#if 0
#include "../include/adc_battery.h"
#include "../alarm_sdcard_evt_proc.h"


esp_err_t esp_wifi_sta_get_rssi(int *rssi);
extern bool zb_is_wifi_connect();
int zb_show_bgPlayerDlg(void);
int zb_show_alarmPromptDlg(void);
extern void zb_set_lcdDontAutoOffMode(int16_t confirmCode);
extern int get_tlb_cur_version_no(void);

extern int is_download_update_cfg_task_running(void);
extern bool esp32s3_http_get_update_cfg_finised_state(void);
extern void esp32s3_http_get_update_cfg(void);

#endif

int32_t zb_get_autoScreenOffCnt(void);


//#include "../setting/parental_control_pin.h"
#include "tlb_main_img.h"
#include "./mainmenu_base.h"


#define TAG  "mmu:"

extern const lv_img_dsc_t* MMN_BG;
extern const lv_img_dsc_t* MMN_BG_BLACK;
#define DOES_UP_DOWN_DIR_LOOP  0


#define MMN_1ST_LEVEL_PG_TOTAL   4  // 一级菜单总页数 
#define MMN_2ND_LEVEL_SCR_NO_ST  4  // 二级菜单起始页号 
#define MMN_2ND_LEVEL_PG_N  4
#define MMN_MENU_PG_TOTAL  (4+4)  // 菜单总页数(含各级的)

#define MMN_IS_1ST_LEVEL_MENU(scrNo)  ((scrNo)>=0 && (scrNo)<MMN_1ST_LEVEL_PG_TOTAL)
#define MMN_IS_2ND_LEVEL_MENU(scrNo)  ((scrNo)>=MMN_2ND_LEVEL_SCR_NO_ST && (scrNo)<MMN_MENU_PG_TOTAL)

#define MMN_SETTING_SCR_NO 2  // 设置所在页号 
#define MMN_XIMALY_SCR_NO  4  // 喜马拉雅所在页号 

//#define MMN_BACKDOOR_SCR_NO 2 // 后门所在的页号 

//----------
#define MMN_2ND_LEVEL_MENU_FUNC_NO_ST 0xc0 
#define MMN_KEWAI_TINGLI_MENU_FUNC_NO ((MMN_2ND_LEVEL_MENU_FUNC_NO_ST)+0)
#define MMN_GUOXUE_MENU_FUNC_NO ((MMN_2ND_LEVEL_MENU_FUNC_NO_ST)+1)
#define MMN_MY_RES_MENU_FUNC_NO ((MMN_2ND_LEVEL_MENU_FUNC_NO_ST)+2)


//#define MMN_APP_STORE_MENU_FUNC_NO  ((MMN_2ND_LEVEL_MENU_FUNC_NO_ST)+0)

//----------
#define C_MMNU_PREV_PG  -3
#define C_MMNU_NEXT_PG  -1
#define C_MMNU_DONOT_DO -2


//-----------------
#define C_MMN_BAT_LEVEL_N  5 // 主界面电量级别数
#define C_MMN_BAT_VAL_MIN   3440  // 电池最低电量
#define C_MMN_BAT_VAL_TOP_STAGE  3983  // 二级电量起始电压 mV, 必须为 g_battery_topLevelsTbl[]表的末项值! 
#define C_MMN_BAT_VAL_MAX   4150  // 电池最高电量
// 3.5V提示低电提醒，3.45V强制关机。

//-----------------
#define SECRET_HOLD_TIME 3000 // 长按3秒触发
static int32_t g_secretHoldStartTime = 0;
static bool g_devModeActive = false;


typedef struct {
    uint8_t grid_col;//列
    uint8_t grid_row;//行
    uint8_t col_span;//列跨度
    uint8_t row_span;//行跨度
    uint8_t resImgNo;//图片号
    uint8_t m_funcNo;//点击后要进入的界面  AP号
    uint16_t m_apInParm; // AP 输入参数 
    const char * pTitleTxt; // 标题 
} mnu_item_inf_t;

typedef struct { 
    lv_point_t m_loc;
    const char * m_pText;  // text or img 
} mnu_txt_item_t;

typedef struct { 
    uint8_t itemTotal;//项目总数 
    uint8_t gridTotal;  // 总 grid 数，按 colN * rowN 来算.
    uint8_t colNumEachRow;//每一行有多少列
    uint8_t exTextItemNum;//额外的文本项目信息
    const lv_coord_t * pColDscTbl;
    const lv_coord_t * pRowDscTbl;
    const mnu_item_inf_t * pItemInfTbl;
    const mnu_txt_item_t * pExTxtItemTbl;
} mnu_page_dsc_t;  // 菜单页面基本信息 


#define C_GRID_NUM_MAX   80
#define MMNU_PG_ITEM_N_MAX  30

//------------------ 

static int16_t  g_scrNo = -1;  // 当前屏号, 取值 自0起为一二级菜单页号, 还有 MMN_SCR_NO_PWD_LOGIN 等常量.
static int8_t   g_srcNoBak;  // 进入网络设置前的页面号 
static int8_t   g_1stLevelMnuScrNo;
static int8_t   g_1stLevelMnuSelNo;
static int16_t  g_itemNo = 0; // 当前(屏内)选项号

static int8_t  g_batteryLevelNo;  // 电量级别, bit7 为 1 (C_RECHARGE_FLAG) 表示充电, 这里 level 是 5 级的;
static int8_t  g_batteryLevelNoPct; 
static int8_t  g_wifiLevelNo = -1;  // wifi级别, -1 表示关闭, 0 未连接 ok;
int16_t  g_curTimeHM = -1;   // 时分，low byte 分, high byte 时 

static lv_timer_t * g_mmn_tmr;
static int8_t  g_subCntInTmr;

static lv_obj_t * g_rootWin;
static lv_obj_t * g_statBar;
static lv_obj_t * g_winA;
static int8_t   g_isSerialNoOk;
static int8_t   g_hasPromptDiskSpc;

static int _simState = 0;
static int _simPower = 0;
static int _classMode = 0;
static int _muteMode = 0;
static int _cy_conNect = 0;
static int _isWifiConnected = 0;
static int _wifiSignalStrength = 0;
static bool _isCharging = 0;
static int _power = 0;

//quectel_handler_t* g_sim_handler = NULL; //phone 句柄

static lv_obj_t * g_level1TopBtns[MMN_1ST_LEVEL_PG_TOTAL];

static lv_obj_t * g_page_btns[MMNU_PG_ITEM_N_MAX];

static uint8_t g_gridNoToItemNoTbl[C_GRID_NUM_MAX];

static lv_style_t style_indicator;
static lv_style_t style_focus;
static lv_obj_t* g_indicatorBars[MMN_1ST_LEVEL_PG_TOTAL] = { NULL };
//-------------------------

#define MMNU_EX_TXT_X_B   180
#define MMNU_EX_TXT_Y     35 
#define MMNU_EX_ICON_X     37
#define MMNU_EX_ICON_Y     30

#define MMNU_SEARCH_X_DIFF 28
#define MMNU_SEARCH_Y_DIFF 4


//----------------------------------------------
static int _mmn_promptNetCfg_scr_init(void);


//------------------------------
static const int16_t g_sectLocY_tbl[] = { 0, 400, 800, 1200 };  // (页内)各段(按y坐标)的定位表, non-std

static const mnu_item_inf_t g_pgYuwen_itmTbl[] = {  // 
	{
	.grid_col = 0,  .grid_row = 0,
	.col_span = 2,  .row_span = 1,
	.resImgNo = MMN_bigchs - 1, .m_funcNo = C_ZB_AP_DAYUWEN },  // 大语文
	{
	.grid_col = 0,  .grid_row = 1,
	.col_span = 1,  .row_span = 1,
	.resImgNo = MMN_textbook_yuwen - 1, .m_funcNo = C_ZB_AP_TEXTBOOK_CN, .m_apInParm = DATUM_CHINESE_TEXTBOOK },  // 国学天地 
		{
	.grid_col = 1,  .grid_row = 1,
	.col_span = 1,  .row_span = 1,
	.resImgNo = MMN_guoxue_tiandi - 1, .m_funcNo = MMN_GUOXUE_MENU_FUNC_NO },  // 国学天地 
		{
	.grid_col = 0,  .grid_row = 2,
	.col_span = 2,  .row_span = 1,
	.resImgNo = MMN_story - 1, .m_funcNo = C_ZB_AP_KEWAIGUSHI },
};

static const lv_coord_t g_pgYuwen_row_dsc[] = { MMNU_GRID_TOP_AID_ROW_H, 120, 128, 100, MMNU_GRID_BOTTOM_AID_ROW_H, LV_GRID_TEMPLATE_LAST };

static const mnu_page_dsc_t g_pgYuwen_dsc_inf = {
    .itemTotal = M_ARRAY_LEN(g_pgYuwen_itmTbl),
    .gridTotal = 2 * 3,
    .colNumEachRow = 2,
    .pRowDscTbl = g_pgYuwen_row_dsc,
    .pItemInfTbl = g_pgYuwen_itmTbl,
    .exTextItemNum = 0,
    .pExTxtItemTbl = NULL
};

//-----------------------
static const mnu_item_inf_t g_pgEng_itmTbl[] = { 

	{
	.grid_col = 0,  .grid_row = 2,
	.col_span = 1,  .row_span = 1,
	.resImgNo = MMN_textbook_eng - 1,.m_funcNo = C_ZB_AP_TEXTBOOK_ENG, .m_apInParm = DATUM_ENGLISH_TEXTBOOK },  //英语教材
	{
	.grid_col = 1,  .grid_row = 2,
	.col_span = 1,  .row_span = 1,
	.resImgNo = MMN_fenji - 1, .m_funcNo = C_ZB_AP_FENJIYUEDU, .m_apInParm = DATUM_HYNEMAN_READ },  // 大猫阅读 //.m_funcNo = C_ZB_AP_AUD_PLAYER,
	{
	.grid_col = 0,  .grid_row = 0,
	.col_span = 2,  .row_span = 1,
	.resImgNo = MMN_jingxuan - 1, .m_funcNo = MMN_KEWAI_TINGLI_MENU_FUNC_NO },  // 课外听力  .m_funcNo = MMN_KEWAI_TINGLI_MENU_FUNC_NO ,
};
/* 
{
.grid_col = 0,	.grid_row = 5,
.col_span = 1,	.row_span = 1,
.resImgNo = MMN_Group_758 - 1, .m_funcNo = C_ZB_AP_NO_STUDYWORD },	// english word   */


static const lv_coord_t g_pgEng_row_dsc[] = { MMNU_GRID_TOP_AID_ROW_H,180, 68, MMNU_GRID_BOTTOM_AID_ROW_H, LV_GRID_TEMPLATE_LAST };

static const mnu_page_dsc_t g_pgEng_dsc_inf = {
    .itemTotal = M_ARRAY_LEN(g_pgEng_itmTbl),
    .gridTotal = 2 * 3,
    .colNumEachRow = 2,
    .pRowDscTbl = g_pgEng_row_dsc,
    .pItemInfTbl = g_pgEng_itmTbl,
    .exTextItemNum = 0,
    .pExTxtItemTbl = NULL,
};
//-------------------------
//-------------------------
/* 
 {
.grid_col = 1,	.grid_row = 0,
.col_span = 1,	.row_span = 2,
.resImgNo = MMN_setting - 1, .m_funcNo = MMN_SETTING_MENU_FUNC_NO },  // 设置 
*/

static const mnu_item_inf_t g_pgApp_itmTbl[] = {

#if 1// 06.20的版本，主要用这个
	{
	.grid_col = 0,	.grid_row = 0, 
	.col_span = 1,	.row_span = 1,
	.resImgNo = MMN_payment_app - 1, .m_funcNo = C_ZB_AP_ALIPAY,.pTitleTxt = "支付宝" },	// 支付宝 
	{
	.grid_col = 1,	.grid_row = 0, 
	.col_span = 1,	.row_span = 1,
	.resImgNo = MMN_address_book - 1, .m_funcNo = C_ZB_AP_NO_CONTACT, .pTitleTxt = "通迅录" },	// 通迅录 
	{
	.grid_col = 2,	.grid_row = 0, 
	.col_span = 1,	.row_span = 1,
	.resImgNo = MMN_dictation - 1, .pTitleTxt = "听写"},	// 听写 

	{
	.grid_col = 0,	.grid_row = 2, 
	.col_span = 1,	.row_span = 1,
	.resImgNo = MMN_youdao - 1, .m_funcNo = C_ZB_AP_DICTIONARY,.pTitleTxt = "词典" },	// 词典 
	{
	.grid_col = 1,	.grid_row = 2, 
	.col_span = 1,	.row_span = 1,
	.resImgNo = MMN_my_resource - 1, .m_funcNo = MMN_MY_RES_MENU_FUNC_NO , .pTitleTxt = "我的资源" },	// my res 

     {
	.grid_col = 0,	.grid_row = 3,
	.col_span = 1,	.row_span = 1,
	.resImgNo = MMN_setting- 1, .m_funcNo = C_ZB_AP_SETTING, .pTitleTxt = "设置" },	// 
     {
	.grid_col = 2,	.grid_row = 2,
	.col_span = 1,	.row_span = 1,
	.resImgNo = MMN_inbox - 1,.m_funcNo = C_ZB_AP_CLOUD_DISK ,.pTitleTxt = "移动云盘" },	// C_ZB_AP_INBOX    .m_funcNo = C_ZB_AP_INBOX ,

};


#else   //06.14 入库版本
{
	.grid_col = 0, .grid_row = 0,
		.col_span = 1, .row_span = 1,
		.resImgNo = MMN_address_book - 1, .m_funcNo = C_ZB_AP_NO_CONTACT, .pTitleTxt = "通迅录"
},	// 支付宝 
	{
	.grid_col = 1,	.grid_row = 0,
	.col_span = 1,	.row_span = 1,
	.resImgNo = MMN_my_resource - 1, .m_funcNo = MMN_MY_RES_MENU_FUNC_NO , .pTitleTxt = "练习题" },	// 通迅录 
	{
	.grid_col = 2,	.grid_row = 0,
	.col_span = 1,	.row_span = 1,
	.resImgNo = MMN_setting - 1, .m_funcNo = C_ZB_AP_SETTING, .pTitleTxt = "设置" },	// 听写 
};
#endif

#if 0
	{
	.grid_col = 1,  .grid_row = 0,
	.col_span = 1,  .row_span = 2,
	.resImgNo = MMN_sys_upgrade - 1,  .m_funcNo = C_ZB_AP_SETTING, .m_apInParm = UPDATE_SETTING },  //  upgrade
	{
	.grid_col = 0,	.grid_row = 2,
	.col_span = 1,	.row_span = 2,
	.resImgNo = MMN_parent_pin - 1,  .m_funcNo = C_ZB_AP_PARENTAL_PIN }, // 家长锁 
	 {
	.grid_col = 1,	.grid_row = 2,
	.col_span = 1,	.row_span = 2,
	.resImgNo = MMN_app_store - 1, .m_funcNo = MMN_APP_STORE_MENU_FUNC_NO },  // 应用市场 
	{
	.grid_col = 0,	.grid_row = 4,
	.col_span = 1,	.row_span = 2,
	.resImgNo = MMN_time_mgmt - 1,   }, // 时间管理 
	{
	.grid_col = 1,	.grid_row = 4,
	.col_span = 1,	.row_span = 2,
	.resImgNo = MMN_recorder - 1,   }, // 录音机 
	{
	.grid_col = 0,  .grid_row = 6,
	.col_span = 1,  .row_span = 2,
	.resImgNo = MMN_wifi_cfg - 1, .m_funcNo = C_ZB_AP_SETTING, .m_apInParm = NETWORK_SETTING },  // wifi
	{
	.grid_col = 1,  .grid_row = 6,
	.col_span = 1,  .row_span = 2,
	.resImgNo = MMN_brightness - 1, .m_funcNo = C_ZB_AP_SETTING, .m_apInParm = BRIGHTNESS_SETTING },  // brightness
	{
	.grid_col = 0,  .grid_row = 8,
	.col_span = 1,  .row_span = 2,
	.resImgNo = MMN_auto_off - 1,  .m_funcNo = C_ZB_AP_SETTING, .m_apInParm = AUTO_ONOFF_SETTING },  // auto-off 
	{
	.grid_col = 1,  .grid_row = 8,
	.col_span = 1,  .row_span = 2,
	.resImgNo = MMN_disk_space - 1,  .m_funcNo = C_ZB_AP_SETTING, .m_apInParm = STORE_SPACE_SETTING },  //  disk space
	{
	.grid_col = 0,  .grid_row = 10,
	.col_span = 1,  .row_span = 2,
	.resImgNo = MMN_about_device - 1, .m_funcNo = C_ZB_AP_SETTING, .m_apInParm = MY_DEV_SETTING  },  // my device
#endif

static const lv_coord_t g_pgApp_col_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };

static const lv_coord_t g_pgApp_row_dsc[] = { MMNU_GRID_TOP_AID_ROW_H, 106, 8, 76, 140,
    MMNU_GRID_BOTTOM_AID_ROW_H, LV_GRID_TEMPLATE_LAST };

#if 0
const mnu_txt_item_t stgDrawItems_i[] = {
#define MY_INFO_LOC_X  16
#define MY_INFO_LOC_Y  172
	{ .m_loc.x = MY_INFO_LOC_X + 8,    .m_loc.y = MY_INFO_LOC_Y,
	  .m_pText = "支付宝",	 },
	{ .m_loc.x = MY_INFO_LOC_X + 109 + 8, .m_loc.y = MY_INFO_LOC_Y, 
	  .m_pText = "通讯录",	  },
	{ .m_loc.x = MY_INFO_LOC_X + 109*2 + 16, .m_loc.y = MY_INFO_LOC_Y, 
	  .m_pText = "听写",	 },

	{ .m_loc.x = MY_INFO_LOC_X + 0 + 16, .m_loc.y = MY_INFO_LOC_Y + 146, 
	  .m_pText = "词典",	},
	{ .m_loc.x = MY_INFO_LOC_X + 109, .m_loc.y = MY_INFO_LOC_Y + 146,
	  .m_pText = "我的资源",	 },
	{ .m_loc.x = MY_INFO_LOC_X + 109*2 + 16, .m_loc.y = MY_INFO_LOC_Y + 146,
	  .m_pText = "设置",  },
};
#endif

static const mnu_page_dsc_t g_pgApp_dsc_inf = {
	.itemTotal = M_ARRAY_LEN(g_pgApp_itmTbl),
	.gridTotal = 3 * 2,
	.colNumEachRow = 3,
	.pColDscTbl = g_pgApp_col_dsc, // 
	.pRowDscTbl = g_pgApp_row_dsc,
	.pItemInfTbl = g_pgApp_itmTbl,
	.exTextItemNum = 0,
	.pExTxtItemTbl = NULL,
};
//------------------------------
static const mnu_item_inf_t g_pgAI_itmTbl[] = {

	{
	.grid_col = 0,	.grid_row = 1,
	.col_span = 3,	.row_span = 3,
	.resImgNo = MMN_AI_kouyu - 1 },	// AI口语训练

	{
	.grid_col = 0,	.grid_row = 3,
	.col_span = 3,	.row_span = 1,
	.resImgNo = MMN_startbtn -1,.m_funcNo = C_ZB_AP_LISTENAI_CHAT,  } //开始对话
};

static const lv_coord_t g_pgAI_col_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };

static const lv_coord_t g_pgAI_row_dsc[] = { MMNU_GRID_TOP_AID_ROW_H,128, 126, 50,
	MMNU_GRID_BOTTOM_AID_ROW_H, LV_GRID_TEMPLATE_LAST };

static const mnu_page_dsc_t g_pgAI_dsc_inf = {
    .itemTotal = M_ARRAY_LEN(g_pgAI_itmTbl),
    .gridTotal = 3 * 2,
    .colNumEachRow = 3,
    .pColDscTbl = g_pgAI_col_dsc, // 
    .pRowDscTbl = g_pgAI_row_dsc,
    .pItemInfTbl = g_pgAI_itmTbl,
    .exTextItemNum = 0,
    .pExTxtItemTbl = NULL,
};
//------------------------------
static const mnu_item_inf_t g_pgKewaitl_itmTbl[] = { 
	{
	.grid_col = 0,  .grid_row = 0,
	.col_span = 2,  .row_span = 1,
	.resImgNo = MMN_childrens_song - 1, .m_funcNo = C_ZB_AP_ER_GE, .m_apInParm = DATUM_CHILD_SONG  },  // 儿歌  
	{
	.grid_col = 0,  .grid_row = 1,
	.col_span = 2,  .row_span = 1,
	.resImgNo = MMN_situational_dialogue - 1,.m_funcNo = C_ZB_AP_QING_JING, .m_apInParm = DATUM_SITUATIONAL_DIALOGUE},  // 情景对话 
	{
	.grid_col = 0,  .grid_row = 2,
	.col_span = 2,  .row_span = 1,
	.resImgNo = MMN_JianQiao - 1, .m_funcNo = C_ZB_AP_JIAN_QIAO  }, //
	{
	.grid_col = 0,  .grid_row = 3,
	.col_span = 2,  .row_span = 1,
	.resImgNo = MMN_WaiJiao - 1,.m_funcNo = C_ZB_AP_WAI_JIAO},    //
	{
	.grid_col = 0,  .grid_row = 4,
	.col_span = 2,  .row_span = 1,
	.resImgNo = MMN_YinYuMinZhu - 1,.m_funcNo = C_ZB_AP_YINYUMINZHU},    //
};

static const lv_coord_t g_pgKewaitl_row_dsc[] = { MMNU_GRID_TOP_AID_ROW_H , 100, 100 , 100 , 100 ,100, MMNU_GRID_BOTTOM_AID_ROW_H, LV_GRID_TEMPLATE_LAST };

static const mnu_page_dsc_t g_pgKewaitl_dsc_inf = {
    .itemTotal = M_ARRAY_LEN(g_pgKewaitl_itmTbl),
    .gridTotal = 2 * 4,
    .colNumEachRow = 2,
    .pRowDscTbl = g_pgKewaitl_row_dsc,
    .pItemInfTbl = g_pgKewaitl_itmTbl,
    .exTextItemNum = 0,
    .pExTxtItemTbl = NULL,
};

//------------------------------
static const mnu_item_inf_t g_pgGuoxue_itmTbl[] = { 
	{
	.grid_col = 0,  .grid_row = 0,
	.col_span = 2,  .row_span = 1,
	.resImgNo = MMN_guoxue_xunting - 1, .m_funcNo = C_ZB_AP_GUOXUEXUNTIN, .m_apInParm = DATUM_CHINESE_CLASSIC },  // 国学熏听 
	{
	.grid_col = 0,  .grid_row = 1,
	.col_span = 2,  .row_span = 1,
	.resImgNo = MMN_old_poem - 1, .m_funcNo = C_ZB_AP_GUSHISUITIN, .m_apInParm = DATUM_OLD_POEM },  // 古诗 
};

static const lv_coord_t g_pgGuoxue_row_dsc[] = { MMNU_GRID_TOP_AID_ROW_H,  100, 100, MMNU_GRID_BOTTOM_AID_ROW_H, LV_GRID_TEMPLATE_LAST };

static const mnu_page_dsc_t g_pgGuoxue_dsc_inf = {
    .itemTotal = M_ARRAY_LEN(g_pgGuoxue_itmTbl),
    .gridTotal = 2 * 2,
    .colNumEachRow = 2,
    .pRowDscTbl = g_pgGuoxue_row_dsc,
    .pItemInfTbl = g_pgGuoxue_itmTbl,
    .exTextItemNum = 0,
    .pExTxtItemTbl = NULL,
};
//------------------------------
static const mnu_item_inf_t g_pgMyRes_itmTbl[] = { 
	{
	.grid_col = 0,  .grid_row = 0,
	.col_span = 2,  .row_span = 1,
	.resImgNo = MMN_my_favourite - 1,  .m_funcNo = C_ZB_AP_COLLECT },  // 我的收藏  C_ZB_AP_COLLECT
	{
	.grid_col = 0,  .grid_row = 1,
	.col_span = 2,  .row_span = 1,
	.resImgNo = MMN_my_download - 1, .m_funcNo = C_ZB_AP_MY_DOWNLOAD,},  // 我的下载  .m_funcNo = C_ZB_AP_MY_DOWNLOAD,
};

static const lv_coord_t g_pgMyres_row_dsc[] = { MMNU_GRID_TOP_AID_ROW_H,  100, 102, MMNU_GRID_BOTTOM_AID_ROW_H, LV_GRID_TEMPLATE_LAST };

static const mnu_page_dsc_t g_pgMyres_dsc_inf = {
    .itemTotal = M_ARRAY_LEN(g_pgMyRes_itmTbl),
    .gridTotal = 2 * 2,
    .colNumEachRow = 2,
    .pRowDscTbl = g_pgMyres_row_dsc,
    .pItemInfTbl = g_pgMyRes_itmTbl,
    .exTextItemNum = 0,
    .pExTxtItemTbl = NULL,
};


//--------------------
static const mnu_page_dsc_t* g_pg_dsc_tbl[] = { 
    &g_pgEng_dsc_inf,
    &g_pgYuwen_dsc_inf,
    & g_pgAI_dsc_inf,
    &g_pgApp_dsc_inf,
    &g_pgKewaitl_dsc_inf,
    &g_pgGuoxue_dsc_inf,
    &g_pgMyres_dsc_inf,
};


//----------------------------------------- 
static const uint8_t g_acKeySeqForBackdoor[] = { LV_KEY_EX_FUNC1, LV_KEY_ESC, 0 };

#define MMN_BACKDOOR_AIM_PROD_TST 1
#define MMN_BACKDOOR_AIM_MISC_TST 2
#define MMN_BACKDOOR_AIM_SYNC_TIME 3
#define MMN_BACKDOOR_AIM_KEY_DRV_LOG_LVL 4
#define MMN_BACKDOOR_AIM_LCD_DONT_AUTO_OFF 5
#define MMN_BACKDOOR_AIM_ACT_A  6

static int32_t g_backdoorkeyTmSt;
static int16_t g_keyMatchN_forBDoor = 0; // 后门操作按键匹配数 
static int8_t  g_backdoorBranchNo;

//----------------------------------------- 
/*
 * @return : > BIND_BACKDOOR_AIM_PRODUCT_TST .. 要进测试AP 模式, <= 0 表示调用者不必处理 
 */
static int16_t keyProcForBackdoor(int16_t * pKeyMatchLen, int keyVal) { 
	int16_t iFuncRet = 0;
	int16_t keyMatchLen = (*pKeyMatchLen);
	bool bContinueMatch = false;

	if (keyMatchLen < 2) {
		if (keyVal == g_acKeySeqForBackdoor[keyMatchLen]) {
			keyMatchLen++;
			bContinueMatch = true;
			if (1 == keyMatchLen) {
				g_backdoorkeyTmSt = zb_get_time_ms_since_boot();
			}
		}
	}
	else if (2 == keyMatchLen) {
		if (LV_KEY_LEFT == keyVal) {
			iFuncRet = MMN_BACKDOOR_AIM_PROD_TST;
		}
		else if (LV_KEY_DOWN == keyVal || LV_KEY_UP == keyVal) {
			g_backdoorBranchNo = (LV_KEY_DOWN == keyVal ? 0 : 1);
			keyMatchLen++;
			bContinueMatch = true;
		}
	}
	else {  // when matchLen is 3
		if (0 == g_backdoorBranchNo) {  // when prs F1 - Esc - Down
			if (LV_KEY_UP == keyVal) {
				iFuncRet = MMN_BACKDOOR_AIM_KEY_DRV_LOG_LVL;
			}
			else if (LV_KEY_EX_RECORD == keyVal) {
				iFuncRet = MMN_BACKDOOR_AIM_MISC_TST;
			}
		}
		else {  // when prs F1 - Esc - Up 
			if (LV_KEY_ESC == keyVal) {
				iFuncRet = MMN_BACKDOOR_AIM_LCD_DONT_AUTO_OFF;
			}
			else if (LV_KEY_DOWN == keyVal) {
				iFuncRet = MMN_BACKDOOR_AIM_SYNC_TIME;
			}
			else if (LV_KEY_EX_RECORD == keyVal) {
				iFuncRet = MMN_BACKDOOR_AIM_ACT_A;
			}
		}
	}
	(*pKeyMatchLen) = (bContinueMatch ? keyMatchLen : 0);

	if (iFuncRet > 0) {
		int32_t timeNowMs = zb_get_time_ms_since_boot();
		if (timeNowMs >= g_backdoorkeyTmSt + 15000) { // the backKeys sustain too long time, ignore it.
			ESP_LOGI(TAG, "bDoor sustain too long.");
			iFuncRet = 0;
		}
	}
	if (!bContinueMatch) {
		g_backdoorkeyTmSt = 0;
	}
	return iFuncRet;
}

//----------------------------------------- 

/**
 *  output: g_gridNoToItemNoTbl[] 
 */
static void buildGridNoToItemNoTbl(const mnu_item_inf_t * pg_itmTbl, uint8_t itemTotal, uint8_t colNumEachRow, uint8_t gridTotal) {
	memset(g_gridNoToItemNoTbl, 0xff, sizeof(g_gridNoToItemNoTbl));
	for (int16_t itmNo = 0; itmNo < itemTotal; itmNo++) {
		uint8_t colNoSt  = pg_itmTbl[itmNo].grid_col;
		uint8_t rowNoSt  = pg_itmTbl[itmNo].grid_row;
		uint8_t col_span = pg_itmTbl[itmNo].col_span;
		uint8_t row_span = pg_itmTbl[itmNo].row_span;
		for (uint8_t rowNo = rowNoSt; rowNo < rowNoSt + row_span; rowNo++) {
			int16_t rowStartGrid = colNumEachRow * rowNo;
			if (colNumEachRow * (rowNo + row_span) == gridTotal) { // non-std,  (用于末尾行)把第2列的 itmNo 也当作第1列的 
				g_gridNoToItemNoTbl[rowStartGrid + 1] = (uint8_t)itmNo; 
			}
			for (uint8_t colNo = colNoSt; colNo < colNoSt + col_span; colNo++) {
				g_gridNoToItemNoTbl[rowStartGrid + colNo] = (uint8_t)itmNo;
			}
		}
	}
    for (int16_t gridNo = 0; gridNo < gridTotal; gridNo++) {
        if (0xff == g_gridNoToItemNoTbl[gridNo]) {
            g_gridNoToItemNoTbl[gridNo] = (gridNo + 1 < gridTotal ? g_gridNoToItemNoTbl[gridNo + 1] : 0);  // non-std 
        }
    }
}

static int8_t getSectionNoInY(const mnu_item_inf_t * pg_itmTbl, int16_t itemNo) { 
    if (itemNo < 0) return 0;
    uint8_t rowNo  = pg_itmTbl[itemNo].grid_row;
    if (rowNo <= 1)
        return 0;
    int8_t sectionNo =  rowNo / 4;
    return sectionNo;
}

/* static int16_t calcPageHeight(const mnu_page_dsc_t * p_pg_dsc) { 

    const lv_coord_t * p_rowDscTbl = p_pg_dsc->pRowDscTbl;
    int16_t idx = 0;
    int16_t heightSum = 0;
    for (; idx < 100; idx++) {
        if (p_rowDscTbl[idx] == LV_GRID_TEMPLATE_LAST) break;
        heightSum += p_rowDscTbl[idx];
    }
    return heightSum;
} */

/** 
 * @return  -1 nextPage, -3 : prevPage, >=0 new itemNo
 */
static int16_t  mmnu_dir_key_proc(const mnu_page_dsc_t * pgDscInf, int16_t itemNo, lv_dir_t dir) {
#if 1
	return 0;
#else
	if (!pgDscInf) return 0;
	if (itemNo < 0 || itemNo >= pgDscInf->itemTotal) {
		return 0;
	}

	int16_t gridNo = 0;
	uint8_t rowNo = pgDscInf->pItemInfTbl[itemNo].grid_row;
    uint8_t colNo = pgDscInf->pItemInfTbl[itemNo].grid_col;
    int8_t rowSpan = pgDscInf->pItemInfTbl[itemNo].row_span;
    int8_t colSpan = pgDscInf->pItemInfTbl[itemNo].col_span;
    int16_t itemNoNew = 0;
    int8_t rowTotal = (pgDscInf->gridTotal + pgDscInf->colNumEachRow - 1) / pgDscInf->colNumEachRow;
    if (dir == LV_DIR_LEFT) {
        if (0 == colNo) return C_MMNU_PREV_PG;
        gridNo = rowNo * pgDscInf->colNumEachRow + (colNo - 1);
        itemNoNew = g_gridNoToItemNoTbl[gridNo];
        if (itemNoNew == itemNo) return C_MMNU_PREV_PG; // non-std, 其右边格子是空的, 那就向左边翻页
    }
    else if (dir == LV_DIR_RIGHT) {
        if (colNo + colSpan >= pgDscInf->colNumEachRow) return C_MMNU_NEXT_PG;
        colNo += colSpan;
        gridNo = rowNo * pgDscInf->colNumEachRow + colNo;
        itemNoNew = g_gridNoToItemNoTbl[gridNo];
        if ((itemNoNew == itemNo) && (rowNo + rowSpan >= rowTotal)) { // for half of bottomRow 
            return C_MMNU_NEXT_PG;
        }
    }
    else if (dir == LV_DIR_TOP) {
        if (rowNo <= 0) {
        #if DOES_UP_DOWN_DIR_LOOP != 0
            gridNo = pgDscInf->colNumEachRow * (rowTotal - 1) + colNo;  // at top, then goto bottom 
        #else
            return C_MMNU_DONOT_DO;
        #endif
        }
        else {
            gridNo = pgDscInf->colNumEachRow * (rowNo - 1) + colNo;
        }
        itemNoNew = g_gridNoToItemNoTbl[gridNo];
    }
    else if (dir == LV_DIR_BOTTOM) {
        if (rowNo + rowSpan >= rowTotal) { // at bottom, then goto top
        #if DOES_UP_DOWN_DIR_LOOP != 0
            gridNo = 0;
        #else
            return C_MMNU_DONOT_DO;
        #endif
        }
        else {
            gridNo = pgDscInf->colNumEachRow * (rowNo + rowSpan) + colNo;
        }
        itemNoNew = g_gridNoToItemNoTbl[gridNo];
    }
    LV_ASSERT(itemNoNew != 0xff);
    return itemNoNew;
#endif
}

//------------------
static void _mmnu_draw_ex_cb(lv_event_t * e) {
#if 0
	if (g_scrNo < 0 || g_scrNo >= MMN_MENU_PG_TOTAL) {
        return;
    }

    lv_obj_t * obj = lv_event_get_target(e);
    lv_draw_ctx_t * draw_ctx = lv_event_get_draw_ctx(e);

    lv_area_t widget_coords;
    lv_obj_get_coords(obj, &widget_coords);
    lv_draw_label_dsc_t label_draw_dsc;
    lv_draw_label_dsc_init(&label_draw_dsc);
    //lv_obj_init_draw_label_dsc(obj, LV_PART_MAIN, &label_draw_dsc);
    lv_color_t colorTxt = LV_COLOR_MAKE(0, 0, 0);  // LV_COLOR_MAKE(0, 0xe0, 0);
    label_draw_dsc.color = colorTxt;
    label_draw_dsc.align = LV_TEXT_ALIGN_LEFT;

    const mnu_page_dsc_t * p_pg_dsc_tbl = g_pg_dsc_tbl[g_scrNo];
    int8_t txtNum = p_pg_dsc_tbl->exTextItemNum;
    int16_t scroll_y = lv_obj_get_scroll_y(obj);
    for (int8_t txtNo = 0; txtNo < txtNum; txtNo++) {
        const mnu_txt_item_t * exTxtItem = p_pg_dsc_tbl->pExTxtItemTbl + txtNo;
        lv_img_src_t src_type = lv_img_src_get_type(exTxtItem->m_pText); 
        if (LV_IMG_SRC_SYMBOL == src_type) {
            label_draw_dsc.ofs_x = exTxtItem->m_loc.x;
            label_draw_dsc.ofs_y = exTxtItem->m_loc.y - MMNU_STATUS_BAR_H - scroll_y; // todo
            lv_draw_label(draw_ctx, &label_draw_dsc, &widget_coords, exTxtItem->m_pText, NULL);
        }
        else { // is img type. 
            lv_area_t icn_coords = widget_coords;
            lv_draw_img_dsc_t dsc;
            lv_draw_img_dsc_init(&dsc);
            const lv_img_dsc_t  * pImgDsc = (lv_img_dsc_t *)exTxtItem->m_pText;
            dsc.opa = LV_OPA_100;
            dsc.blend_mode = LV_BLEND_MODE_NORMAL;  
            icn_coords.x1 = exTxtItem->m_loc.x;
            icn_coords.y1 = exTxtItem->m_loc.y - scroll_y;
            icn_coords.x2 = icn_coords.x1 + pImgDsc->header.w - 1;
            icn_coords.y2 = icn_coords.y1 + pImgDsc->header.h - 1;
            lv_draw_img(draw_ctx, &dsc, &icn_coords,  pImgDsc); 
        }
    }
#endif
}

// 检查当前是否在主菜单（一级菜单）
bool is_main_menu_active(void) {
	return MMN_IS_1ST_LEVEL_MENU(g_scrNo);
}


static bool isThereNewVersionInUpgCfg()
{
    return false;
}

//------------------
// 电量电压对照表 (满电部分)
// Note: 该表首项对就 99% 电量, 末项对 85% 电量 (mV);
static const int16_t g_battery_topLevelsTbl[] = {
	4120, 4098, 4089, 4080, 4064, 4049, 4033, 4027, 4021, 4011, 4003, 3996, 3992, 3989, 3983 };

// Note: 获取电量级别, 输出 0 ~ 100 
int16_t mmn_get_vbat_level(bool doesLog, int16_t batVal) {
#define MMN_BAT_TOP_THREAD_PCT  (100 - 1 - M_ARRAY_LEN(g_battery_topLevelsTbl))
	if (doesLog) {
		ESP_LOGI(TAG, "mmn get bat %d", (int)batVal);
	}
	if (batVal <= C_MMN_BAT_VAL_MIN) {
		return 0;
	}
	else if (batVal >= C_MMN_BAT_VAL_MAX) {
		return 100;
	}

	int level = 0;
	if (batVal < C_MMN_BAT_VAL_TOP_STAGE) {
		level = ((batVal - C_MMN_BAT_VAL_MIN) * MMN_BAT_TOP_THREAD_PCT) / (C_MMN_BAT_VAL_TOP_STAGE - C_MMN_BAT_VAL_MIN);
	}
	else {
		int sub = 0;
		for (; sub < M_ARRAY_LEN(g_battery_topLevelsTbl); sub++) {
			if (batVal >= g_battery_topLevelsTbl[sub])
				break;
		}
		level = 100 - 1 - sub;
	}
	return level;
}


/*
 * @return : bit0 == 1 means battery change, bit1 == 1 means wifi change, bit2 == 1 means  time change
 */
static int _mmn_update_statbar_parm(bool hasLog) {
	int8_t  updStatFlag = 0;
#ifndef C_PC_SIM
	int batLevelPct = 0, chargeStat = 0;
	int getBatRet = tz_get_battery_info(&batLevelPct, &chargeStat);
	if (hasLog) {
		ESP_LOGI(TAG, "getBatR %d, %d", getBatRet, batLevelPct);
	}

	int8_t batLevel = 0;
	if (batLevelPct >= 90) {
		batLevel = 4;
	}
	else if (batLevelPct >= 60) {
		batLevel = 3;
	}
	else if (batLevelPct >= 40) {
		batLevel = 2;
	}
	else if (batLevelPct >= 20) {
		batLevel = 1;
	}
	if (chargeStat) {
		batLevel |= C_RECHARGE_FLAG;
	}
	g_batteryLevelNoPct = batLevelPct;
	if (batLevel != g_batteryLevelNo) {
		g_batteryLevelNo = batLevel;
		updStatFlag |= 1;
	}
	//--------------
	int8_t rssiLevel = -1;
	tz_wifi_status_t stWifiStat = { 0 };
	int wifiRet = tz_get_wifi_status(&stWifiStat);
	if (hasLog) {
		ESP_LOGI(TAG, "getWifi %d  %s, isOn:%d, link:%d", wifiRet, stWifiStat.ssid, (int)stWifiStat.on, (int)stWifiStat.link);
	}
	int wifiStat = (stWifiStat.link ? 1 : 0);
	if (wifiStat) { // wifi is link
		rssiLevel = 0;
		int rssiVal = stWifiStat.signal_qua;
		bool bNetConnect = stWifiStat.link;

		if (rssiVal < 1) {
			if (hasLog) {
				ESP_LOGE(TAG, "fail to rd rssi %d", rssiVal);
			}
		} else {
			if (hasLog) {
				ESP_LOGI(TAG, "rd rssi ret %d", rssiVal);
			}
			if (rssiVal >= 90) {
				rssiLevel = 4;
			} else if (rssiVal >= 70) {
				rssiLevel = 3;
			} else if (rssiVal >= 50) {
				rssiLevel = 2;
			} else if (rssiVal >= 30) {
				rssiLevel = 1;
			} else {
				rssiLevel = 0;
			}
		}
	}
	else {
		if (hasLog) {
			ESP_LOGI(TAG, "wifi is off.");
		}
	}
	if (rssiLevel != g_wifiLevelNo) {
		g_wifiLevelNo = rssiLevel;
		updStatFlag |= 2;
	}
#else
	g_batteryLevelNo = (3 | C_RECHARGE_FLAG);
	g_wifiLevelNo = 3;
#endif
	//---------------  upd time disp.
	time_t now;
	struct tm timeinfo;
	time(&now);
#ifndef C_PC_SIM
	setenv("TZ", "CST-8", 1);  // 将时区设置为中国标准时间
	tzset();
	localtime_r(&now, &timeinfo);
#else
	/* struct tm * pTmInfSrc = localtime(&now);
	timeinfo = (*pTmInfSrc); */

	localtime_s(&timeinfo, &now);
#endif
	int16_t tiemHM = ((timeinfo.tm_hour << 8) | timeinfo.tm_min);
	//ESP_LOGI(TAG, "timeA~~ HM 0x%hx", tiemHM);
	if (tiemHM != g_curTimeHM) {
		g_curTimeHM = tiemHM;
		updStatFlag |= 4;
	}
	//--------------- 
	if (updStatFlag) { 
		if (hasLog) {
			ESP_LOGI(TAG, "updStatBar %hx", (int16_t)updStatFlag);
		}
	}

	return updStatFlag;
}


static void _mmn_timer_cb(struct _lv_timer_t *tmr) {
	// ESP_LOGI(TAG, "tmr A %d", (int)g_scrNo);

	g_subCntInTmr++;
	if (g_subCntInTmr >= 2) {
		g_subCntInTmr = 0;
	}
	if (!(MMN_IS_1ST_LEVEL_MENU(g_scrNo) ||  MMN_IS_2ND_LEVEL_MENU(g_scrNo) || 
			(MMN_SCR_NO_PROMPT_FOR_NEW_USER == g_scrNo)) ) {
		return;
	}

	int updFlag = _mmn_update_statbar_parm(false);
	if (MMN_SCR_NO_PROMPT_FOR_NEW_USER == g_scrNo) {
		if ((updFlag & 0x05) != 0) {
			mmn_promptForNewUsr_scr_invalid();
		}
	}
	else { // when is on level 1、2 menu;
		if (updFlag) {
			if (g_statBar) {
				lv_obj_invalidate(g_statBar);
			}
		}

#define MMN_DLOAD_UPG_CFG_TIME_INTERVAL_MINUTE  (60 * 24 * 4)

#define MMN_PROMPT_UPGRADE_TIME_INTERVAL_MINUTE  (60 * 36)  // debug


	}
	return;
}
void _mmn_update_statbar_ref(){
	if(g_statBar) {
        lv_obj_invalidate(g_statBar);
    }
}
void _mmn_update_statbar_bat(bool isCharging,int power) {
	_isCharging = isCharging;
	_power = power;
	if(g_statBar == NULL) return;
}

void _mmn_update_statbar_wifi(bool isConnected,int power) {
	_isWifiConnected = isConnected;
	_wifiSignalStrength = power;
}
void _mmn_update_statbar_sim_state(int state) {
	_simState = state;
}
void _mmn_update_statbar_sim_power(int power) {
	_simPower = power;
}

// 更新课堂模式
void _mmn_update_statbar_class_mode(int mode) {
	_classMode = mode;
}

//更新响铃状态
void _mmn_update_statbar_mute_state(int mode) {
	_muteMode = mode;
}

//更新成研状态
void _mmn_update_statbar_cy_state(int mode) {
	_cy_conNect = mode;
}



static void _mmnu_draw_statbar_cb(lv_event_t * e) {
#define MMN_RECHARGE_ICN_W  18
    lv_obj_t * obj = lv_event_get_target(e);
    lv_draw_ctx_t * draw_ctx = lv_event_get_draw_ctx(e);

    lv_area_t widget_coords;
    lv_obj_get_coords(obj, &widget_coords);
	{
#ifdef C_PC_SIM
		_power = 4;
		_isCharging = 1;
#endif
		const lv_img_dsc_t  * pImgDscBat;
		if(_isCharging) {
			pImgDscBat = g_battery_charging_tlb[_power];
		}else{
			pImgDscBat = g_battery_L_tlb[_power];
		}
		int16_t iconW = pImgDscBat->header.w;
		lv_area_t icn_coords = widget_coords;
		icn_coords.x1 = SCREEN_WIDTH - 50;
		icn_coords.x2 = icn_coords.x1 + iconW - 1;
		icn_coords.y1 = 4 + 5;
		icn_coords.y2 = icn_coords.y1 + pImgDscBat->header.h - 1;
		lv_draw_img_dsc_t dsc;
		lv_draw_img_dsc_init(&dsc);
		dsc.opa = LV_OPA_100;
		dsc.blend_mode = LV_BLEND_MODE_NORMAL;  // LV_BLEND_MODE_NORMAL;
		lv_draw_img(draw_ctx, &dsc, &icn_coords,  pImgDscBat);	
	}
	{
		lv_area_t icn_coords = widget_coords;
		lv_draw_img_dsc_t dsc;
#ifndef C_PC_SIM
		if (_isWifiConnected) {
#else
		if (1) {
			_wifiSignalStrength = 6;
#endif
			const lv_img_dsc_t  * pImgDscWifi = g_wifi_tlb[_wifiSignalStrength/2 - 1];
			int16_t iconW = pImgDscWifi->header.w;
			icn_coords = widget_coords;
			icn_coords.x1 = SCREEN_WIDTH - 80;
			icn_coords.y1 = 4;
			icn_coords.x2 = icn_coords.x1 + iconW - 1;
			icn_coords.y2 = icn_coords.y1 + pImgDscWifi->header.h - 1;
			lv_draw_img_dsc_init(&dsc);
			dsc.opa = LV_OPA_100;
			dsc.blend_mode = LV_BLEND_MODE_NORMAL;  // 
			lv_draw_img(draw_ctx, &dsc, &icn_coords,  pImgDscWifi);  // 
		}
	}

	//是否响铃
	{
		lv_area_t icn_coords = widget_coords;
		lv_draw_img_dsc_t dsc;
		if (_muteMode == 1) { 
			const lv_img_dsc_t* pImgDscsignal = g_XXX_img_dsc_tbl + mute - 1;
			int16_t iconW = pImgDscsignal->header.w;
			icn_coords = widget_coords;
			icn_coords.x1 = 185;
			icn_coords.y1 = 7;
			icn_coords.x2 = icn_coords.x1 + iconW - 1;
			icn_coords.y2 = icn_coords.y1 + pImgDscsignal->header.h - 1;
			lv_draw_img_dsc_init(&dsc);
			dsc.opa = LV_OPA_100;
			dsc.blend_mode = LV_BLEND_MODE_NORMAL;  // 
			lv_draw_img(draw_ctx, &dsc, &icn_coords, pImgDscsignal);
		}
		else
		{
			const lv_img_dsc_t* pImgDscsignal = g_XXX_img_dsc_tbl + no_mute - 1;
			int16_t iconW = pImgDscsignal->header.w;
			icn_coords = widget_coords;
			icn_coords.x1 = 185;
			icn_coords.y1 = 7;
			icn_coords.x2 = icn_coords.x1 + iconW - 1;
			icn_coords.y2 = icn_coords.y1 + pImgDscsignal->header.h - 1;
			lv_draw_img_dsc_init(&dsc);
			dsc.opa = LV_OPA_100;
			dsc.blend_mode = LV_BLEND_MODE_NORMAL;  // 
			lv_draw_img(draw_ctx, &dsc, &icn_coords, pImgDscsignal);
		}
	}

#if 0
	{
		if (1) {  //是否蓝牙
			lv_area_t icn_coords = widget_coords;
			lv_draw_img_dsc_t dsc;
			const lv_img_dsc_t* pImgDscsignal = g_XXX_img_dsc_tbl + no_mute - 1;
			int16_t iconW = pImgDscsignal->header.w;
			icn_coords = widget_coords;
			icn_coords.x1 = 205;
			icn_coords.y1 = 7;
			icn_coords.x2 = icn_coords.x1 + iconW - 1;
			icn_coords.y2 = icn_coords.y1 + pImgDscsignal->header.h - 1;
			lv_draw_img_dsc_init(&dsc);
			dsc.opa = LV_OPA_100;
			dsc.blend_mode = LV_BLEND_MODE_NORMAL;  // 
			lv_draw_img(draw_ctx, &dsc, &icn_coords, pImgDscsignal);
		}
	}
#endif


	 //  成研连接
	{
		if (_cy_conNect == 1) {
			lv_area_t icn_coords = widget_coords;
			lv_draw_img_dsc_t dsc;
			const lv_img_dsc_t* pImgDscsignal = g_XXX_img_dsc_tbl + cy_cnet - 1;
			int16_t iconW = pImgDscsignal->header.w;
			icn_coords = widget_coords;
			icn_coords.x1 = 80;
			icn_coords.y1 = 7;
			icn_coords.x2 = icn_coords.x1 + iconW - 1;
			icn_coords.y2 = icn_coords.y1 + pImgDscsignal->header.h - 1;
			lv_draw_img_dsc_init(&dsc);
			dsc.opa = LV_OPA_100;
			dsc.blend_mode = LV_BLEND_MODE_NORMAL;  // 
			lv_draw_img(draw_ctx, &dsc, &icn_coords, pImgDscsignal);
		}
	}
	
// ================== 课堂模式 ================== // 
	{
		if (_classMode == 1) {
			lv_draw_label_dsc_t label_draw_dsc;
			lv_draw_label_dsc_init(&label_draw_dsc);
#if DARK_MODE
			lv_color_t colorTxt = LV_COLOR_MAKE(0xff, 0xff, 0xff);
#else
			lv_color_t colorTxt = LV_COLOR_MAKE(0, 0, 0);  // 黑色
#endif
			label_draw_dsc.color = colorTxt;
			label_draw_dsc.align = LV_TEXT_ALIGN_LEFT;

#ifndef C_PC_SIM
			//lv_ft_info_t info = generate_font_info(17, FT_FONT_STYLE_NORMAL);
		//	label_draw_dsc.font = info.font;
#endif
			label_draw_dsc.ofs_x = 100;
			label_draw_dsc.ofs_y = 5;


			char text1[20] = "课堂模式";
			lv_draw_label(draw_ctx, &label_draw_dsc, &widget_coords, text1, NULL);
		}
	}

// =============================================== //
	{
		lv_area_t icn_coords = widget_coords;
		lv_draw_img_dsc_t dsc;
		if(_simState != SIM_OUT){
			const lv_img_dsc_t  * pImgDscsignal = g_signal_tlb[_simPower];
			int16_t iconW = pImgDscsignal->header.w;
			icn_coords = widget_coords;
			if(_isWifiConnected){
				icn_coords.x1 = SCREEN_WIDTH - 117;
			}else{
				icn_coords.x1 = SCREEN_WIDTH - 80;
			}
			icn_coords.y1 = -3 + 8;
			icn_coords.x2 = icn_coords.x1 + iconW - 1;
			icn_coords.y2 = icn_coords.y1 + pImgDscsignal->header.h - 1;
			lv_draw_img_dsc_init(&dsc);
			dsc.opa = LV_OPA_100;
			dsc.blend_mode = LV_BLEND_MODE_NORMAL;  // 
			lv_draw_img(draw_ctx, &dsc, &icn_coords,  pImgDscsignal);  // 



//			lv_draw_label_dsc_t label_draw_dsc;
//			lv_draw_label_dsc_init(&label_draw_dsc);
//#if DARK_MODE
//			lv_color_t colorTxt = LV_COLOR_MAKE(0xff,0xff,0xff);
//#else
//			lv_color_t colorTxt = LV_COLOR_MAKE(0, 0, 0);  // 黑色
//#endif
//			label_draw_dsc.color = colorTxt;
//			label_draw_dsc.align = LV_TEXT_ALIGN_RIGHT;
//
//			lv_draw_label_dsc_t label_draw_dsc1;
//			lv_draw_label_dsc_init(&label_draw_dsc1);
//			label_draw_dsc1.color = colorTxt;
//			label_draw_dsc1.align = LV_TEXT_ALIGN_RIGHT;
//
//#ifndef C_PC_SIM
//			lv_ft_info_t info = generate_font_info(17, FT_FONT_STYLE_NORMAL);
//			label_draw_dsc1.font = info.font;	
//			label_draw_dsc.font = info.font;
//#endif
//			label_draw_dsc1.ofs_x = -150;  
//			label_draw_dsc1.ofs_y = 5;
//
//			label_draw_dsc.ofs_x = -125; 
//			label_draw_dsc.ofs_y = 5;
//
//			
//			char text1[20] = "4G";  
//			lv_draw_label(draw_ctx, &label_draw_dsc, &widget_coords, text1, NULL);
//
//			
//			char text2[20] = "HD"; 
//			lv_draw_label(draw_ctx, &label_draw_dsc1, &widget_coords, text2, NULL);

		}else{
			const lv_img_dsc_t  * pImgDscsimDis = &simDis;
			int16_t iconW = pImgDscsimDis->header.w;
			icn_coords = widget_coords;
			icn_coords.x1 = 57;
			icn_coords.y1 = 2;
			icn_coords.x2 = icn_coords.x1 + iconW - 1;
			icn_coords.y2 = icn_coords.y1 + pImgDscsimDis->header.h - 1;
			lv_draw_img_dsc_init(&dsc);
			dsc.opa = LV_OPA_100;
			dsc.blend_mode = LV_BLEND_MODE_NORMAL;  // 
			lv_draw_img(draw_ctx, &dsc, &icn_coords,  pImgDscsimDis);  //
		}
	}
	// _mmn_draw_sim_indicator(e);
	// _mmn_draw_wifi_indicator(e);
	// _mmn_draw_battery_indicator(e);

    //----------------------
    if (g_curTimeHM != -1) {
        lv_draw_label_dsc_t label_draw_dsc;
        lv_draw_label_dsc_init(&label_draw_dsc);
#if !DARK_MODE
        lv_color_t colorTxt = LV_COLOR_MAKE(0, 0, 0);  // LV_COLOR_MAKE(0, 0xe0, 0);
#else
		lv_color_t colorTxt = LV_COLOR_MAKE(0xFF, 0xFF, 0xFF);
#endif
        label_draw_dsc.color = colorTxt;
        label_draw_dsc.align = LV_TEXT_ALIGN_LEFT;

#ifndef C_PC_SIM
		lv_ft_info_t info = generate_font_info(17, FT_FONT_STYLE_NORMAL);
        label_draw_dsc.ofs_x = 12;
        label_draw_dsc.ofs_y = 4;
        label_draw_dsc.font = info.font;
#endif
        char acTmText[20];
        snprintf(acTmText, sizeof(acTmText), "%02hd:%02hd", (int16_t)((g_curTimeHM>>8)&0x3f), (int16_t)(g_curTimeHM&0x3f));
        lv_draw_label(draw_ctx, &label_draw_dsc, &widget_coords, acTmText, NULL);
    }
    //----------------------
#if 0
	if (MMN_IS_1ST_LEVEL_MENU(g_scrNo))  // display indicate bar.
	{
		lv_area_t icn_coords = { 0 };
		lv_draw_img_dsc_t dsc;
		lv_draw_img_dsc_init(&dsc);
		const lv_img_dsc_t	* pImgDscBar = g_XXX_img_dsc_tbl + MMN_indicate_bar - 1;
		const lv_img_dsc_t	* pImgDscDot = g_XXX_img_dsc_tbl + MMN_indicate_dot - 1;

		lv_coord_t x1 = 94;
		for (int16_t sub = 0; sub < MMN_1ST_LEVEL_PG_TOTAL; sub++) {
			int16_t subWidth = (sub == g_scrNo ? 11 : 6);
			const lv_img_dsc_t * pImgDscOne = (sub == g_scrNo ? pImgDscBar : pImgDscDot);
			dsc.opa = LV_OPA_100;
			dsc.blend_mode = LV_BLEND_MODE_NORMAL;	
			icn_coords.x1 = x1;
			icn_coords.y1 = MMNU_STATUS_BAR_THICK_H - 8 - 6;
			icn_coords.x2 = x1 + subWidth - 1;
			icn_coords.y2 = MMNU_STATUS_BAR_THICK_H - 8 - 1;
			lv_draw_img(draw_ctx, &dsc, &icn_coords,  pImgDscOne); 
			x1 += subWidth + 3;
		}
	}
#endif
    return;
}

#ifdef C_PC_SIM
bool g_isSdcardReady = true;
#endif

#define IS_AP_RELY_ON_SERIALNO(apNo)  ((C_ZB_AP_SETTING != apNo) && (C_ZB_AP_PRODUCT_TST != apNo))

#define IS_AP_RELY_ON_SDCARD(apNo)  ((C_ZB_AP_SETTING != apNo) && (C_ZB_AP_PRODUCT_TST != apNo))

static void _mmnu_selectItemProc() {
    const uint64_t  g_apNeedWifiMask = ((1 << C_ZB_AP_SEARCH)); // 

    const mnu_page_dsc_t * p_pg_dsc_tbl = g_pg_dsc_tbl[g_scrNo];
    int16_t itemNo = g_itemNo;
    uint8_t funcNo = p_pg_dsc_tbl->pItemInfTbl[itemNo].m_funcNo;
    uint16_t apInParm = p_pg_dsc_tbl->pItemInfTbl[itemNo].m_apInParm;

    if (0 == funcNo) {
        int64_t tmNow = zb_get_time_ms_since_boot();
        ESP_LOGI(TAG, "tmNowMs %lld, sec: %d", tmNow, (int)(tmNow/1000));
    }

    if (funcNo >= MMN_2ND_LEVEL_MENU_FUNC_NO_ST && funcNo < MMN_2ND_LEVEL_MENU_FUNC_NO_ST + MMN_2ND_LEVEL_PG_N) { // jump to 2nd Level menu.
        int8_t scrNoNew = (int8_t)(funcNo - MMN_2ND_LEVEL_MENU_FUNC_NO_ST + MMN_2ND_LEVEL_SCR_NO_ST);
        ESP_LOGI(TAG, "nx, will jmp to 2ndMmn, pg%d", scrNoNew);
		
        g_1stLevelMnuScrNo = g_scrNo;
        g_1stLevelMnuSelNo = itemNo;
        main_menu_pg_init(scrNoNew, true);
        return;
    }
    else if (funcNo >= 1) 
{
    //#ifndef C_PC_SIM

		LV_LOG_USER("执行AP切换 funcNo=%d, apInParm=0x%04X", funcNo, apInParm);
		if (funcNo == C_ZB_AP_AUD_PLAYER) {
			LV_LOG_USER("进入音频播放器，参数类型：%d", apInParm);
		}
    #if 0
        if (!g_isSerialNoOk && IS_AP_RELY_ON_SERIALNO(funcNo)) {
            char acDevSn[ZB_PROD_SN_LEN + 1] = { 0 };
            int ret = zb_readProductSerialNo(acDevSn, sizeof(acDevSn));
            if ((ret < 0) || (strlen(acDevSn) < ZB_PROD_SN_LEN)) {
                ESP_LOGW(TAG, "fail to get sn, %d", ret);
                app_msgbox_timeout("注意", "设备未刷系列号!", 3000, NULL, NULL);
                return;
            }
            else if (ret > 0) {
                g_isSerialNoOk = 1;
            }
        }
    #endif

    #ifndef C_PC_SIM
        if (!g_hasPromptDiskSpc && !is_app_msgbox_shown(1)) {
            uint64_t total_space = 0LL, free_space = 0LL;
#if 0
            if (esp32s3_get_sdcard_capacity(&total_space, &free_space) == 0) {
        #define DISK_FREE_MIN_KB_ALLOW  (100 * 1024) 
                ESP_LOGI(TAG, "disk ttl:%lld, free%lld kB", total_space, free_space);
                int minnFreeSpaceKB = (total_space * 1) / 250;  //  calc 0.4 % of whole disk.
                if (minnFreeSpaceKB < DISK_FREE_MIN_KB_ALLOW) {
                    minnFreeSpaceKB = DISK_FREE_MIN_KB_ALLOW;
                }
                if (free_space < minnFreeSpaceKB) {
                    g_hasPromptDiskSpc = true;
                    ESP_LOGE(TAG, "disk freeSpace too small!");
                    app_msgbox_timeout("注意", "TF卡剩余空间不足,\n请及时清理空间!", 3000, NULL, NULL);
                    return;
                }
            }
            else {
                ESP_LOGE(TAG, "fail to get diskCapacity!");
            }
#endif
        }
        if ((funcNo < 8 * sizeof(g_apNeedWifiMask)) && ((1 << funcNo) & g_apNeedWifiMask)) {
            if (!zb_is_wifi_connect()) {
                main_menu_pg_init(MMN_SCR_NO_PROMPT_NET_CFG, false);
                return;
            }
        }
    #endif

    #if 0
        if (C_ZB_AP_XIMALAYA == funcNo) {
            if (!prp_isNowPasswordConvenient()) {
                uint32_t hashVal = prp_pwdHash_read();
                if (hashVal != 0) {
                    main_menu_pg_init(MMN_SCR_NO_PWD_LOGIN, false);
                    return;
                }
            }
        }
    #endif
#if 0
		if (funcNo == 8)
		{
			//zb_requestSwitchApIn(funcNo, 1);
			mydownload_page_init(C_ZB_AP_MY_DOWNLOAD, 0);
			LV_LOG_USER("enter my download\r\n");
		}
		else
		{
			zb_requestSwitchApIn(funcNo, apInParm);
		}
#else
        zb_requestSwitchApIn(funcNo, apInParm);
#endif
    }
	else
	{
	   LV_LOG_USER("NO FUNC\r\n");
	   create_dialog_with_timer(false, "正在开发中...", 2000, 0, 0);
		
	}

    return;
}


//----------------------------------------------
static void _mmn_item_btn_click_cb(lv_event_t * e)
{
	if (e->code != LV_EVENT_SHORT_CLICKED)
		return;
	lv_obj_t* obj = lv_event_get_target(e);
	
	int32_t userData = (int32_t)lv_obj_get_event_user_data(obj, _mmn_item_btn_click_cb); 
	ESP_LOGI(TAG, "itm %ld clcked", userData);
	int16_t itemNoPrv = g_itemNo;
	if ((itemNoPrv >= 0) && (itemNoPrv != userData)) {  // todo, remove check stat of prev btn.
		const mnu_page_dsc_t * p_pg_dsc_tbl = g_pg_dsc_tbl[g_scrNo];
		//lv_obj_clear_state(g_page_btns[itemNoPrv], LV_STATE_CHECKED);
	}
	g_itemNo = userData;
	_mmnu_selectItemProc();
	return;
}

static void _mmn_topBtns_sel(int16_t pageNo)
{
	for (int pg = 0; pg < MMN_1ST_LEVEL_PG_TOTAL; pg++) {
		lv_obj_t * objItem = g_level1TopBtns[pg];
		lv_obj_t* lineshow = g_indicatorBars[pg];
		if (pg == pageNo) {
			lv_obj_add_state(objItem, LV_STATE_FOCUS_KEY);
			lv_obj_clear_flag(lineshow, LV_OBJ_FLAG_HIDDEN);
			
		}
		else {
			lv_obj_clear_state(objItem, LV_STATE_FOCUS_KEY);
			lv_obj_add_flag(lineshow, LV_OBJ_FLAG_HIDDEN);
		}
	}
	return;
}

static void _tabvw_valChg_EvtCb(lv_event_t * evt)
{
	lv_event_code_t evtCode = evt->code;
	if (LV_EVENT_VALUE_CHANGED == evtCode) {
		int16_t pgNo = lv_tabview_get_tab_act(g_winA);
		g_scrNo = pgNo;
		_mmn_topBtns_sel(pgNo);
	}
	return;
}

static void _page_WinEvtCb(lv_event_t * evt) {
#if 1
    lv_event_code_t evtCode = evt->code;

    int keyVal = 0;
    switch (evtCode) {
        /* case LV_EVENT_KEY_STAGE:
            keyVal = lv_event_get_key(evt);
            if ((LV_EX_FLAG_KEY_MASK & keyVal) != LV_EX_FLAG_KEY_REPEAT) {
                return;
            }
            keyVal &= ~LV_EX_FLAG_KEY_MASK;
            // fallthrough here.
           */
        case LV_EVENT_KEY_STAGE: {
            keyVal = lv_event_get_key(evt);
            ESP_LOGI(TAG, "mmnu key A %x", keyVal);
            if ((LV_EX_FLAG_KEY_MASK & keyVal) != LV_EX_FLAG_KEY_SHORT) {
                return;
            }
            keyVal &= ~LV_EX_FLAG_KEY_MASK;
            ESP_LOGI(TAG, "mmnu key B %x", keyVal);
            if (LV_KEY_EX_RECORD == keyVal) {
                if ((g_scrNo >= 0 && g_scrNo < MMN_MENU_PG_TOTAL) && (0 == g_keyMatchN_forBDoor)) {
                    zb_requestSwitchAp(C_ZB_AP_SEARCH);
                    return;
                }
            }
            else if (LV_KEY_ESC == keyVal) {
                ESP_LOGI(TAG, "key esc A");
                if (MMN_IS_2ND_LEVEL_MENU(g_scrNo)) {
                    int8_t scrNoNew = g_1stLevelMnuScrNo;
                    ESP_LOGI(TAG, "nx, will ret pg%d", scrNoNew);
                    lv_indev_keypad_wait_release();
                    g_itemNo = g_1stLevelMnuSelNo;
                    main_menu_pg_init(scrNoNew, false);
                    return;
                }
            }
            if (MMN_1ST_LEVEL_PG_TOTAL - 1 == g_scrNo) { // 在最末屏, 支持 backdoor 
                int backDoorRet = keyProcForBackdoor(&g_keyMatchN_forBDoor, keyVal);
                if (MMN_BACKDOOR_AIM_PROD_TST == backDoorRet) {
                    zb_requestSwitchAp(C_ZB_AP_PRODUCT_TST);
                    return;
                }
                else if (MMN_BACKDOOR_AIM_MISC_TST == backDoorRet) {
                    return;
                }
                else if (MMN_BACKDOOR_AIM_ACT_A == backDoorRet) {
                    return;
                }
                else if (MMN_BACKDOOR_AIM_LCD_DONT_AUTO_OFF == backDoorRet) {
                    zb_set_lcdDontAutoOffMode(16127);
                    app_msgbox_timeout("", "自动息屏功能已关闭(演示用)", 2000|MSGBOX_TOAST_FLAG, NULL, NULL);
                }
                else if (MMN_BACKDOOR_AIM_SYNC_TIME == backDoorRet) {
                }
            }

            int16_t itemNo = g_itemNo;
            int8_t moveDir = LV_DIR_NONE;
            switch (keyVal) {
                case LV_KEY_LEFT:
                    moveDir = LV_DIR_LEFT;
                    break;
                case LV_KEY_RIGHT:
                    moveDir = LV_DIR_RIGHT;
                    break;
                case LV_KEY_UP:
                    moveDir = LV_DIR_TOP;
                    break;
                case LV_KEY_DOWN:
                    moveDir = LV_DIR_BOTTOM;
                    break;
#if 0
                case LV_KEY_ENTER:
                    lv_indev_keypad_wait_release();
                    _mmnu_selectItemProc();
#endif
                    break;
                case LV_KEY_EX_FUNC1:
                    break;
                default:
                    break;
            }
            if (moveDir != LV_DIR_NONE) {
                const mnu_page_dsc_t * p_pg_dsc_tbl = g_pg_dsc_tbl[g_scrNo];
                int16_t itemNoNew = mmnu_dir_key_proc(p_pg_dsc_tbl, itemNo, moveDir);
                if (itemNoNew >= 0) { // change cur item
                    lv_obj_clear_state(g_page_btns[itemNo], LV_STATE_CHECKED);
                    int8_t sectNo = getSectionNoInY(p_pg_dsc_tbl->pItemInfTbl, itemNoNew);
                    g_itemNo = itemNoNew;
                    lv_obj_add_state(g_page_btns[itemNoNew], LV_STATE_CHECKED);
                    lv_obj_scroll_to_y(g_winA, g_sectLocY_tbl[sectNo], LV_ANIM_OFF);  // 
                    lv_event_stop_processing(evt);
                }
                else if (C_MMNU_NEXT_PG == itemNoNew) {
                    if (!MMN_IS_1ST_LEVEL_MENU(g_scrNo))
                        break;
                    int16_t scrNoNew = (g_scrNo + 1) % MMN_1ST_LEVEL_PG_TOTAL;
                    ESP_LOGI(TAG, "nx, will goto pg%d", scrNoNew);
                    lv_indev_keypad_wait_release();
                    main_menu_pg_init(scrNoNew, true);
                }
                else if (C_MMNU_PREV_PG == itemNoNew) {
                    if (!MMN_IS_1ST_LEVEL_MENU(g_scrNo))
                        break;
                    int16_t scrNoNew = (g_scrNo + MMN_1ST_LEVEL_PG_TOTAL  - 1) % MMN_1ST_LEVEL_PG_TOTAL;
                    ESP_LOGI(TAG, "prv, will goto pg%d", scrNoNew);
                    lv_indev_keypad_wait_release();
                    main_menu_pg_init(scrNoNew, true);
                }
            }
            break;
        }
        case LV_EVENT_GESTURE: {
            if (!MMN_IS_1ST_LEVEL_MENU(g_scrNo))
                break;
            lv_indev_wait_release(lv_indev_get_act());
            lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
            if (dir == LV_DIR_RIGHT) {
                if (g_scrNo > 0) {
                    main_menu_pg_init(g_scrNo - 1, true);
                }
            }
            else if (dir == LV_DIR_LEFT) {
                if (g_scrNo < MMN_1ST_LEVEL_PG_TOTAL - 1) {
                    main_menu_pg_init(g_scrNo + 1, true);
                }
            }
            break;
        }
        case LV_EVENT_DRAW_POST:
            _mmnu_draw_ex_cb(evt);
            break;
        default:
            break;
    }
#endif
}

static void _mmn_pageTab_btn_click_cb(lv_event_t * evt)
{
	lv_obj_t* obj = lv_event_get_target(evt);

	int32_t userData = (int32_t)lv_obj_get_event_user_data(obj, _mmn_pageTab_btn_click_cb); 
	ESP_LOGI(TAG, "pgTbl %ld clcked", userData);
	int pageNo = userData;
	if (pageNo == g_scrNo)
		return;

	if (pageNo == 4)
	{
#if 0 //---wsy  06.27
		LV_LOG_USER("ENTER PLAYER\r\n");
		int stat = player_is_play();
		if(stat)	zb_requestSwitchAp(C_ZB_AP_PLAYER);
		
		else create_dialog_with_timer(false, "无音频播放", 2000, 0, 0);
#endif
		return;
	}
	_mmn_topBtns_sel(pageNo);
	g_scrNo = pageNo;
	lv_tabview_set_act(g_winA, pageNo, LV_ANIM_OFF);
	return;
}

// 主页播放器UI更新函数
static void update_home_player_ui(TZ_PLAYSTATE_T state)
{
	if (!g_player_ui_state.home_play_btn ||
		!lv_obj_is_valid(g_player_ui_state.home_play_btn)) {
		return;
	}

	const lv_img_dsc_t* img_src = NULL;

	switch (state) {
	case TZ_PLAYING:
		img_src = g_XXX_img_dsc_tbl + MMN_play - 1; // 播放图标
		break;

	case TZ_PLAYPAUSED:
	case TZ_PLAYSTOPPED:
		img_src = g_XXX_img_dsc_tbl + MMN_pause - 1; // 暂停图标
		break;

	default:
		break;
	}

	if (img_src) {
		lv_obj_set_style_bg_img_src(g_player_ui_state.home_play_btn, img_src, 0);
		lv_obj_invalidate(g_player_ui_state.home_play_btn);
		//ESP_LOGI(TAG,"updata player_btn\r\n");
	}
}

// 当主页面销毁时
void home_page_destroy(void)
{
	g_player_ui_state.home_play_btn = NULL;
	g_player_ui_state.update_callback = NULL;
	g_player_ui_state.is_home_active = false;
}

static void mmn_createTopBtns(lv_obj_t * parent, int selNo)
{
    printf("mmn_createTopBtns\n");
	static bool style_inited = false;
	if (!style_inited) {
		lv_style_init(&style_indicator);
		lv_style_set_bg_color(&style_indicator, lv_color_hex(0x008CFF)); // 蓝色
		lv_style_set_bg_opa(&style_indicator, LV_OPA_COVER);
		lv_style_set_radius(&style_indicator, 2);

		
		lv_style_init(&style_focus);
		lv_style_set_border_width(&style_focus, 0);      
		lv_style_set_outline_width(&style_focus, 0);       
		lv_style_set_shadow_width(&style_focus, 0);       
		style_inited = true;
	}

#define TOP_BTN_BORDER 7
	const int16_t aBtnResNoTbl[] = { MMN_text_english_sel - 1,   MMN_txt_yuwen_sel - 1,  MMN_txt_kouyu - 1, MMN_txt_app_sel - 1,MMN_pause - 1 };
	for (int sn = 0; sn < MMN_1ST_LEVEL_PG_TOTAL; sn++) {
		int subImg = (sn == selNo);
		const lv_img_dsc_t * pImgDsc = g_XXX_img_dsc_tbl + aBtnResNoTbl[sn]; 
		lv_obj_t* btn = lv_btn_create(parent);
		lv_obj_set_size(btn, 60, 30);
		lv_obj_set_style_radius(btn, 0, 0);
		lv_obj_set_style_bg_color(btn, lv_color_hex(0xffffff), 0);
		lv_obj_set_style_bg_opa(btn, LV_OPA_0, 0);
		lv_obj_set_style_outline_width(btn, 0,0);
		lv_obj_set_style_shadow_width(btn, 0, 0);
		lv_obj_set_style_bg_img_src(btn, pImgDsc, 0);
	    lv_obj_add_style(btn, &style_focus, LV_STATE_FOCUS_KEY);
		lv_obj_add_style(btn, &style_focus, LV_STATE_FOCUSED);  
		lv_obj_set_pos(btn, 10 + 60 * sn, 40);
		lv_obj_set_style_border_width(btn, 0, 0);
		lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
		if (sn == selNo) {
			lv_obj_add_state(btn, LV_STATE_FOCUS_KEY);
		}
		lv_obj_set_style_opa(btn, LV_OPA_60, 0);
		lv_obj_set_style_opa(btn, LV_OPA_100, LV_STATE_FOCUS_KEY);
		lv_obj_set_style_opa(btn, LV_OPA_100, LV_STATE_PRESSED);
		lv_obj_add_event_cb(btn, _mmn_pageTab_btn_click_cb, LV_EVENT_CLICKED, sn);
		g_level1TopBtns[sn] = btn;

		lv_obj_t* indicator = lv_obj_create(parent);
		lv_obj_add_style(indicator, &style_indicator, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_size(indicator,20,5); 
		lv_obj_align_to(indicator, btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
		lv_obj_add_flag(indicator, LV_OBJ_FLAG_HIDDEN);
		lv_obj_set_style_radius(indicator, 10,0);
		lv_obj_set_style_border_width(indicator, 0,0);
		g_indicatorBars[sn] = indicator;
		if (sn == selNo) {
			lv_obj_clear_flag(indicator, LV_STATE_FOCUS_KEY);
		}
	}

	const lv_img_dsc_t* pImgDsc = g_XXX_img_dsc_tbl + aBtnResNoTbl[4];
	lv_obj_t* btn = lv_btn_create(parent);
	lv_obj_set_size(btn, 60, 30);
	lv_obj_set_style_radius(btn, 0, 0);
	lv_obj_set_style_bg_color(btn, lv_color_hex(0xffffff), 0);
	lv_obj_set_style_bg_opa(btn, LV_OPA_0, 0);
	lv_obj_set_style_outline_width(btn, 0, 0);
	lv_obj_set_style_shadow_width(btn, 0, 0);
	lv_obj_set_style_bg_img_src(btn, pImgDsc, 0);
	lv_obj_add_style(btn, &style_focus, LV_STATE_FOCUS_KEY);
	lv_obj_add_style(btn, &style_focus, LV_STATE_FOCUSED);
	lv_obj_set_pos(btn, 20 + 80 * 3, 40);
	lv_obj_set_style_border_width(btn, 0, 0);
	lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);

	lv_obj_add_event_cb(btn, _mmn_pageTab_btn_click_cb, LV_EVENT_CLICKED, 4);
	
	g_player_ui_state.home_play_btn = btn;
	g_player_ui_state.is_home_active = true;
	g_player_ui_state.update_callback = update_home_player_ui;
	return;
}


static void _mmn_esc_btn_click_cb(lv_event_t * evt)
{
	lv_obj_t* obj = lv_event_get_target(evt);

	ESP_LOGI(TAG, "btnEsc clcked A");
	if (MMN_IS_2ND_LEVEL_MENU(g_scrNo)) {
		int8_t scrNoNew = g_1stLevelMnuScrNo;
		ESP_LOGI(TAG, "nx, will ret pg%d", scrNoNew);
		g_itemNo = g_1stLevelMnuSelNo;
		main_menu_pg_init(scrNoNew, false);
		return;
	}
	return;
}

static void mmn_createEscBtn(lv_obj_t * parent) {
    printf("mmn_createEscBtn 2\n");
#define TOP_BTN_BORDER 12
	lv_obj_t* cont = lv_obj_create(parent);
	lv_obj_remove_style_all(cont); // 移除默认样式
	lv_obj_set_pos(cont, 0, 0); // 调整位置偏移
	lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
	lv_obj_set_style_pad_all(cont, 10, 0); // 透明内边距
	lv_obj_add_event_cb(cont, _mmn_esc_btn_click_cb, LV_EVENT_CLICKED, NULL);

	lv_obj_t * img = lv_img_create(cont);
	const lv_img_dsc_t * pImgDsc = g_XXX_img_dsc_tbl + MMN_icn_esc - 1;
	lv_img_set_src(img, pImgDsc);
	lv_obj_set_size(img, pImgDsc->header.w + TOP_BTN_BORDER * 2, pImgDsc->header.h + TOP_BTN_BORDER * 2);
	lv_obj_set_pos(img, 10, 2);
	lv_obj_set_style_pad_all(img, TOP_BTN_BORDER, 0);
	lv_obj_set_style_radius(img, 4, 0);
	lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);

	lv_color_t colorBg = lv_color_hex(0xc0c0c0);
	lv_obj_set_style_bg_color(img, colorBg, 0);
	lv_obj_set_style_bg_opa(img, LV_OPA_0, 0);
	lv_obj_set_style_bg_opa(img, LV_OPA_100, LV_STATE_FOCUS_KEY);
	lv_obj_set_style_bg_opa(img, LV_OPA_10, LV_STATE_PRESSED);
	/* lv_obj_set_style_opa(img, LV_OPA_60, 0);
	lv_obj_set_style_opa(img, LV_OPA_100, LV_STATE_FOCUS_KEY);
	lv_obj_set_style_opa(img, LV_OPA_80, LV_STATE_PRESSED); */
	
	lv_color_t colorBorderFcs = lv_color_white();
	lv_obj_set_style_border_color(img, colorBorderFcs, 0);
	lv_obj_add_event_cb(img, _mmn_esc_btn_click_cb, LV_EVENT_CLICKED, NULL);
}

static void crtPageBody(int16_t pgNo, bool doesInitStat, lv_obj_t * objParent)
{
		static const lv_coord_t col_dsc_dflt[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
		const mnu_page_dsc_t * p_pg_dsc_tbl = g_pg_dsc_tbl[pgNo];
		//lv_obj_set_style_pad_all(winA, 0, 0);
		const lv_coord_t * colDsc = (p_pg_dsc_tbl->pColDscTbl ? p_pg_dsc_tbl->pColDscTbl : col_dsc_dflt);

		bool isLevel1stMnu = MMN_IS_1ST_LEVEL_MENU(pgNo);
		lv_obj_set_grid_dsc_array(objParent, colDsc, p_pg_dsc_tbl->pRowDscTbl);

		lv_obj_set_style_border_width(objParent, 0, 0);
		lv_obj_set_style_pad_all(objParent, 0, 0);
		lv_obj_set_style_pad_row(objParent, 12, 0);
		lv_obj_set_style_bg_opa(objParent, LV_OPA_0, 0);
		printf("crtPageBody pgNo=%d\n", pgNo);
		if(pgNo == 4){
			lv_obj_t* lblTitle = lv_label_create(g_statBar);
			lv_label_set_text(lblTitle, "有道英语精选");
#if DARK_MODE
			lv_obj_set_style_text_color(lblTitle, lv_color_white(),0);
#endif
#ifndef C_PC_SIM
			lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
#endif
			lv_obj_align(lblTitle, LV_ALIGN_CENTER, 0,5);
			lv_obj_set_scrollbar_mode(objParent, LV_SCROLLBAR_MODE_OFF);  // 关闭滚动条
		}else if(pgNo == 5){
			lv_obj_t* lblTitle = lv_label_create(g_statBar);
			lv_label_set_text(lblTitle, "国学天地");
#if DARK_MODE
			lv_obj_set_style_text_color(lblTitle, lv_color_white(), 0);
#endif
#ifndef C_PC_SIM
			lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
#endif
			lv_obj_align(lblTitle, LV_ALIGN_CENTER, 0,5);
			lv_obj_set_scroll_dir(objParent, LV_DIR_NONE);
		}else if(pgNo == 6){
			lv_obj_t* lblTitle = lv_label_create(g_statBar);
			lv_label_set_text(lblTitle, "我的资源");
#if DARK_MODE
			lv_obj_set_style_text_color(lblTitle, lv_color_white(), 0);
#endif
#ifndef C_PC_SIM
			lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
#endif
			lv_obj_align(lblTitle, LV_ALIGN_CENTER, 0,5);
			lv_obj_set_scroll_dir(objParent, LV_DIR_NONE);
		}else{
			lv_obj_set_scroll_dir(objParent, LV_DIR_NONE);
		}
		lv_obj_set_style_pad_column(objParent, 0, 0);
		

		//ESP_LOGI(TAG, "curMainWin: %p", winA);

		int16_t entryNo = 0;
		int16_t itemTotal = p_pg_dsc_tbl->itemTotal; //
#define MMNU_GRID_LAYOUT_TOP_AID_ROW_N   1
		memset(g_page_btns, 0, sizeof(g_page_btns));
		for (; entryNo < itemTotal; entryNo++) {
#define C_ICON_BORDER_W  4
#define C_ICON_BORDER_H  4
			lv_obj_t * itemBg = NULL;
			lv_obj_t * img = NULL;

			/* ==== 新增：判断是否AI页面的"AI口语训练"项  06.11==== */
			bool isAIKouyuItem = (pgNo == 2) && (entryNo == 0);
			const mnu_item_inf_t * pItemInf = p_pg_dsc_tbl->pItemInfTbl + entryNo;
			uint8_t imgNo = pItemInf->resImgNo;
			bool isSmallIcn = false;
			if (imgNo != (uint8_t)-1) {
				const lv_img_dsc_t * pImgDsc = g_XXX_img_dsc_tbl + imgNo; // 
				if (!pItemInf->pTitleTxt) {
					img = lv_img_create(objParent);
					lv_img_set_src(img, pImgDsc);
					lv_obj_set_size(img, pImgDsc->header.w + C_ICON_BORDER_W * 2, pImgDsc->header.h + C_ICON_BORDER_H * 2);
					if (pImgDsc->header.w < 85) {
						isSmallIcn = true;
					}
					if (!isAIKouyuItem) {
						lv_obj_set_ext_click_area(img, 15);
						lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
					}
				}
				else {
					itemBg = lv_obj_create(objParent);
					lv_obj_remove_style_all(itemBg);
					lv_coord_t itemH = pImgDsc->header.h + C_ICON_BORDER_H * 2 + 32;
					lv_obj_set_size(itemBg, pImgDsc->header.w + C_ICON_BORDER_W * 2, itemH);
					if (pImgDsc->header.w < 85) {
						isSmallIcn = true;
					}
					img = lv_img_create(itemBg);
					lv_img_set_src(img, pImgDsc);
					lv_obj_set_size(img, LV_PCT(100), pImgDsc->header.h + C_ICON_BORDER_H * 2);
					lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 0);

					lv_obj_t * lblTitle = lv_label_create(itemBg);
#ifndef C_PC_SIM
					lv_label_set_cjk_font(lblTitle, 18, FT_FONT_STYLE_NORMAL);
#endif
					lv_label_set_text(lblTitle, pItemInf->pTitleTxt);		
#if DARK_MODE
					lv_obj_set_style_text_color(lblTitle, lv_color_white(), 0);
#endif
					lv_obj_align(lblTitle, LV_ALIGN_BOTTOM_MID, 0, -3);
				}

				if (0 == pItemInf->m_funcNo) {
					//lv_obj_set_style_img_recolor_opa(img, LV_OPA_20, 0);
					//lv_obj_add_event_cb(img, mmnOth_draw_spaceItem_cb, LV_EVENT_DRAW_MAIN, NULL);
				}
				/* else if ((MMN_SETTING_SCR_NO == pageNo) && (MMN_sys_upgrade - 1 == imgNo) &&
						isThereNewVersionInUpgCfg()) { // for update
					lv_obj_t * my_Cir = lv_obj_create(img);
					lv_obj_set_scrollbar_mode(my_Cir , LV_SCROLLBAR_MODE_OFF);
					lv_obj_set_size(my_Cir, 16, 16);
					lv_obj_align(my_Cir, LV_ALIGN_TOP_RIGHT, -5, 5);
					lv_color_t colorRed = lv_color_hex(0xE04040);
					lv_obj_set_style_bg_color(my_Cir, colorRed, 0);
					lv_obj_set_style_radius(my_Cir, LV_RADIUS_CIRCLE, 0);
				} */
			} else {
				img = lv_obj_create(objParent);
				lv_coord_t icnW = (pItemInf->col_span >= 2 ? 215 : 102);
				lv_obj_set_size(img, icnW, 102);
				lv_obj_set_style_bg_color(img, lv_color_hex(0xff87CEEB), 0);
			}
			lv_obj_t * objItemBg = (itemBg ? itemBg : img);
			g_page_btns[entryNo] = img;
			int radius = ((1 == pItemInf->col_span) && (1 == pItemInf->row_span)) ? 11+C_ICON_BORDER_W : 19+C_ICON_BORDER_W;
			if (isSmallIcn) {
				radius = 19;
			}
			lv_obj_set_style_border_width(img, C_ICON_BORDER_W, 0);
			lv_obj_set_style_radius(img, radius, 0);
			lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
		#if 0
			lv_color_t colorBorderFcs = LV_COLOR_MAKE(0, 0xaf, 0xaf);
			lv_obj_set_style_border_color(img, colorBorderFcs, LV_STATE_PRESSED);
			lv_obj_set_style_border_opa(img, LV_OPA_0, 0);
			lv_obj_set_style_border_opa(img, LV_OPA_COVER, LV_STATE_PRESSED);
		#endif
			lv_obj_set_style_border_opa(img, LV_OPA_0, 0);
			if (!isAIKouyuItem) {
				lv_obj_set_style_img_recolor_opa(img, 15, LV_STATE_PRESSED); // < LV_OPA_10
			}

			int8_t col = pItemInf->grid_col;  // 
			int8_t row = pItemInf->grid_row + MMNU_GRID_LAYOUT_TOP_AID_ROW_N;
			int8_t colSpan = pItemInf->col_span;
			int8_t rowSpan = pItemInf->row_span;
			lv_obj_set_grid_cell(objItemBg, LV_GRID_ALIGN_CENTER, col, colSpan,   LV_GRID_ALIGN_CENTER, row, rowSpan);
			if (!isAIKouyuItem) {
				lv_obj_add_event_cb(img, _mmn_item_btn_click_cb, LV_EVENT_SHORT_CLICKED, entryNo);
			}

		}

		buildGridNoToItemNoTbl(p_pg_dsc_tbl->pItemInfTbl, p_pg_dsc_tbl->itemTotal, p_pg_dsc_tbl->colNumEachRow, p_pg_dsc_tbl->gridTotal);  // 

		if (doesInitStat || g_itemNo < 0 || g_itemNo >= itemTotal) { 
			g_itemNo = 0; // non-std 
		}
		//lv_obj_add_state(g_page_btns[g_itemNo], LV_STATE_CHECKED);
}

extern quectel_handler_t* state_handler;
static void insert_sim_stat()
{
	LV_LOG_USER("insert_sim_stat A\r\n");
#ifndef C_PC_SIM
	int sim_status = 0;
	if (!state_handler) {
		printf("tz_hal_quectel_handler_new failed\n");
		return -1;
	}
	
	sim_status = state_handler->get_sim_state(&_simState);
	LV_LOG_USER("sim_status:%d\n", sim_status);

#endif

}

//back door act
 //后续可以根据g_devModeActive在按键处理添加后门操作
static void _statbar_event_cb(lv_event_t* e) {
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* obj = lv_event_get_target(e);

	if (code == LV_EVENT_PRESSING) {
		if (g_secretHoldStartTime == 0) {
			g_secretHoldStartTime = zb_get_time_ms_since_boot();
		}


	}
	else if (code == LV_EVENT_RELEASED) { 

		int32_t holdTime = zb_get_time_ms_since_boot() - g_secretHoldStartTime;
		g_secretHoldStartTime = 0;

		if (holdTime >= SECRET_HOLD_TIME) {
			g_devModeActive = !g_devModeActive;

			// 显示开发者模式状态
			const char* status = g_devModeActive ? "激活" : "关闭";
			char msg[50];
			snprintf(msg, sizeof(msg), "开发者模式已%s", status);
			app_msgbox_timeout("DEV MODE", msg, 2000 | MSGBOX_TOAST_FLAG, NULL, NULL);

			// 在状态栏显示指示器
			lv_obj_invalidate(g_statBar);
		}
	}
}


int main_menu_pg_init(int8_t pageNo, bool doesInitStat)
{
	if (g_rootWin) {
		lv_obj_del(g_rootWin);	 g_rootWin = NULL;
		memset(g_page_btns, 0, sizeof(g_page_btns));
	}
	home_page_destroy();
	g_statBar = NULL;  g_winA = NULL;
	switch (g_scrNo) {
		case MMN_SCR_NO_PROMPT_FOR_NEW_USER: {
			mmn_promptForNewUsr_scr_deinit();
			break;
		}
		case MMN_SCR_NO_NET_CFG: {
			network_setting_exit();
			break;
		}
		case MMN_SCR_NO_PWD_LOGIN: {
			prp_loginScr_deinit();
			break;
		}
		default:
			break;
	}
	ESP_LOGI(TAG, "pgInit A %d", pageNo);
	//--------------------------
	if (pageNo >= 0x20) {
		bool hasInit = false;
		switch (pageNo) {
			case MMN_SCR_NO_PROMPT_FOR_NEW_USER: {
				mmn_promptForNewUsr_scr_init();
				hasInit = true;
				break;
			}
			case MMN_SCR_NO_PROMPT_NET_CFG: {
				_mmn_promptNetCfg_scr_init();
				hasInit = true;
				break;
			}
			case MMN_SCR_NO_NET_CFG:
				network_setting_entry();
				hasInit = true;
				break;
			case MMN_SCR_NO_PWD_LOGIN:
				prp_loginScr_init(0);
				hasInit = true;
				break;
			default:
				break;
		}
		if (hasInit) {
			g_scrNo = pageNo;
		}
		else {
			ESP_LOGE(TAG, "wrong scrNo:%d", pageNo);
		}
		return (hasInit ? 1 : -1);
	}

	g_scrNo = pageNo;
	bool isLevel1stMnu = MMN_IS_1ST_LEVEL_MENU(pageNo);
	lv_obj_t * winBg = lv_obj_create(lv_scr_act());
	lv_obj_set_size(winBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_pad_all(winBg, 0, 0);
	lv_obj_set_style_border_width(winBg, 0, 0);
	lv_obj_set_scroll_dir(winBg, LV_DIR_NONE);

#if !DARK_MODE
	const lv_img_dsc_t* pImgDsc = &MMN_BG;
#else
	const lv_img_dsc_t* pImgDsc = &MMN_BG_BLACK;
#endif
	lv_obj_set_style_bg_img_src(winBg, pImgDsc,0);

	lv_obj_t * winA = NULL; //tabview  组件主窗体
	if (isLevel1stMnu) {
		winA = lv_tabview_create(winBg, LV_DIR_TOP, 5); // 
		lv_obj_set_style_bg_opa(winA, LV_OPA_0, LV_PART_MAIN); 
		lv_obj_set_style_border_opa(winA, LV_OPA_0, LV_PART_MAIN);

	//	lv_obj_set_style_transform_angle(winA, 0, LV_STATE_DEFAULT);
	} 
	else {
		winA = lv_obj_create(winBg);
	}
	g_winA = winA;
#if 0
	lv_coord_t statAreaH = (isLevel1stMnu ? MMNU_STATUS_BAR_THICK_H : MMNU_STATUS_BAR_H);
#else
	lv_coord_t statAreaH = (isLevel1stMnu ? MMNU_STATUS_BAR_THICK_H+7 : MMNU_STATUS_BAR_H+7);
#endif
	lv_obj_set_style_pad_all(winA, 0, 0);
	lv_obj_set_style_border_width(winA, 0, 0);
	lv_obj_set_size(winA, SCREEN_WIDTH, SCREEN_HEIGHT - statAreaH);
	lv_obj_align(winA, LV_ALIGN_BOTTOM_LEFT, 0, 0);
	lv_obj_t * aTbls[MMN_1ST_LEVEL_PG_TOTAL] = { 0 };
	if (isLevel1stMnu) {
		for (int sn = 0; sn < MMN_1ST_LEVEL_PG_TOTAL; sn++) {
			char tabNm[8];
			tabNm[0] = 'A' + sn;
			tabNm[1] = '\0';
			aTbls[sn] = lv_tabview_add_tab(winA, tabNm);
			lv_obj_set_style_bg_opa(aTbls[sn], LV_OPA_0, LV_PART_MAIN);
			

		}
	}

	lv_color_t borderColor = LV_COLOR_MAKE(0xff,0xff,0xff);
	lv_obj_set_style_border_color(winA, borderColor, 0);
	//----------
	lv_obj_t * pObjStatBar = lv_obj_create(winBg);
	lv_obj_set_size(pObjStatBar, SCREEN_WIDTH, statAreaH);
	//lv_obj_set_style_opa(pObjStatBar, LV_OPA_0,0);
	lv_obj_set_style_bg_opa(pObjStatBar, LV_OPA_0, 0);
	lv_obj_set_style_border_width(pObjStatBar, 0, 0);
	lv_obj_clear_flag(pObjStatBar, LV_OBJ_FLAG_SCROLLABLE);

	lv_obj_add_event_cb(pObjStatBar, _statbar_event_cb, LV_EVENT_ALL, NULL);

	g_statBar = pObjStatBar;
	//----------
	if (isLevel1stMnu) {
		mmn_createTopBtns(pObjStatBar, pageNo);
		lv_obj_add_event_cb(pObjStatBar, _mmnu_draw_statbar_cb, LV_EVENT_DRAW_MAIN, NULL);
	}
	else {
		mmn_createEscBtn(pObjStatBar);
		printf("mmn_createEscBtn 1\n");
	}
	sys_setCurMainWinAndCb(winA, _page_WinEvtCb);

	if (isLevel1stMnu) {
#if 1
		lv_obj_t* tab_btns = lv_tabview_get_tab_btns(winA);
		lv_obj_add_flag(tab_btns,  LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(g_indicatorBars[pageNo], LV_OBJ_FLAG_HIDDEN);
		_mmn_topBtns_sel(pageNo);
#endif
		for (int pgNo = 0; pgNo < MMN_1ST_LEVEL_PG_TOTAL; pgNo++) {
			crtPageBody(pgNo, doesInitStat, aTbls[pgNo]);
		}
	}
	else {
		crtPageBody(pageNo, doesInitStat, winA);
	}

	lv_obj_add_event_cb(winA, _page_WinEvtCb, LV_EVENT_ALL, NULL);
#if 1
	//lv_obj_clear_flag(winA, LV_OBJ_FLAG_GESTURE_BUBBLE); // 允许手势事件冒泡
#endif
	//--------------
	g_rootWin = winBg;
	if (isLevel1stMnu) {
		lv_tabview_set_act(winA, pageNo, LV_ANIM_OFF);
		lv_obj_add_event_cb(winA, _tabvw_valChg_EvtCb, LV_EVENT_VALUE_CHANGED, NULL);

	}
	//----------------- seek to current subPage in vert dir. 
	/* int8_t sectNo = getSectionNoInY(p_pg_dsc_tbl->pItemInfTbl, g_itemNo);
	if (sectNo > 0) {
		lv_obj_scroll_to_y(g_winA, g_sectLocY_tbl[sectNo], LV_ANIM_OFF);
	} */
	return 1;
}

//----------------------------------------------
static void _promptNetCfg_draw_cb(lv_event_t * evt)
{
#define MY_INFO_LOC_X  63
#define MY_INFO_LOC_Y  148
	const misc_draw_item_t drawItems_i[] = {
		{ .m_locX = MY_INFO_LOC_X + 0, .m_locY = MY_INFO_LOC_Y, .m_type = 0,
		  .unn.m_pText = "按",   },
		{ .m_locX = MY_INFO_LOC_X + 20, .m_locY = MY_INFO_LOC_Y, .m_type = 1, 
		  .unn = { .m_imgHash = COMMON_IMG_OK_PLAYER_YELLOW },  },
		{ .m_locX = MY_INFO_LOC_X + 42, .m_locY = MY_INFO_LOC_Y, .m_type = 0,
		  .unn.m_pText = "键连接",    },
 	};

	const misc_draw_mult_items_t stDrawMultItems_I = {
		.m_num = 3,
		.pstDrawItems = drawItems_i,
	};

 	const misc_draw_item_t drawItemsTxt_t[] = {
		{ .m_locX = MY_INFO_LOC_X - 3, .m_locY = MY_INFO_LOC_Y, .m_type = 0,
		  .unn.m_pText = "按[确认]键连接",    },
 	};
	const misc_draw_mult_items_t stDrawMultItems_T = {
		.m_num = 1,
		.pstDrawItems = drawItemsTxt_t,
	};

	if (zb_img_res_get_img_total() > 0) {
		draw_txt_img_group_cb(evt, &stDrawMultItems_I);
	}
	else {
		draw_txt_img_group_cb(evt, &stDrawMultItems_T);
	}
}

static void _promptNetCfg_scr_cb(lv_event_t * evt) {
	lv_event_code_t evtCode = evt->code;

#if 0
    
	int keyVal = 0;
	switch (evtCode) {
		case LV_EVENT_DRAW_POST:
			_promptNetCfg_draw_cb(evt);
			break;
		case LV_EVENT_KEY_STAGE:
			keyVal = lv_event_get_key(evt);
			switch (keyVal) {
				case (LV_EX_FLAG_KEY_SHORT | LV_KEY_ENTER): {
					ESP_LOGI(TAG, "mmnu keyEnt");
					main_menu_pg_init(MMN_SCR_NO_NET_CFG, 0);
					break;
				}
				case (LV_EX_FLAG_KEY_SHORT | LV_KEY_ESC): {
					int8_t scrNo = g_srcNoBak;
					ESP_LOGI(TAG, "mmnu keyEsc %d", (int)scrNo);
					main_menu_pg_init(scrNo, 0);
					break;
				}
				default:
					break;
			}
			break;
		case LV_EVENT_KEY: {
			if (!keyVal) {
				keyVal = lv_event_get_key(evt);
			}
			ESP_LOGI(TAG, "mmnu key %x", keyVal);
			break;
		}
		case LV_EVENT_DELETE: {
			g_rootWin = NULL;
			break;
		}
		default:
			break;
	}
#endif
	return;
}


//弹窗提示图片 请连接网络
static int _mmn_promptNetCfg_scr_init(void) {
	printf("_mmn_promptNetCfg_scr_init A\n");
	ESP_LOGI(TAG, "prptNetCftScr A");

	g_srcNoBak = g_scrNo;
	if (g_srcNoBak < 0 || g_srcNoBak >= MMN_MENU_PG_TOTAL) {
		g_srcNoBak = 0;
	}
	lv_obj_t * winBg = lv_obj_create(lv_scr_act());
	lv_obj_set_size(winBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_pad_all(winBg, 0, 0);
	lv_obj_set_style_radius(winBg, 0, 0);
	lv_obj_set_style_bg_color(winBg, lv_color_hex(0xfff4d2), 0);

	lv_obj_t* label = lv_label_create(winBg);
	bool isNetSwOpen = getPrjNetworkSWstate();
	const char * pInfEx = (isNetSwOpen ? "\n网络尝未就绪" : "");
	lv_label_set_text_fmt(label, "该功能需要连接网络%s", pInfEx);
	lv_obj_set_style_text_line_space(label, 3, 0);
	lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_align(label, LV_ALIGN_TOP_MID, 0, (pInfEx[0] ? 120 - 20 : 120));
	
	lv_obj_add_event_cb(winBg, _promptNetCfg_scr_cb, LV_EVENT_ALL, NULL);
	lv_group_add_obj(lv_group_get_default(), winBg);
	lv_group_focus_obj(winBg);
	g_rootWin = winBg;
	g_scrNo = MMN_SCR_NO_PROMPT_NET_CFG;
	return 0;
}


int main_menu_ap_init(void) {

    int16_t scrNo = g_scrNo;
    ESP_LOGI(TAG, "mmnuApInit A %hd", scrNo);
    bool doesInitStat = false;

    /* if (!mmn_promptForNewUsr_scr_wasShown() && (0 == getPrjGlobalParm(ZB_PARM_SKIP_TIPS_FOR_NEWUSR, 0, NULL))) {
        scrNo = MMN_SCR_NO_PROMPT_FOR_NEW_USER;
        g_scrNo = scrNo;
    }
    else */
    if (scrNo < 0 || scrNo >= MMN_MENU_PG_TOTAL) {
        scrNo = 0;
        g_scrNo = scrNo;
        g_itemNo = 0;
        doesInitStat = true;
    }
    _mmn_update_statbar_parm(true);
    main_menu_pg_init(scrNo, doesInitStat);
    g_mmn_tmr = lv_timer_create(_mmn_timer_cb, 1600, NULL);
    if (!g_mmn_tmr) {
        ESP_LOGE(TAG, "fail to crt tmr A!#");
    }
    return 1;
}

int main_menu_ap_cb(lv_event_code_t msg, int param) { 
    switch (msg) {
        case EVENT_FREE_AP_RES:
			home_page_destroy();
            if (g_mmn_tmr) {
                lv_timer_del(g_mmn_tmr);  g_mmn_tmr = NULL;
            }
            g_statBar = NULL;  g_winA = NULL;
            if ((g_scrNo < 0x20) || (MMN_SCR_NO_PROMPT_NET_CFG == g_scrNo)) {
                lv_obj_del(g_rootWin);  g_rootWin = NULL;
            }
            if (MMN_SCR_NO_PWD_LOGIN == g_scrNo) {
                g_scrNo = MMN_XIMALY_SCR_NO;
            }
            else if (MMN_SCR_NO_NET_CFG == g_scrNo) {
                network_setting_exit();
            }
//			if(g_sim_handler){
//#ifndef C_PC_SIM
//				tz_hal_quectel_handler_destory(g_sim_handler);
//#endif
//				g_sim_handler = NULL;
//			}
			
            ESP_LOGI(TAG, "mmnuApFreeRes Z");
            break;

        case EVENT_EX_CMD: {
            ESP_LOGI(TAG, "rcv exCmd par:%d", param);
            if (MMN_SCR_NO_PWD_LOGIN == g_scrNo) {
            #if 0
                prp_loginScr_deinit();
                if (PRP_CMD_CODE_LOGIN_OK == param) {
                    zb_requestSwitchAp(C_ZB_AP_XIMALAYA);
                }
                else { // fail to login, then restore to mainmenu
                    int8_t scrNo = MMN_XIMALY_SCR_NO;
                    main_menu_pg_init(scrNo, 0);
                }
            #endif
            }
            else if (MMN_SCR_NO_NET_CFG == g_scrNo) {
                if (NET_CONN_CMD_CODE == param) {
                    int8_t scrNo = g_srcNoBak;
                    ESP_LOGI(TAG, "retFromNetCfg, %d", (int)scrNo);
                    main_menu_pg_init(scrNo, 0);
                }
            }
			else if (PLY_CMD_PLY_LST_RET == param) {
				LV_LOG_USER("enter PLY_CMD_PLY_LST_RET EVENT_EX_CMD \r\n");
				zb_requestSwitchAp(C_ZB_AP_MAINMENU);
			}
            break;
        }
        default:
            break;
    }
    return 0;
}

/** 查询当前屏选项号 
 */
int main_menu_get_scr_sel_no() {
	return g_itemNo;
}

uint8_t mmn_get_batteryLevelNo(void)
{
	return g_batteryLevelNo;
}

void mmn_set_screenNo(int8_t scrNo, uint8_t checkCode)
{
	if (checkCode != MMN_CHECK_CODE)
		return;
	g_scrNo = scrNo;
}



