#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif
#ifndef C_PC_SIM
#include "LvR9/trd/sql3/include/sqlite3.h"
#endif
#include <stdbool.h>
#include <stdint.h>
#include "../data_support/res_type.h"

    /**
     * @brief 收藏类型枚举
     */
    typedef enum {
        COLLECTION_TYPE_MUSIC,
        COLLECTION_TYPE_ALBUM
    } CollectionType;


    /**
 * @brief 音乐来源类型
 */
    //typedef enum {
    //    MUSIC_SOURCE_BUILT_IN = 0,    // 内置资源
    //    MUSIC_SOURCE_THIRD_PARTY = 1,  // 第三方资源
    //    MUSIC_SOURCE_YOUDAO = 2        // 有道资源
    //} MusicSourceType;
    typedef e_res_store MusicSourceType;
    /**
     * @brief 收藏项结构
     *
     * 新增 is_online 字段标识在线资源
     */
    typedef struct {
        char* path;         ///< 文件路径或在线URL
        char* title;        ///< 标题
        uint16_t playNum;   ///< 播放次数
        uint32_t timestamp; ///< 收藏时间戳
        MusicSourceType source_type;  ///< 资源来源
        bool is_downloaded;
        uint16_t appNo;     /// 数据来源模块
        uint32_t sndTime;   /// 音频播放时长，单位秒
    } CollectionItem;

    // 收藏服务句柄
    typedef struct CollectionService CollectionService;

    typedef struct CollectionFilter {
        const char* path;           // 文件路径筛选条件
        const char* title;          // 标题筛选条件
        MusicSourceType source_type; // 资源来源类型筛选条件
        bool is_downloaded;         // 是否下载筛选条件
    } CollectionFilter;


    // ========================= 生命周期管理 ========================= //

    /**
     * @brief 创建收藏服务实例
     *
     * @param db_path SQLite3 数据库文件路径
     * @return CollectionService* 服务实例句柄
     */
    CollectionService* collection_service_create();

    /**
     * @brief 销毁收藏服务实例
     */
    void collection_service_destroy(CollectionService* service);

    // ========================= 核心功能接口 ========================= //

    /**
     * @brief 添加或更新收藏项
     *
     * @param is_online 新增参数标识是否在线资源
     */
    bool collection_service_add_item(
        //CollectionService* service,
        CollectionType type,
        const char* path,
        const char* title,
        uint16_t playNum,
        MusicSourceType source_type,
        bool is_downloaded,
        uint16_t appNo,
        uint32_t sndTime
        );

    /**
     * @brief 删除收藏项
     */
    bool collection_service_remove_item(
        //CollectionService* service,
        CollectionType type,
        const char* path,
        uint16_t appNo);

    /**
     * @brief 批量删除收藏项
     *
     * @param paths 路径数组
     * @param count 路径数量
     */
    bool collection_service_remove_items(
        //CollectionService* service,
        CollectionType type,
        const char** paths,
        uint32_t count,
        uint16_t appNo);

    /**
     * @brief 获取收藏项数量
     */
    uint32_t collection_service_get_count(
        //CollectionService* service,
        CollectionType type,
        uint16_t appNo);

    // ========================= 数据访问接口 ========================= //

    /**
     * @brief 获取指定位置的收藏项
     *
     * 注意: 返回的指针在下次调用前有效
     */
    const CollectionItem* collection_service_get_item(
        //CollectionService* service,
        CollectionType type,
        uint32_t index,
        uint16_t appNo);

    /**
     * @brief 获取指定位置的文件路径
     */
    const char* collection_service_get_path(
        //CollectionService* service,
        CollectionType type,
        uint32_t index,
        uint16_t appNo);

    /**
     * @brief 获取指定位置的标题
     */
    const char* collection_service_get_title(
        //CollectionService* service,
        CollectionType type,
        uint32_t index,
        uint16_t appNo);

    /**
     * @brief 查找收藏项
     */
    const CollectionItem* collection_service_find_item(
        //CollectionService* service,
        CollectionType type,
        const char* path,
        uint16_t appNo);

    // ========================= 排序功能 ========================= //

    /**
     * @brief 设置排序方式
     *
     * @param order_by SQL ORDER BY 子句 (e.g. "timestamp DESC", "playNum DESC")
     */
    void collection_service_set_sort_order(
        //CollectionService* service,
        CollectionType type,
        const char* order_by);

    // ========================= 持久化操作 ========================= //

    /**
     * @brief 执行事务
     *
     * @param exec_func 事务执行函数
     * @param user_data 用户数据
     */
    bool collection_service_transaction(
        //CollectionService* service,
        bool (*exec_func)(CollectionService*, void*),
        void* user_data);

    /**
     * @brief 导出数据到JSON文件
     */
    bool collection_service_export_to_json(
        //CollectionService* service,
        uint16_t appNo,
        const char* json_path);

    /**
     * @brief 从JSON文件导入数据
     */
    bool collection_service_import_from_json(
        //CollectionService* service,
        CollectionType type,
        const char* json_path);

    /**
 * @brief 设置资源来源类型
 */
    bool collection_service_set_source_type(
        //CollectionService* service,
        CollectionType type,
        const char* path,
        MusicSourceType source_type,
        uint16_t appNo);

    /**
     * @brief 获取资源来源类型
     */
    MusicSourceType collection_service_get_source_type(
        //CollectionService* service,
        CollectionType type,
        const char* path,
        uint16_t appNo);

    /**
 * @brief 根据筛选条件获取收藏项数量
 *
 * @param service 收藏服务实例句柄
 * @param type 收藏类型
 * @param filter_sql 筛选条件的 SQL 语句（如 "title LIKE '%keyword%'"）
 * @return uint32_t 符合条件的收藏项数量
 */
    uint32_t collection_service_get_count_with_filter(
        //CollectionService* service,
        CollectionType type,
        uint16_t appNo,
        const struct CollectionFilter* filter);

    /**
     * @brief 根据筛选条件获取指定位置的收藏项
     *
     * 注意: 返回的指针在下次调用前有效
     *
     * @param service 收藏服务实例句柄
     * @param type 收藏类型
     * @param filter_sql 筛选条件的 SQL 语句（如 "title LIKE '%keyword%'"）
     * @param index 指定位置的索引
     * @return const CollectionItem* 符合条件的收藏项指针
     */
    const CollectionItem* collection_service_get_item_with_filter(
        //CollectionService* service,
        CollectionType type,
        uint16_t appNo,
        const struct CollectionFilter* filter,
        uint32_t index);

    CollectionItem* collection_service_get_all_items_with_filter(
        //CollectionService* service,
        CollectionType type,
        uint16_t appNo,
        const CollectionFilter* filter,
        uint32_t* count);

    void collection_service_free_items_array(CollectionItem* items, uint32_t count);


bool add_to_collection(CollectionType type, const char* path, const char* title, MusicSourceType source_type,uint16_t appNo,uint32_t sndTime);
static void build_filter_sql(const CollectionFilter* filter, char* sql, size_t sql_size);

/// <param name="res_genre">资源来源</param>
/// <param name="url_path">资源url</param>
/// <returns>true :资源在数据表中</returns>
bool check_url_in_db(e_res_store res_genre, const char* url_path);

#ifdef __cplusplus
}
#endif

#endif