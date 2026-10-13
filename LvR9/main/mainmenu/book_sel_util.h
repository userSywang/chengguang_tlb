#ifndef _BOOK_SEL_UTIL_H_
#define _BOOK_SEL_UTIL_H_

#include "stdint.h"


extern void gradePublishsInfo_free(struct grade_publish_inf * pSubjPubls);

extern int readGradePublishsFromJson(const char * pcJson, const char * pcSubject, struct grade_publish_inf * pDstInfTbl);


extern struct BOK_VER_CACHE_WRT_STAT * bok_ver_cache_wrt_begin(void);

extern int bok_ver_cache_wrt_once(struct BOK_VER_CACHE_WRT_STAT * pCacheWrtStat, const char * pcJson);

extern void bok_ver_cache_wrt_end(struct BOK_VER_CACHE_WRT_STAT * pCacheWrtStat, bool doesSave, uint32_t tokenTmStampMinute);


extern int initPublishOfSubject(int16_t subject, struct grade_publish_inf * pDstInfTbl, uint32_t * piTmStamp);

extern int bok_readTokenInCacheFile(uint32_t * pTimeStampTokenMinute);


#endif

