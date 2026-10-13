#include "yd_book_select.h"
#include "../utils/misc_utils.h"
#include "../data_parse/cJSON.h"
#include "../zb_base.h"
#include "../utils/encode_utils.h"
#include <time.h>

#define TAG "YD_BOOK_SELECT"

#define CHUNK_SIZE (2048)  // 每次读取分文件的大小 (字节)

#ifndef C_PC_SIM
#define BOOK_JSON "/oem/_data/textbook/s_s.json"
#else
#define BOOK_JSON "/userdata/_data/textbook/s_s.json"
#endif

static const char* c_downRecordPath = "/userdata/_data/_download.txt";
yd_bk_node* g_yd_bkHead = NULL;    // 保存教材列表数据
static const char* c_subject[] = { "英语", "语文","英文儿歌","情景对话","国学熏听" ,"古诗随听" };// , "全部"};
static const char* c_grades[] = { "小学", "初中", "高中" };// , "全部"};

//#define RES_BUILTIN_BOOKS_PATH	"/oem/_data/books/"
//sprintf(buf, "/%s/_data/textbook/books/%s/book000/album.mnu", BOOK_PATH, text);

const char* gc_res_store_path[] = { 
#ifdef C_PC_SIM
    "/userdata/_data/textbook/",  // 内置
#else
    "/oem/_data/textbook/",             // 内置
#endif
    "/userdata/_data/textbook/",        // 在线: box  // boll
    "/userdata/_data/yd/",              // 有道在线资源
    "/oem/_data/yd/",                   // 有道内置资源
};


static char* g_pversion[MAX_VERSION_NUM] ;// [MAX_VERSION_NUM] [120] ;    // 保存教材版本
//static char g_pversion[MAX_VERSION_NUM][120];    // 保存教材版本
static char g_version_num = 0;
static yd_book_info_t* gp_cur_book;

static char* get_version_by_id(char pos) {
    return g_pversion[pos];
}

static uint16_t get_version_sum()
{
    return g_version_num;
}

// 默认是存在 / 的，切记
static char* split_file_path_posi(const char* path)
{
    const char* last_slash = strrchr(path, '/');
    const char* last_backslash = strrchr(path, '\\');
    const char* last_sep = (last_slash > last_backslash) ? last_slash : last_backslash;
    if (last_sep)
        return last_sep;
    else
        return path;
}

static void _extract_directory(const char* path, char* dir_out) {
    // 查找最后一个路径分隔符（支持Windows和Unix）
    const char* last_sep = split_file_path_posi(path);

    if (last_sep) {
        size_t len = last_sep - path;
        strncpy(dir_out, path, len);
        dir_out[len] = '\0';
    }
    else {
        strcpy(dir_out, ".");  // 没有分隔符，使用当前目录
    }
}

static int insert_books_node(char* grade, char* name, char* cover_url, char* book_url) {
    yd_bk_node* new_node = (yd_bk_node*)heap_caps_malloc_dflt(sizeof(yd_bk_node));
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

    if (g_yd_bkHead == NULL) {
        g_yd_bkHead = new_node;
    }
    else {
        yd_bk_node* temp = g_yd_bkHead;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_node;
    }
    return 0;
}

/*
static void yd_book_node_free(void) {
    yd_bk_node* pNode = g_bkHead;
    if (pNode) {
        while (pNode != NULL) {
            yd_bk_node* pNodeNx = pNode->next;
            heap_caps_free(pNode);
            pNode = pNodeNx;
        }
        g_bkHead = NULL;
    }
    return;
}*/
void yd_book_node_free(void) {
    yd_bk_node* pNode = g_yd_bkHead;

    if (pNode) {
        while (pNode != NULL) {
            yd_bk_node* pNodeNx = pNode->next;
            //zb_img_res_free_one();

            /*lv_img_dsc_t* img_Src = pNode->data.pimg_dsc;
            if (img_Src) {
                heap_caps_free(img_Src->data);
                img_Src->data = NULL;
                heap_caps_free(img_Src);
            }*/
            heap_caps_free(pNode);
            pNode = pNodeNx;
        }
        g_yd_bkHead = NULL;
    }
}


void yd_book_versions_free() {
    for (int i = 0; i < g_version_num; i++) {
        if (g_pversion[i]) {
            heap_caps_free(g_pversion[i]);
            g_pversion[i] = NULL;
        }
    }
    //heap_caps_free(g_pversion);
    g_version_num = 0;
}


#define MNU_JSON_FILE_LEN_MAX  80000 
const char** yd_read_versions(e_book_genre subject, e_stage stage, uint16_t* version_num)
{
    int funcErrNo = -1;  // >= 0 means success 
    FILE* fp;
    cJSON* root = NULL;
    int16_t iTotal = 0;
    yd_book_versions_free();
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

    cJSON* sgJson = cJSON_GetObjectItem(cJSON_GetObjectItem(root, c_subject[subject]), c_grades[stage]);
    int16_t sum = cJSON_GetArraySize(sgJson);
    cJSON* item = NULL;
    for (int i = 0; i < sum; i++) {
        item = cJSON_GetArrayItem(sgJson, i);
        cJSON* name_item = cJSON_GetObjectItemCaseSensitive(item, "nm");
        uint16_t len = strlen(name_item->valuestring);
        /*char* p = (char*)heap_caps_malloc_dflt(len+1);
        strcpy(p, name_item->valuestring);
        g_pversion[g_version_num] = p;*/
        g_pversion[g_version_num] = heap_caps_malloc_dflt(len + 1);
        strcpy(g_pversion[g_version_num], name_item->valuestring);
        g_pversion[g_version_num][len] = '\0';
        g_version_num++;
    }
    if (root) {
        cJSON_Delete(root);
        heap_caps_free(acBuf);
        root = NULL;
    }
    *version_num = g_version_num;
    return g_pversion;
}

yd_bk_node* yd_read_books(e_book_genre subject, e_stage stage, const char* version_name, uint16_t* book_num)
{
    int funcErrNo = -1;  // >= 0 means success 
    FILE* fp;
    cJSON* root = NULL;
    int16_t iTotal = 0;
    char version[200] = { 0 };
    yd_book_node_free();
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

    if (subject > 1) {
        stage = STAGE_PUPIL;
        sprintf(version, "%s", c_subject[subject]);
    }
    else {
        if (version_name == NULL) {
            ESP_LOGE(TAG, "version is null，check\n");
            funcErrNo =  -6;
        }else
            sprintf(version, "%s", version_name);
    }

    if (funcErrNo != -1)
    {
        *book_num = 0;
        return NULL;
    }

    cJSON * sgJson = cJSON_GetObjectItem(cJSON_GetObjectItem(root, c_subject[subject]), c_grades[stage]);
    int16_t sum = cJSON_GetArraySize(sgJson);
    cJSON* item = NULL;
    int16_t bookSum = 0;
    for (int i = 0; i < sum; i++) {
        item = cJSON_GetArrayItem(sgJson, i);
        cJSON* vname_item = cJSON_GetObjectItemCaseSensitive(item, "nm");
        uint16_t len = strlen(vname_item->valuestring);
        if (strcmp(vname_item->valuestring, version) == 0) {
            cJSON* bookJson = cJSON_GetObjectItem(item, "bks");
            bookSum = cJSON_GetArraySize(bookJson);
            for (int k = 0; k < bookSum; k++) {
                cJSON* bookItem = cJSON_GetArrayItem(bookJson, k);
                //"grade": "一年级",
                //    "name" : "教材全解北京版一年级上册",
                //    "icon" : "cover/bjb_1_1",
                //    "book" : "boll/bjb/bjb_1_1"
                cJSON* gradeItem = cJSON_GetObjectItem(bookItem, "gd");
                cJSON* bnameItem = cJSON_GetObjectItem(bookItem, "nm");
                cJSON* iconUrlItem = cJSON_GetObjectItem(bookItem, "ic");
                cJSON* bookUrlItem = cJSON_GetObjectItem(bookItem, "bk");

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
    *book_num = bookSum;

    return g_yd_bkHead;
}

static yd_book_info_t* get_booknode_by_bookbin(const char* book_url) {
    yd_bk_node* pNode = g_yd_bkHead;
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


static char* get_cur_book_name() {
    if (gp_cur_book != NULL)
        return gp_cur_book->name;
    return "";
}
//static lv_img_dsc_t* get_cur_book_img_dsc() {
//    if (gp_cur_book != NULL)
//        return gp_cur_book->pimg_dsc;
//    return NULL;
//}
//

bool check_down_from_txt(e_res_store res_genre, const char* url)
{
    FILE* fp = NULL;
    bool bFind = false;

    fp = zb_fopen(c_downRecordPath, "r");
    if (fp == NULL)
        return false;
    char line_buf[2002],*p;
    char compare_str[500] = { 0 };
    sprintf(compare_str, "%d;%s", res_genre, url);

    while (true) {
        p = fgets(line_buf, 2000, fp);
        if (p == NULL)
            break;
        if (memcmp(line_buf, compare_str, strlen(compare_str)) == 0)
        {
            bFind = true;
            break;
        }
    }
    zb_fclose(fp);
    return bFind;
}

bool get_cg_album_path(const char* url, char** local_path)
{
    sprintf(*local_path, "%s%s/book000/album.mnu", gc_res_store_path[RES_CG_ONLINE - 1], url);
    if (!zb_isFileExist(*local_path))
        return false;

    return true;
}

static char* sanitize_filename(const char* original_name,char  **sanitized) {
    if (original_name == NULL) {
        return strdup("unnamed");
    }

    // 非法字符集合（根据 Linux/Android 文件系统规则）
    const char* illegal_chars = "/\\:*?\"<>| ";  // 添加更多需要过滤的字符
    const char* unsafe_chars = "$&;`'!(){}[]~#%^,=";  // 可能引起问题的字符

    // 计算新字符串长度
    size_t len = strlen(original_name);
    if (len == 0) {
        strcpy(*sanitized, "unnamed");
        return *sanitized;
    }

    char* dest = *sanitized;
    const char* src = original_name;

    // 处理前导点（隐藏文件可能引起问题）
    if (*src == '.') {
        *dest++ = '_';  // 将前导点替换为下划线
        src++;
    }

    // 遍历并处理每个字符
    while (*src) {
        // 检查非法字符
        if (strchr(illegal_chars, *src) || *src < 0x20) {
            *dest++ = '_';  // 替换为下划线
            src++;
            continue;
        }

        // 检查可能引起问题的字符
        if (strchr(unsafe_chars, *src)) {
            // 使用 URL 编码风格替换
            sprintf(dest, "%%%02X", (unsigned char)*src);
            dest += 3;
            //*dest++ = '_';
            src++;
            continue;
        }

        // 保留合法字符
        *dest++ = *src++;
    }

    *dest = '\0';  // 字符串结束符

    // 处理连续替换字符导致的过长名称
    if (strlen(*sanitized) > 255) {
        *sanitized[255] = '\0';  // 截断到最大长度
    }

    return *sanitized;
}
/*
* 检查资源是否已下载，已下载返回 true 否则返回false，同时把待下载的或已下载的本地资源路径（local_path）返回
* res_youdao: true:是有道资源，否则 晨光
* url: 有道资源url 或者教材路径
* local_path： 返回本地路径
* book_id: 专辑id
*/
bool check_res_down(e_res_store res_genre, const char* url, char** local_path,uint8_t book_id, bool create_dir)
{
    char path[400];
    char chk_path[400];
    bool bExist = true;

    if (*url == '/')
        url++;

    if (*local_path == NULL) {
        ESP_LOGI(TAG, "文件指针为空：local_path！");
        return false;
    }
    if (res_genre == RES_YD_ONLINE) {
        strcpy(path, url);
        char* p = &chk_path;
        char* split_sep = split_file_path_posi(path);
        *split_sep++ = 0;
        sanitize_filename(split_sep, &p);
        //strcpy(split_sep, chk_path);
        //*(split_sep-1) = '/';
        sprintf(*local_path, "%s%s/%s", gc_res_store_path[res_genre - 1], path, chk_path);
        sprintf(chk_path, "%s/%s", path, chk_path);
    }
    else if (res_genre == RES_CG_BUILDIN) {
        sprintf(path, "%s%s", gc_res_store_path[res_genre - 1], url);
        sprintf(*local_path, "%s/album.mnu", path);
        sprintf(chk_path, "%s/album.mnu", path);
    }
    else { // RES_CG_ONLINE
        sprintf(path, "%s%s", gc_res_store_path[res_genre - 1], url);
        sprintf(*local_path, "%s/book%03d/book.bin", path, book_id);
        sprintf(chk_path, "%s",url); // 在表中仅仅存放的是url
    }
    ESP_LOGI(TAG, "返回的本地路径是：%s！\n", *local_path);

    if (create_dir) {
#ifndef C_PC_SIM
        ESP_LOGI(TAG, "创建目录：%s！", path);
        mkdir_p(path, 0777);
#else
        zb_mkdir(path, 0777);
#endif
    }
    ESP_LOGI(TAG, "------check_path: %s\n", chk_path);
    if (true)
    {
#ifndef C_PC_SIM
        bExist = check_url_in_db(res_genre, chk_path);
        if (!bExist)
            ESP_LOGI(TAG, "------check_path: %s不存在\n", chk_path);
#endif
    }else
    if (!zb_isFileExist(chk_path)) {
        ESP_LOGI(TAG, "------check_path: %s不存在\n", chk_path);
        bExist = false;
    }

    return bExist;
}

/*
* 拼接出晨光下载的链接
*/
int get_cg_net_url(char* part_path, char** net_url) {
    // http://speed2.bolledu.net/BOLLTVVideo/upload/data/chenguang-tlb/download/books/boll/wyxbz3q/wyxbz3q_4_1/book.bin
    // http://speed2.bolledu.net/BOLLTVVideo/upload/data/chenguang-tlb/download/books/boll/bjb/bjb_2_1/book.bin
    // http://speed2.bolledu.net/BOLLTVVideo/upload/data/chenguang-tlb/download/books/nimibox/人教统编版/tape8a_002001_202412231522/book.bin
    if (*net_url == NULL)
        return -1;
    if (memcmp(part_path, "box", 3) == 0) {
        int errorNo;
        char tmp_buf[300] = { 0 };
        sprintf(tmp_buf, "nimi%s", part_path);
        url_encode_t* purl = zb_urlEnc_init((const char*)RES_NET_BOOKS_PATH, &errorNo);
        zb_urlEnc_setPath(purl, tmp_buf);
        sprintf(*net_url, "%s/book.bin", purl->m_pcUrlBuf);
        zb_urlEnc_deinit(purl);
        //sprintf(*net_url, "%s/nimi%s/book.bin", RES_NET_BOOKS_PATH, part_path);
    }
    else
        sprintf(*net_url, "%s/%s/book.bin", RES_NET_BOOKS_PATH, part_path);
    return 1;
}


uint32_t get_int(const unsigned char* bytes) {
    uint32_t num = 0;
    num |= bytes[0] << 0;
    num |= bytes[1] << 8;
    num |= bytes[2] << 16;
    num |= bytes[3] << 24;

    //ESP_LOGI(TAG, "%s[%ld]\n", __FUNCTION__, num);
    return num;
}

uint16_t get_short(const unsigned char* bytes) {
    uint16_t num = 0;
    num |= bytes[0] << 0;
    num |= bytes[1] << 8;

    return num;
}


/*
*  拆book.bin 生成album.bin, unit001.bin ........
*  book_path: 原数据
*  dest_path: 目标目录,如果目标目录为NULL，则拆到book_path 同一级目录下
*/
int yd_split_cg_book(const char* book_path, const char* dest_path) {
    yd_split_book_t sbook = { 0 };
    char split_path[ZB_MAX_PATH] = { 0 };
    if (dest_path == NULL) {
        _extract_directory(book_path, split_path);
        dest_path = split_path;
    }
    else {
#ifndef C_PC_SIM
        mkdir_p(dest_path, 0777);
#else
        zb_mkdir(dest_path,0);
#endif // !C_PC_SIM

    }

    FILE* rf = zb_fopen(book_path, "rb");
    FILE* wf = NULL;
    if (!rf) {
        ESP_LOGE(TAG, "Fail to open file: %s\n", book_path);
        return -1;
    }
    ESP_LOGE(TAG, "succeed to open file: %s\n", book_path);
    printf("succeed to open file: %s\n", book_path);
    uint32_t bytes_read;
    char tmp_file_path[ZB_MAX_PATH];
    uint8_t buffer[CHUNK_SIZE];
    const unsigned char* pbuf = NULL;
    char part_path[256];
    uint32_t blk_start = 0, blk_end = 0, blk_mid = 0;

    bytes_read = zb_fread(&buffer, 1, CHUNK_SIZE, rf);
    //bytes_read = zb_fread(&buffer, 1, 1000, rf);
    pbuf = &buffer[0x10];
    sbook.unit_sum = get_short(pbuf);
    //zb_fread(&chk_sum, 2, 1, fp);
    pbuf += 4;
    sbook.header_size = get_int(pbuf);
    pbuf += 4;
    sbook.file_size = get_int(pbuf);

    sbook.addr_lst = (uint32_t*)heap_caps_malloc_dflt(sizeof(uint32_t) * (sbook.unit_sum + 2));
    if (sbook.addr_lst) {
        pbuf = &buffer[sbook.header_size];
        for (int i = 0; i <= sbook.unit_sum + 1; i++)
        {
            sbook.addr_lst[i] = get_int(pbuf);
            pbuf += 4;
        }
    }

    sprintf(tmp_file_path, "%s/album.mnu", dest_path);
    ESP_LOGW(TAG, " %s   %d:  %s\n", __FUNCTION__, __LINE__, tmp_file_path);
    wf = zb_fopen(tmp_file_path, "wb");
    pbuf = &buffer[sbook.addr_lst[0]];
    sbook.pos = sbook.addr_lst[0];
    blk_end = bytes_read;
    int left_size  = blk_end - sbook.addr_lst[0];
    // 读取并拆分文件
    while (1) {
        if (sbook.pos + left_size < sbook.addr_lst[sbook.file_cnt + 1])
        {
            fwrite(pbuf, 1, left_size, wf);
            sbook.pos += left_size;
        }
        else {
            blk_mid = sbook.addr_lst[sbook.file_cnt+1] - sbook.pos;
            fwrite(pbuf, 1, blk_mid, wf);
            zb_fclose(wf);
            sbook.pos += blk_mid;
            
            if (sbook.file_cnt == sbook.unit_sum)
                break;
            sbook.file_cnt += 1;
            sprintf(tmp_file_path, "%s/unit%03d.bin", dest_path, sbook.file_cnt);
            wf = zb_fopen(tmp_file_path, "wb");
            if (sbook.pos < sbook.addr_lst[sbook.file_cnt + 1]) {
                zb_fwrite(pbuf+blk_mid, 1, bytes_read -blk_mid, wf);
                sbook.pos += bytes_read - blk_mid;
            }
        }
        // 读取一块数据
        bytes_read = zb_fread(buffer, 1, CHUNK_SIZE, rf);
        pbuf = &buffer[0];
        if (bytes_read == 0) {
            break;
        }
        left_size = bytes_read;

    }
    zb_fclose(rf);

    //zb_remove(book_path);
    printf("删除文件：%s\n", book_path);

    return 1;
}


// 确保文件存在（不存在则创建），并以追加模式打开
static FILE * open_or_create_append(const char* path) {
    // 首先尝试以追加模式打开（如果文件存在）
    FILE* fp = zb_fopen(path, "a");
    if (fp) {
        zb_fclose(fp);  // 关闭后重新以追加模式打开（确保位置正确）
        return zb_fopen(path, "a");
    }

    // 文件不存在，创建新文件
    fp = zb_fopen(path, "w");
    if (!fp) {
        LV_LOG_ERROR("File creation failed: %s", path);
        return NULL;
    }
    zb_fclose(fp);

    // 以追加模式重新打开
    return zb_fopen(path, "a");
}


/*
* 下载完成时提供的调用接口，用来保存下载的信息
* title: 显示名
* res_genre: 资源存储类别
* url_path:  请求下载的url链接
*/
//下载完成调用传参数：、类别【网盘，有道，教材】、下载路径
int save_down_record(const char* title, e_res_store res_genre, const char* url_path)
{
    FILE* fp;
/*
    time_t now;
    time(&now);
    struct tm timeinfo;
#ifndef C_PC_SIM
    setenv("TZ", "CST-8", 1);  // 将时区设置为中国标准时间
    tzset();
    localtime_r(&now, &timeinfo);
#else
    localtime_s(&timeinfo, &now);
#endif
 */
    //int64_t timeCur = (int64_t)time(NULL);
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_buf[29];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H%M%S", tm_info);

    fp = open_or_create_append(c_downRecordPath);
    if (fp != NULL) {
        char buf[1000];
        sprintf(buf, "%d;%s;%s;%s\n", res_genre, url_path, time_buf, title);
        fputs(buf, fp);
    }
    zb_fclose(fp);
}

bool get_res_path(e_res_store res_genre, const char* url, char** local_path)
{
    if (*url == '/')
        url++;

    if (*local_path == NULL) {
        ESP_LOGI(TAG, "文件指针为空：local_path！");
        return false;
    }

    if (res_genre == RES_YD_ONLINE) {
        sprintf(*local_path, "%s%s", gc_res_store_path[res_genre - 1], url);
    }
    else if (res_genre == RES_CG_ONLINE) {
        sprintf(*local_path, "%s%s", gc_res_store_path[res_genre - 1], url);
    }
    else
    {
        ESP_LOGI(TAG, "资源类型不正确！：%d\n", res_genre);
    }
    ESP_LOGI(TAG, "返回的本地路径是：%s！\n", *local_path);

    if (!zb_isFileExist(*local_path)) {
        ESP_LOGI(TAG, "------check_path: %s不存在\n", *local_path);
        return false;
    }

    return true;
}
