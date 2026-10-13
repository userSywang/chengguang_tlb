
#ifndef _MISC_UITLS_H_
#define _MISC_UITLS_H_

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
//#include "../collect/collection_manager.h"
#include "../data_support/db_manager.h" 
#include "../zb_base.h"
#ifndef C_PC_SIM
//#include "../include/sqlite3.h"
#endif
#ifdef C_PC_SIM
#include <sysinfoapi.h>
#endif

#include <lvgl/src/misc/lv_types.h>

#ifndef C_PC_SIM 
//#include <syslog.h>

extern char * print_sys_time(void);
#include "../../trd/tz_sys_lib/include/mlog.h"

#else
#include <lvgl/src/misc/lv_log.h>
#endif


#include <lvgl/src/misc/lv_timer.h> 
//增
#define _MAX_PATH 1024
#define CONFIG_FATFS_MAX_LFN 1024
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480

#ifndef C_PC_SIM
//#define LV_KEY_EX_FUNC1           0xe002
//#define LV_KEY_EX_RECORD          0xe003
extern CollectionService* service;


#endif

#define M_ARRAY_LEN(arr)  (sizeof(arr)/sizeof(arr[0]))


//---------------------------------------------- 

#define M_BIT_SET(REG,POS)     ((REG) |= (1 << (POS)))
#define M_BIT_CLR(REG,POS)     ((REG) &= (~(1 << (POS))))
#define M_BIT_XOR(REG,POS)     ((REG) ^= (~(1 << (POS))))
#define M_BIT_TEST(REG,POS)    ((REG) & (1 << (POS)))

#define M_ARRAY_LEN(arr)  (sizeof(arr)/sizeof(arr[0]))

#define M_ALIGN_4BYTE(size)   (((size)+3)&0xfffffffc)


#define M_OFFSET_OF(type, memb) ((unsigned int)(&((type *)0)->memb)) 

#define M_CONTAINER_OF(ptr, type, memb) ((type *)((char *)(ptr) - M_OFFSET_OF(type, memb))) 


#define GET_UINT16_BIGENDIAN(buf)     ((uint16_t)(\
  ((uint16_t)*((uint8_t *)(buf)    ) << 8)\
| ((uint16_t)*((uint8_t *)(buf) + 1)     ) ))

#define GET_UINT32_BIGENDIAN(buf)    ((uint32_t)(\
  ((uint32_t)*((uint8_t *)(buf)    ) << 24)\
| ((uint32_t)*((uint8_t *)(buf) + 1) << 16)\
| ((uint32_t)*((uint8_t *)(buf) + 2) << 8 )\
| ((uint32_t)*((uint8_t *)(buf) + 3)      ) ))

#define GET_UINT16_LITTLEENDIAN(buf)  ((uint16_t)(\
  ((uint16_t)*((uint8_t *)(buf)    )     )\
| ((uint16_t)*((uint8_t *)(buf) + 1) << 8) ))

#define GET_UINT32_LITTLEENDIAN(buf) ((uint32_t)(\
  ((uint32_t)*((uint8_t *)(buf)    )      )\
| ((uint32_t)*((uint8_t *)(buf) + 1) << 8 )\
| ((uint32_t)*((uint8_t *)(buf) + 2) << 16)\
| ((uint32_t)*((uint8_t *)(buf) + 3) << 24) ))


//----------------------------------------------
/* AP No define */
#define C_ZB_AP_MAINMENU    1  // mainMenu 


#define C_ZB_AP_PRODUCT_TST 2  // 产测 目前无定义使用
#define C_ZB_AP_PLAYER     2   //播放器
#define C_ZB_AP_SETTING     3  // 
#define C_ZB_AP_INSIDETEST  4  // testAP, for test some func, NOT for release version. 
#define C_ZB_AP_NO_CONTACT  5 // 通迅录和打电话 

#define C_ZB_AP_EXAMPLE_B   6  // exmaple AP B, ...  NOT for release version. 
#define C_ZB_AP_BOOK_SELECT 7  // 教材选择 
#define C_ZB_AP_MY_DOWNLOAD     8  //我的下载

#define C_ZB_AP_LISTENAI_CHAT	9 	//儿童聊天
#define C_ZB_AP_PINYIN_STUDY	10 	//拼音学习
#define C_ZB_AP_COLLECT         11  //我的收藏 
#define C_ZB_AP_NO_RECENTLYPLAYED  12 //最近播放 
#define C_ZB_AP_NO_STUDYWORD   13 //单词学习 
#define C_ZB_AP_SEARCH         14 //搜索 
#define C_ZB_AP_ALIPAY          15 //支付宝

#define C_ZB_AP_BD_NET_DISK     16  //百度网盘 
#define C_ZB_AP_UCC_A           17  //认识汉字的年级
#define C_ZB_AP_PHONICS_A       18  //自然拼读的音类型
#define C_ZB_AP_EAR_PLAN_A      19  //磨耳朵计划1  本周计划


#define C_ZB_AP_PLAYER_TYPE_START   21
#define C_ZB_AP_AUD_PLAYER      21 
#define C_ZB_AP_TEXTBOOK_ENG        22  // (英语/语文)教材 
#define C_ZB_AP_TEXTBOOK_CN         23  
#define C_ZB_AP_PLAYER_TYPE_END     24
#define C_ZB_AP_UPD_CODE_BY_FILE 23
#define C_ZB_AP_PARENTAL_PIN    24
#define C_ZB_AP_DICT_EC         25
#define C_ZB_AP_ER_GE           26
#define C_ZB_AP_QING_JING       27
#define C_ZB_AP_JIAN_QIAO       28
#define C_ZB_AP_WAI_JIAO        29
#define C_ZB_AP_YINYUMINZHU     30
#define C_ZB_AP_DAYUWEN         31
#define C_ZB_AP_KEWAIGUSHI      32
#define C_ZB_AP_GUOXUEXUNTIN      33
#define C_ZB_AP_GUSHISUITIN      34
#define C_ZB_AP_DICTIONARY      35
#define C_ZB_AP_FENJIYUEDU      36
#define C_ZB_AP_CLOUD_DISK      37


#define C_ZB_INVALID_AP_NO  0

// 判断是否是 会启动背景播放的 AP 
#define M_IS_AP_START_AUD_BG(apNo)  ((apNo)>=C_ZB_AP_PLAYER_TYPE_START && (apNo)<=C_ZB_AP_PLAYER_TYPE_END)

//----------------------------------------------
enum
{
    DATUM_ENGLISH_TEXTBOOK = 0,   // 英语教材
    DATUM_OLD_POEM,         // 古诗古文
    DATUM_FAMOUS_BOOK,      // 必读名著
    DATUM_CHILD_SONG,       // 儿歌
    DATUM_STORY_KINGDOM,    // 故事王国
    DATUM_CHINESE_CLASSIC,  // 国学熏听
    DATUM_BEAUT_ESSAY,      //  美文美句
    DATUM_PAULIN_UNCLE,     // 保林叔叔
    DATUM_CHINESE_TEXTBOOK, // 语文教材
    DATUM_HYNEMAN_READ,     // 海尼曼阅读 
    DATUM_HISTORICAL_STORY, // 历史故事 
    DATUM_IDIOM_PARADISE,   // 成语乐园 
    DATUM_BAIKE_ZHISHI,     // 百科知识 
    DATUM_CAISHANG_JIAOYU,  // 财商教育 
    DATUM_SITUATIONAL_DIALOGUE,   // 情景对话 
};


// 设置的子项 
#define		NETWORK_SETTING			1	//网络设置
#define		BRIGHTNESS_SETTING		2	//亮度设置
#define		AUTO_ONOFF_SETTING		3	//定时关机
#define		UPDATE_SETTING			4	//系统升级
#define		STORE_SPACE_SETTING		5	//存储空间
#define		MY_DEV_SETTING			6	//我的设备


//----------------------------------------------

#define ZB_WORK_DIR    "/userdata/_data"          // 系统工作文件夹 
#define ZB_WORK_DIR_WITH_SEP  ZB_WORK_DIR "/"   // 系统工作文件夹(尾部带/ )

#ifndef C_PC_SIM
#define ZB_PRESET_DATA_DIR  "/resource/sysData"   // 系统预置数据文件夹 
#else
#define ZB_PRESET_DATA_DIR  "/userdata/sysData"   // 系统预置数据文件夹 
#endif
#define ZB_PRESET_DATA_DIR_WITH_SEP  ZB_PRESET_DATA_DIR "/"  // 系统预置数据文件夹(尾部带/ )



#define LOW_POWER_DLG_CONFIRM_CODE  10723

#define DARK_MODE 1  //样式切换 全局

//----------------------------------------------
// global param value for EVENT_EX_CMD 
#define PLY_CMD_PLY_LST_RET 0xe0   // 从音频播放器的列表版面返回 
#define PLY_CMD_PLY_TXT_RET 0xe2   // 从音频播放器的播放版面返回 

#define NET_CONN_CMD_CODE   0xf1   // 从网络设置版面返回 

//----------------------------------------------

typedef void (*zb_func_void)(void);


#define zb_requestSwitchAp(apNoNew)  zb_requestSwitchApIn(apNoNew, 0)

extern void zb_requestSwitchApIn(uint8_t apNoNew, uint16_t exParm);

extern uint8_t zb_getApNo(void);

/** 读取 AP (初始化时)入口参数 
 */
extern uint16_t zb_getApInParm(void);


extern void zb_ex_cmd_timer_cb(lv_timer_t * timer);

//----------------------------------------------

extern const char * g_p____bar;


#if 1

#define LOG_LOCAL_LEVEL  3

typedef enum {
    ESP_LOG_USER,       /* */
    ESP_LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
    ESP_LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
    ESP_LOG_INFO,       /*!< Information messages which describe normal flow of events */
    ESP_LOG_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    ESP_LOG_VERBOSE     /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} esp_log_level_t;



#define ESP_LOG_LEVEL_LOCAL(level, tag, format, ...) do {               \
        if (level <= LOG_LOCAL_LEVEL) _lv_log_add((4-level), __FILE__, __LINE__, tag, format, ##__VA_ARGS__); \
    } while(0)


#endif


#ifndef C_PC_SIM

#define ESP_LOGE(tag,format,... )  \
    do { \
        printf(RED "%s %s::%s[%d] " NONE format "\n", print_sys_time(), __FILENAME__, tag, __LINE__, \
               ##__VA_ARGS__);                                                                 \
        fflush(stdout);   \
    } while (0)

#define ESP_LOGW(tag,format,... )  \
    do { \
        printf(YELLOW "%s %s::%s[%d] " NONE format "\n", print_sys_time(), __FILENAME__, tag, __LINE__, \
               ##__VA_ARGS__);                                                                  \
        fflush(stdout);   \
    } while (0)

#define ESP_LOGI(tag,format, ...)  \
    do { \
        printf("%s %s::%s[%d] " NONE format "\n", print_sys_time(), __FILENAME__, tag, __LINE__, \
               ##__VA_ARGS__);                                                                  \
        fflush(stdout);  \
    } while (0)

#define ESP_LOGD(tag,format, ...)  \
    do { \
        printf(GREEN "%s %s::%s[%d] " NONE format "\n", print_sys_time(), __FILENAME__, tag, __LINE__, \
               ##__VA_ARGS__);                                                                  \
        fflush(stdout);   \
    } while (0)

#else
#define ESP_LOGE( tag, format, ... ) ESP_LOG_LEVEL_LOCAL(ESP_LOG_ERROR,   tag, format, ##__VA_ARGS__)
#define ESP_LOGW( tag, format, ... ) ESP_LOG_LEVEL_LOCAL(ESP_LOG_WARN,    tag, format, ##__VA_ARGS__)

#define ESP_LOGI( tag, format, ... ) ESP_LOG_LEVEL_LOCAL(ESP_LOG_INFO,    tag, format, ##__VA_ARGS__)
#define ESP_LOGD( tag, format, ... ) ESP_LOG_LEVEL_LOCAL(ESP_LOG_DEBUG,   tag, format, ##__VA_ARGS__)
#endif



/* 内存申请 (可以精细控制, 比如有性能要求、地址对齐要求 等)
 */
void * heap_caps_malloc(size_t size, uint32_t flag);


void * heap_caps_realloc(void * ptr, size_t size, uint32_t flag);


void heap_caps_free(void * ptr) ;



/* 缺省的内存申请 (没有特别要求)
 */
void * heap_caps_malloc_dflt(size_t size);

void * heap_caps_realloc_dflt(void * ptr, size_t size);

void zb_mem_print(void);


//----------------------------------------------
#define ZB_MAX_PATH  260  // 最大允许的文件路径长度(含结尾\0)


// 打开文件 
FILE * zb_fopen(const char * filePath, const char * mode);

int zb_fread(void * ptr, int size, int nmemb, FILE * stream);

int zb_fwrite(void * ptr, int size, int nmemb, FILE * stream);

#define zb_fseek    fseek

// 关闭文件 
int zb_fclose( FILE *fp );


/** 获取当前打开的文件句柄数 
 *  Note: 主要用于 PC 仿真上, 要求打开、关闭文件句柄得使用  zb_open()、zb_fclose() 
 */
int zb_getOpenedFileNum();


/** 创建文件夹 
 * 每次只能创建一层 
 */
int zb_mkdir(const char *path, int mode);

/** 删除文件或文件夹 
 */
int zb_remove(const char *path);


/** 删除文件或文件夹, 支持非空文件夹
 */
int zb_del_dir(const char * path);
int zb_delete_directory(const char * path);


/** 删除文件夹 (必须是空文件夹)
 */
int zb_rmdir(const char *path);


/** 文件改名
 */
int zb_rename(const char *old_name, const char *new_name);


int zb_access(const char *pathname, int mode);

/** 判断文件是否存在
 * @return 非0 表示存在 
 */
#define zb_isFileExist(path)   (zb_access(path, 0) == 0)


/** (按路径)获取文件的各种属性标志(比如 S_IFREG、S_IFDIR)
 */
int zb_stat__st_mode(const char *pathname, int * p_st_mode);


int64_t zb_stat__mtime(const char *pathname);


/** 获取文件长度，单位:字节 
 */
int64_t zb_getFileLength(const char *pathname);

int sys_get_sdcard_capacity( uint64_t * total_space , uint64_t * free_space);

//----------------------------------------------

/** 同步系统时间 (需要在连网状态)
 *  can not use int PC_SIM mode 
 */
extern void  sys_time_sync_init_ex(void);



/** 查询当前时刻 (开机以前的时长, 单位为 us)
 * 用于小机模式, obsolete 
 */
extern int64_t esp_timer_get_time(void);


// 获取开机以来的时长(单位 ms)
extern int32_t zb_get_time_ms_since_boot(void);

#ifndef C_PC_SIM
#define zb_get_time_us_since_boot()  0 //(esp_timer_get_time())

#else
extern int64_t zb_get_time_us_since_boot(void);
#endif


//----------------------------------------------
/** 
 * Note: caller need free the return buf. 
 */
char * base64_encode(const void *src, size_t len, size_t *out_len);

char * zb_base64_encode(const void *src, size_t len, size_t *out_len);
char * zb_base64_url_encode(const void *src, size_t len, size_t *out_len);

//----------------------------------------------

// CRC32 计算 
uint32_t  esp_rom_crc32_le(uint32_t crc, uint8_t const * buf, uint32_t len);

#ifndef _ROM_MD5_HASH_H_
struct MD5ContextEx {
	uint32_t buf[4];
	uint32_t bits[2];
	uint8_t in[64];
};

void MD5InitEx(struct MD5ContextEx * context);
void MD5UpdateEx(struct MD5ContextEx * context, unsigned char const * buf, unsigned len);
void MD5FinalEx(unsigned char digest[16], struct MD5ContextEx * context);
#endif

//--------------------------

int hmac_sha1(const uint8_t *key, size_t key_len, const uint8_t *data, size_t data_len, uint8_t *mac);


//----------------------------------------------
#ifdef C_PC_SIM
#define strtok_r  strtok_s

#define strcasecmp stricmp
#endif


#ifdef C_PC_SIM
extern size_t strlcpy(char *dst, const char *src, size_t dstBufLen);
#endif

extern char * zb_strdup(const char * s);

extern int zb_str_replace(const char * pcSrc, char * pcDest, int bufLen, const char * pcSubOld, const char * pcSubNew);


// 把 byte数组 (按码值)转为 hex 字串 
extern int zb_bin2hex(const char *bin, size_t len, char* hexBuf, int bufSize);


//----------------------------------------------
#define LAYER_FLG_RCV_AUD_EVT  1
#define LAYER_FLG_FULL_SCR     8


#ifdef LV_GROUP_H
/** (屏幕层栈里)添加一个屏幕层 
 * @parm  parm 的值 LAYER_FLG_RCV_AUD_EVT ... 
 * @return 负值表示失败 
 */
extern int zb_group_push(lv_group_t * group, int16_t parm);

/** (屏幕层栈里)弹出一个屏幕层 
 * @parm doesDelGroup  true 表示需要该函数来销毁相应 group 
 * @return 负值表示失败, >= 0 表示 group 总层数;
 */
extern int zb_group_pop(bool doesDelGroup);

extern bool zb_is_groupOnTopLayer(lv_group_t * group);


extern bool zb_screen_layer_is_full(void);

#endif


/** 判断 AP 层界面是否被其他系统版面完全盖住 (是否被响闹之类全屏间版面完全盖住);
 */
bool zb_isApScreenFullCovered(void);



bool zb_is_wifi_connect(void);




/**
 * @brief 获取后台播放器状态
 * @return -1 :表示没有后台播放
 *		  0，1，2，3，4，5表示播放状态
 */
extern int get_player_backstage_state(void);

/** 停止后台播放 
 */
extern int play_text_page_free(void);

//
void* heap_caps_malloc(size_t size, uint32_t flag);
void* heap_caps_realloc(void* ptr, size_t size, uint32_t flag);
void heap_caps_free(void* ptr);
void* heap_caps_malloc_dflt(size_t size);
void* heap_caps_realloc_dflt(void* ptr, size_t size);


#endif

