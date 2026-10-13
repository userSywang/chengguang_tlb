#ifndef _BOOK_SEL_H_
#define _BOOK_SEL_H_

#include "stdint.h"


//————————



#define book_sel_module_init()  book_sel_ap_init()


extern int book_sel_ap_init(void);


/** 销毁教材选择界面 
 * note: 用于供 AP 的 EVENT_FREE_AP_RES 消息处理里调用 
 */
extern void book_sel_module_del(void);



//----------------------------------------------

#define BKSEL_GRADE_TOTAL  12

#define BKSEL_ITEM_N_MAX   50  // (指定科目年级的 出版社 数量的最大值)


#define  PATH_BKSEL_CACHE "/sdcard/_data/bookselCache.bin"


struct grade_publish_inf {
	int16_t  m_subject : 8; // 1 - yuwen, 2 -english
	int16_t  m_grade : 8;   // 1~6, 7~9, 10~12
	int16_t  m_publishNum;
	char ** ppcPublishs;   // (某一科目、年级的)各出版社字串指针的数组的首地址 
};

struct BOK_VER_CACHE_FHEAD {
	char m_acIdentifier[8];
	uint32_t m_timeStamp;
	char    acReserveA[4];
	int32_t m_chnJsonOffset;
	int32_t m_chnJsonLen;  // 中文publish json 字串长度 
	int32_t m_engJsonOffset;
	int32_t m_engJsonLen;  // 英文publish json 字串长度 
	char    acReserveB[32];

	uint32_t m_timeStampTokenMinute;
	char    m_acToken[32+1];
	char    acReserveC[39];
};


struct BOK_VER_CACHE_WRT_STAT {
	void * m_pBuf;
	int m_bufSize;
	int16_t m_wrtStep; //
	int m_dataOffset;
	struct BOK_VER_CACHE_FHEAD stCacheFHead;
};


#endif

