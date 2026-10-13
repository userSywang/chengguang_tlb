#include "player_data_parse.h"

#define TAG "explLsn"

/**
 * @brief 释放菜单文件的资源
 */
void free_player_book_info_t(player_book_info_t *pmit)
{
    if (pmit->pTile)
    {
        heap_caps_free(pmit->pTile);
        pmit->pTile = NULL;
    }
    if (pmit->pFileName)
    {
        heap_caps_free(pmit->pFileName);
        pmit->pFileName = NULL;
    }
    if (pmit->pBookInf)
    {
        LSNSPK_book_close(pmit->pBookInf);
        pmit->pBookInf = NULL;
    }
}

int player_get_book_inf(char *book_path, player_book_info_t *pmit)
{
    int ret = 0;
    if (!book_path)
    {
        return -1;
        ESP_LOGE(TAG, "book_path is null\n");
    }
    do
    {
        int errNo = 0;
        pmit->pBookInf = LSNSPK_book_open(book_path, &errNo);
        if (!pmit->pBookInf || errNo < 0)
        {
            ESP_LOGE(TAG, "errNo:%d fail to open book %s\n", (int)errNo, book_path);
            ret = -1;
            break;
        }
        pmit->cnt = ((LSNSPK_book_t *)pmit->pBookInf)->lesson_n;
        pmit->pTile = (char **)heap_caps_malloc_dflt(((LSNSPK_book_t *)pmit->pBookInf)->lesson_n * sizeof(char *));
        pmit->pFileName = (char **)heap_caps_malloc_dflt(((LSNSPK_book_t *)pmit->pBookInf)->lesson_n * sizeof(char *));
        if (pmit->pTile == NULL || pmit->pFileName == NULL)
        {
            free_player_book_info_t(pmit);
            ret = -2;
            break;
        }

        for (int lessonNo = 0; lessonNo < ((LSNSPK_book_t *)pmit->pBookInf)->lesson_n; lessonNo++)
        {
            pmit->pTile[lessonNo] = LSNSPK_book_get_item_filed(pmit->pBookInf, lessonNo, 0);
            pmit->pFileName[lessonNo] = LSNSPK_book_get_item_filed(pmit->pBookInf, lessonNo, 1);
            // ESP_LOGI(TAG, "lesson No%2d, <%s> %s", (int)lessonNo, pmit->pFileName[lessonNo], pmit->pTile[lessonNo]);
        }
    } while (0);
    return ret;
}

void free_player_unit_info_t(player_unit_info_t *pst)
{
    if (pst->pOrginal)
    {
        // sugy 240807 for: 把课文句子的 txtBuff 改为独立申请 
        if (pst->doesUseExtraSentence) {
            int sentenceNum = ((LSNSPK_unit_t *)pst->pUnitInf)->sentence_n;
            for (int idx = 0; idx < sentenceNum; idx++) {
                if (pst->pOrginal[idx]) {
                    heap_caps_free(pst->pOrginal[idx]);
                }
            }
        }
        heap_caps_free(pst->pOrginal);
        pst->pOrginal = NULL;
    }
    LV_LOG_USER(" pst->ptInTm: %p", pst->ptInTm);
    if (pst->ptInTm)
    {
        heap_caps_free(pst->ptInTm);
        pst->ptInTm = NULL;
    }
    if (pst->pTranslation)
    {
        heap_caps_free(pst->pTranslation);
        pst->pTranslation = NULL;
    }
    if (pst->pUnitInf)
    {
        LSNSPK_unit_close(pst->pUnitInf);
        pst->pUnitInf = NULL;
    }
}

int player_get_unit_info(char *path, player_unit_info_t *pst)
{
//#define UNIT_SENTENCE_NUM_MAX  200
    uint16_t sentence_cnt = 0;
    int errNo = 0;
    ESP_LOGI(TAG, "%s A", __func__);
    pst->pUnitInf = LSNSPK_unit_open(path, &errNo);
    if (!pst->pUnitInf || errNo < 0)
    {
        ESP_LOGE(TAG, "fail %d, to load %s\n", errNo, path);
        return -1;
    }
    pst->fp = ((LSNSPK_unit_t *)pst->pUnitInf)->fpUnit;
    pst->encParm = ((LSNSPK_unit_t *)pst->pUnitInf)->encParm;
    sentence_cnt = ((LSNSPK_unit_t *)pst->pUnitInf)->sentence_n;
    /* if (sentence_cnt > UNIT_SENTENCE_NUM_MAX) // non-std 
    {
        ESP_LOGW(TAG, "too much sentence %hd, modf to %d", sentence_cnt, UNIT_SENTENCE_NUM_MAX);
        sentence_cnt = UNIT_SENTENCE_NUM_MAX;
        ((LSNSPK_unit_t *)pst->pUnitInf)->sentence_n  = sentence_cnt;
    } */
		
    pst->cnt = sentence_cnt;
    pst->audLen = LSNSPK_get_lesson_aud_location(pst->pUnitInf, &pst->audOffset);
    if (sentence_cnt)
    {
        pst->pOrginal = (char **)heap_caps_malloc_dflt(sizeof(char *) * sentence_cnt);
        pst->pTranslation = (char **)heap_caps_malloc_dflt(sizeof(char *) * sentence_cnt);
        pst->ptInTm = (int32_t *)heap_caps_malloc_dflt(sizeof(int) * (sentence_cnt + 1));
        if (!(pst->pOrginal && pst->pTranslation && pst->ptInTm))
        {
            LSNSPK_unit_close(pst->pUnitInf);
            free_player_unit_info_t(pst);
            ESP_LOGE(TAG, "heap_caps_malloc_dflt fail\n");
            return -2;
        }
        // ESP_LOGI(TAG, "lesson aud at %d, len %d", (int)pst->audOffset, (int)pst->audLen);
        uint16_t idx;
        /* const char * pTrans = LSNSPK_get_lesson_sentence(pst->pUnitInf, 0, LSNSPK_COL_TRANSLATION);
        if (pTrans && pTrans[0]) {
            pst->doesUseExtraSentence = 1;
        } */
        LSNSPK_get_lesson_sentence(pst->pUnitInf, 0, LSNSPK_COL_TRANSLATION); // ensure has sentenceInfo.
        pst->doesUseExtraSentence = ((LSNSPK_unit_t *)pst->pUnitInf)->hasTrans;
        ESP_LOGI(TAG, "useExtraSent:%hd, sentence_n: %d\n", (short)pst->doesUseExtraSentence,  ((LSNSPK_unit_t *)pst->pUnitInf)->sentence_n);

        for (idx = 0; idx < ((LSNSPK_unit_t *)pst->pUnitInf)->sentence_n; idx++)
        {
            const char * pOrg = LSNSPK_get_lesson_sentence(pst->pUnitInf, idx, LSNSPK_COL_ORIGINAL);         
            const char * pTrans = LSNSPK_get_lesson_sentence(pst->pUnitInf, idx, LSNSPK_COL_TRANSLATION);
            if (pst->doesUseExtraSentence)  // sugy 240807 for: 把课文句子的 txtBuff 改为独立申请 
            {
                if (!(pTrans && pTrans[0]))
                {
                    pTrans = "";
                }
                
                if (!pOrg || pOrg[0] == '\0') continue;  //wsy 250701  目前发现句子可能会有空值，需要做处理

                int textBufLen = strlen(pOrg) + 1 + strlen(pTrans) + 2;
                const char * pWholeText = heap_caps_malloc_dflt(textBufLen);
                snprintf(pWholeText, textBufLen, "%s\n%s\n", pOrg, pTrans);
                pst->pOrginal[idx]     = pWholeText;
                pst->pTranslation[idx] = NULL;
            }
            else
            {
                if (!pOrg || pOrg[0] == '\0') continue;   //wsy 250701  目前发现句子可能会有空值，需要做处理

                pst->pOrginal[idx]     = (char *)pOrg;
                pst->pTranslation[idx] = (char *)pTrans;
            }
            pst->ptInTm[idx] = LSNSPK_get_lesson_sent_ptInTime(pst->pUnitInf, idx);

#if 0  //打印数据内容
			if((pst->pTranslation[idx] != NULL) && (pst->pOrginal[idx] != NULL))
			{
				ESP_LOGI(TAG, "S [%2d] %dms  %s / %s", (int)idx, (int)pst->ptInTm[idx], pst->pOrginal[idx], pst->pTranslation[idx]);
				
			}
			else if(pst->pOrginal[idx] != NULL)
			{
				ESP_LOGI(TAG, "S [%2d] %dms  %s ", (int)idx, (int)pst->ptInTm[idx], pst->pOrginal[idx]);
			}
#endif
        }
        ESP_LOGI(TAG, "--------------A-----------------\n");
        pst->ptInTm[idx] = LSNSPK_get_lesson_sent_ptInTime(pst->pUnitInf, idx);
    }
	LV_LOG("%s [%d]  \n", __FUNCTION__,__LINE__);
    return 0;
}



//尝试获取单个音频数据 ---wsy 06.24
int update_song_info_in_book_info(player_book_info_t* pbook, int index, char* book_path) {
    
    if (!pbook || !book_path || index < 0) {
        ESP_LOGE(TAG, "Invalid parameters");
        return -1;
    }

    // 打开专辑文件获取最新信息
    int errNo = 0;
    LSNSPK_book_t* pBook = LSNSPK_book_open(book_path, &errNo);
    if (!pBook || errNo < 0) {
        ESP_LOGE(TAG, "Failed to open book: %s, error: %d", book_path, errNo);
        return -2;
    }

    // 验证索引有效性
    if (index >= pBook->lesson_n) {
        ESP_LOGE(TAG, "Index %d out of range (max: %d)", index, pBook->lesson_n);
        LSNSPK_book_close(pBook);
        return -3;
    }

    // 动态调整内存大小
    if (pBook->lesson_n != pbook->cnt) {
   
        char** newTitles = heap_caps_realloc_dflt(pbook->pTile, pBook->lesson_n * sizeof(char*));
        char** newFiles = heap_caps_realloc_dflt(pbook->pFileName, pBook->lesson_n * sizeof(char*));

        if (!newTitles || !newFiles) {
            ESP_LOGE(TAG, "Memory realloc failed");
            LSNSPK_book_close(pBook);
            return -4;
        }

        
        for (int i = pbook->cnt; i < pBook->lesson_n; i++) {
            newTitles[i] = NULL;
            newFiles[i] = NULL;
        }

        pbook->pTile = newTitles;
        pbook->pFileName = newFiles;
        pbook->cnt = pBook->lesson_n; // 更新歌曲总数
    }

    // 释放旧数据并更新指定歌曲
    if (pbook->pTile[index]) heap_caps_free(pbook->pTile[index]);
    if (pbook->pFileName[index]) heap_caps_free(pbook->pFileName[index]);

    pbook->pTile[index] = strdup(LSNSPK_book_get_item_filed(pBook, index, 0));
    pbook->pFileName[index] = strdup(LSNSPK_book_get_item_filed(pBook, index, 1));

    LSNSPK_book_close(pBook);

    // 检查是否复制成功
    if (!pbook->pTile[index] || !pbook->pFileName[index]) {
        ESP_LOGE(TAG, "String duplication failed");
        return -5;
    }

    ESP_LOGI(TAG, "Updated song %d: %s -> %s",
        index, pbook->pTile[index], pbook->pFileName[index]);
    return 0;
}









int lrc_parse_file(const char* path, player_unit_info_t* pst) {
    T_AUD_LYRIC audLyric = { 0 };
#define LRC_BUFFER_SIZE 512

 
    int ret = Aud_LyrOpen(&audLyric, path, 0);
    if (ret != 0) return -1;

   
    pst->cnt = audLyric.lyrIdx.cntIdx;
    if (pst->cnt == 0) {
        Aud_LyrClose(&audLyric);
        return -2;
    }


    pst->ptInTm = (int32_t*)heap_caps_malloc_dflt(sizeof(int) * (pst->cnt + 1));
    pst->pOrginal = (char**)heap_caps_malloc_dflt(sizeof(char*) * pst->cnt);
    pst->pTranslation = (char**)heap_caps_malloc_dflt(sizeof(char*) * pst->cnt);

   
    char lineBuf[LRC_BUFFER_SIZE];
    int actualSentenceCount = 0;

    for (int i = 0; i < pst->cnt; i++) {
       
        pst->ptInTm[actualSentenceCount] = audLyric.lyrIdx.idxTbl[i][0];

      
        int16_t lineLen = LRC_BUFFER_SIZE - 1;
        if (Aud_LyrGetByNo(&audLyric, i, lineBuf, &lineLen) != 0) {
            continue;  
        }
        lineBuf[lineLen] = '\0'; 

      
        if (i + 1 < pst->cnt &&
            audLyric.lyrIdx.idxTbl[i][0] == audLyric.lyrIdx.idxTbl[i + 1][0]) {

      
            pst->pOrginal[actualSentenceCount] = (char*)heap_caps_malloc_dflt(lineLen + 1);
            strcpy(pst->pOrginal[actualSentenceCount], lineBuf);

           
            lineLen = LRC_BUFFER_SIZE - 1;
            Aud_LyrGetByNo(&audLyric, i + 1, lineBuf, &lineLen);
            lineBuf[lineLen] = '\0';

            pst->pTranslation[actualSentenceCount] = (char*)heap_caps_malloc_dflt(lineLen + 1);
            strcpy(pst->pTranslation[actualSentenceCount], lineBuf);

            i++;  
        }
        else {
            // 单语歌词
            pst->pOrginal[actualSentenceCount] = (char*)heap_caps_malloc_dflt(lineLen + 1);
            strcpy(pst->pOrginal[actualSentenceCount], lineBuf);
            pst->pTranslation[actualSentenceCount] = NULL;  
        }

        actualSentenceCount++;
    }

    
    pst->cnt = actualSentenceCount;
    pst->ptInTm[pst->cnt] = Aud_LyrGetLyrMaxTime(&audLyric);  

   
    Aud_LyrClose(&audLyric);
    return 0;
}


void lrc_unit_free(player_unit_info_t* pst) {
    if (!pst) return;

    for (int i = 0; i < pst->cnt; i++) {
        if (pst->pOrginal[i]) heap_caps_free(pst->pOrginal[i]);
        if (pst->pTranslation[i]) heap_caps_free(pst->pTranslation[i]);
    }

    if (pst->ptInTm) heap_caps_free(pst->ptInTm);
    if (pst->pOrginal) heap_caps_free(pst->pOrginal);
    if (pst->pTranslation) heap_caps_free(pst->pTranslation);
}