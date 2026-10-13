
#include <time.h>
#include <lvgl/lvgl.h>
#include "../utils/misc_utils.h"

#ifndef C_PC_SIM
#include "../include/adc_battery.h"
#include "../alarm_sdcard_evt_proc.h"

extern bool zb_is_wifi_connect();
#endif

#include "../lvgl_main.h"

#if 0
#include "../data_parse/img_res.h"
#include "../data_parse/cJSON.h"

#include "../utils/global_parm.h"

#include "book_sel.h"
#include "book_sel_util.h"

extern volatile char g_token[];

//------------------------------------------------------

#define TAG   "bkslUtl"

#define BKSL_CACHE_FHEAD_V1_STR  "bkslCac1"
#define BKSL_CACHE_FHEAD_V2_STR  "bkslCac2"

//------------------------------------------------------

/* 由年级名求得(内部的)年级号 
 *   这里对应的 namibox 里的年级名, 不会出现 [初四]的名称 
 */
static int16_t getGradeNoFromText(const char * pcGrade) {
	const char * pcGradeChrs = "一二三四五六初高";

	char acGrade1stCh[5];
	memcpy(acGrade1stCh, pcGrade, 3);
	acGrade1stCh[3] = '\0';

	char * pcTmp = strstr(pcGradeChrs, acGrade1stCh);
	if (!pcTmp) {
		ESP_LOGE(TAG, "error grade %s", pcGrade);
		return 1;  // error
	}
	int pos = (pcTmp - pcGradeChrs) / 3;
	if (pos < 6)
		return pos + 1;
	else {
		int gradeBase = 6;  // 初中
		if (memcmp(pcGrade, "\xe5\x88\x9d", 3) != 0) { 
			gradeBase = 9;  // 高中
		}
		if (memcmp(pcGrade + 3, "\xe4\xb8\x80", 3) == 0) { // 一 
			return gradeBase + 1;
		}
		if (memcmp(pcGrade + 3, "\xe4\xba\x8c", 3) == 0) { // 二 
			return gradeBase + 2;
		}
		if (memcmp(pcGrade + 3, "\xe4\xb8\x89", 3) == 0) { // 三 
			return gradeBase + 3;
		}
	}
	return 1;
}


/** 释放指定科目的年级版本数据.
 */
void gradePublishsInfo_free(struct grade_publish_inf * pSubjPubls)
{
	ESP_LOGI(TAG, "grdPublInf free");
	LV_ASSERT(pSubjPubls);
	for (int16_t grade = 0; grade < BKSEL_GRADE_TOTAL; grade++) {
		char ** pcPublsNames = pSubjPubls[grade].ppcPublishs; // point to head of this grade's infoTbl.
		int16_t publNum = pSubjPubls[grade].m_publishNum;
		if (publNum > 0) {
			LV_ASSERT(pcPublsNames);
			for (int16_t sub = 0; sub < publNum; sub++) {
				LV_ASSERT(pcPublsNames[sub]);
				heap_caps_free(pcPublsNames[sub]);  // free space of string.
				pcPublsNames[sub] = NULL;
			}
			heap_caps_free(pcPublsNames);  // free this arrayOfPointer.
			pSubjPubls[grade].ppcPublishs = NULL;
			pSubjPubls[grade].m_publishNum = 0;
		}
	}
	return;
}


/**
 * @param pcSubject - json 里的科目字段名 
 * Note: pDstInfTbl[] item num is BKSEL_GRADE_TOTAL at least. 
 */
int readGradePublishsFromJson(const char * pcJson, const char * pcSubject, struct grade_publish_inf * pDstInfTbl)
{
	int iFuncRet = -1;
	cJSON * root = NULL;
	do {
		if (!pcJson || !pcJson[0]) {
			iFuncRet = -3;  break;
		}
		if (!pDstInfTbl) {
			iFuncRet = -5;  break;
		}
		ESP_LOGI(TAG, "rdGrdPubl A");
		root = cJSON_Parse(pcJson);
		if (!root) {
			iFuncRet = -9;  break;
		}

		ESP_LOGI(TAG, "rdGrdPubl C");
		cJSON * jsonCode = cJSON_GetObjectItem(root, "code");
		if (!jsonCode || !cJSON_IsNumber(jsonCode)) {
			iFuncRet = -11;  break;
		}
		if (jsonCode->valueint != 0) {
			iFuncRet = -13;  break;
		}

		ESP_LOGI(TAG, "rdGrdPubl E");
		cJSON * jsonData = cJSON_GetObjectItem(root, "data");
		if (!jsonData || !cJSON_IsObject(jsonData)) {
			iFuncRet = -15;  break;
		}
		ESP_LOGI(TAG, "rdGrdPubl G");
		cJSON * jsonGradeVer = cJSON_GetObjectItem(jsonData, "grade_book_version");
		if (!jsonGradeVer || !cJSON_IsObject(jsonGradeVer)) {
			iFuncRet = 17;  break;
		}

		ESP_LOGI(TAG, "rdGrdPubl J");
		cJSON * jsonGradePubl = cJSON_GetObjectItem(jsonGradeVer, pcSubject);
		if (!jsonGradePubl || !cJSON_IsArray(jsonGradePubl)) {
			iFuncRet = 19;  break;
		}

		gradePublishsInfo_free(pDstInfTbl);  // 释放原来的年级版本数据.

		ESP_LOGI(TAG, "rdGrdPubl L");
		int16_t gradeTypeN = cJSON_GetArraySize(jsonGradePubl);
		for (int16_t subGrade = 0; subGrade < gradeTypeN; subGrade++)
		{
			cJSON * jsonGradeItm = cJSON_GetArrayItem(jsonGradePubl, subGrade);
			ESP_LOGI(TAG, "rdGrdPubl P No %hd", subGrade);
			cJSON * jsonGrade = cJSON_GetObjectItem(jsonGradeItm, "grade");
			if ((jsonGrade != NULL) && cJSON_IsString(jsonGrade)) {
				int16_t gradeNo = getGradeNoFromText(jsonGrade->valuestring);
				struct grade_publish_inf * pDstInf = pDstInfTbl + (gradeNo - 1);
				pDstInf->m_grade = gradeNo;
				ESP_LOGI(TAG, "rdGrdPubl Q grade %hd", gradeNo);

				cJSON * jsonPubls = cJSON_GetObjectItem(jsonGradeItm, "book_versions");
				int16_t publNum = cJSON_GetArraySize(jsonPubls);
				if (publNum > BKSEL_ITEM_N_MAX) {
					ESP_LOGW(TAG, "publN is too big,n:%d", publNum);
					publNum = BKSEL_ITEM_N_MAX;
				}
				pDstInf->ppcPublishs = (char **)heap_caps_malloc_dflt(sizeof(void *) * publNum);
				pDstInf->m_publishNum = publNum;
				memset(pDstInf->ppcPublishs, 0, sizeof(void *) * publNum);
				for (int16_t lit = 0; lit < publNum; lit++) {
					cJSON * jsonPublOne = cJSON_GetArrayItem(jsonPubls, lit);
					//ESP_LOGI(TAG, "lit No%hd, %s", lit, jsonPublOne->valuestring);
					pDstInf->ppcPublishs[lit] = zb_strdup(jsonPublOne->valuestring);
				}
			}
		}

		for (int16_t gradeNo = 10; gradeNo < BKSEL_GRADE_TOTAL + 1; gradeNo++ )  // sugy 20240914 for (主要是语文的出版社) 没有高中部分、初四, non-std.
		{
			struct grade_publish_inf * pDstInf = pDstInfTbl + (gradeNo - 1);
			if (0 == pDstInf->m_publishNum) {
				pDstInf->m_grade = gradeNo;
				pDstInf->m_publishNum = 1;
				pDstInf->ppcPublishs = (char **)heap_caps_malloc_dflt(sizeof(void *) * 1);
				pDstInf->ppcPublishs[0] = zb_strdup("人教版");
			}
		}

		iFuncRet = 0;
	} while (0);

	if (root) {
		cJSON_Delete(root);  root = NULL;
	}
	if (iFuncRet < 0) {
		ESP_LOGE(TAG, "rdGrdPubl fail: %d", iFuncRet);
	}
	else {
		ESP_LOGI(TAG, "rdGrdPubl done z");
	}
	return iFuncRet;
}


#if 1


/** 把书本出版社信息写入初始化 
 */
struct BOK_VER_CACHE_WRT_STAT * bok_ver_cache_wrt_begin(void)
{
	ESP_LOGI(TAG, "cachWrt begin A");
	struct BOK_VER_CACHE_WRT_STAT * pCacheWrtStat = NULL;

	pCacheWrtStat = (struct BOK_VER_CACHE_WRT_STAT *)heap_caps_malloc_dflt(sizeof(struct BOK_VER_CACHE_WRT_STAT));
	memset(pCacheWrtStat, 0, sizeof(struct BOK_VER_CACHE_WRT_STAT));
	;
	return pCacheWrtStat;
}

/** 把书本出版社信息写入一个学科的 
 * @note 要求:第一次调用该函数写入语文的出版社, 第二次调用 写入英语的出版社 
 */
int bok_ver_cache_wrt_once(struct BOK_VER_CACHE_WRT_STAT * pCacheWrtStat, const char * pcJson)
{
	LV_ASSERT(pCacheWrtStat);
	LV_ASSERT(pCacheWrtStat->m_wrtStep < 2);
	int jsonLen = strlen(pcJson);

	if (0 == pCacheWrtStat->m_wrtStep) {
		int bufSize = sizeof(struct BOK_VER_CACHE_FHEAD) + jsonLen + 1;
		pCacheWrtStat->m_pBuf = (char *)heap_caps_malloc_dflt(bufSize);
		pCacheWrtStat->m_bufSize = bufSize;

		memcpy(pCacheWrtStat->m_pBuf, &(pCacheWrtStat->stCacheFHead), sizeof(struct BOK_VER_CACHE_FHEAD));

		pCacheWrtStat->stCacheFHead.m_chnJsonOffset = sizeof(struct BOK_VER_CACHE_FHEAD);
		pCacheWrtStat->stCacheFHead.m_chnJsonLen = jsonLen;
		memcpy((char *)pCacheWrtStat->m_pBuf + sizeof(struct BOK_VER_CACHE_FHEAD), pcJson, jsonLen + 1);

		pCacheWrtStat->m_dataOffset = bufSize;
	}
	else {
		int bufSize = pCacheWrtStat->m_dataOffset + jsonLen + 1;
		pCacheWrtStat->m_pBuf = (char *)heap_caps_realloc_dflt(pCacheWrtStat->m_pBuf, bufSize);

		pCacheWrtStat->stCacheFHead.m_engJsonOffset = pCacheWrtStat->m_dataOffset;
		pCacheWrtStat->stCacheFHead.m_engJsonLen = jsonLen;
		memcpy((char *)pCacheWrtStat->m_pBuf + pCacheWrtStat->m_dataOffset, pcJson, jsonLen + 1);

		pCacheWrtStat->m_dataOffset += jsonLen + 1;
	}
	pCacheWrtStat->m_wrtStep++;
	return 0;
}


void bok_ver_cache_wrt_end(struct BOK_VER_CACHE_WRT_STAT * pCacheWrtStat, bool doesSave, uint32_t tokenTmStampMinute)
{
	LV_ASSERT(pCacheWrtStat);

	ESP_LOGI(TAG, "cachWrt end A");
	if (doesSave) {
		// complete fhead info;
		struct BOK_VER_CACHE_FHEAD  *pstFHead =  &(pCacheWrtStat->stCacheFHead);
		pstFHead->m_timeStamp = (uint32_t)time(NULL);
		pstFHead->m_timeStampTokenMinute = tokenTmStampMinute;
	#ifndef C_PC_SIM
		strlcpy(pstFHead->m_acToken, g_token, sizeof(pstFHead->m_acToken));
	#endif
		memcpy(pstFHead->m_acIdentifier, BKSL_CACHE_FHEAD_V2_STR, 8);
		memcpy(pCacheWrtStat->m_pBuf, pstFHead, sizeof(struct BOK_VER_CACHE_FHEAD)); // update final fileHead.

		FILE * fp = NULL;
		do {
			fp = zb_fopen(PATH_BKSEL_CACHE, "wb");
			if (!fp) {
				ESP_LOGE(TAG, "fail to crt file %s", PATH_BKSEL_CACHE);
				break;
			}
			fwrite(pCacheWrtStat->m_pBuf, 1, pCacheWrtStat->m_dataOffset, fp);
		} while (0);

		if (fp) {
			zb_fclose(fp);  fp = NULL;
		}
		ESP_LOGI(TAG, "cachWrt aft close");
	}

	if (pCacheWrtStat->m_pBuf) {
		heap_caps_free(pCacheWrtStat->m_pBuf);  pCacheWrtStat->m_pBuf = NULL;
	}
	heap_caps_free(pCacheWrtStat);
	ESP_LOGI(TAG, "cachWrt end Z");
	return;
}


/**
 * @return >= 0 means success 
 */
int bok_readTokenInCacheFile(uint32_t * pTimeStampTokenMinute)
{
#define JC_TOKEN_VALID_PERIOD_SEC  (3600 * 24 * 7)
	FILE * fp = NULL;
	int iFuncRet = -1;

	ESP_LOGI(TAG, "rdTokInCchFl A");
	do {
		struct BOK_VER_CACHE_FHEAD stFHead = { 0 };
		fp = zb_fopen(PATH_BKSEL_CACHE, "rb");
		if (!fp) {
			iFuncRet = -5;	break;
		}
		fread(&stFHead, 1, sizeof(struct BOK_VER_CACHE_FHEAD), fp);
		int8_t fheadVer = -1;
		if (memcmp(stFHead.m_acIdentifier, BKSL_CACHE_FHEAD_V2_STR, 8) == 0) {
			fheadVer = 2;
		}
		if (fheadVer < 0) {
			iFuncRet = -7;  break;
		}
		if ((0 == stFHead.m_timeStampTokenMinute) || !stFHead.m_acToken[0]) {
			iFuncRet = -11;  break;
		}
			
		time_t timeNow = time(NULL);
		int64_t timeOfToken = (int64_t)stFHead.m_timeStampTokenMinute * 60;
		if (!((timeNow >= timeOfToken) && (timeNow < timeOfToken + JC_TOKEN_VALID_PERIOD_SEC)) ) {
			iFuncRet = -15;  break;
		}
	#ifndef C_PC_SIM
		strlcpy(g_token, stFHead.m_acToken, 33);  // non-std TOKEN_LENGTH
	#endif
		ESP_LOGI(TAG, "got bok token from fl, %s", stFHead.m_acToken);
		if (pTimeStampTokenMinute) {
			(*pTimeStampTokenMinute) = stFHead.m_timeStampTokenMinute;
		}
		iFuncRet = 0;
	}while (0);

	if (fp) {
		zb_fclose(fp);  fp = NULL;
	}
	if (iFuncRet < 0) {
		ESP_LOGW(TAG, "rdTokInCchFl fail, ret %d", iFuncRet);
	}
	return iFuncRet;
}


/** 由缓存文件 来初始化某一学科的出版社信息 
 * @param subject 0-语文， 1-英语 
 * @param return < 0 means fail
 */
int initPublishOfSubject(int16_t subject, struct grade_publish_inf * pDstInfTbl, uint32_t * piTmStamp)
{
	FILE * fp = NULL;
	char * pcJson = NULL;
	int iFuncRet = -1;

	if (!zb_isFileExist(PATH_BKSEL_CACHE)) {
		ESP_LOGW(TAG, "no cache file %s", PATH_BKSEL_CACHE);
		return -3;
	}

	ESP_LOGI(TAG, "intPubl of sub %hd", subject);
	do {
		struct BOK_VER_CACHE_FHEAD stFHead = { 0 };
		fp = zb_fopen(PATH_BKSEL_CACHE, "rb");
		if (!fp) {
			iFuncRet = -5;	break;
		}
		fread(&stFHead, 1, sizeof(struct BOK_VER_CACHE_FHEAD), fp);
		int8_t fheadVer = -1;
		if (memcmp(stFHead.m_acIdentifier, BKSL_CACHE_FHEAD_V1_STR, 8) == 0) {
			fheadVer = 1;
		}
		else if (memcmp(stFHead.m_acIdentifier, BKSL_CACHE_FHEAD_V2_STR, 8) == 0) {
			fheadVer = 2;
		}
		if (fheadVer < 0) {
			iFuncRet = -7;	break;
		}
		int jsonLen = (0 == subject ? stFHead.m_chnJsonLen : stFHead.m_engJsonLen);
		int32_t jsonOffset = (0 == subject ? stFHead.m_chnJsonOffset : stFHead.m_engJsonOffset);
		if (jsonLen < 2 || jsonLen > 1024 * 1024) {
			iFuncRet = -9;	break;
		}
		if (piTmStamp) {
			(*piTmStamp) = stFHead.m_timeStamp;
		}
		pcJson = heap_caps_malloc_dflt(jsonLen + 1);
		LV_ASSERT(pcJson);
		fseek(fp, jsonOffset, SEEK_SET);
		fread(pcJson, 1, jsonLen, fp);
		pcJson[jsonLen] = '\0';

		const char * pcSubject = (0 == subject ? "语文" : "英语");
		int ret = readGradePublishsFromJson(pcJson, pcSubject, pDstInfTbl);
		if (ret < 0) {
			iFuncRet = -13;
		}
		else {
			iFuncRet = 0;
		}
	} while (0);


	if (pcJson) {
		heap_caps_free(pcJson);  pcJson = NULL;
	}
	if (fp) {
		zb_fclose(fp);  fp = NULL;
	}

	if (iFuncRet < 0) {
		ESP_LOGE(TAG, "initPublOfSubj fail, ret %d", iFuncRet);
	}
	return iFuncRet;
}




#endif
#endif


