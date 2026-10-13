#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <stdio.h>

#define _ANALYZE_LSN_C_

#include "../utils/misc_utils.h"
#include <lvgl/src/misc/lv_assert.h>
#include "./cJSON.h"


#include "analyze_listening.h"


#define TAG "anlLsn"

#define		WORD_MAX_SIZE	300

#define		WORD_ADDR_TBL_LEN	5
#define 	WORD_ADDR		0  // word data 
#define		EXPLAIN_ADDR	1  // lessen data 
#define		SOUND_ADDR		2  // audio data
#define		IMAGE_ADDR		3  // image data
#define		END_ADDR		4


typedef struct {
	int32_t  m_addr;
	int32_t  m_length;
} lstn_addr_len_t;

typedef struct // (一个词条的)解释的数据
{
	char *pO;  // original text  (only refer)     原文
	char *pP;  // pinyin or phonetic (only refer) 拼音或音标 
	char *pT;  // translation text  (only refer)  翻译文本
	char *pZc; // 组词  (only refer)
} ExplainData, *P_ExplainData;

struct WordArea;


typedef struct LsnSentence {
	char *pOrg;    // original text  (only refer)      原文
	char *pTrans;  // translation text  (only refer)   翻译文本
	int32_t ptInTime;
} LsnSentence;


struct LSNSPK_unit      // unit 文件基本信息 
{
    uint16_t word_num;		// 单词数量 
    uint16_t sentence_n;	// 课文句子数量 
    uint16_t encParm;	// 加密因子 
    uint16_t hasTrans : 1;
    FILE * fpUnit;
    //----- private 
    char version[7];    //
    uint16_t check_sum;	// 校验码 
    uint32_t file_size; // 文件大小 

    uint32_t word_start_addr; // 单词开始地址 
    uint32_t text_start_addr; // 课文地址 
	uint32_t quanjie_start_addr;	// 外研版全解数据地址

    struct WordArea * pWdArea;
    //---- lessonArea 
    lstn_addr_len_t lesson_text_loc;  // 课文文本偏移 
    uint32_t  ptInTimeTblAddr;     // 各句音频时刻表的地址 
    uint32_t  lessonTimeTotal;
    lstn_addr_len_t lesson_aud_loc; 
    char * pLsnTextBuf;
    LsnSentence * pLsnSentenceTbl;
    //uint32_t  * pPtInTimeTbl;
	struct QuanJieData* pQJ;	// 全解数据
};

struct QuanJieData
{
	uint8_t item_sum;
	lstn_addr_len_t* pAddr_list;	// 单词、课文等地址列表 

	char** pChapter;				// 章节字符串指针， 首个字符表示类型
};

struct  WordArea      // (unit文件)单词区的基本信息 
{
    lstn_addr_len_t addr_list[WORD_ADDR_TBL_LEN];	// 单词、课文等地址列表 
    uint8_t  word_width;				// 单词文本宽度, deprecated

    char * pWord[WORD_MAX_SIZE]; // 各单词项指针, only refer here.
    ExplainData * explain[WORD_MAX_SIZE];
    char * pWordAreaBuf; // buffer for text of all words    单词
    char * pExplAreaBuf; // buffer for text of all explain  解释
};


//---------------------------------------------
static void _LSNSPK_dec_string(char * pStr, int len, uint8_t encParam)
{
	for (int idx = 0; idx < len; idx++) {
		pStr[idx] ^= encParam;
	}
}

/* 获取全解数据：
*  style = 0: 表示读取章节数据，否则，其他
*  num: 引用，返回读取的项目数
*  用完了数据，需要释放资源
 * @return >= 0 means success.
 */
char* LSNSPK_read_qj_data(const char* pPathUnit, uint8_t style, uint8_t* chapter_num, uint32_t* size, int* pErrNo)
{
	char acBuf[40];
	int iFuncRet = -1;
	FILE* fp = NULL;
	char* pBuf = NULL;
	do {
		fp = zb_fopen(pPathUnit, "rb");
		if (!fp) {
			iFuncRet = -3;  break;
		}
		zb_fread(acBuf, 16, 1, fp);
		if (memcmp(acBuf, "UNITZbTlb_01", 12) != 0) {
			iFuncRet = -5;  break;
		}
		zb_fseek(fp, 60, 0);

		uint32_t quanjie_start_addr;
		zb_fread(&quanjie_start_addr, 4, 1, fp); // 2024.08.22 rsh 添加
		//------ 读取全解数据
		if (quanjie_start_addr > 0) {
			zb_fseek(fp, quanjie_start_addr, SEEK_SET);
			zb_fread(chapter_num, 4, 1, fp);
			zb_fseek(fp, quanjie_start_addr + (style + 4) * 4, SEEK_SET);
			uint32_t start_addr, end_addr;
			zb_fread(&start_addr, 4, 1, fp);
			zb_fread(&end_addr, 4, 1, fp);
			*size = end_addr - start_addr;
			ESP_LOGI(TAG, "QJ_DATA:SIZE=%ld,%d", *size,style);
			zb_fseek(fp, quanjie_start_addr + start_addr, SEEK_SET);
			pBuf = (char*)heap_caps_malloc_dflt(*size + 1);
			zb_fread(pBuf, *size, 1, fp);
			*(pBuf + *size) = 0;
			uint32_t addr = (quanjie_start_addr + start_addr);
			uint32_t llen = (uint32_t)strlen(pBuf);
			ESP_LOGI(TAG, "QJ_DATA:start_addr=%ld, LEN=%ld", addr, llen);
		}

	} while (0);

	if (iFuncRet < 0) {
		if (fp) {
			zb_fclose(fp);  fp = NULL;
		}
	}
	if (pErrNo) {
		(*pErrNo) = iFuncRet;
	}
	return pBuf;
}

/* 初始化 unit 文件的基本信息 
 * @return >= 0 means success. 
 */
struct LSNSPK_unit * LSNSPK_unit_open(const char * pPathUnit,  int * pErrNo)
{ 
	char acBuf[40];
	int iFuncRet = -1;
	FILE * fp = NULL;
	struct LSNSPK_unit * pUnitInf = NULL;
	do {
		fp = zb_fopen(pPathUnit, "rb");
		if (!fp) {
			iFuncRet = -3;  break;
		}
		zb_fread(acBuf, 16, 1, fp);
		if (memcmp(acBuf, "UNITZbTlb_01", 12) != 0)	 {
			iFuncRet = -5;  break;
		}
		pUnitInf = (struct  LSNSPK_unit*)heap_caps_malloc_dflt(sizeof(struct  LSNSPK_unit));
		memset(pUnitInf, 0, sizeof(struct  LSNSPK_unit));

		zb_fseek(fp, 18, 0);

		zb_fread(pUnitInf->version, 6, 1, fp);
		zb_fread(&pUnitInf->encParm, 2, 1, fp);
		zb_fread(&pUnitInf->check_sum, 2, 1, fp);
		zb_fread(&pUnitInf->file_size, 4, 1, fp);

		zb_fread(acBuf, 16, 1, fp);  // reserve area 

		zb_fread(&pUnitInf->word_start_addr, 4, 1, fp);
		zb_fread(&pUnitInf->text_start_addr, 4, 1, fp);
		zb_fread(&pUnitInf->word_num, 2, 1, fp);
		zb_fread(&pUnitInf->sentence_n, 2, 1, fp);
		zb_fread(&pUnitInf->quanjie_start_addr, 4, 1, fp); // 2024.08.22 rsh 添加
		if (pUnitInf->word_num > 3000 || pUnitInf->sentence_n > 3000) {
			iFuncRet = -9; break;        
		}
		//if (pUnitInf->word_num <= 0 && pUnitInf->sentence_n <= 0) {
		//	iFuncRet = -11;
  //          //break;           
		//}
        if (pUnitInf->word_num) {
            pUnitInf->pWdArea = (struct  WordArea*)heap_caps_malloc_dflt(sizeof(struct  WordArea));
            memset(pUnitInf->pWdArea, 0, sizeof(struct  WordArea));
            printf("pUnitInf->pWdArea == %p\n", pUnitInf->pWdArea);
            //------ 初始化(unit的)单词区的基本信息 
            zb_fseek(fp, pUnitInf->word_start_addr, SEEK_SET);
            zb_fread(&pUnitInf->pWdArea->addr_list[WORD_ADDR], 4, 2, fp);
            zb_fread(&pUnitInf->pWdArea->addr_list[EXPLAIN_ADDR], 4, 2, fp);
            zb_fread(&pUnitInf->pWdArea->addr_list[SOUND_ADDR], 4, 2, fp);
            zb_fread(&pUnitInf->pWdArea->addr_list[IMAGE_ADDR], 4, 2, fp);
            zb_fread(&pUnitInf->pWdArea->word_width, 1, 1, fp);
        }
        //------ 初始化课文区的基本信息 
        zb_fseek(fp, pUnitInf->text_start_addr, SEEK_SET);
        zb_fread(&pUnitInf->lesson_text_loc, 4, 2, fp);
        zb_fread(&pUnitInf->ptInTimeTblAddr, 4, 1, fp);
        zb_fread(acBuf, 4, 1, fp);
        zb_fread(&pUnitInf->lesson_aud_loc, 4, 2, fp);

		//------ 读取全解数据
		/*if (pUnitInf->quanjie_start_addr > 0) {
			pUnitInf->pQJ = (struct  QuanJieData*)heap_caps_malloc_dflt(sizeof(struct  QuanJieData));

			zb_fseek(fp, pUnitInf->quanjie_start_addr, SEEK_SET);
			zb_fread(&pUnitInf->pQJ->item_sum, 4, 1, fp);
			zb_fread(acBuf, 12, 1, fp);  // reserve area 
			pUnitInf->pQJ->pAddr_list = (lstn_addr_len_t*)heap_caps_malloc_dflt(sizeof(lstn_addr_len_t) * (pUnitInf->pQJ->item_sum + 2));

			for (int i = 0; i < pUnitInf->pQJ->item_sum + 2; i++)
			{
				zb_fread(&pUnitInf->pQJ->pAddr_list[i].m_addr, 4, 1, fp);
			}
			for (int i = 0; i < pUnitInf->pQJ->item_sum + 1; i++)
			{
				pUnitInf->pQJ->pAddr_list[i].m_length = pUnitInf->pQJ->pAddr_list[i + 1].m_addr - pUnitInf->pQJ->pAddr_list[i].m_addr;
			}
		}*/


		pUnitInf->fpUnit = fp;
		iFuncRet = 0;
	} while (0);

	if (iFuncRet < 0) {
		if (fp) {
			zb_fclose(fp);  fp = NULL;
		}
	}
	if (pErrNo) {
		(*pErrNo) = iFuncRet;
	}
	return pUnitInf;
}


void LSNSPK_unit_close(struct LSNSPK_unit * pUnitInf)
{
	if (!pUnitInf) return;
	
	if (pUnitInf->fpUnit) {
		zb_fclose(pUnitInf->fpUnit);  pUnitInf->fpUnit = NULL;
	}
	if (pUnitInf->pLsnSentenceTbl) {
		heap_caps_free(pUnitInf->pLsnSentenceTbl);  pUnitInf->pLsnSentenceTbl = NULL;
	}

	for (int i = 0; i <pUnitInf->word_num; i ++) {
		if (pUnitInf->pWdArea->explain[i])
		{
			heap_caps_free(pUnitInf->pWdArea->explain[i]);
		}	
	}
	if (pUnitInf->pWdArea)
	{
		heap_caps_free(pUnitInf->pWdArea->pWordAreaBuf);
		heap_caps_free(pUnitInf->pWdArea->pExplAreaBuf);
		heap_caps_free(pUnitInf->pWdArea);
	}
	heap_caps_free(pUnitInf);
}


/**
 * 拆解解释文本里各子项 以及其内部各字段 
 */
static P_ExplainData * explain_split(char * str, uint16_t size, int16_t maxItemN, P_ExplainData ppEd[]) {
	int i = 0;
	P_ExplainData ped;
	char tag = 0;
	char * pFiledSt;  // 字段的起始指针 (only refer).
	int16_t itemCnt = 0;

	ped = (ExplainData*)heap_caps_malloc_dflt(sizeof(ExplainData));
	memset(ped, 0, sizeof(ExplainData));
	pFiledSt = str;
	for (; i < size; i ++)
	{
		unsigned char ch = (unsigned char)*str;
		if (ch < 0x1F) { // 0x1F 是解释之间的换行
			char tagNx = (*str);
			if (tag > 0) {  // if now is in one filed.
				switch (tag) {
				case 0x05:
					ped->pO = pFiledSt;
					break;
				case 0x06:
					ped->pP = pFiledSt;
					break;
				case 0x07:
					ped->pT = pFiledSt;
					break;
				case 0x08:
					ped->pZc = pFiledSt;
					break;
				default:
					break;
				}
			}
			if (tagNx >= 5 && tagNx <= 8) {
				(*str) = 0;  // fill stop-char to this field text. 
				pFiledSt = str + 1;
				tag = tagNx;  // save later tag (for next field). 
			}
			str++; 
			if (tagNx == 0x0a) {
				*(str - 1) = 0;  // fill stop-char to tail field of this row. 
				*ppEd++ = ped;
				itemCnt++;
				if (itemCnt >= maxItemN) break;
				ped = (ExplainData *)heap_caps_malloc_dflt(sizeof(ExplainData));
				memset(ped, 0, sizeof(ExplainData));
				tag = 0;
			}
		}
		else {
			if ('\x1f' == ch) { // restore \n in text
				(*str) = '\n';
			}
			str++;
		}
	}

	*ppEd++ = ped;
	return ppEd;
}

/** 获取单词语音(在文件里)的位置
 * 第一个参数pLsnSpk：结构的指针包含有关音频文件或数据库的信息
 * 第二个参数no：要查找的词汇的编号或索引
 * 第三个参数pOffset：接收找到的音频数据在文件中的偏移位置
 * @return  length of audio, <= 0 means fail
 * @返回音频长度，<= 0表示失败
 */
int32_t LSNSPK_read_word_aud_location(struct LSNSPK_unit * pLsnSpk, int16_t no, int32_t * pOffset) {

	ESP_LOGE(TAG, "已进入 获取单词语音(在文件里)的位置：LSNSPK_read_word_aud_location");
	FILE * fp = NULL;
	int iFuncRet = -1;

	if (!pLsnSpk || no < 0) return -1;
	int32_t sound_addr = pLsnSpk->word_start_addr + pLsnSpk->pWdArea->addr_list[SOUND_ADDR].m_addr;
	fp = pLsnSpk->fpUnit;
	do {
		uint32_t aAudAddrTwo[2] = { 0 };
		uint32_t noAddr = sound_addr + no * 4;
		zb_fseek(fp, noAddr, 0);
		zb_fread(aAudAddrTwo, 4, 2, fp);
		uint32_t audLen = aAudAddrTwo[1] - aAudAddrTwo[0];
		if (pOffset) {
			(*pOffset) = sound_addr + aAudAddrTwo[0];
		}
		iFuncRet = audLen;
	} while (0);

	return iFuncRet;
}

/** 读取一个单词的图片信息
 * @param pSize 图片大小, 其 x、y 成员分别表示图片的宽、高;
 * @return  >0 means success, <= 0 means fail @return >0表示成功，<= 0表示失败
 * 第一个参数pLsnSpk：结构体文件基本信息
 * 第二个参数no：要读取的图像的编号或索引
 * 第三个参数ppImgDsc：指向 lv_img_dsc_t 指针的指针，返回读取的图像描述符
 * 第四个参数pSize：返回图像的大小（包含宽度和高度）
 */
int32_t LSNSPK_read_word_img_info(struct LSNSPK_unit * pLsnSpk, int16_t no, lv_img_dsc_t ** ppImgDsc, lv_point_t * pSize) {
#define MY_IMG_HEADER_LEN  2
#define LV_IMG_DSC_LEN 12

	FILE * fp = NULL;
	int iFuncRet = -1;

	if (ppImgDsc) {
		(*ppImgDsc) = NULL;
	}
	if (!pLsnSpk || no < 0) return -1;
	if (!pLsnSpk->pWdArea->addr_list[IMAGE_ADDR].m_addr) return -3;

	int32_t img_base_addr = pLsnSpk->word_start_addr + pLsnSpk->pWdArea->addr_list[IMAGE_ADDR].m_addr;
	fp = pLsnSpk->fpUnit;
	do {
		if (pSize) {
			uint32_t sizeItemOffset = img_base_addr + MY_IMG_HEADER_LEN + 4 * no;
			pSize->x = pSize->y = 0;
			zb_fseek(fp, sizeItemOffset, 0);
			zb_fread(pSize, 1, 4, fp);
		}
		int imgSizeAreaLen = 4 * pLsnSpk->word_num;
		uint32_t aAddrTwo[2] = { 0 };
		uint32_t noAddr = img_base_addr + MY_IMG_HEADER_LEN + imgSizeAreaLen + no * 4;
		zb_fseek(fp, noAddr, 0);
		zb_fread(aAddrTwo, 4, 2, fp);
		int32_t imgLen = aAddrTwo[1] - aAddrTwo[0];
		if (ppImgDsc) {
			int imgOffset = img_base_addr + aAddrTwo[0];
			ESP_LOGI(TAG, "imgAddr 0x%x", imgOffset);
			lv_img_dsc_t * pImg = (lv_img_dsc_t *)heap_caps_malloc_dflt(LV_IMG_DSC_LEN + imgLen);
			zb_fseek(pLsnSpk->fpUnit, imgOffset, 0);
			zb_fread(pImg, 1, 8, pLsnSpk->fpUnit);
			zb_fread((char *)pImg + LV_IMG_DSC_LEN, 1, imgLen, pLsnSpk->fpUnit);
			((int *)pImg)[2] = (int)((char *)pImg + LV_IMG_DSC_LEN);  // non-std, set ptr of img.
			(*ppImgDsc) = pImg;
		}
		iFuncRet = imgLen;
	} while (0);
    ESP_LOGI(TAG, "每一次调用LSNSPK_read_word_img_info返回的图像大小iFuncRet：%d", iFuncRet);
	return iFuncRet;
}


int LSNSPK_read_word_aud(struct LSNSPK_unit * pLsnSpk, int16_t no, void * pBuf, int bufLen) {
	FILE * fp = NULL;
	int iFuncRet = -1;

	if (!pLsnSpk) return -1;
	int32_t sound_addr = pLsnSpk->word_start_addr + pLsnSpk->pWdArea->addr_list[SOUND_ADDR].m_addr;
	fp = pLsnSpk->fpUnit;
	do {
		uint32_t aAudAddrTwo[2] = { 0 };
		uint32_t noAddr = sound_addr + no * 4;
		zb_fseek(fp, noAddr, 0);
		zb_fread(aAudAddrTwo, 4, 2, fp);
		int32_t audLen = aAudAddrTwo[1] - aAudAddrTwo[0];

		if (!pBuf) {
			return audLen;
		}
		if (bufLen < audLen) {
			iFuncRet = -5;
			break;
		}
		zb_fseek(fp, sound_addr + aAudAddrTwo[0], 0);
		zb_fread(pBuf, 1, audLen, fp);
		iFuncRet = audLen;
	} while (0);

	return iFuncRet;
}

/*
 * @param flag - 1 word, 2 expl, 3 both;
 */
static int _read_word_text_inf(struct LSNSPK_unit * pUnitInf, int16_t flag) {

	if (!pUnitInf) return -1;
	uint32_t base_addr = pUnitInf->word_start_addr;
	uint32_t word_addr = base_addr + pUnitInf->pWdArea->addr_list[WORD_ADDR].m_addr;

	if (flag & 1) {
		// 1、读取单词列表
		if (pUnitInf->pWdArea->pWordAreaBuf) {
			free(pUnitInf->pWdArea->pWordAreaBuf);   pUnitInf->pWdArea->pWordAreaBuf = NULL;
		}
		zb_fseek(pUnitInf->fpUnit, word_addr, SEEK_SET);
		int32_t wdAreaBufLen = pUnitInf->pWdArea->addr_list[WORD_ADDR].m_length;
		pUnitInf->pWdArea->pWordAreaBuf = (char *)heap_caps_malloc_dflt(wdAreaBufLen + 1);
		memset(pUnitInf->pWdArea->pWordAreaBuf, 0, wdAreaBufLen + 1);
		zb_fread(pUnitInf->pWdArea->pWordAreaBuf, 1, wdAreaBufLen, pUnitInf->fpUnit);  // read wordArea texts 
		if (pUnitInf->encParm > 0) {
			for (int k = 0; k < wdAreaBufLen; k++) {
				pUnitInf->pWdArea->pWordAreaBuf[k] ^= pUnitInf->encParm;
			}
		}
		char * pWord = pUnitInf->pWdArea->pWordAreaBuf;
		for (int i = 0; i < pUnitInf->word_num; i++) {
			char * pWordE = strchr(pWord, 0x0a);
			if (!pWordE && (i == pUnitInf->word_num - 1)) {
				pWordE = strchr(pWord, '\0');
			}
			LV_ASSERT(pWordE <= pUnitInf->pWdArea->pWordAreaBuf + wdAreaBufLen);
			(*pWordE) = '\0';
			pUnitInf->pWdArea->pWord[i] = pWord;
			pWord = pWordE + 1;
		}
	}

	// 2、读取 单词解释内容
	if (flag & 2) {
		if (pUnitInf->pWdArea->pExplAreaBuf) {
			free(pUnitInf->pWdArea->pExplAreaBuf);   pUnitInf->pWdArea->pExplAreaBuf = NULL;
		}
		uint32_t explain_addr = base_addr + pUnitInf->pWdArea->addr_list[EXPLAIN_ADDR].m_addr;
		uint32_t explain_size = pUnitInf->pWdArea->addr_list[EXPLAIN_ADDR].m_length;
		zb_fseek(pUnitInf->fpUnit, explain_addr, SEEK_SET);

		char *buf = (char*)heap_caps_malloc_dflt(explain_size+1);
		zb_fread(buf, explain_size, 1, pUnitInf->fpUnit);
		pUnitInf->pWdArea->pExplAreaBuf = buf;
		for (uint32_t k = 0; k < explain_size; k++) {
			buf[k] ^= pUnitInf->encParm;
		}
		buf[explain_size] = '\0';
		explain_split(buf, explain_size, M_ARRAY_LEN(pUnitInf->pWdArea->explain), &pUnitInf->pWdArea->explain[0]);
	}
	return 0;
}

/**
 * 拆解课文文本里各子项 以及其内部各字段 
 * return : >= 0 means success, bit0 1 means has translation field.
 */
static int _lesson_split(char * str, uint16_t size, int16_t maxItemN, LsnSentence * pEd) {
	int i = 0;
	char tag = 0;
	char * pFiledSt;  // 字段的起始指针 (only refer).
	int16_t itemCnt = 0;
	bool hasTrans = false;

	LsnSentence stLsnSentence = { 0 };
	pFiledSt = str;
	for (; i < size; i ++)
	{
		unsigned char ch = (unsigned char)*str;
		if (ch < 0x1F) { // 0x1F 是解释之间的换行
			char tagNx = (*str);
			if (tag > 0) {  // if now is in one filed.
				switch (tag) {
				case 0x05:
					stLsnSentence.pOrg = pFiledSt;
					break;
				case 0x07:
					stLsnSentence.pTrans = pFiledSt;
					hasTrans = true;
					break;
				default:
					break;
				}
			}
			if (tagNx >= 5 && tagNx <= 7) {
				(*str) = 0;  // fill stop-char to this field text. 
				pFiledSt = str + 1;
				tag = tagNx;  // save later tag (for next field). 
			}
			str++; 
			if (tagNx == 0x0a) {
				*(str - 1) = 0;  // fill stop-char to tail field of this row. 
				*pEd++ = stLsnSentence;
				itemCnt++;
				if (itemCnt >= maxItemN) break;
				memset(&stLsnSentence, 0, sizeof(LsnSentence));
				tag = 0;
			}
		}
		else {
			if ('\x1f' == ch) { // restore \n in text
				(*str) = '\n';
			}
			str++;
		}
	}

	*pEd++ = stLsnSentence;
	return (hasTrans ? 1 : 0);
}

static int _read_lesson_text_inf(struct LSNSPK_unit * pUnitInf) {

	if (!pUnitInf) return -1;
	uint32_t base_addr = pUnitInf->text_start_addr;

	// 读取课文内容
	{
		if (!pUnitInf->pLsnTextBuf) {
			free(pUnitInf->pLsnTextBuf);   pUnitInf->pLsnTextBuf = NULL;
		}
		if (!pUnitInf->pLsnSentenceTbl) {
			pUnitInf->pLsnSentenceTbl = (struct LsnSentence *)heap_caps_malloc_dflt(sizeof(struct LsnSentence) * (pUnitInf->sentence_n + 1));
			memset(pUnitInf->pLsnSentenceTbl, 0, sizeof(struct LsnSentence) * (pUnitInf->sentence_n + 1));
		}
		uint32_t lesson_txt_addr = base_addr + pUnitInf->lesson_text_loc.m_addr;
		int textAreaLen = pUnitInf->lesson_text_loc.m_length;
		zb_fseek(pUnitInf->fpUnit, lesson_txt_addr, SEEK_SET);

		char * buf = (char*)heap_caps_malloc_dflt(textAreaLen + 1);
		zb_fread(buf, textAreaLen, 1, pUnitInf->fpUnit);
		pUnitInf->pLsnTextBuf = buf;
		if (pUnitInf->encParm) {
			for (int32_t k = 0; k < textAreaLen; k++) {
				buf[k] ^= pUnitInf->encParm;
			}
		}
		buf[textAreaLen] = '\0';
		int ret = _lesson_split(buf, textAreaLen, pUnitInf->sentence_n, pUnitInf->pLsnSentenceTbl); // 
		if (ret & 1) {
			pUnitInf->hasTrans = 1;
		}
		ESP_LOGI(TAG, "unit has trans %hd", (short)pUnitInf->hasTrans);
		//---- read ptInTime table 
		int32_t * pPtInTimeTbl = (int32_t *)heap_caps_malloc_dflt(4 * (pUnitInf->sentence_n + 1));
		memset(pPtInTimeTbl, 0, 4 * (pUnitInf->sentence_n + 1));
		zb_fseek(pUnitInf->fpUnit, base_addr + pUnitInf->ptInTimeTblAddr, SEEK_SET);
		zb_fread(pPtInTimeTbl, 4, pUnitInf->sentence_n + 1, pUnitInf->fpUnit);
		pUnitInf->lessonTimeTotal = pPtInTimeTbl[pUnitInf->sentence_n];
		for (int16_t idx = 0; idx < pUnitInf->sentence_n + 1; idx++) {
			pUnitInf->pLsnSentenceTbl[idx].ptInTime = pPtInTimeTbl[idx];
		}
		free(pPtInTimeTbl);   pPtInTimeTbl = NULL;
	}
	return 0;
}

//读取单词信息
//根据输入参数返回相应的const char指针
const char * LSNSPK_get_word_field(struct LSNSPK_unit * pUnitInf, int16_t idx, int16_t col) { 
	if (!pUnitInf || idx < 0 || col < 0) return NULL;

	int16_t wdPatchFlag = 0;
	if (!pUnitInf->pWdArea->pExplAreaBuf) {
		wdPatchFlag = 2;
	}
	if (wdPatchFlag) { // if word inf not be read, read it first. 
		_read_word_text_inf(pUnitInf, wdPatchFlag);
	}
	if (idx >= pUnitInf->word_num) {
		return NULL;
	}

	const ExplainData * pstExpl = pUnitInf->pWdArea->explain[idx];
	if (!pstExpl) {
		return NULL;
	}
	switch (col) {
		case LSNSPK_COL_ORIGINAL:
			return pstExpl->pO;
		case LSNSPK_COL_PHONETIC:
			return pstExpl->pP;
		case LSNSPK_COL_TRANSLATION:
			return pstExpl->pT;
        case LSNSPK_COL_PHRASE:     //组词
            return pstExpl->pZc;
        default:
			break;
	}
	return NULL;
}

/** 读取一条句子(原文或译文)
 */
const char * LSNSPK_get_lesson_sentence(struct LSNSPK_unit * pUnitInf, int16_t idx, int16_t col) { 
	if (!pUnitInf || idx < 0 || col < 0) return NULL;

	if (!pUnitInf->pLsnSentenceTbl) { // if leeson inf not be read, read it first. 
		_read_lesson_text_inf(pUnitInf);
	}
	if (idx >= pUnitInf->sentence_n) {
		return NULL;
	}

	const struct LsnSentence * pLsnSentence = (const struct LsnSentence*)(pUnitInf->pLsnSentenceTbl + idx);
	if (!pLsnSentence) {
		return NULL;
	}
	switch (col) {
		case LSNSPK_COL_ORIGINAL:
			return pLsnSentence->pOrg;
		case LSNSPK_COL_TRANSLATION:
			return pLsnSentence->pTrans;
		default:
			break;
	}
	return NULL;
}

/** 获取课文语音整体数据的长度、位置
 */
int LSNSPK_get_lesson_aud_location(struct LSNSPK_unit * pLsnSpk, int32_t * pOffset) {
	if (pOffset) {
		(*pOffset) = pLsnSpk->text_start_addr + pLsnSpk->lesson_aud_loc.m_addr;
	}
	return pLsnSpk->lesson_aud_loc.m_length;
}

/** 获取课文一个句子语音位置
 * @return 该句的语音(在整个 unit 语音里的)位置, 单位 ms
 */
int32_t LSNSPK_get_lesson_sent_ptInTime(struct LSNSPK_unit * pUnitInf, int16_t idx) {
	int iFuncRet = -1;

	do {
		if (!pUnitInf || idx < 0)
			break;

		if (idx > pUnitInf->sentence_n) {
			iFuncRet = -3;  break;
		}
		if (!pUnitInf->pLsnSentenceTbl) {
			_read_lesson_text_inf(pUnitInf);
			if (!pUnitInf->pLsnSentenceTbl) {
				iFuncRet = -5;	break;
			}
		}
		iFuncRet = 0;
	} while (0);

	if (iFuncRet < 0)
		return iFuncRet;
	return pUnitInf->pLsnSentenceTbl[idx].ptInTime;
}

//----------------------------------------------
struct LSNSPK_UnitItem { 
    char * pcUnitTitle;  // 标题, only refer
    char * pcUnitFName;  // 文件名, only refer
};

struct LSNSPK_book {       // unit 文件基本信息 
    uint16_t lesson_n;	// 课文数量 
    uint16_t subject;  // 0 - english, 1 - yuwen. 
    char unitRltvPath[60];  // unit文件相对路径 
    //------  private members
    uint16_t encPara;
    uint16_t resType;
    uint16_t coverImgType;

    FILE * fpBook;
    struct LSNSPK_UnitItem * pUnitItems;
    char * pUnitsTxtBuff;  // buffer for unitItems txt
};


/**
 * 拆解书本目录文本里各子项 
 */
static int book_catalog_split(char * str, uint16_t textLen, int16_t maxItemN, struct LSNSPK_UnitItem * pLessons) 
{
	char * strSt = str;  // 字段的起始指针 (only refer).
	int16_t itemCnt = 0;

	for (; itemCnt < maxItemN; itemCnt++) {

		if (str >= strSt + textLen) break;
		char * itemStart = str;
		char * pEnd = strchr(str, 0x0a);
		if (!pEnd) {
			pEnd = strchr(str, '\0');
		}
		if (pEnd == str) break;  // reach end of catalogue.

		(*pEnd) = '\0';
		char * pChapterTailMark = strchr(str, 0x05);
		if (pChapterTailMark) {
			str = pChapterTailMark + 1;  // skip chapter's name.
		}
		char *pMid = strchr(str, 0x06);
		if (pMid) {
			(*pMid) = '\0';
		}
		pLessons[itemCnt].pcUnitTitle = str;
		pLessons[itemCnt].pcUnitFName = (pMid ? pMid + 1 : "");
		str = pEnd + 1;
	}
	return itemCnt;
}


/* 初始化 menu 文件的基本信息 
 * @return >= 0 means success. 
 */
struct LSNSPK_book * LSNSPK_book_open(const char * pPathUnit,  int * pErrNo) { 
#define STR_BOOK_FILE_IDENTIF "MENUZbTlb_01"
	char acBuf[40];
	int iFuncRet = -1;
	FILE * fp = NULL;
	struct LSNSPK_book * pBookInf = NULL;
	do {
		fp = zb_fopen(pPathUnit, "rb");
		if (!fp) {
			iFuncRet = -3;  break;
		}
		zb_fread(acBuf, 16, 1, fp);
		if (memcmp(acBuf, STR_BOOK_FILE_IDENTIF, sizeof(STR_BOOK_FILE_IDENTIF) - 1) != 0) {
			iFuncRet = -5;  break;
		}
		pBookInf = (struct  LSNSPK_book*)heap_caps_malloc_dflt(sizeof(struct  LSNSPK_book));
		memset(pBookInf, 0, sizeof(struct  LSNSPK_book));

		zb_fseek(fp, 0x10, 0);
		zb_fread(&pBookInf->lesson_n, 2, 1, fp);
		if (pBookInf->lesson_n > 800) {
			iFuncRet = -9;  break;
		}
		zb_fseek(fp, 0x18, 0);
		zb_fread(&pBookInf->encPara, 2, 1, fp);
		zb_fseek(fp, 0x20, 0);
		zb_fread(&pBookInf->resType, 2, 1, fp);
		zb_fread(&pBookInf->coverImgType, 2, 1, fp);

		zb_fseek(fp, 0x32, 0);
		zb_fread(&pBookInf->subject, 2, 1, fp);//英语 语文

		zb_fseek(fp, 0x70, 0);
		zb_fread(&pBookInf->unitRltvPath, 1, 48, fp);
		zb_fread(acBuf, 1, 8, fp);

		int32_t aiUnitDatAddrTwo[2] = { 0 };
		zb_fread(aiUnitDatAddrTwo, 4, 2, fp);
		int32_t unitTextAreaLen = aiUnitDatAddrTwo[1] - aiUnitDatAddrTwo[0];
		if (unitTextAreaLen <= 0 || unitTextAreaLen > 1024 * 100) { 
			iFuncRet = -11;  break;
		}
		
		pBookInf->pUnitsTxtBuff = (char *)heap_caps_malloc_dflt(unitTextAreaLen + 1);
		memset(pBookInf->pUnitsTxtBuff, 0, unitTextAreaLen + 1);
		zb_fseek(fp, aiUnitDatAddrTwo[0], 0);
		zb_fread(pBookInf->pUnitsTxtBuff, 1, unitTextAreaLen, fp);
		_LSNSPK_dec_string(pBookInf->pUnitsTxtBuff, unitTextAreaLen, (uint8_t)pBookInf->encPara);

		pBookInf->pUnitItems = (struct LSNSPK_UnitItem *)heap_caps_malloc_dflt(
				sizeof(struct LSNSPK_UnitItem) * pBookInf->lesson_n);
		memset(pBookInf->pUnitItems, 0, sizeof(struct LSNSPK_UnitItem) * pBookInf->lesson_n);
        //处理文本缓冲区并填充 pUnitItems 数组  
		book_catalog_split(pBookInf->pUnitsTxtBuff, unitTextAreaLen, pBookInf->lesson_n, pBookInf->pUnitItems);

		pBookInf->fpBook = fp;
		iFuncRet = 0;
	} while (0);

	if (iFuncRet < 0) {
		if (fp) {
			zb_fclose(fp);  fp = NULL;
		}
		if (pBookInf) {
			heap_caps_free(pBookInf);  pBookInf = NULL;
		}
	}
	if (pErrNo) {
		(*pErrNo) = iFuncRet;
	}
	return pBookInf;
}

void LSNSPK_book_close(struct LSNSPK_book * pBookInf)
{
	if (!pBookInf) return;

	if (pBookInf->pUnitsTxtBuff) {
		heap_caps_free(pBookInf->pUnitsTxtBuff);  pBookInf->pUnitsTxtBuff = NULL;
	}
	if (pBookInf->pUnitItems) {
		heap_caps_free(pBookInf->pUnitItems);  pBookInf->pUnitItems = NULL;
	}

    if (pBookInf->fpBook != NULL) {
        zb_fclose(pBookInf->fpBook);
        pBookInf->fpBook = NULL;
    }
	heap_caps_free(pBookInf);

	return;
}

// 获取书本某一课的某一个字段(标题或者文件名)
const char * LSNSPK_book_get_item_filed(struct LSNSPK_book * pBookInf, int16_t idx, int16_t fieldNo)
{
	if (!pBookInf) return NULL;
	do {
		if (idx < 0 || idx >= pBookInf->lesson_n || fieldNo < 0 || fieldNo >= 2) {
			break;
		}
		if (0 == fieldNo) {
			return pBookInf->pUnitItems[idx].pcUnitTitle;//获取标题名称
		} else {
			return pBookInf->pUnitItems[idx].pcUnitFName;//获取文件名
		}
	} while(0);

	return NULL;
}

lv_img_dsc_t * LSNSPK_get_cover_data(const char* pPathUnit, int* pErrNo) {
	char acBuf[40];
	int iFuncRet = -1;
	FILE* fp = NULL;
	
	lv_img_dsc_t* pImgDsc = NULL;

	do {
		fp = zb_fopen(pPathUnit, "rb");
		if (!fp) {
			iFuncRet = -3;  break;
		}
		zb_fread(acBuf, 16, 1, fp);
		if (memcmp(acBuf, STR_BOOK_FILE_IDENTIF, sizeof(STR_BOOK_FILE_IDENTIF) - 1) != 0) {
			iFuncRet = -5;  break;
		}

		zb_fseek(fp, 0xA0, 0);
		int32_t coverDataAddrTwo[2] = { 0 };
		zb_fread(coverDataAddrTwo, 4, 2, fp);
		int32_t coverDataLen = coverDataAddrTwo[1] - coverDataAddrTwo[0];
		if (coverDataAddrTwo[0] == 0 || coverDataLen <= 0) {
			iFuncRet = -11;  break;
		}

		lv_img_dsc_t stImgDsc = { 0 };
		zb_fseek(fp, coverDataAddrTwo[0]+4, 0);
		zb_fread(&stImgDsc, 1, 8, fp);
		LV_ASSERT(stImgDsc.data_size >= 1 && stImgDsc.data_size < 1024 * 500);
		void* imgBuf = heap_caps_malloc_dflt(stImgDsc.data_size); // 
		if (!imgBuf) {
			iFuncRet = -7;  break;
		}
		zb_fread(imgBuf, 1, stImgDsc.data_size, fp);
		stImgDsc.data = imgBuf;

		pImgDsc = heap_caps_malloc_dflt(sizeof(lv_img_dsc_t));
		(*pImgDsc) = stImgDsc;
		iFuncRet = 0;
	} while (0);

	if (iFuncRet < 0) {
		ESP_LOGW(TAG, "getCover fail %d", iFuncRet);
	}
	if (fp != NULL) {
		zb_fclose(fp);  fp = NULL;
	}
	return pImgDsc;
}

//--------------------

void LSNSPK_menu_close(struct LSNSPK_MENU * pMenu)
{
    if (!pMenu) return;
    int16_t num = pMenu->m_itemTotal;

    for (int16_t i = 0; i < num; i++) {
        heap_caps_free(pMenu->astAlbumTbl[i].m_ptitle);
    }
    if (pMenu->m_fp != NULL) {
        zb_fclose(pMenu->m_fp);
        pMenu->m_fp = NULL;
    }
    if (pMenu) {
        heap_caps_free(pMenu);  pMenu = NULL;
    }
    return;
}


#define MNU_JSON_FILE_LEN_MAX  4096 

struct LSNSPK_MENU * LSNSPK_menu_open(const char* pcAlbumPath)
{
    FILE* fp = NULL;
    cJSON* json = NULL;
    struct LSNSPK_MENU * pMenu = NULL;
    struct DIC_ALBUM_ITEM_s* p_albums = NULL; // only refer
    int funcErrNo = -1;  // >= 0 means success 
    int16_t iTotal = 0;

    char * acBuf = (char *)heap_caps_malloc_dflt(MNU_JSON_FILE_LEN_MAX);
    do {
        fp = zb_fopen(pcAlbumPath, "rb");
        if (!fp) {
            ESP_LOGE(TAG, "Fail to openF %s", pcAlbumPath);
            funcErrNo = -3;
            break;
        }

        int len = zb_fread(acBuf, 1, MNU_JSON_FILE_LEN_MAX - 1, fp);
		if (len >= 0) {
			acBuf[len] = '\0';
		}
        cJSON* item = NULL;

        json = cJSON_Parse(acBuf);
        if (json == NULL) {
            iTotal = cJSON_GetErrorPtr();
            cJSON_Delete(json);
            p_albums = NULL;
            funcErrNo = -5;
            break;
        }
        // 确保解析得到的是数组
        if (cJSON_IsArray(json)) {
            cJSON* item = NULL;
            int16_t sum = cJSON_GetArraySize(json);
            pMenu = (struct LSNSPK_MENU*)heap_caps_malloc_dflt(sizeof(struct LSNSPK_MENU) + sizeof(struct DIC_ALBUM_ITEM_s) * sum);
            p_albums = pMenu->astAlbumTbl;
            // 遍历JSON数组中的每个元素
            for (int i = 0; i < sum; i++) {
                item = cJSON_GetArrayItem(json, i);
                cJSON* title_item = cJSON_GetObjectItemCaseSensitive(item, "title");
                if (cJSON_IsString(title_item) && (title_item->valuestring != NULL)) {
                    int titleLen = strlen(title_item->valuestring);
                    p_albums[iTotal].m_ptitle = (char*)heap_caps_malloc_dflt(titleLen + 1);
                    strcpy(p_albums[iTotal].m_ptitle, title_item->valuestring);
                    //printf("Name: %s\n", title_item->valuestring);
                }
                cJSON* num_item = cJSON_GetObjectItemCaseSensitive(item, "num");
                p_albums[iTotal].m_total = cJSON_GetNumberValue(num_item);
                cJSON* cover_item = cJSON_GetObjectItemCaseSensitive(item, "cover");
                if (strlen(cover_item->valuestring) > 0)
                    p_albums[iTotal].m_imgId = atoi(cover_item->valuestring);
                else
                    p_albums[iTotal].m_imgId = -1;
                iTotal += 1;
            }
        }

        // 清理分配的cJSON结构体
        cJSON_Delete(json);
        funcErrNo = 0;

    } while (0);

    if (acBuf) {
        heap_caps_free(acBuf);   acBuf = NULL;
    }
    if (funcErrNo < 0) {
        ESP_LOGE(TAG, "mnuOpen fail: %d, %s", funcErrNo, pcAlbumPath);
        if (fp) {
            zb_fclose(fp);  fp = NULL;
        }
        if (pMenu) {
            heap_caps_free(pMenu);  pMenu = NULL;
        }
    }
	else if (pMenu) {
		pMenu->m_itemTotal = iTotal;
		pMenu->m_fp = fp;
	}
    return pMenu;
}


int readAudFromUnitFile(const char * pcPathIn, int offset, int len, uint8_t encParm, const char * pcPathDst)
{ 
	int iFuncRet = -1;
	FILE * f_in = NULL, * f_dst = NULL;
	char * pcBuf = NULL;
	do {
		f_in = zb_fopen(pcPathIn, "rb");
		if (!f_in) {
			iFuncRet = -3;  break;
		}
		f_dst = zb_fopen(pcPathDst, "wb");
		if (!f_dst) {
			iFuncRet = -5;  break;
		}
		pcBuf = heap_caps_malloc_dflt(len);
		if (!pcBuf) {
			iFuncRet = -7;	break;
		}
		fseek(f_in, offset, 0);
		fread(pcBuf, 1, len, f_in);
		for (int sub = 0; sub < len; sub++) {
			pcBuf[sub] ^= encParm;
		}
		fwrite(pcBuf, 1, len, f_dst);
		iFuncRet = 0;
	} while (0);

	if (f_dst) {
		zb_fclose(f_dst);  f_dst = NULL;
	}
	if (f_in) {
		zb_fclose(f_in);  f_in = NULL;
	}
	if (pcBuf) {
		heap_caps_free(pcBuf);  pcBuf = NULL;
	}
	if (iFuncRet < 0) {
		ESP_LOGE(TAG, "fail to readAud %d", iFuncRet);
	}
	return iFuncRet;
}


