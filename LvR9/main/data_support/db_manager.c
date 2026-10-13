#include "../utils/misc_utils.h"
#include "../data_parse/img_res.h"
#include "../include/img_a_tlb.h"
//#include "../collect/ui_collect.h"
#include "../data_parse/cJSON.h"
#include "../data_parse/cJSON_Utils.h"
#include "../player/player_list.h"
#include "../ex_widget/app_msg.h"
#include <errno.h>
#include "db_manager.h"

#include <string.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TAG  "DB_MM"

const char* gc_db_path = "/userdata/_data/R9_yd_sql.db";

// 数据库表结构
#define COLLECTION_TABLE_SCHEMA \
    "CREATE TABLE IF NOT EXISTS collections (" \
    "id INTEGER PRIMARY KEY AUTOINCREMENT," \
    "type INTEGER NOT NULL," \
    "path TEXT NOT NULL," \
    "title TEXT NOT NULL," \
    "playNum INTEGER DEFAULT 0," \
    "timestamp INTEGER DEFAULT 0," \
    "source_type INTEGER DEFAULT 0," \
    "is_downloaded BOOLEAN DEFAULT 0," \
    "appNo INTEGER DEFAULT 0," \
    "sndTime INTEGER DEFAULT 0," \
    "UNIQUE(appNo, source_type,playNum, path)" \
    ");" \
    "CREATE INDEX IF NOT EXISTS idx_appNo ON collections(appNo);" \
    "CREATE INDEX IF NOT EXISTS idx_timestamp ON collections(timestamp);" \
    "CREATE INDEX IF NOT EXISTS idx_play_count ON collections(playNum);"

CollectionService* s_collection_service = NULL;

#ifndef C_PC_SIM



// 服务内部结构
struct CollectionService {
    sqlite3* db;                // 数据库句柄
    sqlite3_stmt* get_item_stmt; // 预编译语句缓存
    CollectionItem last_item;    // 最后获取的项
    char* current_order;         // 当前排序方式
};

/* ======================== 数据库辅助函数 ======================== */

// 执行SQL语句
static bool exec_sql(const char* sql) {
    char* err_msg = NULL;
    int rc = sqlite3_exec(s_collection_service->db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}


// 准备预编译语句
static sqlite3_stmt* prepare_stmt(const char* sql) {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(s_collection_service->db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare error: %s\n", sqlite3_errmsg(s_collection_service->db));
        return NULL;
    }
    return stmt;
}

// 绑定文本参数
static void bind_text_param(sqlite3_stmt* stmt, int index, const char* value) {
    if (value) sqlite3_bind_text(stmt, index, value, -1, SQLITE_TRANSIENT);
    else sqlite3_bind_null(stmt, index);
}

/* ======================== 服务实现 ======================== */

CollectionService* collection_service_create() {
    CollectionService* service = malloc(sizeof(CollectionService));
    if (!service) return NULL;

    memset(service, 0, sizeof(CollectionService));

    // 打开数据库
    int rc = sqlite3_open(gc_db_path, &service->db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(service->db));
        free(service);
        return NULL;
    }

    s_collection_service = service;

    // 创建表结构
    if (!exec_sql(COLLECTION_TABLE_SCHEMA)) {
        sqlite3_close(service->db);
        free(service);
        return NULL;
    }

    return service;
}

void collection_service_destroy(CollectionService* service) {
    if (!service) return;

    if (service == s_collection_service) {
        s_collection_service = NULL;
    }
    // 清理预编译语句
    if (service->get_item_stmt) sqlite3_finalize(service->get_item_stmt);

    // 清理最后获取的项
    if (service->last_item.path) free(service->last_item.path);
    if (service->last_item.title) free(service->last_item.title);

    // 清理排序字符串
    if (service->current_order) free(service->current_order);

    // 关闭数据库
    sqlite3_close(service->db);
    free(service);
}

bool collection_service_add_item(
    CollectionType type,
    const char* path,
    const char* title,
    uint16_t playNum,
    MusicSourceType source_type, 
    bool is_downloaded,
    uint16_t appNo,
    uint32_t sndTime)
{
    ESP_LOGI(TAG, "add_item：path = %s,title=%s\n",path, title);
    if (!s_collection_service || !path || !title) return false;
    ESP_LOGI(TAG, "type=%d,path=%s,title=%s\n", type, path, title);
    const char* sql = "INSERT OR REPLACE INTO collections "
        "(type, path, title, playNum, timestamp, source_type, is_downloaded, appNo, sndTime) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt = prepare_stmt(sql);
    if (!stmt) return false;

    // 绑定参数
    sqlite3_bind_int(stmt, 1, type);
    sqlite3_bind_text(stmt, 2, path, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, title, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, playNum);
    sqlite3_bind_int64(stmt, 5, time(NULL));
    sqlite3_bind_int(stmt, 6, source_type); 
    sqlite3_bind_int(stmt, 7, is_downloaded ? 1 : 0);
    sqlite3_bind_int(stmt, 8, appNo);
    sqlite3_bind_int(stmt, 9, sndTime);

    // 执行语句
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool collection_service_remove_item(
    CollectionType type,
    const char* path,
    uint16_t appNo)
{
    if (!s_collection_service || !path) return false;

    const char* sql = "DELETE FROM collections WHERE type = ? AND path = ? AND appNo =?";
    sqlite3_stmt* stmt = prepare_stmt(sql);
    if (!stmt) return false;

    sqlite3_bind_int(stmt, 1, type);
    sqlite3_bind_text(stmt, 2, path, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, appNo);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool collection_service_remove_items(
    CollectionType type,
    const char** paths,
    uint32_t count,
    uint16_t appNo)
{
    if (!paths || count == 0) return false;

    // 开始事务
    if (!exec_sql("BEGIN TRANSACTION")) return false;

    bool success = true;
    for (uint32_t i = 0; i < count; i++) {
        if (!collection_service_remove_item(type, paths[i],appNo)) {
            success = false;
            break;
        }
    }

    // 提交或回滚事务
    exec_sql(success ? "COMMIT" : "ROLLBACK");
    return success;
}

uint32_t collection_service_get_count(
    CollectionType type,
    uint16_t appNo)
{
    const char* sql = "SELECT COUNT(*) FROM collections WHERE type = ? AND appNo = ?";
    sqlite3_stmt* stmt = prepare_stmt(sql);
    if (!stmt) return 0;

    sqlite3_bind_int(stmt, 1, type);
    sqlite3_bind_int(stmt, 2, appNo);

    uint32_t count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

const CollectionItem* collection_service_get_item(
    CollectionType type,
    uint32_t index,
    uint16_t appNo)
{

    // 准备或重用预编译语句
    const char* base_sql = "SELECT path, title, playNum, timestamp, source_type, is_downloaded, appNo, sndTime "
        "FROM collections WHERE type = ? AND appNo = ?";

    // 如果排序方式改变，重新准备语句
    if (!s_collection_service->get_item_stmt ||
        (s_collection_service->current_order && strcmp(s_collection_service->current_order, s_collection_service->current_order) != 0)) {

        if (s_collection_service->get_item_stmt) {
            sqlite3_finalize(s_collection_service->get_item_stmt);
            s_collection_service->get_item_stmt = NULL;
        }

        // 构建完整SQL
        char sql[256];
        snprintf(sql, sizeof(sql), "%s ORDER BY %s LIMIT 1 OFFSET ?",
            base_sql,
            s_collection_service->current_order ? s_collection_service->current_order : "timestamp DESC");

        s_collection_service->get_item_stmt = prepare_stmt(sql);
        if (!s_collection_service->get_item_stmt) return NULL;
    }

    // 绑定参数
    sqlite3_bind_int(s_collection_service->get_item_stmt, 1, type);
    sqlite3_bind_int(s_collection_service->get_item_stmt, 2, index);
    sqlite3_bind_int(s_collection_service->get_item_stmt, 3, appNo);

    // 执行查询
    if (sqlite3_step(s_collection_service->get_item_stmt) != SQLITE_ROW) {
        sqlite3_reset(s_collection_service->get_item_stmt);
        return NULL;
    }

    // 释放上次分配的内存
    if (s_collection_service->last_item.path) free(s_collection_service->last_item.path);
    if (s_collection_service->last_item.title) free(s_collection_service->last_item.title);

    // 复制数据到缓存项
    s_collection_service->last_item.path = strdup((const char*)sqlite3_column_text(s_collection_service->get_item_stmt, 0));
    s_collection_service->last_item.title = strdup((const char*)sqlite3_column_text(s_collection_service->get_item_stmt, 1));
    s_collection_service->last_item.playNum = sqlite3_column_int(s_collection_service->get_item_stmt, 2);
    s_collection_service->last_item.timestamp = sqlite3_column_int64(s_collection_service->get_item_stmt, 3);
    s_collection_service->last_item.source_type = (MusicSourceType)sqlite3_column_int(s_collection_service->get_item_stmt, 4);  // 来源类型
    s_collection_service->last_item.is_downloaded = sqlite3_column_int(s_collection_service->get_item_stmt, 5) != 0;
    s_collection_service->last_item.appNo = sqlite3_column_int(s_collection_service->get_item_stmt, 6);
    s_collection_service->last_item.sndTime = sqlite3_column_int(s_collection_service->get_item_stmt, 7);

    sqlite3_reset(s_collection_service->get_item_stmt);
    return &s_collection_service->last_item;
}

const char* collection_service_get_path(
    CollectionType type,
    uint32_t index,
    uint16_t appNo)
{
    const CollectionItem* item = collection_service_get_item(type, index,appNo);
    return item ? item->path : NULL;
}

const char* collection_service_get_title(
    CollectionType type,
    uint32_t index,
    uint16_t appNo)
{
    const CollectionItem* item = collection_service_get_item(type, index, appNo);
    return item ? item->title : NULL;
}

const CollectionItem* collection_service_find_item(
    CollectionType type,
    const char* path,
    uint16_t appNo)
{
    if (!s_collection_service|| !path) return NULL;

    const char* sql = "SELECT path, title, playNum, timestamp, source_type, is_downloaded ,appNo,sndTime "
        "FROM collections WHERE type = ? AND path = ? AND appNo = ? ";
    sqlite3_stmt* stmt = prepare_stmt(sql);
    if (!stmt) return NULL;

    sqlite3_bind_int(stmt, 1, type);
    sqlite3_bind_text(stmt, 2, path, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, appNo);

    const CollectionItem* item = NULL;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // 使用服务缓存存储结果
        if (s_collection_service->last_item.path) free(s_collection_service->last_item.path);
        if (s_collection_service->last_item.title) free(s_collection_service->last_item.title);

        s_collection_service->last_item.path = strdup((const char*)sqlite3_column_text(stmt, 0));
        s_collection_service->last_item.title = strdup((const char*)sqlite3_column_text(stmt, 1));
        s_collection_service->last_item.playNum = sqlite3_column_int(stmt, 2);
        s_collection_service->last_item.timestamp = sqlite3_column_int64(stmt, 3);
        s_collection_service->last_item.source_type = (MusicSourceType)sqlite3_column_int(stmt, 4);
        s_collection_service->last_item.is_downloaded = sqlite3_column_int(stmt, 5) != 0;
        s_collection_service->last_item.appNo = sqlite3_column_int(stmt, 6);
        s_collection_service->last_item.sndTime = sqlite3_column_int(stmt, 7);

        item = &s_collection_service->last_item;
    }

    sqlite3_finalize(stmt);
    return item;
}

void collection_service_set_sort_order(
    CollectionType type,
    const char* order_by)
{
    if (!s_collection_service) return;

    // 释放旧排序字符串
    if (s_collection_service->current_order) {
        free(s_collection_service->current_order);
        s_collection_service->current_order = NULL;
    }

    // 复制新排序字符串
    if (order_by) {
        s_collection_service->current_order = strdup(order_by);
    }

    // 重置预编译语句
    if (s_collection_service->get_item_stmt) {
        sqlite3_finalize(s_collection_service->get_item_stmt);
        s_collection_service->get_item_stmt = NULL;
    }
}

bool collection_service_transaction(
    bool (*exec_func)(CollectionService*, void*),
    void* user_data)
{
    if (!s_collection_service || !exec_func) return false;

    // 开始事务
    if (!exec_sql("BEGIN EXCLUSIVE TRANSACTION")) return false;

    // 执行用户函数
    bool success = exec_func(s_collection_service, user_data);

    // 提交或回滚
    exec_sql(success ? "COMMIT" : "ROLLBACK");
    return success;
}

bool collection_service_export_to_json(
    uint16_t appNo,
    const char* json_path)
{
    if (!s_collection_service || !json_path) return false;

    const char* sql = "SELECT * FROM collections WHERE appNo = ?";
    sqlite3_stmt* stmt = prepare_stmt(sql);
    if (!stmt) return false;

    sqlite3_bind_int(stmt, 1, appNo);

    // 创建JSON根对象
    cJSON* root = cJSON_CreateArray();
    if (!root) {
        sqlite3_finalize(stmt);
        return false;
    }

    // 遍历结果集
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        cJSON* item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "type", sqlite3_column_int(stmt, 1));
        cJSON_AddStringToObject(item, "path", (const char*)sqlite3_column_text(stmt, 2));
        cJSON_AddStringToObject(item, "title", (const char*)sqlite3_column_text(stmt, 3));
        cJSON_AddNumberToObject(item, "playNum", sqlite3_column_int(stmt, 4));
        cJSON_AddNumberToObject(item, "timestamp", sqlite3_column_int64(stmt, 5));
        cJSON_AddNumberToObject(item, "source_type", sqlite3_column_int(stmt, 6));
        cJSON_AddBoolToObject(item, "is_downloaded", sqlite3_column_int(stmt, 7));
        cJSON_AddBoolToObject(item, "appNo", sqlite3_column_int(stmt, 8));
        cJSON_AddItemToArray(root, item);
    }

    sqlite3_finalize(stmt);

    // 写入文件
    FILE* fp = fopen(json_path, "w");
    if (!fp) {
        cJSON_Delete(root);
        return false;
    }

    char* json_str = cJSON_Print(root);
    fputs(json_str, fp);
    fclose(fp);

    cJSON_Delete(root);
    free(json_str);
    return true;
}

bool collection_service_import_from_json(
    CollectionType type,
    const char* json_path)
{
    if (!s_collection_service || !json_path) return false;

    // 读取文件
    FILE* fp = fopen(json_path, "r");
    if (!fp) return false;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buffer = malloc(size + 1);
    fread(buffer, 1, size, fp);
    buffer[size] = '\0';
    fclose(fp);

    // 解析JSON
    cJSON* root = cJSON_Parse(buffer);
    free(buffer);
    if (!root) return false;

    // 开始事务
    if (!exec_sql("BEGIN TRANSACTION")) {
        cJSON_Delete(root);
        return false;
    }

    // 遍历JSON数组
    bool success = true;
    int array_size = cJSON_GetArraySize(root);
    for (int i = 0; i < array_size; i++) {
        cJSON* item = cJSON_GetArrayItem(root, i);

        // 只导入匹配类型的项
        cJSON* json_type = cJSON_GetObjectItem(item, "type");
        if (!json_type || json_type->valueint != type) continue;

        // 提取字段
        cJSON* path = cJSON_GetObjectItem(item, "path");
        cJSON* title = cJSON_GetObjectItem(item, "title");
        cJSON* playNum = cJSON_GetObjectItem(item, "playNum");
        cJSON* timestamp = cJSON_GetObjectItem(item, "timestamp");
        cJSON* source_type = cJSON_GetObjectItem(item, "source_type");
        cJSON* is_downloaded = cJSON_GetObjectItem(item, "is_downloaded");

        if (!path || !title) {
            success = false;
            break;
        }

        // 添加到数据库
        const char* sql = "INSERT OR IGNORE INTO collections "
            "(type, path, title, playNum, timestamp, source_type, is_downloaded) " // 修正字段
            "VALUES (?, ?, ?, ?, ?, ?, ?)";

        sqlite3_stmt* stmt = prepare_stmt(sql);
        if (!stmt) {
            success = false;
            break;
        }

        sqlite3_bind_int(stmt, 1, type);
        sqlite3_bind_text(stmt, 2, path->valuestring, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, title->valuestring, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, playNum ? playNum->valueint : 0);
        sqlite3_bind_int64(stmt, 5, timestamp ? timestamp->valueint : time(NULL));
        //sqlite3_bind_int(stmt, 6, source_type ? source_type->valueint : RES_CG_BUILDIN);
        sqlite3_bind_int(stmt, 6, source_type->valueint);
        sqlite3_bind_int(stmt, 7, is_downloaded ? cJSON_IsTrue(is_downloaded) : 0);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            success = false;
        }

        sqlite3_finalize(stmt);
    }

    // 提交或回滚
    exec_sql(success ? "COMMIT" : "ROLLBACK");
    cJSON_Delete(root);
    return success;
}

// 添加收藏
bool add_to_collection(CollectionType type, const char* path, const char* title, MusicSourceType source_type,uint16_t appNo,uint32_t sndTime) {
    ESP_LOGI(TAG, "add_to_collection:\n");
    bool stat = collection_service_add_item(
        type,
        path,
        title,
        0,      // 播放次数
        source_type,
        false,   // 下载状态
        appNo,
        sndTime
    );
    return stat;
}

// 取消收藏
void remove_from_collection(CollectionType type, const char* path,uint16_t appNo) {

    collection_service_remove_item(
        type,
        path,
        appNo
    );
}

// 检查是否已收藏
bool is_item_collected(CollectionType type, const char* path,uint16_t appNo) {

    return collection_service_find_item(
        type,
        path,
        appNo
    ) != NULL;
}

// 设置资源来源类型
bool collection_service_set_source_type(
    CollectionType type,
    const char* path,
    MusicSourceType source_type,
    uint16_t appNo)
{
    if (!s_collection_service || !path) return false;

    const char* sql = "UPDATE collections SET source_type = ? "
        "WHERE type = ? AND path = ? AND appNo = ?";
    sqlite3_stmt* stmt = prepare_stmt(sql);
    if (!stmt) return false;

    sqlite3_bind_int(stmt, 1, source_type);
    sqlite3_bind_int(stmt, 2, type);
    sqlite3_bind_text(stmt, 3, path, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, appNo);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

// 获取资源来源类型
MusicSourceType collection_service_get_source_type(
    CollectionType type,
    const char* path,
    uint16_t appNo)
{
    if (!s_collection_service || !path) return RES_CG_BUILDIN;  // 默认值

    const char* sql = "SELECT source_type FROM collections "
        "WHERE type = ? AND path = ? AND appNo = ?";
    sqlite3_stmt* stmt = prepare_stmt(sql);
    if (!stmt) return RES_CG_BUILDIN;

    sqlite3_bind_int(stmt, 1, type);
    sqlite3_bind_text(stmt, 2, path, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, appNo);

    MusicSourceType source_type = RES_CG_BUILDIN;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        source_type = (MusicSourceType)sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return source_type;
}

uint32_t collection_service_get_count_with_filter(
    CollectionType type,
    uint16_t appNo,
    const CollectionFilter* filter)
{
    if (!s_collection_service) return 0;

    char filter_sql[256] = { 0 };
    build_filter_sql(filter, filter_sql, sizeof(filter_sql));

    ESP_LOGI(TAG, "collection_service_get_count_with_filter:  %s\n", filter_sql);
    char sql[256];
    if (strlen(filter_sql) > 0) {
        snprintf(sql, sizeof(sql), "SELECT COUNT(*) FROM collections WHERE type = ? AND appNo = ? AND %s", filter_sql);
    }
    else {
        snprintf(sql, sizeof(sql), "SELECT COUNT(*) FROM collections WHERE type = ? AND appNo = ?");
    }

    sqlite3_stmt* stmt = prepare_stmt(sql);
    if (!stmt) return 0;

    sqlite3_bind_int(stmt, 1, type);
    sqlite3_bind_int(stmt, 2, appNo);

    uint32_t count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

const CollectionItem* collection_service_get_item_with_filter(
    CollectionType type,
    uint16_t appNo,
    const CollectionFilter* filter,
    uint32_t index)
{
    if (!s_collection_service) return NULL;

    char filter_sql[256];
    build_filter_sql(filter, filter_sql, sizeof(filter_sql));

    const char* base_sql = "SELECT path, title, playNum, timestamp, source_type, is_downloaded, appNo,sndTime  "
        "FROM collections WHERE type = ? AND appNo = ?";

    // 如果排序方式改变，重新准备语句
    if (!s_collection_service->get_item_stmt ||
        (s_collection_service->current_order && strcmp(s_collection_service->current_order, s_collection_service->current_order) != 0)) {

        if (s_collection_service->get_item_stmt) {
            sqlite3_finalize(s_collection_service->get_item_stmt);
            s_collection_service->get_item_stmt = NULL;
        }

        // 构建完整 SQL
        char sql[256];
        if (strlen(filter_sql) > 0) {
            snprintf(sql, sizeof(sql), "%s AND %s ORDER BY %s LIMIT 1 OFFSET ?",
                base_sql,
                filter_sql,
                s_collection_service->current_order ? s_collection_service->current_order : "timestamp DESC");
        }
        else {
            snprintf(sql, sizeof(sql), "%s ORDER BY %s LIMIT 1 OFFSET ?",
                base_sql,
                s_collection_service->current_order ? s_collection_service->current_order : "timestamp DESC");
        }

        s_collection_service->get_item_stmt = prepare_stmt(sql);
        if (!s_collection_service->get_item_stmt) return NULL;
    }

    // 绑定参数
    sqlite3_bind_int(s_collection_service->get_item_stmt, 1, type);
    sqlite3_bind_int(s_collection_service->get_item_stmt, 2, appNo);
    sqlite3_bind_int(s_collection_service->get_item_stmt, 3, index);

    // 执行查询
    if (sqlite3_step(s_collection_service->get_item_stmt) != SQLITE_ROW) {
        sqlite3_reset(s_collection_service->get_item_stmt);
        return NULL;
    }

    // 释放上次分配的内存
    if (s_collection_service->last_item.path) free(s_collection_service->last_item.path);
    if (s_collection_service->last_item.title) free(s_collection_service->last_item.title);

    // 复制数据到缓存项
    s_collection_service->last_item.path = strdup((const char*)sqlite3_column_text(s_collection_service->get_item_stmt, 0));
    s_collection_service->last_item.title = strdup((const char*)sqlite3_column_text(s_collection_service->get_item_stmt, 1));
    s_collection_service->last_item.playNum = sqlite3_column_int(s_collection_service->get_item_stmt, 2);
    s_collection_service->last_item.timestamp = sqlite3_column_int64(s_collection_service->get_item_stmt, 3);
    s_collection_service->last_item.source_type = (MusicSourceType)sqlite3_column_int(s_collection_service->get_item_stmt, 4);
    s_collection_service->last_item.is_downloaded = sqlite3_column_int(s_collection_service->get_item_stmt, 5) != 0;
    s_collection_service->last_item.appNo = sqlite3_column_int(s_collection_service->get_item_stmt, 6);;
    s_collection_service->last_item.sndTime = sqlite3_column_int(s_collection_service->get_item_stmt, 7);

    sqlite3_reset(s_collection_service->get_item_stmt);
    return &s_collection_service->last_item;
}


/**
 * @brief 获取满足筛选条件的所有收藏项
 *
 * @param service 收藏服务实例句柄
 * @param type 收藏类型
 * @param filter 筛选条件
 * @param count [out] 返回的数组长度
 * @return CollectionItem* 动态分配的数组，使用后需调用 collection_service_free_items_array 释放
 */
CollectionItem* collection_service_get_all_items_with_filter(
    CollectionType type,
    uint16_t appNo,
    const CollectionFilter* filter,
    uint32_t* count)
{
    if (!s_collection_service || !count) return NULL;
    ESP_LOGI(TAG, "filter: type=%d, appNo=%d\n",type, appNo);

    // 获取符合条件的记录数量
    *count = collection_service_get_count_with_filter(type,appNo, filter);
    if (*count == 0) return NULL;

    // 构建基础SQL
    const char* base_sql = "SELECT path, title, playNum, timestamp, source_type, is_downloaded ,appNo ,sndTime  "
        "FROM collections WHERE type = ? AND appNo = ?";

    ESP_LOGI(TAG, "count=%d, filter: 3333: %s\n", *count, base_sql);

    // 构建过滤条件SQL
    char filter_sql[256];
    build_filter_sql(filter, filter_sql, sizeof(filter_sql));

    // 构建完整SQL
    char full_sql[512];
    if (strlen(filter_sql)){
        snprintf(full_sql, sizeof(full_sql), "%s AND %s", base_sql, filter_sql);
    }
    else {
        strncpy(full_sql, base_sql, sizeof(full_sql));
    }
    // 添加排序
    if (s_collection_service->current_order) {
        strncat(full_sql, " ORDER BY ", sizeof(full_sql) - strlen(full_sql) - 1);
        strncat(full_sql, s_collection_service->current_order, sizeof(full_sql) - strlen(full_sql) - 1);
    }

    ESP_LOGI(TAG, "filter: 5555: %s\n", full_sql);

    // 准备语句
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(s_collection_service->db, full_sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(s_collection_service->db));
        return NULL;
    }

    // 绑定类型参数
    sqlite3_bind_int(stmt, 1, type);
    sqlite3_bind_int(stmt, 2, appNo);

    ESP_LOGI(TAG, "filter: 7777\n");
    // 分配内存
    CollectionItem* items = malloc(*count * sizeof(CollectionItem));
    if (!items) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    memset(items, 0, *count * sizeof(CollectionItem));

    // 读取数据
    uint32_t index = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && index < *count) {
        // 复制字符串字段
        const char* path = (const char*)sqlite3_column_text(stmt, 0);
        items[index].path = path ? strdup(path) : NULL;

        const char* title = (const char*)sqlite3_column_text(stmt, 1);
        items[index].title = title ? strdup(title) : NULL;

        // 复制其他字段
        items[index].playNum = sqlite3_column_int(stmt, 2);
        items[index].timestamp = sqlite3_column_int64(stmt, 3);
        items[index].source_type = (MusicSourceType)sqlite3_column_int(stmt, 4);
        items[index].is_downloaded = sqlite3_column_int(stmt, 5) != 0;
        items[index].appNo = sqlite3_column_int(stmt, 6);
        items[index].sndTime = sqlite3_column_int(stmt, 7);

        index++;
    }

    // 确保实际获取的数量正确
    *count = index;

    sqlite3_finalize(stmt);
    return items;
}



static void build_filter_sql(const CollectionFilter* filter, char* sql, size_t sql_size) {
    sql[0] = '\0';
    if (filter == NULL)
        return;
    ESP_LOGI(TAG, "build_filter_sql\n");
    bool first_condition = true;

    if (filter->path) {
        if (!first_condition) strncat(sql, " AND ", sql_size - strlen(sql) - 1);
        strncat(sql, "path LIKE '%", sql_size - strlen(sql) - 1);
        strncat(sql, filter->path, sql_size - strlen(sql) - 1);
        strncat(sql, "%'", sql_size - strlen(sql) - 1);
        first_condition = false;
    }

    if (filter->title) {
        if (!first_condition) strncat(sql, " AND ", sql_size - strlen(sql) - 1);
        strncat(sql, "title LIKE '%", sql_size - strlen(sql) - 1);
        strncat(sql, filter->title, sql_size - strlen(sql) - 1);
        strncat(sql, "%'", sql_size - strlen(sql) - 1);
        first_condition = false;
    }

    if (filter->source_type != -1) {
        if (!first_condition) strncat(sql, " AND ", sql_size - strlen(sql) - 1);
        char source_type_str[20];
        snprintf(source_type_str, sizeof(source_type_str), "source_type = %d", filter->source_type);
        strncat(sql, source_type_str, sql_size - strlen(sql) - 1);
        first_condition = false;
    }

    if (filter->is_downloaded) {
        if (!first_condition) strncat(sql, " AND ", sql_size - strlen(sql) - 1);
        strncat(sql, "is_downloaded = 1", sql_size - strlen(sql) - 1);
    }
}

/// <param name="res_genre">资源来源</param>
/// <param name="url_path">资源url</param>
/// <returns>true :资源在数据表中</returns>
bool check_url_in_db(e_res_store res_genre, const char* url_path) {
    //CollectionType type,
    //    uint16_t appNo,
    //    const CollectionFilter* filter,
    //    uint32_t index)
    const CollectionFilter filter = {
        .path = url_path,
        .source_type = res_genre
    };
    CollectionType type = COLLECTION_TYPE_MUSIC;
    if (res_genre == RES_CG_ONLINE)
        type = COLLECTION_TYPE_ALBUM;

    int cnt = collection_service_get_count_with_filter(type, C_ZB_AP_MY_DOWNLOAD, &filter);
    ESP_LOGI(TAG, "check_url_in_db count = %d\n", cnt);
    return true ?cnt>0:false;
}
#endif
/**
 * @brief 释放由 collection_service_get_all_items_with_filter 返回的数组
 *
 * @param items 数组指针
 * @param count 数组长度
 */
void collection_service_free_items_array(CollectionItem* items, uint32_t count)
{
    if (!items) return;

    for (uint32_t i = 0; i < count; i++) {
        if (items[i].path) free(items[i].path);
        if (items[i].title) free(items[i].title);
    }
    free(items);
}

