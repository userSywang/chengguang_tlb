
#include <lvgl/lvgl.h>

#include "../utils/misc_utils.h"
#include "../ex_widget/misc_draw.h"

#include "../data_parse/dict_parse.h"
#ifndef C_PC_SIM
#include  <cJSON.h>
#else
#include "../data_parse/cJSON.h"
#endif

#ifndef C_PC_SIM
#include "../include/audio_stream.h"  
#include "../include/audio_record.h"

//#include "../managed_components/espressif__esp-dsp/modules/common/include/dsp_platform.h"
//#include "../include/audio_media_def.h"
//#include "../include/audio_player.h"
//#include "app_codec.h"

#endif
#include "../lvgl_main.h"


#include "../data_parse/img_res.h"
#include "../ex_widget/app_msg.h"


#include "../player/player_http_client.h"

#include "../utils/zb_aud_api.h"

#include "jump_dic.h"
#include "jump_dic_aud.h"


#ifndef C_PC_SIM
void  music_player_status(music_play_status_t status , void *user_data);
#endif

extern int play_http_post_create(int rq_num);

extern int dic_searchEngVar(struct DIC_LIB_s * pDicEc, char * pcEngWord, char * pcOrgWord, int orgBufLen);


static void _updateOneEngWord(struct DIC_LIB_s * pDicEc);


//extern const lv_font_t lv_font_montserrat_20;
extern char * g_plyHttpRsltBuf;


#define TAG  "jmpDic:"

//----------------------------------------------------------------
#define WORD_AUD_IS_ONLINE  0  // 1 - online audio,  0 - offline audio 

#define DIC_ENG_WORD_BUF_LEN	30

#define C_WORDS_N_MAX  80
#define C_EXPL_JSON_BUF_LEN  2048

#define C_EXPL_TXT_BUF_LEN  1800
#define C_AUD_URL_BUF_LEN   300

#define WORD_SEL_NO_NONE  ((uint8_t)(-1))  // 无效选择号 

#define PATH_DIC_EC_A  "/sdcard/sysData/dict/engchn.dic"

//----------------------------------------------------------------
struct DIC_LIB_s * g_pDicEc = NULL;


static lv_obj_t  * g_bgWin = NULL;

static lv_obj_t  * g_flexObj;
static lv_obj_t  * g_headwdObj;
static lv_obj_t  * g_spkObjTbl[2];
static lv_obj_t  * g_explObj;
static lv_obj_t  * g_promptBar;
static lv_timer_t * g_dicTmr;

static uint8_t  g_quitMode;  // 0- delOnly, 1- escThisScreen 
static uint8_t  g_wordsNum;
static uint8_t  g_wordSelNo; // 当前单词号， 0xff 表示未选中

static char ** g_wordsTbl;

static int32_t g_dicEnResultStart;  // 在EC词典的第一个匹配的词条序号 
static int8_t  g_dicEnResultNum;
static int8_t  g_dicEnResultSel;
static char * g_pcExpl;  // 用于存放原解释 json 字串
static char * g_pcExplInObj;
static char * g_pcWordAudUk;
static char * g_pcWordAudUs;

static bool g_isExplAtTailPg;
static int16_t g_expScrollY;

static bool  g_noAudFlag;
static int32_t g_audNoUs; 
static int32_t g_audNoUk; 

static int8_t g_audTypeSel;  // 0 us or 1 uk 
static int32_t g_startTm;

static int32_t g_audQueryTm;  // 开始 aud 查询的时刻, ms 
//----------------------------------------------------------------

static int getNonWordPartLen(const char * str) {
	int strSt = str;
	char ch;
	while (1) {
		ch = (*str);
		if (!ch)
			break;
		if ((ch >= 'A') && (ch <= 'Z'))
			break;
		if ((ch >= 'a') && (ch <= 'z'))
			break;
		if ('\'' == ch)
			break;
		str++;
	}
	return (str - strSt);
}

static int getWordPartLen(const char * str) {
	int strSt = str;
	char ch;
	while (1) {
		ch = (*str);
		if (!ch)
			break;
		if (! ((ch >= 'A') && (ch <= 'Z')) && !((ch >= 'a') && (ch <= 'z')) && ('\'' != ch))
			break;
		str++;
	}
	return (str - strSt);
}


/** 拆分英文字串 
 * @output g_wordsTbl[]
 * @return 得到单词个数 
 * 字串来源     zb_global_str_read(char * pcBuf, int bufLen); 
 *  以 A~Z、a~z、单撇     以外的字符为分隔符 ().  保留'.'、'-'、'\''数字;
 */
static int16_t _dic_split_words(char * pcSentence) { 

	int16_t wordNo = 0;
	int pos = getNonWordPartLen(pcSentence);
	while (wordNo < C_WORDS_N_MAX) {
		int wordLen = getWordPartLen(pcSentence + pos);
		if (wordLen <= 0) // read a spaceWord, stop.
			break;
		g_wordsTbl[wordNo] = pcSentence + pos;
		pos += wordLen;
		if (!pcSentence[pos]) {
			wordNo++;
			break;  // reach tail of str.
		}
		pcSentence[pos] = '\0';  // fill tailChar of word.
		pos++;
		pos += getNonWordPartLen(pcSentence + pos);
		wordNo++;
	}
	return wordNo;
}

static void cvtAudioToTwo(const char * pcAudOrg, int32_t * audNoTwo)
{
	audNoTwo[0] = atoi(pcAudOrg);
	audNoTwo[1] = -1;
	char * pTmp = strchr(pcAudOrg, ':');
	if (pTmp) {
		audNoTwo[1] = atoi(pTmp + 1);
	}
}

//----------------------------------------------------------------------
static void dictScrResInit(void) {
	if (!g_pcWordAudUk) {
		g_pcWordAudUk = (char *)heap_caps_malloc_dflt(C_AUD_URL_BUF_LEN);
		g_pcWordAudUk[0] = '\0';
	}
	if (!g_pcWordAudUs) {
		(g_pcWordAudUs) = (char *)heap_caps_malloc_dflt(C_AUD_URL_BUF_LEN);
		g_pcWordAudUs[0] = '\0';
	}
}

static void dictScrResDeinit(void) {
	if (g_pcWordAudUs) {
		heap_caps_free((g_pcWordAudUs));  (g_pcWordAudUs) = NULL;
	}
	if (g_pcWordAudUk) {
		heap_caps_free(g_pcWordAudUk);  g_pcWordAudUk = NULL;
	}
	dic_audLib_deinit();
}

static void _dictSpkObjSetCheck() {
	for (int16_t sub = 0; sub < 2; sub++) {
		if (sub == g_audTypeSel) {
			lv_obj_add_state(g_spkObjTbl[sub], LV_STATE_CHECKED);
		}
		else {
			lv_obj_clear_state(g_spkObjTbl[sub], LV_STATE_CHECKED);
		}
	}
}


static void _selectAnotherWord(uint8_t selNoNew) {
	if ((selNoNew == g_wordSelNo) || (selNoNew > g_wordsNum))
		return;

	if (g_wordSelNo != WORD_SEL_NO_NONE) {
		lv_obj_t * labelPrev = lv_obj_get_child(g_flexObj, g_wordSelNo);
		lv_obj_clear_state(labelPrev, LV_STATE_CHECKED);
	}
	lv_obj_t * labelNew = lv_obj_get_child(g_flexObj, selNoNew);
	lv_obj_add_state(labelNew, LV_STATE_CHECKED);
	g_wordSelNo = (uint8_t)selNoNew;
	lv_obj_scroll_to_view(labelNew, LV_ANIM_OFF);

	if (!g_dicTmr)
		return;
	lv_timer_pause(g_dicTmr);
	lv_timer_reset(g_dicTmr);

	g_audTypeSel = 0;
	lv_obj_clear_state(g_spkObjTbl[0], LV_STATE_CHECKED);
	lv_obj_clear_state(g_spkObjTbl[1], LV_STATE_CHECKED);
	g_startTm = zb_get_time_ms_since_boot();
	lv_timer_resume(g_dicTmr);
}


static void _update_srchResult(void) {
	if (!g_flexObj)
		return;

	int iErrNo = 0;
	struct DIC_LIB_s * pDicEc = g_pDicEc;
	if (!pDicEc) {
		pDicEc = zb_dic_open(PATH_DIC_EC_A, &iErrNo);
	}
	if (!pDicEc) {
		ESP_LOGE(TAG, "Fail to open dic ret %d, %s", iErrNo, PATH_DIC_EC_A);
		return;
	}
	g_pDicEc = pDicEc;
	if (!g_pcExpl) {
		g_pcExpl = (char *)heap_caps_malloc_dflt(C_EXPL_JSON_BUF_LEN);
	}
	if (!g_pcExplInObj) {
		g_pcExplInObj = (char *)heap_caps_malloc_dflt(C_EXPL_TXT_BUF_LEN);
	}
	
	lv_obj_t * labelCur = lv_obj_get_child(g_flexObj, g_wordSelNo);
	if (!labelCur) {
		ESP_LOGE(TAG, "wordLabel is space!");
		return;
	}

	const char * pcWord = lv_label_get_text(labelCur);
	ESP_LOGI(TAG, "bfr srchWord %s", pcWord);
#ifndef C_PC_SIM
	#if WORD_AUD_IS_ONLINE
	music_player_stop();
	#else
	audio_stream_stop_play();
	#endif
#endif
	g_noAudFlag = false;
	bool isVarMatch = false;
	int wordNo = zb_dic_search_word(pDicEc, pcWord);
	char acEngOrg[DIC_ENG_WORD_BUF_LEN];
	acEngOrg[0] = '\0';
	if (wordNo < 0) {
		ESP_LOGW(TAG, "try srch var [%s]", pcWord);
		//wordNo = dic_searchEngVar(pDicEc, pcWord, acEngOrg, sizeof(acEngOrg));
		wordNo = 0;
		if (wordNo >= 0) {
			isVarMatch = true;
		}
	}

	if (wordNo < 0) {
		ESP_LOGW(TAG, "Fail to findWord [%s]", pcWord);
		g_dicEnResultStart = -1;
		g_dicEnResultNum = 0;
	}
	else {
		int wordNoSt = 0;
		int8_t acMatchRateTbl[30];
		int wordNoBest = wordNo;
		char * pcWordN = (isVarMatch ? acEngOrg : pcWord);

		acMatchRateTbl[0] = 0;
		wordNoBest = zb_dic_find_contiue_match_words(pDicEc, pcWordN, wordNo, 
				&wordNoSt, acMatchRateTbl, sizeof(acMatchRateTbl),  false); 
		LV_ASSERT(wordNoBest >= 0);

		g_dicEnResultStart = wordNoSt;
		g_dicEnResultNum = strlen((char *)acMatchRateTbl);
		g_dicEnResultSel = (wordNoBest - wordNoSt);
		LV_ASSERT(g_dicEnResultNum <= sizeof(acMatchRateTbl) - 1);
		LV_ASSERT(g_dicEnResultSel >= 0 && g_dicEnResultSel < g_dicEnResultNum);
		// 
	}
	_updateOneEngWord(pDicEc);

	return;
}

static void _updateOneEngWord(struct DIC_LIB_s * pDicEc)
{
	bool hasExpl = false;
	int wordNo = (g_dicEnResultStart >= 0 ? g_dicEnResultStart + g_dicEnResultSel : - 1);
	int ret = zb_dic_get_explain(pDicEc, wordNo, g_pcExpl, C_EXPL_JSON_BUF_LEN);
	if (ret <= 0) {
		ESP_LOGE(TAG, "Fail to get expl, ret %d", ret);
	}
	else {
		hasExpl = true;
	}

	g_expScrollY = 0;
	if (!hasExpl) {
		lv_label_set_text(g_headwdObj, "");
		lv_obj_scroll_to_y(g_explObj, 0, LV_ANIM_OFF);
		lv_label_set_text(g_explObj, "Sorry! 查无此词");
		g_audNoUs = g_audNoUk = -1;
		lv_obj_add_flag(g_spkObjTbl[0], LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(g_spkObjTbl[1], LV_OBJ_FLAG_HIDDEN);
	}
	else {
		cJSON * cjsonRoot = NULL;
		cJSON * cjsonHeadwd = NULL, * cjsonPhn = NULL, * cjsonAud = NULL, * cjsonExpl = NULL;
		const char * pcHeadwd = "";
		const char * pcPhn = "",  * pcAud = NULL;
		const char * pcExpl = "";
		do {
			cjsonRoot = cJSON_Parse(g_pcExpl);
			cjsonHeadwd = cJSON_GetObjectItem(cjsonRoot, "w");
			if (cJSON_HasObjectItem(cjsonRoot, "p")) {
				cjsonPhn = cJSON_GetObjectItem(cjsonRoot, "p");
			}
			if (cJSON_HasObjectItem(cjsonRoot, "s")) {
				cjsonAud = cJSON_GetObjectItem(cjsonRoot, "s");
			}
			cjsonExpl = cJSON_GetObjectItem(cjsonRoot, "c");
			pcHeadwd = cjsonHeadwd->valuestring;
			if (cjsonPhn) {
				pcPhn = cjsonPhn->valuestring;
			}
			if (cjsonAud) {
				pcAud = cjsonAud->valuestring;
			}
			pcExpl = cjsonExpl->valuestring;
		} while(0);

		lv_label_set_text(g_headwdObj, pcHeadwd);

		g_pcExplInObj[0] = '\0';
		int pos = 0;
		if (pcPhn && pcPhn[0]) {
			#define PHN_BUF_LEN 70
			char acPhnA[PHN_BUF_LEN], acPhnB[PHN_BUF_LEN];
			acPhnA[0] = acPhnB[0] = '\0';
			cvtPhoneticToTwo(pcPhn, acPhnA, acPhnB, PHN_BUF_LEN);
			if (!acPhnB[0]) {
				sprintf(g_pcExplInObj, "/%s/", acPhnA);
			}
			else {
				sprintf(g_pcExplInObj, "/%s/  /%s/", acPhnA, acPhnB);
			}
			pos = strlen(g_pcExplInObj);
			g_pcExplInObj[pos++] = '\n';
			g_pcExplInObj[pos] = '\0';
		}
		g_audNoUs = g_audNoUk = -1;
		if (pcAud && pcAud[0]) {
			int32_t aiAudTwo[2] = { 0 };
			cvtAudioToTwo(pcAud, aiAudTwo);
			g_audNoUs = aiAudTwo[0];
			g_audNoUk = aiAudTwo[1];
		}
		strncat(g_pcExplInObj + pos, pcExpl, C_EXPL_TXT_BUF_LEN - pos - 1);
		if (cjsonRoot) {
			cJSON_Delete(cjsonRoot);  cjsonRoot = NULL;
		}

		g_isExplAtTailPg = false;
		lv_obj_scroll_to_y(g_explObj, 0, LV_ANIM_OFF);
		lv_label_set_text(g_explObj, g_pcExplInObj);
		int32_t aTwoAudId[2];
		aTwoAudId[0] = g_audNoUs;
		aTwoAudId[1] = g_audNoUk;
		int16_t audTypeNum = (g_audNoUs != -1) + (g_audNoUk != -1);
		g_audTypeSel = 0;
		if (audTypeNum >= 2) {
			_dictSpkObjSetCheck();
		}
		for (int16_t audType = 0; audType < 2; audType++) {
			if (aTwoAudId[audType] != -1) {
				lv_obj_clear_flag(g_spkObjTbl[audType], LV_OBJ_FLAG_HIDDEN);
			}
			else {
				lv_obj_add_flag(g_spkObjTbl[audType], LV_OBJ_FLAG_HIDDEN);
			}
		}
	}

	lv_area_t rcArea = { .x2 = SCREEN_WIDTH - 1, .y1 = SCREEN_HEIGHT - 28, .y2 = SCREEN_HEIGHT -1, };
	lv_obj_invalidate_area(g_bgWin, &rcArea);
	return;
}


/** 解析 json, 得到单语语音的 url
 * @return : 0 -无语音,  1 - 成功, < 0 失败
 */
static int readAudUrlFromJson() {
	cJSON * root = NULL;
	int iFuncRet = -1;
	do {
		const char * pcJson = NULL;
		const char * pcMainArr = NULL;  // "englishDictEntities" 
	#ifndef C_PC_SIM
		pcJson = g_plyHttpRsltBuf;
	#else
		pcJson = "{\"code\":200,\"englishDictEntities\":[{\"synonymList\":[\"Saul\"],\"phoneticEn\":\"pl\",\"explanationList\":[{\"pos\":\"n\",\"meaning\":\"保罗(男子名)\"}],\"dictCode\":\"110002\",\"phoneticUsAudio\":\"http://api.buguzx.cn/media/soundmark_source/dataOutUs/p/Paul.mp3\",\"id\":122925,\"word\":\"Paul\",\"phoneticEnAudio\":\"http://api.buguzx.cn/media/soundmark_source/dataOutUk/p/Paul.mp3\"}]}";
	#endif
		root = cJSON_Parse(pcJson);
		if (!root) {
			iFuncRet = -9;	break;
		}
		//ESP_LOGI(TAG, "rdAudRslt B");
		cJSON * jsonCode = cJSON_GetObjectItem(root, "code");
		if (!jsonCode || !cJSON_IsNumber(jsonCode)) {
			iFuncRet = -11;  break;
		}
		if (jsonCode->valueint != 200) {
			iFuncRet = -13;  break;
		}
		cJSON * jsonMArray = cJSON_GetObjectItem(root, "englishDictEntities");
		if (!jsonMArray || !cJSON_IsArray(jsonMArray)) {
			iFuncRet = -15;  break;
		}
		int16_t subNum = cJSON_GetArraySize(jsonMArray);
		const char * pcAimWord = lv_label_get_text(g_headwdObj);
		bool matchOnce = false;
		g_pcWordAudUk[0] = '\0';
		g_pcWordAudUs[0] = '\0';
		for (int16_t sub = 0; sub < subNum; sub++) { 
			cJSON * jsonWdItm = cJSON_GetArrayItem(jsonMArray, sub);
			if (!jsonWdItm)
				continue;
			cJSON * jsonWord = cJSON_GetObjectItem(jsonWdItm, "word");
			if (!jsonWord || !jsonWord->valuestring || !jsonWord->valuestring[0])
				continue;
			ESP_LOGI(TAG, "word %2hd %s", sub, jsonWord->valuestring);
			if (strcasecmp(jsonWord->valuestring, pcAimWord) == 0) { // match when ignoreCase
				cJSON * jsonAudEn = cJSON_GetObjectItem(jsonWdItm, "phoneticEnAudio");
				cJSON * jsonAudUs = cJSON_GetObjectItem(jsonWdItm, "phoneticUsAudio");
				bool hasAudEn = (jsonAudEn && jsonAudEn->valuestring && jsonAudEn->valuestring[0]);
				bool hasAudUs = (jsonAudUs && jsonAudUs->valuestring && jsonAudUs->valuestring[0]);
				if (!hasAudEn && !hasAudUs)
					continue;
				bool isExactMatch = false;
				if (strcmp(jsonWord->valuestring, pcAimWord) == 0) { // match exact
					isExactMatch = true;
				}
				if (!matchOnce  || isExactMatch) {
					g_pcWordAudUk[0] = '\0';
					g_pcWordAudUs[0] = '\0';
					if (hasAudEn) {
						strncat(g_pcWordAudUk, jsonAudEn->valuestring, C_AUD_URL_BUF_LEN - 1);
					}
					if (hasAudUs) {
						strncat(g_pcWordAudUs, jsonAudUs->valuestring, C_AUD_URL_BUF_LEN - 1);
					}
					if (isExactMatch)
						break;
				}
				matchOnce = true;
			}
		}
		iFuncRet = 0;
		if (matchOnce) {
			iFuncRet = 1;
		}
	} while (0);

	if (iFuncRet < 0) {
		ESP_LOGE(TAG, "rdAudUrl fail: %d", iFuncRet);
	}
	else {
		ESP_LOGI(TAG, "rdAudUrl done Z");
	}
	return iFuncRet;
}



static void _dic_timer_cb(struct _lv_timer_t * tmr) {
	int32_t tmCur = 0;

	tmCur = zb_get_time_ms_since_boot();
	if (tmCur >= (g_startTm + 600))
	{
		lv_timer_pause(tmr);
		_update_srchResult();
	}
}


static int initFlexView(lv_obj_t * flexObj) {
	char acSentence[240];
	acSentence[0] = '\0';

	zb_global_str_read(acSentence, sizeof(acSentence)); 
	if (!acSentence[0]) {  // debug
		strncat(acSentence, "China's new energy industry does not have an \"overcapacity\" issue, said Zhang Jianhua, head of the National Energy Administration on Thursday.",
				sizeof(acSentence) - 1);
	}
	int wordNum = _dic_split_words(acSentence);
	if (wordNum <= 0) {
		ESP_LOGE(TAG, "sentence is space!");
		return -1;
	}

	g_wordsNum = wordNum;
	for (int16_t wordNo = 0; wordNo < wordNum; wordNo++) {
		lv_obj_t * labelOne = lv_label_create(flexObj);
		lv_label_set_text(labelOne, g_wordsTbl[wordNo]);
		lv_obj_set_style_pad_all(labelOne, 3, 0);
		lv_obj_set_style_bg_color(labelOne, lv_color_hex(0xffa500), LV_STATE_CHECKED);
		lv_obj_set_style_bg_opa(labelOne, LV_OPA_COVER, 0);
	}
	return 0;
}

void dictScreen_del(void) {
	if (g_bgWin) {
		lv_obj_del(g_bgWin);  g_bgWin = NULL;
	}
	if (g_pDicEc != NULL) {	// if if quit AP, close dict.
		zb_dic_close(g_pDicEc);  g_pDicEc = NULL;
	}
	return;
}

static void _my_prompt_draw_cb(lv_event_t * evt)
{
#define MY_INFO_LOC_X  36
 	const misc_draw_item_t drawItems_spk[] = {
		{ .m_locX = MY_INFO_LOC_X, .m_locY =  SCREEN_HEIGHT - 28, .m_type = 1, 
		  .unn = { .m_imgHash = COMMON_IMG_OK_PLAYER_YELLOW },  },
		{ .m_locX = MY_INFO_LOC_X + 24, .m_locY =  SCREEN_HEIGHT - 28 + 3, .m_type = 0,
		  .unn.m_pText = "发音",   },
 	};

	const misc_draw_item_t drawItems_swch[] = {
		{ .m_locX = MY_INFO_LOC_X+66, .m_locY =  SCREEN_HEIGHT - 28, .m_type = 1, 
		  .unn = { .m_imgHash = COMMON_IMG_AB_KEY_YELLOW },  },
		{ .m_locX = MY_INFO_LOC_X+66+24, .m_locY =	SCREEN_HEIGHT - 28 + 3, .m_type = 0,
		  .unn.m_pText = "切换相似词" },
	};
	const misc_draw_mult_items_t stDrawItemsSpk = {
		.m_num = 2,
		.pstDrawItems = drawItems_spk,
	};
	const misc_draw_mult_items_t stDrawItemsSwch = {
		.m_num = 2,
		.pstDrawItems = drawItems_swch,
	};

	/* char acBuf[100];
	zb_bin2hex(&drawItems, sizeof(misc_draw_item_t) * 3, acBuf, sizeof(acBuf));
	ESP_LOGI(TAG, "mem: %s", acBuf);  */

	if (g_audNoUs >= 0 || g_audNoUk >= 0) {
		draw_txt_img_group_cb(evt, &stDrawItemsSpk);
	}
	if (g_dicEnResultNum >= 2) {
		draw_txt_img_group_cb(evt, &stDrawItemsSwch);
	}
}


static void _jmpDic_WinEvtCb(lv_event_t * evt) {
	lv_event_code_t evtCode = evt->code;

	int keyVal = 0;
	switch (evtCode) {
		case LV_EVENT_KEY_STAGE:
			keyVal = lv_event_get_key(evt);

			int keyType = (LV_EX_FLAG_KEY_MASK & keyVal);
			keyVal &= ~LV_EX_FLAG_KEY_MASK;
			if (LV_EX_FLAG_KEY_SHORT == keyType) {
				if (LV_KEY_ESC == keyVal) {
					//quit dict view.
					g_quitMode = 1;
					dictScreen_del();
					return;
				}
			}
			if (keyType != LV_EX_FLAG_KEY_REPEAT) {
				return;
			}
			
			// fallthrough here.
		case LV_EVENT_KEY: {
			if (!keyVal) {
				keyVal = lv_event_get_key(evt);
			}
			ESP_LOGI(TAG, "key %x", keyVal);
			switch(keyVal) {
				case LV_KEY_LEFT:
				case LV_KEY_RIGHT: {
					int diff = (keyVal == LV_KEY_LEFT ? g_wordsNum - 1 : 1);
					int wordNoNew = (g_wordSelNo + diff) % g_wordsNum;
					_selectAnotherWord(wordNoNew);
					break;
				}
				case LV_KEY_UP:
				case LV_KEY_DOWN: {
				#define MY_LABEL_PAGE_H 80
					int diffY = (LV_KEY_UP == keyVal ? -MY_LABEL_PAGE_H : MY_LABEL_PAGE_H);
					if ((g_expScrollY <= 0) && (diffY < 0)) { // at head, can not scroll up.
						return;
					}
					if (g_isExplAtTailPg && (diffY > 0))  // has turn to end.
						return;

					/* if (LV_KEY_DOWN == keyVal) {
						lv_area_t txt_coords;
						lv_obj_get_content_coords(g_explObj, &txt_coords);
						//lv_coord_t	scroll_max = lv_obj_get_scroll_top(obj) + lv_obj_get_scroll_bottom(obj);
						if (g_expScrollY + diffY + MY_LABEL_PAGE_H >= txt_coords.y2 - txt_coords.y1) 
							break;
					} */
					g_expScrollY += diffY;
					lv_obj_scroll_to_y(g_explObj, g_expScrollY, LV_ANIM_OFF);
					lv_coord_t yReal = lv_obj_get_scroll_y(g_explObj);
					if (yReal < g_expScrollY) {
						g_isExplAtTailPg = true;
					}
					else {
						g_isExplAtTailPg = false;
					}
					g_expScrollY = yReal;
					break;
				}
				case LV_KEY_ENTER: {
					lv_event_stop_processing(evt);
					lv_indev_keypad_wait_release();
			#if WORD_AUD_IS_ONLINE
				#ifndef C_PC_SIM
					if (!zb_is_wifi_connect()) 
				#else
					if (1)
				#endif
					{
						app_msgbox_timeout("","请先打开网络连接!", 1000, NULL, NULL);
						break;
					}
					bool isQueryAudProcessing = false;
					if ((0 != g_audQueryTm) || (zb_get_time_ms_since_boot() <= (g_audQueryTm + 6000))) {
						isQueryAudProcessing = true;
					}
					if (isQueryAudProcessing)
						break;
				#ifndef C_PC_SIM
					music_play_status_t audStat = music_player_state_get();
					if (MUSIC_STATUS_PLAYING == audStat) {
						ESP_LOGI(TAG, "stop prevPly");
						music_player_stop();
						break;
					}
				#endif
					const char * pcWord = lv_label_get_text(g_headwdObj);
					if (pcWord && pcWord[0]) {
						g_audQueryTm = zb_get_time_ms_since_boot();
						zb_global_str_write(pcWord);
						play_http_post_create(REQUEST_EN_WORD_AUD_URL); 
					}
			#else
					int audTypeNum = (g_audNoUs != -1) + (g_audNoUk != -1);
					if (audTypeNum <= 0)
						break;
					int16_t audType = 0;
					int sndIdx = -1;
					if (1 == audTypeNum) {
						audType = (g_audNoUs != -1 ? 0 : 1);
					}
					else if (2 == audTypeNum) {
						audType = g_audTypeSel;
					}
					else {
						ESP_LOGW(TAG, "word has no aud");
						break;
					}
					sndIdx = (0 == audType ? g_audNoUs : g_audNoUk);

					char audLibDecParm = 0;
					int ret = dic_ensureAudLibInfoInit(audType, &audLibDecParm);
					if (ret < 0) {
						ESP_LOGE(TAG, "fail to initDicAud");
						break;
					}
					int32_t audOffset = 0;
					int audLen = getAudLengthAndOffset(audType, sndIdx, &audOffset);
					if (audLen <= 0) {
						ESP_LOGW(TAG, "fail to get aud No %d, err %d", sndIdx, audLen);
						break;
					}
				#ifndef C_PC_SIM
					aud_file_part_play(NULL, g_aAudFileHdls[audType], audOffset, audLen, audLibDecParm, MUSIC_PLAY_SPEED_1_00);
				#endif
			#endif
					break;
				}
				case LV_KEY_EX_FUNC1: {  // 切换相似词 
					if (g_dicEnResultNum <= 1)
						break;
					g_dicEnResultSel++;
					if (g_dicEnResultSel >= g_dicEnResultNum) {
						g_dicEnResultSel = 0;
					}
					_updateOneEngWord(g_pDicEc);
					break;
				}
				default:
					break;
			}
			break;
		}
		case EVENT_EX_AUD_STREAM: {
			int * pInt = (int *)lv_event_get_param(evt);
			int parm = (pInt ? (*pInt) : 0);
			ESP_LOGI(TAG, "evtAudStrm 0x%x", parm);
			// lv_event_stop_processing(evt);
		#ifndef C_PC_SIM
			if (parm == MUSIC_STATUS_FINISH) {
				int audTypeNum = (g_audNoUs != -1) + (g_audNoUk != -1);
				if (audTypeNum >= 2) {	// switch spk btn;
					int16_t audTypeOld = g_audTypeSel;
					g_audTypeSel = !g_audTypeSel;
					_dictSpkObjSetCheck();
				}
			}
		#endif
			break;
		}
		case LV_EVENT_READY: {
			int * pInt = (int *)lv_event_get_param(evt);
			int parm = (pInt ? (*pInt) : 0);
			ESP_LOGI(TAG, "receive readyEvt %d", parm);

		#if WORD_AUD_IS_ONLINE
			switch (parm) {
				case NET_RSLT_WORD_AUD: {
					g_audQueryTm = 0;
					//---- parseAudUrl from json, play aud.
					int ret = readAudUrlFromJson();
					if (ret < 0)
						break;
				
					if (!g_pcWordAudUk[0] && !g_pcWordAudUs[0]) { // if word has no audio
						g_noAudFlag = true;
						lv_obj_add_flag(g_spkObjTbl[0], LV_OBJ_FLAG_HIDDEN);// hide playBtn
						lv_obj_add_flag(g_spkObjTbl[1], LV_OBJ_FLAG_HIDDEN);
						break;
					}
				#ifndef C_PC_SIM
					music_player_cfg_t player_cfg={
						.auto_mute = true ,
						.task_prio = 10 , 
						.user_data = NULL,
					};
					ESP_LOGI(TAG, "start ply A");
					int initRet = 0, playRet = 0;
					initRet = music_player_init(&player_cfg);
					music_player_set_status_callback(music_player_status);
					const char * pcAudUrl = (g_pcWordAudUk[0] ? g_pcWordAudUk : g_pcWordAudUs);
					ESP_LOGI(TAG, "start ply %s", pcAudUrl);
					playRet = music_player_set_play_uri(pcAudUrl,  MUSIC_SOURCE_TYPE_HTTP);
					ESP_LOGI(TAG, "mscPly ret %d", playRet);
				#endif
					break;
				}
				default:
					break;
			}
		#endif
			break;
		}
		case LV_EVENT_DELETE:
	#ifndef C_PC_SIM
		#if WORD_AUD_IS_ONLINE
			music_player_stop();
		#else
			audio_stream_stop_play();
		#endif
	#endif
			if (g_dicTmr) {
				lv_timer_del(g_dicTmr);   g_dicTmr = NULL;
			}
			dictScrResDeinit();
			g_headwdObj = NULL;
			g_explObj = NULL;  g_promptBar = NULL;
			if ((0 == g_quitMode) && (g_pDicEc != NULL)) {  // if if quit AP, close dict.
				zb_dic_close(g_pDicEc);  g_pDicEc = NULL;
			}
			if (g_quitMode) {  // switch to prev screen. 
				play_text_return_init(0);
			}
			g_bgWin = NULL;
			break;
		default:
			break;
	}
}


int dictScreen_init(void) {
	static const lv_coord_t col_dsc[] = // 
		{ 17, LV_GRID_FR(1), 17, LV_GRID_TEMPLATE_LAST };
	static const lv_coord_t row_dsc[] = // 
		{ 18, 50, 32, 34, 30, 8, LV_GRID_FR(1), 40, LV_GRID_TEMPLATE_LAST };

	ESP_LOGI(TAG, "showBgPlyDlg A");
	dictScrResInit();

	do {
		lv_obj_t * bgObj = lv_obj_create(lv_scr_act());
		lv_obj_set_size(bgObj, SCREEN_WIDTH, SCREEN_HEIGHT);
		
		lv_obj_set_grid_dsc_array(bgObj, col_dsc, row_dsc);
		lv_obj_set_style_pad_row(bgObj, 0, 0);
		lv_obj_set_style_pad_column(bgObj, 0, 0);

		//lv_obj_set_style_bg_color(bgObj, lv_color_hex(0x606060), 0);
		lv_obj_set_style_border_width(bgObj, 0, 0);
		lv_obj_set_style_radius(bgObj, 0, 0);
		lv_img_dsc_t * bgImgDsc = zb_img_res_load_one(NULL, PLAYER_IMG_DIC_BEIJING, IMG_FREE_TYPE_AUTO, NULL);
		lv_obj_set_style_bg_color(bgObj, lv_color_hex(0xb2abff), 0);
		lv_obj_set_style_bg_img_src(bgObj, bgImgDsc, 0);
		lv_obj_set_style_bg_img_opa(bgObj, LV_OPA_COVER, 0);

		lv_obj_t * flexObj = lv_obj_create(bgObj);
		lv_obj_set_flex_flow(flexObj, LV_FLEX_FLOW_ROW_WRAP);
		lv_obj_set_grid_cell(flexObj, LV_GRID_ALIGN_STRETCH, 1, 1,  LV_GRID_ALIGN_STRETCH, 1, 1);
		lv_obj_set_style_radius(flexObj, 0, 0);
		lv_obj_set_style_border_width(flexObj, 0, 0);
		//lv_obj_set_style_bg_opa(flexObj, LV_OPA_0, 0);


		lv_obj_t * headwdObj = lv_label_create(bgObj);
		lv_obj_set_grid_cell(headwdObj, LV_GRID_ALIGN_CENTER, 1, 1,  LV_GRID_ALIGN_STRETCH, 3, 1);
		lv_obj_set_style_text_align(headwdObj, LV_TEXT_ALIGN_CENTER, 0);
		//lv_obj_set_style_text_font(headwdObj, &lv_font_montserrat_20, 0);
		g_headwdObj = headwdObj;

		lv_obj_t * spkObj = lv_obj_create(bgObj);
		lv_obj_set_size(spkObj, 150, LV_SIZE_CONTENT);
		lv_obj_set_grid_cell(spkObj, LV_GRID_ALIGN_CENTER, 1, 1,  LV_GRID_ALIGN_CENTER, 4, 1);
		lv_obj_set_style_border_width(spkObj, 0, 0);

		lv_obj_t * spkObjA = lv_img_create(spkObj);
		const lv_img_dsc_t * imgDscSpkA = zb_img_res_load_one(NULL, PLAYER_IMG_DIC_MEISHI, 
				IMG_FREE_TYPE_AUTO, NULL);
		lv_obj_set_size(spkObjA, imgDscSpkA->header.w, imgDscSpkA->header.h);
		lv_obj_set_style_bg_img_src(spkObjA, imgDscSpkA, 0);
		lv_obj_align_to(spkObjA, spkObj, LV_ALIGN_LEFT_MID, 0, 0);
		g_spkObjTbl[0] = spkObjA;
		lv_obj_t * spkObjB = lv_img_create(spkObj);
		const lv_img_dsc_t * imgDscSpkB = zb_img_res_load_one(NULL, PLAYER_IMG_DIC_YINGSHI, 
				IMG_FREE_TYPE_AUTO, NULL);
		lv_obj_set_size(spkObjB, imgDscSpkB->header.w, imgDscSpkB->header.h);
		lv_obj_set_style_bg_img_src(spkObjB, imgDscSpkB, 0);
		lv_obj_align_to(spkObjB, spkObj, LV_ALIGN_RIGHT_MID, 0, 0);
		g_spkObjTbl[1] = spkObjB;
		lv_obj_set_style_border_width(spkObjA, 1, LV_STATE_CHECKED);
		lv_obj_set_style_radius(spkObjA, 10, 0);
		lv_obj_set_style_border_width(spkObjB, 1, LV_STATE_CHECKED);
		lv_obj_set_style_radius(spkObjB, 10, 0);

		lv_obj_t * explObj = lv_label_create(bgObj);
		lv_obj_set_grid_cell(explObj, LV_GRID_ALIGN_STRETCH, 1, 1,  LV_GRID_ALIGN_STRETCH, 6, 1);
		lv_obj_set_style_text_line_space(explObj, 3, 0);
		lv_obj_add_flag(explObj, LV_OBJ_FLAG_SCROLLABLE);
		lv_obj_set_scroll_dir(explObj, LV_DIR_VER);
		g_explObj = explObj;

		g_wordsTbl = (char **)heap_caps_malloc_dflt(sizeof(void *) * C_WORDS_N_MAX); 
		memset(g_wordsTbl, 0, sizeof(void *) * C_WORDS_N_MAX);

		initFlexView(flexObj);
		g_flexObj = flexObj;
		g_bgWin = bgObj;
		g_wordSelNo = WORD_SEL_NO_NONE; 
		g_quitMode = 0;
		g_isExplAtTailPg = false;
		lv_obj_add_event_cb(bgObj, _jmpDic_WinEvtCb, LV_EVENT_ALL, NULL);
		lv_obj_add_event_cb(bgObj, _my_prompt_draw_cb, LV_EVENT_DRAW_POST, NULL);
		lv_group_add_obj(lv_group_get_default(), bgObj);
		sys_setCurMainWinObj(bgObj);
		g_dicTmr = lv_timer_create(_dic_timer_cb, 250, NULL);
		lv_timer_pause(g_dicTmr);
		_selectAnotherWord(0);
	}while(0);

	return 0;
}



