
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "misc_utils.h"
#include "./encode_utils.h"



//--------------------------------------------------------------

//--------------------------------------------------------------

static const char * TAG = "encUtl";

//--------------------------------------------------------------

/**
 * @param  extra bit0 == 1 means is path type;
 * @return return < 0 means fail, >= 0 means output length.
 */
static int _enc_urlEncode(const char * string, char * pcDest, int bufSize, int8_t extra) 
{
	const char* hexChars = "0123456789ABCDEF";
	int length = strlen(string);
	int index = 0;
	int iFuncRet = -1;

	if (!string || !pcDest || (bufSize < 4))
		return -3;

	for (int i = 0; ; i++) 
	{
		char currentChar = string[i];
		bool hasProc = false;

		if ((0 == currentChar) || (i >= length)) { // reach tail of srcStr;
			iFuncRet = index;
			break;
		}
		if ((currentChar & 0x80) == 0) {
			if (isalnum(currentChar) || currentChar == '-' || currentChar == '_' || currentChar == '.' || currentChar == ':')
			{
				pcDest[index++] = currentChar; // Append alphanumeric and safe characters as is
				hasProc = true;
			} 
			else if (' ' == currentChar) 
			{
				pcDest[index++] = '+'; // Convert space to '+'
				hasProc = true;
			}
			else if ('/' == currentChar) {
				if (extra & 1) {
					pcDest[index++] = '/'; // keep slash symbol in path.
					hasProc = true;
				}
			}
		}

		if (!hasProc) {
			pcDest[index++] = '%'; // Convert other characters to percent-encoding
			pcDest[index++] = hexChars[(currentChar >> 4) & 0xF]; // Append first hexadecimal digit
			pcDest[index++] = hexChars[currentChar & 0xF]; // Append second hexadecimal digit
		}
	}

	pcDest[index] = '\0'; // Add null terminator
	return iFuncRet;
}



url_encode_t * zb_urlEnc_init(const char * pcUrlMain, int * pErrorNo)
{
	if (!pcUrlMain || !pcUrlMain[0])
		return NULL;
	url_encode_t * pUrlEnc = (url_encode_t *)heap_caps_malloc_dflt(sizeof(url_encode_t));
	memset(pUrlEnc, 0, sizeof(url_encode_t));

	int urlMainLen = strlen(pcUrlMain);
	int urlBufSize = urlMainLen * 3 + 200;

	pUrlEnc->m_pcUrlBuf = (char *)heap_caps_malloc_dflt(urlBufSize);
	pUrlEnc->m_bufSize = urlBufSize;

	memcpy(pUrlEnc->m_pcUrlBuf, pcUrlMain, urlMainLen + 1);
	if (pcUrlMain[urlMainLen - 1] != '/') {
		pUrlEnc->m_pcUrlBuf[urlMainLen++] = '/';
		pUrlEnc->m_pcUrlBuf[urlMainLen] = '\0';
	}
	pUrlEnc->m_curLen = urlMainLen;

	pUrlEnc->m_urlProgressMark |= (1 << 0);
	return pUrlEnc;
}

void zb_urlEnc_deinit(url_encode_t * pUrlEnc)
{
	if (!pUrlEnc)
		return;

	if (pUrlEnc->m_pcUrlBuf) {
		heap_caps_free(pUrlEnc->m_pcUrlBuf);
	}
	heap_caps_free(pUrlEnc);
	return;
}


/** (给 url 里)填入路径部分 
 * @param : path url 的路径, 其头部不可带斜杠(/);
 * @return >= 0 means success.
 */
int zb_urlEnc_setPath(url_encode_t * pUrlEnc, const char * pcPath)
{
	if (!pUrlEnc)
		return -1;

	if (pUrlEnc->m_urlProgressMark != 1)
		return -3;

	int pathLen = strlen(pcPath);
	if (0 == pathLen)
		return -5;

	int bufSizeRequired = pUrlEnc->m_curLen + 3 * pathLen + 1;
	if (pUrlEnc->m_bufSize < bufSizeRequired) {
		int bufSizeNew = bufSizeRequired;
		if (bufSizeNew < pUrlEnc->m_bufSize * 2) {
			bufSizeNew = pUrlEnc->m_bufSize * 2;
		}
		pUrlEnc->m_pcUrlBuf = heap_caps_realloc_dflt(pUrlEnc->m_pcUrlBuf, bufSizeNew);
		pUrlEnc->m_bufSize = bufSizeNew;
	}

	int ret = _enc_urlEncode(pcPath, pUrlEnc->m_pcUrlBuf + pUrlEnc->m_curLen, 
				pUrlEnc->m_bufSize - pUrlEnc->m_curLen, 1);
	if (ret < 0) {
		ESP_LOGE(TAG, "fail to encode %s", pcPath);
		return -9;
	}

	pUrlEnc->m_curLen += ret;
	if ('/' == pUrlEnc->m_pcUrlBuf[pUrlEnc->m_curLen - 1]) {
		pUrlEnc->m_curLen--;
		pUrlEnc->m_pcUrlBuf[pUrlEnc->m_curLen] = '\0';  // remove tail slash of path.
	}
	pUrlEnc->m_urlProgressMark |= (1 << 1);
	return 0;
}


/** (给 url 里)填入一个查询参数 
 * @return >= 0 means success.
 */
int zb_urlEnc_addQueryParm(url_encode_t * pUrlEnc, const char * pcKey, const char * pcValue)
{
	if (!pUrlEnc)
		return -1;

	if ((pUrlEnc->m_urlProgressMark & 3) != 3)
		return -3;

	if (!pcKey || !pcKey[0] || !pcValue || !pcValue[0])
		return -5;

	int keyLen = strlen(pcKey);
	int valLenRequired = strlen(pcValue) * 3;
	int newParmLen = keyLen + valLenRequired + 3;

	if (pUrlEnc->m_bufSize < pUrlEnc->m_curLen + newParmLen) {
		int bufSizeNew = pUrlEnc->m_curLen + newParmLen;
		if (bufSizeNew < pUrlEnc->m_bufSize * 2) {
			bufSizeNew = pUrlEnc->m_bufSize * 2;
		}
		pUrlEnc->m_pcUrlBuf = heap_caps_realloc_dflt(pUrlEnc->m_pcUrlBuf, bufSizeNew);
		pUrlEnc->m_bufSize = bufSizeNew;
	}

	char chDelim = '?';
	if ((pUrlEnc->m_urlProgressMark & (1 << 2)) != 0) {
		chDelim = '&';
	}
	int pos = pUrlEnc->m_curLen;
	snprintf(pUrlEnc->m_pcUrlBuf + pos, pUrlEnc->m_bufSize - pos, "%c%s=", chDelim, pcKey);
	pos += keyLen + 2;

	int valLen = _enc_urlEncode(pcValue, pUrlEnc->m_pcUrlBuf + pos, pUrlEnc->m_bufSize - pos, 0);
	if (valLen < 0) {
		ESP_LOGE(TAG, "fail to fill parmVal! %s", pcKey);
	}
	else {
		pos += valLen;
	}
	pUrlEnc->m_curLen = pos;
	pUrlEnc->m_urlProgressMark |= (1 << 2);
	return 0;
}


//----------------------------------------------------------------------

int zb_utf16_to_utf8(wchar_t * src, char * putf8, int dstBufLen) 
{
	int len=0;
	while (*src)
	{
		if (len + 1 >= dstBufLen)
			break;
		if (*src < 0x80) //one byte
		{
			putf8[len++] = (char)*src;
		}
		else if (*src < 0x800) //two byte
		{
			if (len + 2 >= dstBufLen)
				break;
			putf8[len++] = 0xC0 | (*src >> 6);
			putf8[len++] = 0x80 | (*src & 0x3F);
		}
		else //3 bytes
		{
			if (len + 3 >= dstBufLen)
				break;
			putf8[len++] = 0xE0 | (*src >> 12);
			putf8[len++] = 0x80 | (*src >>6 & 0x3F);
			putf8[len++] = 0x80 | (*src &0x3F);
		} 
		src ++;
	}
	putf8[len] = 0;
	return len;
} 


#include "Gbk2U16.inc"

int zb_gbk_to_utf16(const       char *pszMultiByte, int nMultiByteNum, wchar_t * pwsWideChar, int nWCBuffSize)
{
	int	nWideCharNum	= 0;
	uint32_t	uFirst, uSecond;
	uint32_t	uIndexNo;
	wchar_t * pwsU16;

	// 010. Verify.
	if (!pszMultiByte || (nMultiByteNum <= 0))
		goto	lblExit;
	// 080.
	pwsU16	= gwcGbk2U16Tbl;
	if (!pwsWideChar)
	{
		// 100. 仅仅计算转换后UTF16字符的个数。
		while (nMultiByteNum > 0)
		{
			uFirst	= *(uint8_t *)pszMultiByte ++;
			nMultiByteNum --;
			if ((uFirst > 0x80) && (uFirst < 0xff))
			{
				// 120. 必须使用第二个字节。
				nMultiByteNum --;
			}
			nWideCharNum ++;
		}
	}
	else
	{
		// 300. 转换。
		while ((nMultiByteNum > 0) && (nWCBuffSize > 0))
		{
			uFirst	= *(uint8_t *)pszMultiByte ++;
			nMultiByteNum --;
			if ((uFirst > 0x80) && (uFirst < 0xff))
			{
				// 120. 必须判断第二个字节。
				if (nMultiByteNum > 0)
				{
					nMultiByteNum --;
					// Double bytes.
					uSecond	= *(uint8_t *)pszMultiByte ++;
					if (uSecond <= 0x3f)
					{
						if (!uSecond)
							*pwsWideChar 	= 0;
						else
							*pwsWideChar 	= 0x3f;
					}
					else
					{
						uIndexNo	= (uFirst - 0x81) * 192;
						uIndexNo	+= (uSecond - 0x40);
						*pwsWideChar	= *(pwsU16 + uIndexNo);
					}
				}
				else
				{
					// Single byte.
					*pwsWideChar 	= 0;
				}
			}
			else
			{
				// Single-byte code.
				if (uFirst < 0x80)
					*pwsWideChar	= (wchar_t)uFirst;
				else if (uFirst == 0x80)
					*pwsWideChar	= 0x20ac;
				else
					*pwsWideChar	= 0xf8f5;
			}
			pwsWideChar ++;
			nWideCharNum ++;
			nWCBuffSize --;
		}
		if (nWCBuffSize > 0)
			*pwsWideChar	= 0;
	}
		
lblExit:
	return nWideCharNum;
}

#if 0
int GbkIsLeadByte(uint uCode)
{
	if ((uCode >= 0x81) && (uCode <= 0xfe))
		return 1;
	else
		return 0;
}
int GbkIsDigit(uint uCode)
{
	if (((uCode >= '0') && (uCode <= '9'))
		|| ((uCode >= 0xa3b0) && (uCode <= 0xa3b9)))
		return 1;
	else
		return 0;
}
int GbkIsLowercase(uint uCode)
{
	if (((uCode >= 'a') && (uCode <= 'z'))
		|| ((uCode >= 0xa3e1) && (uCode <= 0xa3fa)))
		return 1;
	else
		return 0;
}
int GbkIsUppercase(uint uCode)
{
	if (((uCode >= 'A') && (uCode <= 'Z'))
		|| ((uCode >= 0xa3c1) && (uCode <= 0xa3da)))
		return 1;
	else
		return 0;
}
#endif


#define ENC_CHK_READ_LEN_DFLT (1024 * 8)

int isStrMatchGbk(const char * pcStr, int limitLen)
{
	if (0 == limitLen) {
		limitLen = ENC_CHK_READ_LEN_DFLT;
	}

	int pos = 0;
	while (1) {
		uint8_t ch = (uint8_t)(*pcStr);
		int chLen = 1;

		if (0 == ch) // reach tail
			break;
		if ((uint8_t)ch < 0x81) {
		}
		else {
			uint8_t chB = (uint8_t)pcStr[1];
			chLen = 2;
			if (pos  + chLen > limitLen) // (按极限长度)已容纳不下全角字符了 
				break;
			if (!(chB >= 0x40 && chB <= 0xfe) || (0x7f == chB))
				return 0; // not GBK
		}
		pcStr += chLen;
		pos += chLen;
		if (pos >= limitLen)
			break;
	}
	return 1;
}


/*
 * @return :  1 - utf-8, 2 - utf-16 3-gbk
 */
int judgeTextFileEncoding(const char * pcPath, int limitLen)
{
	if (!pcPath || !pcPath[0] || (limitLen < 0))
		return -1;

	FILE * fp = NULL;
	char * pcTxtBuf = NULL;
	int encodingRet = ENCODING_TYPE_UTF8;
	if (0 == limitLen) {
		limitLen = ENC_CHK_READ_LEN_DFLT;
	}
	do {
		fp = zb_fopen(pcPath, "rb");
		if (!fp)
			break;
		pcTxtBuf = (char *)heap_caps_malloc_dflt(limitLen);

		int readLen = fread(pcTxtBuf, 1, limitLen, fp);
		if (readLen <= 0) {  // non-std, treat space file as utf-8 
			encodingRet = ENCODING_TYPE_UTF8;  break;
		}
		if ((readLen >= 3) && (memcmp(pcTxtBuf, "\xef\xbb\xbf", 3) == 0)) {
			encodingRet = ENCODING_TYPE_UTF8;  break;
		}
		if ((readLen >= 2) && (memcmp(pcTxtBuf, "\xff\xfe", 2) == 0)) {
			encodingRet = ENCODING_TYPE_UTF16;  break;
		}
		pcTxtBuf[readLen - 1] = '\0';
		if (isStrMatchGbk(pcTxtBuf, readLen - 1)) {
			encodingRet = ENCODING_TYPE_GBK;  break;
		}
	}while (0);

	if (fp) {
		zb_fclose(fp);  fp = NULL;
	}
	if (pcTxtBuf) {
		heap_caps_free(pcTxtBuf);  pcTxtBuf = NULL;
	}
	return encodingRet;
}



