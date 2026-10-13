
#include <lvgl/lvgl.h>
#include <lvgl/src/draw/lv_img_decoder.h>

#include "../utils/misc_utils.h"

#include <stdio.h>
#include <stdlib.h>

#include "img_res.h"

#define TAG "img_res"


/**
 * open zb image res
 * Note: this func is deprecated, pls use zb_img_res_open()、zb_img_res_load_one() instead. 
 * @return >= 0 means success 
 */
 #if 0
int loadImageFromFile(char * pcPath, int16_t idx, lv_img_dsc_t * pImgDsc) { 

	FILE * fp = NULL;
	int iFuncRet = -1;
	do {
		fp = zb_fopen(pcPath, "rb");
		if (!fp) {
			ESP_LOGE(TAG, "Fail to open file %s", pcPath);
			iFuncRet = -3;  break;
		}
		char acHeader[48] = { 0 };
		zb_fread(acHeader, 1, sizeof(acHeader), fp);
		int16_t itemTotal = *(int16_t *)(acHeader + 0x10);
		int16_t idxItemW =  *(int16_t *)(acHeader + 0x18);
		int16_t fileHeadLen = *(int16_t *)(acHeader + 0x1a);

		if (idx < 0 || idx >= itemTotal) {
			iFuncRet = -5;  break;
		}
		char acIdxItemTwo[8*2] = { 0 };
		zb_fseek(fp, fileHeadLen + idxItemW * idx, 0);
		zb_fread(acIdxItemTwo, idxItemW, 2, fp);
		int offset = *(int *)acIdxItemTwo;
		int offsetB = *(int *)(acIdxItemTwo + idxItemW);
		int imgLen = offsetB - offset;

		lv_img_dsc_t stImgDsc = { 0 };
		zb_fseek(fp, offset, 0);
		zb_fread(&stImgDsc, 1, 8, fp);
		LV_ASSERT(stImgDsc.data_size >= 1 && stImgDsc.data_size < 1024 * 400);
		void * imgBuf = heap_caps_malloc_dflt(stImgDsc.data_size);
		zb_fread(imgBuf, 1, stImgDsc.data_size, fp);
		stImgDsc.data = imgBuf;

        (*pImgDsc) = stImgDsc;
		iFuncRet = 0;
	} while (0);

	if (fp) {
		zb_fclose(fp);   fp = NULL;
	}
	if (iFuncRet < 0) {
		ESP_LOGW(TAG, "ret: %d", iFuncRet);
	}
	return iFuncRet;
}
#endif

//------------------------------------------------------
#define C_IMG_CACHE_TBL_LEN  30

typedef struct { 
	uint32_t  m_hashVal;
	lv_img_dsc_t * m_pImgDsc;
	uint16_t  m_imgDataLen;  // when size > 64Kb, use 0xffff
	int16_t  m_lastAccessNo; // start from  1 (0 means space item), less means old, greater means new
} img_cache_item_t;

#define M_IMGRES_INT16_INC(i) i++; \
	if (i <= 0) { \
		i = 1; \
	} 

//------------------------------------------------------
static zb_img_file_res_t * g_pImgRes = NULL; 

static img_cache_item_t * g_pImgCache = NULL;
static int16_t g_accessNoCur = 0;  // 1 to 32767, then round to C_IMG_CACHE_TBL_LEN + 1

//--------------------------------------

int16_t zb_img_res_get_img_total(void) { 
	if (!g_pImgRes) return 0;
	return g_pImgRes->m_imgTotal;
}

zb_img_file_res_t * zb_img_res_open(const char * pcPath)
{
	FILE * pFile = NULL;
	int iFuncRet = -1;
	int16_t itemTotal = 0;
	int16_t idxItemW = 0;
	int16_t fileHeadLen = 0;
	uint8_t  encParm = 0;

	ESP_LOGI(TAG, "imgOpen A");
	if (g_pImgRes && g_pImgRes->m_fp && g_pImgCache) { // img res already load.
		ESP_LOGW(TAG, "warn,imgRes reopen!");
		return g_pImgRes;
	}
	if (!g_pImgCache) {
		g_pImgCache = heap_caps_malloc_dflt(sizeof(img_cache_item_t) * C_IMG_CACHE_TBL_LEN);
		memset(g_pImgCache, 0, sizeof(img_cache_item_t) * C_IMG_CACHE_TBL_LEN);
		g_accessNoCur = 1;
	}
	do {
		pFile = zb_fopen(pcPath, "rb");
		if (!pFile) {
			iFuncRet = -3;  break;
		}

		// read fileHeader;
		char acHeader[32] = { 0 };
		zb_fread(acHeader, 1, 32, pFile);
		if (memcmp(acHeader, "IMAGE", 5) != 0) {
			iFuncRet = -5;  break;
		}
		
		itemTotal = GET_UINT16_LITTLEENDIAN(acHeader + 0x10);
		encParm   = GET_UINT16_LITTLEENDIAN(acHeader + 0x14);
		idxItemW = GET_UINT16_LITTLEENDIAN(acHeader + 0x18);
		fileHeadLen = GET_UINT16_LITTLEENDIAN(acHeader + 0x1a);

		if (itemTotal <= 0 || idxItemW < 4 || idxItemW > 8) {
			iFuncRet = -7;  break;
		}
		iFuncRet = 0;
	} while (0);

	if (iFuncRet >= 0) {
		zb_img_file_res_t * pImgFileRes = (zb_img_file_res_t *)heap_caps_malloc_dflt(sizeof(zb_img_file_res_t));
		memset(pImgFileRes, 0, sizeof(zb_img_file_res_t));
		pImgFileRes->m_fp = pFile;
		pImgFileRes->m_imgTotal = itemTotal;
		pImgFileRes->m_idxItemW = idxItemW;
		pImgFileRes->m_idxTblOffset = fileHeadLen + 4 * itemTotal;
		pImgFileRes->m_encParm = encParm;
		pImgFileRes->m_pHashIdxTbl = (uint32_t *)heap_caps_malloc_dflt(4 * itemTotal);
		
		int hashIdxTblOffset = fileHeadLen;
		memset(pImgFileRes->m_pHashIdxTbl, 0, 4 * itemTotal);
		zb_fseek(pFile, hashIdxTblOffset, 0);
		zb_fread(pImgFileRes->m_pHashIdxTbl, 1, 4 * itemTotal, pFile);
		if (!g_pImgRes) {
			g_pImgRes = pImgFileRes;
		}
		return pImgFileRes;
	}
	ESP_LOGE(TAG, "fail to res open, ret %d,%s",  iFuncRet, pcPath);
	if (pFile) {
		zb_fclose(pFile);
	}
	return NULL;
}

void zb_img_res_close(zb_img_file_res_t * img_res)
{ 
	if (-1 == (int)img_res) {  // -1 means release the only imgRes. 
		img_res = g_pImgRes;
	}
	if (!img_res) return;

	ESP_LOGI(TAG, "resCloseB1");
	if (img_res->m_fp) {
		ESP_LOGI(TAG, "resCloseB 2");
		zb_fclose(img_res->m_fp);   img_res->m_fp = NULL;
	}
	ESP_LOGI(TAG, "resCloseC");
	if (img_res->m_pHashIdxTbl) {
		heap_caps_free(img_res->m_pHashIdxTbl);
	}
	free(img_res);
	if (img_res == g_pImgRes) {
		g_pImgRes = NULL;
	}
	// sugaoyi 20240527 add, when close img res, reset imgCache. 
	if (g_pImgCache) {
		memset(g_pImgCache, 0, sizeof(img_cache_item_t) * C_IMG_CACHE_TBL_LEN);
	}
	g_accessNoCur = 1;

}

// 
static void _insert_item_to_cache(uint32_t imgHash, lv_img_dsc_t * pImgDsc)
{
	int16_t minPrior = 0;
	int16_t minPriorPos = -1;

	if (!pImgDsc) return;
	minPrior = g_pImgCache[0].m_lastAccessNo;
	minPriorPos = 0;
	for (int16_t idx = 1; idx < C_IMG_CACHE_TBL_LEN; idx++) {
		int16_t lastAccessNo = g_pImgCache[idx].m_lastAccessNo;
		if (minPrior > lastAccessNo) {
			minPrior = lastAccessNo;
			minPriorPos = idx;
		}
	}
	img_cache_item_t * pCacheItem = g_pImgCache + minPriorPos;
	lv_img_dsc_t * pImgDscOld = pCacheItem->m_pImgDsc;
	if (pImgDscOld) {
		heap_caps_free(pImgDscOld->data);
		heap_caps_free(pImgDscOld);  pCacheItem->m_pImgDsc = NULL;
	}
	pCacheItem->m_hashVal = imgHash;
	pCacheItem->m_pImgDsc = pImgDsc;
	pCacheItem->m_lastAccessNo = g_accessNoCur;
	M_IMGRES_INT16_INC(g_accessNoCur)
}

static int16_t _img_find_cache_pos(uint32_t imgHash)
{
	int16_t foundNo = -1;
	for (int16_t idx = 0; idx < C_IMG_CACHE_TBL_LEN; idx++) {
		if (imgHash == g_pImgCache[idx].m_hashVal) {
			foundNo = idx;  break;
		}
	}
	if ((foundNo >= 0) && !g_pImgCache[foundNo].m_pImgDsc) {
		return -3;
	}
	return foundNo;
}

static int16_t _img_find_by_hash(zb_img_file_res_t * img_res, uint32_t imgHash) {
	int16_t iFuncRet = -1;
	int16_t iL = 0, iH;
	iH = img_res->m_imgTotal - 1;
	
	uint32_t * pHashIdxTbl = img_res->m_pHashIdxTbl;
	do {
		if (imgHash < pHashIdxTbl[iL]) {
			iFuncRet = -3;	break;
		}
		if (imgHash == pHashIdxTbl[iL]) {
			iFuncRet = 0;  break;
		}
		if (imgHash > pHashIdxTbl[iH]) {
			iFuncRet = -5;	break;
		}
		if (imgHash == pHashIdxTbl[iH]) {
			iFuncRet = iH;  break;
		}
		while (iL + 1 < iH) {
			int16_t mid = (iL + iH) / 2;
			if (pHashIdxTbl[mid] < imgHash) {
				iL = mid;
			}
			else if (pHashIdxTbl[mid] > imgHash) {
				iH = mid;
			}
			else {
				iFuncRet = mid;  break;
			}
		}
	}while (0);
	if (iFuncRet < 0) {
		ESP_LOGW(TAG, "fail to find hash %x, %hd", (int)imgHash, iFuncRet);
	}
	return iFuncRet;
}


/**
 * (从 zb_img_res 文件)读取一张图的的数据到内存 img_dsc struct; 
 * @param extra  0 - auto free;  1 - free manual;
 * @return not 0  means success. 
 */
lv_img_dsc_t * zb_img_res_load_one(zb_img_file_res_t * img_res, uint32_t imgHash, 
		img_free_type_e img_free_type, int * pErrNo)
{
	lv_img_dsc_t * pImgDsc = NULL;
	int16_t idx = 0;
	int16_t iFuncRet = -1;
	if (!img_res) {
		img_res = g_pImgRes;
        if (!img_res) return NULL;
	}
	do {
		if (IMG_FREE_TYPE_AUTO == img_free_type) {
			int16_t imgCacheNo = _img_find_cache_pos(imgHash);
			if (imgCacheNo >= 0) {  // img 缓存命中了 
				pImgDsc = g_pImgCache[imgCacheNo].m_pImgDsc;
				g_pImgCache[imgCacheNo].m_lastAccessNo = g_accessNoCur;
				M_IMGRES_INT16_INC(g_accessNoCur)
				iFuncRet = 0;   break;
			}
		}
	
		idx = _img_find_by_hash(img_res, imgHash);
		if (idx < 0 || idx >= img_res->m_imgTotal) {
			iFuncRet = -3;  break;
		}
		char acIdxItemTwo[8*2] = { 0 };
		zb_fseek(img_res->m_fp, img_res->m_idxTblOffset + img_res->m_idxItemW * idx, 0);
		zb_fread(acIdxItemTwo, 1, img_res->m_idxItemW * 2, img_res->m_fp);
		int offsetTwo[2];  // 
		// 
		offsetTwo[0] = GET_UINT32_LITTLEENDIAN(acIdxItemTwo);
		// offsetTwo[1] = GET_UINT32_LITTLEENDIAN(acIdxItemTwo + img_res->m_idxItemW);

		lv_img_dsc_t stImgDsc = { 0 };
		zb_fseek(img_res->m_fp, offsetTwo[0], 0);
		zb_fread(&stImgDsc, 1, 8, img_res->m_fp);
		LV_ASSERT(stImgDsc.data_size >= 1 && stImgDsc.data_size < 1024 * 500);
		void * imgBuf = heap_caps_malloc_dflt(stImgDsc.data_size); // 
		if (!imgBuf) {
			iFuncRet = -7;  break;
		}
		zb_fread(imgBuf, 1, stImgDsc.data_size, img_res->m_fp);
		stImgDsc.data = imgBuf;

		pImgDsc = heap_caps_malloc_dflt(sizeof(lv_img_dsc_t));
		(*pImgDsc) = stImgDsc;
		if (IMG_FREE_TYPE_MANUAL != img_free_type) {
			_insert_item_to_cache(imgHash, pImgDsc);
		}
		iFuncRet = 0;
	} while (0);

	if (iFuncRet < 0) { 
		ESP_LOGE(TAG, "fail to res load, ret %d",  iFuncRet);
		if (pImgDsc) {
			heap_caps_free(pImgDsc);   pImgDsc = NULL;
		}
	}
	if (pErrNo) {
		(*pErrNo) = iFuncRet;
	}
	return pImgDsc;
	
}

void zb_img_res_free_one(lv_img_dsc_t * pImgDsc)
{
	if (!pImgDsc) return;
#ifdef C_PC_SIM
	// ensure it is not in imgCacheTbl;
	int16_t idx = 0;
	for (; idx < C_IMG_CACHE_TBL_LEN; idx++) {
		if (g_pImgCache[idx].m_pImgDsc) {
			LV_ASSERT(pImgDsc != g_pImgCache[idx].m_pImgDsc);
		}
	}
#endif
	heap_caps_free(pImgDsc->data);  pImgDsc->data = NULL;
	heap_caps_free(pImgDsc);
}



static lv_ll_t* g_p_img_cover_ll = NULL, g_img_cover_ll;


inline void _imgDst_unload(lv_img_dsc_t * imgDsc)
{
	if (!imgDsc)
		return;

	heap_caps_free(imgDsc->data);  imgDsc->data = NULL;
	heap_caps_free(imgDsc);
	return;
}


static lv_img_dsc_t* _traverse_img_cover(uint32_t crc)
{	
	lv_img_dsc_t* pResult = NULL;
	zb_img_cover_res_t* pHead = (zb_img_cover_res_t*)_lv_ll_get_head(g_p_img_cover_ll);
	while (pHead) {
		ESP_LOGI(TAG, "该本书的详细版本是：%p", pHead->img_dsc);
		if (pHead->crc == crc)
		{
			pResult = pHead->img_dsc;
			break;
		}
		pHead = _lv_ll_get_next(g_p_img_cover_ll, pHead);
	}
	return pResult;
}

extern lv_img_dsc_t* LSNSPK_get_cover_data(const char* pPathUnit, int* pErrNo);

lv_img_dsc_t* zb_img_cover_load_one(const char* mnu_path, int* pErrNo)
{
	uint32_t crc = esp_rom_crc32_le(0xee, (uint8_t const *)mnu_path, strlen(mnu_path));
	if (g_p_img_cover_ll == NULL) {
		g_p_img_cover_ll = &g_img_cover_ll;
		_lv_ll_init(g_p_img_cover_ll, sizeof(zb_img_cover_res_t));
	}
	zb_img_cover_res_t* pResult = NULL;
	pResult = _traverse_img_cover(crc);
	if (pResult != NULL) {
		return pResult;
	}
	lv_img_dsc_t* pimg = LSNSPK_get_cover_data(mnu_path, NULL);
	if (pimg == NULL) {
		if (pErrNo)
			*pErrNo = -1;//不存在封面圖片資源
		return pResult;
	}
	zb_img_cover_res_t* new_node = (zb_img_cover_res_t*)_lv_ll_ins_tail(g_p_img_cover_ll);
	new_node->crc = crc;
	new_node->img_dsc = pimg;
	return pimg;
}

static void _traverse_img_cover_free(lv_ll_t *p_lv_ll)
{
	zb_img_cover_res_t* pHead = (zb_img_cover_res_t*)_lv_ll_get_head(p_lv_ll);
	while (pHead) {
		_imgDst_unload(pHead->img_dsc);
		pHead->img_dsc = NULL;
		pHead = _lv_ll_get_next(p_lv_ll, pHead);
	}
}

void zb_img_cover_unload()
{
	_traverse_img_cover_free(g_p_img_cover_ll);
	_lv_ll_clear(g_p_img_cover_ll);
	g_p_img_cover_ll = NULL;
}

/*
* 从单独的bin数据读取图片数据，
* 注意需要自己去释放资源
*/
lv_img_dsc_t* zb_load_image_from_file(char* pcPath) {
	lv_img_dsc_t* stImgDsc = NULL;
	FILE* fp = NULL;
	do {
		fp = zb_fopen(pcPath, "rb");
		if (!fp) {
			ESP_LOGE(TAG, "Fail to open file %s", pcPath);
			return stImgDsc;
		}

		stImgDsc = heap_caps_malloc_dflt(sizeof(lv_img_dsc_t));
		zb_fread(stImgDsc, 1, 8, fp);
		LV_ASSERT(stImgDsc->data_size >= 1 && stImgDsc->data_size < 1024 * 400);
		void* imgBuf = heap_caps_malloc_dflt(stImgDsc->data_size);
		zb_fread(imgBuf, 1, stImgDsc->data_size, fp);
		stImgDsc->data = imgBuf;
	} while (0);

	if (fp) {
		zb_fclose(fp);   fp = NULL;
	}
	return stImgDsc;
}