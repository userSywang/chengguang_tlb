#ifndef _ANALYZE_LISTENING_H_
#define _ANALYZE_LISTENING_H_

#include <lvgl/src/misc/lv_types.h>
#include <lvgl/src/draw/lv_img_buf.h>


#include <stdio.h>



#define LSNSPK_COL_ORIGINAL     0
#define LSNSPK_COL_PHONETIC     1
#define LSNSPK_COL_TRANSLATION  2
#define LSNSPK_COL_PHRASE       3



struct LSNSPK_unit;

typedef struct {
    uint16_t word_num;		// 单词数量 
    uint16_t sentence_n;	// 课文句子数量 
    uint16_t encParm;		// 加密因子 
    uint16_t hasTrans : 1;  // 文章是否包含译文 
    FILE * fpUnit;
} LSNSPK_unit_t;


/** 打开一个单元(课)
 */
struct LSNSPK_unit * LSNSPK_unit_open(const char * pPathUnit,  int * pErrNo);


/**  关闭单元(课)
 */
void LSNSPK_unit_close(struct LSNSPK_unit * pUnitInf);


/** 查询一个单词的一个字段 
 * @param  col, 字段号，比如 LSNSPK_COL_ORIGINAL ...
 */
const char * LSNSPK_get_word_field(struct LSNSPK_unit * pUnitInf, int16_t idx, int16_t col);

/** 获取单词语音(在文件里)的位置 
 * @return  length of audio, <= 0 means fail
 */
int32_t LSNSPK_read_word_aud_location(struct LSNSPK_unit * pLsnSpk, int16_t no, int32_t * pOffset);

/** 读取一个单词的语音数据
 * @return  length of audio, <= 0 means fail
 */
int LSNSPK_read_word_aud(struct LSNSPK_unit * pLsnSpk, int16_t no, void * pBuf, int bufLen);


/** 读取一个单词的图片信息 
 * @param pSize 图片大小, 其 x、y 成员分别表示图片的宽、高;
 * @return  >0 means success, <= 0 means fail
 * Note: 执行成功时, 产生的 imgDsc 需要 caller 调用 heap_caps_free() 来释放 
 */
int32_t LSNSPK_read_word_img_info(struct LSNSPK_unit * pLsnSpk, int16_t no, lv_img_dsc_t ** ppImgDsc, lv_point_t * pSize);



/** 读取一条句子(原文或译文)
 */
const char * LSNSPK_get_lesson_sentence(struct LSNSPK_unit * pUnitInf, int16_t idx, int16_t col);

/** 获取课文语音整体数据的长度、位置 
 */
int LSNSPK_get_lesson_aud_location(struct LSNSPK_unit * pLsnSpk, int32_t * pOffset);


/** 获取课文一个句子语音位置
 * @return 该句的语音(在整个 unit 语音里的)位置, 单位 ms
 */
int32_t LSNSPK_get_lesson_sent_ptInTime(struct LSNSPK_unit * pUnitInf, int16_t idx);

/** 读取外研版的全解数据
 * @return 该条目的具体文本内容
 */
char* LSNSPK_read_qj_data(const char* pPathUnit, uint8_t style, uint8_t* chapter_num, uint32_t* size, int* pErrNo);
//--------------------------------------------------------------

struct LSNSPK_book;

typedef struct {       // unit 文件基本信息 
    uint16_t lesson_n;	// 课文数量 
    uint16_t subject;  // 0 - english, 1 - yuwen. 
    char unitRltvPath[0];  // unit文件相对路径 
} LSNSPK_book_t;

/** 初始化 book 文件的基本信息 
 * @return >= 0 means success. 
 */
struct LSNSPK_book * LSNSPK_book_open(const char * pPathUnit,  int * pErrNo);

/** 关闭 book 文件 
 */
void LSNSPK_book_close(struct LSNSPK_book * pBookInf);

/** 取得目录里的一项的一个字段 
 * @param colNo - 0 title, 1 fileName. 
 */
const char * LSNSPK_book_get_item_filed(struct LSNSPK_book * pBookInf, int16_t idx, int16_t fieldNo);


//----------------------------------------------------------------------

struct DIC_ALBUM_ITEM_s {
    char* m_ptitle;                    // 专辑标题
    int16_t m_total;                   // 该专辑下的课数量
    int32_t m_imgId;                   // 课的封面图片ID， 指向
};

struct LSNSPK_MENU {
	int16_t m_itemTotal;
	FILE  * m_fp;
	struct DIC_ALBUM_ITEM_s astAlbumTbl[0];	
};


struct LSNSPK_MENU * LSNSPK_menu_open(const char* pcAlbumPath);

void LSNSPK_menu_close(struct LSNSPK_MENU * pMenu);

lv_img_dsc_t * LSNSPK_get_cover_data(const char* pPathUnit, int* pErrNo);

#endif


