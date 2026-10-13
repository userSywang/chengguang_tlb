/**
 * 为了便于 editor 识别文字编码,请保留本行的中文注释 
 * @file	log_aud_lyric.h
 * @brief	manager music lyric
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#include "../utils/misc_utils.h"
#include "../utils/encode_utils.h"

#include <lvgl/lvgl.h> 

#include "aud_lyric.h"


//------------------------------------------------------------------------------------
#define TAG  "logAudLrc"

#define MAX_FILE_LEN    260

#define  COMMON_BUFFER_SIZE  (1024 * 8)
//-------------------------------------------------------

#define UTL_ALIGNBASE	(sizeof(uint32_t))
#define UTL_ALIGNBIGK	(sizeof(uint32_t)<<2)
#define UTL_ALIGNED(x)	((uint32_t)(x) & (sizeof(uint32_t)-1))
#define UTL_ISNULL(x)	(((x)-0x01010101) & ~(x) & 0x80808080)


char * Utl_StrCpyN(char * strDest, const char * strSrc, T_S32 length)
{
	char * ptr0 = strDest;

	assert((strDest && strSrc));

	if ((length >= UTL_ALIGNBASE) && !UTL_ALIGNED(strDest) && !UTL_ALIGNED(strSrc))
	{
		T_U32* align1 = (T_U32*)strDest;
		T_U32* align2 = (T_U32*)strSrc;

		while (!UTL_ISNULL(*align2) && length >= UTL_ALIGNBASE)
		{
			*align1++ = *align2++;
			length -= UTL_ALIGNBASE;
		}

		strDest = (char *)align1;
		strSrc = (char *)align2;
	}

	while (length-- > 0)
	{
		if (!(*strDest++ = *strSrc++))
			break;
	}

	*strDest++ = 0;

	return ptr0;
}


int64_t zb_getFileLength_byHdl(FILE * fp) {
	//ESP_LOGE(TAG, "已进入：zb_getFileLength");
	int ret = 0;
	int64_t posBak = ftell(fp);

	fseek(fp, 0, SEEK_END);
	int64_t len = ftell(fp);
	fseek(fp, posBak, SEEK_SET);
	return len;
}

//-------------------------------------------------------------

/**
 * @brief	Convert lyric time string to U32 type
 * @param	pStr:	string pointer
			pRst:	result
 * @return	T_U8
 * @retval	0: OK
 **/
static T_U8	Aud_LyrCvtTimStr2U32(T_S8* pStr,T_U32* pRst)
{
	char	timeAsc[3][3] = {0};
	T_U16	len;

	len = strlen((char *)pStr);
	timeAsc[0][0] = *(pStr+0);
	timeAsc[0][1] = *(pStr+1);
	timeAsc[1][0] = *(pStr+3);
	timeAsc[1][1] = *(pStr+4);
	if(len < 5)
	{
		timeAsc[2][0] = 0;
		timeAsc[2][1] = 0;
	}
	else
	{
		timeAsc[2][0] = *(pStr+6);
		timeAsc[2][1] = *(pStr+7);
	}

	*pRst = (T_U32)( Utl_Atoi(timeAsc[0])*60*1000 + Utl_Atoi(timeAsc[1])*1000 +
					 Utl_Atoi(timeAsc[2]) );
	return 0;
}

/**
 * @brief	Check string is time string
 * @param	pBuf:		string buffer
 * @return	T_BOOL
 * @retval	true:	time string
			false:	not time string
 **/
static T_BOOL Aud_LyrCheckIsTimeStr(char * pBuf)
{
	if( (*(pBuf+2) != ASCII_COLON) )
	{
		return false;
	}
	if( (*pBuf > '9' ) || (*pBuf < '0' ) || (*(pBuf+1) > '9' ) || (*(pBuf+1) < '0' ) ||
		(*(pBuf+3) > '9' ) || (*(pBuf+3) < '0' ) || (*(pBuf+4) > '9' ) || (*(pBuf+4) < '0' ) /*||
		(*(pBuf+6) > '9' ) || (*(pBuf+6) < '0' ) || (*(pBuf+7) > '9' ) || (*(pBuf+7) < '0' )*/ )
	{
		return false;
	}
	return true;
}


/**
 * @brief	Insert an element to lyric index table
 * @param	tDest:	destination time
			pos:	destination position
			lyrIdx: T_AUD_LYRIC_IDX
 * @return	T_U8
 * @retval	0: OK
			1: inserted index less than
			2: inserted index more than
			3: other error
 **/
static T_U8 Aud_LyrIdxInsertEle(T_AUD_LYRIC_IDX* lyrIdx, T_U32 tDest, T_U32 pos)
{
	T_U16 i,j;
	T_U32* pLyrIdx;

	i = 0;
	j = 0;
	pLyrIdx = lyrIdx->idxTbl[0];

	//max and min value process
	if(0 == lyrIdx->cntIdx)
	{//first time
		lyrIdx->idxMax[0] = tDest;
		lyrIdx->idxMax[1] = pos;
		lyrIdx->idxMin[0] = tDest;
		lyrIdx->idxMax[1] = pos;
	}
	else
	{//no first time
		if(tDest > lyrIdx->idxMax[0])
		{
			lyrIdx->idxMax[0] = tDest;
			lyrIdx->idxMax[1] = pos;
		}
		if(tDest < lyrIdx->idxMin[0] )
		{
			lyrIdx->idxMin[0] = tDest;
			lyrIdx->idxMax[1] = pos;
		}
	}

	//index table value process
	if(0 == lyrIdx->cntIdx)
	{
		lyrIdx->cntIdx++;
		*(pLyrIdx) = tDest;
		*(pLyrIdx+1) = pos;
		return 2;
	}

	if(tDest <= *(pLyrIdx+i*2))
	{//less than 1st item.
        if (lyrIdx->bFirstBulid)	// If lrc has too much sentence(>LYRIC_MAX_INDEX_ELE), then we think that the sentence must appear in time order.
        {
 	        if(lyrIdx->cntIdx < LYRIC_MAX_INDEX_ELE )
	        {
                for(j = lyrIdx->cntIdx ; j>i; j--)
                {
					*(pLyrIdx+j*2) = *(pLyrIdx+(j-1)*2);
					*(pLyrIdx+j*2+1) = *(pLyrIdx+(j-1)*2+1);
                }
				*(pLyrIdx+i*2)      = tDest;
				*(pLyrIdx+i*2+1)    = pos;
                lyrIdx->cntIdx ++;
	        }
        }
		return 1;
	}

	for(i=0; i<lyrIdx->cntIdx; i++)
	{
		if(tDest > *(pLyrIdx+i*2))
		{//larger than 1st one
			if( tDest < *(pLyrIdx+(i+1)*2) )
			{//little than 2ed one,insert it
				for(j=LYRIC_MAX_INDEX_ELE-2; j>i; j--)
				{
					*(pLyrIdx+(j+1)*2) = *(pLyrIdx+j*2);
					*(pLyrIdx+(j+1)*2+1) = *(pLyrIdx+j*2+1);
				}
				*(pLyrIdx+(i+1)*2) = tDest;
				*(pLyrIdx+(i+1)*2+1) = pos;

				if(lyrIdx->cntIdx < LYRIC_MAX_INDEX_ELE)
				{
					lyrIdx->cntIdx ++;
				}
				return 0;
			}
			else if( tDest > *(pLyrIdx+(i+1)*2) )
			{//larger than 2ed one
				continue;
			}
			else
			{//equal
				return 0;
			}
		}
	}

	if(lyrIdx->cntIdx < LYRIC_MAX_INDEX_ELE)
	{
		*(pLyrIdx+lyrIdx->cntIdx*2) = tDest;
		*(pLyrIdx+lyrIdx->cntIdx*2+1) = pos;
		lyrIdx->cntIdx ++;
		return 2;
	}

	return 3;
}

/**
 * @brief	Analysis buffer and produce lyric index table
 * @param	pBuf:		data buffer
			offsetPos:	offset for file
			lenBuf:		len of data
			pLyrIdx:	T_AUD_LYRIC_IDX pointer
 * @return	T_U8
 * @retval	0: OK
 **/
static T_U32 Aud_LyrCalcIdxTbl(T_AUD_LYRIC_IDX* pLyrIdx, char * pBuf, T_U32 offsetPos, T_U32 lenBuf)
{
	T_U32	posComm,posRBracket,posLBracket,tValTemp,addoffset,posLbak;
	T_U32	i, j;
	T_U16	cntIdxCnt;
	T_U8	bLBracket;

	posComm = 0;
	posRBracket = 0;
	posLBracket = 0;
	bLBracket = 0;
	cntIdxCnt = 0;

	//search index
	for( i=0; i < lenBuf; i++ )
	{
		if(ASCII_LBRACKET == *(pBuf+i))
		{//'[' flag
			posLBracket = i;
			posRBracket = 0;
			bLBracket = 1;
		}
		else if(ASCII_RBRACKET == *(pBuf+i))
		{//']' flag
			posRBracket = i;
			if(1 == bLBracket)
			{
				if(Aud_LyrCheckIsTimeStr(pBuf+posLBracket+1))
				{//rigth :.
					Aud_LyrCvtTimStr2U32((int8_t *)pBuf+posLBracket+1, &tValTemp);
					//跳过连续时间标签
					addoffset = posRBracket+1;
                    posLbak = posLBracket;
					if (ASCII_LBRACKET == *(pBuf+posRBracket+1))
					{
    					for(j=posRBracket+1; j < lenBuf; j++)
    					{
    					    if (ASCII_LBRACKET == *(pBuf+j))
    					    {//'[' flag
    					        posLBracket = j;
                    			posRBracket = 0;
                    			bLBracket = 1;
    					    }
    					    else if (ASCII_RBRACKET == *(pBuf+j))
                    		{//']' flag
                    			posRBracket = j;
                    			if (1 == bLBracket)
			                    {
			                        if (Aud_LyrCheckIsTimeStr(pBuf+posLBracket+1))
				                    {
                                        addoffset = posRBracket+1;
                                        if (ASCII_LBRACKET != *(pBuf+j+1))
        			                    {
        			                        break;
        			                    }
				                    }
				                    else
				                    {
				                        break;
				                    }
			                    }
			                    posRBracket = 0;
                    			posLBracket = 0;
                    			bLBracket = 0;
                    		}
    					}
    					if ((j >= lenBuf) || (*(pBuf+j+1) == 0))
    					{
    					    posLBracket = posLbak;
    					    break;
    					}
    				}
    				else if (0 == *(pBuf+posRBracket+1))
    			    {
    			        posLBracket = posLbak;
    					break;
    			    }
					Aud_LyrIdxInsertEle(pLyrIdx, tValTemp, addoffset+offsetPos);
				}
			}
			posComm = 0;
			posRBracket = 0;
			posLBracket = 0;
			bLBracket = 0;
		}
	}
	if (1 == bLBracket)
	{
	    lenBuf = posLBracket;
	}

	return lenBuf;
}

/**
 * @brief	Get lyric name from music name, allow dest and sour is same buff.
 * @param	dest:	destination file path
			sour:	source file path
 * @return	T_BOOL
 * @return	true: OK
 **/
T_BOOL Aud_LyrGetLyrName(char * dest, const char * sour)
{
    T_S32		i, len;
    char		extName[5] = {'.','l','r','c',0};

    if ((NULL == sour) || (0 == sour[0]))
    {
        *dest = 0;
        return false;
    }

    len = strlen(sour);
    for (i=len-1; i>=0; i--)
    {
        if ('.' == *(sour+i))
		{
            break;
		}
    }

    if (i < 0) {  // not fond '.', then copy all path.
        i = len;
    }
    if (dest != sour) {
        Utl_StrCpyN(dest, sour, i);
	}
	Utl_StrCpyN((dest+i), extName, 5);
	return true;
}

/**
 * @brief	Establish index table by the basic time and position
 * @param	audLyric:	T_AUD_LYRIC pointer
			time:		basic time
			pos:		basic position
 * @return	T_U8
 * @retval	0: OK
 **/
static T_U8	Aud_LyrEstIdxTbl(T_AUD_LYRIC* audLyric, T_U32 time, T_U32 pos)
{
	T_U16 i;
	T_U32 len, curlen, totalen, templen;
	T_U32 tempSeek;

	audLyric->lyrIdx.idxTbl[0][0] =  time;
	audLyric->lyrIdx.idxTbl[0][1] =  pos;
	if (0 == time && 0 == pos)
	{
	    audLyric->lyrIdx.bFirstBulid = true;
	}
	else
	{
	    audLyric->lyrIdx.bFirstBulid = false;
	}
	for(i=1; i<LYRIC_MAX_INDEX_ELE; i++)
	{
		audLyric->lyrIdx.idxTbl[i][0] = 0;
		audLyric->lyrIdx.idxTbl[i][1] = 0;
	}

	totalen = 0;
	tempSeek = audLyric->lyrBegPos+audLyric->StartAddr;
	while(1)
	{
		if (totalen >= audLyric->DataLen) break;
		else if (totalen + audLyric->lenTempBuf - 1 > audLyric->DataLen)
			templen = audLyric->DataLen-totalen;
		else
			templen = audLyric->lenTempBuf - 1;

		Fwl_FileSeek(audLyric->hFile, tempSeek, SEEK_SET);
		len = Fwl_FileRead(audLyric->hFile, audLyric->pTempBuf, templen);
		*(audLyric->pTempBuf + len) = 0;
		curlen = Aud_LyrCalcIdxTbl( &audLyric->lyrIdx, audLyric->pTempBuf, tempSeek, len );
		tempSeek += curlen;
		totalen += curlen;
		if (curlen == 0)
		{
		    break;
		}
	}

	//当歌曲第一个时间标签不为0时应强行置0
	if (audLyric->lyrIdx.idxTbl[0][0])
	{
	    audLyric->lyrIdx.idxTbl[0][0] = 0;
	}

	//akerror("lrcTotal", audLyric->lyrIdx.cntIdx, true);
	return 0;
}

/**
 * @brief	get a line lyric from buffer
 * @param	pBufSour:	source buffer
			lenBuf:		length of source buffer
			pBufDest:	destination buffer, output parameters
			lenDestBuf: max length of destination buffer
 * @return	T_U8
 * @retval	0: OK
			-1: can not find a line
 **/
int Aud_LyrGetLineLyrInBuf(const char * pBufSour, int32_t lenBuf, char * pDestLyr, int16_t * pLineLen)
{
	T_U32	posComm, posLBracket;
	T_U32	i;
	T_U8	bLBracket;

	posComm = lenBuf;
	if (posComm)
	{
	    posComm--;
	}
	posLBracket = 0;
	bLBracket = 0;

	//get len of lyric
	for( i=0; i < lenBuf; i++ )
	{
		if(ASCII_LBRACKET == *(pBufSour+i))
		{//'[' flag
		    posLBracket = i;
			bLBracket = 1;
		}
		else if(ASCII_RBRACKET == *(pBufSour+i))
		{//']' flag
			if(1 == bLBracket)
			{
				if(Aud_LyrCheckIsTimeStr(pBufSour+posLBracket+1))
				{
				    posComm = posLBracket;
				    if (posComm)
                	{
                	    posComm--;
                	}
					break;
				}
			}
			posLBracket = 0;
			bLBracket = 0;
		}
	}
	//去掉末尾'\r' '\n'
	while(posComm)
	{
	    if ((ASCII_COMMAND != *(pBufSour+posComm)) \
	        && (ASCII_NEWLINE != *(pBufSour+posComm)))
	    {
	        break;
	    }
	    posComm--;
	}

	T_U32 lenLine = posComm + 1;
	int outputLenMax = (*pLineLen);
	if (lenLine > outputLenMax) {
		lenLine = outputLenMax;
	}
	Utl_StrCpyN(pDestLyr, pBufSour, lenLine);
	pDestLyr[lenLine] = '\0';
	*pLineLen = (T_U16)lenLine;
	return 0;
}


int	Aud_LyrGetByNo(T_AUD_LYRIC* audLyric, int16_t sn, char * pDestLyr, int16_t * pLineLen)
{
	if (sn < 0 || (sn >= audLyric->lyrIdx.cntIdx)) {
		return -1;
	}
	if (!pDestLyr)
		return -3;
	int bufLen = COMMON_BUFFER_SIZE;
	memset(audLyric->pTempBuf, 0, bufLen);
	fseek(audLyric->hFile, audLyric->lyrIdx.idxTbl[sn][1], SEEK_SET);
	fread(audLyric->pTempBuf, 1, bufLen - 2, audLyric->hFile);

	pDestLyr[0] = '\0';
	int ret = Aud_LyrGetLineLyrInBuf(audLyric->pTempBuf, bufLen - 2, pDestLyr, pLineLen);
	if (ret != 0) {
		ESP_LOGW(TAG, "No %02d getLry ret %d", sn + 1, ret);
		return -9;
	}
	// ESP_LOGI(TAG, "No %02d:%s", sn + 1, pDestLyr);
	return 0;
}



/**
 * @brief	Check and get 1st position
 * @param	audLyric
 * @return	T_BOOL
 * @retval	true
 **/
static T_BOOL Aud_LyrGetInitPos(T_AUD_LYRIC* audLyric)
{
	T_U32	len;

	audLyric->lyrBegPos = 0;
	//read file
	audLyric->pTempBuf = (char*)heap_caps_malloc_dflt(COMMON_BUFFER_SIZE);
	audLyric->lenTempBuf = COMMON_BUFFER_SIZE;
	memset((void *)audLyric->pTempBuf, 0, audLyric->lenTempBuf);
	Fwl_FileSeek(audLyric->hFile,0+audLyric->StartAddr,0);
	len = Fwl_FileRead(audLyric->hFile, audLyric->pTempBuf, audLyric->lenTempBuf-2);
	if( 0 == len)
	{
		return 1;
	}

	//check file format
    if ((*((T_U8*)audLyric->pTempBuf) == 0xff) && ((*((T_U8 *)audLyric->pTempBuf)+1) == 0xfe))
    {//unicode,UTF-8
		return 2;
    }

	audLyric->lyrBegPos = 0;
	audLyric->lyrIdx.idxMin[0] = 60000;
	audLyric->lyrIdx.idxMax[0] = 0;
	return 0;
}

/**
 * @brief	Get lyric index
 * @param	audLyric
			destTime
 * @return	T_U8
 * @retval	
			1 ~ 3 ok,	3 means  less than 1st tmPoint(locate to 1st item)
 **/
 int	Aud_LyrGetLyrIdx(T_AUD_LYRIC * audLyric, T_U32 destTime)
{
	T_U16	i,j;
	T_U8	fGetIdx = 0;

	//check 10 times
	for (j = 0; j < 10; j++)
	{
		for (i = 0; i < audLyric->lyrIdx.cntIdx - 1; i++)
		{
			if (destTime < audLyric->lyrIdx.idxTbl[i][0])
			{//less than all index in current page
				audLyric->lyrIdx.pcRead = 0;
				audLyric->lyrIdx.lineNxtPos = 0;
				audLyric->lyrIdx.lineCurScrCnt = 0;
				audLyric->lyrIdx.lineMaxScrCnt = 0;
				if (destTime <= audLyric->lyrIdx.idxMin[0])
				{//less than 1st index value
					fGetIdx = 3;
					break;
				}
				else
				{//less than current page index,but have little index value
					fGetIdx = 5;
					audLyric->lyrIdx.cntIdx = 0;
					memset(audLyric->lyrIdx.idxTbl, 0, LYRIC_MAX_INDEX_ELE * 2);
					Aud_LyrEstIdxTbl(audLyric, audLyric->lyrIdx.idxMin[0], audLyric->lyrIdx.idxMin[1]);
					break;
				}
			}

			if ((destTime >= audLyric->lyrIdx.idxTbl[i][0]) && (destTime < audLyric->lyrIdx.idxTbl[i + 1][0]))
			{//found the right index
				if ((audLyric->lyrIdx.pcRead != i) || (audLyric->RefreshFg == true))
				{//index changed
					audLyric->lyrIdx.pcRead = i;
					audLyric->lyrIdx.lineNxtPos = 0;
					audLyric->lyrIdx.lineCurScrCnt = 0;
					audLyric->lyrIdx.lineMaxScrCnt = 0;
					audLyric->RefreshFg = false;
					fGetIdx = 1;	//get correct index
				}
				else if (audLyric->lyrIdx.pcRead == i && audLyric->lyrIdx.lineCurScrCnt < audLyric->lyrIdx.lineMaxScrCnt \
					&& audLyric->lyrIdx.lineNxtPos != 0)
				{
					fGetIdx = 1;
				}
				break;
			}

			if (i + 2 == audLyric->lyrIdx.cntIdx)
			{//not found correct index
				if (audLyric->lyrIdx.cntIdx + 1 < LYRIC_MAX_INDEX_ELE)
				{//end of all index
					if (audLyric->lyrIdx.pcRead != i)
					{//index changed
						if (audLyric->lyrIdx.pcRead != audLyric->lyrIdx.cntIdx - 1)
						{
							audLyric->lyrIdx.pcRead = i;
							audLyric->lyrIdx.lineNxtPos = 0;
							audLyric->lyrIdx.lineCurScrCnt = 0;
							audLyric->lyrIdx.lineMaxScrCnt = 0;
							fGetIdx = 2;	//more than all index
						}
						else if (audLyric->RefreshFg == true)
						{
							audLyric->RefreshFg = false;
							fGetIdx = 2;	//more than all index
						}
					}
					else
					{
						audLyric->lyrIdx.pcRead = audLyric->lyrIdx.cntIdx - 1;
						fGetIdx = 2;	//more than all index
					}
					break;
				}
				else
				{//more than current page index,but have larger index value,need to rebuild
					T_U32 baseTime, basePos;

					baseTime = audLyric->lyrIdx.idxTbl[audLyric->lyrIdx.cntIdx - 1][0];
					basePos = audLyric->lyrIdx.idxTbl[audLyric->lyrIdx.cntIdx - 1][1];
					audLyric->lyrIdx.pcRead = 0;
					audLyric->lyrIdx.lineNxtPos = 0;
					audLyric->lyrIdx.lineCurScrCnt = 0;
					audLyric->lyrIdx.lineMaxScrCnt = 0;
					audLyric->lyrIdx.cntIdx = 1;
					memset(audLyric->lyrIdx.idxTbl, 0, LYRIC_MAX_INDEX_ELE * 2 * 4);
					Aud_LyrEstIdxTbl(audLyric, baseTime, basePos);
					fGetIdx = 4;
					break;
				}
			}//end of //not found correct index
		}//end of check index

		if ((1 == fGetIdx) || (2 == fGetIdx) || (3 == fGetIdx))
		{
			break;
		}
	}

	return fGetIdx;
}



static int convertLrcFileIfNeed(const char * pcPath)
{
	int encoding = judgeTextFileEncoding(pcPath, 600);
	int iFuncRet = -1;

	if (ENCODING_TYPE_GBK == encoding) {
		char * pcGbkBuf = NULL, * pcUtf8Buf = NULL;
		wchar_t * pUniBuf = NULL;
		FILE * fp = NULL;

		ESP_LOGI(TAG, "cvtLrcFl A");
		do {
			int txtGbLen = zb_getFileLength(pcPath);
			if (txtGbLen > 1024 * 24) // too long
				break;
			pcGbkBuf = (char *)heap_caps_malloc_dflt(txtGbLen + 1);
			fp = zb_fopen(pcPath, "rb");
			if (!fp) {
				iFuncRet = -2;  break;
			}
			pcGbkBuf[0] = '\0';
			fread(pcGbkBuf, 1, txtGbLen, fp);
			zb_fclose(fp);  fp = NULL;
			pcGbkBuf[txtGbLen] = 0;
			//-------
			int uniBufLen =  txtGbLen + 1;
			pUniBuf = (wchar_t *)heap_caps_malloc_dflt(2 * uniBufLen);
			pUniBuf[0] = 0;
			int utf16StrLen = zb_gbk_to_utf16(pcGbkBuf, txtGbLen, pUniBuf, uniBufLen);
			//-------
			int utf8BufLen =  3 + utf16StrLen * 3 + 1;
			pcUtf8Buf = (char *)heap_caps_malloc_dflt(utf8BufLen);
			memcpy(pcUtf8Buf, "\xef\xbb\xbf", 3 + 1);
			int txtU8Len = zb_utf16_to_utf8(pUniBuf, pcUtf8Buf + 3, utf8BufLen);
			//-------------------------- write utf8 lrc to file (with BOM)
			fp = zb_fopen(pcPath, "wb");
			if (!fp) {
				iFuncRet = -12;	break;
			}
			fwrite(pcUtf8Buf, 1, 3 + txtU8Len, fp);
			zb_fclose(fp);  fp = NULL;
			iFuncRet = 1;
		} while (0);

		if (iFuncRet < 0) {
			ESP_LOGW(TAG, "cvt lrc Fl fail %d", iFuncRet);
		}
		if (fp != NULL) {
			zb_fclose(fp);  fp = NULL;
		}
		if (pUniBuf) {
			heap_caps_free(pUniBuf);  pUniBuf = NULL;
		}
		if (pcUtf8Buf) {
			heap_caps_free(pcUtf8Buf);  pcUtf8Buf = NULL;
		}
		if (pcGbkBuf) {
			heap_caps_free(pcGbkBuf);	pcGbkBuf = NULL;
		}
	}
	else {
		iFuncRet = 0;
	}
	return iFuncRet;
}


/**
 * @brief	Aud_LyrOpen
 * @param	audLyric
			fileName
 * @return	T_U8
 * @retval	0:	open OK
			-1:	file operation error
			-2:	file format error
 **/
int Aud_LyrOpen(T_AUD_LYRIC* audLyric, const char * fileName, int extra)
{
	char	fileNameLyr[MAX_FILE_LEN+20];

	Aud_LyrGetLyrName(fileNameLyr, fileName);
	if (extra & 1) {
		convertLrcFileIfNeed(fileNameLyr);
	}

	audLyric->hFile = zb_fopen(fileNameLyr, "rb");
	if (!audLyric->hFile) {
		ESP_LOGE(TAG, "Fail to open fl %s", fileNameLyr);
		return -1;
	}
	//ESP_LOGI(TAG, "---has open fl %s", fileNameLyr);

	audLyric->StartAddr = 0;
	audLyric->DataLen = zb_getFileLength_byHdl(audLyric->hFile);
	if (NULL == audLyric->hFile)
	{
		ESP_LOGW(TAG, "Aud_LyrOpen failed");
		return -2;
	}

	if(0 != Aud_LyrGetInitPos(audLyric))
	{
		ESP_LOGW(TAG, "Aud_LyrOpen,format error");
		zb_fclose(audLyric->hFile);
		audLyric->hFile = NULL;
		return -2;
	}
	T_U8 ret = Aud_LyrEstFirIdxTbl(audLyric);
	if (ret != 0) {
		return -7;
	}
    return 0;
}


/**
 * @brief	Establish first index table by the basic time and position
 * @param	audLyric:	T_AUD_LYRIC pointer
 * @return	T_U8
 * @retval	0: OK, other: failed
 **/
T_U8 Aud_LyrEstFirIdxTbl(T_AUD_LYRIC* audLyric)
{
	memset((T_VOID*) (&audLyric->lyrIdx),0,sizeof(T_AUD_LYRIC_IDX));
	audLyric->lyrBegPos = 0;
	if (NULL == audLyric->pTempBuf) {
		audLyric->pTempBuf = (char *)heap_caps_malloc_dflt(COMMON_BUFFER_SIZE);
	}
	audLyric->lenTempBuf = COMMON_BUFFER_SIZE;
	audLyric->RefreshFg = true;
	return Aud_LyrEstIdxTbl(audLyric, 0, 0 );
}



/**
 * @brief	Close lyric
 * @param	audLyric
 * @return	T_BOOL
 * @retval	true:	operation successful
			false:	operation failed
 **/
T_BOOL Aud_LyrClose(T_AUD_LYRIC * audLyric)
{
	if (!audLyric)
		return false;

	if (audLyric->hFile != NULL)
	{
		zb_fclose(audLyric->hFile); audLyric->hFile = NULL;
	}
	if (audLyric->pTempBuf) {
		heap_caps_free(audLyric->pTempBuf);  audLyric->pTempBuf = NULL;
	}
	return true;
}


/**
 * @brief	Get lyric max time
 * @param	audLyric
 * @return	T_U32
 * @retval	0: OK
 **/
T_U32 Aud_LyrGetLyrMaxTime(T_AUD_LYRIC* audLyric)
{
    assert(audLyric!=NULL);

	return audLyric->lyrIdx.idxMax[0];
}

