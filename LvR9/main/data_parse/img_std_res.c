
#include <lvgl/lvgl.h>
#include <lvgl/src/draw/lv_img_decoder.h>

#include "../utils/misc_utils.h"

#include <stdio.h>
#include <stdlib.h>

#include "img_std_res.h"

#define TAG "imgStdR"


//------------------------------------------------------
#define C_IMGSTD_CACHE_TBL_LEN  15 // imgStd 缓冲区大小 


#define HASH_VAL_FOR_REPL_ZERO  0x5a5a

typedef struct { 
	uint32_t  m_hashVal;
	lv_img_dsc_t * m_pImgDsc;
	uint16_t  m_imgDataLen;  // when size > 64Kb, use 0xffff
	int16_t  m_lastAccessNo; // start from  1 (0 means space item), less means old, greater means new
} imgstd_cache_item_t;


#define M_IMGSTD_INT16_INC(i) i++; \
	if (i < 0) { \
		i = 0; \
	} 


//------------------------------------------------------

//static zb_img_std_res_t * g_pImgRes = NULL; 

static imgstd_cache_item_t * g_pImgCache = NULL;
static int16_t g_accessNoCur = 0;  // 1 to 32767, then round to 1

//--------------------------------------------------------------

/** (从文件里)加载独立 jpg/png 图片资源 
 * @return  >= 0 means success 
 */
lv_img_dsc_t * zb_stdImgRes_load(const char * pcImgPath, int * pErrCode)
{
	lv_img_dsc_t * imgDsc = NULL;
	int iFuncRet = -1;
	lv_res_t result = 0;
	LV_ASSERT(pcImgPath);

	do {
		imgDsc = (lv_img_dsc_t *)heap_caps_malloc_dflt(sizeof(lv_img_dsc_t));
		LV_ASSERT(imgDsc != NULL);
		memset(imgDsc, 0, sizeof(lv_img_dsc_t));
		result = lv_img_decoder_get_info(pcImgPath, &imgDsc->header);
		if (result != LV_RES_OK) {
			iFuncRet = -5;  break;
		}

		lv_img_decoder_dsc_t imgDecDsc = { 0 };

		result = lv_img_decoder_open(&imgDecDsc, pcImgPath, lv_color_white(), 0);
		if (result != LV_RES_OK) {
			iFuncRet = -7;  break;
		}

		bool isPng = (imgDecDsc.img_data != NULL);
		lv_coord_t imgHeight = imgDsc->header.h;
		lv_coord_t imgWidth  = imgDsc->header.w;

		int imgDecLen = 2 * imgWidth * imgHeight;
		if (!isPng) {
			uint8_t * imgDat = (uint8_t *)heap_caps_malloc_dflt(imgDecLen);
			LV_ASSERT(imgDat != NULL);
			memset(imgDat, 0, imgDecLen);

			for (lv_coord_t y = 0; y < imgHeight; y++) {
				
				lv_img_decoder_read_line(&imgDecDsc, 0, y, imgWidth, imgDat + (2 * imgWidth) * y);
			}

			imgDsc->data = imgDat;
		}
		else {
			imgDsc->data = imgDecDsc.img_data;
			imgDecDsc.img_data = NULL;  // 以免调用 lv_img_decoder_close() 时把该 imb_buf 给释放了 
		}
		lv_img_decoder_close(&imgDecDsc);

		imgDsc->data_size = imgDecLen;
		imgDsc->header.cf = LV_IMG_CF_TRUE_COLOR;
		iFuncRet = 0;
	} while (0);


	if (iFuncRet < 0) {
		ESP_LOGI(TAG, "tstImgDec fail, %d, %s", iFuncRet, pcImgPath);
		if (imgDsc) {
			heap_caps_free(imgDsc);  imgDsc = NULL;
		}
	}
	if (pErrCode) {
		(*pErrCode) = iFuncRet;
	}
	return imgDsc;
}


void zb_stdImgRes_unload(lv_img_dsc_t * imgDsc)
{
	if (!imgDsc)
		return;

	heap_caps_free(imgDsc->data);  imgDsc->data = NULL;
	heap_caps_free(imgDsc);
	return;
}


static int16_t _imgStd_find_cache_pos(uint32_t imgHash)
{
	int16_t foundNo = -1;
	for (int16_t idx = 0; idx < C_IMGSTD_CACHE_TBL_LEN; idx++) {
		if (imgHash == g_pImgCache[idx].m_hashVal) {
			foundNo = idx;  break;
		}
	}
	if ((foundNo >= 0) && !g_pImgCache[foundNo].m_pImgDsc) {
		return -3;
	}
	return foundNo;
}

// 
static void _imglst_insert_item_to_cache(uint32_t imgHash, lv_img_dsc_t * pImgDsc, bool isPng)
{
	int16_t minPrior = 0;
	int16_t minPriorPos = -1;

	if (!pImgDsc) return;
	minPrior = g_pImgCache[0].m_lastAccessNo;
	minPriorPos = 0;
	for (int16_t idx = 1; idx < C_IMGSTD_CACHE_TBL_LEN; idx++) {
		int16_t lastAccessNo = g_pImgCache[idx].m_lastAccessNo;
		if (minPrior > lastAccessNo) {
			minPrior = lastAccessNo;
			minPriorPos = idx;
		}
	}
	imgstd_cache_item_t * pCacheItem = g_pImgCache + minPriorPos;
	lv_img_dsc_t * pImgDscOld = pCacheItem->m_pImgDsc;
	if (pImgDscOld) { // free old imgDsc
		heap_caps_free(pImgDscOld->data);
		heap_caps_free(pImgDscOld);  pCacheItem->m_pImgDsc = NULL;
	}
	pCacheItem->m_hashVal = imgHash;
	pCacheItem->m_pImgDsc = pImgDsc;
	pCacheItem->m_lastAccessNo = g_accessNoCur;
	M_IMGSTD_INT16_INC(g_accessNoCur)
}


int zb_stdImgPool_init(void)
{
	int iFuncRet = -1;
	if (g_pImgCache) {
		ESP_LOGW(TAG, "already init, skip");
		return -1;
	}

	do {
		int cacheBufSize = sizeof(imgstd_cache_item_t) * C_IMGSTD_CACHE_TBL_LEN;
		g_pImgCache = (imgstd_cache_item_t *)heap_caps_malloc_dflt(cacheBufSize);
		if (!g_pImgCache)
			break;
		memset(g_pImgCache, 0, cacheBufSize);
		g_accessNoCur = 1;

		iFuncRet = 0;
	} while (0);

	return iFuncRet;
}

void zb_stdImgPool_deinit(void)
{
	if (!g_pImgCache)
		return;

	for (int sn = 0; sn < C_IMGSTD_CACHE_TBL_LEN; sn++)
	{
		imgstd_cache_item_t * pItem = g_pImgCache + sn;
		if (pItem->m_pImgDsc != NULL) {
			zb_stdImgRes_unload(pItem->m_pImgDsc);
			pItem->m_pImgDsc = NULL;
		}
	}

	heap_caps_free(g_pImgCache);
	g_pImgCache = NULL;
	return;
}


/**
 * (从 原图片文件或 cache里)读取一张图的数据到内存 img_dsc struct, 并自动加到缓存里; 
 * @param 
 * @return note 0  means success. 
 */
lv_img_dsc_t * zb_stdImgPool_load_one(const char * pcImgPath, uint32_t * piHash, int * pErrNo)
{
	lv_img_dsc_t * pImgDsc = NULL;
	int16_t idx = 0;
	int16_t iFuncRet = -1;

	if (!g_pImgCache) {
		return NULL;
	}
	LV_ASSERT(pcImgPath);

	do {
		// cacl hash;
		uint32_t hashVal = 0;
		if (!piHash || (0 == (*piHash))) {
			int strLen = strlen(pcImgPath);
			hashVal = esp_rom_crc32_le(0, (uint8_t const *)pcImgPath, strLen);
			if (0 == hashVal) {
				hashVal = HASH_VAL_FOR_REPL_ZERO;
			}
			if (piHash) {
				(*piHash) = hashVal;
			}
		}
		else {
			if (piHash) {
				hashVal = (*piHash);
			}
		}

		int16_t noInCache = _imgStd_find_cache_pos(hashVal);
		if (noInCache >= 0) {  // img 缓存命中了 
			pImgDsc = g_pImgCache[noInCache].m_pImgDsc;
			g_pImgCache[noInCache].m_lastAccessNo = g_accessNoCur;
			M_IMGSTD_INT16_INC(g_accessNoCur)
			iFuncRet = 0;
			break;
		}

		//  build one imgDsc
		pImgDsc = zb_stdImgRes_load(pcImgPath, NULL);
		if (NULL == pImgDsc) {
			iFuncRet = -9;  break;
		}
		_imglst_insert_item_to_cache(hashVal, pImgDsc, false);

		iFuncRet = 0;
	} while (0);

	if (iFuncRet < 0) {
		ESP_LOGW(TAG, "resLoad fail %d", iFuncRet);
	}
	return pImgDsc;
}


