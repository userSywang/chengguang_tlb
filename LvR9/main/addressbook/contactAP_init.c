#include <time.h>
#include <lvgl/lvgl.h>
#include "../utils/misc_utils.h"
#include "lv_conf.h"
#include <math.h> 
//#include "lvglGaussian.h"

#include "../lvgl_main.h"


#include "../data_parse/img_res.h"

#include "../utils/global_parm.h"
#include "../ex_widget/app_msg.h"
#include "../utils/global_parm.h"

#include "../data_parse/analyze_listening.h"

#include "contact_ap.h"
#include "contactAP_data.h"
#include "../utils/style_tz.h"

#include"../ex_widget/keyboard.h"

#ifndef C_PC_SIM

#include "LvR9/trd/tz_audio_player/include/tz_audio_player.h"

#include "LvR9/trd/ubus/include/tz_hal_quectel.h"
#include "LvR9/trd/ubus/include/ubus_uloop_receiver.h"
#include "LvR9/trd/tz_sys_lib/include/cvi_tz_sys.h"
#include "LvR9/trd/tz_sys_lib/include/mlog.h"
#else

#endif

// 函数声明
lv_obj_t* tbcd_createMainWin(const char* pcTitle);
static int tbcd_no_contentList_scr_init(int param);
static int tbcd_add_contact_scr_init(int param);
int tbcd_calling_scr_init(CallEvent param);
static int tbcd_ready_call_scr_init(int param);
int tbcd_incoming_call_scr_init(int param);
int tbcd_sos_scr_init(int param);
static void _tbcd_contentList_scr_cb(lv_event_t* evt);
static void _tbcd_add_contact_scr_cb(lv_event_t* evt);
static void _tbcd_calling_scr_cb(lv_event_t* evt);
static void _tbcd_incoming_call_scr_cb(lv_event_t* evt);
static void _sos_event_cb(lv_event_t* evt);
void incoming_call_scr_del(void);
static void _tbcd_create_contact_cb(lv_event_t* evt);

static void seed_phone_ask();
static void seed_phone_call();
void  seed_phone_Nocall();
static void* recv_phone_enent_hander(void* arg);
static void on_click_item(lv_event_t* event);
static void _tbcd_pickup_cb();
void _tbcd_no_pickup_cb();
static void callBtn_cb();
void free_calling_scr();

void sos_call_next_number();
void start_sos_calling();
void sos_call_timeout_handler();

static void _tbcd_return_cb(lv_event_t* evt);
static ContactItem* create_contact_item(lv_obj_t* parent, TBCD_CONTACT_RECORD_ITEM* contact);

static void safe_delete_timer(lv_timer_t** timer);

// 定义主标题背景色

#define LABEL_COLOR  0x787F87 //提示词颜色
#define btn_def_color 0xE8E8EE
#define btn_focus_color 0x393F4D

//SOS按键颜色
#define CONFIRMATION_BTN_COLOR  0xEE4E4E 
#define CANCEL_BTN_COLOR  0xB3B3B3
#define SOS_LABEL_COLOR  0xFFFFFF


// 定义界面布局相关常量
#if !DARK_MODE
#define TITLE_BG_COLOR	0xFFFFFF  //主标题背景色
#define TEXT_COLOR   0x000000
#else
#define TITLE_BG_COLOR   0X000000
#define TEXT_COLOR   0xFFFFFF
#endif

#define TBWD_SOS_BG_COLOR    0X000000
#define TBWD_SOS_BTN_RADIUS   27
#define TBWD_BG_CORNER_RADIUS  12
#define TBWD_MAIN_AREA_OBJ_LOCY  42  // 主区域组件 y 位置 
#define TBWD_MAIN_AREA_OBJ_H	(SCREEN_HEIGHT - TBWD_MAIN_AREA_OBJ_LOCY)  // 主区域组件的高 

#define CONTACT_ITEM_HEIGHT 74
#define CONTENT_HEIGHT 56
#define IMG_WIDTH 53  //53
#define ITEM_RADIUS 8
#define BG_COLOR lv_color_hex(0xF5F7FA)

#define TITLE_HEIGHT 60       // 标题栏高度
#define NAV_HEIGHT 64         // 导航栏高度
#define TABVIEW_HEIGHT (SCREEN_HEIGHT - TITLE_HEIGHT - NAV_HEIGHT) // TabView高度



static int16_t  g_scrNo = -1;  // 当前屏号 

// 定义日志标签
static const char* TAG = "contact";
static lv_group_t* sys_group;

AppView g_app_views[2];

// 全局变量定义
TBCD_CONTACT_INF g_pContactAp = { 0 };
TBCD_POP_UP_INF* g_popUp_compile = NULL;//编辑弹窗

TBCD_POP_UP_INF  g_popUp_ask ={ NULL};
calling calling_strct = {NULL};
sos sos_strct = {NULL};
TBCD_CONTACT_RECORD_ITEM new_contact = {NULL};//当前使用/编辑联系人数据
TBCD_CONTACT_RECORD_ITEM calling_contact = {0};//当前通话联系人数据
TBCD_CONTACT_RECORD_ITEM popup_contact = { 0 };//来电弹窗联系人数据


// 定义屏幕初始化和释放函数指针类型
typedef int (*FN_TBCD_SCREEN_INIT)(int parm);
typedef int (*FN_TBCD_SCREEN_FREE)(void);

struct DIC_SCR_INF {
	FN_TBCD_SCREEN_INIT  m_fn_init;
	FN_TBCD_SCREEN_FREE  m_fn_free;
};



static lv_style_t style_clickBtn;
void custom_style_set_img_recolor(lv_style_t* style) { //点击图片图片有按下风格

	lv_style_set_img_recolor(style, lv_color_black());

	lv_style_set_img_recolor_opa(style, LV_OPA_30);
}


static void quectel_test()
{
	LV_LOG_USER("quectel_test A\r\n");
#ifndef C_PC_SIM
	quectel_handler_t* state_handler_TEST;
	if(!state_handler_TEST)
	{
		state_handler_TEST = tz_hal_quectel_handler_new();
		LV_LOG_USER("state_handler_TEST: %p", state_handler_TEST);
		int response_status = state_handler_TEST->call(calling_contact.phone);
		printf("response_status:%d\n", response_status);
    }


#endif
	LV_LOG_USER("quectel_test B\r\n");
}

static void seed_phone_ask()
{
#ifndef C_PC_SIM
	int response_status = 0;

	printf("seed_phone_ask A\r\n");
	LV_LOG_USER("state_handler: %p", state_handler);
	if (!state_handler) {
		printf("tz_hal_quectel_handler_new failed\n");
		state_handler = tz_hal_quectel_handler_new();
		if (!state_handler) {
			printf("tz_hal_quectel_handler_new failed\n");
		    return -1;
		}
	}
	printf("seed_phone_ask B\r\n");
	response_status = state_handler->answer();
	printf("response_status:%d\n", response_status);
#endif
}

//主动拨打 实例
static void seed_phone_call()
{
#ifndef C_PC_SIM
	int response_status = 0;
	if (!state_handler) {
		printf("tz_hal_quectel_handler_new failed\n");
		return -1;
	}
	callBtn_cb();

	LV_LOG_USER("seed_phone_call A\r\n");
	response_status = state_handler->call(calling_contact.phone);
	printf("response_status:%d\n", response_status);
#else
	callBtn_cb();
#endif

}
void seed_phone_Nocall()
{
#ifndef C_PC_SIM
	int response_status = 0;
	//quectel_handler_t* handler = tz_hal_quectel_handler_new();
	if (!state_handler) {
		printf("tz_hal_quectel_handler_new failed\n");
		return -1;
	}
	response_status = state_handler->hangup();
	printf("response_status:%d\n", response_status);
#endif
}

static lv_obj_t* labelTitle;     // 公共标题

/* 输入缓存 */
static char input_buf[20] = { 0 };
static lv_style_t highlight_style;
static lv_style_t nav_active_style;

// 设置导航栏图片样式
static void set_nav_icon_state(bool is_contact_active) {
	// 设置联系人图标状态
	uint32_t contact_img_id = is_contact_active ?
		ADDRESSBOOK_IMG_ADDRESS_ICON_FOCUS :
		ADDRESSBOOK_IMG_ADDRESS_ICON_UNFOUCS;

	// 设置拨号盘图标状态
	uint32_t dialpad_img_id = is_contact_active ?
		ADDRESSBOOK_IMG_DIAL_ICON_UNFOUCS :
		ADDRESSBOOK_IMG_DIAL_ICON_FOCUS;

	// 加载并设置图标
	lv_img_set_src(g_pContactAp.nav.contact_img,
		zb_img_res_load_one(NULL, contact_img_id, IMG_FREE_TYPE_AUTO, NULL));
	lv_img_set_src(g_pContactAp.nav.dialpad_img,
		zb_img_res_load_one(NULL, dialpad_img_id, IMG_FREE_TYPE_AUTO, NULL));
}

// 设置导航栏文本颜色
static void set_nav_text_color(bool is_contact_active) {
	lv_color_t active_color = lv_color_hex(0x008CFF);
	lv_color_t inactive_color = lv_color_hex(0x666666);

	lv_obj_set_style_text_color(g_pContactAp.nav.contact_text,
		is_contact_active ? active_color : inactive_color, 0);
	lv_obj_set_style_text_color(g_pContactAp.nav.dialpad_text,
		is_contact_active ? inactive_color : active_color, 0);
}

static void switch_contact_tab(uint8_t tab_index) {
	bool is_contact = (tab_index == 0);
	set_nav_icon_state(is_contact);
	set_nav_text_color(is_contact);
	lv_tabview_set_act(g_pContactAp.g_tabview, tab_index, LV_ANIM_OFF);
}


static void _tabvw_valChg_EvtCb(lv_event_t* evt)
{
	lv_event_code_t evtCode = evt->code;
	if (LV_EVENT_VALUE_CHANGED == evtCode) {
		int16_t pgNo = lv_tabview_get_tab_act(g_pContactAp.g_tabview);

		if (!pgNo) switch_contact_tab(0);
		else switch_contact_tab(1);
	}
	return;
}

/* 事件回调 */
static void switch_contact(lv_event_t* e) {

	switch_contact_tab(0);
}

static void switch_dialpad(lv_event_t* e) {	
	switch_contact_tab(1);
}

static void num_button_cb(lv_event_t* e) {
	lv_obj_t* btn = lv_event_get_target(e);
	lv_obj_t* label = lv_obj_get_child(btn, 0);
	const char* txt = lv_label_get_text(label);

	strncat(input_buf, txt, 1); // 仅取第一个字符
	lv_label_set_text(g_pContactAp.dialpad.input_label, input_buf);
}

static void clear_cb(lv_event_t* e) {
    char current_text[20];
    strncpy(current_text, input_buf, sizeof(current_text) - 1);
    current_text[sizeof(current_text) - 1] = '\0';

    if (strlen(current_text) > 0) {
        current_text[strlen(current_text) - 1] = '\0';
    }

    lv_label_set_text(g_pContactAp.dialpad.input_label, current_text);

    strncpy(input_buf, current_text, sizeof(input_buf) - 1);
    input_buf[sizeof(input_buf) - 1] = '\0';
}

static void on_call_button_clicked(lv_event_t* e) {
	if (strlen(input_buf) > 0) {
#ifndef C_PC_SIM
		quectel_handler_t* handler = tz_hal_quectel_handler_new();
		handler->call(input_buf);
		// 跳转到通话界面
#endif
		tbcd_switchScr(TBCD_SCR_CALLING, 0);
	}
}

void create_contact_list_screen(lv_obj_t* parent) {

	// 添加标题
	labelTitle = lv_label_create(parent);
	lv_obj_set_style_pad_top(labelTitle, 10, 0);
	lv_obj_align(labelTitle, LV_ALIGN_TOP_MID, 0, 5);
	lv_label_set_text(labelTitle, "通讯录");
	lv_obj_set_style_text_align(labelTitle, LV_TEXT_ALIGN_CENTER, 0);
#if DARK_MODE
	lv_obj_set_style_text_color(labelTitle, lv_color_white(), 0);
#endif

#ifndef C_PC_SIM
	lv_label_set_cjk_font(labelTitle, 18, FT_FONT_STYLE_NORMAL);
#endif

	lv_obj_t* returnBtn = lv_img_create(parent);
	lv_img_dsc_t* returnBtn_img = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_CONTACT_RETURN, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(returnBtn, returnBtn_img);
	lv_obj_set_style_pad_top(returnBtn, 10, 0);
	lv_obj_align(returnBtn, LV_ALIGN_TOP_LEFT, 10, 0);
	lv_obj_add_flag(returnBtn, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(returnBtn, _tbcd_return_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_add_style(returnBtn, &style_clickBtn, LV_STATE_PRESSED);

	lv_obj_t* createBtn = lv_img_create(parent);
	lv_obj_align(createBtn, LV_ALIGN_TOP_RIGHT, -10, 0);
	lv_obj_set_style_pad_top(createBtn, 10, 0);
	if (g_pContactAp.contact_count < TBCD_CONTACT_MAX)
	{
		lv_img_dsc_t* createBtn_img = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_ADDCONNECT, IMG_FREE_TYPE_AUTO, NULL);
		lv_img_set_src(createBtn, createBtn_img);
		lv_obj_add_flag(createBtn, LV_OBJ_FLAG_CLICKABLE);
		lv_obj_add_flag(createBtn, LV_STATE_PRESSED);
	}
	else
	{
		lv_img_dsc_t* createBtn_img = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_UN_CONNECT, IMG_FREE_TYPE_AUTO, NULL);
		lv_img_set_src(createBtn, createBtn_img);
		lv_obj_clear_flag(createBtn, LV_OBJ_FLAG_CLICKABLE);
		lv_obj_clear_flag(createBtn, LV_STATE_PRESSED);
	}
	lv_obj_add_event_cb(createBtn, _tbcd_create_contact_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_add_style(createBtn, &style_clickBtn, LV_STATE_PRESSED);

#if !test_file
	if (g_pContactAp.contact_count <= 0) // 根据文件中的联系人进行判断
#else
	if (0)
#endif
	{
		g_pContactAp.labelInf = lv_label_create(parent); // 使用传入的parent

		lv_label_set_text(g_pContactAp.labelInf, "暂无联系人\n点击右上角添加联系人");
		lv_obj_set_style_text_align(g_pContactAp.labelInf, LV_TEXT_ALIGN_CENTER, 0);
		lv_label_set_long_mode(g_pContactAp.labelInf, LV_LABEL_LONG_WRAP);
		lv_obj_set_style_text_color(g_pContactAp.labelInf, lv_color_hex(LABEL_COLOR), LV_STATE_DEFAULT);
		lv_obj_center(g_pContactAp.labelInf);
	}
	else
	{
		// 创建主容器 - 使用传入的parent
		lv_obj_t* ui_main = lv_obj_create(parent);
		lv_obj_remove_style_all(ui_main);
		lv_obj_set_size(ui_main, 300, LV_SIZE_CONTENT);
		lv_obj_align(ui_main, LV_ALIGN_TOP_MID, 10, 50);
		lv_obj_set_flex_flow(ui_main, LV_FLEX_FLOW_COLUMN);

#if !test_file
		for (int i = 0; i < g_pContactAp.contact_count; i++) {
			LV_LOG_USER("name :%s", g_pContactAp.contacts[i].name);
			LV_LOG_USER("num_img: %d", g_pContactAp.contacts[i].num_img);
			LV_LOG_USER("phone:%s", g_pContactAp.contacts[i].phone);
			g_pContactAp.contactList.contactItem[i] = create_contact_item(ui_main, &g_pContactAp.contacts[i]);
		}
#else
		// 测试模式下的联系人创建
		int num_contacts = 3; // 测试数量
		TBCD_CONTACT_RECORD_ITEM* contacts = (TBCD_CONTACT_RECORD_ITEM*)malloc(num_contacts * sizeof(TBCD_CONTACT_RECORD_ITEM));
		if (!contacts) {
			LV_LOG_ERROR("Memory allocation failed");
			return;
		}

		// 为每个联系人赋值
		for (int i = 0; i < num_contacts; i++) {
			char name[30];
			snprintf(name, sizeof(name), "联系人 %d", i + 1);
			strncpy(contacts[i].name, name, sizeof(contacts[i].name) - 1);
			contacts[i].name[sizeof(contacts[i].name) - 1] = '\0';

			char phone[20];
			snprintf(phone, sizeof(phone), "123456789%d", i);
			strncpy(contacts[i].phone, phone, sizeof(contacts[i].phone) - 1);
			contacts[i].phone[sizeof(contacts[i].phone) - 1] = '\0';

			LV_LOG_USER("%s %s", contacts[i].name, contacts[i].phone);
			g_pContactAp.contactList.contactItem[i] = create_contact_item(ui_main, &contacts[i]);
		}

		// 保存测试联系人数据
		if (tbcd_save_contact_record(CONTACT_FILE_PATH, g_pContactAp.contactList.contactItem, num_contacts) != 0) {
			LV_LOG_ERROR("保存测试联系人失败");
		}

		free(contacts); // 释放临时创建的联系人数组
#endif
		// 创建提示标签 - 注意是创建在ui_main容器内部
		g_pContactAp.labelInf = lv_label_create(ui_main);
		lv_label_set_text(g_pContactAp.labelInf, "\n\n         仅支持添加3个亲情号码");
		lv_label_set_long_mode(g_pContactAp.labelInf, LV_LABEL_LONG_WRAP);
		lv_obj_set_style_text_align(g_pContactAp.labelInf, LV_TEXT_ALIGN_CENTER, 0);
		lv_obj_set_style_text_color(g_pContactAp.labelInf, lv_color_hex(LABEL_COLOR), LV_STATE_DEFAULT);
	}
}

void create_dialpad_screen(lv_obj_t* parent) {  // 添加父组件参数

	// 添加标题
	labelTitle = lv_label_create(parent);
	lv_obj_set_style_pad_top(labelTitle, 10, 0);
	lv_obj_align(labelTitle, LV_ALIGN_TOP_MID, 0, 5);
	lv_label_set_text(labelTitle, "通讯录");
	lv_obj_set_style_text_align(labelTitle, LV_TEXT_ALIGN_CENTER, 0);
#if DARK_MODE
	lv_obj_set_style_text_color(labelTitle, lv_color_white(), 0);
#endif

#ifndef C_PC_SIM
	lv_label_set_cjk_font(labelTitle, 18, FT_FONT_STYLE_NORMAL);
#endif
	
	lv_obj_t* returnBtn = lv_img_create(parent);
	lv_img_dsc_t* returnBtn_img = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_CONTACT_RETURN, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(returnBtn, returnBtn_img);
	lv_obj_set_style_pad_top(returnBtn, 10, 0);
	lv_obj_align(returnBtn, LV_ALIGN_TOP_LEFT, 10, 0);
	lv_obj_add_flag(returnBtn, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(returnBtn, _tbcd_return_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_add_style(returnBtn, &style_clickBtn, LV_STATE_PRESSED);


	/* 数字输入显示 */
	g_pContactAp.dialpad.input_label = lv_label_create(parent); 
	lv_label_set_text(g_pContactAp.dialpad.input_label, "");
	lv_obj_set_style_text_font(g_pContactAp.dialpad.input_label, &lv_font_montserrat_24, 0);
	lv_obj_align(g_pContactAp.dialpad.input_label, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_set_style_text_color(g_pContactAp.dialpad.input_label, lv_color_white(), 0);

	/* 数字键盘网格 */
	g_pContactAp.dialpad.grid = lv_obj_create(parent);  
	lv_obj_set_size(g_pContactAp.dialpad.grid, 240, 260);
	lv_obj_align(g_pContactAp.dialpad.grid, LV_ALIGN_CENTER, 0, -10);
	lv_obj_set_layout(g_pContactAp.dialpad.grid, LV_LAYOUT_GRID);
	lv_obj_set_style_bg_color(g_pContactAp.dialpad.grid, lv_color_hex(TITLE_BG_COLOR), 0);
	lv_obj_add_style(g_pContactAp.dialpad.grid, get_global_style(STYLE_BTN_BASE), LV_PART_MAIN | LV_STATE_DEFAULT);

	/* 3列等宽，4行等高 */
	static lv_coord_t col_dsc[] = { 80, 80, 80, LV_GRID_TEMPLATE_LAST };
	static lv_coord_t row_dsc[] = { 60, 60, 60, 60, LV_GRID_TEMPLATE_LAST };
	lv_obj_set_grid_dsc_array(g_pContactAp.dialpad.grid, col_dsc, row_dsc);

	/* 创建数字按钮 */
	const char* btn_map[] = { "1\n ", "2\n", "3\n",
							 "4\n", "5\n", "6\n",
							 "7\n", "8\n", "9\n",
							 "*\n", "0\n", "#\n" };

	for (uint8_t i = 0; i < 12; i++) {
		lv_obj_t* btn = lv_btn_create(g_pContactAp.dialpad.grid);  // 注意：这里仍然是grid的子组件
		lv_obj_set_size(btn, 48, 48);
		if (i == 0) lv_obj_add_style(btn, &highlight_style, 0);
		lv_obj_set_style_radius(btn, 90, 0);
		lv_obj_set_style_bg_color(btn, lv_color_hex(0x212121), 0);
		lv_obj_set_style_shadow_width(btn, 0, 0);

		lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_CENTER, i % 3, 1,
			LV_GRID_ALIGN_CENTER, i / 3, 1);

		lv_obj_t* label = lv_label_create(btn);
		lv_label_set_text(label, btn_map[i]);
		lv_obj_set_style_text_color(label, lv_color_hex(TEXT_COLOR), 0);
		lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
		lv_label_set_recolor(label, true);
		lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
		lv_obj_center(label);
		lv_obj_set_y(label, 15);
		lv_obj_add_event_cb(btn, num_button_cb, LV_EVENT_CLICKED, NULL);
	};


	lv_obj_t* call_btn = lv_img_create(parent); 
	lv_img_dsc_t* img = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_DIAL_CALL, IMG_FREE_TYPE_AUTO, NULL);
	lv_obj_align(call_btn, LV_ALIGN_BOTTOM_MID, 0, -15);
	lv_img_set_src(call_btn, img);
	lv_obj_add_flag(call_btn, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(call_btn, on_call_button_clicked, LV_EVENT_CLICKED, NULL);

	
	lv_obj_t* del_btn = lv_img_create(parent);
	lv_img_dsc_t* del_img = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_DIAL_DEL, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(del_btn, del_img);
	lv_obj_align(del_btn, LV_ALIGN_BOTTOM_RIGHT, -62, -30);
	lv_obj_add_flag(del_btn, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(del_btn, clear_cb, LV_EVENT_CLICKED, NULL);
}

void create_navigation(void) {

	if (g_pContactAp.nav.nav_bar)
	{
		lv_obj_del(g_pContactAp.nav.nav_bar);
		g_pContactAp.nav.nav_bar = NULL;
	}
	g_pContactAp.nav.nav_bar = lv_obj_create(g_pContactAp.root);
	lv_obj_set_size(g_pContactAp.nav.nav_bar, LV_PCT(100), 64);
	lv_obj_align(g_pContactAp.nav.nav_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_set_style_border_width(g_pContactAp.nav.nav_bar, 0,0);
	lv_obj_set_style_bg_color(g_pContactAp.nav.nav_bar, lv_color_hex(TITLE_BG_COLOR), 0);
	//lv_obj_set_flex_flow(nav_bar, LV_FLEX_FLOW_ROW);
	//lv_obj_set_flex_align(nav_bar, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);


	g_pContactAp.nav.ADD_res = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_ADDRESS_ICON_FOCUS, IMG_FREE_TYPE_AUTO, NULL);
	
	g_pContactAp.nav.contact_btn = lv_btn_create(g_pContactAp.nav.nav_bar);
	lv_obj_align(g_pContactAp.nav.contact_btn, LV_ALIGN_TOP_LEFT,82,8);
	lv_obj_add_event_cb(g_pContactAp.nav.contact_btn, switch_contact, LV_EVENT_CLICKED, NULL);
	g_pContactAp.nav.contact_img = lv_img_create(g_pContactAp.nav.contact_btn);
	lv_img_set_src(g_pContactAp.nav.contact_img, g_pContactAp.nav.ADD_res);
	lv_obj_add_style(g_pContactAp.nav.contact_btn, get_global_style(STYLE_BTN_OPA), LV_PART_MAIN | LV_STATE_DEFAULT);


	g_pContactAp.nav.contact_text = lv_label_create(g_pContactAp.nav.nav_bar);
#ifndef C_PC_SIM
	lv_label_set_text(contact_text, "通讯录");
	lv_label_set_cjk_font(contact_text, 14, FT_FONT_STYLE_NORMAL);
#else
lv_label_set_text(g_pContactAp.nav.contact_text, "address");
lv_obj_set_style_text_font(g_pContactAp.nav.contact_text, &lv_font_montserrat_14, 0);
#endif
lv_obj_set_style_text_color(g_pContactAp.nav.contact_text, lv_color_hex(0x008CFF),0);
	lv_obj_align_to(g_pContactAp.nav.contact_text, g_pContactAp.nav.contact_btn, LV_ALIGN_OUT_BOTTOM_MID,0,0);





	g_pContactAp.nav.DUALPAD_res = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_DIAL_ICON_UNFOUCS, IMG_FREE_TYPE_AUTO, NULL);
	g_pContactAp.nav.dialpad_btn = lv_btn_create(g_pContactAp.nav.nav_bar);
	lv_obj_align(g_pContactAp.nav.dialpad_btn, LV_ALIGN_TOP_LEFT, 203, 8);
	lv_obj_add_style(g_pContactAp.nav.dialpad_btn, &nav_active_style, 0); // 初始选中样式
	lv_obj_add_event_cb(g_pContactAp.nav.dialpad_btn, switch_dialpad, LV_EVENT_CLICKED, NULL);
	g_pContactAp.nav.dialpad_img = lv_img_create(g_pContactAp.nav.dialpad_btn);
	lv_img_set_src(g_pContactAp.nav.dialpad_img, g_pContactAp.nav.DUALPAD_res);
	lv_obj_add_style(g_pContactAp.nav.dialpad_btn, get_global_style(STYLE_BTN_OPA), LV_PART_MAIN | LV_STATE_DEFAULT);

	g_pContactAp.nav.dialpad_text = lv_label_create(g_pContactAp.nav.nav_bar);
#ifndef C_PC_SIM
	lv_label_set_text(dialpad_text, "拨号键盘");
#else
	lv_label_set_text(g_pContactAp.nav.dialpad_text, "dialpad");
	lv_obj_set_style_text_font(g_pContactAp.nav.dialpad_text, &lv_font_montserrat_14, 0);
#endif
	lv_obj_set_style_text_color(g_pContactAp.nav.dialpad_text, lv_color_hex(0x666666), 0);
	lv_obj_align_to(g_pContactAp.nav.dialpad_text, g_pContactAp.nav.dialpad_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

}


// 定义无内容屏幕事件处理
static void _tbcd_contentList_scr_cb(lv_event_t* evt) {
	lv_obj_t* obj = lv_event_get_target(evt);
	int keyVal = 0;
	lv_event_code_t evtCode = evt->code;

	if (LV_EVENT_KEY_STAGE == evt->code) {
		keyVal = lv_event_get_key(evt);
		if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_REPEAT) {
			keyVal &= ~LV_EX_FLAG_KEY_MASK;
			evtCode = LV_EVENT_KEY;
		}
	}

	switch (evtCode) {
	case LV_EVENT_KEY_STAGE:
		if ((LV_EX_FLAG_KEY_MASK & keyVal) != LV_EX_FLAG_KEY_SHORT)
			break;
		keyVal &= 0xff;
		switch (keyVal) {
		case LV_KEY_ESC:
			zb_requestSwitchAp(C_ZB_AP_MAINMENU);
			LV_LOG_USER("esc mainmenu\r\n");
			lv_event_stop_processing(evt);
			break;


#ifdef C_PC_SIM
		case LV_KEY_EX_FUNC1:
		{
			//tbcd_ready_call_scr_init(0); //拨电话and编辑弹窗
 			//tbcd_incoming_call_scr_init(0);//来电弹窗
			//tbcd_contact_has_phone("17687457221", &popup_contact);
			//tbcd_sos_scr_init(0); //紧急呼救
			
			tbcd_calling_scr_init(EVENT_CALL_OUT);
			LV_LOG_USER("LV_KEY_EX_FUNC1\r\n");
			break;
		}
#endif


		default:
			break;
		}
		break;
	case LV_EVENT_DELETE:	
		g_pContactAp.root = NULL;
		break;
	default:
		break;
	}
}



//添加联系人按钮回调
static void _tbcd_create_contact_cb(lv_event_t* evt) {
	lv_obj_t* obj = lv_event_get_target(evt);

	if (1)//根据文件中的联系人进行判断
	{
		LV_LOG_USER("enter create_contact_cb");
		g_pContactAp.addContact.scrStat = 1;
		tbcd_switchScr(TBCD_SCR_ADD_CONTACT, g_pContactAp.addContact.scrStat);
	}
	else
	{

	}
}

static void _tbcd_return_cb(lv_event_t* evt) {
	lv_obj_t* obj = lv_event_get_target(evt);

	if (g_scrNo == 1)
	{
		tbcd_switchScr(TBCD_SCR_NO_CONTENT, 0);
	}
	else 
	{
		zb_requestSwitchAp(C_ZB_AP_MAINMENU);
	}
}

/* 事件处理（需在循环外定义静态函数）*/
static lv_anim_t scroll_anim;
static lv_coord_t target_scroll_x;

static void scroll_end_handler(lv_event_t* e) {
	lv_obj_t* obj = lv_event_get_target(e);
	lv_coord_t scroll_x = lv_obj_get_scroll_x(obj);
	lv_coord_t max_scroll = IMG_WIDTH-3;

	if (scroll_x > max_scroll / 2) {
		target_scroll_x = max_scroll; // 显示图片
	}
	else {
		target_scroll_x = 0; // 隐藏图片
	}

	// 使用动画平滑滚动
	lv_anim_init(&scroll_anim);
	lv_anim_set_var(&scroll_anim, obj);

	lv_anim_set_exec_cb(&scroll_anim, (lv_anim_exec_xcb_t)lv_obj_set_scroll_snap_x);
	lv_anim_set_values(&scroll_anim, scroll_x, target_scroll_x);
	lv_anim_set_time(&scroll_anim, 300);
	lv_anim_set_path_cb(&scroll_anim, lv_anim_path_ease_out);
	lv_anim_start(&scroll_anim);
}

static void scroll_handler(lv_event_t* e) {
	lv_obj_t* obj = lv_event_get_target(e);
	lv_coord_t scroll_x = lv_obj_get_scroll_x(obj);
	lv_coord_t max_scroll = IMG_WIDTH-3;

	// 实时限制滚动范围
	if (scroll_x < 0) {
		lv_obj_set_scroll_snap_x(obj, 0);
	}
	else if (scroll_x > max_scroll) {
		lv_obj_set_scroll_snap_x(obj, max_scroll);
	}
}

static void  contact_click_handler(lv_event_t* e)
{
	lv_obj_t* obj = lv_event_get_target(e);
	ContactItem* ci = lv_event_get_user_data(e);  // 直接获取用户数据

	LV_LOG_USER("contact_click_handler\r\n");
	if (!ci || !ci->data) {
		LV_LOG_ERROR("Invalid contact item data");
		return;
	}

	if (ci->data && &new_contact) {
		
		memcpy(&new_contact, ci->data, sizeof(TBCD_CONTACT_RECORD_ITEM));
		LV_LOG_USER("Clicked contact: %s, Phone: %s", ci->data->name, ci->data->phone);

		tbcd_ready_call_scr_init(0);
	}
}


static void del_click_handler(lv_event_t* e) {
	// 获取要删除的联系人索引
	int delete_index = -1;
	ContactItem* ci = lv_event_get_user_data(e);
	if (!ci || !ci->data) return;

	// 遍历全局数组查找匹配项
	for (int i = 0; i < g_pContactAp.contact_count; i++) {
		if (strcmp(ci->data->phone, g_pContactAp.contacts[i].phone) == 0 &&
			strcmp(ci->data->name, g_pContactAp.contacts[i].name) == 0) {
			delete_index = i;
			break;
		}
	}

	if (delete_index == -1) return;

	// 数据前移（使用memmove处理可能的重叠区域）
	if (delete_index < g_pContactAp.contact_count - 1) {
		memmove(&g_pContactAp.contacts[delete_index],
			&g_pContactAp.contacts[delete_index + 1],
			(g_pContactAp.contact_count - delete_index - 1) * sizeof(TBCD_CONTACT_RECORD_ITEM));
	}

	g_pContactAp.contact_count--;

	// 保存到文件
	if (tbcd_save_contact_record(CONTACT_FILE_PATH) != 0) {
		ESP_LOGE(TAG, "删除后保存失败");
	}

	tbcd_switchScr(TBCD_SCR_NO_CONTENT, 0);
}

static ContactItem* create_contact_item(lv_obj_t* parent, TBCD_CONTACT_RECORD_ITEM* contact) {
	// 参数检查
	if (!parent || !contact) {
		ESP_LOGE(TAG, "Invalid parameters to create_contact_item");
		return NULL;
	}

	if (contact->name == NULL && contact->phone == NULL && contact->num_img == NULL)
	{
		ESP_LOGE(TAG, "NAME OR PHONE IS NULL \r\n");
		return NULL;
	}

	// 内存分配
	ContactItem* ci = calloc(1, sizeof(ContactItem));
	if (!ci) {
		ESP_LOGE(TAG, "Failed to allocate memory for ContactItem");
		return NULL;
	}

	ci->data = contact;

	// 创建主容器
	ci->item = lv_obj_create(parent);
	if (!ci->item) {
		ESP_LOGE(TAG, "Failed to create contact item");
		free(ci);
		return NULL;
	}

	// 初始化主容器样式
	lv_obj_remove_style_all(ci->item);
	lv_obj_set_size(ci->item, lv_pct(92), CONTACT_ITEM_HEIGHT);
	//lv_obj_set_flex_flow(ci->item, LV_FLEX_FLOW_ROW);
    lv_obj_set_scroll_dir(ci->item, LV_DIR_HOR);
	lv_obj_set_style_radius(ci->item, ITEM_RADIUS, 0);
	lv_obj_set_style_base_dir(ci->item, LV_BASE_DIR_LTR, 0);
	lv_obj_set_flex_align(ci->item, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	// 创建内容区域
	lv_obj_t* content = lv_obj_create(ci->item);
	if (!content) {
		ESP_LOGE(TAG, "Failed to create content area");
		lv_obj_del(ci->item);
		free(ci);
		return NULL;
	}

	// 内容区域样式
	lv_obj_set_size(content, lv_pct(100), CONTENT_HEIGHT);
	lv_obj_set_style_bg_color(content, lv_color_hex(0x212121), LV_PART_MAIN);
	lv_obj_set_style_border_width(content, 0, 0);


	uint8_t num = ci->data->num_img;
	lv_obj_t* icon = lv_img_create(content);
	lv_img_dsc_t* icon_res;
	if (num == 1) icon_res = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_ADDLIST_MUM, IMG_FREE_TYPE_AUTO, NULL);
	else if (num == 2) icon_res = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_ADDLIST_DAD, IMG_FREE_TYPE_AUTO, NULL);
	else   icon_res = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_ADDLIST_UNKNOW, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(icon, icon_res);
	lv_obj_align(icon, LV_ALIGN_LEFT_MID,16,0);

	// 创建文本标签
	ci->label = lv_label_create(content);
	if (!ci->label) {
		ESP_LOGE(TAG, "Failed to create label");
		lv_obj_del(ci->item);
		free(ci);
		return NULL;
	}



	lv_obj_set_width(ci->label, LV_PCT(30));
	lv_obj_align_to(ci->label, icon, LV_ALIGN_OUT_RIGHT_MID,16,0);
	lv_label_set_text_fmt(ci->label, "  %s", contact->name);
	lv_obj_set_style_text_color(ci->label, lv_color_hex(0XFFFFFF), LV_STATE_DEFAULT);



	// 创建删除按钮
	lv_obj_t* del_btn = lv_img_create(ci->item);
	if (!del_btn) {
		ESP_LOGE(TAG, "Failed to create delete button");
		lv_obj_del(ci->item);
		free(ci);
		return NULL;
	}

	lv_img_dsc_t* del_img = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_DEL_CONNECT, IMG_FREE_TYPE_AUTO, NULL);
	if (!del_img) {
		ESP_LOGE(TAG, "Failed to load delete image");
		lv_obj_del(ci->item);
		free(ci);
		return NULL;
	}

	lv_img_set_src(del_btn, del_img);
	lv_obj_set_size(del_btn, IMG_WIDTH, CONTENT_HEIGHT);
	lv_obj_add_flag(del_btn, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_style(del_btn, &style_clickBtn, LV_STATE_PRESSED);
	if (ci)
	{
		lv_obj_add_event_cb(content, contact_click_handler, LV_EVENT_CLICKED, ci);
		lv_obj_add_event_cb(del_btn, del_click_handler, LV_EVENT_CLICKED, ci);
	}
	lv_obj_add_event_cb(ci->item, scroll_handler, LV_EVENT_SCROLL, NULL);
	lv_obj_add_event_cb(ci->item, scroll_end_handler, LV_EVENT_SCROLL_END, NULL);

	return ci;
}

#define test_file 0

// 定义无内容屏幕初始化
static int tbcd_no_contentList_scr_init(int param) {

	setup_ui(lv_scr_act());
	return 0;


	if (g_pContactAp.root)
	{
		lv_obj_del(g_pContactAp.root);
		g_pContactAp.root = NULL;
	}

	lv_obj_t* bgObj = lv_obj_create(lv_scr_act());
	lv_obj_set_size(bgObj, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_pad_all(bgObj, 0, 0);
	lv_obj_set_style_border_width(bgObj, 0, 0);
	lv_obj_set_style_bg_color(bgObj, lv_color_hex(TITLE_BG_COLOR), 0);
	lv_obj_set_style_radius(bgObj, 0, 0);
	g_pContactAp.root = bgObj;


	g_pContactAp.g_tabview = lv_tabview_create(bgObj, LV_DIR_TOP, 0);
	lv_obj_set_style_bg_opa(g_pContactAp.g_tabview, LV_OPA_0, LV_PART_MAIN);
	lv_obj_set_style_border_opa(g_pContactAp.g_tabview, LV_OPA_0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(g_pContactAp.g_tabview, 0, 0);
	lv_obj_set_style_border_width(g_pContactAp.g_tabview, 0, 0);
	lv_obj_set_size(g_pContactAp.g_tabview, SCREEN_WIDTH, 416);
	lv_obj_align(g_pContactAp.g_tabview, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_add_event_cb(g_pContactAp.g_tabview, _tabvw_valChg_EvtCb, LV_EVENT_VALUE_CHANGED, NULL);
	for (int i = 0; i < 2; i++) {
		g_app_views[i].root = lv_tabview_add_tab(g_pContactAp.g_tabview, "");
		g_app_views[i].is_created = false;

		lv_obj_set_style_bg_opa(g_app_views[i].root, LV_OPA_0, LV_PART_MAIN);
	}
	
	// 加载联系人数据 ,应该设置为结构体变量
	int validContactN = tbcd_load_contact_record(CONTACT_FILE_PATH);
	LV_LOG_USER("g_pContactAp.contact_count: %d", g_pContactAp.contact_count);
	ESP_LOGI(TAG, "%s C, contactN %d", __func__, validContactN);

	int num_contacts = 1; 




	lv_obj_add_event_cb(bgObj, _tbcd_contentList_scr_cb, LV_EVENT_ALL, NULL);
	lv_group_add_obj(lv_group_get_default(), bgObj);
	sys_setCurMainWinAndCb(bgObj, _tbcd_contentList_scr_cb);
	
	create_contact_list_screen(g_app_views[0].root);

	create_dialpad_screen(g_app_views[1].root);

	create_navigation();

	return 0;
}

void ready_call_scr_del(void) 
{
	LV_LOG_USER("del A");

	if (g_popUp_compile) {
		if (g_popUp_compile->popup) lv_obj_del(g_popUp_compile->popup);
		g_popUp_compile->popup = NULL;
		heap_caps_free(g_popUp_compile);
		g_popUp_compile = NULL;
	}
#if 0
	if (sys_group) {
		lv_group_del(sys_group); 
		sys_group = NULL;       
	}
#else //没用
	if (sys_group) {
		lv_indev_t* indev = lv_indev_get_next(NULL);
		while (indev) {
			if (lv_event_get_indev(indev) == sys_group) {
				lv_indev_set_group(indev, NULL);  // 解绑输入设备
			}
			indev = lv_indev_get_next(indev);
		}
		lv_group_del(sys_group);
		sys_group = NULL;
		lv_indev_reset(NULL, lv_indev_get_act());
}
#endif

	
	LV_LOG_USER("ready_call_scr_delete Z\n");
}
static void callBtn_cb()
{
	ready_call_scr_del();

	tbcd_calling_scr_init(EVENT_CALL_OUT);
}

static void compileBtn_cb()
{

	g_pContactAp.addContact.scrStat = 0;
	tbcd_switchScr(TBCD_SCR_ADD_CONTACT, g_pContactAp.addContact.scrStat);
}


static void bg_click_cb(lv_event_t* e) {  //暂定弹窗形式的页面都可以触发这个
	lv_obj_t* bgObj = lv_event_get_target(e);
	lv_obj_t* rect = (lv_obj_t*)lv_obj_get_user_data(bgObj);

	lv_point_t click_pos;
	lv_indev_get_point(lv_indev_get_act(), &click_pos);

	lv_point_t pos_in_rect;
	lv_obj_t* parent = lv_obj_get_parent(rect);
	lv_area_t rect_coords;
	lv_obj_get_coords(rect, &rect_coords);

	pos_in_rect.x = click_pos.x - rect_coords.x1;
	pos_in_rect.y = click_pos.y - rect_coords.y1;
	

	if (!_lv_area_is_point_on(&rect_coords, &click_pos, 0)) {
		incoming_call_scr_del();
		ready_call_scr_del();
	}
}


static void _tbcd_ready_call_scr_cb(lv_event_t* evt)
{
	lv_obj_t* obj = lv_event_get_target(evt);
	int keyVal = 0;
	lv_event_code_t evtCode = evt->code;

	if (LV_EVENT_KEY_STAGE == evt->code) {
		keyVal = lv_event_get_key(evt);
		if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_REPEAT) {
			keyVal &= ~LV_EX_FLAG_KEY_MASK;
			evtCode = LV_EVENT_KEY;
		}
	}

	switch (evtCode) {
	case LV_EVENT_KEY_STAGE:
		if ((LV_EX_FLAG_KEY_MASK & keyVal) != LV_EX_FLAG_KEY_SHORT)
			break;
		keyVal &= 0xff;
		switch (keyVal) {
		case LV_KEY_ESC:
			if (sys_group && zb_is_groupOnTopLayer(sys_group)) {
				ready_call_scr_del();
			}
			lv_event_stop_processing(evt);
			lv_indev_keypad_wait_release();

			//sys_setCurMainWinObj(g_pContactAp.root);
			break;
		case LV_KEY_ENTER:

			break;
#ifdef C_PC_SIM

		case LV_KEY_EX_FUNC1:
		{
			tbcd_incoming_call_scr_init(0);//来电弹窗 
			//tbcd_sos_scr_init(0); //紧急呼救
			break;
		}
#endif
		default:
			break;
		}
		break;
	case LV_EVENT_DELETE:
		if (sys_group != NULL)
		{
			lv_group_del(sys_group);  sys_group = NULL;
		}
		zb_group_pop(false);
		//g_pContactAp.root = NULL;
		break;
	default:
		break;
	}
}



// 定义编辑/拨电话 弹窗初始化
int tbcd_ready_call_scr_init(int param) {   //点击联系人后触发  

	if (g_popUp_compile) {
		if (g_popUp_compile->popup) lv_obj_del(g_popUp_compile->popup);
		heap_caps_free(g_popUp_compile);
		g_popUp_compile = NULL;
	}
	// 分配新内存
	g_popUp_compile = (TBCD_POP_UP_INF*)heap_caps_malloc_dflt(sizeof(TBCD_POP_UP_INF));
	memset(g_popUp_compile, 0, sizeof(TBCD_POP_UP_INF)); 

	lv_obj_t* bgObj = lv_obj_create(g_pContactAp.root);
	//lv_obj_t* bgObj = lv_obj_create(lv_layer_top());  //不一定要顶层，因为还需要通话弹窗  这个可以直接在联系人页面生成
	//bgObj->flags |= LV_OBJ_FLAG_OUT_OF_DEF_GROUP;
	lv_obj_set_size(bgObj, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_scroll_dir(bgObj, LV_DIR_NONE);
	lv_obj_set_style_pad_all(bgObj, 0, 0);
	lv_obj_set_style_radius(bgObj, 0, 0);
	lv_obj_set_style_border_width(bgObj, 0, 0);
	lv_obj_set_style_bg_color(bgObj, lv_color_hex(TBWD_SOS_BG_COLOR), 0);
	g_popUp_compile->popup = bgObj;

	// Make the background clickable
	lv_obj_add_flag(bgObj, LV_OBJ_FLAG_CLICKABLE);


	static lv_style_t style;
	lv_style_init(&style);


	lv_style_set_bg_opa(&style, LV_OPA_70); 
	lv_obj_add_style(bgObj, &style, 0);

	lv_style_init(&style_clickBtn);
	custom_style_set_img_recolor(&style_clickBtn);

	lv_obj_t* rect = lv_obj_create(bgObj);
	lv_obj_set_size(rect, SCREEN_WIDTH+100, SCREEN_WIDTH + 100); 
	lv_obj_set_pos(rect, -50, 300); 
	lv_obj_set_style_bg_color(rect, lv_color_hex(0xFFFFFF), LV_PART_MAIN); 
	lv_obj_set_style_radius(rect, 360, LV_PART_MAIN); 

	lv_obj_set_user_data(bgObj, rect);

	lv_obj_add_event_cb(bgObj, bg_click_cb, LV_EVENT_CLICKED, NULL);


	lv_obj_t* btnAnswer = lv_img_create(rect);
	lv_img_dsc_t* btnAnswer_img = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_COMPILE, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(btnAnswer, btnAnswer_img);
	lv_obj_set_style_pad_top(btnAnswer, 10, 0);
	lv_obj_align(btnAnswer, LV_ALIGN_TOP_MID, -40, 40);
	lv_obj_add_flag(btnAnswer, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_style(btnAnswer, &style_clickBtn, LV_STATE_PRESSED);
	lv_obj_add_event_cb(btnAnswer, compileBtn_cb, LV_EVENT_CLICKED, NULL);

	lv_obj_t* btnAnswer_labelName = lv_label_create(rect);
	lv_label_set_text(btnAnswer_labelName, "编辑");
	lv_obj_align_to(btnAnswer_labelName, btnAnswer, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);


	//打电话(变量名有误，需要修改)
	lv_obj_t* btnHangup = lv_img_create(rect);
	lv_img_dsc_t* btnHangup_img = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_PHONECALL, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(btnHangup, btnHangup_img);
	lv_obj_set_style_pad_top(btnHangup, 10, 0);
	lv_obj_align(btnHangup, LV_ALIGN_TOP_MID, 40, 40);
	lv_obj_add_flag(btnHangup, LV_OBJ_FLAG_CLICKABLE);

	lv_obj_add_style(btnHangup, &style_clickBtn, LV_STATE_PRESSED);
	lv_obj_add_event_cb(btnHangup, on_click_item, LV_EVENT_CLICKED, "call");
	//lv_obj_add_event_cb(btnHangup, callBtn_cb, LV_EVENT_CLICKED, NULL);


	lv_obj_t* btnHangup_labelName = lv_label_create(rect);
	lv_label_set_text(btnHangup_labelName, "打电话");
	lv_obj_align_to(btnHangup_labelName, btnHangup, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

#if 1
	if (sys_group) {
		lv_group_remove_all_objs(sys_group);
		lv_group_del(sys_group);         
		sys_group = NULL;                   
	}
	sys_group = lv_group_create();
	zb_group_push(sys_group, 0);
	lv_group_add_obj(sys_group, bgObj);
	lv_group_focus_obj(rect);
#else
	lv_group_add_obj(lv_group_get_default(), bgObj);
	sys_setCurMainWinObj(bgObj);
#endif
	lv_obj_add_event_cb(bgObj, _tbcd_ready_call_scr_cb, LV_EVENT_ALL, NULL);
	return 0;
}

#if 0
static void dropdown_event_handler(lv_event_t* e) {
	lv_obj_t* dropdown = lv_event_get_target(e);
	uint16_t selected = lv_dropdown_get_selected(dropdown);
	char buf[32];
	lv_dropdown_get_selected_str(dropdown, buf, sizeof(buf));
	LV_LOG_USER("选中索引：%d, 文本：%s", selected, buf);
}
#else
static void dropdown_event_handler(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED) {
		uint32_t sel = lv_dropdown_get_selected(obj);
		char buf[32];
		lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
		LV_LOG_USER("选中索引：%d, 文本：%s", sel, buf);
	}
}
#endif
static void phone_input_event_cb(lv_event_t* e) {
	lv_obj_t* ta = lv_event_get_target(e);  
	const char* text = lv_textarea_get_text(ta);  

	// 过滤非数字字符
	char* p = (char*)text;
	while (*p) {
		if (!isdigit(*p)) {
			lv_textarea_set_text(ta, "");  
			break;
		}
		p++;
	}

	if ((text == NULL ||strlen(text) == 0)|| (g_pContactAp.addContact.selected_avatar_index == 0)) {

		//LV_LOG_USER("g_pContactAp.addContact.selected_avatar_index: %d", g_pContactAp.addContact.selected_avatar_index);
		lv_obj_set_style_bg_color(g_pContactAp.addContact.complete_btn, lv_color_hex(0x000000), LV_PART_MAIN);
		lv_obj_set_style_text_color(g_pContactAp.addContact.complete_btn, lv_color_hex(0xB6BFC9), LV_PART_MAIN);
		lv_obj_clear_flag(g_pContactAp.addContact.complete_btn, LV_OBJ_FLAG_CLICKABLE);  
	}
	else {
		//LV_LOG_USER("BLUE :g_pContactAp.addContact.selected_avatar_index: %d", g_pContactAp.addContact.selected_avatar_index);

		lv_obj_set_style_bg_color(g_pContactAp.addContact.complete_btn, lv_color_hex(0x000000), LV_PART_MAIN);
		lv_obj_set_style_text_color(g_pContactAp.addContact.complete_btn, lv_color_hex(0x008CFF), LV_PART_MAIN);
		lv_obj_add_flag(g_pContactAp.addContact.complete_btn, LV_OBJ_FLAG_CLICKABLE);  
	}
}

static void create_contact_cancel_cb(lv_event_t* e)
{
	tbcd_switchScr(TBCD_SCR_NO_CONTENT, 0);
}

static void create_contact_complete_cb(lv_event_t* e) {

	if (!g_pContactAp.addContact.complete_btn) {
		ESP_LOGE(TAG, "Invalid context");
		return;
	}

	if (g_pContactAp.contact_count >= TBCD_CONTACT_MAX) {
		ESP_LOGE(TAG, "联系人已满");
		return;
	}

	// 获取电话号码输入
	const char* phone = lv_textarea_get_text(g_pContactAp.addContact.phone_input);


	// 获取选中的姓名
	char selected_str[32] = { 0 };
	lv_dropdown_get_selected_str(g_pContactAp.addContact.dropdown,
		selected_str,
		sizeof(selected_str));


	uint8_t avatar_index = 0;
	// 验证头像索引有效性
	if (g_pContactAp.addContact.selected_avatar_index >= 3) {
		ESP_LOGE(TAG, "Invalid avatar index");
		
		avatar_index = 0;
	}
	else {
		avatar_index = g_pContactAp.addContact.selected_avatar_index;
	}


	if (g_pContactAp.addContact.scrStat)
	{
		TBCD_CONTACT_RECORD_ITEM*add_contact = &g_pContactAp.contacts[g_pContactAp.contact_count];

		strncpy(add_contact->name, selected_str, sizeof(add_contact->name) - 1);
		add_contact->name[sizeof(add_contact->name) - 1] = '\0';
		strncpy(add_contact->phone, phone, sizeof(add_contact->phone) - 1);
		add_contact->phone[sizeof(add_contact->phone) - 1] = '\0';
		add_contact->num_img = g_pContactAp.addContact.selected_avatar_index;

		g_pContactAp.contact_count++;

		if (tbcd_save_contact_record(CONTACT_FILE_PATH) != 0) {
			g_pContactAp.contact_count--;
			ESP_LOGE(TAG, "保存失败");
			return;
		}
	}
	else
	{
		
		if ((g_pContactAp.addContact.curCont_Index == -1)|| (g_pContactAp.addContact.curCont_Index >3))
			return;

		// 数据前移（使用memmove处理可能的重叠区域）
		if (g_pContactAp.addContact.curCont_Index < g_pContactAp.contact_count - 1) {
			memmove(&g_pContactAp.contacts[g_pContactAp.addContact.curCont_Index],
				&g_pContactAp.contacts[g_pContactAp.addContact.curCont_Index + 1],
				(g_pContactAp.contact_count - g_pContactAp.addContact.curCont_Index - 1) * sizeof(TBCD_CONTACT_RECORD_ITEM));
		}

		g_pContactAp.contact_count--;

		TBCD_CONTACT_RECORD_ITEM* add_contact = &g_pContactAp.contacts[g_pContactAp.contact_count];

		strncpy(add_contact->name, selected_str, sizeof(add_contact->name) - 1);
		add_contact->name[sizeof(add_contact->name) - 1] = '\0';
		strncpy(add_contact->phone, phone, sizeof(add_contact->phone) - 1);
		add_contact->phone[sizeof(add_contact->phone) - 1] = '\0';
		add_contact->num_img = g_pContactAp.addContact.selected_avatar_index;

		g_pContactAp.contact_count++;

		// 保存到文件
		if (tbcd_save_contact_record(CONTACT_FILE_PATH) != 0) {
			ESP_LOGE(TAG, "删除后保存失败");
		}
	}

	
	tbcd_switchScr(TBCD_SCR_NO_CONTENT, 0);
}


// 头像选中样式（蓝色边框）
static lv_style_t style_avatar_selected;
static void choose_img_style(void) {
	lv_style_init(&style_avatar_selected);
	lv_style_set_border_color(&style_avatar_selected, lv_color_hex(0x008CFF));
	lv_style_set_border_width(&style_avatar_selected, 2);
	lv_style_set_border_opa(&style_avatar_selected, LV_OPA_COVER);
	lv_style_set_radius(&style_avatar_selected,90);
}
static lv_style_t style_avatar_default;
// 头像默认样式（无边框或灰色边框）
static void unchoose_img_style(void) {
	lv_style_init(&style_avatar_default);
	lv_style_set_border_color(&style_avatar_default, lv_color_hex(0xE0E0E0));
	lv_style_set_border_width(&style_avatar_default, 0);
	lv_style_set_border_opa(&style_avatar_default, LV_OPA_COVER);
}

static void avatar_click_cb(lv_event_t* e) {
	lv_obj_t* img = lv_event_get_target(e);
	const char* phone = lv_textarea_get_text(g_pContactAp.addContact.phone_input);

	int8_t clicked_index = 0;
	for (int i =1; i < 4; i++) {
		if (img == g_pContactAp.addContact.avatars[i-1]) {
			clicked_index = i;
			//g_pContactAp.addContact.selected_avatar_index = clicked_index;
			LV_LOG_USER("clicked_index: %d", clicked_index);
			break;
		}
	}
	if (clicked_index == 0)
		return;


	if (g_pContactAp.addContact.selected_avatar_index == clicked_index) {
		lv_obj_remove_style(img, &style_avatar_selected, LV_PART_MAIN);
		g_pContactAp.addContact.selected_avatar_index = 0;

		if ((phone == NULL || strlen(phone) == 0) || (g_pContactAp.addContact.selected_avatar_index == 0)) {
			LV_LOG_USER("g_pContactAp.addContact.selected_avatar_index: %d", g_pContactAp.addContact.selected_avatar_index);
			lv_obj_set_style_bg_color(g_pContactAp.addContact.complete_btn, lv_color_hex(0x000000), LV_PART_MAIN);
			lv_obj_set_style_text_color(g_pContactAp.addContact.complete_btn, lv_color_hex(0xB6BFC9), LV_PART_MAIN);
			lv_obj_clear_flag(g_pContactAp.addContact.complete_btn, LV_OBJ_FLAG_CLICKABLE);
		}
		else {
			LV_LOG_USER("BLUE :g_pContactAp.addContact.selected_avatar_index: %d", g_pContactAp.addContact.selected_avatar_index);
			lv_obj_set_style_bg_color(g_pContactAp.addContact.complete_btn, lv_color_hex(0x000000), LV_PART_MAIN);
			lv_obj_set_style_text_color(g_pContactAp.addContact.complete_btn, lv_color_hex(0x008CFF), LV_PART_MAIN);
			lv_obj_add_flag(g_pContactAp.addContact.complete_btn, LV_OBJ_FLAG_CLICKABLE);
		}
		return;
	}

	if (g_pContactAp.addContact.selected_avatar_index > 0) {
		if (g_pContactAp.addContact.selected_avatar_index > 0)
		{
			lv_obj_remove_style(g_pContactAp.addContact.avatars[g_pContactAp.addContact.selected_avatar_index-1],
				&style_avatar_selected, LV_PART_MAIN);
		}
	}

	lv_obj_add_style(img, &style_avatar_selected, LV_PART_MAIN);
	g_pContactAp.addContact.selected_avatar_index = clicked_index;
	
	if ((phone == NULL || strlen(phone) == 0) || (g_pContactAp.addContact.selected_avatar_index == 0)) {
	
		//LV_LOG_USER("g_pContactAp.addContact.selected_avatar_index: %d", g_pContactAp.addContact.selected_avatar_index);
		lv_obj_set_style_bg_color(g_pContactAp.addContact.complete_btn, lv_color_hex(0x000000), LV_PART_MAIN);
		lv_obj_set_style_text_color(g_pContactAp.addContact.complete_btn, lv_color_hex(0xB6BFC9), LV_PART_MAIN);
		lv_obj_clear_flag(g_pContactAp.addContact.complete_btn, LV_OBJ_FLAG_CLICKABLE);
	}
	else {
		//LV_LOG_USER("BLUE :g_pContactAp.addContact.selected_avatar_index: %d", g_pContactAp.addContact.selected_avatar_index);
		lv_obj_set_style_bg_color(g_pContactAp.addContact.complete_btn, lv_color_hex(0x000000), LV_PART_MAIN);
		lv_obj_set_style_text_color(g_pContactAp.addContact.complete_btn, lv_color_hex(0x008CFF), LV_PART_MAIN);
		lv_obj_add_flag(g_pContactAp.addContact.complete_btn, LV_OBJ_FLAG_CLICKABLE);
	}
}


static void phone_focus_event_cb(lv_event_t* e) {
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* kb = g_pContactAp.addContact.keyboard;

	if (code == LV_EVENT_FOCUSED) {

		lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
	
		lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
	}
	else if (code == LV_EVENT_DEFOCUSED) {

		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}
}

static void keyboard_num_event_cb(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* ta = lv_event_get_target(e);
	lv_obj_t* kb = lv_event_get_user_data(e);
	LV_LOG("%s :[ %d] code: %d\n", __FUNCTION__, __LINE__, code);
	if (code == LV_EVENT_FOCUSED)
	{
		//lv_keyboard_set_textarea(kb, ta);
		//lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}

	if (code == LV_EVENT_DEFOCUSED) {

	}
	else if (code == LV_EVENT_CANCEL) { //

		LV_LOG("%s :[ %d] LV_EVENT_CANCEL ta: %p\n", __FUNCTION__, __LINE__, ta);
	}
	else if (code == LV_EVENT_READY) {//确认键，输入结束处理
		LV_LOG("%s :[ %d] LV_EVENT_READY ta: %p \n", __FUNCTION__, __LINE__, ta);
	
		lv_obj_add_flag(g_pContactAp.addContact.keyboard, LV_OBJ_FLAG_HIDDEN);

		lv_group_focus_obj(g_pContactAp.root);
	}
	else if (code == LV_EVENT_VALUE_CHANGED)
	{
		//char* psw_text = lv_textarea_get_text(ta);
		//LV_LOG("chars: %s \n", psw_text);
		//		LV_LOG("len: %s \n", strlen(psw_text));
	}
	lv_event_stop_processing(e);
}




// 定义联系人列表屏幕事件处理
static void _tbcd_add_contact_scr_cb(lv_event_t* evt) {
	lv_obj_t* obj = lv_event_get_target(evt);
	int keyVal = 0;
	lv_event_code_t evtCode = evt->code;

	if (LV_EVENT_KEY_STAGE == evt->code) {
		keyVal = lv_event_get_key(evt);
		if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_REPEAT) {
			keyVal &= ~LV_EX_FLAG_KEY_MASK;
			evtCode = LV_EVENT_KEY;
		}
	}

	switch (evtCode) {
	case LV_EVENT_KEY_STAGE:
		if ((LV_EX_FLAG_KEY_MASK & keyVal) != LV_EX_FLAG_KEY_SHORT)
			break;
		keyVal &= 0xff;
		switch (keyVal) {
		case LV_KEY_ESC:
			tbcd_switchScr(TBCD_SCR_NO_CONTENT, 0);  // 
			LV_LOG_USER("return TBCD_SCR_NO_CONTENT \r\n");
			lv_event_stop_processing(evt);
			break;
		case LV_KEY_ENTER:

			break;
		default:
			break;
		}
		break;
	case LV_EVENT_DELETE:
		g_pContactAp.root = NULL;
		break;
	default:
		break;
	}
}



/**
 * @brief 定义添加联系人屏幕初始化
 * @param 区别编辑/新增   1：新增  0：编辑
 */
int tbcd_add_contact_scr_init(int param) {

#define move_x_scr 0//20 
	if (g_pContactAp.root)
	{
		lv_obj_del(g_pContactAp.root);
		g_pContactAp.root = NULL;
	}

	g_pContactAp.addContact.scrStat = param;
	lv_obj_t* mainBg = lv_obj_create(lv_scr_act());
	lv_obj_set_size(mainBg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_center(mainBg);
	lv_obj_set_style_pad_all(mainBg, 0, 0);
#if DARK_MODE
	lv_obj_set_style_bg_color(mainBg, lv_color_black(), LV_PART_MAIN);
#else
	lv_obj_set_style_bg_color(mainBg, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
#endif
	
	lv_obj_set_style_bg_opa(mainBg, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_radius(mainBg, 0, LV_PART_MAIN);
	lv_obj_set_style_shadow_width(mainBg, 10, LV_PART_MAIN);
	lv_obj_set_style_border_width(mainBg, 0, LV_PART_MAIN);
	lv_obj_set_style_radius(mainBg, 0, LV_PART_MAIN);
	lv_obj_set_style_shadow_color(mainBg, lv_color_hex(0x000000), LV_PART_MAIN);
	lv_obj_set_scroll_dir(mainBg, LV_DIR_NONE);

	g_pContactAp.root = mainBg;

	g_pContactAp.addContact.selected_avatar_index = 0;
	choose_img_style();
	unchoose_img_style();

	// 创建标题栏
	lv_obj_t* title_bar = lv_obj_create(mainBg);
	lv_obj_set_size(title_bar, 320, 60);
	lv_obj_set_pos(title_bar, -move_x_scr, 0);
	lv_obj_set_style_border_width(title_bar, 0, 0);
#if DARK_MODE
	lv_obj_set_style_bg_color(title_bar, lv_color_black(), LV_PART_MAIN);
#else
	lv_obj_set_style_bg_color(title_bar, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
#endif
	lv_obj_set_scrollbar_mode(title_bar, LV_SCROLLBAR_MODE_OFF);

	// 取消按钮
	g_pContactAp.addContact.cancel_btn = lv_btn_create(title_bar);
	lv_obj_set_size(g_pContactAp.addContact.cancel_btn, 60, 40);
	lv_obj_align(g_pContactAp.addContact.cancel_btn, LV_ALIGN_TOP_LEFT, 0, 5);
	lv_obj_set_style_border_width(g_pContactAp.addContact.cancel_btn, 0, 0);
	lv_obj_set_style_bg_color(g_pContactAp.addContact.cancel_btn, lv_color_hex(0x152405), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(g_pContactAp.addContact.cancel_btn, LV_OPA_0, LV_PART_MAIN);

	lv_obj_set_style_outline_width(g_pContactAp.addContact.cancel_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_outline_pad(g_pContactAp.addContact.cancel_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(g_pContactAp.addContact.cancel_btn, lv_color_hex(0xB6BFC9), LV_PART_MAIN);
	lv_obj_set_style_shadow_width(g_pContactAp.addContact.cancel_btn, 0, 0);
	lv_obj_add_event_cb(g_pContactAp.addContact.cancel_btn, create_contact_cancel_cb, LV_EVENT_CLICKED, NULL);

	lv_obj_set_style_text_color(g_pContactAp.addContact.cancel_btn, lv_color_hex(0x000000), LV_PART_MAIN);
	lv_label_t* cancel_label = lv_label_create(g_pContactAp.addContact.cancel_btn);
	lv_label_set_text(cancel_label, "取消");
#if DARK_MODE
	lv_obj_set_style_text_color(cancel_label, lv_color_white(), LV_PART_MAIN);
#endif
	lv_obj_center(cancel_label);

	// 标题
	lv_obj_t* title_label = lv_label_create(title_bar);
	lv_label_set_text(title_label, "新建联系人");
	lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 110, 14);
#if DARK_MODE
	lv_obj_set_style_text_color(title_label, lv_color_white(), LV_PART_MAIN);
#else
	lv_obj_set_style_text_color(title_label, lv_color_hex(0x000000), LV_PART_MAIN);
#endif
	// 完成按钮
	g_pContactAp.addContact.complete_btn = lv_btn_create(title_bar);
	lv_obj_clear_flag(g_pContactAp.addContact.complete_btn, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_set_size(g_pContactAp.addContact.complete_btn, 60, 40);
	lv_obj_align(g_pContactAp.addContact.complete_btn, LV_ALIGN_TOP_RIGHT,-10, 5);
	lv_obj_set_style_border_width(g_pContactAp.addContact.complete_btn, 0, 0);
	lv_obj_set_style_bg_color(g_pContactAp.addContact.complete_btn, lv_color_hex(0x000000), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(g_pContactAp.addContact.complete_btn, LV_OPA_0, LV_PART_MAIN);
	lv_obj_set_style_outline_width(g_pContactAp.addContact.complete_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_outline_pad(g_pContactAp.addContact.complete_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(g_pContactAp.addContact.complete_btn, lv_color_hex(0xB6BFC9), LV_PART_MAIN);
	lv_obj_set_style_shadow_width(g_pContactAp.addContact.complete_btn, 0, 0);
	lv_obj_add_event_cb(g_pContactAp.addContact.complete_btn, create_contact_complete_cb, LV_EVENT_CLICKED, NULL);

	lv_label_t* complete_label = lv_label_create(g_pContactAp.addContact.complete_btn);
	lv_label_set_text(complete_label, "完成");
	lv_obj_center(complete_label);
	

	// 电话输入框
	lv_obj_t* phone_label = lv_label_create(mainBg);
	lv_label_set_text(phone_label, "电话");
	lv_obj_set_pos(phone_label, 20 - move_x_scr, 80);
	lv_obj_set_style_text_color(phone_label, lv_color_hex(0x007AFF), LV_PART_MAIN);

	g_pContactAp.addContact.phone_input = lv_textarea_create(mainBg);
	lv_textarea_set_one_line(g_pContactAp.addContact.phone_input, true);
	lv_textarea_set_accepted_chars(g_pContactAp.addContact.phone_input, "0123456789");
	lv_textarea_set_text_selection(g_pContactAp.addContact.phone_input, true);  // 可选：允许选中文本
	lv_obj_set_size(g_pContactAp.addContact.phone_input, 280, 40);
	lv_obj_set_pos(g_pContactAp.addContact.phone_input, 20 - move_x_scr, 110);
#if DARK_MODE
	lv_obj_set_style_bg_color(g_pContactAp.addContact.phone_input, lv_color_hex(0x212121), LV_PART_MAIN);
#else
	lv_obj_set_style_bg_color(g_pContactAp.addContact.phone_input, lv_color_hex(0xF2F2F7), LV_PART_MAIN);
#endif
	lv_obj_set_style_bg_opa(g_pContactAp.addContact.phone_input, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_radius(g_pContactAp.addContact.phone_input, 8, LV_PART_MAIN);
	lv_obj_set_style_border_width(g_pContactAp.addContact.phone_input, 0, 0);
	lv_textarea_set_placeholder_text(g_pContactAp.addContact.phone_input, "增加电话");

	// 关键修改：设置文本左对齐 + 垂直居中
	lv_obj_set_style_text_align(g_pContactAp.addContact.phone_input, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);  // 文本左对齐
	lv_obj_set_style_pad_left(g_pContactAp.addContact.phone_input, 10, LV_PART_MAIN);   // 左边距
	lv_obj_set_style_pad_right(g_pContactAp.addContact.phone_input, 10, LV_PART_MAIN);  // 右边距
	lv_obj_set_style_pad_top(g_pContactAp.addContact.phone_input, 12, LV_PART_MAIN);     // 上边距（调整垂直居中）

#if DARK_MODE
	lv_obj_set_style_text_color(g_pContactAp.addContact.phone_input, lv_color_white(), LV_PART_MAIN);

	lv_obj_set_style_text_color(g_pContactAp.addContact.phone_input, lv_color_hex(0x9E9E9E), LV_PART_TEXTAREA_PLACEHOLDER);

	lv_obj_set_style_bg_color(g_pContactAp.addContact.phone_input, lv_color_hex(0x212121), LV_PART_MAIN);


	lv_obj_set_style_radius(g_pContactAp.addContact.phone_input, 8, LV_PART_MAIN);

	lv_obj_set_style_border_width(g_pContactAp.addContact.phone_input, 1, LV_PART_MAIN);
	lv_obj_set_style_border_color(g_pContactAp.addContact.phone_input, lv_color_hex(0x424242), LV_PART_MAIN);
#endif


	lv_obj_add_flag(g_pContactAp.addContact.phone_input, LV_OBJ_FLAG_CLICK_FOCUSABLE);
	lv_obj_add_event_cb(g_pContactAp.addContact.phone_input, phone_focus_event_cb, LV_EVENT_FOCUSED, NULL);
	lv_obj_add_event_cb(g_pContactAp.addContact.phone_input, phone_focus_event_cb, LV_EVENT_DEFOCUSED, NULL);

	lv_obj_add_event_cb(g_pContactAp.addContact.phone_input, phone_input_event_cb, LV_EVENT_VALUE_CHANGED, NULL);




	lv_obj_t* name_label = lv_label_create(mainBg);
	lv_label_set_text(name_label, "姓名");
	lv_obj_set_pos(name_label, 20 - move_x_scr, 170);
	lv_obj_set_style_text_color(name_label, lv_color_hex(0x007AFF), LV_PART_MAIN);

	// 创建下拉列表
	g_pContactAp.addContact.dropdown = lv_dropdown_create(mainBg);
	lv_dropdown_set_options(g_pContactAp.addContact.dropdown, "妈妈\n爸爸\n奶奶\n爷爷\n叔叔\n阿姨\n紧急联络人");
	lv_obj_set_width(g_pContactAp.addContact.dropdown, SCREEN_WIDTH - 50);
	lv_obj_set_height(g_pContactAp.addContact.dropdown, 40);
	lv_obj_set_style_border_width(g_pContactAp.addContact.dropdown, 0, 0);
	lv_obj_set_style_radius(g_pContactAp.addContact.dropdown, 8, LV_PART_MAIN);

#if DARK_MODE
	// 设置暗黑模式样式
	lv_obj_set_style_bg_color(g_pContactAp.addContact.dropdown, lv_color_hex(0x212121), LV_PART_MAIN);
	lv_obj_set_style_text_color(g_pContactAp.addContact.dropdown, lv_color_white(), LV_PART_MAIN);

	// 设置下拉列表的暗黑模式样式
	lv_obj_t* list = lv_dropdown_get_list(g_pContactAp.addContact.dropdown);
	lv_obj_set_style_bg_color(list, lv_color_hex(0x212121), LV_PART_MAIN);
	lv_obj_set_style_text_color(list, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_border_width(list, 0, 0);

	// 设置选择器样式（滚动条等）
	lv_obj_set_style_bg_color(list, lv_color_hex(0x424242), LV_PART_SCROLLBAR);
#endif

	lv_obj_align_to(g_pContactAp.addContact.dropdown, name_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
	lv_obj_set_style_pad_top(g_pContactAp.addContact.dropdown, 12, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(g_pContactAp.addContact.dropdown, 12, LV_PART_MAIN);
	lv_obj_set_style_pad_left(g_pContactAp.addContact.dropdown, 10, 0);
	lv_obj_add_event_cb(g_pContactAp.addContact.dropdown, dropdown_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

	lv_obj_t* dropdownList = lv_dropdown_get_list(g_pContactAp.addContact.dropdown);

	if (dropdownList) {
		lv_obj_set_width(dropdownList, SCREEN_WIDTH - 50);         
		lv_obj_set_style_pad_row(dropdownList, 15, 0);           
		lv_obj_set_style_min_height(dropdownList, 100, 0);
		lv_obj_set_style_pad_top(dropdownList, 8, 0);   
		lv_obj_set_style_pad_bottom(dropdownList, 8, 0); 
		lv_obj_set_style_pad_left(dropdownList, 10, 0); 
	}




	// 头像选择区域
	lv_obj_t* avatar_label = lv_label_create(mainBg);
	lv_label_set_text(avatar_label, "选择头像");
	lv_obj_set_pos(avatar_label, 20 - move_x_scr, 260);
	lv_obj_set_style_text_color(avatar_label, lv_color_hex(0x007AFF), LV_PART_MAIN);

	// 头像容器
	lv_obj_t* avatar_container = lv_obj_create(mainBg);
	lv_obj_set_size(avatar_container, 350, 150);
	lv_obj_set_pos(avatar_container, 0-move_x_scr, 290);
	lv_obj_clear_flag(avatar_container, LV_OBJ_FLAG_SCROLLABLE);
#if DARK_MODE
	lv_obj_set_style_bg_color(avatar_container, lv_color_black(), LV_PART_MAIN);
#else
	lv_obj_set_style_bg_color(avatar_container, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
#endif
	lv_obj_set_style_border_color(avatar_container, lv_color_hex(0xE0E0E0), LV_PART_MAIN);
	lv_obj_set_style_border_width(avatar_container, 1, LV_PART_MAIN);
	lv_obj_set_style_radius(avatar_container, 8, LV_PART_MAIN);
	lv_obj_set_style_border_width(avatar_container, 0, 0);

	g_pContactAp.addContact.avatar_img[0] = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_MUM, IMG_FREE_TYPE_AUTO, NULL);
	g_pContactAp.addContact.avatar_img[1] = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_DAD, IMG_FREE_TYPE_AUTO, NULL);
	g_pContactAp.addContact.avatar_img[2] = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_UNKONW, IMG_FREE_TYPE_AUTO, NULL);

	for (int i = 0; i < 3; i++) {
		g_pContactAp.addContact.avatars[i] = lv_img_create(avatar_container);
		lv_obj_set_pos(g_pContactAp.addContact.avatars[i], 5 + i * 100, 15);
		lv_img_set_src(g_pContactAp.addContact.avatars[i], g_pContactAp.addContact.avatar_img[i]);
		lv_obj_add_style(g_pContactAp.addContact.avatars[i], &style_avatar_default, LV_PART_MAIN);
		lv_obj_add_flag(g_pContactAp.addContact.avatars[i], LV_OBJ_FLAG_CLICKABLE);
		lv_obj_add_event_cb(g_pContactAp.addContact.avatars[i], avatar_click_cb, LV_EVENT_CLICKED, NULL);
	}

	if (!param)
	{
		lv_textarea_set_text(g_pContactAp.addContact.phone_input, new_contact.phone);
		//lv_dropdown_set_text(g_pContactAp.addContact.dropdown, new_contact.name);

#if 1
		int selected_index = -1;
		const char* option_list[] = { "妈妈", "爸爸", "奶奶", "爷爷", "叔叔", "阿姨" };
		for (int i = 0; i < sizeof(option_list) / sizeof(option_list[0]); i++) {
			if (strcmp(new_contact.name, option_list[i]) == 0) {
				selected_index = i;
				break;
			}
		}
		if (selected_index != -1) {
			lv_dropdown_set_selected(g_pContactAp.addContact.dropdown, selected_index);
		}
#else
		LV_LOG_USER("new_contact.name :%s", new_contact.name);
		lv_dropdown_get_selected_str(g_pContactAp.addContact.dropdown, new_contact.name, sizeof(new_contact.name));
#endif
		lv_obj_add_style(g_pContactAp.addContact.avatars[new_contact.num_img - 1], &style_avatar_selected, LV_PART_MAIN);
		g_pContactAp.addContact.selected_avatar_index = new_contact.num_img;


		// 遍历全局数组查找匹配项
		for (int i = 0; i < g_pContactAp.contact_count; i++) {
			if (strcmp(new_contact.phone, g_pContactAp.contacts[i].phone) == 0) {
				g_pContactAp.addContact.curCont_Index = i;
				break;
			}
		}
	}


	g_pContactAp.addContact.keyboard = lv_keyboard_create(mainBg);
	lv_obj_set_style_text_font(g_pContactAp.addContact.keyboard, &lv_font_montserrat_16, 0);
	lv_keyboard_set_textarea(g_pContactAp.addContact.keyboard, g_pContactAp.addContact.phone_input);
	lv_obj_add_flag(g_pContactAp.addContact.keyboard, LV_OBJ_FLAG_HIDDEN);

	//键盘
	lv_obj_set_size(g_pContactAp.addContact.keyboard, SCREEN_WIDTH, 160);
	lv_obj_align(g_pContactAp.addContact.keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_set_style_radius(g_pContactAp.addContact.keyboard, 10, LV_PART_MAIN);
	lv_obj_set_style_bg_color(g_pContactAp.addContact.keyboard, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
	
	lv_keyboard_set_mode(g_pContactAp.addContact.keyboard, LV_KEYBOARD_MODE_NUMBER);
	lv_obj_add_event_cb(g_pContactAp.addContact.keyboard, keyboard_num_event_cb, LV_EVENT_ALL, g_pContactAp.addContact.keyboard);

	lv_group_add_obj(lv_group_get_default(), mainBg);
	sys_setCurMainWinObj(mainBg);
	lv_obj_add_event_cb(mainBg, _tbcd_add_contact_scr_cb, LV_EVENT_ALL, NULL);
	
	return 0;
}



// 定义拨打电话屏幕事件处理
static void _tbcd_calling_scr_cb(lv_event_t* evt) {
	lv_obj_t* obj = lv_event_get_target(evt);
	int keyVal = 0;
	lv_event_code_t evtCode = evt->code;

	if (LV_EVENT_KEY_STAGE == evt->code) {
		keyVal = lv_event_get_key(evt);
		if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_REPEAT) {
			keyVal &= ~LV_EX_FLAG_KEY_MASK;
			evtCode = LV_EVENT_KEY;
		}
	}

	switch (evtCode) {
	case LV_EVENT_KEY_STAGE:
		if ((LV_EX_FLAG_KEY_MASK & keyVal) != LV_EX_FLAG_KEY_SHORT)
			break;
		keyVal &= 0xff;
		switch (keyVal) {
		case LV_KEY_ESC:
			tbcd_switchScr(TBCD_SCR_NO_CONTENT, 0);//拨打电话退出后应该是退出到接电话前定位的位置
			lv_event_stop_processing(evt);
			break;
#ifdef C_PC_SIM
		case LV_KEY_EX_FUNC1:
			//free_calling_scr();
			tbcd_calling_scr_init(EVENT_ANSWER);
			break;
#endif
		default:
			break;
		}
		break;
	case LV_EVENT_DELETE:
		calling_strct.root = NULL;
		calling_strct.btnHangup = NULL;
		calling_strct.labelName = NULL;
		break;
	default:
		break;
	}
}

static void _btnHangup_click_cb(lv_event_t* evt) {  //
	// 终止所有定时器
	safe_delete_timer(&g_pContactAp.sos_timer);

	// 更新状态机
	g_pContactAp.sos_state = SOS_CALL_IDLE;

	// 执行挂断操作
#ifndef C_PC_SIM
	seed_phone_Nocall();
#endif
	if (calling_strct.call_timer) {
		lv_timer_del(calling_strct.call_timer);
		calling_strct.call_timer = NULL;
	}
	// 界面切换
	free_calling_scr();
}

static void format_call_time(char* buf, size_t buf_size) {
	uint32_t duration_ms = lv_tick_elaps(calling_strct.call_start_time);
	uint32_t seconds = duration_ms / 1000;
	uint32_t mins = seconds / 60;
	seconds %= 60;

	snprintf(buf, buf_size, "已通话:%02d:%02d", mins, seconds);
}

static void call_timer_cb(lv_timer_t* timer) {
	char time_str[16];
	format_call_time(time_str, sizeof(time_str));

	// 安全更新标签内容
	if (calling_strct.call_time_label && lv_obj_is_valid(calling_strct.call_time_label)) {
		lv_label_set_text(calling_strct.call_time_label, time_str);
	}
	else {
		lv_timer_del(timer);
	}
}

void free_calling_scr()
{
	LV_LOG_USER("free_calling_scr\r\n");
	if (calling_strct.call_timer) {
		lv_timer_del(calling_strct.call_timer);
		calling_strct.call_timer = NULL;
	}
	if (calling_strct.root) 
	{
		lv_obj_del(calling_strct.root);
		calling_strct.root = NULL;
	}
}
void update_calling_timer_create()
{
	if (calling_strct.root) {
		printf("tbcd_calling_scr_init G \n");
		calling_strct.call_start_time = lv_tick_get();
		if (calling_strct.call_timer) {
			lv_timer_del(calling_strct.call_timer);
			calling_strct.call_timer = NULL;
		}
		printf("tbcd_calling_scr_init H \n");
		calling_strct.call_timer = lv_timer_create(call_timer_cb, 1000, NULL);
	}
;
}


// 定义拨打电话屏幕初始化   参数设定为  0：拨打  1;来电  2:通话中
int tbcd_calling_scr_init(CallEvent param) {


		printf("enter tbcd_calling_scr_init \n");

		if (calling_strct.root)
		{
			lv_obj_del(calling_strct.root);
			calling_strct.root = NULL;
		}

		printf("calling_strct.root is not null \n");

		lv_obj_t* bgObj = lv_obj_create(lv_scr_act());
		lv_obj_set_size(bgObj, SCREEN_WIDTH, SCREEN_HEIGHT);
		lv_obj_set_style_pad_all(bgObj, 0, 0);
		lv_obj_set_style_radius(bgObj, 0, 0);
		lv_obj_clear_flag(bgObj, LV_OBJ_FLAG_SCROLLABLE);
		calling_strct.root = bgObj;
		printf("tbcd_calling_scr_init A \n");

		lv_obj_t* bg_obj = lv_img_create(bgObj);
		lv_img_dsc_t* bgObj_bg = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_BGCALLING, IMG_FREE_TYPE_AUTO, NULL);
		lv_img_set_src(bg_obj, bgObj_bg);
		printf("tbcd_calling_scr_init B \n");

		printf("tbcd_calling_scr_init C \n");
		lv_obj_t* labelName = lv_label_create(bgObj);
		lv_label_set_text(labelName, calling_contact.name);
		lv_obj_set_size(labelName, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
		lv_obj_align(labelName, LV_ALIGN_CENTER, 0, 30);
#ifndef C_PC_SIM
		lv_label_set_cjk_font(labelName, 40, FT_FONT_STYLE_BOLD);
#endif
		lv_obj_set_style_text_color(labelName, lv_color_hex(0x18191B), LV_STATE_DEFAULT);

		printf("tbcd_calling_scr_init D\n");
		//lv_obj_t* labelphone_num = lv_label_create(bgObj);
		//lv_label_set_text(labelphone_num, calling_contact.phone);
		//lv_obj_align(labelphone_num, LV_ALIGN_CENTER, 0, 50);
		printf("tbcd_calling_scr_init E \n");

		calling_strct.call_time_label = lv_label_create(bgObj);
		lv_obj_align(calling_strct.call_time_label, LV_ALIGN_CENTER, 0, -30);
		lv_label_set_text(calling_strct.call_time_label, "");
		lv_obj_set_style_text_color(calling_strct.call_time_label, lv_color_hex(0x9AA0A7), LV_STATE_DEFAULT);
		//lv_obj_set_style_text_font(calling_strct.call_time_label, &lv_font_montserrat_18, 0);

		if (param != 2)
		{
			lv_obj_t* label_remind = lv_label_create(bgObj);

			(param == 0 ? lv_label_set_text(label_remind, "正在呼叫联系人") : lv_label_set_text(label_remind, "来电人"));
			lv_obj_align(label_remind, LV_ALIGN_TOP_MID, 0, 212);
			lv_obj_set_style_text_align(label_remind, LV_TEXT_ALIGN_CENTER, 0);
			lv_obj_set_style_text_color(label_remind, lv_color_hex(0x9AA0A7), LV_STATE_DEFAULT);
#ifndef C_PC_SIM
			lv_label_set_cjk_font(label_remind, 18, FT_FONT_STYLE_NORMAL);
#endif


			lv_obj_t* btnHangup = lv_img_create(bgObj);
			lv_obj_set_width(btnHangup, LV_SIZE_CONTENT);
			lv_obj_set_height(btnHangup, LV_SIZE_CONTENT);
			if (param == 0)
			{
				lv_img_dsc_t* btnHangup_img = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_CALLING_HANDUP, IMG_FREE_TYPE_AUTO, NULL);
				lv_img_set_src(btnHangup, btnHangup_img);
				lv_obj_align(btnHangup, LV_ALIGN_BOTTOM_MID, 0, -20);
				lv_obj_add_flag(btnHangup, LV_OBJ_FLAG_CLICKABLE);
			}
			else
			{
				lv_img_dsc_t* btnHangup_img = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_HAND_UP, IMG_FREE_TYPE_AUTO, NULL);
				lv_img_set_src(btnHangup, btnHangup_img);
				lv_obj_align(btnHangup, LV_ALIGN_BOTTOM_MID, -50, -20);
				lv_obj_add_flag(btnHangup, LV_OBJ_FLAG_CLICKABLE);

				lv_obj_t* btnPickup = lv_img_create(bgObj);
				lv_img_dsc_t* btnPickup_img = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_PHONECALL, IMG_FREE_TYPE_AUTO, NULL);
				lv_img_set_src(btnPickup, btnPickup_img);
				lv_obj_set_width(btnPickup, LV_SIZE_CONTENT);
				lv_obj_set_height(btnPickup, LV_SIZE_CONTENT);

				lv_obj_align(btnPickup, LV_ALIGN_BOTTOM_MID, 50, -20);
				lv_obj_add_flag(btnPickup, LV_OBJ_FLAG_CLICKABLE);

				lv_obj_add_style(btnPickup, &style_clickBtn, LV_STATE_PRESSED);

				lv_obj_add_event_cb(btnPickup, on_click_item, LV_EVENT_CLICKED, "ask");
				lv_obj_add_event_cb(btnPickup, _btnHangup_click_cb, LV_EVENT_CLICKED, NULL);
			}


			lv_obj_add_style(btnHangup, &style_clickBtn, LV_STATE_PRESSED);

			lv_obj_add_event_cb(btnHangup, on_click_item, LV_EVENT_CLICKED, "nocall");
			lv_obj_add_event_cb(btnHangup, _btnHangup_click_cb, LV_EVENT_CLICKED, NULL);

		}
		else if (param == 2)
		{
			lv_obj_t* btnHangup = lv_img_create(bgObj);
			lv_img_dsc_t* btnHangup_img = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_CALLING_HANDUP, IMG_FREE_TYPE_AUTO, NULL);
			lv_img_set_src(btnHangup, btnHangup_img);
			lv_obj_set_width(btnHangup, LV_SIZE_CONTENT);
			lv_obj_set_height(btnHangup, LV_SIZE_CONTENT);

			lv_obj_align(btnHangup, LV_ALIGN_BOTTOM_MID, 0, -20);
			lv_obj_add_flag(btnHangup, LV_OBJ_FLAG_CLICKABLE);

			lv_obj_add_style(btnHangup, &style_clickBtn, LV_STATE_PRESSED);

			lv_obj_add_event_cb(btnHangup, on_click_item, LV_EVENT_CLICKED, "nocall");
			lv_obj_add_event_cb(btnHangup, _btnHangup_click_cb, LV_EVENT_CLICKED, NULL);

			printf("tbcd_calling_scr_init F \n");
			lv_label_set_text(calling_strct.call_time_label, "已通话:00:00");
			//lv_obj_set_style_text_font(calling_strct.call_time_label, &lv_font_montserrat_18, 0);
			printf("tbcd_calling_scr_init G \n");
			calling_strct.call_start_time = lv_tick_get();
			if (calling_strct.call_timer) {
				lv_timer_del(calling_strct.call_timer);
				calling_strct.call_timer = NULL;
			}
			printf("tbcd_calling_scr_init H \n");
			calling_strct.call_timer = lv_timer_create(call_timer_cb, 1000, NULL);
		}

	lv_obj_t* who = lv_img_create(bgObj);

	uint32_t img_res = 0;

	if (calling_contact.num_img == 1) {
		img_res = ADDRESSBOOK_IMG_BIG_MUM;
	}
	else if (calling_contact.num_img == 2) {
		img_res = ADDRESSBOOK_IMG_BIG_DAD;
	}
	else {
		img_res = ADDRESSBOOK_IMG_BIG_UNKONW;
	}
	lv_img_dsc_t* who_img = zb_img_res_load_one(NULL, img_res, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(who, who_img);
	lv_obj_set_width(who, LV_SIZE_CONTENT);
	lv_obj_set_height(who, LV_SIZE_CONTENT);
	lv_obj_align(who, LV_ALIGN_TOP_MID, 0, 50);


	printf("tbcd_calling_scr_init F \n");

	//挂电话
	printf("tbcd_calling_scr_init G \n");

	lv_obj_add_event_cb(bgObj, _tbcd_calling_scr_cb, LV_EVENT_ALL, NULL);
	lv_group_add_obj(lv_group_get_default(), bgObj);
	//sys_setCurMainWinObj(bgObj);
	sys_setCurMainWinAndCb(bgObj, _tbcd_calling_scr_cb);
	printf("tbcd_calling_scr_init H \n");
	return 0;
}

void incoming_call_scr_del(void)
{
	LV_LOG_USER("ready_call_scr_delete A\n");

	if (g_popUp_ask.popup)
	{
		lv_obj_del(g_popUp_ask.popup);
		g_popUp_ask.popup = NULL; // 置空
	}
	LV_LOG_USER("ready_call_scr_delete Z\n");
}

// 定义来电显示屏幕事件处理
static void _tbcd_incoming_call_scr_cb(lv_event_t* evt) {
	lv_obj_t* obj = lv_event_get_target(evt);
	int keyVal = 0;
	lv_event_code_t evtCode = evt->code;

	if (LV_EVENT_KEY_STAGE == evt->code) {
		keyVal = lv_event_get_key(evt);
		if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_REPEAT) {
			keyVal &= ~LV_EX_FLAG_KEY_MASK;
			evtCode = LV_EVENT_KEY;
		}
	}

	switch (evtCode) {
	case LV_EVENT_KEY_STAGE:
		if ((LV_EX_FLAG_KEY_MASK & keyVal) != LV_EX_FLAG_KEY_SHORT)
			break;
		keyVal &= 0xff;
		switch (keyVal) {
		case LV_KEY_ESC:
			incoming_call_scr_del();
			lv_event_stop_processing(evt);
			break;
		default:
			break;
		}
		break;
	case LV_EVENT_DELETE:
		if (sys_group != NULL)
		{
			lv_group_del(sys_group);  sys_group = NULL;
		}
		zb_group_pop(false);
		g_popUp_ask.popup = NULL;
		break;
	default:
		break;
	}
}

void _tbcd_no_pickup_cb()
{
	LV_LOG_USER("_tbcd_no_pickup_cb A\n");
	memset(&calling_contact,0, sizeof(TBCD_CONTACT_RECORD_ITEM));
	LV_LOG_USER("_tbcd_no_pickup_cb B\n");
	incoming_call_scr_del();
	LV_LOG_USER("_tbcd_no_pickup_cb C\n");
}


static void _tbcd_pickup_cb()  //
{
	LV_LOG_USER("ENTER PICKUP CB\r\n");
	// 停止超时检测
	if (g_pContactAp.sos_timer) {
		lv_timer_del(g_pContactAp.sos_timer);
		g_pContactAp.sos_timer = NULL;
	}

	// 更新状态为已接通
	g_pContactAp.sos_state = SOS_CALL_CONNECTED;

	// 正常通话界面
	tbcd_calling_scr_init(EVENT_ANSWER);
}


// 定义通话中来电显示屏幕初始化
int tbcd_incoming_call_scr_init(int param) {

	if (g_popUp_ask.popup)
	{
		lv_obj_del(g_popUp_ask.popup);
		g_popUp_ask.popup = NULL;
	}
	lv_obj_t* bgObj = lv_obj_create(lv_layer_top());
		bgObj->flags |= LV_OBJ_FLAG_OUT_OF_DEF_GROUP;
	lv_obj_set_size(bgObj, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_scroll_dir(bgObj, LV_DIR_NONE);
	lv_obj_set_style_border_width(bgObj, 0, 0);

	lv_obj_set_style_bg_color(bgObj, lv_color_hex(TBWD_SOS_BG_COLOR), 0);


	static lv_style_t style;
	lv_style_init(&style);
	lv_style_set_bg_opa(&style,LV_OPA_70); 
	lv_obj_add_style(bgObj, &style, 0);
	g_popUp_ask.popup = bgObj;




	lv_obj_t* rect = lv_obj_create(bgObj);
	lv_obj_set_size(rect, SCREEN_WIDTH + 100, SCREEN_WIDTH + 100);
	lv_obj_set_pos(rect, -60, 300);
	lv_obj_set_style_bg_color(rect, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
	lv_obj_set_style_radius(rect, 190, LV_PART_MAIN);

#ifndef C_PC_SIM
	lv_obj_t* labelName = lv_label_create(rect);
	lv_label_set_text_fmt(labelName, "%s号码来电", popup_contact.phone);
	lv_obj_align(labelName, LV_ALIGN_TOP_MID, 0, 30);
#else//也许后面改成 名称来电
	lv_obj_t* labelName = lv_label_create(rect);
	lv_label_set_text_fmt(labelName, "%s号码来电", popup_contact.phone);
	
	lv_obj_align(labelName, LV_ALIGN_TOP_MID, 0, 30);
#endif

    //lv_obj_set_user_data(bgObj, rect);

	//lv_obj_add_event_cb(bgObj, bg_click_cb, LV_EVENT_CLICKED, NULL);

	lv_obj_t* btnAnswer = lv_img_create(rect);
	lv_img_dsc_t* btnAnswer_img = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_HAND_UP, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(btnAnswer, btnAnswer_img);
	lv_obj_set_style_pad_top(btnAnswer, 10, 0);
	lv_obj_align(btnAnswer, LV_ALIGN_TOP_MID, -60, 60);
	lv_obj_add_flag(btnAnswer, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(btnAnswer, on_click_item, LV_EVENT_CLICKED, "nocall");
	lv_obj_add_event_cb(btnAnswer, _tbcd_no_pickup_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_add_style(btnAnswer, &style_clickBtn, LV_STATE_PRESSED);

	//来电(变量名有误，需要修改)
	lv_obj_t* btnHangup = lv_img_create(rect);
	lv_img_dsc_t* btnHangup_img = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_PHONECALL, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(btnHangup, btnHangup_img);
	lv_obj_set_style_pad_top(btnHangup, 10, 0);
	lv_obj_align(btnHangup, LV_ALIGN_TOP_MID, 60, 60);
	lv_obj_add_flag(btnHangup, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_style(btnHangup, &style_clickBtn, LV_STATE_PRESSED);
	lv_obj_add_event_cb(btnHangup, on_click_item, LV_EVENT_CLICKED, "ask");
	//lv_obj_add_event_cb(btnHangup, _tbcd_pickup_cb, LV_EVENT_CLICKED, NULL);


	sys_group = lv_group_create();
	zb_group_push(sys_group, 0);
	lv_group_add_obj(sys_group, bgObj);

	lv_obj_add_event_cb(bgObj, _tbcd_incoming_call_scr_cb, LV_EVENT_ALL, NULL);

	return 0;
}


static void _sos_delself()
{

	if (sos_strct.root)
	{
		lv_obj_del(sos_strct.root);
		sos_strct.root = NULL;
	}

}






void _sos_event_cb(lv_event_t* evt) {
	lv_event_code_t code = lv_event_get_code(evt);
	lv_obj_t* obj = lv_event_get_target(evt);



	 if (code == LV_EVENT_CLICKED) {
		if (obj == sos_strct.btnCancel) {
			LV_LOG_USER("sos.btnCancel click\n");

			LV_LOG_USER("g_pContactAp:%p\r\n", g_pContactAp);
			memset(&popup_contact, 0, sizeof(TBCD_CONTACT_RECORD_ITEM));
#if 0
			if (sys_group && zb_is_groupOnTopLayer(sys_group)) {
				_sos_delself();
			}
#else
			_sos_delself();
#endif
			lv_event_stop_processing(evt);
			lv_indev_wait_release(lv_indev_get_act());
			LV_LOG_USER("g_pContactAp:%p\r\n", g_pContactAp);
			//tbcd_switchScr(TBCD_SCR_NO_CONTENT, 0);
			//lv_event_stop_processing(evt);
			//lv_indev_keypad_wait_release();
		}
		else if (obj == sos_strct.btnConfirm) {

			_sos_delself();
			//tbcd_switchScr(TBCD_SCR_CALLING, 0);
			
			start_sos_calling();
			lv_event_stop_processing(evt);
			lv_indev_wait_release(lv_indev_get_act());
			
		}
	}
	 else if (code == LV_EVENT_DELETE) {

		 //if (sys_group != NULL)
		 //{
			// lv_group_del(sys_group);  sys_group = NULL;
		 //}
		 //zb_group_pop(false);
		 sos_strct.root = NULL;
	 }

}



#define CANVAS_WIDTH  320
#define CANVAS_HEIGHT  480


// 定义SOS紧急呼叫屏幕初始化
int tbcd_sos_scr_init(int param) {

	if (sos_strct.root)
	{
		lv_obj_del(sos_strct.root);
		sos_strct.root = NULL;
	}
#ifndef C_PC_SIM
	tz_play_stop_audio();
#endif

	//sys_group = lv_group_create();
	//zb_group_push(sys_group, 0);

	sos_strct.root = lv_obj_create(lv_layer_sys());
	lv_obj_set_size(sos_strct.root, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_bg_color(sos_strct.root, lv_color_hex(TBWD_SOS_BG_COLOR), 0);
	lv_obj_set_size(sos_strct.root, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(sos_strct.root, 0, 0);
	lv_obj_set_style_radius(sos_strct.root, 0, 0);
	lv_obj_set_scroll_dir(sos_strct.root, LV_DIR_NONE);

	static lv_style_t style;
	lv_style_init(&style);


	lv_style_set_bg_opa(&style, LV_OPA_70); 
	lv_obj_add_style(sos_strct.root, &style, 0);
	
	lv_style_init(&style_clickBtn);
	custom_style_set_img_recolor(&style_clickBtn);

#if 0
	lv_obj_set_style_shadow_width(sos_strct.root, 10, 0);
	lv_obj_set_style_shadow_opa(sos_strct.root, LV_OPA_80, 0); 
	lv_obj_set_style_shadow_color(sos_strct.root, lv_color_hex(0x000000), 0); 
	lv_obj_set_style_shadow_spread(sos_strct.root, 5,0); 
#endif

	lv_obj_t* labelTitle = lv_label_create(sos_strct.root);
	lv_label_set_text(labelTitle, "SOS紧急呼叫");
	lv_obj_align(labelTitle, LV_ALIGN_TOP_MID, 0, 20);
	lv_obj_set_style_text_color(labelTitle, lv_color_hex(SOS_LABEL_COLOR), LV_STATE_DEFAULT);

	sos_strct.btnConfirm = lv_btn_create(sos_strct.root);
	lv_obj_set_size(sos_strct.btnConfirm, 215, 55);
	lv_obj_align(sos_strct.btnConfirm, LV_ALIGN_BOTTOM_MID, 0, -60);
	lv_obj_set_style_bg_color(sos_strct.btnConfirm, lv_color_hex(CONFIRMATION_BTN_COLOR), 0);
	lv_obj_set_style_radius(sos_strct.btnConfirm, TBWD_SOS_BTN_RADIUS, 0);

	lv_obj_t* label_btnConfirm = lv_label_create(sos_strct.btnConfirm);
	lv_obj_set_width(label_btnConfirm, LV_SIZE_CONTENT);
	lv_obj_set_height(label_btnConfirm, LV_SIZE_CONTENT);
	lv_obj_set_align(label_btnConfirm, LV_ALIGN_CENTER);

	lv_label_set_text(label_btnConfirm, "确认呼叫");
	lv_obj_set_style_text_color(label_btnConfirm, lv_color_hex(SOS_LABEL_COLOR), LV_STATE_DEFAULT);
	

	sos_strct.btnCancel = lv_btn_create(sos_strct.root);
	lv_obj_set_size(sos_strct.btnCancel, 215, 55);
	lv_obj_align(sos_strct.btnCancel, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_set_style_bg_color(sos_strct.btnCancel, lv_color_hex(CANCEL_BTN_COLOR), 0);
	lv_obj_set_style_radius(sos_strct.btnCancel, TBWD_SOS_BTN_RADIUS, 0);

	lv_obj_t* label_btnCancel = lv_label_create(sos_strct.btnCancel);
	lv_obj_set_width(label_btnCancel, LV_SIZE_CONTENT);  
	lv_obj_set_height(label_btnCancel, LV_SIZE_CONTENT);    
	lv_obj_set_align(label_btnCancel, LV_ALIGN_CENTER);
	lv_label_set_text(label_btnCancel, "取消");
	lv_obj_set_style_text_color(label_btnCancel, lv_color_hex(SOS_LABEL_COLOR), LV_STATE_DEFAULT);
	



	//lv_group_add_obj(sys_group, sos_strct.root);
	lv_obj_add_event_cb(sos_strct.root, _sos_event_cb, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(sos_strct.btnCancel, _sos_event_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_add_event_cb(sos_strct.btnConfirm, _sos_event_cb, LV_EVENT_CLICKED, NULL);

#if 0

	static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(CANVAS_WIDTH, CANVAS_HEIGHT)];
	lv_obj_t* canvas = lv_canvas_create(lv_layer_top());
	lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR_ALPHA);
	lv_obj_center(canvas);

	lv_draw_gaussian_blur_dsc_t gaussian_blur;
	gaussian_blur.canvas = canvas;
	gaussian_blur.x = 50;
	gaussian_blur.y = 280;
	gaussian_blur.width = 320;
	gaussian_blur.height = 480;
	gaussian_blur.r = 50; //模糊半径，越大越模糊
	gaussian_blur.border_width = 1;
	gaussian_blur.border_color = lv_color_hex(0xABABAB); //边框颜色
	gaussian_blur.border_radius = 30; //边框半径
	gaussian_blur.blur_type = AVERAGEBLUR;
	lvgl_gaussian_t gaussian;
	lv_draw_gaussian_blur(&gaussian_blur);

	gaussian_blur.x = 570;
	gaussian_blur.y = 280;
	gaussian_blur.width = 660;
	gaussian_blur.height = 390;
	lv_draw_gaussian_blur(&gaussian_blur);
#else

#endif
	return 0;
}

// 定义主界面创建函数
lv_obj_t* tbcd_createMainWin(const char* pcTitle) {
	lv_obj_t* bgObj = lv_obj_create(lv_scr_act());
	lv_obj_set_size(bgObj, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_pad_all(bgObj, 0, 0);
	lv_obj_set_style_border_width(bgObj, 0, 0);
	lv_obj_set_style_bg_color(bgObj, lv_color_hex(TITLE_BG_COLOR), 0);
	lv_obj_set_scroll_dir(bgObj, LV_DIR_NONE);

	// 添加标题
	lv_obj_t* labelTitle = lv_label_create(bgObj);
	lv_obj_set_style_pad_left(labelTitle, 16, 0);
	lv_obj_align(labelTitle, LV_ALIGN_TOP_LEFT, 0, 12);
	lv_label_set_text(labelTitle, pcTitle);

	// 添加主背景
	lv_obj_t* main_bg = lv_obj_create(bgObj);
	lv_obj_set_size(main_bg, SCREEN_WIDTH, TBWD_MAIN_AREA_OBJ_H);
	lv_obj_align(main_bg, LV_ALIGN_TOP_LEFT, 0, TBWD_MAIN_AREA_OBJ_LOCY);
	lv_obj_set_style_pad_all(main_bg, 4, 0);
	lv_obj_set_style_pad_top(main_bg, 8, 0);
	lv_obj_set_style_radius(main_bg, TBWD_BG_CORNER_RADIUS, 0);
	lv_obj_set_style_bg_color(main_bg, lv_color_white(), 0);

	return bgObj;
}

#define SOS_TIMEOUT_MS 30000  // 30秒超时

static void safe_delete_timer(lv_timer_t** timer) {
	if (*timer) {
		lv_timer_pause(*timer);
		lv_timer_del(*timer);
		*timer = NULL;
	}
}

// 超时回调函数
static void sos_timeout_cb(lv_timer_t* timer) {     //
	if (g_pContactAp.sos_state != SOS_CALL_ACTIVE) return;

	ESP_LOGI(TAG, "[SOS] Timeout on call to %s",
		g_pContactAp.sos_contacts[g_pContactAp.current_sos_index].phone);

	
#ifndef C_PC_SIM
	seed_phone_Nocall();
	
	g_pContactAp.current_sos_index++;
#endif

	
	safe_delete_timer(&g_pContactAp.sos_timer);


	sos_call_next_number();	
}


// SOS拨打入口函数
void start_sos_calling() {     //

	tbcd_load_contact_record(CONTACT_FILE_PATH);

	memset(g_pContactAp.sos_contacts, 0, sizeof(g_pContactAp.sos_contacts));
	g_pContactAp.current_sos_index = 0;

	uint8_t idx = 0;
	for (int i = 0; i < g_pContactAp.contact_count; i++) {
		if (strstr(g_pContactAp.contacts[i].name, "紧急联络人")) {
			memcpy(&g_pContactAp.sos_contacts[idx++],
				&g_pContactAp.contacts[i],
				sizeof(TBCD_CONTACT_RECORD_ITEM));
			break;
		}
	}

	
	for (int i = 0; i < g_pContactAp.contact_count; i++) {
		if (idx >= TBCD_CONTACT_MAX) break;
		if (!strstr(g_pContactAp.contacts[i].name, "紧急联络人")) {
			memcpy(&g_pContactAp.sos_contacts[idx++],
				&g_pContactAp.contacts[i],
				sizeof(TBCD_CONTACT_RECORD_ITEM));
		}
	}

	g_pContactAp.sos_state = SOS_CALL_ACTIVE;
	sos_call_next_number();
}

void sos_call_next_number() {           //
	
	if (g_pContactAp.current_sos_index >= TBCD_CONTACT_MAX ||
		!g_pContactAp.sos_contacts[g_pContactAp.current_sos_index].phone[0]) {
		ESP_LOGI(TAG, "[SOS] All contacts tried");
		safe_delete_timer(&g_pContactAp.sos_timer);
		//tbcd_switchScr(TBCD_SCR_NO_CONTENT, 0);
		free_calling_scr();
		return;
	}

	
	TBCD_CONTACT_RECORD_ITEM* contact =
		&g_pContactAp.sos_contacts[g_pContactAp.current_sos_index];


	memcpy(&calling_contact, contact, sizeof(TBCD_CONTACT_RECORD_ITEM));
	seed_phone_call();
	LV_LOG_USER("sos_call_next_number A \r\n");

	LV_LOG_USER("sos_call_next_number B \r\n");
	safe_delete_timer(&g_pContactAp.sos_timer);
	g_pContactAp.sos_timer = lv_timer_create(sos_timeout_cb, SOS_TIMEOUT_MS, NULL);
	lv_timer_set_repeat_count(g_pContactAp.sos_timer, 1); // 确保单次触发
	LV_LOG_USER("sos_call_next_number C \r\n");



#ifndef C_PC_SIM
	int ret = state_handler->call(contact->phone);
	if (ret != 0) {
		ESP_LOGE(TAG, "[SOS] Failed to call %s, error:%d", contact->phone, ret);
		g_pContactAp.current_sos_index++;
		sos_call_next_number();
}
#else
	
	ESP_LOGI(TAG, "[SIM] Calling %s...", contact->phone);
	
#endif
}















// 定义屏幕信息表
static const struct DIC_SCR_INF tbcdScrInfTbl[] = {
	{ tbcd_no_contentList_scr_init,  NULL },//TBCD_SCR_NO_CONTENT
	{ tbcd_add_contact_scr_init, NULL },//TBCD_SCR_ADD_CONTACT
	{ tbcd_calling_scr_init, free_calling_scr},//TBCD_SCR_CALLING
	{ tbcd_incoming_call_scr_init, NULL },//TBCD_SCR_INCOMING_CALL
	{ tbcd_sos_scr_init,  NULL },//TBCD_SCR_SOS
	{ NULL, NULL },
};

#define test_maoboli 0

#if  test_maoboli  //毛玻璃效果测试成功(但lvgl8.3版本仅限图片设置毛玻璃效果，lvgl9才支持当前屏幕快照)
static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(200, 200)];
static lv_draw_img_dsc_t t;
static lv_area_t a;
static lv_obj_t* bak, * obj1, * canvas, * slider;
lv_img_dsc_t* bgObj_bg;
lv_img_dsc_t* img_btn;
static void pressing_cb(lv_event_t* e);
static void slider_cb(lv_event_t* e);
static int b;

void maoglass()
{

	a.x1 = 0;
	a.x2 = 200;
	a.y1 = 0;
	a.y2 = 200;




	bak = lv_img_create(lv_scr_act());
	lv_obj_set_size(bak, 800, 480);
	bgObj_bg = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_BGCALLING, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(bak, bgObj_bg);

	lv_obj_t* btn = lv_img_create(bak);
	lv_obj_set_size(btn, 100, 100);
	img_btn = zb_img_res_load_one(NULL, ADDRESSBOOK_IMG_DAD, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(btn, img_btn);

	slider = lv_slider_create(lv_scr_act());
	lv_obj_align(slider, LV_ALIGN_RIGHT_MID, -20, 0);
	lv_obj_set_size(slider, 20, 200);
	lv_slider_set_range(slider, 0, 100);
	lv_obj_add_event_cb(slider, slider_cb, LV_EVENT_VALUE_CHANGED, 0);

	obj1 = lv_obj_create(lv_scr_act());
	lv_obj_set_size(obj1, 200, 200);
	lv_obj_set_style_radius(obj1, 50, 0);
	lv_obj_set_style_clip_corner(obj1, true, LV_PART_MAIN);
	lv_obj_clear_flag(obj1, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_event_cb(obj1, pressing_cb, LV_EVENT_PRESSING, 0);


	lv_draw_img_dsc_init(&t);

	canvas = lv_canvas_create(obj1);
	lv_canvas_set_buffer(canvas, cbuf, 200, 200, LV_IMG_CF_TRUE_COLOR);
	lv_obj_center(canvas);

	lv_canvas_draw_img(canvas, 0, 0, img_btn, &t);
	lv_canvas_blur_hor(canvas, &a, 10);
	lv_canvas_blur_ver(canvas, &a, 10);


}


static void pressing_cb(lv_event_t* e)
{
	lv_point_t point;

	lv_indev_get_point(lv_indev_get_act(), &point);

	lv_obj_set_pos(obj1, point.x - 100, point.y - 100);

	lv_canvas_draw_img(canvas, -point.x + 100, -point.y + 100, img_btn, &t);
	lv_canvas_blur_hor(canvas, &a, b);
	lv_canvas_blur_ver(canvas, &a, b);


}


static void slider_cb(lv_event_t* e)
{

	b = lv_slider_get_value(slider);

	lv_canvas_draw_img(canvas, -lv_obj_get_x(obj1), -lv_obj_get_y(obj1), img_btn, &t);
	lv_canvas_blur_hor(canvas, &a, b);
	lv_canvas_blur_ver(canvas, &a, b);

}
#endif




/**
 * @brief 通讯录模块初始化函数
 *
 * @return int 初始化结果
 */

int contact_ap_init(void)
{
	ESP_LOGI(TAG, "%s A", __func__);

#if !test_maoboli

	LV_LOG_USER("g_pContactAp:%p\r\n", g_pContactAp);
	
	lv_style_init(&style_clickBtn);
	custom_style_set_img_recolor(&style_clickBtn);

	ESP_LOGI(TAG, "%s E", __func__);

#if 1
	tbcd_no_contentList_scr_init(0);
#else  
	tbcd_switchScr(TBCD_SCR_NO_CONTENT, 0);
#endif


	ESP_LOGI(TAG, "%s G", __func__);


#else

	maoglass();
#endif
	return 0;
}


int contact_ap_handler(lv_event_code_t msg, int param)
{
	switch (msg) {
	case EVENT_FREE_AP_RES:
		ESP_LOGI(TAG, "ApFreeRes Z");

		FN_TBCD_SCREEN_FREE fnFree = tbcdScrInfTbl[g_scrNo].m_fn_free;
		if (fnFree) {  // 释放当前屏幕资源
			(*fnFree)();
		}
		else {
			if (g_pContactAp.root) {
				lv_obj_del(g_pContactAp.root);
				g_pContactAp.root = NULL;
			}
			//if (g_pContactAp) {
			//	for (int i = 0; i < TBCD_CONTACT_MAX; i++) {
			//		if (g_pContactAp.contactList.contactItem[i]) {
			//			heap_caps_free(g_pContactAp.contactList.contactItem[i]);
			//			g_pContactAp.contactList.contactItem[i] = NULL;
			//		}
			//	}
			//	heap_caps_free(g_pContactAp);
			//	g_pContactAp = NULL;
			//}
		}

		ESP_LOGI(TAG, "freeAp G");
		ESP_LOGI(TAG, "freeAp K");


		break;

	case EVENT_EX_CMD:
		ESP_LOGI(TAG, "rcv exCmd par:%d", param);
		break;

	default:
		break;
	}
	return 0;
}





// 定义屏幕切换函数
void tbcd_switchScr(int8_t scrNoNew, int parm) {

	if (scrNoNew == TBCD_SCR_DIALPAD) {
		lv_obj_clear_flag(g_pContactAp.dialpad.dialpad_cont, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(g_pContactAp.root, LV_OBJ_FLAG_HIDDEN);
		return;
	}

	if ((scrNoNew < 0) || (scrNoNew >= M_ARRAY_LEN(tbcdScrInfTbl))) {
		ESP_LOGE(TAG, "error scrNo %d", scrNoNew);
		return;
	}

	FN_TBCD_SCREEN_INIT fnInit = tbcdScrInfTbl[scrNoNew].m_fn_init;
	if (!fnInit) {
		ESP_LOGW(TAG, "newScr cb isNul!");
		return;
	}

	if ((int)g_scrNo >= 0) {
		FN_TBCD_SCREEN_FREE fnFree = tbcdScrInfTbl[g_scrNo].m_fn_free;
		if (fnFree) {
			(*fnFree)();
		}
		else {
			incoming_call_scr_del();
			ready_call_scr_del();
			LV_LOG_USER("g_pContactAp:%p\r\n", g_pContactAp);
			//if (g_pContactAp)
			//{
				if (g_pContactAp.root) {
					lv_obj_del(g_pContactAp.root);
					g_pContactAp.root = NULL;
				}
			//}
	
			
		}
	}

	g_scrNo = scrNoNew;
	(*fnInit)(parm);
}


static void on_click_item(lv_event_t* event)
{
	const char* text = (const char*)event->user_data;
	if (strcmp(text, "en") == 0)
	{
		printf("create_activity en\n");
	}
	else if (strcmp(text, "set") == 0)
	{
		printf("create_activity set\n");
	}
	else if (strcmp(text, "ask") == 0)
	{
		printf("on_click_item :ask \n");
		seed_phone_ask();
		_tbcd_pickup_cb();
	}
	else if (strcmp(text, "call") == 0)
	{
		printf("create_activity call\n");
		memcpy(&calling_contact, &new_contact, sizeof(TBCD_CONTACT_RECORD_ITEM));
		printf("new_contact.phone :%s\n", new_contact.phone);
		printf("calling_contact.phone :%s\n", calling_contact.phone);
#ifndef C_PC_SIM

		//quectel_test();
		seed_phone_call();
#else
		
		callBtn_cb();
#endif
	
	}
	else if (strcmp(text, "nocall") == 0)
	{
		printf("create_activity nocall\n");
		seed_phone_Nocall();
	}
}