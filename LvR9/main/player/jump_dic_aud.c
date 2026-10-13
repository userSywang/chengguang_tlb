

#include "../utils/misc_utils.h"
//#include "../../components/lvgl/src/misc/lv_assert.h" 
#include "jump_dic_aud.h"

#include <stdlib.h>
#include <string.h>


#define TAG "dicAud:"

#define STR_SNDLIB_IDENTIF  "BlDSndA"

#define PATH_DIC_AUD_US  "/sdcard/sysData/dict/endsnd_am.bin"
#define PATH_DIC_AUD_UK  "/sdcard/sysData/dict/endsnd_br.bin"

//----------------------------------------------

static S_SND_LIB_INF g_sndLibAInf;
static S_SND_LIB_INF g_sndLibBInf;
FILE * g_aAudFileHdls[2];

//----------------------------------------------

int dic_ensureAudLibInfoInit(int sndLib, char *pDecPara) {
	unsigned char cPara = '\0';
	S_SND_LIB_INF *pSndLibInf = NULL;
	char acBuff[24];
	FILE * f_sndLib = NULL;

	//LV_ASSERT(sndLib <= 1 && sndLib >= 0);
	if (pDecPara) {
		(*pDecPara) = '\0';
	}
	
	f_sndLib = g_aAudFileHdls[sndLib];
	if (!f_sndLib) {
		const char * pcPath = (0 == sndLib ? PATH_DIC_AUD_US : PATH_DIC_AUD_UK);
		f_sndLib = zb_fopen(pcPath, "rb");
		if (!f_sndLib) {
			ESP_LOGE(TAG, "fail to open %s", PATH_DIC_AUD_US);
			return -3;
		}
		g_aAudFileHdls[sndLib] = f_sndLib;
	}
	
	pSndLibInf = (0 == sndLib ? &g_sndLibAInf : &g_sndLibBInf);
	if ((pSndLibInf->iSndTotal > 0) && (pSndLibInf->decPara != 0)) {
		if (pDecPara) {
			(*pDecPara) = pSndLibInf->decPara;
		}
		return 0;
	}
	//---------
	memset(acBuff, 0, sizeof(acBuff));
	fseek(f_sndLib, 0, 0);
	fread(acBuff, 1, sizeof(acBuff), f_sndLib);
	if (memcmp(acBuff, STR_SNDLIB_IDENTIF, 8) != 0) {
		printf("sndlib not valid\n");
		return -5;
	}
	int fmtVer = GET_UINT16_LITTLEENDIAN(acBuff + 16);
	if (!((fmtVer >= 0) && (fmtVer < 8))) {  // sndLib 格式版本过高,不支持.
		return -7;
	}
	
	pSndLibInf->iSndTotal = GET_UINT32_LITTLEENDIAN(acBuff + 8);
	pSndLibInf->iDataBaseOffset = GET_UINT32_LITTLEENDIAN(acBuff + 12);
	int idxTblOffset = GET_UINT16_LITTLEENDIAN(acBuff + 18); 
	if (idxTblOffset < 0x20 || idxTblOffset > 0x4000) {
		idxTblOffset = 0x20;
	}
	pSndLibInf->idxTblOffset = idxTblOffset;
	//-----------
	cPara = '\0';
	fseek(f_sndLib, -1, SEEK_END);
	fread(&cPara, 1, 1, f_sndLib);
	cPara ^= 0x3a;
	pSndLibInf->decPara = cPara;
	if (pDecPara) {
		(*pDecPara) = cPara;
	}
	return 1;
}


static void bDic_cvtBuf_32(unsigned char param, unsigned int fileOffset, unsigned char * pBuf, int size) {

#define C_SND_KEEP_AREA_LEN  0x20
	if (fileOffset + size <= C_SND_KEEP_AREA_LEN) {
		return;
	}

	int skip = 0;
	if (fileOffset < C_SND_KEEP_AREA_LEN) {  // 起始位置仍在 keep区, 跳过.
		skip = C_SND_KEEP_AREA_LEN - fileOffset;
		fileOffset = C_SND_KEEP_AREA_LEN;
		pBuf += skip;
		size -= skip;
	}
	int count = size;
	while (count > 0) {
		(*pBuf) ^= param;
		pBuf++;
		count--;
	}
	return;
}


/* 按语音号求得语音长度 
 * @param   sndLib 语音库(0或1)
 * @return  语音长度, < 0 means fail 
 */
int getAudLengthAndOffset(int sndLib, int sndIdx, int32_t * pOffset) {
	int iFuncRet = -1;

	//LV_ASSERT(sndLib <= 1 && sndLib >= 0);
	do {
		FILE * f_sndLib = NULL;
		char decParm = 0;
		f_sndLib = g_aAudFileHdls[sndLib];

		unsigned char buffer[4*2];
		int twoOffsetOrg[2];
		int32_t twoOffset[2];
		int length = 0;

		if (sndIdx < 0) {
			return -3;
		}
		const S_SND_LIB_INF *pSndLibInf = (0 == sndLib ? &g_sndLibAInf : &g_sndLibBInf);
		//----- check sndIdx valid (for newDict with oldSndLib)
		if ((sndIdx < 0) || (sndIdx >= pSndLibInf->iSndTotal)) {
			ESP_LOGE(TAG, "sndIdx %d < sndTtl:%d", sndLib, pSndLibInf->iSndTotal);
			return -9;
		}

		int iFilePos = pSndLibInf->idxTblOffset + 4 * sndIdx;
		fseek(f_sndLib, iFilePos, 0);
		fread(buffer, 4, 2, f_sndLib);
		memcpy(twoOffsetOrg, buffer, 4 * 2);
		bDic_cvtBuf_32(pSndLibInf->decPara, iFilePos, buffer, 4 * 2);
		twoOffset[0] =  GET_UINT32_LITTLEENDIAN(buffer);
		twoOffset[1] =  GET_UINT32_LITTLEENDIAN(buffer + 4);
		length = twoOffset[1] - twoOffset[0];
		iFuncRet = length;
		if (iFuncRet <= 0) {
			printf("----11L i:%x %x; %x, %x; oA:%lx,oB:%lx\n", sndIdx,iFilePos, twoOffsetOrg[0],twoOffsetOrg[1],	twoOffset[0], twoOffset[1]);
			return -11;
		}
		if (pOffset) {
			(*pOffset) = pSndLibInf->iDataBaseOffset + twoOffset[0];
		}
	} while (0);

	return iFuncRet;
}


void dic_audLib_deinit() {
	FILE * fp = g_aAudFileHdls[0];
	if (fp) {
		zb_fclose(fp);
		g_aAudFileHdls[0] = NULL;
	}
	fp = g_aAudFileHdls[1];
	if (fp) {
		zb_fclose(fp);
		g_aAudFileHdls[1] = NULL;
	}
}



