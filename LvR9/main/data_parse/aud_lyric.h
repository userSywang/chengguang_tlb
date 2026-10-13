/** 为了便于 editor 识别文字编码,请保留本行的中文注释 
 */
#ifndef _AUD_LYRIC_H_
#define _AUD_LYRIC_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef unsigned char           T_U8;       /* unsigned 8 bit integer */
typedef unsigned short          T_U16;      /* unsigned 16 bit integer */
typedef unsigned long           T_U32;      /* unsigned 32 bit integer */
typedef signed char             T_S8;       /* signed 8 bit integer */
typedef signed short            T_S16;      /* signed 16 bit integer */
typedef signed long             T_S32;      /* signed 32 bit integer */
typedef void                    T_VOID;     /* void */
typedef	 float				    T_FLOAT;

typedef signed char             T_CHR;      /* char */
typedef unsigned char           T_BOOL;     /* BOOL type */

typedef T_VOID* T_pVOID;    /* pointer of void data */
typedef const T_VOID* T_pCVOID;   /* const pointer of void data */

typedef T_S8* T_pSTR;     /* pointer of string */
typedef const T_S8* T_pCSTR;    /* const pointer of string */


typedef T_U16                   T_WCHR;     /**< unicode char */
typedef T_U16* T_pWSTR;    /* pointer of unicode string */
typedef const T_U16* T_pCWSTR;   /* const pointer of unicode string */


typedef T_U8* T_pDATA;    /* pointer of data */
typedef const T_U8* T_pCDATA;   /* const pointer of data */


#define Utl_StrLen  strlen
#define Utl_Atoi(str)    atoi(str)

#define T_hFILE T_S32

#define Fwl_FileSeek  fseek
#define Fwl_FileRead(fp,buf,len)  fread(buf, 1, len, fp)


//-------------------------------------------------

/** ascill code */
#define ASCII_END         0x00      // '\0'
#define ASCII_COLON       0x3A      // ':'
#define ASCII_SEPARATOR   0x3B      //';'
#define ASCII_DOT         0x2E      // '.'
#define ASCII_SOLIDUS     0x2F      // '/'
#define ASCII_RES_SOLIDUS 0x5C      // '\'
#define ASCII_STAR        0x2A      // '*'
#define ASCII_QUESTION    0x3F      // '?'
#define ASCII_LBRACKET    0x5b      // '['
#define ASCII_RBRACKET    0x5d      // ']'
#define ASCII_COMMAND	  0x0D		//'\n'
#define ASCII_NEWLINE	  0x0A		//'\r'
#define ASCII_LESSFLAG    0x3C		//'<'
#define ASCII_MORERFLAG   0x3E		//'>'

/** lyric param */
#define LYRIC_MAX_INDEX_ELE				90


/** lyric time index manage */
typedef struct {
	T_U32		idxTbl[LYRIC_MAX_INDEX_ELE][2]; /* lyric index table			*/
	T_U32		idxMax[2];						/* max time of lyric			*/
	T_U32		idxMin[2];						/* min time of lyric			*/
	T_U16		cntIdx;							/* sentence counter				*/
	T_U16		pcRead;							/* read pc						*/
	T_U16		lineNxtPos;						/* position in line				*/
	T_U8		lineMaxScrCnt;					/* max display times for 1 line	*/
	T_U8		lineCurScrCnt;					/* current display times		*/
    T_BOOL      bFirstBulid;                    /*first bulid lyric table*/
} T_AUD_LYRIC_IDX;

/** Lyric manage */
typedef struct {
	T_U32           lyrBegPos;					/* lyric content begin position	*/
	FILE  *      hFile;						/* file handle					*/
	T_AUD_LYRIC_IDX lyrIdx;						/* lyric index table			*/
	char           *pTempBuf;					/* malloc by up layer,must more than 1*1024Byte */
	T_U32           lenTempBuf;					/* length of temp buffer		*/
	T_BOOL          RefreshFg;                  /* refresh flag */
	
	T_U32 StartAddr; //
	T_U32 DataLen; //
} T_AUD_LYRIC;



/**
 * @brief	Aud_LyrOpen
 * @param	audLyric
			fileName
 * @return	T_U8
 * @retval	0:	open OK
			-1:	file operation error
			-2:	file format error
 **/
int Aud_LyrOpen(T_AUD_LYRIC* audLyric, const char * fileName, int extra);


/**
 * @brief	Close lyric
 * @param	audLyric
 * @return	T_BOOL
 * @retval	true:	operation successful
			false:	operation failed
 **/
T_BOOL Aud_LyrClose(T_AUD_LYRIC* audLyric);

/**
 * @brief	Establish first index table by the basic time and position
 * @param	audLyric:	T_AUD_LYRIC pointer
 * @return	T_U8
 * @retval	0: OK, other: failed
 **/
T_U8 Aud_LyrEstFirIdxTbl(T_AUD_LYRIC* audLyric);

/**
 * @brief	Get lyric name from music name
 * @param	dest:	destination file path
			sour:	source file path
 * @return	T_U8
 * @retval	0: OK
 **/
// T_BOOL Aud_LyrGetLyrName(char * dest, const char * src);


/**
 * @brief	Get lyric max time
 * @param	audLyric
 * @return	T_U32
 * @retval	0: OK
 **/
T_U32 Aud_LyrGetLyrMaxTime(T_AUD_LYRIC* audLyric);


extern int Aud_LyrGetLineLyrInBuf(const char * pBufSour, int32_t lenBuf, char * pDestLyr, int16_t * pLineLen);


/** 根据序号读取句子文本 
 * @return  0 表示成功 
 */
extern int	Aud_LyrGetByNo(T_AUD_LYRIC* audLyric, int16_t sn, char* pDestLyr, int16_t* pLineLen);


/** locate sentence by timePoint 
 * @return 1 ` 3 表示定位 ok, 句号存放在 audLyric-》lyrIdx.pcRead 里; 
 */
extern int	Aud_LyrGetLyrIdx(T_AUD_LYRIC * audLyric, T_U32 destTime);



#ifdef __cplusplus
}
#endif


#endif
