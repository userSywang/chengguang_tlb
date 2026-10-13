#ifndef _DICT_PARSE_H_
#define _DICT_PARSE_H_

#include <stdbool.h>

#include <lvgl/src/misc/lv_types.h>

typedef struct {
    int32_t  m_word_n;
    int32_t  m_expl_n;
} DIC_LIB_t;


struct DIC_LIB_s;


struct DIC_LIB_s * zb_dic_open(const char * pcDicPath, int * piErrNo);

void zb_dic_close(struct DIC_LIB_s * pstDic);

int zb_dic_read_one_head(struct DIC_LIB_s * pstDic, int wordNo, char * pcBuf, int bufLen);


/**
 * @return < 0 means fail.
 */
int zb_dic_search_word(struct DIC_LIB_s * pDic, const char * pcWord);


/**
 * @return < 0 means fail.
 */
int zb_dic_search_word_exact(struct DIC_LIB_s * pDicEc, const char * pcWord);


/** get phonetic 
 */
int zb_dic_get_phonetic(struct DIC_LIB_s * pDicEc, int wordNo, char * pcPhnBuf, int phnBufLen, 
	char * pcAimBuf, int aimBufLen);



/** 
 * @return >=0 0 means success; 其值为匹配度最高的那一词条的序号;
 * Note: 返回的 pMatchRateTbl[] 以 0 值作为结尾标记;
 *   本函数执行后, caller  调用 st rlen(pMatchRateTbl) 即可得形似词个数. 
 */
int zb_dic_find_contiue_match_words(struct DIC_LIB_s * pDic, const char * pcAimWord, 
		int searchStartNo, int * piWordNoSt, int8_t * pMatchRateTbl, int16_t tblLen, bool isSentenceHead); 


/**
 * read explain.
 * when pcBuf is NULL, only measure expl text length.
 * @return the length of explain.
 */
int zb_dic_get_explain(struct DIC_LIB_s * pDic, int wordNo, char * pcBuf, int bufLen);


/** 
 * split origin phone into two part (am. and br. mode)
 */
void cvtPhoneticToTwo(const char * pcPhnOrg, char * pcPhnA, char * pcPhnB, int bufLen);


#endif

