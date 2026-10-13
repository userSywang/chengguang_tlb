#ifndef _JUMP_DIC_AUD_H_
#define _JUMP_DIC_AUD_H_

//#include "../../components/lvgl/src/misc/lv_types.h" 


#include <stdio.h>


typedef struct {
	unsigned char decPara;
	short idxTblOffset;
	int  iSndTotal;
	int  iDataBaseOffset;
} S_SND_LIB_INF;


extern FILE * g_aAudFileHdls[2];


extern int dic_ensureAudLibInfoInit(int sndLib, char *pDecPara);

extern int getAudLengthAndOffset(int sndLib, int sndIdx, int32_t * pOffset);

extern void dic_audLib_deinit(void);


#endif

