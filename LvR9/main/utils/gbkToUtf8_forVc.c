
#ifdef C_PC_SIM
#include <windows.h>
#include <stdio.h>

typedef int funcConvertStr(unsigned short *);

// note: unit of iDstBuffSize is WCHAR.
//       if pwcStrDst == NULL, mean only measure the length of destString.
int GbkToUnicode(char* pStrSrc, wchar_t *pwcStrDst, int iDstBuffSize)
{
	int  iUniLenEx = MultiByteToWideChar( CP_ACP,0,	pStrSrc,-1,
		NULL, 0);
	if (NULL == pwcStrDst) {
		return iUniLenEx - 1;
	}

	if (iUniLenEx > iDstBuffSize) {
		iUniLenEx = iDstBuffSize;
	}
	iUniLenEx = MultiByteToWideChar( CP_ACP, 0,	pStrSrc,-1,
		pwcStrDst, iUniLenEx);
	
	if (iUniLenEx > 0) {
		return iUniLenEx - 1;	// don't include \0
	}
	return 0;
}

//-----------------------------
//  input : if param == 0, then fill end flag in tail when cut.
//   note : if pszUTF8 == NULL, mean only measure the length of destString.
// return : length of destString(don't include \0);
int UTF16ToUTF8(const wchar_t* pUnicode, char* pStrDst, int iDstBuffSize, int param)
{
	int	iCutWLen = -1;
	int	iCurFlgLen = 0;

	int iDestLenEx = WideCharToMultiByte( CP_UTF8,0,	pUnicode,-1,
		NULL,	0, NULL, NULL);
	if (NULL == pStrDst) {
		return iDestLenEx - 1;
	}

	if ((param & 1) == 0) {
		iCurFlgLen = 1;
	}

	if (iDestLenEx > iDstBuffSize) {
		int	iWLen, iWPos, iWPosPrev;

		iWLen = (int)wcslen(pUnicode);
		iWPosPrev = iWPos = (iDstBuffSize - 1) / 4;	// non-std, one char take 4 byte at most.
		for (; iWPos < iWLen; iWPos++) {
			iDestLenEx = WideCharToMultiByte( CP_UTF8, 0,	pUnicode,iWPos,
				NULL, 0, NULL, NULL);
			if (iDestLenEx + iCurFlgLen > iDstBuffSize) {
				break;
			}
			iWPosPrev = iWPos;
		}
		iCutWLen = iWPosPrev;
	}

	iDestLenEx = WideCharToMultiByte( CP_UTF8, 0,	pUnicode, (iCutWLen >= 0 ? iCutWLen : -1),
		pStrDst, iDestLenEx, NULL, NULL);
	if (iCutWLen >= 0) {	// use partial of inputBuff.
		if (iDestLenEx >= 0) {
			pStrDst[iDestLenEx] = '\0';
		}
	}
	else {	// use whole of inputBuff, include terminating null char.
		if (iDestLenEx > 0) {
			iDestLenEx--;
		}
	}
	if (iDestLenEx > 0) {
		if (iCurFlgLen > 0) {
			if (iCutWLen >= 0 && iDestLenEx + 1 < iDstBuffSize) {
				pStrDst[iDestLenEx++] = '?';
				pStrDst[iDestLenEx] = '\0';
			}
		}
		return iDestLenEx;	// don't include \0
	}
	return 0;
}

int UTF8ToUTF16(char* pStrSrc, wchar_t* pwcStrDst, int iDstBuffSize, int param)
{
	int  iUniLenEx = MultiByteToWideChar( CP_UTF8,0,	pStrSrc,-1,
		NULL, 0);

	if (NULL == pwcStrDst) {
		return iUniLenEx - 1;
	}

	if (iUniLenEx > iDstBuffSize) {
		iUniLenEx = iDstBuffSize;
	}
	iUniLenEx = MultiByteToWideChar( CP_UTF8, 0,	pStrSrc,-1,
		pwcStrDst, iUniLenEx);

	if (iUniLenEx > 0) {
		return iUniLenEx - 1;	// don't include \0
	}
	return 0;
}

// param : pwAid[] 转换辅助 buffer, 该 buffer 必须足够大.
int GbkToUtf8(char* pStrSrc, char *pcStrDst, int iDstBuffSiz, wchar_t *pwAid, 
			  int iMidBuffSiz, funcConvertStr funConvert) {

	int isTmpAlloc = 0;
	int iFuncRet = 0;

	if (!pwAid) {
		iMidBuffSiz = GbkToUnicode(pStrSrc, NULL, 0) + 1;
		
		pwAid = (wchar_t *)malloc(sizeof(WORD) * iMidBuffSiz);
		if (!pwAid) {
			fprintf(stderr, "Fail to allocate mem in GbkToUtf8!\n");
			return -1;
		}
		isTmpAlloc = 1;
	}
	GbkToUnicode(pStrSrc, pwAid, iMidBuffSiz);
	if (funConvert != NULL) {
		(*funConvert)((unsigned short *)pwAid);
	}
	iFuncRet = UTF16ToUTF8(pwAid, pcStrDst, iDstBuffSiz, 0);
	if (isTmpAlloc) {
		free(pwAid);
		pwAid = NULL;
	}
	return iFuncRet;
}

//---------------------------
//  input : if param == 0, then fill end flag in tail when cut.
// return : length of destString(don't include \0);
int UnicodeToGbk(const wchar_t* pUnicode, char* pStrDst, int iDstBuffSize, int param)
{
	int	iCutWLen = -1;
	int	iCutFlgLen = 0;		// 截取标志符的长度(0 or 1)

	int iGbkLenEx = WideCharToMultiByte( CP_ACP,0,	pUnicode,-1,
		NULL,	0, NULL, NULL);
	if (NULL == pStrDst) {
		return iGbkLenEx - 1;
	}

	if ((param & 1) == 0) {
		iCutFlgLen = 1;
	}

	if (iGbkLenEx > iDstBuffSize) {
		int	iWLen, iWPos, iWPosPrev;
		int	iGbkLengh;

		iWLen = (int)wcslen(pUnicode);
		iWPosPrev = iWPos = (iDstBuffSize - 1) / 2;	// a GBK char take 2 bytes at most.
		for (; iWPos < iWLen; iWPos++) {
			iGbkLengh = WideCharToMultiByte( CP_ACP, 0,	pUnicode,iWPos,
				NULL, 0, NULL, NULL);
			if (iGbkLengh + iCutFlgLen + 1 > iDstBuffSize) {
				break;
			}
			iWPosPrev = iWPos;
		}
		iCutWLen = iWPosPrev;
	}

	iGbkLenEx = WideCharToMultiByte( CP_ACP, 0,	pUnicode, (iCutWLen >= 0 ? iCutWLen : -1),
		pStrDst, iDstBuffSize, NULL, NULL);
	if (iCutWLen >= 0) {	// use partial of inputBuff.
		if (iGbkLenEx >= 0) {
			pStrDst[iGbkLenEx] = '\0';
		}
	}
	else {	// use whole of inputBuff, include terminating null char.
		if (iGbkLenEx > 0) {
			iGbkLenEx--;
		}
	}
	//--- 到这里时, iGbkLenEx 所指长度已经不包含\0.

	if (iGbkLenEx > 0) {
		if (iCutFlgLen > 0) {
			if (iCutWLen >= 0 && iGbkLenEx + 1 < iDstBuffSize) {
				pStrDst[iGbkLenEx++] = '?';
				pStrDst[iGbkLenEx] = '\0';
			}
		}
		return iGbkLenEx;	// don't include \0
	}
	return 0;
}

//---------------------------
//  input : if param == 0, then fill end flag in tail when cut.
//   note : if pwcStrDst == NULL, mean only measure the size of destString.
// return : length of destString(don't include \0);
int UTF8ToGBK(char* pcStrSrc, char* pcStrDst, int iDstBuffSize, int param)
{
	int	iStrUTF16Len, iStrGbkLen;
	wchar_t	*pwUniData = NULL;

	iStrUTF16Len = UTF8ToUTF16(pcStrSrc, NULL, 0, 0);
	pwUniData = (wchar_t *)malloc(sizeof(wchar_t) * (iStrUTF16Len + 1));
	UTF8ToUTF16(pcStrSrc, pwUniData, iStrUTF16Len + 1, 0);

	if (NULL == pcStrDst) {
		iStrGbkLen = UnicodeToGbk(pwUniData, NULL, 0, 0);
	}
	else {
		iStrGbkLen = UnicodeToGbk(pwUniData, pcStrDst, iDstBuffSize, 0);
	}

	if (pwUniData != NULL) {
		free(pwUniData);
		pwUniData = NULL;
	}
	return iStrGbkLen;
}
#endif

