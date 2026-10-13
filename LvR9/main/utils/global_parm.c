/** 全局参数相关  
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef C_PC_SIM
//#include <sys/stat.h>
#else
#include <direct.h>
#endif

#ifndef C_PC_SIM
//#include "app_codec.h"
#endif


#include  "../zb_base.h"
#include "misc_utils.h"
#include "global_parm.h"

#ifndef C_PC_SIM
#include "../../trd/tz_sys_lib/include/cvi_tz_sys.h"
#endif


#define TAG  "glbPar"

//-------------------------------
#define		WIFI_LIST_IMDEPENDENT		1
//-------------------------------


#define PARM_BRIGHTNESS_MIN   10
#define PARM_BRIGHTNESS_MAX  255
#define PARM_BRIGHTNESS_DFLT 140

#define PARM_SCR_AUTO_OFF_TM_MIN  30   // 自动关屏时间 的最小值, unit: second 
#define PARM_SCR_AUTO_OFF_TM_MAX  300  // 自动关屏时间 的最大值 
#define PARM_SCR_AUTO_OFF_TM_DFLT 30

#define PARM_PWR_AUTOOFF_TM_MIN 3   // unit: minute 
#define PARM_PWR_AUTOOFF_TM_MAX 60
#define PARM_PWR_AUTOOFF_TM_DFLT 10


#define C_GLOBAL_PARM_MARK 0x1001  // 当 prj_global_parm_t 结构有任何变化时,请递增该常量. 


//----------------------------------------------
// 下面带 old 字样的类型 仅用于版本数据转移用.

typedef struct { 
	int8_t m_grade;  // 年级, 1表示小学一年级 
	int8_t reserve;
	char acPublishVer[40];
}  grade_with_publ_old_t;  // (某一科目的)年级出版社信息 

typedef struct {
	char  ssid[32]; 		//名称
	char  password[64]; 	//password
} wifi_recent_inf_old_t;


typedef struct { 
	int16_t	sw;   				// wifi 开关，0 --关，1--开;  
	wifi_recent_inf_old_t rec_list[5];
} prj_global_network_parm_old_t;				    // 项目全局参数(需要存盘的) 

typedef struct { 
	int16_t  m_identifierByVersion; // 全局参数格式标识 
	int16_t  m_volumeLevel;		// 音量级别 
	int16_t  m_scrBrightness;	// 屏幕亮度 
	int16_t  m_autoOffTime;		// 自动关屏时长(秒);
	int16_t  m_autoOffcheck;	// 自动关机复选框位置(自0起的选项号);
	grade_with_publ_old_t m_gradePublish[2];  // 中英文教材的年级、出社信息.
	prj_global_network_parm_old_t m_network; // 已作废(仅用于让老版本平滑升级)
	//[------ 这一组响闹的参数只在休眠之前保存到文件,再次唤醒时读出 
	int64_t  m_alarmTmPt;  // 下次响闹的时刻 
	int8_t   m_alarmAudKind;   // 响闹音乐素材类别 
	int8_t   m_alarmNoForOnceMode; // 最近一次响闹的 planNo (仅用于onceMode 时)
	int16_t  m_alarmAudBookNo; // 响闹音乐专辑号 
	//]------ 
	volatile int16_t  m_studyTimeMinute;  // 学习时长(分钟)
	volatile int16_t  m_listenArticles;  // 精听篇数 
	volatile int16_t  m_listenRepeatCnt; // 跟读次数 
} prj_global_parm_old_t;    // 项目全局参数(过渡处理,用于让老版本小机升级时 维持住 wifi 参数)


//----------------------------------------------

static const char* C_GLOBAL_WIFILIST_MARK = "wifi list"; //

static const char * g_glbParmPath = "/userdata/_data/_gblParam.bin";

static const char * g_wifi_recent_path = "/userdata/_data/_wifi_list.bin";

//------------------

static prj_global_parm_t g_stGlobalParm;

static prj_global_network_parm_t g_network_data;

// (从文件读出来后)确保全局参数值是有效的 
static int16_t correctGlobalParm(prj_global_parm_t * pGlbParm)
{
	int16_t	hasCorrect = 0;
	ESP_LOGI(TAG, "crctGlbParm");
	if ((pGlbParm->m_volumeLevel < PARM_VOLUME_MIN) || (pGlbParm->m_volumeLevel > PARM_VOLUME_MAX)) {
		pGlbParm->m_volumeLevel = PARM_VOLUME_DFLT;
		hasCorrect = 1;
	}

	if ((pGlbParm->m_scrBrightness < PARM_BRIGHTNESS_MIN) || (pGlbParm->m_scrBrightness > PARM_BRIGHTNESS_MAX)) {
		pGlbParm->m_scrBrightness = PARM_BRIGHTNESS_DFLT;
		hasCorrect = 1;
	}

	if ((pGlbParm->m_autoOffTime <  PARM_SCR_AUTO_OFF_TM_MIN) || (pGlbParm->m_autoOffTime >  PARM_SCR_AUTO_OFF_TM_MAX)) { 
		pGlbParm->m_autoOffTime = PARM_SCR_AUTO_OFF_TM_DFLT;
		hasCorrect = 1;
	}

	if ((pGlbParm->m_autoOffcheck < PARM_PWR_AUTOOFF_TM_MIN) || (pGlbParm->m_autoOffcheck > PARM_PWR_AUTOOFF_TM_MAX)) {
		pGlbParm->m_autoOffcheck = PARM_PWR_AUTOOFF_TM_DFLT;
		hasCorrect = 1;
	}
	// todo; correct other parm 
	return hasCorrect;
}


/** 从相应磁盘文件里 加载产品全局参数 
 * @return 0 正常数据 / 1 文件不存在或无效数据 
 * 调用时机: 系统开机时调用， 或者 保存参数前的比较时调用; 
 */
int loadPrjGlobalParm(prj_global_parm_t * pGlbParm)
{
	int iFuncRet = -1;
	prj_global_parm_t stGlobalParm = { 0 };

	ESP_LOGI(TAG, "%s, [%d] ",__FUNCTION__, __LINE__);
	ESP_LOGI(TAG, "loadGblPar A pGlbParm: %p\n", pGlbParm);
	if (!pGlbParm) {
		pGlbParm = &g_stGlobalParm;
	}
	ESP_LOGI(TAG, "prj_global_parm_t size: %d \n", sizeof(prj_global_parm_t));
	
	FILE * f_cfg = zb_fopen(g_glbParmPath, "rb");
	ESP_LOGI(TAG, "zb_fopen f_cfg : %p \n", f_cfg);
	if (f_cfg)
	{
		int ret = zb_fread(&stGlobalParm, 1, sizeof(prj_global_parm_t), f_cfg);
		ESP_LOGI(TAG, "rd ret %d", ret);
	} 
	else 
	{
		ESP_LOGW(TAG, "ldPrjGlb fail to open %s", g_glbParmPath);
	}
	if (f_cfg) {
		zb_fclose(f_cfg);  f_cfg = NULL;
	}

	if (stGlobalParm.m_identifierByVersion != C_GLOBAL_PARM_MARK) 
	{
		ESP_LOGW(TAG, "ldPrjGlb fail, idt 0x%hx", stGlobalParm.m_identifierByVersion);
		uint16_t identifierRd = stGlobalParm.m_identifierByVersion;
		memset(&stGlobalParm, 0, sizeof(prj_global_parm_t));
		stGlobalParm.m_identifierByVersion = C_GLOBAL_PARM_MARK;
		//---- 各参数取默认值
		stGlobalParm.m_volumeLevel = PARM_VOLUME_DFLT;
		stGlobalParm.m_scrBrightness = PARM_BRIGHTNESS_DFLT;
		stGlobalParm.m_autoOffTime = PARM_SCR_AUTO_OFF_TM_DFLT; //
		stGlobalParm.m_gradePublish[0].m_grade = 1;  // for yuwei
		//strcpy(stGlobalParm.m_gradePublish[0].acPublishVer, "人教新起点");

		stGlobalParm.m_gradePublish[1].m_grade = 1;  // for english
		iFuncRet = 1;
	} 
	else 
	{
		ESP_LOGI(TAG, "rdIdnt x%hx, aimIdnt x%hx", stGlobalParm.m_identifierByVersion, C_GLOBAL_PARM_MARK);
		correctGlobalParm(&stGlobalParm);
		iFuncRet = 0;
	}
	(*pGlbParm) = stGlobalParm;
	ESP_LOGI(TAG, "%s, [%d] Z",__FUNCTION__, __LINE__);
	return iFuncRet;
}


/** 把当前全局参数写入磁盘 (为了避免过多的写盘动作,本函数会先判断参数跟原来存入的是否相同). 
 * 调用时机: (一般是)关机时, 或者 从设置AP退出时. 
 */
void savePrjGlobalParm(void)
{
	prj_global_parm_t stGlobalParmOrg = { 0 };

	ESP_LOGI(TAG, "%s, [%d] ",__FUNCTION__, __LINE__);
	loadPrjGlobalParm(&stGlobalParmOrg);
	
	ESP_LOGI(TAG, "%s, [%d] ",__FUNCTION__, __LINE__);
	if (memcmp(&stGlobalParmOrg, &g_stGlobalParm, sizeof(prj_global_parm_t)) == 0)
	{
		ESP_LOGI(TAG, "glbPar no chg, need not save");
		return;
	}

	ESP_LOGI(TAG, "%s, [%d] ",__FUNCTION__, __LINE__);

	FILE * f_cfg = NULL;
	do {
		if (!zb_isFileExist(GLB_PARAM_PATH)) {
			zb_mkdir((GLB_PARAM_PATH), 0777);
		}
		f_cfg = zb_fopen(g_glbParmPath, "wb");
		if (!f_cfg) {
			ESP_LOGE(TAG, "Fail to crt fl %s", g_glbParmPath);
			break;
		}
		ESP_LOGI(TAG, "glbPar  zb_fwrite  g_stGlobalParm \r\n");
		zb_fwrite(&g_stGlobalParm, 1, sizeof(prj_global_parm_t), f_cfg);
		zb_fclose(f_cfg);  f_cfg = NULL;
		ESP_LOGI(TAG, "%s, [%d], ok ",__FUNCTION__, __LINE__);
	} while (0);

	
	return;
}


/* 使得项目全局参数生效 
 * 调用时机: 开机后刚 load 出全局参数; 或者 从设置AP退出时. 
 */
void globalParmTakeEffect(void)
{
	// todo;
	// autooffParm_takeEffect;

	int16_t bright = g_stGlobalParm.m_scrBrightness;
	if (bright < 10) bright = 10;
#ifndef C_PC_SIM
	tz_set_lcd_brightness(bright);

	int16_t volume = PARM_VOLUME_GLB_TO_DRV(g_stGlobalParm.m_volumeLevel);
	tz_play_set_volume(volume);
#endif
}
//================================================================[]
//================================================================[]
//wifi 获取最连接的列表
wifi_recent_info_t * getPrjNetworkRecentList(void)
{
#if WIFI_LIST_IMDEPENDENT
	return(g_network_data.rec_list);
#else
	return(g_stGlobalParm.m_network.rec_list);

#endif
}
//wifi 开关状态

uint16_t  getPrjNetworkSWstate(void)
{
#if WIFI_LIST_IMDEPENDENT
	return(g_stGlobalParm.m_networkSw);
#else
	return(g_stGlobalParm.m_network.sw);
	
#endif
}

void  setPrjNetworkSWstate(void)
{
#if WIFI_LIST_IMDEPENDENT
	g_stGlobalParm.m_networkSw = 1;
#else
	g_stGlobalParm.m_network.sw = 1;
#endif
}

void  clearPrjNetworkSWstate(void)
{
#if WIFI_LIST_IMDEPENDENT
	g_stGlobalParm.m_networkSw = 0;
#else
	g_stGlobalParm.m_network.sw = 0;
#endif
}


/* 读取指定的全局参数 (自动关机时间、亮度... 等)
 */
int16_t getPrjGlobalParm(SYS_PARM_TYPE type, int16_t bufLen, void * buf)
{
	if (type < 0 || type >= ZB_PARM_NO_MAX_NEXT) return 0;
	prj_global_parm_t stGlobalPara = g_stGlobalParm;
	int16_t wRet = 0;
	if (buf) {
		*(char *)buf = '\0';
	}
	switch (type) {
		case ZB_PARM_VOLUME:
			wRet = stGlobalPara.m_volumeLevel;
			break;
		case ZB_PARM_SCR_BRIGHTNESS:
			wRet = stGlobalPara.m_scrBrightness;
			break;
		case ZB_PARM_SCR_AUTO_OFF_TIME:
			wRet = stGlobalPara.m_autoOffTime;
			break;
		case ZB_PARM_WIFI_SW:
			wRet = g_stGlobalParm.m_networkSw;
			break;
		case ZB_PARM_POWEROFF_FLAG: {
				wRet = stGlobalPara.m_autoOffcheck;
			}
			break;
		case ZB_PARM_BOOK_GARDE_CHN: { 
			wRet = stGlobalPara.m_gradePublish[0].m_grade;
			break;
		}
		case ZB_PARM_BOOK_GARDE_ENG: { 
			wRet = stGlobalPara.m_gradePublish[1].m_grade;
			break;
		}
		case ZB_PARM_BOOK_PUBL_CHN: { 
			int datLen = strlen(stGlobalPara.m_gradePublish[0].acPublishVer);
			if (bufLen < datLen + 1)
				return -1;
			memcpy(buf, &stGlobalPara.m_gradePublish[0].acPublishVer, datLen + 1);
			break;
		}
		case ZB_PARM_BOOK_PUBL_ENG: { 
			int datLen = strlen(stGlobalPara.m_gradePublish[1].acPublishVer);
			if (bufLen < datLen + 1)
				return -1;
			memcpy(buf, &stGlobalPara.m_gradePublish[1].acPublishVer, datLen + 1);
			break;
		}
		case ZB_PARM_ALARM_TM_PT: {
			int datLen = sizeof(stGlobalPara.m_alarmTmPt);
			if (bufLen < datLen)
				return -1;
			memcpy(buf, &stGlobalPara.m_alarmTmPt, datLen);
			break;
		}
		case ZB_PARM_ALARM_AUD_DAT_KIND: {
			wRet = stGlobalPara.m_alarmAudKind;
			break;
		}
		case ZB_PARM_ALARM_AUD_BOOK_NO: {
			wRet = stGlobalPara.m_alarmAudBookNo;
			break;
		}
		case ZB_PARM_ALARM_NO_FOR_ONCEMODE: {
			wRet = stGlobalPara.m_alarmNoForOnceMode;
			break;
		}
		case ZB_PARM_STUDY_TIME_MINUTE: {
			wRet = stGlobalPara.m_studyTimeMinute;
			break;
		}
		case ZB_PARM_LISTEN_ARTICLES: {
			wRet = stGlobalPara.m_listenArticles;
			break;
		}
		case ZB_PARM_LISTEN_REPEAT_CNT: {
			wRet = stGlobalPara.m_listenRepeatCnt;
			break;
		}
		default:
			wRet = -1;
			break;
	}
	if ((type != ZB_PARM_SCR_AUTO_OFF_TIME) && (type != ZB_PARM_POWEROFF_FLAG)) {
		ESP_LOGI(TAG, "getPrjGlobalPara type %d, ret %d", (int)type, (int)wRet);
	}
	return wRet;
}



/* 设置指定的全局参数 (自动关机时间、亮度... 等)
 */
int16_t setPrjGlobalParm(SYS_PARM_TYPE type, int16_t bufLen, void * buf)
{
	if (type < 0 || type >= ZB_PARM_NO_MAX_NEXT) return 0;
	prj_global_parm_t *pstGlobalPara = &g_stGlobalParm;
	int16_t type_len = 0;

	switch (type) {
		case ZB_PARM_VOLUME:
			pstGlobalPara->m_volumeLevel = 0;
			type_len = sizeof(pstGlobalPara->m_volumeLevel);
			if(bufLen > type_len)
			{	
				bufLen = type_len;
			}			
			memcpy(&pstGlobalPara->m_volumeLevel, buf, bufLen);	
			
			break;
		case ZB_PARM_SCR_BRIGHTNESS:
			pstGlobalPara->m_scrBrightness = 0;
			type_len = sizeof(pstGlobalPara->m_scrBrightness);
			if(bufLen > type_len)
			{	
				bufLen = type_len;
			}			
			memcpy(&pstGlobalPara->m_scrBrightness, buf, bufLen);	
			ESP_LOGI(TAG, "setPrjGlobalPara type %d, m_scrBrightness :%d ", (int)type, pstGlobalPara->m_scrBrightness);
			
			break;
		case ZB_PARM_SCR_AUTO_OFF_TIME:  // 自动关屏时长 
			pstGlobalPara->m_autoOffTime = 0;
			type_len = sizeof(pstGlobalPara->m_autoOffTime);
			if(bufLen > type_len)
			{	
				bufLen = type_len;
			}			
			memcpy(&pstGlobalPara->m_autoOffTime, buf, bufLen);	
			ESP_LOGI(TAG, "setPrjGlobalPara type %d, m_autoOffTime :%d ", (int)type, pstGlobalPara->m_autoOffTime);
			
			break;
		case ZB_PARM_WIFI_SW:	
			g_stGlobalParm.m_networkSw = *(int8_t *)buf;
			break;
		case ZB_PARM_POWEROFF_FLAG: {  
			type_len = sizeof(pstGlobalPara->m_autoOffcheck);
			if (bufLen > type_len)
			{
				bufLen = type_len;
			}
			memcpy(&pstGlobalPara->m_autoOffcheck, buf, bufLen);
			ESP_LOGI(TAG, "setPrjGlobalPara type %d, m_autoOffcheck :%d ", (int)type, pstGlobalPara->m_autoOffcheck);
			break;
		}
		case ZB_PARM_BOOK_GARDE_CHN: { 
			type_len = sizeof(pstGlobalPara->m_gradePublish[0].m_grade);
			if (bufLen > type_len)
			{
				bufLen = type_len;
			}
			memcpy(&pstGlobalPara->m_gradePublish[0].m_grade, buf, bufLen);
			break;
		}
		case ZB_PARM_BOOK_GARDE_ENG: { 
			type_len = sizeof(pstGlobalPara->m_gradePublish[1].m_grade);
			if (bufLen > type_len)
			{
				bufLen = type_len;
			}
			memcpy(&pstGlobalPara->m_gradePublish[1].m_grade, buf, bufLen);
			break;
		}
		case ZB_PARM_BOOK_PUBL_CHN: { 
			type_len = sizeof(pstGlobalPara->m_gradePublish[0].acPublishVer);
			if (bufLen > type_len)
			{
				bufLen = type_len;
			}
			if (0 == bufLen) {
				pstGlobalPara->m_gradePublish[0].acPublishVer[0] = '\0';
				strncat(pstGlobalPara->m_gradePublish[0].acPublishVer, buf, type_len - 1);
			}
			else {
				memcpy(&pstGlobalPara->m_gradePublish[0].acPublishVer, buf, bufLen);
			}
			break;
		}
		case ZB_PARM_BOOK_PUBL_ENG: { 
			type_len = sizeof(pstGlobalPara->m_gradePublish[1].acPublishVer);
			if (bufLen > type_len)
			{
				bufLen = type_len;
			}
			if (0 == bufLen) {
				pstGlobalPara->m_gradePublish[0].acPublishVer[0] = '\0';
				strncat(pstGlobalPara->m_gradePublish[0].acPublishVer, buf, type_len - 1);
			}
			else {
				memcpy(&pstGlobalPara->m_gradePublish[1].acPublishVer, buf, bufLen);
			}
			break;
		}
		case ZB_PARM_ALARM_TM_PT: {
			type_len = sizeof(pstGlobalPara->m_alarmTmPt);
			if (bufLen > type_len) {
				bufLen = type_len;
			}
			memcpy(&pstGlobalPara->m_alarmTmPt, buf, bufLen);
			break;
		}
		case ZB_PARM_ALARM_AUD_DAT_KIND: { 
			pstGlobalPara->m_alarmAudKind = ((int8_t*)buf)[0];
			break;
		}
		case ZB_PARM_ALARM_AUD_BOOK_NO: { 
			pstGlobalPara->m_alarmAudBookNo = GET_UINT16_LITTLEENDIAN(buf);
			break;
		}
		case ZB_PARM_ALARM_NO_FOR_ONCEMODE: {
			pstGlobalPara->m_alarmNoForOnceMode = ((int8_t*)buf)[0];
			break;
		}
		case ZB_PARM_STUDY_TIME_MINUTE: {
			pstGlobalPara->m_studyTimeMinute = GET_UINT16_LITTLEENDIAN(buf);
			break;
		}
		case ZB_PARM_LISTEN_ARTICLES: {
			pstGlobalPara->m_listenArticles = GET_UINT16_LITTLEENDIAN(buf);
			break;
		}
		case ZB_PARM_LISTEN_REPEAT_CNT: {
			pstGlobalPara->m_listenRepeatCnt = GET_UINT16_LITTLEENDIAN(buf);
			break;
		}
		default:
			break;
	}
	ESP_LOGI(TAG, "setPrjGlobalPara type %d, ", (int)type);
	return 1;
}

int remove_global_parm_data(void)
{
	int ret = zb_remove(g_glbParmPath);	
	loadPrjGlobalParm(NULL);
	load_wifi_data(NULL);
	return(ret);
}


//======================================================================

#define C_SERIAL_HEADER_LEN 8    // 序列号数据头的长度 
#define ZB_PROD_SN_WITH_HEAD_LEN  (C_SERIAL_HEADER_LEN + ZB_PROD_SN_LEN)
#define ZB_PROD_SN_PATH  "/spiffs/devSn"


#define C_SERIAL_INDENT 0x362a  // 序列号区的头部标识.

#define SN_AREA_HEADER_DFLT "\x2a\x36\xaa\xaa\xff\xff\xff\xff"

//------------------------------------------------------
/**
 * @return  <= 0 means fail.
 */
static int _file_write_dat(const char * pcPath, const void * buf, int datLen) {
	FILE * fp = NULL;
	int iFuncRet = -1;
	if (!buf || (datLen <= 0))
		return -1;
	do {
		fp = zb_fopen(pcPath, "wb");
		if (!fp) {
			iFuncRet = -3;  break;
		}
		fwrite(buf, 1, datLen, fp);
		iFuncRet = datLen;
	} while (0);

	if (fp) {
		zb_fclose(fp);  fp = NULL;
	}
	if (iFuncRet < 0) {
		ESP_LOGI(TAG, "flWrtDat fail:%d", iFuncRet);
	}
	return iFuncRet;
}

/**
 * @return  <= 0 means fail.
 */
static int _file_read_dat(const char * pcPath, void * buf, int bufLen) {
	FILE * fp = NULL;
	int iFuncRet = -1;
	if (!buf || (bufLen <= 0))
		return -1;
	do {
		memset(buf, 0, bufLen);
		fp = zb_fopen(pcPath, "rb");
		if (!fp) {
			iFuncRet = -3;  break;
		}
		fread(buf, 1, bufLen, fp);
		iFuncRet = bufLen;
	} while (0);

	if (fp) {
		zb_fclose(fp);  fp = NULL;
	}
	if (iFuncRet < 0) {
		ESP_LOGI(TAG, "flReadDat fail:%d", iFuncRet);
	}
	return iFuncRet;
}


/** 读取产品序列号 
 * @return  > 0 means success.
 */
int zb_readProductSerialNo(char * buf, int bufLen) {
	char acSerial[ZB_PROD_SN_WITH_HEAD_LEN + 1] = { 0 };
	int iFuncRet = -1;
	printf("zb readPrjSn A");

	if (!buf) return -1;
	buf[0] = '\0';
	if (bufLen < ZB_PROD_SN_LEN) {
		return -3;
	}

#ifndef C_PC_SIM
	tz_system_info_t  stSysInf = { 0 };
	tz_get_system_info(&stSysInf);
	strlcpy(buf, stSysInf.sn_str, bufLen);
#else
	int iRet = _file_read_dat(ZB_PROD_SN_PATH, acSerial, ZB_PROD_SN_WITH_HEAD_LEN);
	uint16_t indent = GET_UINT16_LITTLEENDIAN(acSerial);
	if ((iRet <= 0) || !acSerial[0] || indent != C_SERIAL_INDENT) {
		buf[0] = '\0';
		iFuncRet = -5;
	} else {
		strncat(buf, acSerial + C_SERIAL_HEADER_LEN, bufLen - 1);
		iFuncRet = 1;
	}
#endif
	printf("zb readPrjSn Z, %s, ret %d", buf, iFuncRet);
	return iFuncRet;
}

/** 写入产品序列号 
 * @return  > 0 means success.
 */
int zb_writeProductSerialNo(const char * buf, int16_t comfirmCode) {
	int len;
	char acSerial[ZB_PROD_SN_WITH_HEAD_LEN + 1] = { 0 };

	if (comfirmCode != WRT_SN_CONFIRM_CODE) { // check for safe 
		return -3;
	}
	len = (int)strlen(buf);
	if (len > ZB_PROD_SN_LEN) {
		ESP_LOGE(TAG, "Err: zb wrtPrjSn len moreThan %d, %s\n", ZB_PROD_SN_LEN, buf);
		return -5;
	}
	if (len < ZB_PROD_SN_LEN) {
		ESP_LOGW(TAG, "warn! wrt sn that's too short:%d", len);
	}
#ifndef C_PC_SIM
	int iRet = -1;
	ESP_LOGW(TAG, "notSupport wrtSn!");
#else
	memcpy(acSerial, SN_AREA_HEADER_DFLT, sizeof(SN_AREA_HEADER_DFLT) - 1);
	*(short *)(acSerial + 2) = len;
	strncat(acSerial + C_SERIAL_HEADER_LEN, buf, ZB_PROD_SN_LEN);
	int iRet = _file_write_dat(ZB_PROD_SN_PATH, acSerial, ZB_PROD_SN_WITH_HEAD_LEN);
	if ((iRet < 0)) { // fail, retry wrt.
	#ifndef C_PC_SIM
		// vTaskDelay(pdMS_TO_TICKS(25));
	#endif
		iRet = _file_write_dat(ZB_PROD_SN_PATH, acSerial, ZB_PROD_SN_WITH_HEAD_LEN);
	}

	if (iRet > 0) {
		if (iRet > C_SERIAL_HEADER_LEN) {
			iRet -= C_SERIAL_HEADER_LEN;
		}
		ESP_LOGI(TAG, "zb wrtPrjSn: %s\n", acSerial + C_SERIAL_HEADER_LEN);
	}
	else {
	    ESP_LOGE(TAG, "Err: zb wrtPrjSn fail: %s\n", acSerial + C_SERIAL_HEADER_LEN);
	}
#endif
	return iRet;
}

//==================================================
//
static uint16_t checksum16(uint16_t * buf, int len)
{
	unsigned long sum = 0;
	int i = 0;
	for(i = 0; i < len; i++)
	{
		sum += buf[i];
	}
	return((uint16_t)(sum & 0xffff));
}

//==================================================
//处理wifi 列表数据做checksum处理, 同时会填入文件头 
static void wifi_parm_checksum(prj_global_network_parm_t * p_wifi_parm)
{
	uint16_t	chk_sum = 0;
	strcpy(p_wifi_parm->identify_str, C_GLOBAL_WIFILIST_MARK);

	chk_sum = checksum16(p_wifi_parm, sizeof(prj_global_network_parm_t)/2-1);
	p_wifi_parm->checksum = chk_sum;
	return;
}

//处理wifi 列表数据做checksum 验证 
// @return : 1 表示 ok, 0 表示失败 
static int wifi_parm_check(prj_global_network_parm_t * p_wifi_parm)
{
	uint16_t  chk_sum = 0;
	chk_sum = checksum16(p_wifi_parm, sizeof(prj_global_network_parm_t)/2-1);
	
	ESP_LOGI(TAG, "%s, [%d] identify_str: %s ",__FUNCTION__, __LINE__, p_wifi_parm->identify_str);
	if(strcmp(p_wifi_parm->identify_str, C_GLOBAL_WIFILIST_MARK) != 0)
	{
		return(0);
	}
	
	if(p_wifi_parm->checksum == chk_sum)
	{
		ESP_LOGI(TAG, "%s, [%d] checksum: %02x Correct",__FUNCTION__, __LINE__, chk_sum);
		return(1);
	}
	else
	{
		ESP_LOGI(TAG, "%s, [%d] checksum: %02x, file_checksum: %02xerror",__FUNCTION__, __LINE__, p_wifi_parm->checksum, chk_sum);
		return(0);

	}
	
}

//==================================================
//读wifi 名称密码列表文件
int load_wifi_data(prj_global_network_parm_t* p_wifi_parm)
{
	prj_global_network_parm_t t_wifi_parm = { 0 };

	ESP_LOGI(TAG, "%s, [%d] p_wifi_parm: %p ",__FUNCTION__, __LINE__, p_wifi_parm);
	if (!p_wifi_parm) {
		p_wifi_parm = &g_network_data;
	}
	ESP_LOGI(TAG, "prj_global_parm_t size: %d \n", sizeof(prj_global_network_parm_t));
	
	FILE * f_cfg = zb_fopen(g_wifi_recent_path, "rb");
	ESP_LOGI(TAG, "zb_fopen f_cfg : %p \n", f_cfg);
	if (f_cfg)
	{
		int ret = zb_fread(&t_wifi_parm, 1, sizeof(prj_global_network_parm_t), f_cfg);
		ESP_LOGI(TAG, "rd ret %d", ret);
	} 
	else 
	{
		ESP_LOGW(TAG, "load_wfif_data fail to open: %s", g_wifi_recent_path);
	}
	if (!f_cfg|| !wifi_parm_check(&t_wifi_parm)) 
	{
		ESP_LOGI("", "%s, [%d]  p_wifi_parm intial",__FUNCTION__, __LINE__);
		memset(&t_wifi_parm, 0, sizeof(prj_global_network_parm_t));
		
		wifi_parm_checksum(&t_wifi_parm);
		
		if (f_cfg) 
		{
			zb_fclose(f_cfg);  
			f_cfg = NULL;
		}
	} 
	else 
	{
		ESP_LOGI("", "%s, [%d]  p_wifi_parm identify_str: %s",__FUNCTION__, __LINE__, t_wifi_parm.identify_str);
		zb_fclose(f_cfg);  
		f_cfg = NULL;
		
	}
	(*p_wifi_parm) = t_wifi_parm;
	
	ESP_LOGI(TAG, "%s, [%d] Z",__FUNCTION__, __LINE__);

	return(1);
}

//===========================================
//存储wifi 名称密码列表文件
void save_wifi_data(void)
{
	prj_global_network_parm_t t_wifi_parm = { 0 };

	load_wifi_data(&t_wifi_parm);
	
	ESP_LOGI("", "%s, [%d] ",__FUNCTION__, __LINE__);
	if (memcmp(&t_wifi_parm, &g_network_data, sizeof(prj_global_network_parm_t)) == 0) 
	{
		ESP_LOGI("", "%s, [%d] not need to save!",__FUNCTION__, __LINE__);
		
		return;
	}

	ESP_LOGI("", "%s, [%d] ",__FUNCTION__, __LINE__);

	FILE * f_cfg = NULL;
	do {
		if (!zb_isFileExist(GLB_PARAM_PATH)) {
			zb_mkdir((GLB_PARAM_PATH), 0777);
		}
		f_cfg = zb_fopen(g_wifi_recent_path, "wb");
		if (!f_cfg) {
			ESP_LOGE(TAG, "Fail to crt fl %s", g_wifi_recent_path);
			break;
		}
		wifi_parm_checksum(&g_network_data);
		ESP_LOGI("", "%s, [%d] write g_wifi_recent_path ",__FUNCTION__, __LINE__);
		zb_fwrite(&g_network_data, 1, sizeof(prj_global_network_parm_t), f_cfg);
		zb_fclose(f_cfg);  f_cfg = NULL;
		ESP_LOGI(TAG, "%s, [%d], ok ",__FUNCTION__, __LINE__);
	} while (0);

	
	return;
}


