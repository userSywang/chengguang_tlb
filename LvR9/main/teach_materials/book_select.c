#include "book_select.h"
#include "../utils/misc_utils.h"
#include "../data_parse/cJSON.h"

#define TAG "BOOK_SELECT"

#ifndef C_PC_SIM
#define BOOK_JSON "/oem/_data/textbook/s_s.json"
#else
#define BOOK_JSON "/userdata/_data/textbook/s_s.json"
#endif


bk_node* g_bkHead = NULL;    // 保存教材列表数据

//char** g_pversion = NULL;    // 保存教材版本
char g_pversion[MAX_VERSION_NUM][120];    // 保存教材版本
char g_version_num = 0;
book_info_t *gp_cur_book;

char* get_version_by_id(char pos) {
    return g_pversion[pos];
}

uint16_t get_version_sum()
{
    return g_version_num;
}

int insert_books_node(char *grade, char* name, char* cover_url, char* book_url) {
    bk_node* new_node = (bk_node*)heap_caps_malloc_dflt(sizeof(bk_node));
    if (new_node != NULL)
    {
        strcpy(new_node->data.name, name);
        strcpy(new_node->data.grade, grade);
        strcpy(new_node->data.cover_url, cover_url);
        strcpy(new_node->data.book_url, book_url);
        new_node->next = NULL;
    }
    else {
        ESP_LOGI(TAG, "插入节点失败：%s， %s . [%d]", name, book_url, __LINE__);
        return -1;
    }

    ESP_LOGI(TAG, "插入节点：%s， %s . [%d]", name, book_url, __LINE__);

    if (g_bkHead == NULL) {
        g_bkHead = new_node;
    }
    else {
        bk_node* temp = g_bkHead;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_node;
    }
    return 0;
}


void book_node_free(void) {
    bk_node* pNode = g_bkHead;
    if (pNode) {
        while (pNode != NULL) {
            bk_node* pNodeNx = pNode->next;
            heap_caps_free(pNode);
            pNode = pNodeNx;
        }
        g_bkHead = NULL;
    }
    return;
}


void book_versions_free() {
    /*if (g_pversion != NULL) {
        for (int i = 0; i < g_version_num; i++) {
            heap_caps_free(g_pversion[i]);
            g_pversion[i] = NULL;
        }
        heap_caps_free(g_pversion);
        g_version_num = 0;
        g_pversion = NULL;
    }*/
    g_version_num = 0;
}

void check_memory() {
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);

    ESP_LOGE(TAG, "----**---Used: %d (%d%%), Frag: %d%%, Biggest free: %d\n",
        mon.total_size - mon.free_size,
        mon.used_pct,
        mon.frag_pct,
        mon.free_biggest_size);
}

#define MNU_JSON_FILE_LEN_MAX  80000 
int read_versions(char* subject, char* stage2grade)
{  
    int funcErrNo = -1;  // >= 0 means success 
    FILE* fp;
    cJSON* root = NULL;
    int16_t iTotal = 0;
    check_memory();
    book_versions_free();
    //g_pversion = (char**)heap_caps_malloc_dflt(MAX_VERSION_NUM * sizeof(char*));

    fp = zb_fopen(BOOK_JSON, "rb");
    if (!fp) {
        ESP_LOGE(TAG, "Fail to openF %s", BOOK_JSON);
        funcErrNo = -3;
    }
    char* acBuf = (char*)heap_caps_malloc_dflt(MNU_JSON_FILE_LEN_MAX);
    int len = zb_fread(acBuf, 1, MNU_JSON_FILE_LEN_MAX - 1, fp);
    if (len >= 0) {
        acBuf[len] = '\0';
    }

    root = cJSON_Parse(acBuf);
    if (root == NULL) {
        iTotal = cJSON_GetErrorPtr();
        heap_caps_free(acBuf);
        cJSON_Delete(root);
        funcErrNo = -5;
    }

    cJSON *sgJson = cJSON_GetObjectItem(cJSON_GetObjectItem(root, subject), stage2grade);
    int16_t sum = cJSON_GetArraySize(sgJson);
    cJSON* item = NULL;
    for (int i = 0; i < sum; i++) {
        item = cJSON_GetArrayItem(sgJson, i);
        cJSON* name_item = cJSON_GetObjectItemCaseSensitive(item, "name");
        uint16_t len = strlen(name_item->valuestring);
        /*char* p = (char*)heap_caps_malloc_dflt(len+1);
        strcpy(p, name_item->valuestring);
        g_pversion[g_version_num] = p;*/
        strcpy(g_pversion[g_version_num], name_item->valuestring);
        g_pversion[g_version_num][len] = '\0';
        g_version_num++;
    }
    if (root) {
        cJSON_Delete(root);
        heap_caps_free(acBuf);
        root = NULL;
    }
    return sum;
}

int read_books(char* subject, char* stage2grade, char* version)
{
    int funcErrNo = -1;  // >= 0 means success 
    FILE* fp;
    cJSON* root = NULL;
    int16_t iTotal = 0;

    fp = zb_fopen(BOOK_JSON, "rb");
    if (!fp) {
        ESP_LOGE(TAG, "Fail to openF %s", BOOK_JSON);
        funcErrNo = -3;
    }
    char* acBuf = (char*)heap_caps_malloc_dflt(MNU_JSON_FILE_LEN_MAX);
    if (acBuf == NULL) {
        ESP_LOGE(TAG, "Fail to malloc %d", MNU_JSON_FILE_LEN_MAX);
        funcErrNo = -4;
    }


    int len = zb_fread(acBuf, 1, MNU_JSON_FILE_LEN_MAX - 1, fp);
    if (len >= 0) {
        acBuf[len] = '\0';
    }

    root = cJSON_Parse(acBuf);
    if (root == NULL) {
        iTotal = cJSON_GetErrorPtr();
        ESP_LOGE(TAG, "parse json error");
        funcErrNo = -5;
    }

    if (funcErrNo != -1)
    {
        return funcErrNo;
    }

    cJSON* sgJson = cJSON_GetObjectItem(cJSON_GetObjectItem(root, subject), stage2grade);
    int16_t sum = cJSON_GetArraySize(sgJson);
    cJSON* item = NULL;
    int16_t bookSum = 0;
    for (int i = 0; i < sum; i++) {
        item = cJSON_GetArrayItem(sgJson, i);
        cJSON* vname_item = cJSON_GetObjectItemCaseSensitive(item, "name");
        uint16_t len = strlen(vname_item->valuestring);
        if (strcmp(vname_item->valuestring, version) == 0) {
            cJSON* bookJson = cJSON_GetObjectItem(item, "books");
            bookSum = cJSON_GetArraySize(bookJson);
            for (int k = 0; k < bookSum; k++) {
                cJSON* bookItem = cJSON_GetArrayItem(bookJson, k);
                //"grade": "一年级",
                //    "name" : "教材全解北京版一年级上册",
                //    "icon" : "cover/bjb_1_1",
                //    "book" : "boll/bjb/bjb_1_1"
                cJSON* gradeItem = cJSON_GetObjectItem(bookItem, "grade");
                cJSON* bnameItem = cJSON_GetObjectItem(bookItem, "name");
                cJSON* iconUrlItem = cJSON_GetObjectItem(bookItem, "icon");
                cJSON* bookUrlItem = cJSON_GetObjectItem(bookItem, "book");

                insert_books_node(gradeItem->valuestring, bnameItem->valuestring, iconUrlItem->valuestring, bookUrlItem->valuestring);
            }
        }

    }
    if (root) {
        heap_caps_free(acBuf);
        acBuf = NULL;
        cJSON_Delete(root);  
        root = NULL;
    }

    return bookSum;
}

book_info_t* get_booknode_by_bookbin(const char* book_url) {
    bk_node* pNode = g_bkHead;
    gp_cur_book = NULL;
    if (pNode) {
        while (pNode != NULL) {
            if (strcmp(pNode->data.book_url, book_url) == 0) {
                gp_cur_book = &pNode->data;
                break;
            }
            pNode = pNode->next;
        }
    }
    return gp_cur_book;
}


char* get_cur_book_name() {
    if (gp_cur_book != NULL)
        return gp_cur_book->name;
    return "";
}
lv_img_dsc_t* get_cur_book_img_dsc() {
    if (gp_cur_book != NULL)
        return gp_cur_book->pimg_dsc;
    return NULL;
}
