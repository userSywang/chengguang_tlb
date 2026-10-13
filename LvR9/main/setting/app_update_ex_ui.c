
#include <time.h>
#include <lvgl/lvgl.h>
#include "../utils/misc_utils.h"

#ifndef C_PC_SIM
#include "../include/adc_battery.h"
#endif

#include "../zb_base.h"
#include "../lvgl_main.h"

#include "../ex_widget/app_msg.h"

#include "./app_update_by_file.h"


extern const char * get_tlb_cur_version(void);
extern int get_tlb_cur_version_no(void);



#define TAG  "codeUpdFl"

//----------------------------------------------


typedef struct {
	int16_t m_versonNo;
	char m_acVersionStr[24];
} code_upd_file_info_t;


enum upd_code_file_stat {
	CODE_FILE_STAT_OK = 0,
	CODE_FILE_STAT_NOT_MATCH,
	CODE_FILE_STAT_NOT_EXIST,
	CODE_FILE_STAT_VERSION_LOW,
	CODE_FILE_STAT_NOT_MATCH_EX,
};

//----------------------------------------------


static enum upd_code_file_stat g_updFileStat;
static bool g_isUpdateDoing;
static int volatile g_updateProcess;  // from 0 to 100;
static int volatile g_updatePrcsPrevDisp;

static lv_obj_t * g_bgObj;
static lv_obj_t * g_lblPromptA;

static lv_timer_t * g_codeUpdApTmr;

//----------------------------------------------

/*
 * @return  enum upd_code_file_stat, 0 means success
 */
static enum upd_code_file_stat getCodeUpdateFileInfo(code_upd_file_info_t * pstUpdFileInfo)
{
	FILE * fp = NULL;
	enum upd_code_file_stat iFuncRet = CODE_FILE_STAT_NOT_MATCH;

	LV_ASSERT(pstUpdFileInfo != NULL);
	memset(pstUpdFileInfo, 0, sizeof(code_upd_file_info_t));
	do {
		fp = zb_fopen(CODE_UPDATE_FILE_PATH, "rb");
		if (!fp) {
			ESP_LOGE(TAG, "fail to open fl %s", CODE_UPDATE_FILE_PATH);
			iFuncRet = CODE_FILE_STAT_NOT_EXIST;
			break;
		}
		code_upd_file_header_t stFileHead = { 0 };
		fread(&stFileHead, 1, sizeof(code_upd_file_header_t), fp);
		if (memcmp(stFileHead.m_acIndentifier, CODE_UPD_FILE_IDENTIF, 12) != 0) { // todo
			iFuncRet = CODE_FILE_STAT_NOT_MATCH;
			break;
		}

		pstUpdFileInfo->m_versonNo = stFileHead.m_versonNo;
		strncat(pstUpdFileInfo->m_acVersionStr, stFileHead.m_acVersionStr, sizeof(pstUpdFileInfo->m_acVersionStr) - 1);

		int versioNo = get_tlb_cur_version_no();
		if (stFileHead.m_versonNo <= 0 || stFileHead.m_versonNo < versioNo) {  // version it too low.
			iFuncRet = CODE_FILE_STAT_VERSION_LOW;
			break;
		}

		int64_t fileLen = zb_getFileLength(CODE_UPDATE_FILE_PATH);
		if (fileLen < 1024 * 1024 || fileLen >= 8 * 1024 * 1024) { // fileSize not valid.
			iFuncRet = CODE_FILE_STAT_NOT_MATCH_EX;
			break;
		}
		iFuncRet = 0;
	} while (0);

	if (fp) {
		zb_fclose(fp);  fp = NULL;
	}
	if (iFuncRet != 0 ) {
		ESP_LOGW(TAG, "fail to getCodeUpdInf %d", (int)iFuncRet);
	}
	return iFuncRet;
}


static void appUpdByFile_process_cb(int process)
{
	g_updateProcess = process;
}


static void _codeUpdEx_updateProcessDisp()
{
	lv_label_set_text_fmt(g_lblPromptA, "升级中 %d ％", (int)g_updateProcess);
}


static void _codeUpdEx_timer_cb(struct _lv_timer_t *tmr) {
	//ESP_LOGI(TAG, "tmr A");

	zb_clear_autoScreenOffCnt();
	zb_clear_autoPwrOffCnt();
	if (g_updateProcess != g_updatePrcsPrevDisp) {
		_codeUpdEx_updateProcessDisp();
		g_updatePrcsPrevDisp = g_updateProcess;
	}
	return;
}

static void _codeUpdExWinEvtCb(lv_event_t * evt) {
	lv_event_code_t evtCode = evt->code;

	int keyVal = 0;
	switch (evtCode) {
		case LV_EVENT_KEY_STAGE: {
			keyVal = lv_event_get_key(evt);
			if (((LV_EX_FLAG_KEY_MASK & keyVal) != LV_EX_FLAG_KEY_REPEAT) && ((LV_EX_FLAG_KEY_MASK & keyVal) != LV_EX_FLAG_KEY_SHORT)) {
				return;
			}
			keyVal &= ~LV_EX_FLAG_KEY_MASK;
			ESP_LOGI(TAG, "codeUpdEx key %x", keyVal);
			switch (keyVal) {
				case LV_KEY_ENTER:
					ESP_LOGI(TAG, "entKey prs");
					if (g_updFileStat != CODE_FILE_STAT_OK)
						break;
					if (!g_isUpdateDoing) {
						// 显示 0 %; 启动 timer;
						g_isUpdateDoing = true;
						g_updatePrcsPrevDisp = g_updateProcess = 0;
						_codeUpdEx_updateProcessDisp();
						g_codeUpdApTmr = lv_timer_create(_codeUpdEx_timer_cb, 1700, NULL);
				#ifndef C_PC_SIM
						app_file_upgrade_start(appUpdByFile_process_cb);  // 开始升级 
				#endif
					}
					break;
				case LV_KEY_ESC:
					if (!g_isUpdateDoing) {
						zb_requestSwitchAp(C_ZB_AP_MAINMENU);
					}
					break;
				default: {
					//int32_t tmMs = zb_get_time_ms_since_boot();
					//ESP_LOGI(TAG, "elaps tm PrdT:%ld ms, %ld sec", tmMs,  tmMs/1000);
					break;
				}
			}
			break;
		}
		case LV_EVENT_DELETE: {
			if (g_codeUpdApTmr) {
				lv_timer_del(g_codeUpdApTmr);  g_codeUpdApTmr = NULL;
			}
			break;
		}
		default:
			break;
	}
}

static void codeUpdWin_init(code_upd_file_info_t * pstCodeUpdFileInf)
{
	lv_obj_t * mainWin = lv_obj_create(lv_scr_act());
	lv_obj_set_size(mainWin, SCREEN_WIDTH, SCREEN_HEIGHT);

	lv_obj_t * lblTitle = lv_label_create(mainWin);
	lv_label_set_text_static(lblTitle, "软件升级(TF卡方式)");
	//lv_obj_set_pos(lblTitle, 6, 6);

	lv_obj_t * lblPrompt = lv_label_create(mainWin);
	if (g_updFileStat != CODE_FILE_STAT_OK) {
		char acInfo[220];
		acInfo[0] = '\0';
		if (CODE_FILE_STAT_NOT_EXIST == g_updFileStat) {
			snprintf(acInfo, sizeof(acInfo), "升级文件未找到\n(应放到根目录下的\n文件名为 %s)",
					CODE_UPDATE_FILE_NAME);
		}
		else if (CODE_FILE_STAT_VERSION_LOW == g_updFileStat) {
			snprintf(acInfo, sizeof(acInfo), "升级文件的版本过低,\n版本号 %hd", pstCodeUpdFileInf->m_versonNo);
		}
		else {
			snprintf(acInfo, sizeof(acInfo), "升级文件无效");
		}
		lv_label_set_text(lblPrompt, acInfo);
	}
	else {
		lv_label_set_text_fmt(lblPrompt, "待升级版本号 %hd,\n版本名 %s\n按确定键开始升级", 
				pstCodeUpdFileInf->m_versonNo, pstCodeUpdFileInf->m_acVersionStr);
	}
	lv_obj_set_style_text_align(lblPrompt, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_align(lblPrompt, LV_ALIGN_CENTER, 0, 0);
	g_lblPromptA = lblPrompt;

	g_bgObj = mainWin;
	lv_obj_add_event_cb(mainWin, _codeUpdExWinEvtCb, LV_EVENT_ALL, NULL);
	sys_setCurMainWinAndCb(mainWin, _codeUpdExWinEvtCb);
	return;
}


int zb_codeUpdateByFile_ap_handler(lv_event_code_t msg, int param) { 
	switch ((int)msg) {
		case EVENT_FREE_AP_RES:
			ESP_LOGI(TAG, "codeUpdFlApFree A");
			if (g_bgObj) {
				lv_obj_del(g_bgObj);   g_bgObj = NULL;
			}
			ESP_LOGI(TAG, "codeUpdFlApFree Z");
			break;
		case C_EVENT_DOES_BLOCK_SYS_POWER_KEY:
			if (g_isUpdateDoing) {
				return 1;
			}
			return 0;
			break;
		default:
			break;
	}
	return 0;
}


int zb_codeUpdateByFile_ap_init(void) { 
	ESP_LOGI(TAG, "ap init A");

	g_isUpdateDoing = false;
	g_updateProcess = 0;

	code_upd_file_info_t stCodeUpdFileInf = { 0 };
	int ret = getCodeUpdateFileInfo(&stCodeUpdFileInf);
	g_updFileStat = ret;
	if (ret < 0) {
		ESP_LOGW(TAG, "codeUpdByFile notReady, %d", ret);
	#if 0
		char acInfo[120];
		snprintf(acInfo, sizeof(acInfo), "不能程序升级, 错误码 %d", ret);
		app_msgbox_timeout("", acInfo, 2500|MSGBOX_TOAST_FLAG, NULL, NULL);
	#endif
	}

	codeUpdWin_init(&stCodeUpdFileInf);

#ifndef C_PC_SIM
#endif

	return 0;
}



