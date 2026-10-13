#ifndef _GLOBAL_PARM_H_
#define _GLOBAL_PARM_H_

#include "misc_utils.h"
#ifndef C_PC_SIM
#include "LvR9/trd/ubus/include/tz_hal_quectel.h"
#endif
#include "../src/wifi/wifi_connect.h"
//#include "../setting/net_connect.h"


#define ZB_PROD_SN_LEN 	21       // 产品序列号字串长度(字节数)

#define ZB_BOOK_PUBL_STR_BUF_LEN  40


//typedef struct quectel_handler quectel_handler_t;
#ifndef C_PC_SIM
extern quectel_handler_t* state_handler;
#endif

typedef struct { 
	int8_t m_grade;  // 年级, 1表示小学一年级 
	int8_t reserve;
	char acPublishVer[ZB_BOOK_PUBL_STR_BUF_LEN];
}  grade_with_publish_t;  // (某一科目的)年级出版社信息 


typedef struct {  //结构体长度保证为双数
	char  identify_str[10]; //标识符 "wifi list"
	int8_t reserveA[2];
	wifi_recent_info_t rec_list[NETWORK_RECENT_LIST_MAX_NUM];
	uint16_t checksum;   //数据效验和
} prj_global_network_parm_t;				    // 项目全局参数(需要存盘的) 

//----------------------------------------------


typedef struct { 
	int16_t  m_identifierByVersion; // 全局参数格式标识 
	int16_t  m_volumeLevel;		// 音量级别 
	int16_t  m_scrBrightness;	// 屏幕亮度 
	int16_t  m_autoOffTime;		// 自动关屏时长(秒);
	int16_t  m_autoOffcheck;	// 自动关机复选框位置(自0起的选项号);
	int8_t	 m_networkSw;		// wifi 开关，0 --关，1--开; 
	int8_t  m_reseverA[5];
	grade_with_publish_t m_gradePublish[2];  // 中英文教材的年级、出社信息.
	//[------ 这一组响闹的参数只在休眠之前保存到文件,再次唤醒时读出 
	int64_t  m_alarmTmPt;  // 下次响闹的时刻 
	int8_t   m_alarmAudKind;   // 响闹音乐素材类别 
	int8_t   m_alarmNoForOnceMode; // 最近一次响闹的 planNo (仅用于onceMode 时)
	int16_t  m_alarmAudBookNo; // 响闹音乐专辑号 
	int16_t  m_reseverB[4];
	//]------ 
	volatile int16_t  m_studyTimeMinute;  // 学习时长(分钟)
	volatile int16_t  m_listenArticles;  // 精听篇数 
	volatile int16_t  m_listenRepeatCnt; // 跟读次数 
} prj_global_parm_t;    // 项目全局参数(需要存盘的) 


typedef enum { 
    ZB_PARM_VOLUME,         // 音量级别 
    ZB_PARM_SCR_BRIGHTNESS, // 屏幕亮度 
    ZB_PARM_SCR_AUTO_OFF_TIME,  // 自动关屏时长 
    ZB_PARM_WIFI_SW,      	// 网络开关  
    ZB_PARM_WIFI_RECENT_LIST,  // WIFI 最近连接列表
    ZB_PARM_WIFI_SSID,      // WIFI 路由器标识 
    ZB_PARM_WIFI_PWD,       // 路由器口令 
    ZB_PARM_POWEROFF_FLAG,  //自动关机时长 
    ZB_PARM_BOOK_GARDE_CHN,  // 中文教材年级
    ZB_PARM_BOOK_GARDE_ENG,  // 英文教材年级 
    ZB_PARM_BOOK_PUBL_CHN,   // 中文教材出版社
    ZB_PARM_BOOK_PUBL_ENG,   // 英文教材出版社
    ZB_PARM_ALARM_TM_PT,     // 响闹时刻时间戳 
    ZB_PARM_ALARM_AUD_DAT_KIND, // 响闹音乐素材类别 
    ZB_PARM_ALARM_AUD_BOOK_NO,  // 响闹音乐书本编号 
    ZB_PARM_ALARM_NO_FOR_ONCEMODE, // 最近一次响闹的 planNo (仅用于onceMode 时) 
    ZB_PARM_STUDY_TIME_MINUTE,  // 学习时长(分钟)
    ZB_PARM_LISTEN_ARTICLES,    // 精听篇数 
    ZB_PARM_LISTEN_REPEAT_CNT,  // 跟读次数 
    ZB_PARM_NO_MAX_NEXT,
} SYS_PARM_TYPE;


#define PARM_VOLUME_MIN    0
#define PARM_VOLUME_MAX    20
#define PARM_VOLUME_DFLT   5
#define MAX_VOL  PARM_VOLUME_MAX 


#define PARM_VOLUME_GLB_TO_DRV(volNo) ((volNo)*5)
#define PARM_VOLUME_DRV_TO_GLB(volDrvVal) (volDrvVal/5)


wifi_recent_info_t * getPrjNetworkRecentList(void);

uint16_t  getPrjNetworkSWstate(void);


int transferPrjGlobalParmIfNeed(void);


/** 从相应磁盘文件里 加载产品全局参数 
 * 调用时机: 系统开机时调用， 或者 保存参数前的比较时调用; 
 */
int loadPrjGlobalParm(prj_global_parm_t * pGlbParm);



/** 把当前全局参数写入磁盘 (为了避免过多的写盘动作,本函数会先判断参数跟原来存入的是否相同). 
 * 调用时机: (一般是)关机时, 或者 从设置AP退出时. 
 */
void savePrjGlobalParm(void);



/* 使得项目全局参数生效 
 * 调用时机: 开机后刚 load 出全局参数; 或者 从设置AP退出时. 
 */
void globalParmTakeEffect(void);


/** 读取指定的项目全局参数 (自动关机时间、音量... 等)
 */
int16_t getPrjGlobalParm(SYS_PARM_TYPE type, int16_t bufLen, void * buf);


/** 设置指定的项目全局参数
 */
int16_t setPrjGlobalParm(SYS_PARM_TYPE type, int16_t bufLen, void * buf);


//==========================================================================
#define WRT_SN_CONFIRM_CODE  28901 

/** 读取产品序列号 
 * @return  > 0 means success.
 * @note 建议调用者 再调用 strupr()统一为大写后再使用. 
 */
int zb_readProductSerialNo(char * buf, int bufLen);

/** 写入产品序列号 
 * @param  buf 要写入的数据 
 * @param comfirmCode 只能为 WRT_SN_CONFIRM_CODE(即 28901) (安全验证,防止误调用)
 * @return  > 0 means success.
 * @note 一般只能是产测才允许写序列号 
 */
int zb_writeProductSerialNo(const char * buf, int16_t comfirmCode);

//==================================================
//读wifi 名称密码列表文件
int load_wifi_data(prj_global_network_parm_t* p_wifi_parm);

//===========================================
//存储wifi 名称密码列表文件
void save_wifi_data(void);

#endif

