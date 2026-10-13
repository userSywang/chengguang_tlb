#ifndef _ENCODE_UITLS_H_
#define _ENCODE_UITLS_H_

#include "stdint.h"
#include "stdio.h"

#ifdef C_PC_SIM
#include <sysinfoapi.h>
#endif

#include <lvgl/src/misc/lv_types.h>



struct url_encode_s {
	char * m_pcUrlBuf;
	int16_t  m_bufSize;
	int16_t  m_curLen;
	int8_t m_urlProgressMark;  // bit0 main;  bit1 path;  bit2 query;
};

typedef struct url_encode_s url_encode_t;


/** URL 编码初始化 
 * @param pcMain - URL 的头部, 包含 scheme、authority，比如 http://example.com ;
 */
extern url_encode_t * zb_urlEnc_init(const char * pcMain, int * pErrorNo);

/** URL 编码器销毁 
 */
extern void zb_urlEnc_deinit(url_encode_t * pUrlEnc);

/** (给 url 里)填入路径部分 
 * @param : path url 的路径, 其头部不可带斜杠(/);
 * @return >= 0 means success.
 */
extern int zb_urlEnc_setPath(url_encode_t * pUrlEnc, const char * pcPath);

/** (给 url 里)填入一个查询参数 
 * @return >= 0 means success.
 */
extern int zb_urlEnc_addQueryParm(url_encode_t * pUrlEnc, const char * pcKey, const char * pcValue);


//------------------------------------------------------
#define ENCODING_TYPE_UTF8  1
#define ENCODING_TYPE_UTF16 2
#define ENCODING_TYPE_GBK   3

int zb_utf16_to_utf8(wchar_t * src, char * putf8, int dstBufLen);


extern int zb_gbk_to_utf16(const       char *pszMultiByte, int nMultiByteNum, wchar_t * pwsWideChar, int bufLen);


/* 判断文本文件的编码 
 * @return :  1 - utf-8, 2 - utf-16 3-gbk
 */
extern int judgeTextFileEncoding(const char * pcPath, int limitLen);


#endif

