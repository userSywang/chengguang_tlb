
#include "../data_parse/analyze_listening.h"
#include "../utils/misc_utils.h"
#include <lvgl/lvgl.h>


#define TAG "explLsn"


int example_lsnspk(lv_img_dsc_t ** ppImgDscAddr) {
#define TEST_IMG_NO  5

	struct LSNSPK_unit * pUnitInf = NULL;
	const char unit_bin_path[] = "/sdcard/english/Listening/grade1a/unit01.bin";  // unit01.bin";

	int errNo = 0;
	pUnitInf = LSNSPK_unit_open(unit_bin_path, &errNo);
	if (!pUnitInf) {
		ESP_LOGE(TAG, "fail %d, to load %s", errNo, unit_bin_path);
		return -1;
	}

	for (int16_t idx = 0; idx < ((LSNSPK_unit_t *)pUnitInf)->word_num; idx++) { 
		const char * pOrginal = NULL, * pPhn = NULL, * pTrans = NULL;
		pOrginal = LSNSPK_get_word_field(pUnitInf, idx, LSNSPK_COL_ORIGINAL);
		pPhn     = LSNSPK_get_word_field(pUnitInf, idx, LSNSPK_COL_PHONETIC);
		pTrans   = LSNSPK_get_word_field(pUnitInf, idx, LSNSPK_COL_TRANSLATION);
		ESP_LOGI(TAG, "wd[%2d] %s /%s/ %s", idx, pOrginal, pPhn, pTrans);
	}

	// 读取 单词音频 
	{
#define C_AUD_BUF_LEN  (1024*80)
		void * pAudBuf = heap_caps_malloc_dflt(C_AUD_BUF_LEN);
		for (int idx = 0; idx < ((LSNSPK_unit_t *)pUnitInf)->word_num ; idx++) {
			int32_t audOffset = 0;
			int audLen = LSNSPK_read_word_aud_location(pUnitInf, idx, &audOffset);
			ESP_LOGI(TAG, "wdAud No%2d at 0x%x len %d", idx, (int)audOffset, audLen);

		#if 0
			int iRet = LSNSPK_read_word_aud(pUnitInf, idx, pAudBuf, C_AUD_BUF_LEN);
			if (iRet <= 0) {
				ESP_LOGW(TAG, "Fail %d, to read aud No%d !", iRet, idx);
			} else {
				ESP_LOGI(TAG, "wdAud No%2d len %d", idx, iRet);
			}
		#endif
		}
        heap_caps_free(pAudBuf);  pAudBuf = NULL;
	}

	{ // 读取单词图片
#define LV_IMG_DSC_LEN  12
		for (int idx = 0; idx < ((LSNSPK_unit_t *)pUnitInf)->word_num ; idx++) {
			lv_point_t imgSize = { 0 };

			int iRet = LSNSPK_read_word_img_info(pUnitInf, idx, NULL, &imgSize);
			ESP_LOGI(TAG, "wdImgNo%2d len %d, (w %hd, h %hd)", idx, iRet, imgSize.x, imgSize.y);

			if (iRet > 0) {
				lv_img_dsc_t  * pImgDsc = NULL; // 
				iRet = LSNSPK_read_word_img_info(pUnitInf, idx, &pImgDsc, &imgSize);
				if (iRet <= 0) {
					ESP_LOGW(TAG, "Fail %d, to read img No%d !", iRet, idx);
				} else {
					int imgLen = iRet;
					ESP_LOGI(TAG, "wdImg No%2d len %d", idx, imgLen);
					if (ppImgDscAddr && (TEST_IMG_NO == idx)) {
						(*ppImgDscAddr) = pImgDsc; 
					}
				}
			}
		}
	}


	// 读取课文
	int32_t audOffset = 0;
	int lessonAudLen = LSNSPK_get_lesson_aud_location(pUnitInf, &audOffset);
	ESP_LOGI(TAG, "lesson aud at 0x%x, len %d", (int)audOffset, lessonAudLen);
	for (int16_t idx = 0; idx < ((LSNSPK_unit_t *)pUnitInf)->sentence_n; idx++) {
		const char * pOrginal = NULL, * pTrans = NULL;
		pOrginal = LSNSPK_get_lesson_sentence(pUnitInf, idx, LSNSPK_COL_ORIGINAL);
		pTrans = LSNSPK_get_lesson_sentence(pUnitInf, idx, LSNSPK_COL_TRANSLATION);
		int ptInTm = LSNSPK_get_lesson_sent_ptInTime(pUnitInf, idx);
		ESP_LOGI(TAG, "sentc[%2d] %dms,%s / %s", idx, ptInTm, pOrginal, pTrans);
	}
	int idx = ((LSNSPK_unit_t*)pUnitInf)->sentence_n;
	int ptInTm = LSNSPK_get_lesson_sent_ptInTime(pUnitInf, idx);
	ESP_LOGI(TAG, "sentc[%2d] %dms", idx, ptInTm);

	//-----------
	LSNSPK_unit_close(pUnitInf);

	//--------------------------------------
	const char unit_book_path[] = "/sdcard/english/Listening/Grade5A/album.mnu";
	struct LSNSPK_book * pBookInf = NULL;
	do {
		pBookInf = LSNSPK_book_open(unit_book_path, NULL);
		if (!pBookInf) {
			ESP_LOGE(TAG, "fait to open book %s", unit_book_path);
			break;
		}
		for (int lessonNo = 0; lessonNo < ((LSNSPK_book_t *)pBookInf)->lesson_n; lessonNo++) {
			const char * pTitle = LSNSPK_book_get_item_filed(pBookInf, lessonNo, 0);
			const char * pFileName = LSNSPK_book_get_item_filed(pBookInf, lessonNo, 1);
			ESP_LOGI(TAG, "lesson No%2d, <%s> %s", lessonNo, pFileName, pTitle);
		}
	} while (0);

	if (!pBookInf) {
		LSNSPK_book_close(pBookInf);   pBookInf = NULL;
	}

	return 0;
}


