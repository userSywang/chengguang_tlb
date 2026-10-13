#ifndef _MAINMENU_BASE_H_
#define _MAINMENU_BASE_H_

#include <stdint.h>


#define MMNU_STATUS_BAR_H   60  // 顶部状态条的高度 
#define MMNU_STATUS_BAR_THICK_H   70  // 顶部状态条(含英语等按钮时)的高度 

#define MMNU_GRID_TOP_AID_ROW_H     0   // todo 
//#define MMNU_GRID_THIN_ROW_H        0   // 窄行 (现在没了)
#define MMNU_GRID_ROW_H             61  // 行高
#define MMNU_GRID_BOTTOM_AID_ROW_H  16
//------------------------------


#define C_RECHARGE_FLAG  0x80


#define MMN_CHECK_CODE  0xa3


#define MMN_SCR_NO_PROMPT_FOR_NEW_USER  0x70
#define MMN_SCR_NO_PROMPT_NET_CFG  0x71
#define MMN_SCR_NO_NET_CFG         0x72
#define MMN_SCR_NO_PWD_LOGIN       0x73

//----------------------------------------------

typedef struct {
    lv_obj_t* popup;     
    lv_obj_t* mask_layer; 
} NetStatusPopup;

extern bool setNetworkStat;

extern uint8_t mmn_get_batteryLevelNo(void);

extern void mmn_set_screenNo(int8_t scrNo, uint8_t checkCode);

extern int main_menu_pg_init(int8_t pageNo, bool doesInitStat);

extern bool is_main_menu_active(void);

//----------------------
extern bool mmn_promptForNewUsr_scr_wasShown(void);

extern int mmn_promptForNewUsr_scr_init(void);

extern void mmn_promptForNewUsr_scr_deinit(void);

extern void mmn_promptForNewUsr_scr_invalid(void);

extern void mmnOth_draw_spaceItem_cb(lv_event_t * e);

extern void create_dialog_with_timer(bool icon, const char* text, uint32_t timeout_ms, int x, int y);

extern void del_dialog();

extern lv_obj_t* create_custom_button(lv_obj_t* parent, const char* text, lv_event_cb_t event_cb, void* user_data);

extern lv_obj_t* create_network_popup(lv_obj_t* parent, const char* tip, const char* label, int y);

void _mmn_update_statbar_bat(bool isCharging,int power);
void _mmn_update_statbar_wifi(bool isConnected,int power);
void _mmn_update_statbar_sim_state(int state);
void _mmn_update_statbar_sim_power(int power);
void _mmn_update_statbar_class_mode(int mode);
void _mmn_update_statbar_mute_state(int mode);
void _mmn_update_statbar_cy_state(int mode);
extern const char* get_imei_id(void);

#endif


