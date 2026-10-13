#ifndef _PRODUCT_TEST_H_
#define _PRODUCT_TEST_H_

#include "stdint.h"

//————————
typedef int (*FN_PRDTS_SCREEN_INIT)(int parm);
typedef int (*FN_PRDTS_SCREEN_FREE)(void);

struct PRDTS_SCR_INF {
    FN_PRDTS_SCREEN_INIT m_fn_init;
    FN_PRDTS_SCREEN_FREE m_fn_free;
};


typedef struct PRDTS_AP_INF {
	void * m_recBuf;

	//------- for confirmInput screen.
	lv_obj_t * m_textArea;
	lv_obj_t * m_keyboard;
} prdts_ap_inf_t;


#define PRDTS_SCR_NO_MENU    0
#define PRDTS_SCR_NO_KEYBTS  1   // 按键测试 
#define PRDTS_SCR_NO_AUDIO_TS  2  // 语音测试 
#define PRDTS_SCR_NO_AUD_EARPHONE_TS  3  // 耳机测试 
#define PRDTS_SCR_NO_REC_TS  4   // 录音测试 
#define PRDTS_SCR_NO_LCD_TS  5   // 屏幕测试 
#define PRDTS_SCR_NO_TFCARD_TS 6
#define PRDTS_SCR_NO_BATTERY_TS  7   // 电量测试 
#define PRDTS_SCR_NO_RTC_TS   8    // 时钟 
#define PRDTS_SCR_NO_BUILD_SN 9    // SN 号 
#define PRDTS_SCR_NO_NET_CNCT 10   // 网络连接 
#define PRDTS_SCR_NO_MSC_LOOP 11
#define PRDTS_SCR_NO_BURN_IN  12   // 老化 
#define PRDTS_SCR_NO_VERSION  13   // 版本信息 
#define PRDTS_SCR_NO_SN_AMEND    14 //   SN 修正

#define PRDTS_MNU_ITEM_N  13  // 测试项总数 


#define MY_REC_TIME_MS	 (4*1000)
#define PRDTS_REC_BUF_LEN	 (32 * MY_REC_TIME_MS)


#define PATH_TEST_A_MP3  "/resource/sysData/a.mp3"


extern prdts_ap_inf_t  * g_prdtsApInf;

extern lv_obj_t * g_prdTs_bgWin;


void prdTs_switchScr(uint8_t scrNoNew, int parm);


extern int prdTs_getDeviceMac(char * pcMacStr, int bufLen);


#ifdef C_PC_SIM
#define get_esp32s3_wifi_scan_finished_state()  0
#endif

extern void prdTs_wifi_scan_start(void);
extern void prdTs_turn_on_wifi(void);
extern void prdTs_turn_off_wifi(void);


extern int prdTs_aud_scr_init(int parm);
extern int prdTs_aud_scr_deinit(void);

extern int prdTs_audLoop_scr_init(int parm);
extern int prdTs_audLoop_scr_deinit(void);


extern int prdTs_rec_scr_init(int parm);
extern int prdTs_rec_scr_deinit(void);

extern int prdTs_build_sn_scr_init(int parm);
extern int prdTs_build_sn_scr_deinit(void);


extern int prdTs_burnIn_scr_init(int parm);
extern int prdTs_burnIn_scr_deinit(void);


extern int prdTs_net_cnct_scr_init(int parm);
extern int prdTs_net_cnct_scr_deinit(void);



#ifndef C_PC_SIM
extern void  music_player_status(music_play_status_t status , void *user_data);
#endif


#endif

