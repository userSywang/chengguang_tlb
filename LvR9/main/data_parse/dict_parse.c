/** zb 词库解析 
 */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define _DICT_PARSE_C_

#include "../utils/misc_utils.h"
//#include "../../components/lvgl__lvgl/src/misc/lv_assert.h" 

#ifndef C_PC_SIM
#include  <cJSON.h>
#else
#include "../data_parse/cJSON.h"
#endif


#define TAG "dicPrs"


#define DIC_LANG_ENG 1
#define DIC_LANG_CHN 2

struct DIC_LIB_s {
    int32_t  m_word_n;
    int32_t  m_expl_n;
    //--------- private area
    uint8_t m_encParam;
    int8_t  m_langType;  // 1 - eng
    int32_t  m_wordIdxTblAddr;
    int32_t  m_wordAreaBaseAddr;

    int32_t  m_wordMapTblAddr;
    int32_t  m_explIdxTblAddr;
    int32_t  m_explAreaBaseAddr;

    FILE  * m_fp;
};

//---------------------------------------------
static void _dic_dec_string(char * pStr, int len, uint8_t encParam)
{
	for (int idx = 0; idx < len; idx++) {
		pStr[idx] ^= encParam;
	}
}

struct DIC_LIB_s * zb_dic_open(const char * pcDicPath, int * piErrNo)
{
	int iFuncRet = -1;
	struct DIC_LIB_s * pstDic = NULL;
	FILE * fp = NULL;
	do {
		char acBuf[96] = { 0 };
		fp = zb_fopen(pcDicPath, "rb");
		if (!fp) {
			ESP_LOGW(TAG, "Fail to openF %s", pcDicPath);
            iFuncRet = -3;  break;
        }
		
		zb_fread(acBuf, 1, 96, fp);
		if (memcmp(acBuf, "DICTZbTlb_01", 12) != 0) {
			iFuncRet = -5;  break;
		}
		pstDic = heap_caps_malloc_dflt(sizeof(struct DIC_LIB_s));
		memset(pstDic, 0, sizeof(struct DIC_LIB_s));
		pstDic->m_encParam = acBuf[0x12];
		pstDic->m_langType = acBuf[0x16];
		pstDic->m_word_n = GET_UINT32_LITTLEENDIAN(acBuf+0x18);
		pstDic->m_expl_n = GET_UINT32_LITTLEENDIAN(acBuf+0x1c);
		pstDic->m_wordIdxTblAddr   = GET_UINT32_LITTLEENDIAN(acBuf+0x28);
		pstDic->m_wordAreaBaseAddr = GET_UINT32_LITTLEENDIAN(acBuf+0x30);

		pstDic->m_wordMapTblAddr = GET_UINT32_LITTLEENDIAN(acBuf+0x38);
		pstDic->m_explIdxTblAddr = GET_UINT32_LITTLEENDIAN(acBuf + 0x40);
		pstDic->m_explAreaBaseAddr = GET_UINT32_LITTLEENDIAN(acBuf+0x48);

		pstDic->m_fp = fp;
		iFuncRet = 0;
	} while (0);

	if (iFuncRet < 0) {
		if (fp) {
			zb_fclose(fp);  fp = NULL;
		}
		if (pstDic) {
			heap_caps_free(pstDic);  pstDic = NULL;
		}
		if (piErrNo) {
			(*piErrNo) = iFuncRet;
		}
		return NULL;
	}
	return pstDic;
}


void zb_dic_close(struct DIC_LIB_s * pstDic)
{
	if (!pstDic) return;

	if (pstDic->m_fp) {
		zb_fclose(pstDic->m_fp);  pstDic->m_fp = NULL;
	}
	;
	heap_caps_free(pstDic);
	return;
}


/**
 * @return > 0 means word length, < 0 means fail. 
 */
int zb_dic_read_one_head(struct DIC_LIB_s * pstDic, int wordNo, char * pcBuf, int bufLen)
{
	int iFuncRet = -1;

	do {
		if (!pstDic || (wordNo < 0) || !pcBuf || bufLen < 2) {
			iFuncRet = -3;  break;
		}
		if (wordNo >= pstDic->m_word_n) {
			iFuncRet = -5;  break;
		}
		int32_t  aiOffsetTwo[2] = { 0 };
		zb_fseek(pstDic->m_fp, pstDic->m_wordIdxTblAddr + 4 * wordNo, 0);
		zb_fread(aiOffsetTwo, 4, 2, pstDic->m_fp);
		int wordLen = aiOffsetTwo[1] - aiOffsetTwo[0];
		if (wordLen < 1) {
			iFuncRet = -9;  break;
		}
		zb_fseek(pstDic->m_fp, pstDic->m_wordAreaBaseAddr + aiOffsetTwo[0], 0);
		if (wordLen >= bufLen - 1) {
			wordLen = bufLen - 1;
		}
		zb_fread(pcBuf, 1, wordLen, pstDic->m_fp);
		pcBuf[wordLen] = '\0';
		if (pstDic->m_encParam) {
			_dic_dec_string(pcBuf, wordLen, pstDic->m_encParam);
		}
		iFuncRet = wordLen;
	} while (0);

	if (iFuncRet < 0) {
		ESP_LOGE(TAG, "fail to rd, %d", iFuncRet);
	}
	return iFuncRet;
}

int zb_dic_eng_head_to_simple(const char * pcSrc, char * pcDst, int bufLen)
{
	if (!pcSrc || !pcDst || bufLen < 2) {
		return -1;
	}

	int16_t posI = 0, posO = 0;
	while (1) {
		char ch = pcSrc[posI];
		if (!ch) break;
		if (ch >= 'A' && ch <= 'Z') {
			ch += ('a' - 'A');
		}
		if ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) {
			pcDst[posO++] = ch;
			if (posO >= bufLen - 1) break;
		}
		posI++;
	}
	pcDst[posO] = '\0';
	return posO;
}

inline static void _readWordSimple(struct DIC_LIB_s * pDic, int wordNo, char * pcDst, int bufLen)
{
	char acCurWord[120];
	zb_dic_read_one_head(pDic, wordNo, acCurWord, sizeof(acCurWord));
	if (DIC_LANG_ENG== pDic->m_langType) {
		zb_dic_eng_head_to_simple(acCurWord, pcDst, bufLen);
	} else {
		pcDst[0] = '\0';
		strncat(pcDst, acCurWord, bufLen - 1);
	}
}

#define C_WORD_BUF_LEN 100

int zb_dic_search_word(struct DIC_LIB_s * pDic, const char * pcWord)
{
	char acAimSimple[C_WORD_BUF_LEN];
	char acTmpSimple[C_WORD_BUF_LEN];

	bool isEngDic = (DIC_LANG_ENG == pDic->m_langType);
	if (isEngDic) {
		zb_dic_eng_head_to_simple(pcWord, acAimSimple, sizeof(acAimSimple));
	} else {
		acAimSimple[0] = '\0';
		strncat(acAimSimple, pcWord, sizeof(acAimSimple) - 1);
	}

	int iL = 0, iH = pDic->m_word_n - 1;
	int cmpRet = 0;
	_readWordSimple(pDic, 0, acTmpSimple, sizeof(acTmpSimple));
	cmpRet = strcmp(acAimSimple, acTmpSimple);
	if (cmpRet < 0) {
		return -1;
	}
	if (0 == cmpRet) {
		return iL;
	}

	_readWordSimple(pDic, iH, acTmpSimple, sizeof(acTmpSimple));
	cmpRet = strcmp(acAimSimple, acTmpSimple);
	if (cmpRet > 0) {
		return -3;
	}
	if (0 == cmpRet) {
		return iH;
	}

	int iMid = 0;
	while (iL + 1 < iH) {
		iMid = (iL + iH) / 2;
		_readWordSimple(pDic, iMid, acTmpSimple, sizeof(acTmpSimple));
		cmpRet = strcmp(acTmpSimple, acAimSimple);
		if (cmpRet < 0) {
			iL = iMid;
		}
		else if (cmpRet > 0) {
			iH = iMid;
		}
		if (0 == cmpRet) {
			// todo, cmp recent continous words
			return iMid;
		}
	}
	return -2;
}

int16_t zb_dic_getMatchRate(struct DIC_LIB_s * pDic, int wordNo, const char * pcAimWord, const char * pcAimWordSmpl) 
{
	char acCurWord[C_WORD_BUF_LEN];
	char acCurWordSmpl[C_WORD_BUF_LEN];
	acCurWord[0] = '\0';
	zb_dic_read_one_head(pDic, wordNo, acCurWord, sizeof(acCurWord));
	zb_dic_eng_head_to_simple(acCurWord, acCurWordSmpl, sizeof(acCurWordSmpl));
	if (strcmp(acCurWordSmpl, pcAimWordSmpl) != 0)
		return 0;
	if (strcmp(acCurWord, pcAimWord) == 0) // match exactly
		return 0x7f;
	if (strcasecmp(acCurWord, pcAimWord) == 0) { // match when ignoreCase
		if (pcAimWord[0] && pcAimWord[1] &&
			strcmp(acCurWord + 1, pcAimWord + 1) == 0)
		{
			return 0x70;  // only first char case different
		}
		return 0x40;  // just letter case different
	}
	return 10;
}


/** 
 * @return >=0 0 means success; 其值为匹配度最高的那一词条的序号;
 * Note: 返回的 pMatchRateTbl[] 以 0 值作为结尾标记;
 *   本函数执行后, caller  调用 strlen(pMatchRateTbl) 即可得形似词个数.  
 */
int zb_dic_find_contiue_match_words(struct DIC_LIB_s * pDic, const char * pcAimWord, 
		int searchOrgNo, int * piWordNoSt, int8_t * pMatchRateTbl, int16_t tblLen, bool isSentenceHead) 
{
	char acAimWordSmpl[C_WORD_BUF_LEN];

	if (!pDic || !pcAimWord || !pcAimWord[0])
		return -1;
	if ((searchOrgNo < 0) || !piWordNoSt)
		return -2;
	if (!pMatchRateTbl || (tblLen < 2))
		return -3;
	acAimWordSmpl[0] = '\0';
	zb_dic_eng_head_to_simple(pcAimWord, acAimWordSmpl, sizeof(acAimWordSmpl));

	pMatchRateTbl[tblLen - 1] = 0;
	int16_t tblPos = tblLen - 1;
	int wordNo = searchOrgNo;
	// find match backword;
	while (wordNo > 0) {
		int16_t matchRate = zb_dic_getMatchRate(pDic, wordNo, pcAimWord, acAimWordSmpl);
		if (matchRate <= 0)
			break;
		tblPos--;
		pMatchRateTbl[tblPos] = (int8_t)matchRate;
		wordNo--;
	}

	int partAItemN = tblLen - 1 - tblPos;
	if (tblPos != 0) {
		memmove(pMatchRateTbl, pMatchRateTbl + tblPos, partAItemN + 1);
	}
	tblPos = partAItemN;
	(*piWordNoSt) = searchOrgNo - (partAItemN - 1);

	wordNo = searchOrgNo + 1;
	while (wordNo < pDic->m_word_n) {
		if (tblPos >= tblLen - 1)
			break;
		int16_t matchRate = zb_dic_getMatchRate(pDic, wordNo, pcAimWord, acAimWordSmpl);
		if (matchRate <= 0)
			break;
		pMatchRateTbl[tblPos] = (int8_t)matchRate;
		wordNo++;
		tblPos++;
	}
	pMatchRateTbl[tblPos] = 0; // file tailMark.
	int16_t matchNum = tblPos;

	//---------------- find the maxMatch itemNo.
	int8_t matchRatMax = pMatchRateTbl[0];
	int16_t matchMaxPos = 0;
	for (int16_t sub = 1; sub < matchNum; sub++) {
		if (matchRatMax < pMatchRateTbl[sub]) {
			matchRatMax = pMatchRateTbl[sub];
			matchMaxPos  = sub;
		}
	}
	return (*piWordNoSt) + matchMaxPos;
}



/**
 * read explain.
 * when pcBuf is NULL, only measure expl text length. 
 * @return the length of explain.
 */
int zb_dic_get_explain(struct DIC_LIB_s * pDic, int wordNo, char * pcBuf, int bufLen)
{
	int iFuncRet = -1;
	do {
		if (!pDic || (wordNo < 0)) {
			iFuncRet = -1;	break;
		}
		if (wordNo >= pDic->m_word_n) {
			iFuncRet = -3;	break;
		}
		int32_t explNo = 0;
		zb_fseek(pDic->m_fp, pDic->m_wordMapTblAddr + 4 * wordNo, 0);
		zb_fread(&explNo, 1, 4, pDic->m_fp);
		if (explNo < 0 || explNo >= pDic->m_expl_n) {
			iFuncRet = -7;  break;
		}
		int32_t aiExplAddrTwo[2] = { 0 };
		zb_fseek(pDic->m_fp, pDic->m_explIdxTblAddr + 4 * explNo, 0);
		zb_fread(aiExplAddrTwo, 2, 4, pDic->m_fp);

		int explLen = aiExplAddrTwo[1] - aiExplAddrTwo[0];
		if (!pcBuf) {
			return explLen;
		}
		if (explLen < 1) {
			iFuncRet = -11;  break;
		}
		if (explLen + 1 > bufLen) {
			iFuncRet = -13;  break;
		}
		zb_fseek(pDic->m_fp, pDic->m_explAreaBaseAddr + aiExplAddrTwo[0], 0);
		zb_fread(pcBuf, 1, explLen, pDic->m_fp);
		pcBuf[explLen] = '\0';\
		if (pDic->m_encParam) {
			_dic_dec_string(pcBuf, explLen, pDic->m_encParam);
		}
		iFuncRet = explLen;
	} while (0);
	if (iFuncRet < 0) {
		ESP_LOGE(TAG, "getExpl fail,ret %d", iFuncRet);
	}
	return iFuncRet;
}

void cvtPhoneticToTwo(const char * pcPhnOrg, char * pcPhnA, char * pcPhnB, int bufLen)
{
	int iFuncRet = -1;
	if (!pcPhnOrg || !pcPhnA || !pcPhnB || (bufLen < 2)) 
		return;
	(*pcPhnA) = (*pcPhnB) = '\0';
	char * pMid = strchr(pcPhnOrg, ';');
	if (!pMid) {
		strncat(pcPhnA, pcPhnOrg, bufLen - 1);
	}
	else {
		char * pPartB = pMid + 1;
		int partALen = (pMid - pcPhnOrg);
		if (partALen > bufLen - 1) {
			partALen = bufLen - 1;
		}
		strncat(pcPhnA, pcPhnOrg, partALen);
		strncat(pcPhnB, pPartB, bufLen - 1);
	}
	return;
}


int zb_dic_search_word_exact(struct DIC_LIB_s * pDicEc, const char * pcWord)
{
	int searchRet = zb_dic_search_word(pDicEc, pcWord);
	if (searchRet < 0)
		return searchRet;

	int wordNoSt = 0;
	int8_t acMatchRateTbl[30];
	int wordNoBest = searchRet;
	char * pcWordN = pcWord;
	
	acMatchRateTbl[0] = 0;
	wordNoBest = zb_dic_find_contiue_match_words(pDicEc, pcWordN, searchRet, 
			&wordNoSt, acMatchRateTbl, sizeof(acMatchRateTbl),	false); 
	int8_t matchRate = acMatchRateTbl[wordNoBest - wordNoSt];
	if (matchRate >= 0x7f)
		return wordNoBest;
	return -20;
}

int zb_dic_get_phonetic(struct DIC_LIB_s * pDicEc, int wordNo, char * pcPhnBuf, int phnBufLen, 
	char * pcAimBuf, int aimBufLen)
{
	int iFuncRet = -1;
	if (!pDicEc || (wordNo < 0) || !pcPhnBuf || (phnBufLen < 2))
		return -1;
	if (!pcAimBuf || (aimBufLen < 100))
		return -3;

	pcAimBuf[0] = '\0';
	int ret = zb_dic_get_explain(pDicEc, wordNo, pcAimBuf, aimBufLen);
	if (ret <= 0) {
		ESP_LOGE(TAG, "Fail to get expl, ret %d", ret);
		return -3;
	}

	pcPhnBuf[0] = '\0';
	cJSON * cjsonRoot = NULL;
	cJSON * cjsonPhn = NULL;
	const char * pcPhn = "";
	do {
		cjsonRoot = cJSON_Parse(pcAimBuf);
		if (cJSON_HasObjectItem(cjsonRoot, "p")) {
			cjsonPhn = cJSON_GetObjectItem(cjsonRoot, "p");
		}
		if (cjsonPhn) {
			pcPhn = cjsonPhn->valuestring;
		}
	} while(0);

	if (pcPhn && pcPhn[0]) {
		#define PHN_BUF_LEN 70
		char acPhnA[PHN_BUF_LEN], acPhnB[PHN_BUF_LEN];
		acPhnA[0] = acPhnB[0] = '\0';
		cvtPhoneticToTwo(pcPhn, acPhnA, acPhnB, PHN_BUF_LEN);
		if (acPhnA[0]) {
			strlcpy(pcPhnBuf, acPhnA, phnBufLen);
			iFuncRet = 0;
		}
	}
	if (cjsonRoot) {
		cJSON_Delete(cjsonRoot);  cjsonRoot = NULL;
	}

	return iFuncRet;
}



